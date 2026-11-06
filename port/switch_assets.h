#pragma once

#ifdef __SWITCH__

#include <string>

namespace ssb64 {

/**
 * Checks that the game archive and its .recipe sidecar on the SD card match this build.
 *
 * The Switch cannot extract assets itself, so a missing or outdated archive is reported with the
 * system error applet, telling the player to copy fresh files from a desktop build.
 *
 * @param archive path of the archive found on the SD card, or empty when there is none.
 * @return true when the archive is present and was extracted by this build's asset pipeline.
 */
bool SwitchAssetsAreCurrent(const std::string& archive);

} // namespace ssb64

#endif
