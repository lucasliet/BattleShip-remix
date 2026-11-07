/*
 * First-launch asset extraction on the PS5: the title cannot run a Torch sidecar, so Torch is linked
 * in (as on Android) and turns the user's ROM in the title folder into the game archive.
 */
#ifdef __PS5__

#include "ps5_assets.h"

#include "Companion.h"
#include "port_log.h"

#include <array>
#include <dirent.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

extern "C" void ps5_boot_trace(const char* step);

namespace ssb64 {
namespace {

constexpr std::array<const char*, 3> kRomExtensions = { ".z64", ".n64", ".v64" };

/**
 * Logs a message to both the port log and the boot trace, which survives a crash.
 * @param message the line to record
 */
void Report(const std::string& message) {
    port_log("SSB64: %s\n", message.c_str());
    ps5_boot_trace(message.c_str());
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
 * Finds the first N64 ROM dump in a folder; Torch checks its SHA-1 against config.yml. Uses
 * opendir/readdir: std::filesystem::directory_iterator finds nothing in /app0 on the console.
 * @return the ROM path, or an empty path when there is none
 */
std::filesystem::path FindRom(const std::filesystem::path& directory) {
    DIR* listing = opendir(directory.c_str());
    if (listing == nullptr) {
        Report("cannot list " + directory.string());
        return {};
    }
    std::filesystem::path rom;
    for (dirent* entry = readdir(listing); entry != nullptr && rom.empty(); entry = readdir(listing)) {
        const std::filesystem::path candidate = directory / entry->d_name;
        for (const char* romExtension : kRomExtensions) {
            if (candidate.extension() == romExtension) {
                rom = candidate;
            }
        }
    }
    closedir(listing);
    return rom;
}

/**
 * Runs Torch on the ROM and returns the archive it wrote.
 * @return Torch's output path, empty when Torch rejected the ROM or its configuration
 */
std::string RunTorch(const std::filesystem::path& rom, const std::string& configDir, const std::string& outputDir) {
    Companion* companion = new Companion(rom, ArchiveType::O2R, false, configDir, outputDir);
    Companion::Instance = companion;
    companion->Init(ExportType::Binary);
    std::string outputPath = companion->GetOutputPath();
    Companion::Instance = nullptr;
    delete companion;
    return outputPath;
}

bool FinalizeArchive(const std::filesystem::path& written, const std::filesystem::path& archive) {
    std::error_code ec;
    if (written != archive) {
        std::filesystem::rename(written, archive, ec);
        if (ec) {
            Report("could not rename " + written.string() + " to " + archive.string() + ": " + ec.message());
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
        Report("no ROM (.z64/.n64/.v64) in " + titleDir + "; copy the Smash Remix ROM there");
        return false;
    }
    Report("extracting assets from " + rom.string());
    try {
        const std::string written = RunTorch(rom, torchConfigDir, titleDir);
        std::error_code ec;
        if (written.empty() || !std::filesystem::exists(written, ec)) {
            Report("Torch did not write an archive: unsupported ROM dump or missing " + torchConfigDir +
                   "/config.yml");
            return false;
        }
        if (!FinalizeArchive(written, archive)) {
            return false;
        }
    } catch (const std::exception& error) {
        Report(std::string("asset extraction failed: ") + error.what());
        return false;
    }
    Report("asset extraction done");
    return true;
}

} // namespace ssb64

#endif
