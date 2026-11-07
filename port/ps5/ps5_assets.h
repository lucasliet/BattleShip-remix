#pragma once

#include <string>

namespace ssb64 {

/**
 * Makes sure the game archive in the PS5 data folder matches this build, extracting it with Torch
 * from the user's ROM when it is missing or stale. The ROM is looked up in the data folder, then in
 * the title folder (a folder install in /data/homebrew is writable; an .ffpkg is not).
 * @param dataDir writable folder that receives the archive (/data/BattleShip-Remix)
 * @param titleDir the title folder (/app0), with Torch's config.yml and yamls/ under assets/
 * @return true when a current archive is in place
 */
bool Ps5EnsureGameArchive(const std::string& dataDir, const std::string& titleDir);

} // namespace ssb64
