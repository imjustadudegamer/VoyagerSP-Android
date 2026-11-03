package com.voyager.ef;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ContentResolver;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.util.Log;

// The resource class is generated in the app's namespace (org.libsdl.app).
import org.libsdl.app.R;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;

import java.io.File;
import java.io.FileInputStream;
import java.io.BufferedReader;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
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
    /** Config the engine execs before it resolves the renderer module. */
    private static final String CONFIG_NAME = "efspconfig.cfg";
    private static final String RENDERER_CVAR = "cl_renderer";
    /** The renderer modules this APK ships; the value is what CL_InitRef formats in. */
    private static final String RENDERER_VULKAN = "vulkan";
    private static final String RENDERER_GLES3 = "gles3";

    private File mBaseEFDir;
    /** The first-run panel (game data + renderer), kept so it can be re-shown. */
    private Dialog mImportDialog;
    /** Renderer the panel currently shows; null until read from the config. */
    private String mRendererChoice;

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

        firstRunOrLaunch();
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

    /**
     * First-run gate.
     *
     * The engine loads its renderer module once at startup, so the only moment a
     * choice can take effect immediately is before the game starts. That is this
     * screen: game data plus the renderer rows, shown until a renderer has been
     * recorded. Once both are settled the game starts directly.
     */
    private void firstRunOrLaunch() {
        if (allRequiredPresent() && rendererAlreadyChosen()) {
            launchGame();
            return;
        }
        showImportDialog(null);
    }

    /** The renderer the archive selects, or null when it names none yet. */
    private String readRendererChoice() {
        BufferedReader r = null;
        try {
            r = new BufferedReader(new InputStreamReader(
                    new FileInputStream(new File(mBaseEFDir, CONFIG_NAME)), "UTF-8"));
            String line;
            while ((line = r.readLine()) != null) {
                String t = line.trim();
                if (t.startsWith("//") || t.startsWith("#")) {
                    continue;
                }
                if (t.startsWith("seta ") || t.startsWith("set ") || t.startsWith("seta\t")) {
                    String rest = t.substring(t.indexOf(' ') + 1).trim();
                    if (rest.startsWith(RENDERER_CVAR)) {
                        String v = rest.substring(RENDERER_CVAR.length()).trim();
                        if (v.startsWith("\"")) {
                            int q = v.indexOf('"', 1);
                            v = q > 1 ? v.substring(1, q) : v;
                        }
                        return v;
                    }
                }
            }
        } catch (Exception ignored) {
            // no config yet: first run
        } finally {
            if (r != null) try { r.close(); } catch (Exception ignored) {}
        }
        return null;
    }

    /** Record a renderer row as soon as the user picks it, not at launch. */
    private void chooseRenderer(String value) {
        mRendererChoice = value;
        writeRendererChoice(value);
    }

    /** True when the archived config already carries a renderer choice. */
    private boolean rendererAlreadyChosen() {
        BufferedReader r = null;
        try {
            r = new BufferedReader(new InputStreamReader(
                    new FileInputStream(new File(mBaseEFDir, CONFIG_NAME)), "UTF-8"));
            String line;
            while ((line = r.readLine()) != null) {
                String t = line.trim();
                if (t.startsWith("//") || t.startsWith("#")) {
                    continue;
                }
                if (t.startsWith("seta ") || t.startsWith("set ") || t.startsWith("seta\t")) {
                    String rest = t.substring(t.indexOf(' ') + 1).trim();
                    if (rest.startsWith(RENDERER_CVAR)) {
                        return true;
                    }
                }
            }
        } catch (Exception ignored) {
            // no config yet: first run
        } finally {
            if (r != null) try { r.close(); } catch (Exception ignored) {}
        }
        return false;
    }

    /**
     * Rewrite the config with the chosen renderer. The engine execs this file
     * before it resolves the renderer module, so the choice is live on this
     * launch; the write is atomic (temp + rename) so a kill cannot leave a
     * half-written config.
     */
    private void writeRendererChoice(String name) {
        File cfg = new File(mBaseEFDir, CONFIG_NAME);
        File tmp = new File(mBaseEFDir, CONFIG_NAME + ".tmp");
        BufferedReader r = null;
        PrintWriter w = null;
        boolean ok = false;
        try {
            r = cfg.isFile()
                    ? new BufferedReader(new InputStreamReader(new FileInputStream(cfg), "UTF-8"))
                    : null;
            w = new PrintWriter(new OutputStreamWriter(new FileOutputStream(tmp), "UTF-8"));
            String line;
            if (r != null) {
                while ((line = r.readLine()) != null) {
                    String t = line.trim();
                    if ((t.startsWith("seta ") || t.startsWith("set "))
                            && t.substring(t.indexOf(' ') + 1).trim().startsWith(RENDERER_CVAR)) {
                        continue;   // superseded by the line written below
                    }
                    w.println(line);
                }
            }
            w.println("seta " + RENDERER_CVAR + " \"" + name + "\"");
            w.flush();
            ok = !w.checkError();
        } catch (Exception e) {
            Log.e(TAG, "could not write renderer choice", e);
        } finally {
            if (r != null) try { r.close(); } catch (Exception ignored) {}
            if (w != null) w.close();
        }
        if (!ok || !tmp.renameTo(cfg)) {
            tmp.delete();
            Log.w(TAG, "renderer choice not persisted; the game will use its default");
        }
    }

    private void launchGame() {
        startActivity(new Intent(this, EFActivity.class));
        finish();
    }

    /**
     * First-run panel: the game-data instructions and the renderer rows, as one
     * screen. The renderer sits above the buttons so it is a visible choice before
     * any file is picked; the primary button imports when data is missing and
     * starts the game once it is present.
     */
    private void showImportDialog(String extraLine) {
        if (mImportDialog != null) {
            mImportDialog.dismiss();
            mImportDialog = null;
        }
        if (mRendererChoice == null) {
            mRendererChoice = readRendererChoice();
        }
        final boolean dataReady = allRequiredPresent();

        View panel = getLayoutInflater().inflate(R.layout.import_dialog, null);

        String msg = (extraLine != null ? extraLine + "\n\n" : "");
        if (dataReady) {
            msg += "The game data is in place. Pick a renderer and start.";
        } else {
            msg += "Star Trek: Voyager - Elite Force needs the game data from your "
                    + "retail copy of the game:\n\n"
                    + "    pak0.pk3\n    pak1.pk3\n    pak2.pk3\n    pak3.pk3\n"
                    + "    efgamex86.dll\n\n"
                    + "(The paks are in the BaseEF folder of an installed copy; "
                    + "pak0.pk3 is about 540 MB. efgamex86.dll is in the game's main "
                    + "install folder and confirms you own the game.)\n\n"
                    + "Copy them onto this device, then tap \"Select game files\" and "
                    + "pick all of them.\n\n"
                    + "Still missing: " + missingList();
        }
        ((TextView) panel.findViewById(R.id.import_body)).setText(msg);

        RadioButton vulkan = panel.findViewById(R.id.import_renderer_vulkan);
        RadioButton gles3 = panel.findViewById(R.id.import_renderer_gles3);
        (RENDERER_GLES3.equalsIgnoreCase(mRendererChoice) ? gles3 : vulkan).setChecked(true);

        RadioGroup group = panel.findViewById(R.id.import_renderer_group);
        group.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup g, int id) {
                chooseRenderer(id == R.id.import_renderer_gles3 ? RENDERER_GLES3 : RENDERER_VULKAN);
            }
        });

        Button primary = panel.findViewById(R.id.import_select);
        primary.setText(dataReady ? "Start game" : "Select game files");
        primary.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (allRequiredPresent()) {
                    launchGame();
                } else {
                    openPicker();
                }
            }
        });
        panel.findViewById(R.id.import_quit).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

        Dialog d = new Dialog(this);
        d.requestWindowFeature(Window.FEATURE_NO_TITLE);
        d.setContentView(panel);
        d.getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        d.setCancelable(false);
        mImportDialog = d;
        d.show();
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
                            firstRunOrLaunch();
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
