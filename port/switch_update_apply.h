#pragma once

#ifdef __SWITCH__

#include <string>

namespace ssb64 {

/**
 * Moves a staged update (update-pending/ written by the in-game updater) into the app folder.
 *
 * Runs before libultraship opens any file, because the HOS file system refuses to rename or delete
 * a file that is open.
 *
 * @param appDir folder of the running .nro.
 * @return false when a complete staged update could not be applied; the error applet was shown.
 */
bool SwitchApplyPendingUpdate(const std::string& appDir);

} // namespace ssb64

#endif
