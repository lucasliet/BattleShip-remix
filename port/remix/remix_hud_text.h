/**
 * remix_hud_text.h - positioned HUD text of the Remix VS modes (scores, counters, labels) drawn as a port overlay.
 *
 * Remix draws these with its own font (Kabel); the port rebuilds that font from the user's .o2r (remix_font.h) and falls back to
 * the ImGui font only when the font file is missing. The match hooks call
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
#define REMIX_HUD_BAR_SLOTS 16
#define REMIX_HUD_TEXT_LENGTH 48

/**
 * Sets or refreshes one label.
 *
 * @param slot Label index (0..REMIX_HUD_TEXT_SLOTS-1); the caller owns its slots.
 * @param text Text, copied (cut at REMIX_HUD_TEXT_LENGTH - 1 characters).
 * @param x Horizontal centre of the text in N64 screen units.
 * @param y Vertical centre of the text in N64 screen units.
 * @param rgba Colour as 0xRRGGBBAA.
 * @param height Height of the 16 pixel glyph box in N64 screen units; the Remix font scale is height / 16 (14 is Remix's 0.875).
 */
void remix_hud_text_put(int slot, const char *text, float x, float y, uint32_t rgba, float height);

/**
 * Same as remix_hud_text_put with a printf style format (the text is cut at REMIX_HUD_TEXT_LENGTH - 1 characters).
 *
 * @param slot Label index (0..REMIX_HUD_TEXT_SLOTS-1); the caller owns its slots.
 * @param x Horizontal centre of the text in N64 screen units.
 * @param y Vertical centre of the text in N64 screen units.
 * @param rgba Colour as 0xRRGGBBAA.
 * @param height Height of the 16 pixel glyph box in N64 screen units (see remix_hud_text_put).
 * @param format printf format string.
 */
void remix_hud_text_putf(int slot, float x, float y, uint32_t rgba, float height, const char *format, ...);

/**
 * Sets or refreshes one progress bar (the Remix mode meters: King of the Hill, Tug of War). Same life as a label.
 *
 * @param slot Bar index (0..REMIX_HUD_BAR_SLOTS-1).
 * @param x Left edge in N64 screen units.
 * @param y Top edge in N64 screen units.
 * @param width Width of the whole bar in N64 screen units.
 * @param height Height of the bar in N64 screen units.
 * @param fraction Filled part of the bar, 0.0 to 1.0.
 * @param rgba Colour of the filled part as 0xRRGGBBAA.
 * @param flash White flash over the whole bar, 0.0 (none) to 1.0.
 */
void remix_hud_bar_put(int slot, float x, float y, float width, float height, float fraction, uint32_t rgba, float flash);

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
