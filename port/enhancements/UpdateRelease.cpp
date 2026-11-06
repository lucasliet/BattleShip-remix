#include "UpdateRelease.h"

#include <nlohmann/json.hpp>

namespace ssb64 {
namespace enhancements {

namespace {

constexpr const char* kTrustedUrlPrefix = "https://github.com/";

bool HasStringField(const nlohmann::json& object, const char* key) {
    return object.contains(key) && object[key].is_string();
}

std::uint64_t ReadAssetSize(const nlohmann::json& asset) {
    if (!asset.contains("size") || !asset["size"].is_number_unsigned()) return 0;
    return asset["size"].get<std::uint64_t>();
}

const nlohmann::json* FindAssetByName(const nlohmann::json& release, const std::string& assetName) {
    if (!release.contains("assets") || !release["assets"].is_array()) return nullptr;

    for (const auto& asset : release["assets"]) {
        if (!asset.is_object() || !HasStringField(asset, "name")) continue;
        if (asset["name"].get<std::string>() == assetName) return &asset;
    }
    return nullptr;
}

} // namespace

bool IsTrustedDownloadUrl(const std::string& url) {
    if (url.rfind(kTrustedUrlPrefix, 0) != 0) return false;

    for (char c : url) {
        if (c == '"' || c == '\'' || c == '\\' || c <= 0x20 || c == 0x7F) return false;
    }
    return true;
}

ReleaseParseStatus ParseLatestRelease(const std::string& responseBody, const std::string& assetName,
                                      ReleaseAsset* outRelease) {
    nlohmann::json release = nlohmann::json::parse(responseBody, nullptr, false);
    if (!release.is_object() || !HasStringField(release, "tag_name")) return ReleaseParseStatus::Malformed;

    *outRelease = ReleaseAsset{};
    outRelease->tag = release["tag_name"].get<std::string>();

    const nlohmann::json* asset = FindAssetByName(release, assetName);
    if (asset == nullptr || !HasStringField(*asset, "browser_download_url")) return ReleaseParseStatus::AssetMissing;

    std::string url = (*asset)["browser_download_url"].get<std::string>();
    if (!IsTrustedDownloadUrl(url)) return ReleaseParseStatus::UntrustedUrl;

    outRelease->downloadUrl = url;
    outRelease->sizeBytes = ReadAssetSize(*asset);
    return ReleaseParseStatus::Ok;
}

} // namespace enhancements
} // namespace ssb64
