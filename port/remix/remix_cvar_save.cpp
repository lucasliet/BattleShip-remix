/**
 * remix_cvar_save.cpp - see remix_cvar_save.h.
 */

#include "remix_cvar_save.h"

#include <libultraship/bridge/consolevariablebridge.h>
#include <ship/Context.h>
#include <ship/window/Window.h>
#include <ship/window/gui/Gui.h>

namespace {

void SaveNextFrame() {
    auto context = Ship::Context::GetInstance();
    if (!context || !context->GetWindow() || !context->GetWindow()->GetGui()) {
        return;
    }
    context->GetWindow()->GetGui()->SaveConsoleVariablesNextFrame();
}

} // namespace

extern "C" void remix_cvar_set_saved(const char *name, int value) {
    const bool changed = (CVarGet(name) == nullptr) || (CVarGetInteger(name, value) != value);
    CVarSetInteger(name, value);
    if (changed) {
        SaveNextFrame();
    }
}
