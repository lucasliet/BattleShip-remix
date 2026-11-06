package com.jrickey.battleship;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentSender;
import android.content.pm.PackageInstaller;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.nio.charset.StandardCharsets;

/**
 * Self-updater of the Remix APK: reads the latest GitHub release of the fork, downloads its Android APK
 * straight into a PackageInstaller session and commits it; Android then asks the player to confirm the
 * install (InstallStatusReceiver starts that prompt). Network calls block, so run them off the UI thread.
 */
public final class AppUpdater {
    private static final String TAG = "ssb64.updater";
    private static final String LATEST_RELEASE_URL =
        "https://api.github.com/repos/lucasliet/BattleShip-remix/releases/latest";
    private static final String APK_ASSET_NAME = "BattleShip-Remix-android.apk";
    private static final String DOWNLOAD_URL_PREFIX = "https://github.com/";
    private static final int CONNECT_TIMEOUT_MS = 5000;
    private static final int READ_TIMEOUT_MS = 15000;

    /** A newer release with an APK for this app. */
    public static final class Release {
        public final String tag;
        public final String apkUrl;
        public final long apkSize;

        Release(String tag, String apkUrl, long apkSize) {
            this.tag = tag;
            this.apkUrl = apkUrl;
            this.apkSize = apkSize;
        }
    }

    /** Receives the download progress, from 0 to 100. */
    public interface ProgressListener {
        void onProgress(int percent);
    }

    private AppUpdater() { }

    /**
     * Whether this build takes part in self-updates: only release-tagged Remix builds (versionName
     * "v...", set by the release CI); dev and debug builds never update themselves.
     */
    public static boolean isUpdatableBuild() {
        return "remix".equals(BuildConfig.SSB64_VERSION)
            && !BuildConfig.DEBUG
            && BuildConfig.VERSION_NAME.startsWith("v");
    }

    /**
     * Looks up the latest release.
     *
     * @return the release when its tag differs from this build and it has an APK, otherwise null
     *         (also on any network or parse failure, which is logged).
     */
    public static Release findUpdate() {
        if (!isUpdatableBuild()) {
            return null;
        }
        try {
            JSONObject release = new JSONObject(fetchText(LATEST_RELEASE_URL));
            String tag = release.getString("tag_name");
            if (tag.equals(BuildConfig.VERSION_NAME)) {
                Log.i(TAG, "Up to date (" + tag + ")");
                return null;
            }
            return findApkAsset(tag, release.getJSONArray("assets"));
        } catch (IOException | JSONException | RuntimeException e) {
            Log.w(TAG, "Update check failed: " + e);
            return null;
        }
    }

    /**
     * Downloads the release APK into an install session and commits it; the system install prompt
     * follows through InstallStatusReceiver.
     *
     * @throws IOException when the download or the session fails; the session is abandoned.
     */
    public static void downloadAndInstall(Context ctx, Release release, ProgressListener listener)
            throws IOException {
        PackageInstaller installer = ctx.getPackageManager().getPackageInstaller();
        PackageInstaller.SessionParams params =
            new PackageInstaller.SessionParams(PackageInstaller.SessionParams.MODE_FULL_INSTALL);
        params.setAppPackageName(ctx.getPackageName());
        params.setSize(release.apkSize);
        int sessionId = installer.createSession(params);
        try (PackageInstaller.Session session = installer.openSession(sessionId)) {
            writeApk(session, release, listener);
            session.commit(installStatusSender(ctx, sessionId));
        } catch (IOException | RuntimeException e) {
            installer.abandonSession(sessionId);
            throw e;
        }
    }

    private static Release findApkAsset(String tag, JSONArray assets) throws JSONException {
        for (int i = 0; i < assets.length(); i++) {
            JSONObject asset = assets.getJSONObject(i);
            if (!APK_ASSET_NAME.equals(asset.optString("name"))) {
                continue;
            }
            String url = asset.getString("browser_download_url");
            if (!url.startsWith(DOWNLOAD_URL_PREFIX)) {
                Log.w(TAG, "Ignoring APK URL outside " + DOWNLOAD_URL_PREFIX + ": " + url);
                return null;
            }
            return new Release(tag, url, asset.optLong("size", -1));
        }
        Log.i(TAG, "Release " + tag + " has no " + APK_ASSET_NAME);
        return null;
    }

    private static void writeApk(PackageInstaller.Session session, Release release, ProgressListener listener)
            throws IOException {
        HttpURLConnection conn = open(release.apkUrl, "application/octet-stream");
        try (InputStream in = conn.getInputStream();
             OutputStream out = session.openWrite("base.apk", 0, release.apkSize)) {
            copyWithProgress(in, out, release.apkSize, listener);
            session.fsync(out);
        } finally {
            conn.disconnect();
        }
    }

    private static void copyWithProgress(InputStream in, OutputStream out, long total, ProgressListener listener)
            throws IOException {
        byte[] buf = new byte[256 * 1024];
        long done = 0;
        int lastPercent = -1;
        int n;
        while ((n = in.read(buf)) > 0) {
            out.write(buf, 0, n);
            done += n;
            int percent = (total > 0) ? (int) (done * 100 / total) : 0;
            if (percent != lastPercent) {
                lastPercent = percent;
                listener.onProgress(percent);
            }
        }
        if (total > 0 && done != total) {
            throw new IOException("APK download ended at " + done + " of " + total + " bytes");
        }
    }

    private static IntentSender installStatusSender(Context ctx, int sessionId) {
        Intent intent = new Intent(ctx, InstallStatusReceiver.class);
        int flags = PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_MUTABLE;
        return PendingIntent.getBroadcast(ctx, sessionId, intent, flags).getIntentSender();
    }

    private static String fetchText(String url) throws IOException {
        HttpURLConnection conn = open(url, "application/vnd.github+json");
        try (InputStream in = conn.getInputStream()) {
            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            byte[] buf = new byte[16 * 1024];
            int n;
            while ((n = in.read(buf)) > 0) {
                bos.write(buf, 0, n);
            }
            return new String(bos.toByteArray(), StandardCharsets.UTF_8);
        } finally {
            conn.disconnect();
        }
    }

    private static HttpURLConnection open(String url, String accept) throws IOException {
        HttpURLConnection conn = (HttpURLConnection) new URL(url).openConnection();
        conn.setConnectTimeout(CONNECT_TIMEOUT_MS);
        conn.setReadTimeout(READ_TIMEOUT_MS);
        conn.setInstanceFollowRedirects(true);
        conn.setRequestProperty("User-Agent", "BattleShip-Remix-Android");
        conn.setRequestProperty("Accept", accept);
        int status = conn.getResponseCode();
        if (status != HttpURLConnection.HTTP_OK) {
            conn.disconnect();
            throw new IOException("HTTP " + status + " from " + url);
        }
        return conn;
    }
}
