// PS5 self-updater. Same public API as Updater.cpp (declared in enhancements.h). It talks to GitHub
// in-process through the console's HTTP library (ps5/ps5_http.cpp), downloads the release's title
// zip by its API URL (the library cannot open github.com) and unpacks it over the title folder.
// The running eboot.bin stays mapped, so the files can be replaced while the game is open; the new
// build starts the next time the game is opened.
#ifdef __PS5__

#include "enhancements.h"

#include "SwitchHttp.h"
#include "UpdateRelease.h"
#include "UpdateZipInstall.h"
#include "port_log.h"

#include <atomic>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <pthread.h>

#include <ship/Context.h>

namespace ssb64 {
namespace enhancements {

namespace {

#ifndef BATTLESHIP_CURRENT_VERSION
#define BATTLESHIP_CURRENT_VERSION "v1.0.0"
#endif

constexpr const char* kLatestReleaseApi = "https://api.github.com/repos/lucasliet/BattleShip-remix/releases/latest";
constexpr const char* kZipAssetName = "BattleShip-Remix-ps5-PPSA99264.zip";
constexpr const char* kZipRoot = "PPSA99264/";
constexpr const char* kDownloadName = "update.zip.part";
constexpr size_t kWorkerStackBytes = 1024 * 1024;

std::atomic<bool> s_updateChecked{false};
std::atomic<bool> s_updateAvailable{false};
std::atomic<bool> s_isDownloading{false};
std::atomic<bool> s_downloadComplete{false};
std::atomic<bool> s_isCheckingForUpdates{false};
std::atomic<bool> s_updateCheckFailed{false};

std::string s_latestVersion;
std::string s_updateStatus;
std::string s_downloadStatus;
ReleaseAsset s_availableRelease;
std::mutex s_stringMutex;

void SetUpdateStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(s_stringMutex);
    s_updateStatus = status;
}

void SetDownloadStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(s_stringMutex);
    s_downloadStatus = status;
}

void* RunWorkerTask(void* rawTask) {
    std::unique_ptr<std::function<void()>> task(static_cast<std::function<void()>*>(rawTask));
    (*task)();
    return nullptr;
}

/**
 * Runs a task on a detached thread with a stack large enough for TLS and JSON parsing.
 *
 * @param task Work to run.
 * @return False when the thread could not be created.
 */
bool StartWorker(std::function<void()> task) {
    pthread_attr_t attributes;
    pthread_attr_init(&attributes);
    pthread_attr_setstacksize(&attributes, kWorkerStackBytes);
    pthread_attr_setdetachstate(&attributes, PTHREAD_CREATE_DETACHED);

    auto* heapTask = new std::function<void()>(std::move(task));
    pthread_t thread;
    int result = pthread_create(&thread, &attributes, RunWorkerTask, heapTask);
    pthread_attr_destroy(&attributes);
    if (result != 0) delete heapTask;
    return result == 0;
}

void FailUpdateCheck(const std::string& status) {
    s_updateCheckFailed.store(true);
    SetUpdateStatus(status);
}

void ApplyReleaseCheckResult(const std::string& responseBody) {
    ReleaseAsset release;
    ReleaseParseStatus status = ParseLatestRelease(responseBody, kZipAssetName, &release);
    if (status == ReleaseParseStatus::Malformed) {
        FailUpdateCheck("Unable to read update information.");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        s_latestVersion = release.tag;
        s_availableRelease = ReleaseAsset{};
    }

    if (release.tag == BATTLESHIP_CURRENT_VERSION) {
        SetUpdateStatus("Up to date.");
        return;
    }
    if (status != ReleaseParseStatus::Ok || release.apiUrl.empty()) {
        port_log("Updater: release %s has no usable %s asset\n", release.tag.c_str(), kZipAssetName);
        FailUpdateCheck("Latest release has no update for this platform.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        s_availableRelease = release;
    }
    s_updateAvailable.store(true);
    SetUpdateStatus("");
}

void RunUpdateCheck() {
    std::string responseBody;
    HttpOutcome outcome = HttpGetString(kLatestReleaseApi, &responseBody);
    if (!outcome.ok) {
        port_log("Updater: release check failed (HTTP %ld): %s\n", outcome.httpStatus, outcome.error.c_str());
        FailUpdateCheck("Unable to check for updates.");
    } else {
        ApplyReleaseCheckResult(responseBody);
    }
    s_isCheckingForUpdates.store(false);
}

void FailDownload(const std::string& status) {
    SetDownloadStatus("Error: " + status);
    s_isDownloading.store(false);
}

void RunDownload(const ReleaseAsset& release, const std::string& titleDir) {
    const std::string zipPath = titleDir + "/" + kDownloadName;
    HttpOutcome outcome = HttpDownloadToFile(release.apiUrl, zipPath, [](int percent) {
        SetDownloadStatus("Downloading... " + std::to_string(percent) + "%");
    });
    if (!outcome.ok) {
        port_log("Updater: download failed (HTTP %ld): %s\n", outcome.httpStatus, outcome.error.c_str());
        FailDownload("Download failed. " + outcome.error);
        return;
    }

    SetDownloadStatus("Installing update...");
    const ZipInstallStatus install = InstallTitleZip(zipPath, kZipRoot, titleDir);
    std::remove(zipPath.c_str());
    if (install != ZipInstallStatus::Installed) {
        port_log("Updater: install failed: %s\n", DescribeZipInstallStatus(install));
        FailDownload(DescribeZipInstallStatus(install));
        return;
    }

    SetDownloadStatus("Update installed! Close and reopen the game to apply.");
    s_downloadComplete.store(true);
    s_isDownloading.store(false);
}

} // namespace

void CheckForUpdatesAsync(bool force) {
    if (s_isCheckingForUpdates.load() || s_isDownloading.load()) return;
    if (!force && s_updateChecked.load()) return;

    s_updateChecked.store(true);
    s_isCheckingForUpdates.store(true);
    s_updateCheckFailed.store(false);
    s_updateAvailable.store(false);
    s_downloadComplete.store(false);
    SetDownloadStatus("");
    SetUpdateStatus("");

    if (!StartWorker(RunUpdateCheck)) {
        FailUpdateCheck("Unable to check for updates.");
        s_isCheckingForUpdates.store(false);
    }
}

void StartGameUpdate() {
    if (s_isDownloading.load() || !s_updateAvailable.load()) return;

    ReleaseAsset release;
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        release = s_availableRelease;
    }
    if (release.apiUrl.empty()) {
        SetDownloadStatus("Error: No update download available.");
        return;
    }

    const std::string titleDir = Ship::Context::GetAppBundlePath();
    s_isDownloading.store(true);
    SetDownloadStatus("Initializing download...");
    bool started = StartWorker([release, titleDir]() { RunDownload(release, titleDir); });
    if (!started) FailDownload("Could not start the download thread.");
}

bool CanRestartAfterUpdate() {
    return false;
}

void RestartAfterUpdate() {}

bool IsCheckingForUpdates() { return s_isCheckingForUpdates.load(); }
bool IsUpdateAvailable() { return s_updateAvailable.load(); }
bool DidUpdateCheckFail() { return s_updateCheckFailed.load(); }
bool IsDownloading() { return s_isDownloading.load(); }
bool IsDownloadComplete() { return s_downloadComplete.load(); }

std::string GetUpdateStatus() { std::lock_guard<std::mutex> lock(s_stringMutex); return s_updateStatus; }
std::string GetDownloadStatus() { std::lock_guard<std::mutex> lock(s_stringMutex); return s_downloadStatus; }
std::string GetLatestVersion() { std::lock_guard<std::mutex> lock(s_stringMutex); return s_latestVersion; }

} // namespace enhancements
} // namespace ssb64

#endif
