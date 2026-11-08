#pragma once

namespace ssb64 {
namespace enhancements {

/**
 * Folder next to the running .nro where the Switch updater unpacks a release zip during the
 * session; SwitchApplyPendingUpdate moves its files into place at the next boot.
 */
constexpr const char* kPendingFolderName = "update-pending";

/**
 * Empty file written last into the pending folder: a folder without it is an interrupted update
 * and is discarded at boot.
 */
constexpr const char* kPendingMarkerName = ".complete";

} // namespace enhancements
} // namespace ssb64
