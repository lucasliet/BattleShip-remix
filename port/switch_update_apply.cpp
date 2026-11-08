#ifdef __SWITCH__

#include "switch_update_apply.h"

#include <switch.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "enhancements/UpdateStaging.h"
#include "port_log.h"

namespace ssb64 {

namespace {

namespace fs = std::filesystem;

using enhancements::kPendingFolderName;
using enhancements::kPendingMarkerName;

constexpr const char* kBackupSuffix = ".old";

struct ApplyError {
    std::string path;
    std::string reason;
};

void RemoveTree(const fs::path& path) {
    std::error_code ignored;
    fs::remove_all(path, ignored);
}

void ShowApplyError(const std::string& appDir, const ApplyError& error) {
    const std::string headline = "Update could not be applied";
    const std::string details =
        headline + "\n\nCould not move " + error.path + ": " + error.reason +
        "\n\nDelete the " + kPendingFolderName + " folder in " + appDir +
        " and try the update again, or reinstall from the release zip.";
    port_log("SSB64: %s\n", details.c_str());
    ErrorApplicationConfig config;
    if (R_SUCCEEDED(errorApplicationCreate(&config, headline.c_str(), details.c_str()))) {
        errorApplicationShow(&config);
    }
}

bool ListStagedFiles(const fs::path& pendingDir, std::vector<fs::path>* files, ApplyError* error) {
    const fs::path marker = pendingDir / kPendingMarkerName;
    std::error_code walkError;
    fs::recursive_directory_iterator entry(pendingDir, walkError);
    const fs::recursive_directory_iterator end;
    for (; !walkError && entry != end; entry.increment(walkError)) {
        std::error_code statError;
        if (!entry->is_regular_file(statError) || entry->path() == marker) continue;
        files->push_back(entry->path());
    }
    if (!walkError) return true;

    *error = {pendingDir.string(), walkError.message()};
    return false;
}

bool CreateParentFolders(const fs::path& target, ApplyError* error) {
    std::error_code createError;
    fs::create_directories(target.parent_path(), createError);
    if (!createError) return true;

    *error = {target.parent_path().string(), createError.message()};
    return false;
}

bool BackUpExistingTarget(const fs::path& target, const fs::path& backup, ApplyError* error) {
    std::error_code existsError;
    if (!fs::exists(target, existsError)) return true;

    std::remove(backup.c_str());
    if (std::rename(target.c_str(), backup.c_str()) == 0) return true;

    *error = {target.string(), std::strerror(errno)};
    return false;
}

// libnx rename() does not overwrite an existing target, so the old file is moved aside first.
bool ReplaceFile(const fs::path& staged, const fs::path& target, ApplyError* error) {
    const fs::path backup = target.string() + kBackupSuffix;
    if (!BackUpExistingTarget(target, backup, error)) return false;

    if (std::rename(staged.c_str(), target.c_str()) != 0) {
        *error = {target.string(), std::strerror(errno)};
        std::rename(backup.c_str(), target.c_str());
        return false;
    }
    std::remove(backup.c_str());
    return true;
}

bool MoveStagedFile(const fs::path& appDir, const fs::path& pendingDir, const fs::path& staged,
                    ApplyError* error) {
    const fs::path target = appDir / staged.lexically_relative(pendingDir);
    if (!CreateParentFolders(target, error)) return false;
    return ReplaceFile(staged, target, error);
}

bool MoveStagedFiles(const fs::path& appDir, const fs::path& pendingDir, size_t* movedCount,
                     ApplyError* error) {
    std::vector<fs::path> files;
    if (!ListStagedFiles(pendingDir, &files, error)) return false;

    for (const fs::path& staged : files) {
        if (!MoveStagedFile(appDir, pendingDir, staged, error)) return false;
        ++*movedCount;
    }
    return true;
}

} // namespace

bool SwitchApplyPendingUpdate(const std::string& appDir) {
    const fs::path pendingDir = fs::path(appDir) / kPendingFolderName;
    std::error_code existsError;
    if (!fs::exists(pendingDir, existsError)) return true;

    if (!fs::exists(pendingDir / kPendingMarkerName, existsError)) {
        port_log("SSB64: discarding an incomplete staged update in %s\n", pendingDir.c_str());
        RemoveTree(pendingDir);
        return true;
    }

    size_t movedCount = 0;
    ApplyError error;
    if (!MoveStagedFiles(appDir, pendingDir, &movedCount, &error)) {
        ShowApplyError(appDir, error);
        return false;
    }

    RemoveTree(pendingDir);
    port_log("SSB64: applied the staged update, %zu files moved into %s\n", movedCount, appDir.c_str());
    return true;
}

} // namespace ssb64

#endif
