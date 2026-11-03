/**
 * remix_font_decode.c - see remix_font_decode.h.
 */

#include "remix_font_decode.h"

#define GLYPH_BLOCK_SIZE 0x160
#define SPRITE_OFFSET_OF_FIRST_GLYPH 0x118
#define BITMAP_BEHIND_SPRITE 0x10
#define BITMAP_BUFFER_TOKEN_OFFSET 8
#define BITMAP_WIDTH_OFFSET 0
#define TMEM_GROUP_BYTES 8

static uint32_t readBigEndian32(const uint8_t *bytes, size_t at)
{
    return ((uint32_t)bytes[at] << 24) | ((uint32_t)bytes[at + 1] << 16) | ((uint32_t)bytes[at + 2] << 8) |
           (uint32_t)bytes[at + 3];
}

static int readBigEndian16(const uint8_t *bytes, size_t at)
{
    return (int)(((uint32_t)bytes[at] << 8) | (uint32_t)bytes[at + 1]);
}

/* Index in the stored pixel block of pixel x of row y: odd rows swap the two 4-byte halves of each 8-byte group. */
static size_t storedPixelIndex(int x, int y)
{
    size_t index = (size_t)y * REMIX_FONT_GLYPH_SIZE + (size_t)x;

    if ((y & 1) != 0)
    {
        index ^= (TMEM_GROUP_BYTES / 2);
    }
    return index;
}

int remix_font_decode_glyph(const uint8_t *file, size_t size, int character, uint8_t *rgba)
{
    size_t spriteOffset;
    size_t bitmapOffset;
    size_t pixelOffset;
    int x;
    int y;

    if (file == NULL || rgba == NULL || character < REMIX_FONT_FIRST_CHARACTER || character > REMIX_FONT_LAST_CHARACTER)
    {
        return 0;
    }
    spriteOffset = SPRITE_OFFSET_OF_FIRST_GLYPH + (size_t)GLYPH_BLOCK_SIZE * (size_t)(character - REMIX_FONT_FIRST_CHARACTER);
    bitmapOffset = spriteOffset - BITMAP_BEHIND_SPRITE;
    if (bitmapOffset + BITMAP_BUFFER_TOKEN_OFFSET + 4 > size)
    {
        return 0;
    }
    if (readBigEndian16(file, bitmapOffset + BITMAP_WIDTH_OFFSET) != REMIX_FONT_GLYPH_SIZE)
    {
        return 0;
    }
    pixelOffset = (size_t)(readBigEndian32(file, bitmapOffset + BITMAP_BUFFER_TOKEN_OFFSET) & 0xFFFF) * 4;
    if (pixelOffset + (size_t)REMIX_FONT_GLYPH_SIZE * REMIX_FONT_GLYPH_SIZE > size)
    {
        return 0;
    }
    for (y = 0; y < REMIX_FONT_GLYPH_SIZE; y++)
    {
        for (x = 0; x < REMIX_FONT_GLYPH_SIZE; x++)
        {
            uint8_t texel = file[pixelOffset + storedPixelIndex(x, y)];
            uint8_t *out = rgba + ((size_t)y * REMIX_FONT_GLYPH_SIZE + (size_t)x) * 4;

            out[0] = 0xFF;
            out[1] = 0xFF;
            out[2] = 0xFF;
            out[3] = (uint8_t)((texel & 0x0F) * 17);
        }
    }
    return 1;
}
