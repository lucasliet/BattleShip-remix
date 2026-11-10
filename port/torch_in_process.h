#pragma once

#include <string>

namespace ssb64 {

/**
 * Runs Torch inside the game process and turns the ROM into the game archive in outputDir.
 * Torch's log lines also go to logPath while it runs.
 * @param rom path of the user's ROM
 * @param configDir folder holding config.yml and yamls/
 * @param outputDir folder Torch writes the archive into
 * @param logPath extraction log; empty to keep Torch's lines in the game log only
 * @param error set to a user-facing reason when the run fails
 * @return true when Torch configured and wrote its archive
 */
bool RunTorchInProcess(const std::string& rom, const std::string& configDir, const std::string& outputDir,
                       const std::string& logPath, std::string& error);

} // namespace ssb64
