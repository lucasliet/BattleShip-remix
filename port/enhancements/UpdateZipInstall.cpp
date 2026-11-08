#include "UpdateZipInstall.h"

#include <cerrno>
#include <cstdio>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <unistd.h>
#include <zip.h>

namespace ssb64 {
namespace enhancements {

namespace {

constexpr size_t kCopyChunkBytes = 64 * 1024;
constexpr size_t kMaxSymlinkTargetBytes = 4096;
constexpr mode_t kPermissionBitsMask = 07777;
// The PS5 refuses to launch a title whose eboot.bin lacks the execute bit, and fopen creates 0666
// files; the files get the 0777 mode an FTP install gives them.
constexpr mode_t kInstalledFileMode = 0777;
constexpr const char* kStagingSuffix = ".part";

enum class SymlinkPolicy {
    TreatAsFile,
    Reject,
    Create,
};

struct ExtractRules {
    ZipFileMode fileMode;
    SymlinkPolicy symlinks;
};

struct PlannedFile {
    zip_uint64_t index;
    std::string target;
};

bool IsSafeRelativePath(const std::string& path) {
    if (path.empty() || path[0] == '/' || path.find('\\') != std::string::npos) return false;
    size_t start = 0;
    while (start <= path.size()) {
        size_t end = path.find('/', start);
        if (end == std::string::npos) end = path.size();
        const std::string part = path.substr(start, end - start);
        if (part.empty() || part == "." || part == "..") return false;
        start = end + 1;
    }
    return true;
}

/**
 * Creates every parent folder of a file below a base folder.
 * @param baseDir the base folder, which exists
 * @param relative the file's path relative to it
 */
void CreateParentFolders(const std::string& baseDir, const std::string& relative) {
    size_t slash = relative.find('/');
    while (slash != std::string::npos) {
        mkdir((baseDir + "/" + relative.substr(0, slash)).c_str(), 0777);
        slash = relative.find('/', slash + 1);
    }
}

void CreateAllParentFolders(const std::vector<PlannedFile>& plan, const std::string& baseDir) {
    for (const PlannedFile& file : plan) {
        CreateParentFolders(baseDir, file.target);
    }
}

/**
 * Lists the zip's files under rootPrefix with their relative paths.
 * @return NoEntries or UnsafeEntry on a bad zip, Installed when the plan is complete
 */
ZipInstallStatus PlanFiles(zip_t* archive, const std::string& rootPrefix, std::vector<PlannedFile>* plan) {
    const zip_int64_t count = zip_get_num_entries(archive, 0);
    for (zip_int64_t index = 0; index < count; index++) {
        const char* rawName = zip_get_name(archive, static_cast<zip_uint64_t>(index), 0);
        if (rawName == nullptr) return ZipInstallStatus::UnsafeEntry;
        const std::string name = rawName;
        if (name.rfind(rootPrefix, 0) != 0 || name.back() == '/') continue;
        const std::string relative = name.substr(rootPrefix.size());
        if (!IsSafeRelativePath(relative)) return ZipInstallStatus::UnsafeEntry;
        plan->push_back(PlannedFile{ static_cast<zip_uint64_t>(index), relative });
    }
    if (plan->empty()) return ZipInstallStatus::NoEntries;
    return ZipInstallStatus::Installed;
}

/**
 * Reads the unix mode a zip entry was stored with.
 * @return the st_mode value, or 0 when the entry carries no unix attributes
 */
mode_t ReadArchiveMode(zip_t* archive, zip_uint64_t index) {
    zip_uint8_t operatingSystem = 0;
    zip_uint32_t attributes = 0;
    if (zip_file_get_external_attributes(archive, index, 0, &operatingSystem, &attributes) != 0) return 0;
    if (operatingSystem != ZIP_OPSYS_UNIX) return 0;
    return static_cast<mode_t>(attributes >> 16);
}

/**
 * Reads a whole zip entry into memory.
 * @return false when the entry cannot be read or is longer than maxBytes
 */
bool ReadEntryContent(zip_t* archive, zip_uint64_t index, size_t maxBytes, std::string* content) {
    zip_file_t* entry = zip_fopen_index(archive, index, 0);
    if (entry == nullptr) return false;
    std::vector<char> chunk(maxBytes + 1);
    size_t total = 0;
    while (total < chunk.size()) {
        const zip_int64_t read = zip_fread(entry, chunk.data() + total, chunk.size() - total);
        if (read < 0) {
            zip_fclose(entry);
            return false;
        }
        if (read == 0) break;
        total += static_cast<size_t>(read);
    }
    zip_fclose(entry);
    if (total > maxBytes) return false;
    content->assign(chunk.data(), total);
    return true;
}

bool CopyEntryToFile(zip_t* archive, zip_uint64_t index, const std::string& destination) {
    zip_file_t* entry = zip_fopen_index(archive, index, 0);
    if (entry == nullptr) return false;
    FILE* output = std::fopen(destination.c_str(), "wb");
    if (output == nullptr) {
        zip_fclose(entry);
        return false;
    }
    std::vector<char> chunk(kCopyChunkBytes);
    bool ok = true;
    for (;;) {
        const zip_int64_t read = zip_fread(entry, chunk.data(), chunk.size());
        if (read == 0) break;
        if (read < 0 || std::fwrite(chunk.data(), 1, static_cast<size_t>(read), output) != static_cast<size_t>(read)) {
            ok = false;
            break;
        }
    }
    zip_fclose(entry);
    return std::fclose(output) == 0 && ok;
}

/**
 * Applies the permissions the extraction mode asks for.
 * @return false only when a chmod was requested and failed
 */
bool ApplyFileMode(const std::string& path, ZipFileMode fileMode, mode_t archiveMode) {
    switch (fileMode) {
        case ZipFileMode::Executable:
            return chmod(path.c_str(), kInstalledFileMode) == 0;
        case ZipFileMode::FromArchive: {
            const mode_t permissions = archiveMode & kPermissionBitsMask;
            return permissions == 0 || chmod(path.c_str(), permissions) == 0;
        }
        case ZipFileMode::Ignore:
            return true;
    }
    return true;
}

ZipInstallStatus CreateSymlinkEntry(zip_t* archive, zip_uint64_t index, const std::string& destination,
                                    SymlinkPolicy policy) {
    if (policy != SymlinkPolicy::Create) return ZipInstallStatus::UnsafeEntry;
    std::string target;
    if (!ReadEntryContent(archive, index, kMaxSymlinkTargetBytes, &target)) return ZipInstallStatus::ExtractFailed;
    if (!IsSafeRelativePath(target)) return ZipInstallStatus::UnsafeEntry;
    unlink(destination.c_str());
    if (symlink(target.c_str(), destination.c_str()) != 0) return ZipInstallStatus::ExtractFailed;
    return ZipInstallStatus::Installed;
}

ZipInstallStatus ExtractPlannedFile(zip_t* archive, zip_uint64_t index, const std::string& destination,
                                    const ExtractRules& rules) {
    const mode_t archiveMode = ReadArchiveMode(archive, index);
    if (rules.symlinks != SymlinkPolicy::TreatAsFile && S_ISLNK(archiveMode)) {
        return CreateSymlinkEntry(archive, index, destination, rules.symlinks);
    }
    if (!CopyEntryToFile(archive, index, destination)) return ZipInstallStatus::ExtractFailed;
    if (!ApplyFileMode(destination, rules.fileMode, archiveMode)) return ZipInstallStatus::ExtractFailed;
    return ZipInstallStatus::Installed;
}

/**
 * Extracts every planned file to "<baseDir>/<target><suffix>", stopping at the first failure.
 * @return Installed when all files extracted, otherwise the first failing status
 */
ZipInstallStatus ExtractAll(zip_t* archive, const std::vector<PlannedFile>& plan, const std::string& baseDir,
                            const std::string& suffix, const ExtractRules& rules) {
    for (const PlannedFile& file : plan) {
        const std::string destination = baseDir + "/" + file.target + suffix;
        const ZipInstallStatus status = ExtractPlannedFile(archive, file.index, destination, rules);
        if (status != ZipInstallStatus::Installed) return status;
    }
    return ZipInstallStatus::Installed;
}

void RemoveParts(const std::vector<PlannedFile>& plan, const std::string& titleDir) {
    for (const PlannedFile& file : plan) {
        std::remove((titleDir + "/" + file.target + kStagingSuffix).c_str());
    }
}

/**
 * Creates the destination folder when it is missing.
 * @param created set to true only when this call created the folder
 */
ZipInstallStatus PrepareDestination(const std::string& destinationDir, bool* created) {
    if (mkdir(destinationDir.c_str(), 0777) == 0) {
        *created = true;
        return ZipInstallStatus::Installed;
    }
    return errno == EEXIST ? ZipInstallStatus::Installed : ZipInstallStatus::ExtractFailed;
}

/**
 * Removes the files and the now-empty folders a failed extraction left in the destination.
 */
void RemoveExtracted(const std::vector<PlannedFile>& plan, const std::string& destinationDir,
                     bool removeDestination) {
    for (const PlannedFile& file : plan) {
        unlink((destinationDir + "/" + file.target).c_str());
    }
    for (const PlannedFile& file : plan) {
        size_t slash = file.target.rfind('/');
        while (slash != std::string::npos) {
            rmdir((destinationDir + "/" + file.target.substr(0, slash)).c_str());
            slash = slash == 0 ? std::string::npos : file.target.rfind('/', slash - 1);
        }
    }
    if (removeDestination) rmdir(destinationDir.c_str());
}

} // namespace

const char* DescribeZipInstallStatus(ZipInstallStatus status) {
    switch (status) {
        case ZipInstallStatus::Installed:
            return "Update installed.";
        case ZipInstallStatus::OpenFailed:
            return "The downloaded update is not a readable zip.";
        case ZipInstallStatus::NoEntries:
            return "The downloaded update holds no game files.";
        case ZipInstallStatus::UnsafeEntry:
            return "The downloaded update has an unsafe file path.";
        case ZipInstallStatus::ExtractFailed:
            return "Could not unpack the update; the installed game was kept.";
        case ZipInstallStatus::ReplaceFailed:
            return "Could not replace some game files; reinstall the game from the release zip.";
    }
    return "Unknown install error.";
}

ZipInstallStatus InstallTitleZip(const std::string& zipPath, const std::string& rootPrefix,
                                 const std::string& titleDir) {
    int error = 0;
    zip_t* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &error);
    if (archive == nullptr) return ZipInstallStatus::OpenFailed;

    const ExtractRules rules{ ZipFileMode::Executable, SymlinkPolicy::TreatAsFile };
    std::vector<PlannedFile> plan;
    ZipInstallStatus status = PlanFiles(archive, rootPrefix, &plan);
    if (status == ZipInstallStatus::Installed) {
        CreateAllParentFolders(plan, titleDir);
        status = ExtractAll(archive, plan, titleDir, kStagingSuffix, rules);
    }
    zip_close(archive);
    if (status != ZipInstallStatus::Installed) {
        RemoveParts(plan, titleDir);
        return status;
    }

    for (const PlannedFile& file : plan) {
        const std::string target = titleDir + "/" + file.target;
        if (std::rename((target + kStagingSuffix).c_str(), target.c_str()) != 0) {
            status = ZipInstallStatus::ReplaceFailed;
        }
    }
    RemoveParts(plan, titleDir);
    return status;
}

ZipInstallStatus ExtractZipTo(const std::string& zipPath, const ZipExtractOptions& options,
                              const std::string& destinationDir) {
    int error = 0;
    zip_t* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &error);
    if (archive == nullptr) return ZipInstallStatus::OpenFailed;

    const ExtractRules rules{ options.fileMode,
                              options.allowSymlinks ? SymlinkPolicy::Create : SymlinkPolicy::Reject };
    std::vector<PlannedFile> plan;
    bool createdDestination = false;
    bool extractionStarted = false;
    ZipInstallStatus status = PlanFiles(archive, options.rootPrefix, &plan);
    if (status == ZipInstallStatus::Installed) status = PrepareDestination(destinationDir, &createdDestination);
    if (status == ZipInstallStatus::Installed) {
        extractionStarted = true;
        CreateAllParentFolders(plan, destinationDir);
        status = ExtractAll(archive, plan, destinationDir, "", rules);
    }
    zip_close(archive);
    if (status != ZipInstallStatus::Installed && extractionStarted) {
        RemoveExtracted(plan, destinationDir, createdDestination);
    }
    return status;
}

} // namespace enhancements
} // namespace ssb64
