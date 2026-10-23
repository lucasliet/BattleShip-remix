/*
 * remix_overlay.cpp - forwards Remix text messages to libultraship's GameOverlay, which draws its notifications in the
 * ImGui frame of the window.
 */
#include "remix/remix_overlay.h"

#include <libultraship/libultraship.h>

extern "C" void remix_overlay_show_text(const char *text, float seconds) {
    auto window = Ship::Context::GetInstance()->GetWindow();

    if (!window || !window->GetGui()) {
        return;
    }
    window->GetGui()->GetGameOverlay()->TextDrawNotification(seconds, true, "%s", text);
}
