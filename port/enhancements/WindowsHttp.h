#pragma once

#include <string>

namespace ssb64 {
namespace enhancements {

/**
 * Fetches a small text document (such as the GitHub releases API response) over HTTPS with WinHTTP,
 * inside the game process. The Windows updater used to spawn curl.exe for this.
 *
 * @param url HTTPS URL to fetch.
 * @param responseBody Receives the body; capped at 1 MiB.
 * @return true only for a 200 response that was read completely.
 */
bool WindowsHttpGetString(const std::string& url, std::string* responseBody);

} // namespace enhancements
} // namespace ssb64
