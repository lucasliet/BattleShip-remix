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

void ShowAssetError(const std::string& headline) {
    const std::string appDir = Ship::Context::GetAppDirectoryPath();
    const std::string details =
        headline + "\n\n"
        "Run the desktop build of this same release once with your Smash Remix 2.0.1 ROM, then copy "
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
        ShowAssetError(SSB64_O2R_NAME " is missing.");
        return false;
    }
    const std::string recipe = ReadRecipe(archive);
    if (recipe == SSB64_ASSET_RECIPE_HASH) {
        return true;
    }
    port_log("SSB64: %s recipe '%s', this build wants '%s'\n", archive.c_str(),
             recipe.empty() ? "<none>" : recipe.c_str(), SSB64_ASSET_RECIPE_HASH);
    ShowAssetError(SSB64_O2R_NAME " is from a different version of the game.");
    return false;
}

} // namespace ssb64

#endif
