#include "widescreen.h"

#include "../enhancements/enhancements.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <libultraship/bridge/gfxbridge.h>

namespace {

constexpr const char* kWidescreenCVar = "gEnhancements.Widescreen";
// 0 = Stretch (legacy wide pixels), 1 = Center4_3 (letterboxed 2D,
// undistorted). The battle-HUD Spread markers land with the decomp-side
// anchor PRs; Center4_3 is the interim default so nothing distorts.
constexpr const char* kWidescreen2DPolicyCVar = "gEnhancements.Widescreen2DPolicy";

#ifdef __SWITCH__
// Hardware test: Center4_3 leaves side bars on battle backdrops and offsets the
// CSS fighters from the selector, so the Switch stays 4:3 until the Phase 2 anchors land.
constexpr int kWidescreenDefault = 0;
#else
constexpr int kWidescreenDefault = 1;
#endif

bool widescreen_enabled() {
    return CVarGetInteger(kWidescreenCVar, kWidescreenDefault) != 0;
}

int widescreen_2d_policy() {
    return CVarGetInteger(kWidescreen2DPolicyCVar, 1);
}

} // namespace

extern "C" int port_widescreen_enabled(void) {
    return widescreen_enabled() ? 1 : 0;
}

extern "C" float port_widescreen_clip_x_scale(void) {
    if (!widescreen_enabled()) {
        return 1.0f;
    }
    return GfxGetWidescreenClipXScale();
}

extern "C" void port_widescreen_tick(void) {
    /* Mirror the CVar state into libultraship's Fast3D interpreter via the
     * GfxSetWidescreenActive bridge. When active, Interpreter::AdjXForAspectRatio
     * compresses post-projection clip-space X by (4/3)/window_aspect, expanding
     * the visible 4:3 frustum into the wider window. 2D rect ops (HUD,
     * backgrounds, menus) follow the 2D anchor policy CVar:
     * gEnhancements.Widescreen2DPolicy — 1 (default) = Center4_3, mapping the
     * authored 320-space into the centered 4:3 region with pixel aspect 1:1
     * (menus/title/2D effects letterbox; no distortion); 0 = legacy Stretch
     * (authored NDC across the full wide FB). The battle-HUD Spread markers
     * come with the decomp-side anchor PRs (see docs/switch_port_2026-10-12.md).
     *
     * AdjXForAspectRatio derived from Ship of Harkinian's libultraship fork
     *   Copyright (c) The Harbour Masters
     *   https://github.com/HarbourMasters/libultraship
     * Licensed under MIT; see LICENSE at repository root. */
    const int active = widescreen_enabled() ? 1 : 0;
    static int s_last = -1;
    if (active != s_last) {
        s_last = active;
        GfxSetWidescreenActive(active);
    }
    const int policy = widescreen_2d_policy();
    static int s_last_policy = -1;
    if (policy != s_last_policy || (active != 0 && s_last_policy == -1)) {
        s_last_policy = policy;
        // Off => Stretch keeps the CVar-off behavior byte-identical (the
        // anchor transform is a no-op when widescreen is inactive anyway).
        GfxSetUIAnchorMode(active ? policy : 0);
    }
}

namespace ssb64 {
namespace enhancements {

const char* WidescreenCVarName() {
    return kWidescreenCVar;
}

} // namespace enhancements
} // namespace ssb64
