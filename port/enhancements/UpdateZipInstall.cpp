#include "UpdateZipInstall.h"

#include <cstdio>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <zip.h>

namespace ssb64 {
namespace enhancements {

namespace {

constexpr size_t kCopyChunkBytes = 64 * 1024;

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
 * Creates every parent folder of a file below the title folder.
 * @param titleDir the title folder, which exists
 * @param relative the file's path relative to it
 */
void CreateParentFolders(const std::string& titleDir, const std::string& relative) {
    size_t slash = relative.find('/');
    while (slash != std::string::npos) {
        mkdir((titleDir + "/" + relative.substr(0, slash)).c_str(), 0777);
        slash = relative.find('/', slash + 1);
    }
}

/**
 * Lists the zip's files under rootPrefix with their install paths.
 * @return NoEntries or UnsafeEntry on a bad zip, Installed when the plan is complete
 */
ZipInstallStatus PlanFiles(zip_t* archive, const std::string& rootPrefix, const std::string& titleDir,
                           std::vector<PlannedFile>* plan) {
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
    for (const PlannedFile& file : *plan) {
        CreateParentFolders(titleDir, file.target);
    }
    return ZipInstallStatus::Installed;
}

bool ExtractEntry(zip_t* archive, zip_uint64_t index, const std::string& destination) {
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

void RemoveParts(const std::vector<PlannedFile>& plan, const std::string& titleDir) {
    for (const PlannedFile& file : plan) {
        std::remove((titleDir + "/" + file.target + ".part").c_str());
    }
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

    std::vector<PlannedFile> plan;
    ZipInstallStatus status = PlanFiles(archive, rootPrefix, titleDir, &plan);
    for (size_t file = 0; status == ZipInstallStatus::Installed && file < plan.size(); file++) {
        if (!ExtractEntry(archive, plan[file].index, titleDir + "/" + plan[file].target + ".part")) {
            status = ZipInstallStatus::ExtractFailed;
        }
    }
    zip_close(archive);
    if (status != ZipInstallStatus::Installed) {
        RemoveParts(plan, titleDir);
        return status;
    }

    for (const PlannedFile& file : plan) {
        const std::string target = titleDir + "/" + file.target;
        if (std::rename((target + ".part").c_str(), target.c_str()) != 0) {
            status = ZipInstallStatus::ReplaceFailed;
        }
    }
    RemoveParts(plan, titleDir);
    return status;
}

} // namespace enhancements
} // namespace ssb64
