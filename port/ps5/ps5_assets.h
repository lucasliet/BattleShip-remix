#pragma once

#include <string>

namespace ssb64 {

/**
 * Makes sure the game archive in the PS5 data folder matches this build, extracting it with Torch
 * from the user's ROM when it is missing or stale. The ROM is looked up in the data folder, then in
 * the title folder. The data folder is the title folder itself when that is writable (folder
 * installs), otherwise /data/BattleShip-Remix (.ffpkg installs).
 * @param dataDir writable folder that receives the archive
 * @param titleDir the title folder (/app0), with Torch's config.yml and yamls/ under assets/
 * @return true when a current archive is in place
 */
bool Ps5EnsureGameArchive(const std::string& dataDir, const std::string& titleDir);

} // namespace ssb64
