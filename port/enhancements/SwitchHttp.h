#pragma once

#include <functional>
#include <string>

namespace ssb64 {
namespace enhancements {

/**
 * Result of one HTTPS request made through libcurl.
 */
struct HttpOutcome {
    bool ok = false;
    long httpStatus = 0;
    std::string error;
};

/**
 * Receives the download progress as a whole percentage (0-100), only when it changes.
 */
using HttpProgressCallback = std::function<void(int percent)>;

/**
 * Fetches a small text document (such as the GitHub releases API response) over HTTPS.
 * Requires the libnx socket service to be initialized. Safe to call from a worker thread.
 *
 * @param url HTTPS URL to fetch.
 * @param responseBody Receives the body; capped at 1 MiB.
 * @return ok is true only for a 2xx response that was read completely.
 */
HttpOutcome HttpGetString(const std::string& url, std::string* responseBody);

/**
 * Downloads an HTTPS URL into a file, following redirects to HTTPS hosts only.
 * The file is removed again when the transfer fails.
 *
 * @param url HTTPS URL to download.
 * @param destinationPath File to create or overwrite.
 * @param onProgress Called from the calling thread while the transfer runs.
 * @return ok is true only when the whole body was written and the file closed cleanly.
 */
HttpOutcome HttpDownloadToFile(const std::string& url, const std::string& destinationPath,
                               const HttpProgressCallback& onProgress);

} // namespace enhancements
} // namespace ssb64
