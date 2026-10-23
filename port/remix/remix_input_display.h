/**
 * remix_input_display.h - Smash Remix Input Display (InputDisplay.asm) as a port overlay.
 *
 * The match feeds one controller sample per fighter and frame (decomp ftremix/ftremixinputdisplay.c). The overlay draws
 * the samples as N64 controller outlines on top of the window, anchored to the top or the bottom edge by the Input
 * Display panel value of each port. The N64 HUD sprites are not used, so the port needs no Remix sprite path.
 */

#ifndef PORT_REMIX_REMIX_INPUT_DISPLAY_H
#define PORT_REMIX_REMIX_INPUT_DISPLAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Stores the controller sample of `port` (0..3) for this frame. Samples outside the port range are ignored. */
void remix_input_display_update(int port, uint16_t buttons, int8_t stick_x, int8_t stick_y);

/** Hides every port. Called at each scene change so no sample outlives its match. */
void remix_input_display_clear(void);

#ifdef __cplusplus
}

namespace Ship {
class GuiWindow;
}

#include <memory>

/** Creates the overlay window that draws the samples. The port adds it to the Gui once at start-up. */
std::shared_ptr<Ship::GuiWindow> remix_input_display_make_window();
#endif

#endif /* PORT_REMIX_REMIX_INPUT_DISPLAY_H */
