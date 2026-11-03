/**
 * remix_hud_text.h - positioned HUD text of the Remix VS modes (scores, counters, labels) drawn as a port overlay.
 *
 * Remix draws these with its own font (Kabel); the port rebuilds that font from the user's .o2r (remix_font.h) and falls back to
 * the ImGui font only when the font file is missing. Callers pass the exact arguments of the asm Render.draw_string_ call
 * (X, Y of the glyph box top, scale, alignment). The match hooks call
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

/*
 * Text slot map (each owner #defines its slots in its own file). Ranges may be reused only by owners that never draw in
 * the same scene; a label lives two frames, so a reused slot just changes owner when the scene changes.
 *   0..2    results screen: Match Stats prompt and page hints (mnremixvsstatsview.c)
 *   19..23  battle pause: R / Legend and the camera legend (ifremixpauselegend.c), VS and 1P battles
 *   20..23  results screen: KOTH/Smashketball score per port (gmremixgameendrows.c), 30: its row label
 *   25..27  Training: reset count value/label and the item name (sc1ptrainingremix.c)
 *   28      1P Multi-Man: KO counter (sc1premixmultiman.c)
 *   29..30  1P Home-Run Contest: distance and "FT." (sc1premixhrcrun.c)
 *   32..43  12-Character Battle, CSS and results screen: stocks remaining, pool, best character, TKOs, game number
 *           (gmremixtwelvecbhud.c); 44..46 12CB Character Set title and the set name of each player,
 *           47..48 12CB stock number of the panels (mnplayersvs.c)
 *   48..127 results screen: Match Stats page texts, taken in draw order (mnremixvsstatspages.c)
 * Rect slots: 1..47 Match Stats pages (the dimmed background is a game display list object, mnremixvsstatsscene.c). Bar slots: KOTH bars from 0 (gmremixkothhud.c).
 */
#define REMIX_HUD_TEXT_SLOTS 128
#define REMIX_HUD_BAR_SLOTS 16
#define REMIX_HUD_RECT_SLOTS 48
#define REMIX_HUD_TEXT_LENGTH 64

/* Render.LEFT, Render.RIGHT, Render.CENTER: the anchor X is the left edge, the right edge or the middle of the string. */
#define REMIX_HUD_ALIGN_LEFT 0
#define REMIX_HUD_ALIGN_RIGHT 1
#define REMIX_HUD_ALIGN_CENTER 2

/**
 * Sets or refreshes one label with the parameters of Render.draw_string_.
 *
 * @param slot Label index (0..REMIX_HUD_TEXT_SLOTS-1); the caller owns its slots.
 * @param text Text, copied (cut at REMIX_HUD_TEXT_LENGTH - 1 characters).
 * @param x Anchor X in N64 screen units; which part of the string it names depends on align.
 * @param y Top of the 16 pixel glyph box in N64 screen units (the asm Y argument).
 * @param rgba Colour as 0xRRGGBBAA.
 * @param scale Font scale (1.0 is a 16 pixel glyph, 0.8125 and 0.875 are common in the menus).
 * @param align REMIX_HUD_ALIGN_LEFT, REMIX_HUD_ALIGN_RIGHT or REMIX_HUD_ALIGN_CENTER.
 */
void remix_hud_text_put(int slot, const char *text, float x, float y, uint32_t rgba, float scale, int align);

/**
 * Same as remix_hud_text_put for a string that must fit a width: when it is wider than max_width it is squeezed horizontally to
 * max_width, the way Render.apply_max_width_ does (the glyph height is kept).
 *
 * @param slot Label index (0..REMIX_HUD_TEXT_SLOTS-1); the caller owns its slots.
 * @param text Text, copied.
 * @param x Anchor X in N64 screen units.
 * @param y Top of the glyph box in N64 screen units.
 * @param rgba Colour as 0xRRGGBBAA.
 * @param scale Font scale.
 * @param align REMIX_HUD_ALIGN_LEFT, REMIX_HUD_ALIGN_RIGHT or REMIX_HUD_ALIGN_CENTER.
 * @param max_width Widest the string may be in N64 screen units.
 */
void remix_hud_text_put_fit(int slot, const char *text, float x, float y, uint32_t rgba, float scale, int align,
                            float max_width);

/**
 * Same as remix_hud_text_put with a printf style format (the text is cut at REMIX_HUD_TEXT_LENGTH - 1 characters).
 *
 * @param slot Label index (0..REMIX_HUD_TEXT_SLOTS-1); the caller owns its slots.
 * @param x Anchor X in N64 screen units.
 * @param y Top of the glyph box in N64 screen units.
 * @param rgba Colour as 0xRRGGBBAA.
 * @param scale Font scale (see remix_hud_text_put).
 * @param align REMIX_HUD_ALIGN_LEFT, REMIX_HUD_ALIGN_RIGHT or REMIX_HUD_ALIGN_CENTER.
 * @param format printf format string.
 */
void remix_hud_text_putf(int slot, float x, float y, uint32_t rgba, float scale, int align, const char *format, ...);

/**
 * Width a string takes when Render.draw_string_ lays it out.
 *
 * @param text Text to measure.
 * @param scale Font scale.
 * @return Width in N64 screen units, 0 when the Remix font is not available.
 */
float remix_hud_text_measure(const char *text, float scale);

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

/**
 * Sets or refreshes one filled rectangle (Render.draw_rectangle): no outline, alpha blended. Rectangles are drawn under every label.
 *
 * @param slot Rectangle index (0..REMIX_HUD_RECT_SLOTS-1); a higher slot draws over a lower one.
 * @param x Left edge in N64 screen units.
 * @param y Top edge in N64 screen units.
 * @param width Width in N64 screen units.
 * @param height Height in N64 screen units.
 * @param rgba Colour as 0xRRGGBBAA.
 */
void remix_hud_rect_put(int slot, float x, float y, float width, float height, uint32_t rgba);

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
