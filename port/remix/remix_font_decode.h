/**
 * remix_font_decode.h - decoder of the Smash Remix HUD font ("Kabel", Remix file File.CUSTOM_FONT = 0xA15).
 *
 * The font file holds one 16 x 16 IA8 sprite per character 0x21..0x80. Each glyph block is 0x160 bytes: a Bitmap header, a
 * Sprite header and the 256 pixel bytes, whose odd rows are stored with the two 4-byte halves of every 8-byte group swapped
 * (the N64 TMEM word swap). The decoder reads nothing but the bytes it is given, so it is tested with synthetic data and the
 * port never embeds ROM data: the real file comes from the user's own .o2r.
 */

#ifndef PORT_REMIX_REMIX_FONT_DECODE_H
#define PORT_REMIX_REMIX_FONT_DECODE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REMIX_FONT_FIRST_CHARACTER 0x21
#define REMIX_FONT_LAST_CHARACTER 0x80
#define REMIX_FONT_GLYPH_SIZE 16
#define REMIX_FONT_GLYPH_RGBA_BYTES (REMIX_FONT_GLYPH_SIZE * REMIX_FONT_GLYPH_SIZE * 4)

/**
 * Decodes one glyph to white RGBA8 whose alpha is the glyph coverage.
 *
 * @param file Big-endian contents of the font file (the data after the resource header).
 * @param size Number of bytes in file.
 * @param character Character code, REMIX_FONT_FIRST_CHARACTER..REMIX_FONT_LAST_CHARACTER.
 * @param rgba Output, REMIX_FONT_GLYPH_RGBA_BYTES bytes.
 * @return 1 when the glyph was decoded, 0 when the character or the file layout is out of range (rgba is then untouched).
 */
int remix_font_decode_glyph(const uint8_t *file, size_t size, int character, uint8_t *rgba);

#ifdef __cplusplus
}
#endif

#endif /* PORT_REMIX_REMIX_FONT_DECODE_H */
