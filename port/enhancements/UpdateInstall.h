#pragma once

#include <cstdint>
#include <string>

namespace ssb64 {
namespace enhancements {

/**
 * Outcome of swapping a downloaded .nro over the installed one.
 */
enum class InstallStatus {
    Installed,
    PartMissing,
    PartEmpty,
    SizeMismatch,
    NotNroImage,
    BackupFailed,
    ReplaceFailed,
};

/**
 * Describes an install outcome as a short message for the user.
 *
 * @param status Value returned by InstallDownloadedNro.
 * @return Static, human-readable text.
 */
const char* DescribeInstallStatus(InstallStatus status);

/**
 * Validates a downloaded .nro and moves it over the installed one.
 * The old file is kept as "<target>.old" until the new one is in place, and is
 * restored if the swap fails, so a failed update never leaves the user without a game.
 *
 * @param partPath Fully downloaded file ("<target>.part").
 * @param targetPath Installed .nro to replace.
 * @param expectedSizeBytes Size announced by the release asset, or 0 when unknown.
 * @return Installed on success, otherwise the first check that failed.
 */
InstallStatus InstallDownloadedNro(const std::string& partPath, const std::string& targetPath,
                                   std::uint64_t expectedSizeBytes);

} // namespace enhancements
} // namespace ssb64
