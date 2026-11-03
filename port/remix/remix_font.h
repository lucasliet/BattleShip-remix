/**
 * remix_font.h - the Smash Remix HUD font as a port texture atlas, built at run time from the user's own .o2r.
 *
 * Remix draws every HUD string with the "Kabel" bitmap font (File.CUSTOM_FONT). The glyph sprites are decoded from the
 * file already extracted into the .o2r (reloc_misc/file_2581), the advance widths come from the Remix RAM image
 * (Render.character_offsets_custom), and the result is uploaded as one GUI texture. Nothing of the ROM is stored in the
 * repository.
 */

#ifndef PORT_REMIX_REMIX_FONT_H
#define PORT_REMIX_REMIX_FONT_H

#ifdef REGION_REMIX

#include <imgui.h>

#include <string>

namespace RemixFont {

/** Height in pixels of one glyph sprite at scale 1.0 (FONTSIZE_DEFAULT). */
constexpr float kGlyphHeight = 16.0f;

/**
 * Builds the atlas on first use. Must run on the thread that owns the ImGui renderer.
 *
 * @return true when the font is available; false when the font file or the RAM image is missing (callers then fall back).
 */
bool Ensure();

/**
 * Width of a string as Render.draw_string_ lays it out.
 *
 * @param text Text to measure.
 * @param scale Font scale (1.0 is a 16 pixel glyph).
 * @return Width in N64 screen units (one font pixel is one unit at scale 1.0).
 */
float Measure(const std::string &text, float scale);

/**
 * Draws a string with its top-left corner at position.
 *
 * @param drawList Draw list receiving the glyph quads.
 * @param position Top-left corner in window pixels.
 * @param pixelsPerUnit Window pixels per N64 screen unit, horizontally and vertically.
 * @param scale Remix font scale (0.875 for the 12-Character Battle numbers).
 * @param color ImGui colour used as the tint.
 * @param text Text to draw.
 * @param widthFactor Horizontal squeeze applied to glyphs and advances (Render.apply_max_width_ flattens only the X axis).
 */
void Draw(ImDrawList *drawList, ImVec2 position, ImVec2 pixelsPerUnit, float scale, ImU32 color, const std::string &text,
          float widthFactor);

} // namespace RemixFont

#endif

#endif
