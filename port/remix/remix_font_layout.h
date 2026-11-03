/**
 * remix_font_layout.h - pure layout rules of the Smash Remix string drawing (Render.asm draw_string_ and apply_max_width_),
 * separate from the ImGui drawing so the headless unit tests can check them.
 *
 * A glyph advances by (table width + 1) * scale and a space by 4 * scale; the sprite of a glyph is always 16 units wide.
 */

#ifndef PORT_REMIX_REMIX_FONT_LAYOUT_H
#define PORT_REMIX_REMIX_FONT_LAYOUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Width of the glyph sprite in font units; apply_max_width_ adds it to the X of the last glyph (the image width field). */
#define REMIX_FONT_LAYOUT_SPRITE_WIDTH 16
/** Advance of a space in font units (_space branch of draw_string_). */
#define REMIX_FONT_LAYOUT_SPACE_ADVANCE 4

/**
 * Horizontal advance of one character.
 *
 * @param character Character code.
 * @param widths Table width of every character code 0..max_character (the first word byte of Render.character_offsets).
 * @param max_character Last character code that has an entry in widths.
 * @param scale Font scale.
 * @return Advance in N64 screen units; a space and a character without a table entry advance by the space width.
 */
float remix_font_layout_advance(int character, const uint8_t *widths, int max_character, float scale);

/**
 * Width the alignment of draw_string_ uses (its _length_loop): the sum of the advances of every character.
 */
float remix_font_layout_aligned_width(const char *text, const uint8_t *widths, int max_character, float scale);

/**
 * Width apply_max_width_ compares with the maximum: from the X of the first glyph to the right edge of the last glyph
 * sprite. Spaces create no glyph, so leading and trailing spaces do not count. Zero when the text has no glyph.
 */
float remix_font_layout_fit_width(const char *text, const uint8_t *widths, int max_character, float scale);

/**
 * Horizontal squeeze apply_max_width_ applies: max_width / width when the text is wider than max_width, else 1.
 */
float remix_font_layout_fit_factor(const char *text, const uint8_t *widths, int max_character, float scale, float max_width);

#ifdef __cplusplus
}
#endif

#endif
