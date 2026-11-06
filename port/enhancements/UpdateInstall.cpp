#include "UpdateInstall.h"

#include <cstdio>
#include <cstring>

namespace ssb64 {
namespace enhancements {

namespace {

constexpr long kNroMagicOffset = 0x10;
constexpr char kNroMagic[] = {'N', 'R', 'O', '0'};

bool FileExists(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "rb");
    if (file == nullptr) return false;
    std::fclose(file);
    return true;
}

bool ReadFileSize(FILE* file, std::uint64_t* outSize) {
    if (std::fseek(file, 0, SEEK_END) != 0) return false;
    long size = std::ftell(file);
    if (size < 0) return false;
    *outSize = static_cast<std::uint64_t>(size);
    return true;
}

bool HasNroMagic(FILE* file) {
    char magic[sizeof(kNroMagic)] = {};
    if (std::fseek(file, kNroMagicOffset, SEEK_SET) != 0) return false;
    if (std::fread(magic, 1, sizeof(magic), file) != sizeof(magic)) return false;
    return std::memcmp(magic, kNroMagic, sizeof(magic)) == 0;
}

InstallStatus ValidateDownloadedNro(const std::string& partPath, std::uint64_t expectedSizeBytes) {
    FILE* file = std::fopen(partPath.c_str(), "rb");
    if (file == nullptr) return InstallStatus::PartMissing;

    std::uint64_t size = 0;
    InstallStatus status = InstallStatus::Installed;
    if (!ReadFileSize(file, &size) || size == 0) {
        status = InstallStatus::PartEmpty;
    } else if (expectedSizeBytes != 0 && size != expectedSizeBytes) {
        status = InstallStatus::SizeMismatch;
    } else if (!HasNroMagic(file)) {
        status = InstallStatus::NotNroImage;
    }
    std::fclose(file);
    return status;
}

} // namespace

const char* DescribeInstallStatus(InstallStatus status) {
    switch (status) {
        case InstallStatus::Installed: return "Update installed.";
        case InstallStatus::PartMissing: return "Downloaded file is missing.";
        case InstallStatus::PartEmpty: return "Downloaded file is empty.";
        case InstallStatus::SizeMismatch: return "Downloaded file size does not match the release.";
        case InstallStatus::NotNroImage: return "Downloaded file is not a Switch homebrew (.nro).";
        case InstallStatus::BackupFailed: return "Could not back up the current game file.";
        case InstallStatus::ReplaceFailed: return "Could not replace the game file.";
    }
    return "Unknown update error.";
}

InstallStatus InstallDownloadedNro(const std::string& partPath, const std::string& targetPath,
                                   std::uint64_t expectedSizeBytes) {
    InstallStatus validation = ValidateDownloadedNro(partPath, expectedSizeBytes);
    if (validation != InstallStatus::Installed) return validation;

    const std::string backupPath = targetPath + ".old";
    std::remove(backupPath.c_str());

    const bool hadTarget = FileExists(targetPath);
    if (hadTarget && std::rename(targetPath.c_str(), backupPath.c_str()) != 0) return InstallStatus::BackupFailed;

    if (std::rename(partPath.c_str(), targetPath.c_str()) != 0) {
        if (hadTarget) std::rename(backupPath.c_str(), targetPath.c_str());
        return InstallStatus::ReplaceFailed;
    }

    std::remove(backupPath.c_str());
    return InstallStatus::Installed;
}

} // namespace enhancements
} // namespace ssb64
