/*
 * Port menu button for the PS5 title. The system keeps the Create button (SDL's Back, which opens
 * the menu on the other consoles) for its capture menu, so the touchpad click, or L3+R3 together,
 * toggles the menu instead.
 */
#ifdef __PS5__

#include <SDL2/SDL.h>

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
    return pressed;
}

#endif
