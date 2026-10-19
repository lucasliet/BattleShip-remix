/**
 * remix_display_settings.cpp - see remix_display_settings.h.
 *
 * Disable Anti-Aliasing (Toggles.entry_disable_aa) forces the port MSAA level to 1 while it is on. This replaces the VI
 * antialias register writes of AA.asm, which the port does not emulate. FPS Display (Toggles.entry_fps) opens the port
 * Stats window when it turns on, in place of the N64 FPS overlay of FPS.asm; the Stats window shows the port frame rate.
 */

#include "remix_display_settings.h"

#ifdef REGION_REMIX

#include "gui/Compat.h"
#include "remix/remix_symbols.h"
#include "remix/remix_toggles.h"

#include <libultraship/libultraship.h>
#include <libultraship/bridge/consolevariablebridge.h>

namespace {

constexpr uint32_t kMsaaLevelWithoutAntiAliasing = 1;

bool sFpsWasOn = false;

/** MSAA level of the renderer: 1 while Disable Anti-Aliasing is on, otherwise the Anti-aliasing (MSAA) CVar. */
uint32_t MsaaLevelForToggles() {
    if (remix_toggle_get(REMIX_SYM_Toggles__entry_disable_aa) != 0) {
        return kMsaaLevelWithoutAntiAliasing;
    }
    return static_cast<uint32_t>(CVarGetInteger(CVAR_MSAA_VALUE, 1));
}

void SyncMsaaLevel() {
    auto window = Ship::Context::GetInstance()->GetWindow();
    if (window) {
        window->SetMsaaLevel(MsaaLevelForToggles());
    }
}

/** Opens the Stats window on the off-to-on edge of FPS Display only, so closing it again sticks until the next edge. */
void SyncStatsWindowOnFpsEdge() {
    const bool fpsOn = remix_toggle_get(REMIX_SYM_Toggles__entry_fps) != 0;
    if (fpsOn && !sFpsWasOn) {
        CVarSetInteger(CVAR_STATS_WINDOW_OPEN, 1);
    }
    sFpsWasOn = fpsOn;
}

} // namespace

extern "C" void remix_display_settings_sync(void) {
    SyncMsaaLevel();
    SyncStatsWindowOnFpsEdge();
}

#endif
