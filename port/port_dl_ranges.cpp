/**
 * port_dl_ranges.cpp — registry of valid DL memory ranges for the GFX
 * walker's bounds check. See port_dl_ranges.h for the rationale.
 *
 * Wired into libultraship at startup via port_dl_ranges_init(), which
 * registers our port-side bounds-check and address classifier as the
 * callbacks libultraship's Fast::Interpreter uses to validate DL
 * pointers and label diagnostic dumps. libultraship has no compile-time
 * dependency on this file — both callbacks are optional.
 */

#include "port_dl_ranges.h"
#include "fast/interpreter.h"
#include "port_frame_profile.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <vector>

namespace {

struct Entry {
    uintptr_t base;
    size_t size;
    const char *label;
};

/* Small registry (< 100 ranges typical). Registration/unregistration
 * happen only at file load/unload (rare); port_dl_check_addr is called
 * once per GBI command from gfx_step (extremely hot — tens of thousands
 * of calls per frame in a fight). The writer-side vector is guarded by
 * sRangesMtx, but the reader hot path must NOT take that mutex or do a
 * linear scan per command, or busy scenes (fights) tank to ~30 fps while
 * light scenes (menus) stay fine.
 *
 * Hot-path strategy: the GFX walker advances cmd linearly within one DL,
 * so consecutive calls almost always land in the same range. A
 * thread-local single-entry cache of the last in-range hit answers the
 * common case with two comparisons and one relaxed atomic load — no
 * mutex, no scan. sRangesGen is bumped on every mutating register/
 * unregister so a stale cached range can never mask a registry change;
 * on a generation mismatch or cache miss the call falls through to the
 * original locked scan, preserving exact classification behavior. */
std::vector<Entry> sRanges;
std::mutex sRangesMtx;
std::atomic<uint64_t> sRangesGen{0};

struct HitCache {
    uint64_t gen = ~0ull;   /* generation this cache entry was validated at */
    uintptr_t base = 0;
    size_t size = 0;
};
/* Several entries, not one: a scene with many fighters (the 1P VS card's 30 Polygons) branches between
 * the DLs of many files, so a single cached range missed on almost every command and fell into the
 * locked scan. */
constexpr size_t kHitCacheEntries = 8;
thread_local HitCache sHitCache[kHitCacheEntries];
thread_local size_t sHitCacheNext = 0;

/* "Walked past" threshold: how far past a registered range do we still
 * recognise as runaway-from-that-range? One 64 KiB window comfortably
 * covers the gap to the next mmap'd allocation while staying tight
 * enough to fire on the actual walk-off cases. */
constexpr size_t kWalkPastWindow = 0x10000;

} /* namespace */

extern "C" void port_dl_range_register(const void *base, size_t size, const char *label) {
    if ((base == nullptr) || (size == 0)) {
        return;
    }
    uintptr_t b = reinterpret_cast<uintptr_t>(base);
    std::lock_guard<std::mutex> lk(sRangesMtx);
    for (auto &e : sRanges) {
        if (e.base == b) {
            if (e.size != size || e.label != label) {
                e.size = size;
                e.label = label;
                sRangesGen.fetch_add(1, std::memory_order_release);
            }
            return;
        }
    }
    sRanges.push_back({b, size, label});
    sRangesGen.fetch_add(1, std::memory_order_release);
}

extern "C" void port_dl_range_unregister(const void *base) {
    if (base == nullptr) return;
    uintptr_t b = reinterpret_cast<uintptr_t>(base);
    std::lock_guard<std::mutex> lk(sRangesMtx);
    for (auto it = sRanges.begin(); it != sRanges.end(); ++it) {
        if (it->base == b) {
            sRanges.erase(it);
            sRangesGen.fetch_add(1, std::memory_order_release);
            return;
        }
    }
}

namespace {

/* Per-thread copy of the registry sorted by base, rebuilt only when the generation changes (file loads
 * happen in game logic, never during a DL walk). It replaces a locked linear scan per GBI command that
 * cost the Switch most of a frame on the 1P VS card, where 30 Polygons cycle through more files than
 * the hit cache holds. max_end[i] is the largest end among sorted[0..i], so a lookup can stop walking
 * back once no earlier range can still reach addr, which keeps overlapping ranges correct. */
struct RangeSnapshot {
    uint64_t gen = ~0ull;
    std::vector<Entry> sorted;
    std::vector<uintptr_t> max_end;
};
thread_local RangeSnapshot sSnapshot;

const RangeSnapshot &port_dl_ranges_snapshot(uint64_t gen) {
    if (sSnapshot.gen == gen) {
        return sSnapshot;
    }
    std::lock_guard<std::mutex> lk(sRangesMtx);
    sSnapshot.gen = sRangesGen.load(std::memory_order_acquire);
    sSnapshot.sorted = sRanges;
    std::sort(sSnapshot.sorted.begin(), sSnapshot.sorted.end(),
              [](const Entry &a, const Entry &b) { return a.base < b.base; });
    sSnapshot.max_end.resize(sSnapshot.sorted.size());
    uintptr_t max_end = 0;
    for (size_t i = 0; i < sSnapshot.sorted.size(); i++) {
        max_end = std::max(max_end, sSnapshot.sorted[i].base + sSnapshot.sorted[i].size);
        sSnapshot.max_end[i] = max_end;
    }
    return sSnapshot;
}

const Entry *port_dl_ranges_find(const RangeSnapshot &snapshot, uintptr_t addr) {
    auto upper = std::upper_bound(snapshot.sorted.begin(), snapshot.sorted.end(), addr,
                                  [](uintptr_t value, const Entry &e) { return value < e.base; });
    for (size_t i = static_cast<size_t>(upper - snapshot.sorted.begin()); i-- > 0;) {
        if (snapshot.max_end[i] <= addr) {
            return nullptr;
        }
        const Entry &e = snapshot.sorted[i];
        if ((addr - e.base) < e.size) {
            return &e;
        }
    }
    return nullptr;
}

} /* namespace */

static int port_dl_lookup_unprofiled(uintptr_t addr, uintptr_t *range_base, size_t *range_size) {
    *range_base = 0;
    *range_size = 0;
    if (addr == 0) return PORT_DL_UNKNOWN;

    /* Hot path: per-GBI-command. The walker advances cmd linearly, so the
     * previous in-range hit almost always still covers addr. Answer that
     * case with no mutex and no scan. Only valid while the registry hasn't
     * changed (generation match); any register/unregister bumps the gen
     * and forces a re-validation through the locked scan below. */
    const uint64_t gen = sRangesGen.load(std::memory_order_acquire);
    for (const HitCache &c : sHitCache) {
        if (c.gen == gen && c.size != 0 &&
            (addr >= c.base) && ((addr - c.base) < c.size)) {
            *range_base = c.base;
            *range_size = c.size;
            return PORT_DL_IN_RANGE;
        }
    }

    const RangeSnapshot &snapshot = port_dl_ranges_snapshot(gen);
    const Entry *hit = port_dl_ranges_find(snapshot, addr);
    if (hit != nullptr) {
        sHitCache[sHitCacheNext] = HitCache{snapshot.gen, hit->base, hit->size};
        sHitCacheNext = (sHitCacheNext + 1) % kHitCacheEntries;
        *range_base = hit->base;
        *range_size = hit->size;
        return PORT_DL_IN_RANGE;
    }

    /* Each range forbids [base, end + kWalkPastWindow): inside it addr is IN_RANGE or WALKED_PAST. An
     * UNKNOWN addr also reports the gap between forbidden spans around it, so the walker can answer the
     * game-built DLs that live outside every registered file (almost every command on the 1P VS card)
     * without calling back for each one. */
    bool walked_past = false;
    uintptr_t gap_lo = 0;
    uintptr_t gap_hi = UINTPTR_MAX;
    for (const auto &e : snapshot.sorted) {
        const uintptr_t end = e.base + e.size;
        const uintptr_t forbid_hi = (end > UINTPTR_MAX - kWalkPastWindow) ? UINTPTR_MAX : end + kWalkPastWindow;
        if ((addr >= end) && ((addr - end) < kWalkPastWindow)) {
            walked_past = true;
            /* Don't return immediately — another registered range further
             * along could legitimately contain addr (overlapping ranges
             * are allowed at registration). Only commit to WALKED_PAST
             * after confirming addr isn't in any other range. */
        }
        if (forbid_hi <= addr) {
            gap_lo = std::max(gap_lo, forbid_hi);
        } else if (e.base > addr) {
            gap_hi = std::min(gap_hi, e.base);
        }
    }
    if (walked_past) {
        return PORT_DL_WALKED_PAST;
    }
    *range_base = gap_lo;
    *range_size = gap_hi - gap_lo;
    return PORT_DL_UNKNOWN;
}

extern "C" int port_dl_range_lookup(uintptr_t addr, uintptr_t *range_base, size_t *range_size) {
    if (!gPortFrameProfileEnabled) {
        return port_dl_lookup_unprofiled(addr, range_base, range_size);
    }
    const uint64_t start_ns = port_frame_profile_now_ns();
    const int result = port_dl_lookup_unprofiled(addr, range_base, range_size);
    port_frame_profile_add(PORT_PROFILE_DL_CHECK, start_ns);
    return result;
}

extern "C" int port_dl_check_addr(uintptr_t addr) {
    uintptr_t range_base = 0;
    size_t range_size = 0;
    return port_dl_range_lookup(addr, &range_base, &range_size);
}

extern "C" int port_dl_range_classify_str(uintptr_t addr, char *buf, size_t buf_size) {
    if (buf == nullptr || buf_size == 0) return 0;
    if (addr == 0) {
        std::snprintf(buf, buf_size, "null");
        return 0;
    }
    std::lock_guard<std::mutex> lk(sRangesMtx);
    for (const auto &e : sRanges) {
        if ((addr >= e.base) && ((addr - e.base) < e.size)) {
            std::snprintf(buf, buf_size, "%s+0x%lx",
                          e.label ? e.label : "?",
                          static_cast<unsigned long>(addr - e.base));
            return 1;
        }
    }
    /* Fall through to a heuristic label so the diag still tells us
     * something useful for unregistered ranges. */
    if (addr <= 0x0FFFFFFFu) {
        std::snprintf(buf, buf_size, "n64_seg[%u]+0x%x",
                      static_cast<unsigned>((addr >> 24) & 0xFF),
                      static_cast<unsigned>(addr & 0x00FFFFFFu));
    } else if (addr < 0x100000000ull) {
        std::snprintf(buf, buf_size, "low_brk@0x%lx", static_cast<unsigned long>(addr));
    } else {
        std::snprintf(buf, buf_size, "other@0x%lx", static_cast<unsigned long>(addr));
    }
    return 0;
}

/* Called once at port startup to wire our bounds-check and classifier
 * into libultraship's GFX walker and diag dump. libultraship has zero
 * knowledge of port_dl_* symbols — only the function pointers it
 * receives from these registrations. */
extern "C" void port_dl_ranges_init(void) {
    Fast::RegisterDLBoundsCheck(&port_dl_check_addr);
    Fast::RegisterDLRangeLookup(&port_dl_range_lookup);
    Fast::RegisterAddressClassifier(&port_dl_range_classify_str);
}
