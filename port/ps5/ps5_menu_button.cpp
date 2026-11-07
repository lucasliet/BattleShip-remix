/*
 * Port menu button for the PS5 title. The system keeps the Create button (SDL's Back, which opens
 * the menu on the other consoles) for its capture menu, so the touchpad click, or L3+R3 together,
 * toggles the menu instead.
 */
#ifdef __PS5__

#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>

#include <cstdio>

extern "C" void ps5_boot_trace(const char* step);

namespace {

/**
 * Reports whether any open game controller holds the menu chord this frame.
 * @return true while the touchpad or L3+R3 is held on any controller
 */
bool AnyControllerHoldsMenuChord() {
    for (int index = 0; index < SDL_NumJoysticks(); index++) {
        SDL_GameController* controller = SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(index));
        if (controller == nullptr) {
            continue;
        }
        const bool touchpad = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_TOUCHPAD) != 0;
        const bool sticks = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK) != 0 &&
                            SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK) != 0;
        if (touchpad || sticks) {
            return true;
        }
    }
    return false;
}

bool sChordWasHeld = false;

} // namespace

/**
 * Returns true once per press of the menu chord. Called on the main thread from PortPushFrame,
 * after the window has pumped SDL events.
 * @return true when the port menu should toggle this frame
 */
extern "C" bool port_drain_pending_menu_toggle(void) {
    const bool held = AnyControllerHoldsMenuChord();
    const bool pressed = held && !sChordWasHeld;
    sChordWasHeld = held;
    if (pressed) {
        // ImGui looks for gamepads at start and on SDL_CONTROLLERDEVICEADDED, which the PS5's SDL
        // does not send for pads opened later; without this the menu opens but cannot be navigated.
        ImGui_ImplSDL2_SetGamepadMode(ImGui_ImplSDL2_GamepadMode_AutoAll);
    }
    return pressed;
}

/**
 * Temporary: once a second while the menu is open, records what ImGui sees of the pad.
 * @param menuVisible whether the port menu is drawn this frame
 */
extern "C" void port_trace_menu_navigation(bool menuVisible) {
    static unsigned frames;
    if (!menuVisible || ++frames % 60 != 0) return;
    const ImGuiIO& io = ImGui::GetIO();
    SDL_GameController* pad = SDL_NumJoysticks() > 0 ? SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(0)) : nullptr;
    char line[256];
    std::snprintf(line, sizeof line,
                  "menu nav: navflag=%d hasGamepad=%d imguiDown=%d imguiA=%d sdlDown=%d sdlA=%d joysticks=%d isController=%d",
                  (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0,
                  (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0, ImGui::IsKeyDown(ImGuiKey_GamepadDpadDown),
                  ImGui::IsKeyDown(ImGuiKey_GamepadFaceDown),
                  pad != nullptr && SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN),
                  pad != nullptr && SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A), SDL_NumJoysticks(),
                  SDL_IsGameController(0));
    ps5_boot_trace(line);
}

#endif
