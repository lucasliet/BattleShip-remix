#include "WindowsHttp.h"

#include "port_log.h"

#include <memory>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winhttp.h>

namespace ssb64 {
namespace enhancements {

namespace {

constexpr const wchar_t* kUserAgent = L"BattleShip-Updater";
constexpr int kTimeoutMilliseconds = 10000;
constexpr size_t kMaxPageBytes = 1024 * 1024;
constexpr DWORD kHttpOk = 200;

struct InternetHandleCloser {
    void operator()(void* handle) const {
        WinHttpCloseHandle(handle);
    }
};

using InternetHandle = std::unique_ptr<void, InternetHandleCloser>;

struct HttpsTarget {
    std::wstring host;
    std::wstring pathAndQuery;
    INTERNET_PORT port = INTERNET_DEFAULT_HTTPS_PORT;
};

bool LogWinHttpFailure(const char* step) {
    port_log("Updater: WinHTTP could not %s (error %lu)\n", step, GetLastError());
    return false;
}

/**
 * Splits an HTTPS URL into the host, port and path WinHTTP takes separately.
 *
 * @param url ASCII HTTPS URL.
 * @param target Receives the parts.
 * @return false when the URL is not HTTPS or cannot be parsed.
 */
bool ParseHttpsUrl(const std::string& url, HttpsTarget* target) {
    const std::wstring wideUrl(url.begin(), url.end());
    URL_COMPONENTS components{};
    components.dwStructSize = sizeof(components);
    components.dwHostNameLength = static_cast<DWORD>(-1);
    components.dwUrlPathLength = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);
    if (!WinHttpCrackUrl(wideUrl.c_str(), 0, 0, &components)) return false;
    if (components.nScheme != INTERNET_SCHEME_HTTPS || components.dwHostNameLength == 0) return false;

    target->host.assign(components.lpszHostName, components.dwHostNameLength);
    target->pathAndQuery.assign(components.lpszUrlPath, components.dwUrlPathLength);
    if (components.lpszExtraInfo != nullptr) {
        target->pathAndQuery.append(components.lpszExtraInfo, components.dwExtraInfoLength);
    }
    target->port = components.nPort;
    return true;
}

/**
 * Opens a GET request with a 10 s timeout on every phase.
 *
 * @param target Parsed HTTPS URL.
 * @param session Receives the session handle, which must outlive the request.
 * @param connection Receives the connection handle, which must outlive the request.
 * @return The request handle, or an empty handle after logging the failed step.
 */
InternetHandle OpenGetRequest(const HttpsTarget& target, InternetHandle* session, InternetHandle* connection) {
    session->reset(WinHttpOpen(kUserAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
                               WINHTTP_NO_PROXY_BYPASS, 0));
    if (!*session) {
        LogWinHttpFailure("open a session");
        return InternetHandle();
    }
    WinHttpSetTimeouts(session->get(), kTimeoutMilliseconds, kTimeoutMilliseconds, kTimeoutMilliseconds,
                       kTimeoutMilliseconds);

    connection->reset(WinHttpConnect(session->get(), target.host.c_str(), target.port, 0));
    if (!*connection) {
        LogWinHttpFailure("connect");
        return InternetHandle();
    }

    InternetHandle request(WinHttpOpenRequest(connection->get(), L"GET", target.pathAndQuery.c_str(), nullptr,
                                              WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES,
                                              WINHTTP_FLAG_SECURE));
    if (!request) LogWinHttpFailure("open the request");
    return request;
}

DWORD QueryStatusCode(HINTERNET request) {
    DWORD status = 0;
    DWORD size = sizeof(status);
    if (!WinHttpQueryHeaders(request, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                             WINHTTP_HEADER_NAME_BY_INDEX, &status, &size, WINHTTP_NO_HEADER_INDEX)) {
        return 0;
    }
    return status;
}

/**
 * Reads the whole response body.
 *
 * @param request Request whose response was received.
 * @param body Receives the body.
 * @return false on a read error or when the body exceeds 1 MiB.
 */
bool ReadResponseBody(HINTERNET request, std::string* body) {
    std::vector<char> chunk;
    for (;;) {
        DWORD available = 0;
        if (!WinHttpQueryDataAvailable(request, &available)) return LogWinHttpFailure("read the response");
        if (available == 0) return true;
        if (body->size() + available > kMaxPageBytes) {
            port_log("Updater: response is larger than %zu bytes\n", kMaxPageBytes);
            return false;
        }
        chunk.resize(available);
        DWORD read = 0;
        if (!WinHttpReadData(request, chunk.data(), available, &read)) {
            return LogWinHttpFailure("read the response");
        }
        body->append(chunk.data(), read);
    }
}

} // namespace

bool WindowsHttpGetString(const std::string& url, std::string* responseBody) {
    HttpsTarget target;
    if (!ParseHttpsUrl(url, &target)) {
        port_log("Updater: expected an https:// URL, got %s\n", url.c_str());
        return false;
    }

    InternetHandle session;
    InternetHandle connection;
    InternetHandle request = OpenGetRequest(target, &session, &connection);
    if (!request) return false;

    if (!WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        return LogWinHttpFailure("send the request");
    }
    if (!WinHttpReceiveResponse(request.get(), nullptr)) return LogWinHttpFailure("receive the response");

    const DWORD status = QueryStatusCode(request.get());
    if (status != kHttpOk) {
        port_log("Updater: HTTP status %lu from %s\n", status, url.c_str());
        return false;
    }
    responseBody->clear();
    return ReadResponseBody(request.get(), responseBody);
}

} // namespace enhancements
} // namespace ssb64
