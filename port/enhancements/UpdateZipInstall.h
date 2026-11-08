#pragma once

#include <string>

namespace ssb64 {
namespace enhancements {

/**
 * Outcome of installing or unpacking a downloaded zip.
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

/**
 * How ExtractZipTo sets the permissions of the files it writes.
 */
enum class ZipFileMode {
    /** Apply the unix permission bits stored in the archive; keep the fopen default when it holds none. */
    FromArchive,
    /** chmod every file to 0777. */
    Executable,
    /** Never chmod (e.g. FAT on the Switch has no modes). */
    Ignore,
};

/**
 * Options of ExtractZipTo.
 */
struct ZipExtractOptions {
    /** Folder at the top of the zip, with its trailing slash; entries outside it are skipped and the prefix is removed. */
    std::string rootPrefix;
    ZipFileMode fileMode = ZipFileMode::FromArchive;
    /** When false, a symlink entry is an UnsafeEntry; when true it is created if its target stays inside the tree. */
    bool allowSymlinks = false;
};

/**
 * Unpacks the files of a zip under options.rootPrefix into destinationDir, which is created when
 * missing (its parent must exist). Files are written straight at their final path, so the folder
 * should be a fresh staging folder. A symlink entry is created only when options.allowSymlinks is
 * set and its target is relative, free of backslashes and of ".." segments. The result is
 * all-or-nothing: on any failure the files written so far are removed again.
 *
 * @param zipPath Downloaded zip.
 * @param options Prefix, permission and symlink handling.
 * @param destinationDir Folder to unpack into.
 * @return Installed on success, otherwise the first step that failed.
 */
ZipInstallStatus ExtractZipTo(const std::string& zipPath, const ZipExtractOptions& options,
                              const std::string& destinationDir);

} // namespace enhancements
} // namespace ssb64
