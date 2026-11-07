/*
 * First-launch asset extraction on the PS5: the title cannot run a Torch sidecar, so Torch is linked
 * in (as on Android) and turns the user's ROM in the title folder into the game archive.
 */
#ifdef __PS5__

#include "ps5_assets.h"

#include "Companion.h"
#include "port_log.h"

#include <array>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

extern "C" void ps5_boot_trace(const char* step);
extern "C" void ps5_notify(const char* message);

namespace ssb64 {
namespace {

constexpr std::array<const char*, 3> kRomNames = { "baserom.remix.z64", "baserom.remix.n64", "baserom.remix.v64" };

/**
 * Logs a message to both the port log and the boot trace, which survives a crash.
 * @param message the line to record
 */
void Report(const std::string& message) {
    port_log("SSB64: %s\n", message.c_str());
    ps5_boot_trace(message.c_str());
}

/**
 * Reports a message and shows it to the user as a system notification.
 * @param message the line to record and show
 */
void Announce(const std::string& message) {
    Report(message);
    ps5_notify(("BattleShip Remix: " + message).c_str());
}

std::string ReadRecipe(const std::filesystem::path& archive) {
    std::ifstream sidecar(archive.string() + ".recipe");
    std::string recipe;
    std::getline(sidecar, recipe);
    return recipe;
}

bool ArchiveIsCurrent(const std::filesystem::path& archive) {
    std::error_code ec;
    return std::filesystem::exists(archive, ec) && ReadRecipe(archive) == SSB64_ASSET_RECIPE_HASH;
}

/**
 * Finds the ROM by name in a folder; Torch checks its SHA-1 against config.yml. The title cannot
 * list /app0 (opendir fails there), so only the documented names are tried.
 * @return the ROM path, or an empty path when there is none
 */
std::filesystem::path FindRom(const std::filesystem::path& directory) {
    for (const char* romName : kRomNames) {
        const std::filesystem::path candidate = directory / romName;
        FILE* file = fopen(candidate.c_str(), "rb");
        if (file != nullptr) {
            fclose(file);
            return candidate;
        }
    }
    return {};
}

/**
 * Runs Torch on the ROM.
 * @return Torch's output path, empty when Torch rejected the ROM or its configuration
 */
std::string RunTorch(const std::filesystem::path& rom, const std::string& configDir, const std::string& outputDir) {
    Companion* companion = new Companion(rom, ArchiveType::O2R, false, configDir, outputDir);
    Companion::Instance = companion;
    companion->Init(ExportType::Binary);
    const std::string outputPath = companion->GetOutputPath();
    Companion::Instance = nullptr;
    delete companion;
    return outputPath;
}

bool FinalizeArchive(const std::filesystem::path& written, const std::filesystem::path& archive) {
    std::error_code ec;
    if (written != archive) {
        std::filesystem::rename(written, archive, ec);
        if (ec) {
            Announce("could not rename " + written.string() + " to " + archive.string() + ": " + ec.message());
            return false;
        }
    }
    std::ofstream recipe(archive.string() + ".recipe", std::ios::trunc);
    recipe << SSB64_ASSET_RECIPE_HASH << '\n';
    return static_cast<bool>(recipe);
}

} // namespace

bool Ps5EnsureGameArchive(const std::string& titleDir) {
    const std::string torchConfigDir = titleDir + "/assets";
    const std::filesystem::path archive = std::filesystem::path(titleDir) / SSB64_O2R_NAME;
    if (ArchiveIsCurrent(archive)) {
        return true;
    }
    const std::filesystem::path rom = FindRom(titleDir);
    if (rom.empty()) {
        Announce("no baserom.remix.z64 (.n64/.v64) in " + titleDir + "; copy the Smash Remix ROM there");
        return false;
    }
    Announce("extracting the game files from your ROM, this takes a minute or two");
    try {
        const std::string written = RunTorch(rom, torchConfigDir, titleDir);
        std::error_code ec;
        if (written.empty() || !std::filesystem::exists(written, ec)) {
            Announce("Torch did not write an archive: unsupported ROM dump or missing " + torchConfigDir +
                   "/config.yml");
            return false;
        }
        if (!FinalizeArchive(written, archive)) {
            return false;
        }
    } catch (const std::exception& error) {
        Announce(std::string("asset extraction failed: ") + error.what());
        return false;
    }
    Announce("game files ready");
    return true;
}

} // namespace ssb64

#endif
