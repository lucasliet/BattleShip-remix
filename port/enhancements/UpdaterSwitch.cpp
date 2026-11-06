// Nintendo Switch self-updater. Same public API as Updater.cpp (declared in enhancements.h), but it
// talks to GitHub in-process with libcurl instead of shelling out, and replaces the running .nro
// on the SD card. The homebrew loader keeps the running image in memory, so the file can be swapped
// while the game is open; the new build starts the next time the game is launched.
#include "enhancements.h"

#include "SwitchHttp.h"
#include "UpdateInstall.h"
#include "UpdateRelease.h"
#include "port_log.h"

#include <atomic>
#include <cstdio>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include <pthread.h>
#include <switch.h>

#include <ship/Context.h>
#include <ship/window/Window.h>

extern "C" {
extern int __system_argc;
extern char** __system_argv;
}

namespace ssb64 {
namespace enhancements {

namespace {

#ifndef BATTLESHIP_CURRENT_VERSION
#define BATTLESHIP_CURRENT_VERSION "v1.0.0"
#endif

#if defined(REGION_REMIX)
constexpr const char* kLatestReleaseApi =
    "https://api.github.com/repos/lucasliet/BattleShip-remix/releases/latest";
constexpr const char* kNroAssetName = "BattleShip-Remix.nro";
#else
constexpr const char* kLatestReleaseApi =
    "https://api.github.com/repos/JRickey/BattleShip/releases/latest";
constexpr const char* kNroAssetName = "BattleShip.nro";
#endif

constexpr size_t kWorkerStackBytes = 512 * 1024;
constexpr const char* kNroExtension = ".nro";

std::atomic<bool> s_updateChecked{false};
std::atomic<bool> s_updateAvailable{false};
std::atomic<bool> s_isDownloading{false};
std::atomic<bool> s_downloadComplete{false};
std::atomic<bool> s_isCheckingForUpdates{false};
std::atomic<bool> s_updateCheckFailed{false};

std::string s_latestVersion;
std::string s_updateStatus;
std::string s_downloadStatus;
std::string s_installedNroPath;
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

bool EndsWith(const std::string& text, const std::string& suffix) {
    if (text.size() < suffix.size()) return false;
    return text.compare(text.size() - suffix.size(), suffix.size(), suffix) == 0;
}

/**
 * Finds the .nro the homebrew loader started, which is argv[0].
 *
 * @param outPath Receives the SD card path of the running .nro.
 * @return False when the loader gave no usable path (for example when launched as an NSO).
 */
bool ResolveRunningNroPath(std::string* outPath) {
    if (__system_argc < 1 || __system_argv == nullptr || __system_argv[0] == nullptr) return false;

    std::string path = __system_argv[0];
    if (!EndsWith(path, kNroExtension)) return false;

    *outPath = path;
    return true;
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

void PublishAvailableRelease(const ReleaseAsset& release) {
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        s_availableRelease = release;
    }
    s_updateAvailable.store(true);
    SetUpdateStatus("");
}

void ApplyReleaseCheckResult(const std::string& responseBody) {
    ReleaseAsset release;
    ReleaseParseStatus status = ParseLatestRelease(responseBody, kNroAssetName, &release);
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
    if (status != ReleaseParseStatus::Ok) {
        port_log("Updater: release %s has no usable %s asset\n", release.tag.c_str(), kNroAssetName);
        FailUpdateCheck("Latest release has no update for this platform.");
        return;
    }
    PublishAvailableRelease(release);
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

void RunDownload(const ReleaseAsset& release, const std::string& nroPath) {
    const std::string partPath = nroPath + ".part";
    HttpOutcome outcome = HttpDownloadToFile(release.downloadUrl, partPath, [](int percent) {
        SetDownloadStatus("Downloading... " + std::to_string(percent) + "%");
    });
    if (!outcome.ok) {
        port_log("Updater: download failed (HTTP %ld): %s\n", outcome.httpStatus, outcome.error.c_str());
        FailDownload("Download failed. " + outcome.error);
        return;
    }

    SetDownloadStatus("Installing update...");
    InstallStatus install = InstallDownloadedNro(partPath, nroPath, release.sizeBytes);
    if (install != InstallStatus::Installed) {
        std::remove(partPath.c_str());
        port_log("Updater: install failed: %s\n", DescribeInstallStatus(install));
        FailDownload(DescribeInstallStatus(install));
        return;
    }

    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        s_installedNroPath = nroPath;
    }
    SetDownloadStatus(envHasNextLoad() ? "Update installed! Restart the game to apply."
                                       : "Update installed! Close and reopen the game to apply.");
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

    std::string nroPath;
    if (!ResolveRunningNroPath(&nroPath)) {
        SetDownloadStatus("Error: Could not locate the running .nro file.");
        return;
    }

    ReleaseAsset release;
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        release = s_availableRelease;
    }
    if (release.downloadUrl.empty()) {
        SetDownloadStatus("Error: No update download available.");
        return;
    }

    s_isDownloading.store(true);
    SetDownloadStatus("Initializing download...");
    bool started = StartWorker([release, nroPath]() { RunDownload(release, nroPath); });
    if (!started) FailDownload("Could not start the download thread.");
}

bool CanRestartAfterUpdate() {
    return s_downloadComplete.load() && envHasNextLoad();
}

void RestartAfterUpdate() {
    if (!CanRestartAfterUpdate()) return;

    std::string nroPath;
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        nroPath = s_installedNroPath;
    }
    std::string quotedArgv = "\"" + nroPath + "\"";
    Result result = envSetNextLoad(nroPath.c_str(), quotedArgv.c_str());
    if (R_FAILED(result)) {
        port_log("Updater: envSetNextLoad failed: 0x%x\n", result);
        SetDownloadStatus("Error: Could not restart. Close and reopen the game.");
        return;
    }
    Ship::Context::GetInstance()->GetWindow()->Close();
}

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
