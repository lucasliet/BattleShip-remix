#include "port_frame_profile.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>

int gPortFrameProfileEnabled = 0;

namespace {

struct CategoryTotals {
    std::atomic<uint64_t> ns{0};
    std::atomic<uint32_t> calls{0};
};

CategoryTotals sTotals[PORT_PROFILE_COUNT];

constexpr const char *kCategoryNames[PORT_PROFILE_COUNT] = {
    "dl check", "vtx fixup", "tex fixup","file load",
};

} /* namespace */

extern "C" void port_frame_profile_init(void) {
    const char *env = std::getenv("SSB64_FRAME_PROFILE");
    gPortFrameProfileEnabled = (env != nullptr) && (env[0] == '1');
}

extern "C" uint64_t port_frame_profile_now_ns(void) {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                                     std::chrono::steady_clock::now().time_since_epoch())
                                     .count());
}

extern "C" void port_frame_profile_add(PortProfileCategory category, uint64_t start_ns) {
    sTotals[category].ns.fetch_add(port_frame_profile_now_ns() - start_ns, std::memory_order_relaxed);
    sTotals[category].calls.fetch_add(1, std::memory_order_relaxed);
}

extern "C" void port_frame_profile_take(char *buf, unsigned long buf_size) {
    size_t used = 0;
    buf[0] = '\0';
    for (int i = 0; i < PORT_PROFILE_COUNT; i++) {
        const uint64_t ns = sTotals[i].ns.exchange(0, std::memory_order_relaxed);
        const uint32_t calls = sTotals[i].calls.exchange(0, std::memory_order_relaxed);
        if (used >= buf_size) {
            continue;
        }
        const int written = std::snprintf(buf + used, buf_size - used, "%s%s %.1f ms/%u", (i == 0) ? "" : ", ",
                                          kCategoryNames[i], ns / 1e6, calls);
        used += (written > 0) ? static_cast<size_t>(written) : 0;
    }
}
