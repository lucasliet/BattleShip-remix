#include "enhancements.h"

#include "port_log.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include <nlohmann/json.hpp>

#include <ship/Context.h>
#include <ship/window/Window.h>

#if defined(__linux__)
#include <sys/stat.h>
#include <unistd.h>
#endif

#if defined(__APPLE__)
#include "UpdateZipInstall.h"

#include <mach-o/dyld.h>
#include <unistd.h>

#include <climits>
#include <filesystem>
#include <system_error>
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

namespace ssb64 {
namespace enhancements {

// We use atomics for the state flags so the UI thread doesn't have to lock a mutex 60 times a second!
static std::atomic<bool> s_updateChecked{false};
static std::atomic<bool> s_updateAvailable{false};
static std::atomic<bool> s_isDownloading{false};
static std::atomic<bool> s_downloadComplete{false};
static std::atomic<bool> s_isCheckingForUpdates{false};
static std::atomic<bool> s_updateCheckFailed{false};

static std::string s_latestVersion = "";
static std::string s_downloadUrl = "";
static std::string s_updateStatus = "";
static std::string s_downloadStatus = "";
static std::string s_restartPath = "";
static std::mutex s_stringMutex; // Only locks when reading/writing the actual text

namespace {

#ifndef BATTLESHIP_CURRENT_VERSION
#define BATTLESHIP_CURRENT_VERSION "v1.0.0"
#endif

#if defined(REGION_REMIX)
// The Remix build is released by the BattleShip-remix fork, with its own asset names.
constexpr const char* kLatestReleaseApi =
    "https://api.github.com/repos/lucasliet/BattleShip-remix/releases/latest";
constexpr const char* kReleasePageUrl =
    "https://github.com/lucasliet/BattleShip-remix/releases/latest";

#if defined(__linux__)
constexpr const char* kPlatformAssetName = "BattleShip-Remix-x86_64.AppImage";
#elif defined(_WIN32)
constexpr const char* kPlatformAssetName = "BattleShip-Remix-windows.zip";
#elif defined(__APPLE__)
constexpr const char* kPlatformAssetName = SSB64_APP_NAME "-macos.zip";
#else
constexpr const char* kPlatformAssetName = "";
#endif

#else
constexpr const char* kLatestReleaseApi =
    "https://api.github.com/repos/JRickey/BattleShip/releases/latest";
constexpr const char* kReleasePageUrl =
    "https://github.com/JRickey/BattleShip/releases/latest";

#if defined(__linux__)
constexpr const char* kPlatformAssetName = "BattleShip-x86_64.AppImage";
#elif defined(_WIN32)
constexpr const char* kPlatformAssetName = "BattleShip-windows.zip";
#elif defined(__APPLE__)
constexpr const char* kPlatformAssetName = SSB64_APP_NAME "-macos.zip";
#else
constexpr const char* kPlatformAssetName = "";
#endif
#endif

#if !defined(_WIN32)
std::string ShellSingleQuote(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') {
            out += "'\\''";
        } else {
            out += c;
        }
    }
    out += "'";
    return out;
}
#endif

void SetUpdateStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(s_stringMutex);
    s_updateStatus = status;
}

void SetDownloadStatus(const std::string& status) {
    std::lock_guard<std::mutex> lock(s_stringMutex);
    s_downloadStatus = status;
}

#if defined(_WIN32)
// Run a child process with stdout/stderr redirected and no console window.
// Required on Windows because _popen() / system() shell through cmd.exe;
// when called from a /SUBSYSTEM:WINDOWS binary, that creates a visible console.
int RunCaptureNoWindow(std::string cmd,
                       const std::function<void(const std::string&)>& onLine) {
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) return -1;
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = nullptr;

    PROCESS_INFORMATION pi{};
    BOOL ok = CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, TRUE,
                             CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(writePipe);
    if (!ok) {
        CloseHandle(readPipe);
        return -1;
    }

    std::string line;
    char buf[512];
    DWORD got = 0;
    while (ReadFile(readPipe, buf, sizeof(buf), &got, nullptr) && got > 0) {
        for (DWORD i = 0; i < got; ++i) {
            char c = buf[i];
            if (c == '\r' || c == '\n') {
                if (!line.empty()) {
                    onLine(line);
                    line.clear();
                }
            } else {
                line += c;
            }
        }
    }
    if (!line.empty()) onLine(line);
    CloseHandle(readPipe);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return static_cast<int>(code);
}
#endif

int RunCapture(std::string cmd, const std::function<void(const std::string&)>& onLine) {
#if defined(_WIN32)
    return RunCaptureNoWindow(std::move(cmd), onLine);
#else
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return -1;

    int c;
    std::string currentLine;
    while ((c = fgetc(pipe)) != EOF) {
        if (c == '\r' || c == '\n') {
            if (!currentLine.empty()) onLine(currentLine);
            currentLine.clear();
        } else {
            currentLine += static_cast<char>(c);
        }
    }
    if (!currentLine.empty()) onLine(currentLine);
    return pclose(pipe);
#endif
}

std::string CurlCommandPrefix() {
#if defined(__linux__)
    // AppRun prepends bundled AppImage libs for the game itself. Those libs
    // poison distro tools like curl/xdg-open, so strip loader overrides here.
    return "env -u LD_LIBRARY_PATH -u LD_PRELOAD curl";
#else
    return "curl";
#endif
}

std::string BuildCheckCommand() {
#if defined(_WIN32)
    return std::string("curl -fLsS -m 10 -H \"User-Agent: BattleShip-Updater\" ") +
           "\"" + kLatestReleaseApi + "\"";
#else
    return CurlCommandPrefix() +
           " -fLsS -m 10 -H " + ShellSingleQuote("User-Agent: BattleShip-Updater") +
           " " + ShellSingleQuote(kLatestReleaseApi) + " 2>&1";
#endif
}

std::string BuildDownloadCommand(const std::string& destPath, const std::string& url) {
#if defined(_WIN32)
    return "curl -fL -# -o \"" + destPath + "\" \"" + url + "\"";
#else
    return CurlCommandPrefix() + " -fL -# -o " + ShellSingleQuote(destPath) +
           " " + ShellSingleQuote(url) + " 2>&1";
#endif
}

// The URL is spliced into a curl command line, so reject anything that
// could escape the quoting or smuggle extra arguments. Release asset URLs
// are plain https://github.com/... paths — no quotes, spaces, or control
// characters.
bool IsSafeDownloadUrl(const std::string& url) {
    if (url.rfind("https://", 0) != 0) return false;
    for (char c : url) {
        if (c == '"' || c == '\'' || c == '\\' || c <= 0x20 || c == 0x7F) return false;
    }
    return true;
}

bool FindPlatformAssetUrl(const nlohmann::json& release, std::string* outUrl) {
    if (!release.contains("assets") || !release["assets"].is_array()) return false;

    for (const auto& asset : release["assets"]) {
        if (!asset.contains("name") || !asset.contains("browser_download_url")) continue;
        if (!asset["name"].is_string() || !asset["browser_download_url"].is_string()) continue;
        if (asset["name"].get<std::string>() == kPlatformAssetName) {
            std::string url = asset["browser_download_url"].get<std::string>();
            if (!IsSafeDownloadUrl(url)) {
                port_log("Updater: rejecting unsafe download URL\n");
                return false;
            }
            *outUrl = url;
            return true;
        }
    }
    return false;
}

void ResetDownloadStateForNewCheck() {
    s_downloadComplete.store(false);
    SetDownloadStatus("");
}

#if defined(__APPLE__)
std::string BundleStagingPath(const std::string& bundle) {
    return bundle + ".new";
}

std::string BundleBackupPath(const std::string& bundle) {
    return bundle + ".old";
}

void RemoveTree(const std::string& path) {
    std::error_code ignored;
    std::filesystem::remove_all(path, ignored);
}

/**
 * Finds the .app bundle that holds the running executable.
 *
 * @param outBundle Receives the real path of "<name>.app".
 * @return false when the executable is not at "<name>.app/Contents/MacOS/<exe>".
 */
bool ResolveAppBundlePath(std::string* outBundle) {
    char rawPath[PATH_MAX];
    uint32_t rawSize = sizeof(rawPath);
    if (_NSGetExecutablePath(rawPath, &rawSize) != 0) return false;

    char realPath[PATH_MAX];
    if (realpath(rawPath, realPath) == nullptr) return false;

    const std::filesystem::path macOsDir = std::filesystem::path(realPath).parent_path();
    const std::filesystem::path contentsDir = macOsDir.parent_path();
    const std::filesystem::path bundle = contentsDir.parent_path();
    if (macOsDir.filename() != "MacOS" || contentsDir.filename() != "Contents") return false;
    if (bundle.extension() != ".app") return false;

    *outBundle = bundle.string();
    return true;
}

bool ParentIsWritable(const std::string& bundle) {
    const std::string parent = std::filesystem::path(bundle).parent_path().string();
    return access(parent.c_str(), W_OK) == 0;
}

/**
 * Resolves the installed bundle and the download target next to it, reporting why when the
 * app cannot be updated in place.
 *
 * @param outBundle Receives the installed bundle path.
 * @param outZipPath Receives the hidden download path in the folder holding the bundle.
 * @return false after setting the download status to an error.
 */
bool PrepareBundleUpdate(std::string* outBundle, std::string* outZipPath) {
    if (!ResolveAppBundlePath(outBundle)) {
        SetDownloadStatus("Error: Not running from an installed .app bundle.");
        return false;
    }
    if (!ParentIsWritable(*outBundle)) {
        SetDownloadStatus("Error: The folder holding the app is not writable; install the update from the release page.");
        return false;
    }
    const std::filesystem::path parent = std::filesystem::path(*outBundle).parent_path();
    *outZipPath = (parent / ("." SSB64_APP_NAME "-update.zip")).string();
    return true;
}

bool IsStagedBundleComplete(const std::string& stagedBundle) {
    const std::string executable = stagedBundle + "/Contents/MacOS/" SSB64_APP_NAME;
    return access(executable.c_str(), X_OK) == 0;
}

void PublishStagedBundle(const std::string& bundle) {
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        s_restartPath = bundle;
        s_downloadStatus = "Update ready! Restart the game to apply.";
    }
    s_downloadComplete.store(true);
}

/**
 * Unpacks the downloaded zip into "<bundle>.new" and marks the update ready to apply on restart.
 * The zip is removed in every case.
 *
 * @param zipPath Downloaded release zip.
 * @param bundle Installed bundle the staged copy sits next to.
 */
void StageBundleUpdate(const std::string& zipPath, const std::string& bundle) {
    const std::string staging = BundleStagingPath(bundle);
    RemoveTree(staging);

    const ZipExtractOptions options{SSB64_APP_NAME ".app/", ZipFileMode::FromArchive, true};
    const ZipInstallStatus status = ExtractZipTo(zipPath, options, staging);
    std::remove(zipPath.c_str());
    if (status != ZipInstallStatus::Installed) {
        SetDownloadStatus(std::string("Error: ") + DescribeZipInstallStatus(status));
        return;
    }
    if (!IsStagedBundleComplete(staging)) {
        RemoveTree(staging);
        SetDownloadStatus("Error: The downloaded update is incomplete.");
        return;
    }
    PublishStagedBundle(bundle);
}

/** Deletes the folder the previous update left behind; it was still running when it was renamed. */
void RemoveStaleBundleBackup() {
    std::string bundle;
    if (!ResolveAppBundlePath(&bundle)) return;
    RemoveTree(BundleBackupPath(bundle));
}

/**
 * Replaces the installed bundle with "<bundle>.new" and starts it. The running process keeps
 * executing from the renamed "<bundle>.old" folder.
 *
 * @param bundle Installed bundle path.
 */
void SwapBundleAndRelaunch(const std::string& bundle) {
    const std::string backup = BundleBackupPath(bundle);
    if (std::rename(bundle.c_str(), backup.c_str()) != 0) {
        SetDownloadStatus("Error: Could not replace the app. Close and reopen the game.");
        return;
    }
    if (std::rename(BundleStagingPath(bundle).c_str(), bundle.c_str()) != 0) {
        std::rename(backup.c_str(), bundle.c_str());
        SetDownloadStatus("Error: Could not replace the app. Close and reopen the game.");
        return;
    }
    system(("open -n " + ShellSingleQuote(bundle)).c_str());
    Ship::Context::GetInstance()->GetWindow()->Close();
}
#endif

} // namespace

void CheckForUpdatesAsync(bool force) {
    // Check our atomic flags (no locks required)
    if (s_isCheckingForUpdates.load() || s_isDownloading.load()) return;
    if (!force && s_updateChecked.load()) return;

    s_updateChecked.store(true);
    s_isCheckingForUpdates.store(true);
    s_updateCheckFailed.store(false);
    s_updateAvailable.store(false);
    ResetDownloadStateForNewCheck();
    SetUpdateStatus("");

    std::thread([]() {
#if defined(__APPLE__)
        RemoveStaleBundleBackup();
#endif
        std::string response;
        int exitCode = RunCapture(BuildCheckCommand(), [&](const std::string& line) {
            response += line;
        });
        if (exitCode != 0) {
            s_updateCheckFailed.store(true);
            SetUpdateStatus("Unable to check for updates.");
            s_isCheckingForUpdates.store(false);
            return;
        }

        if (!response.empty()) {
            try {
                auto release = nlohmann::json::parse(response);
                if (!release.contains("tag_name") || !release["tag_name"].is_string()) {
                    throw std::runtime_error("latest release missing tag_name");
                }

                std::string latestTag = release["tag_name"].get<std::string>();
                {
                    std::lock_guard<std::mutex> lock(s_stringMutex);
                    s_latestVersion = latestTag;
                    s_downloadUrl.clear();
                }

                if (latestTag != BATTLESHIP_CURRENT_VERSION) {
                    std::string assetUrl;
                    if (!FindPlatformAssetUrl(release, &assetUrl)) {
                        s_updateCheckFailed.store(true);
                        SetUpdateStatus("Latest release has no update for this platform.");
                    } else {
                        {
                            std::lock_guard<std::mutex> lock(s_stringMutex);
                            s_downloadUrl = assetUrl;
                        }
                        s_updateAvailable.store(true);
                        SetUpdateStatus("");
                    }
                } else {
                    SetUpdateStatus("Up to date.");
                }
            } catch (...) {
                s_updateCheckFailed.store(true);
                SetUpdateStatus("Unable to read update information.");
            }
        } else {
            s_updateCheckFailed.store(true);
            SetUpdateStatus("Unable to check for updates.");
        }

        s_isCheckingForUpdates.store(false);
    }).detach();
}

void StartGameUpdate() {
    if (s_isDownloading.load()) return;
    s_isDownloading.store(true);

    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        if (s_downloadUrl.empty()) {
            s_downloadStatus = "Error: No update download available.";
            s_isDownloading.store(false);
            return;
        }
        s_downloadStatus = "Initializing download...";
    }

    std::thread([]() {
        std::string url;
        {
            std::lock_guard<std::mutex> lock(s_stringMutex);
            url = s_downloadUrl;
        }

        #if defined(__linux__)
        const char* appImagePath = getenv("APPIMAGE");
        if (!appImagePath) {
            {
                std::lock_guard<std::mutex> lock(s_stringMutex);
                s_downloadStatus = "Error: Not running as AppImage";
            }
            s_isDownloading.store(false);
            return;
        }

        std::string tempPath = std::string(appImagePath) + ".part";
        std::string cmd = BuildDownloadCommand(tempPath, url);

        #elif defined(_WIN32)
        // Anchor everything to the install directory — CWD can be anywhere
        // (Start Menu launches use the user profile), and the .bat extracts
        // over the game files, so it must run where BattleShip.exe lives.
        std::string exeDir;
        {
            char exePath[MAX_PATH];
            DWORD n = GetModuleFileNameA(nullptr, exePath, MAX_PATH);
            if (n > 0 && n < MAX_PATH) {
                std::string p(exePath, n);
                size_t slash = p.find_last_of("\\/");
                if (slash != std::string::npos) {
                    exeDir = p.substr(0, slash);
                }
            }
        }
        if (exeDir.empty()) {
            {
                std::lock_guard<std::mutex> lock(s_stringMutex);
                s_downloadStatus = "Error: Could not locate install directory.";
            }
            s_isDownloading.store(false);
            return;
        }
        std::string tempZip = exeDir + "\\update_temp.zip";
        std::string cmd = BuildDownloadCommand(tempZip, url);

        #elif defined(__APPLE__)
        std::string bundle;
        std::string zipPath;
        if (!PrepareBundleUpdate(&bundle, &zipPath)) {
            s_isDownloading.store(false);
            return;
        }
        std::string cmd = BuildDownloadCommand(zipPath, url);
        #endif

        auto onProgressLine = [](const std::string& line) {
            if (line.find('%') == std::string::npos) return;
            std::string pctStr;
            for (char ch : line) {
                if ((ch >= '0' && ch <= '9') || ch == '.') {
                    pctStr += ch;
                }
            }
            if (!pctStr.empty()) {
                SetDownloadStatus("Downloading... " + pctStr + "%");
            }
        };

        int exitCode = RunCapture(cmd, onProgressLine);

        if (exitCode == 0) {
            #if defined(__linux__)
            chmod(tempPath.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
            if (std::rename(tempPath.c_str(), appImagePath) == 0) {
                {
                    std::lock_guard<std::mutex> lock(s_stringMutex);
                    s_downloadStatus = "Update complete! Please restart the game.";
                    s_restartPath = appImagePath;
                }
                s_downloadComplete.store(true);
            } else {
                std::lock_guard<std::mutex> lock(s_stringMutex);
                s_downloadStatus = "Error: Failed to replace AppImage.";
            }
            #elif defined(_WIN32)
            std::string batPath = exeDir + "\\update_game.bat";
            FILE* bat = fopen(batPath.c_str(), "w");
            if (bat) {
                fprintf(bat, "@echo off\n");
                fprintf(bat, "cd /d \"%s\"\n", exeDir.c_str());
                fprintf(bat, "echo Update downloaded! Waiting for BattleShip to close before applying...\n");
                fprintf(bat, ":wait\n");
                fprintf(bat, "tasklist /FI \"IMAGENAME eq %s.exe\" 2>NUL | find /I /N \"%s.exe\">NUL\n",
                        SSB64_APP_NAME, SSB64_APP_NAME);
                fprintf(bat, "if \"%%ERRORLEVEL%%\"==\"0\" (\n");
                fprintf(bat, "    timeout /t 1 /nobreak > NUL\n");
                fprintf(bat, "    goto wait\n");
                fprintf(bat, ")\n");
                fprintf(bat, "echo Installing update...\n");
                fprintf(bat, "tar -xf update_temp.zip\n");
                fprintf(bat, "del update_temp.zip\n");
                fprintf(bat, "start %s.exe\n", SSB64_APP_NAME);
                fprintf(bat, "(goto) 2>nul & del \"%%~f0\"\n");
                fclose(bat);

                {
                    std::lock_guard<std::mutex> lock(s_stringMutex);
                    s_downloadStatus = "Update ready! Close the game to apply.";
                }
                ShellExecuteA(nullptr, "open", batPath.c_str(), nullptr, exeDir.c_str(), SW_SHOWNORMAL);
                s_downloadComplete.store(true);
            } else {
                std::lock_guard<std::mutex> lock(s_stringMutex);
                s_downloadStatus = "Error: Failed to create updater script.";
            }
            #elif defined(__APPLE__)
            StageBundleUpdate(zipPath, bundle);
            #endif
        } else {
            {
                std::lock_guard<std::mutex> lock(s_stringMutex);
                s_downloadStatus = "Error: Download failed.";
            }
            #if defined(__linux__)
            std::remove(tempPath.c_str());
            #elif defined(_WIN32)
            std::remove(tempZip.c_str());
            #elif defined(__APPLE__)
            std::remove(zipPath.c_str());
            #endif
        }

        s_isDownloading.store(false);
    }).detach();
}

bool CanRestartAfterUpdate() {
    if (!s_downloadComplete.load()) return false;
#if defined(_WIN32)
    return true;
#elif defined(__linux__) || defined(__APPLE__)
    std::lock_guard<std::mutex> lock(s_stringMutex);
    return !s_restartPath.empty();
#else
    return false;
#endif
}

void RestartAfterUpdate() {
    if (!CanRestartAfterUpdate()) return;
#if defined(__linux__)
    std::string path;
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        path = s_restartPath;
    }
    execl(path.c_str(), path.c_str(), static_cast<char*>(nullptr));
    SetDownloadStatus("Error: Could not restart. Close and reopen the game.");
#elif defined(_WIN32)
    // update_game.bat waits for this process to exit, applies the zip and starts the new exe.
    Ship::Context::GetInstance()->GetWindow()->Close();
#elif defined(__APPLE__)
    std::string bundle;
    {
        std::lock_guard<std::mutex> lock(s_stringMutex);
        bundle = s_restartPath;
    }
    SwapBundleAndRelaunch(bundle);
#endif
}

// OS-aware URL opener for Windows and Mac fallback
void OpenReleasePage() {
    const char* url = kReleasePageUrl;

    #if defined(_WIN32)
    ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
    #elif defined(__APPLE__)
    std::string cmd = std::string("open ") + url;
    system(cmd.c_str());
    #else
    std::string cmd = std::string("env -u LD_LIBRARY_PATH -u LD_PRELOAD xdg-open ") +
                      ShellSingleQuote(url) + " &";
    system(cmd.c_str());
    #endif
}

// Atomic reads - no locks required for the UI thread!
bool IsCheckingForUpdates() { return s_isCheckingForUpdates.load(); }
bool IsUpdateAvailable() { return s_updateAvailable.load(); }
bool DidUpdateCheckFail() { return s_updateCheckFailed.load(); }
bool IsDownloading() { return s_isDownloading.load(); }
bool IsDownloadComplete() { return s_downloadComplete.load(); }

// String getters still need the lock, but they are incredibly fast now
std::string GetUpdateStatus() { std::lock_guard<std::mutex> lock(s_stringMutex); return s_updateStatus; }
std::string GetDownloadStatus() { std::lock_guard<std::mutex> lock(s_stringMutex); return s_downloadStatus; }
std::string GetLatestVersion() { std::lock_guard<std::mutex> lock(s_stringMutex); return s_latestVersion; }

} // namespace enhancements
} // namespace ssb64
