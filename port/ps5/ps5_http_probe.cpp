/*
 * Temporary check that the title can reach GitHub over HTTPS: when <title>/http_probe exists, the
 * latest release is fetched once at boot and the outcome goes to the boot trace.
 */
#ifdef __PS5__

#include "enhancements/SwitchHttp.h"

#include <cstdio>
#include <string>

extern "C" void ps5_boot_trace(const char* step);

namespace ssb64 {

void ProbeDownload(const std::string& label, const std::string& url, const std::string& titleDir) {
    const enhancements::HttpOutcome file =
        enhancements::HttpDownloadToFile(url, titleDir + "/http_probe_" + label + ".bin", nullptr);
    const std::string line = "http probe " + label + ": ok=" + std::to_string(file.ok) + " status=" +
                             std::to_string(file.httpStatus) + " error=" + file.error + " url=" + url;
    ps5_boot_trace(line.c_str());
}

void Ps5RunHttpProbe(const std::string& titleDir) {
    FILE* marker = std::fopen((titleDir + "/http_probe").c_str(), "rb");
    if (marker == nullptr) return;
    std::fclose(marker);
    std::string body;
    const enhancements::HttpOutcome page = enhancements::HttpGetString(
        "https://api.github.com/repos/lucasliet/BattleShip-remix/releases/latest", &body);
    const std::string pageLine = "http probe api: ok=" + std::to_string(page.ok) + " status=" +
                                 std::to_string(page.httpStatus) + " error=" + page.error + " bytes=" +
                                 std::to_string(body.size()) + " head=" + body.substr(0, 80);
    ps5_boot_trace(pageLine.c_str());
    ProbeDownload("github.com", "https://github.com/", titleDir);
    const std::string assetMarker = "\"url\":\"https://api.github.com/repos/lucasliet/BattleShip-remix/releases/assets/";
    const size_t at = body.find(assetMarker);
    if (at == std::string::npos) {
        ps5_boot_trace("http probe: no asset url in the release");
        return;
    }
    const size_t begin = at + 7;
    ProbeDownload("asset", body.substr(begin, body.find('"', begin) - begin), titleDir);
}

} // namespace ssb64

#endif
