#pragma once

#include <string>

namespace ssb64 {

/**
 * Makes sure the game archive in the PS5 data folder matches this build, extracting it with Torch
 * from the user's ROM in the same folder when it is missing or stale.
 * @param dataDir writable folder holding the ROM and the archive (/data/BattleShip-Remix)
 * @param torchConfigDir folder holding Torch's config.yml and yamls/ (inside /app0)
 * @return true when a current archive is in place
 */
bool Ps5EnsureGameArchive(const std::string& dataDir, const std::string& torchConfigDir);

} // namespace ssb64
