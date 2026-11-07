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
    const enhancements::HttpOutcome file = enhancements::HttpDownloadToFile(
        "https://github.com/lucasliet/BattleShip-remix/releases/latest/download/SHA256SUMS.txt",
        titleDir + "/http_probe_download.txt", nullptr);
    const std::string fileLine = "http probe download: ok=" + std::to_string(file.ok) + " status=" +
                                 std::to_string(file.httpStatus) + " error=" + file.error;
    ps5_boot_trace(fileLine.c_str());
}

} // namespace ssb64

#endif
