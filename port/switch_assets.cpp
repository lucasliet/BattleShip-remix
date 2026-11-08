#ifdef __SWITCH__

#include "switch_assets.h"

#include <switch.h>
#include <filesystem>
#include <fstream>
#include <string>
#include "ship/Context.h"

#include "port_log.h"

#ifndef SSB64_ASSET_RECIPE_HASH
#define SSB64_ASSET_RECIPE_HASH "unversioned"
#endif

namespace ssb64 {

namespace {

std::string ReadRecipe(const std::string& archivePath) {
    std::ifstream sidecar(archivePath + ".recipe");
    std::string line;
    std::getline(sidecar, line);
    while (!line.empty() && (line.back() == '\r' || line.back() == ' ')) {
        line.pop_back();
    }
    return line;
}

std::string AbsoluteAppDirectory() {
    const std::string appDir = Ship::Context::GetAppDirectoryPath();
    std::error_code error;
    const std::filesystem::path absolute = std::filesystem::absolute(appDir, error);
    if (error) {
        return appDir;
    }
    return absolute.lexically_normal().string();
}

std::string ShortHash(const std::string& hash) {
    return hash.substr(0, 8);
}

void ShowAssetError(const std::string& headline, const std::string& reason) {
    const std::string appDir = AbsoluteAppDirectory();
    const std::string details =
        headline + "\n\n" + reason + "\n\n"
        "Run the desktop build of this same release once with your Smash Remix 2.0.1 ROM, then copy BOTH "
        SSB64_O2R_NAME " and " SSB64_O2R_NAME ".recipe from its app data folder to:\n  " + appDir +
        "\n\nSee README.txt in the release zip.";
    port_log("SSB64: %s\n", details.c_str());
    ErrorApplicationConfig config;
    if (R_SUCCEEDED(errorApplicationCreate(&config, headline.c_str(), details.c_str()))) {
        errorApplicationShow(&config);
    }
}

} // namespace

bool SwitchAssetsAreCurrent(const std::string& archive) {
    if (archive.empty()) {
        ShowAssetError(SSB64_O2R_NAME " is missing.", "The game data file was not found.");
        return false;
    }
    const std::string recipe = ReadRecipe(archive);
    if (recipe == SSB64_ASSET_RECIPE_HASH) {
        return true;
    }
    port_log("SSB64: %s recipe '%s', this build wants '%s'\n", archive.c_str(),
             recipe.empty() ? "<none>" : recipe.c_str(), SSB64_ASSET_RECIPE_HASH);
    if (recipe.empty()) {
        ShowAssetError(SSB64_O2R_NAME ".recipe is missing.",
                       "The " SSB64_O2R_NAME " is there, but the " SSB64_O2R_NAME
                       ".recipe file that goes with it was not found or is empty.");
        return false;
    }
    ShowAssetError(SSB64_O2R_NAME " is from a different version of the game.",
                   "Its recipe is " + ShortHash(recipe) + ", this build needs " +
                       ShortHash(SSB64_ASSET_RECIPE_HASH) +
                       ". The desktop and the Switch must run the same release.");
    return false;
}

} // namespace ssb64

#endif
