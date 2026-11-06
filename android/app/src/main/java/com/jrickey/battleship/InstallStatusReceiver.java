package com.jrickey.battleship;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInstaller;
import android.os.Build;
import android.util.Log;

/**
 * Receives the status of the self-update install session: starts the system confirmation prompt when the
 * installer asks for the player's action and logs the final result.
 */
public final class InstallStatusReceiver extends BroadcastReceiver {
    private static final String TAG = "ssb64.updater";

    /** Told when the install ends without the app being replaced (failure or the player cancelled). */
    interface FailureListener {
        void onInstallFailed(String reason);
    }

    private static volatile FailureListener sFailureListener;

    static void setFailureListener(FailureListener listener) {
        sFailureListener = listener;
    }

    @Override
    public void onReceive(Context ctx, Intent intent) {
        int status = intent.getIntExtra(PackageInstaller.EXTRA_STATUS, PackageInstaller.STATUS_FAILURE);
        if (status != PackageInstaller.STATUS_PENDING_USER_ACTION) {
            String message = intent.getStringExtra(PackageInstaller.EXTRA_STATUS_MESSAGE);
            Log.i(TAG, "Install status " + status + ": " + message);
            reportFailure(status, message);
            return;
        }
        Intent confirm = confirmationIntent(intent);
        if (confirm == null) {
            Log.w(TAG, "Install needs user action but carries no confirmation intent");
            return;
        }
        confirm.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        ctx.startActivity(confirm);
    }

    private static void reportFailure(int status, String message) {
        FailureListener listener = sFailureListener;
        if (status == PackageInstaller.STATUS_SUCCESS || listener == null) {
            return;
        }
        listener.onInstallFailed(describeFailure(status, message));
    }

    private static String describeFailure(int status, String message) {
        if (status == PackageInstaller.STATUS_FAILURE_ABORTED) {
            return "Update cancelled.";
        }
        if (status == PackageInstaller.STATUS_FAILURE_CONFLICT || status == PackageInstaller.STATUS_FAILURE_INCOMPATIBLE) {
            return "Update could not replace this install (signed by a different key). Uninstall it and install the APK from the release page once.";
        }
        return "Update failed: " + message;
    }

    @SuppressWarnings("deprecation")
    private static Intent confirmationIntent(Intent intent) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            return intent.getParcelableExtra(Intent.EXTRA_INTENT, Intent.class);
        }
        return intent.getParcelableExtra(Intent.EXTRA_INTENT);
    }
}
