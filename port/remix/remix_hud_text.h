/**
 * remix_hud_text.h - positioned HUD text of the Remix VS modes (scores, counters, labels) drawn as a port overlay.
 *
 * Remix draws these with its own font sprites; the port draws them with ImGui. The match hooks call
 * remix_hud_text_put every frame for every label they want visible: a label is shown for the frame it was put in and the
 * next one, so a hook that stops calling it hides the label. Positions are in the N64 screen (320 x 240), the overlay
 * maps them to the 4:3 game image centred in the window.
 */

#ifndef PORT_REMIX_REMIX_HUD_TEXT_H
#define PORT_REMIX_REMIX_HUD_TEXT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REMIX_HUD_TEXT_SLOTS 32
#define REMIX_HUD_TEXT_LENGTH 48

/**
 * Sets or refreshes one label.
 *
 * @param slot Label index (0..REMIX_HUD_TEXT_SLOTS-1); the caller owns its slots.
 * @param text Text, copied (cut at REMIX_HUD_TEXT_LENGTH - 1 characters).
 * @param x Horizontal centre of the text in N64 screen units.
 * @param y Vertical centre of the text in N64 screen units.
 * @param rgba Colour as 0xRRGGBBAA.
 * @param height Height of the text in N64 screen units (the 240-unit screen height scales it).
 */
void remix_hud_text_put(int slot, const char *text, float x, float y, uint32_t rgba, float height);

/** Hides one label right away. */
void remix_hud_text_hide(int slot);

/** Hides every label. Called at each scene change so no label outlives its match. */
void remix_hud_text_clear(void);

#ifdef __cplusplus
}

namespace Ship {
class GuiWindow;
}

#include <memory>

/** Creates the overlay window that draws the labels. The port adds it to the Gui once at start-up. */
std::shared_ptr<Ship::GuiWindow> remix_hud_text_make_window();
#endif

#endif /* PORT_REMIX_REMIX_HUD_TEXT_H */
