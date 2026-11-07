#pragma once

#include <string>

namespace ssb64 {
namespace enhancements {

/**
 * Outcome of installing a downloaded title-folder zip.
 */
enum class ZipInstallStatus {
    Installed,
    OpenFailed,
    NoEntries,
    UnsafeEntry,
    ExtractFailed,
    ReplaceFailed,
};

/**
 * Describes a zip install outcome as a short message for the user.
 *
 * @param status Value returned by InstallTitleZip.
 * @return Static, human-readable text.
 */
const char* DescribeZipInstallStatus(ZipInstallStatus status);

/**
 * Installs the files of a release zip into a title folder. Only entries under rootPrefix are
 * taken, with that prefix removed. Every file is first extracted next to its target as
 * "<target>.part"; the targets are replaced only once all of them extracted, so a failed update
 * leaves the installed files as they were. Files the zip does not hold (ROM, archive, saves) are
 * not touched.
 *
 * @param zipPath Downloaded zip.
 * @param rootPrefix Folder at the top of the zip, with its trailing slash (e.g. "PPSA99264/").
 * @param titleDir Folder to install into.
 * @return Installed on success, otherwise the first step that failed.
 */
ZipInstallStatus InstallTitleZip(const std::string& zipPath, const std::string& rootPrefix,
                                 const std::string& titleDir);

} // namespace enhancements
} // namespace ssb64
