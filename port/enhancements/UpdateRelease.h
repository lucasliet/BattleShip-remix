#pragma once

#include <cstdint>
#include <string>

namespace ssb64 {
namespace enhancements {

/**
 * Release fields the self-updaters need from the GitHub "latest release" response.
 */
struct ReleaseAsset {
    std::string tag;
    std::string downloadUrl;
    /** The asset's API URL (api.github.com), empty when the release has none or it is untrusted. */
    std::string apiUrl;
    std::uint64_t sizeBytes = 0;
};

/**
 * Outcome of reading the GitHub "latest release" response.
 */
enum class ReleaseParseStatus {
    Ok,
    Malformed,
    AssetMissing,
    UntrustedUrl,
};

/**
 * Tells whether a release asset URL may be downloaded by the updater.
 * Only plain https://github.com/ paths without quotes, spaces or control characters pass.
 *
 * @param url Value of the asset's browser_download_url field.
 * @return True when the URL is safe to hand to the HTTP client.
 */
bool IsTrustedDownloadUrl(const std::string& url);

/**
 * Tells whether a release asset API URL may be downloaded by the updater (with an
 * "Accept: application/octet-stream" header). The PS5 updater uses it: the console's HTTP
 * library cannot open github.com, while api.github.com works.
 * Only https://api.github.com/repos/ paths without quotes, spaces or control characters pass.
 *
 * @param url Value of the asset's url field.
 * @return True when the URL is safe to hand to the HTTP client.
 */
bool IsTrustedApiUrl(const std::string& url);

/**
 * Reads the tag and the named asset out of a GitHub "latest release" JSON body.
 * The tag is filled whenever the body is valid, even if the asset lookup fails,
 * so callers can report "up to date" without needing the asset.
 *
 * @param responseBody Raw JSON returned by the releases/latest endpoint.
 * @param assetName Exact asset file name to look for.
 * @param outRelease Receives the tag, download URL, API URL (when trusted) and size.
 * @return Ok when tag and a trusted asset URL were found, otherwise the reason it failed.
 */
ReleaseParseStatus ParseLatestRelease(const std::string& responseBody, const std::string& assetName,
                                      ReleaseAsset* outRelease);

} // namespace enhancements
} // namespace ssb64
