#include "SwitchHttp.h"

#include <cstdio>
#include <memory>
#include <mutex>

#include <curl/curl.h>

namespace ssb64 {
namespace enhancements {

namespace {

constexpr const char* kUserAgent = "BattleShip-Updater";
constexpr long kConnectTimeoutSeconds = 15;
constexpr long kMaxRedirects = 5;
constexpr long kStalledTransferSeconds = 30;
constexpr long kStalledTransferBytesPerSecond = 1;
constexpr long kPageTimeoutSeconds = 30;
constexpr size_t kMaxPageBytes = 1024 * 1024;

using CurlHandle = std::unique_ptr<CURL, decltype(&curl_easy_cleanup)>;

struct PageSink {
    std::string* body;
};

struct FileSink {
    FILE* file;
    const HttpProgressCallback* onProgress;
    int lastPercent;
};

void InitializeCurlOnce() {
    static std::once_flag flag;
    std::call_once(flag, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });
}

size_t AppendToPage(char* chunk, size_t itemSize, size_t itemCount, void* userData) {
    PageSink* sink = static_cast<PageSink*>(userData);
    size_t chunkBytes = itemSize * itemCount;
    if (sink->body->size() + chunkBytes > kMaxPageBytes) return 0;

    sink->body->append(chunk, chunkBytes);
    return chunkBytes;
}

size_t WriteToFile(char* chunk, size_t itemSize, size_t itemCount, void* userData) {
    FileSink* sink = static_cast<FileSink*>(userData);
    return std::fwrite(chunk, itemSize, itemCount, sink->file);
}

int ReportProgress(void* userData, curl_off_t downloadTotal, curl_off_t downloadNow, curl_off_t, curl_off_t) {
    FileSink* sink = static_cast<FileSink*>(userData);
    if (downloadTotal <= 0) return 0;

    int percent = static_cast<int>((downloadNow * 100) / downloadTotal);
    if (percent == sink->lastPercent) return 0;

    sink->lastPercent = percent;
    if (*sink->onProgress) (*sink->onProgress)(percent);
    return 0;
}

void ConfigureHttpsRequest(CURL* curl, const std::string& url, char* errorBuffer) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, kUserAgent);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, kMaxRedirects);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, kConnectTimeoutSeconds);
}

HttpOutcome PerformRequest(CURL* curl, const char* errorBuffer) {
    HttpOutcome outcome;
    CURLcode code = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &outcome.httpStatus);
    if (code == CURLE_OK) {
        outcome.ok = true;
        return outcome;
    }

    outcome.error = curl_easy_strerror(code);
    if (errorBuffer[0] != '\0') outcome.error += std::string(" (") + errorBuffer + ")";
    return outcome;
}

} // namespace

HttpOutcome HttpGetString(const std::string& url, std::string* responseBody) {
    InitializeCurlOnce();
    CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) return HttpOutcome{false, 0, "Could not create the HTTP client."};

    char errorBuffer[CURL_ERROR_SIZE] = {};
    PageSink sink{responseBody};
    ConfigureHttpsRequest(curl.get(), url, errorBuffer);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, kPageTimeoutSeconds);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, AppendToPage);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &sink);
    return PerformRequest(curl.get(), errorBuffer);
}

HttpOutcome HttpDownloadToFile(const std::string& url, const std::string& destinationPath,
                               const HttpProgressCallback& onProgress) {
    InitializeCurlOnce();
    CurlHandle curl(curl_easy_init(), curl_easy_cleanup);
    if (!curl) return HttpOutcome{false, 0, "Could not create the HTTP client."};

    FILE* file = std::fopen(destinationPath.c_str(), "wb");
    if (file == nullptr) return HttpOutcome{false, 0, "Could not create " + destinationPath + "."};

    char errorBuffer[CURL_ERROR_SIZE] = {};
    FileSink sink{file, &onProgress, -1};
    ConfigureHttpsRequest(curl.get(), url, errorBuffer);
    curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_LIMIT, kStalledTransferBytesPerSecond);
    curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, kStalledTransferSeconds);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteToFile);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &sink);
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, ReportProgress);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &sink);

    HttpOutcome outcome = PerformRequest(curl.get(), errorBuffer);
    bool closedCleanly = std::fclose(file) == 0;
    if (outcome.ok && !closedCleanly) outcome = HttpOutcome{false, outcome.httpStatus, "Could not finish writing " + destinationPath + "."};
    if (!outcome.ok) std::remove(destinationPath.c_str());
    return outcome;
}

} // namespace enhancements
} // namespace ssb64
