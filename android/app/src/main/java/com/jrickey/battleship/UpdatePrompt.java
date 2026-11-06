package com.jrickey.battleship;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.widget.TextView;

import java.io.IOException;

/**
 * Boot-time update flow: checks the latest release in the background (bounded wait), offers the update in a
 * dialog and, when accepted, downloads and installs it with the progress shown in the boot status text.
 * Any failure or a refusal continues to the game.
 */
final class UpdatePrompt {
    private static final String TAG = "ssb64.updater";
    private static final long CHECK_WAIT_MS = 4000;

    private final Activity mActivity;
    private final TextView mStatus;
    private final Runnable mContinue;
    private final Handler mMain = new Handler(Looper.getMainLooper());
    private boolean mDecided;

    UpdatePrompt(Activity activity, TextView status, Runnable onContinue) {
        mActivity = activity;
        mStatus = status;
        mContinue = onContinue;
    }

    /** Runs the check, or continues right away on a build that never self-updates. */
    void run() {
        if (!AppUpdater.isUpdatableBuild()) {
            mContinue.run();
            return;
        }
        mStatus.setText("Checking for updates...");
        mMain.postDelayed(() -> decide(null), CHECK_WAIT_MS);
        new Thread(() -> {
            AppUpdater.Release release = AppUpdater.findUpdate();
            mMain.post(() -> decide(release));
        }, "ssb64-update-check").start();
    }

    private void decide(AppUpdater.Release release) {
        if (mDecided) {
            return;
        }
        mDecided = true;
        if (release == null) {
            mContinue.run();
            return;
        }
        new AlertDialog.Builder(mActivity)
            .setTitle("Update available")
            .setMessage("BattleShip Remix " + release.tag + " is available (installed: "
                + BuildConfig.VERSION_NAME + "). Download and install it now?")
            .setPositiveButton("Update", (d, w) -> install(release))
            .setNegativeButton("Later", (d, w) -> mContinue.run())
            .setCancelable(false)
            .show();
    }

    private void install(AppUpdater.Release release) {
        InstallStatusReceiver.setFailureListener(reason -> mMain.post(() -> failAndContinue(reason)));
        mStatus.setText("Downloading " + release.tag + "...");
        new Thread(() -> {
            try {
                AppUpdater.downloadAndInstall(mActivity, release,
                    percent -> mMain.post(() -> mStatus.setText("Downloading " + release.tag + ": " + percent + "%")));
                mMain.post(() -> mStatus.setText("Confirm the install to finish the update."));
            } catch (IOException | RuntimeException e) {
                Log.w(TAG, "Update install failed", e);
                mMain.post(() -> failAndContinue("Update failed: " + e.getMessage()));
            }
        }, "ssb64-update-download").start();
    }

    private void failAndContinue(String reason) {
        InstallStatusReceiver.setFailureListener(null);
        mStatus.setText(reason);
        mMain.postDelayed(mContinue, 4000);
    }
}
