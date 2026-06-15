package com.voyager.ef;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;

import org.libsdl.app.SDLActivity;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Voyager EF engine activity.
 *
 * Before the SDL/native engine starts, extract the port-supplied game files
 * bundled in the APK (rebuilt vm/ui.qvm, gfx/touch button art, the one-time
 * android_defaults.cfg and the autoexec stub) into the OBB directory the
 * engine uses as fs_basepath (/sdcard/Android/obb/com.voyager.ef/baseEF).
 *
 * Extraction runs once per APK version: a marker file baseEF/.assets_version
 * holding the versionCode is written after a successful extraction and
 * checked on later launches. autoexec.cfg is never overwritten once present
 * (users customize it). All files are written via a .tmp + rename so a
 * crash mid-write never leaves a truncated file in place.
 *
 * The retail pak0-3.pk3 are NOT bundled (not redistributable) —
 * ImportActivity (the launcher) walks the user through importing them.
 */
public class EFActivity extends SDLActivity {
    private static final String TAG = "VoyagerSP";
    private static final String VERSION_MARKER = ".assets_version";

    /** Non-null when asset extraction failed; blocks engine startup. */
    private String mFatalError;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        try {
            File obbRoot = getObbDir();   // Android/obb/com.voyager.ef
            if (obbRoot == null) {
                mFatalError = "Cannot access the app's OBB storage directory "
                        + "(shared storage unavailable).";
            } else {
                obbRoot.mkdirs();
                extractAssetsIfNeeded(new File(obbRoot, "baseEF"));
            }
        } catch (Exception e) {
            Log.e(TAG, "asset extraction to OBB failed", e);
            mFatalError = "Extracting bundled game files failed: " + e;
        }

        // Note: super.onCreate() must always run (Activity contract). When
        // mFatalError is set, our loadLibraries() override below throws, so
        // SDLActivity takes its "broken libraries" path: it shows a blocking
        // error dialog containing the message and never starts the engine.
        super.onCreate(savedInstanceState);

        // Go immersive-fullscreen NOW, before the first layout pass.
        // SDLActivity.onCreate() calls setWindowStyle(false) (system bars
        // visible) and only switches to immersive later, asynchronously, when
        // the native engine creates its fullscreen window. By then VKimp_Init
        // has already read the drawable size, so glConfig gets the bar-inset
        // size (e.g. 1794x1017) while the Vulkan surface ends up full-screen
        // (1920x1080) — the frame doesn't fill the screen on the non-FBO path
        // and is slightly stretched on the FBO path. Applying the flags here
        // (and marking mFullscreenModeActive so SDLActivity's
        // onSystemUiVisibilityChange handler keeps re-asserting them) makes
        // the surface full-size before the engine ever measures it.
        applyImmersiveMode();

        if (mFatalError != null) {
            AlertDialog.Builder b = new AlertDialog.Builder(this);
            b.setTitle("Voyager EF error");
            b.setMessage(mFatalError + "\n\nThe game cannot start.");
            b.setCancelable(false);
            b.setPositiveButton("Quit", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            });
            b.show();
        }
    }

    /** Same flag set SDLActivity uses for COMMAND_CHANGE_WINDOW_STYLE(1). */
    private void applyImmersiveMode() {
        int flags = View.SYSTEM_UI_FLAG_FULLSCREEN |
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE;
        getWindow().getDecorView().setSystemUiVisibility(flags);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        getWindow().clearFlags(WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
        // SDLActivity re-asserts immersive on visibility changes when this
        // flag is set, so the bars stay hidden across focus loss/regain.
        SDLActivity.mFullscreenModeActive = true;
    }

    @Override
    public void loadLibraries() {
        if (mFatalError != null) {
            // Aborts SDLActivity.onCreate() before any engine setup.
            throw new RuntimeException(mFatalError);
        }
        super.loadLibraries();
    }

    /** Current APK versionCode (used for the extraction marker). */
    private int apkVersionCode() {
        try {
            return getPackageManager()
                    .getPackageInfo(getPackageName(), 0).versionCode;
        } catch (Exception e) {
            Log.w(TAG, "could not read versionCode", e);
            return -1;
        }
    }

    private void extractAssetsIfNeeded(File baseEF) throws IOException {
        int versionCode = apkVersionCode();
        File marker = new File(baseEF, VERSION_MARKER);

        if (versionCode > 0 && marker.isFile()
                && String.valueOf(versionCode).equals(readSmallFile(marker))) {
            Log.i(TAG, "assets already extracted for versionCode " + versionCode);
            return;
        }

        copyAssetDir(getAssets(), "baseEF", baseEF);

        if (versionCode > 0) {
            writeSmallFile(marker, String.valueOf(versionCode));
        }
    }

    private static String readSmallFile(File f) {
        try (InputStream in = new FileInputStream(f)) {
            byte[] buf = new byte[64];
            int n = in.read(buf);
            return n > 0 ? new String(buf, 0, n, "UTF-8").trim() : "";
        } catch (IOException e) {
            return "";
        }
    }

    private static void writeSmallFile(File f, String content) throws IOException {
        try (OutputStream out = new FileOutputStream(f)) {
            out.write(content.getBytes("UTF-8"));
        }
    }

    private void copyAssetDir(AssetManager am, String assetPath, File dest) throws IOException {
        String[] children = am.list(assetPath);
        if (children == null || children.length == 0) {
            copyAssetFile(am, assetPath, dest);   // leaf = file
            return;
        }
        dest.mkdirs();
        for (String child : children) {
            copyAssetDir(am, assetPath + "/" + child, new File(dest, child));
        }
    }

    private void copyAssetFile(AssetManager am, String assetPath, File dest) throws IOException {
        // Never clobber a user-customized autoexec.cfg.
        if (dest.getName().equals("autoexec.cfg") && dest.isFile()) {
            Log.i(TAG, "keeping existing " + dest);
            return;
        }

        // Write to .tmp and rename so a crash never leaves a truncated file.
        File tmp = new File(dest.getParentFile(), dest.getName() + ".tmp");
        boolean ok = false;
        try (InputStream in = am.open(assetPath);
             OutputStream out = new FileOutputStream(tmp)) {
            byte[] buf = new byte[65536];
            int n;
            while ((n = in.read(buf)) != -1) {
                out.write(buf, 0, n);
            }
            ok = true;
        } finally {
            if (!ok) {
                tmp.delete();
            }
        }
        if (dest.exists() && !dest.delete()) {
            tmp.delete();
            throw new IOException("could not replace " + dest);
        }
        if (!tmp.renameTo(dest)) {
            tmp.delete();
            throw new IOException("could not rename " + tmp + " -> " + dest);
        }
        Log.i(TAG, "extracted " + assetPath + " -> " + dest);
    }
}
