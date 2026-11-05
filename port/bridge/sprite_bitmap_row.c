/**
 * sprite_bitmap_row.c - row stride of a sprite bitmap, see sprite_bitmap_row.h.
 */

#include "sprite_bitmap_row.h"

size_t portSpriteBitmapRowTexels(int width, int width_img, int bpp)
{
    size_t line_bits;
    size_t line_words;
    size_t line_texels;

    if ((width <= 0) || (bpp <= 0))
    {
        return (size_t)width_img;
    }
    line_bits = (bpp == 32) ? 16 : (size_t)bpp;
    line_words = (((size_t)width * line_bits) + 63) / 64;
    line_texels = (line_words * 64) / line_bits;

    return ((size_t)width_img > line_texels) ? line_texels : (size_t)width_img;
}
