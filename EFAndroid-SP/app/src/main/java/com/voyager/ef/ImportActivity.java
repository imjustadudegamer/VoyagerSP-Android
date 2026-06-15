package com.voyager.ef;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Launcher activity: makes sure the retail game data (pak0.pk3 .. pak3.pk3)
 * plus a genuine retail efgamex86.dll are present in the app's OBB directory
 * before the engine starts.
 *
 * The DLL is not loaded or executed by this port — it is required only as
 * proof that the user owns a real copy of the game, which is what the game's
 * source license permits this port to operate alongside.
 *
 * On Android 11+ ordinary file managers cannot write into
 * Android/obb/<package>, so users cannot copy the files there by hand.
 * This activity lets them pick the files with the system document picker
 * (Storage Access Framework) and stream-copies them into the OBB directory,
 * which the app itself may always write to without permissions.
 *
 * Deliberately uses only android.app classes (no androidx dependency).
 */
public class ImportActivity extends Activity {
    private static final String TAG = "VoyagerEF";
    private static final int REQUEST_PICK_PAKS = 42;
    private static final String[] REQUIRED_PAKS = {
            "pak0.pk3", "pak1.pk3", "pak2.pk3", "pak3.pk3"
    };
    /** A genuine retail DLL must accompany the paks as proof of ownership. */
    private static final String REQUIRED_DLL = "efgamex86.dll";

    private File mBaseEFDir;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        File obbRoot = getObbDir();   // Android/obb/<package>
        if (obbRoot == null) {
            showFatal("Cannot access the app's OBB storage directory "
                    + "(shared storage unavailable). The game cannot run.");
            return;
        }
        mBaseEFDir = new File(obbRoot, "baseEF");
        mBaseEFDir.mkdirs();

        if (allRequiredPresent()) {
            launchGame();
        } else {
            showImportDialog(null);
        }
    }

    private boolean allRequiredPresent() {
        return allPaksPresent() && dllPresent();
    }

    private boolean allPaksPresent() {
        for (String pak : REQUIRED_PAKS) {
            File f = new File(mBaseEFDir, pak);
            if (!f.isFile() || f.length() == 0) {
                return false;
            }
        }
        return true;
    }

    /**
     * Verify a genuine retail DLL sits alongside the paks. Lenient on version:
     * any real efgamex86.dll passes — we only check it is a real DOS/PE binary
     * (the "MZ" header) of a plausible size, not a specific build/hash.
     */
    private boolean dllPresent() {
        File f = new File(mBaseEFDir, REQUIRED_DLL);
        if (!f.isFile() || f.length() < 4096) {
            return false;
        }
        InputStream in = null;
        try {
            in = new FileInputStream(f);
            int m = in.read();
            int z = in.read();
            return m == 'M' && z == 'Z';
        } catch (Exception e) {
            return false;
        } finally {
            if (in != null) try { in.close(); } catch (Exception ignored) {}
        }
    }

    private String missingList() {
        StringBuilder sb = new StringBuilder();
        for (String pak : REQUIRED_PAKS) {
            File f = new File(mBaseEFDir, pak);
            if (!f.isFile() || f.length() == 0) {
                if (sb.length() > 0) sb.append(", ");
                sb.append(pak);
            }
        }
        if (!dllPresent()) {
            if (sb.length() > 0) sb.append(", ");
            sb.append(REQUIRED_DLL);
        }
        return sb.toString();
    }

    private void launchGame() {
        startActivity(new Intent(this, EFActivity.class));
        finish();
    }

    /** First-run / missing-data dialog. extraLine, if non-null, is prepended. */
    private void showImportDialog(String extraLine) {
        String msg = (extraLine != null ? extraLine + "\n\n" : "")
                + "Star Trek: Voyager - Elite Force needs the game data from "
                + "your retail copy of the game:\n\n"
                + "    pak0.pk3\n    pak1.pk3\n    pak2.pk3\n    pak3.pk3\n"
                + "    efgamex86.dll\n\n"
                + "(The paks are in the BaseEF folder of an installed copy; "
                + "pak0.pk3 is about 540 MB. efgamex86.dll is in the game's "
                + "main install folder and confirms you own the game.)\n\n"
                + "Copy them onto this device, then tap \"Select game files\" "
                + "and pick all of them.\n\n"
                + "Still missing: " + missingList();

        AlertDialog.Builder b = new AlertDialog.Builder(this);
        b.setTitle("Game data required");
        b.setMessage(msg);
        b.setCancelable(false);
        b.setPositiveButton("Select game files", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                openPicker();
            }
        });
        b.setNegativeButton("Quit", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        });
        b.show();
    }

    private void showFatal(String msg) {
        AlertDialog.Builder b = new AlertDialog.Builder(this);
        b.setTitle("Error");
        b.setMessage(msg);
        b.setCancelable(false);
        b.setPositiveButton("Quit", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                finish();
            }
        });
        b.show();
    }

    private void openPicker() {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_ALLOW_MULTIPLE, true);
        try {
            startActivityForResult(intent, REQUEST_PICK_PAKS);
        } catch (Exception e) {
            showFatal("No document picker available on this device: " + e);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode != REQUEST_PICK_PAKS) {
            return;
        }
        if (resultCode != RESULT_OK || data == null) {
            // user backed out of the picker
            showImportDialog("No files were selected.");
            return;
        }

        // Collect picked URIs (single pick -> getData, multi -> clipData)
        final Uri[] uris;
        if (data.getClipData() != null) {
            int count = data.getClipData().getItemCount();
            uris = new Uri[count];
            for (int i = 0; i < count; i++) {
                uris[i] = data.getClipData().getItemAt(i).getUri();
            }
        } else if (data.getData() != null) {
            uris = new Uri[]{data.getData()};
        } else {
            showImportDialog("No files were selected.");
            return;
        }

        copyPickedFiles(uris);
    }

    private void copyPickedFiles(final Uri[] uris) {
        final ProgressDialog progress = new ProgressDialog(this);
        progress.setTitle("Importing game data");
        progress.setMessage("Starting…");
        progress.setCancelable(false);
        progress.show();

        new Thread(new Runnable() {
            @Override
            public void run() {
                String error = null;
                int copied = 0;
                for (Uri uri : uris) {
                    String name = displayName(uri);
                    if (name == null) {
                        Log.w(TAG, "skipping document with unknown name: " + uri);
                        continue;
                    }
                    setProgressText(progress, "Copying " + name
                            + " (" + (copied + 1) + "/" + uris.length + ")…\n"
                            + "pak0.pk3 is ~540 MB; this can take a while.");
                    try {
                        copyDocument(uri, new File(mBaseEFDir, name), progress, name);
                        copied++;
                    } catch (Exception e) {
                        Log.e(TAG, "failed to import " + name, e);
                        error = "Failed to copy " + name + ": " + e;
                        break;
                    }
                }

                final String fError = error;
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        try {
                            progress.dismiss();
                        } catch (Exception ignored) {
                        }
                        if (isFinishing()) {
                            return;
                        }
                        if (fError != null) {
                            showImportDialog(fError);
                        } else if (allRequiredPresent()) {
                            launchGame();
                        } else {
                            showImportDialog("Some required files are still missing.");
                        }
                    }
                });
            }
        }, "ef-pak-import").start();
    }

    private void setProgressText(final ProgressDialog progress, final String text) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (progress.isShowing()) {
                    progress.setMessage(text);
                }
            }
        });
    }

    /** Resolve the user-visible file name of a picked document. */
    private String displayName(Uri uri) {
        Cursor c = null;
        try {
            c = getContentResolver().query(uri, null, null, null, null);
            if (c != null && c.moveToFirst()) {
                int idx = c.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (idx >= 0) {
                    String name = c.getString(idx);
                    if (name != null) {
                        // basic sanitation: no path separators
                        return new File(name).getName();
                    }
                }
            }
        } catch (Exception e) {
            Log.w(TAG, "could not query display name for " + uri, e);
        } finally {
            if (c != null) c.close();
        }
        return null;
    }

    /**
     * Stream-copy one picked document into the OBB baseEF dir.
     * Writes to a .tmp file first and renames into place so a half-finished
     * copy never looks like a valid pak.
     */
    private void copyDocument(Uri uri, File dest, ProgressDialog progress,
                              String name) throws Exception {
        ContentResolver resolver = getContentResolver();
        File tmp = new File(dest.getParentFile(), dest.getName() + ".tmp");
        InputStream in = null;
        OutputStream out = null;
        boolean ok = false;
        try {
            in = resolver.openInputStream(uri);
            if (in == null) {
                throw new java.io.IOException("could not open " + uri);
            }
            out = new FileOutputStream(tmp);
            byte[] buf = new byte[1024 * 1024];   // 1 MB chunks (pak0 ~540 MB)
            long total = 0;
            long lastReported = 0;
            int n;
            while ((n = in.read(buf)) != -1) {
                out.write(buf, 0, n);
                total += n;
                if (total - lastReported >= 32L * 1024 * 1024) {
                    lastReported = total;
                    setProgressText(progress, "Copying " + name + "…\n"
                            + (total / (1024 * 1024)) + " MB written");
                }
            }
            out.flush();
            ok = true;
        } finally {
            if (in != null) try { in.close(); } catch (Exception ignored) {}
            if (out != null) try { out.close(); } catch (Exception ignored) {}
            if (!ok) {
                tmp.delete();
            }
        }
        if (dest.exists() && !dest.delete()) {
            tmp.delete();
            throw new java.io.IOException("could not replace " + dest);
        }
        if (!tmp.renameTo(dest)) {
            tmp.delete();
            throw new java.io.IOException("could not rename " + tmp + " -> " + dest);
        }
        Log.i(TAG, "imported " + dest + " (" + dest.length() + " bytes)");
    }
}
