/*
 * HTTPS for the PS5 title through the system HTTP library (sceHttp v1, system TLS and CA store),
 * behind the same interface as the Switch's libcurl client (enhancements/SwitchHttp.h). The call
 * sequence and values follow darkxex/ProsperoStore examples/update-check/update_check.c, which
 * runs on hardware from a sandboxed title built on the same boilerplate.
 */
#ifdef __PS5__

#include "enhancements/SwitchHttp.h"

#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>
#include <vector>

extern "C" {
int sceNetPoolCreate(const char* name, int size, int flags);
int sceSslInit(size_t poolSize);
int sceHttpInit(int netPool, int sslContext, size_t poolSize);
int sceHttpCreateTemplate(int context, const char* userAgent, int httpVersion, int autoProxy);
int sceHttpDeleteTemplate(int templateId);
int sceHttpCreateConnectionWithURL(int templateId, const char* url, int keepAlive);
int sceHttpDeleteConnection(int connectionId);
int sceHttpCreateRequestWithURL(int connectionId, int method, const char* url, uint64_t contentLength);
int sceHttpDeleteRequest(int requestId);
int sceHttpSetAutoRedirect(int id, int enabled);
int sceHttpSetResolveTimeOut(int id, uint32_t microseconds);
int sceHttpSetConnectTimeOut(int id, uint32_t microseconds);
int sceHttpSetSendTimeOut(int id, uint32_t microseconds);
int sceHttpSetRecvTimeOut(int id, uint32_t microseconds);
int sceHttpsEnableOption(int id, uint32_t flags);
int sceHttpAddRequestHeader(int id, const char* name, const char* value, int mode);
int sceHttpSendRequest(int requestId, const void* body, size_t size);
int sceHttpGetStatusCode(int requestId, int* status);
int sceHttpGetResponseContentLength(int requestId, int* result, size_t* length);
int sceHttpReadData(int requestId, void* buffer, size_t size);
}

namespace ssb64 {
namespace enhancements {

namespace {

constexpr const char* kUserAgent = "BattleShip-Updater";
constexpr int kHttpVersion11 = 2;
constexpr int kMethodGet = 0;
constexpr int kHeaderModeAdd = 1;
constexpr uint32_t kTimeoutMicroseconds = 15u * 1000u * 1000u;
constexpr uint32_t kTlsChecks = 0x01 | 0x04 | 0x08 | 0x10 | 0x20 | 0x80;
constexpr size_t kMaxPageBytes = 1024 * 1024;
constexpr size_t kReadChunkBytes = 64 * 1024;

struct HttpLibrary {
    int context = -1;
    std::string error;
};

std::string HexCode(int code) {
    char text[16];
    std::snprintf(text, sizeof text, "0x%08x", static_cast<unsigned>(code));
    return text;
}

/**
 * Initializes the network pool, TLS and HTTP libraries once for the process.
 * @return the HTTP context, or an error when a library refused to start
 */
const HttpLibrary& Library() {
    static HttpLibrary library;
    static std::once_flag flag;
    std::call_once(flag, []() {
        const int pool = sceNetPoolCreate("battleship-http", 1024 * 1024, 0);
        if (pool < 0) {
            library.error = "network pool " + HexCode(pool);
            return;
        }
        const int ssl = sceSslInit(304 * 1024);
        if (ssl < 0) {
            library.error = "TLS init " + HexCode(ssl);
            return;
        }
        const int context = sceHttpInit(pool, ssl, 4 * 1024 * 1024);
        if (context < 0) {
            library.error = "HTTP init " + HexCode(context);
            return;
        }
        library.context = context;
    });
    return library;
}

/**
 * Owns one template, connection and request for a GET, deleting them in reverse order.
 */
class Request {
  public:
    explicit Request(const std::string& url) {
        const HttpLibrary& library = Library();
        if (library.context < 0) {
            mError = library.error;
            return;
        }
        mTemplate = sceHttpCreateTemplate(library.context, kUserAgent, kHttpVersion11, 0);
        if (mTemplate < 0) {
            mError = "template " + HexCode(mTemplate);
            return;
        }
        sceHttpSetAutoRedirect(mTemplate, 1);
        sceHttpSetResolveTimeOut(mTemplate, kTimeoutMicroseconds);
        sceHttpSetConnectTimeOut(mTemplate, kTimeoutMicroseconds);
        sceHttpSetSendTimeOut(mTemplate, kTimeoutMicroseconds);
        sceHttpSetRecvTimeOut(mTemplate, kTimeoutMicroseconds);
        sceHttpsEnableOption(mTemplate, kTlsChecks);
        mConnection = sceHttpCreateConnectionWithURL(mTemplate, url.c_str(), 0);
        if (mConnection < 0) {
            mError = "connection " + HexCode(mConnection);
            return;
        }
        mRequest = sceHttpCreateRequestWithURL(mConnection, kMethodGet, url.c_str(), 0);
        if (mRequest < 0) {
            mError = "request " + HexCode(mRequest);
        }
    }

    ~Request() {
        if (mRequest >= 0) sceHttpDeleteRequest(mRequest);
        if (mConnection >= 0) sceHttpDeleteConnection(mConnection);
        if (mTemplate >= 0) sceHttpDeleteTemplate(mTemplate);
    }

    Request(const Request&) = delete;
    Request& operator=(const Request&) = delete;

    /**
     * Sends the request and reads the status line.
     * @return ok for a 2xx status; otherwise the status or the library error
     */
    HttpOutcome Send() {
        if (!mError.empty()) return HttpOutcome{false, 0, mError};
        sceHttpAddRequestHeader(mRequest, "Accept", "application/vnd.github+json, application/octet-stream",
                                kHeaderModeAdd);
        const int sent = sceHttpSendRequest(mRequest, nullptr, 0);
        if (sent < 0) return HttpOutcome{false, 0, "send " + HexCode(sent)};
        int status = 0;
        const int read = sceHttpGetStatusCode(mRequest, &status);
        if (read < 0) return HttpOutcome{false, 0, "status " + HexCode(read)};
        if (status < 200 || status >= 300) return HttpOutcome{false, status, "HTTP " + std::to_string(status)};
        return HttpOutcome{true, status, {}};
    }

    /**
     * @return the response length, or 0 when the server did not send one
     */
    size_t ContentLength() const {
        int result = 0;
        size_t length = 0;
        return sceHttpGetResponseContentLength(mRequest, &result, &length) >= 0 && result == 0 ? length : 0;
    }

    /**
     * @return bytes read into the buffer, 0 at the end of the body, negative on an error
     */
    int Read(char* buffer, size_t size) { return sceHttpReadData(mRequest, buffer, size); }

  private:
    int mTemplate = -1;
    int mConnection = -1;
    int mRequest = -1;
    std::string mError;
};

} // namespace

HttpOutcome HttpGetString(const std::string& url, std::string* responseBody) {
    Request request(url);
    HttpOutcome outcome = request.Send();
    if (!outcome.ok) return outcome;
    char chunk[4096];
    for (;;) {
        const int read = request.Read(chunk, sizeof chunk);
        if (read == 0) return outcome;
        if (read < 0) return HttpOutcome{false, outcome.httpStatus, "read " + HexCode(read)};
        if (responseBody->size() + static_cast<size_t>(read) > kMaxPageBytes) {
            return HttpOutcome{false, outcome.httpStatus, "response larger than 1 MiB"};
        }
        responseBody->append(chunk, static_cast<size_t>(read));
    }
}

HttpOutcome HttpDownloadToFile(const std::string& url, const std::string& destinationPath,
                               const HttpProgressCallback& onProgress) {
    Request request(url);
    HttpOutcome outcome = request.Send();
    if (!outcome.ok) return outcome;
    FILE* file = std::fopen(destinationPath.c_str(), "wb");
    if (file == nullptr) return HttpOutcome{false, outcome.httpStatus, "Could not create " + destinationPath + "."};

    const size_t total = request.ContentLength();
    std::vector<char> chunk(kReadChunkBytes);
    size_t received = 0;
    int lastPercent = -1;
    for (;;) {
        const int read = request.Read(chunk.data(), chunk.size());
        if (read == 0) break;
        if (read < 0 || std::fwrite(chunk.data(), 1, static_cast<size_t>(read), file) != static_cast<size_t>(read)) {
            outcome = HttpOutcome{false, outcome.httpStatus, read < 0 ? "read " + HexCode(read) : "write failed"};
            break;
        }
        received += static_cast<size_t>(read);
        const int percent = total > 0 ? static_cast<int>(received * 100 / total) : -1;
        if (percent != lastPercent && percent >= 0 && onProgress) onProgress(percent);
        lastPercent = percent;
    }
    const bool closedCleanly = std::fclose(file) == 0;
    if (outcome.ok && !closedCleanly) {
        outcome = HttpOutcome{false, outcome.httpStatus, "Could not finish writing " + destinationPath + "."};
    }
    if (!outcome.ok) std::remove(destinationPath.c_str());
    return outcome;
}

} // namespace enhancements
} // namespace ssb64

#endif
