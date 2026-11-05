/**
 * test_sprite_bitmap_row.c - unit tests of the sprite bitmap row stride (port/bridge/sprite_bitmap_row.c).
 */

#include "test_check.h"

#include "bridge/sprite_bitmap_row.h"

static void testVanillaRowsKeepTheirImageWidth(void)
{
    CHECK_EQ(64, (int)portSpriteBitmapRowTexels(62, 64, 16));
    CHECK_EQ(16, (int)portSpriteBitmapRowTexels(16, 16, 16));
    CHECK_EQ(16, (int)portSpriteBitmapRowTexels(8, 16, 4));
    CHECK_EQ(8, (int)portSpriteBitmapRowTexels(6, 8, 32));
    CHECK_EQ(24, (int)portSpriteBitmapRowTexels(20, 24, 8));
}

static void testRemixBookendRowIsTheRenderLine(void)
{
    CHECK_EQ(16, (int)portSpriteBitmapRowTexels(16, 96, 16));
}

static void testImageWiderThanTheLineIsCutToTheLine(void)
{
    CHECK_EQ(8, (int)portSpriteBitmapRowTexels(6, 40, 32));
    CHECK_EQ(32, (int)portSpriteBitmapRowTexels(20, 64, 4));
    CHECK_EQ(24, (int)portSpriteBitmapRowTexels(17, 96, 8));
}

static void testNarrowImageWidthIsKept(void)
{
    CHECK_EQ(10, (int)portSpriteBitmapRowTexels(10, 10, 4));
}

static void testUnknownWidthKeepsTheImageWidth(void)
{
    CHECK_EQ(96, (int)portSpriteBitmapRowTexels(0, 96, 16));
}

void remixTestSpriteBitmapRow(void)
{
    RUN_TEST(testVanillaRowsKeepTheirImageWidth);
    RUN_TEST(testRemixBookendRowIsTheRenderLine);
    RUN_TEST(testImageWiderThanTheLineIsCutToTheLine);
    RUN_TEST(testNarrowImageWidthIsKept);
    RUN_TEST(testUnknownWidthKeepsTheImageWidth);
}
