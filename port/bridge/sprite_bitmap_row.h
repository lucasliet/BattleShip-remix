#ifndef PORT_BRIDGE_SPRITE_BITMAP_ROW_H
#define PORT_BRIDGE_SPRITE_BITMAP_ROW_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Texels per DRAM row of a sprite bitmap.
 *
 * lbCommonDrawSObjBitmap loads the texels with one LoadBlock and samples them through a render tile whose line is
 * `width` texels rounded up to 8 bytes (G_IM_SIZ_*_LINE_BYTES), so that line is the row stride on N64 and width_img
 * only sizes the LoadBlock. Vanilla sprites have width_img equal to that line; the Smash Remix CSS bookends (file 0xA06,
 * 16x96) carry width_img 96 (docs/bugs/remix_css_bookend_width_img_2026-11-04.md).
 *
 * @param width Bitmap width drawn, in texels (Bitmap.width).
 * @param width_img Bitmap image width, in texels (Bitmap.width_img), positive.
 * @param bpp Bits per texel: 4, 8, 16 or 32.
 * @return The row stride in texels: width_img, or the render tile line when width_img is wider than it.
 */
size_t portSpriteBitmapRowTexels(int width, int width_img, int bpp);

#ifdef __cplusplus
}
#endif

#endif
