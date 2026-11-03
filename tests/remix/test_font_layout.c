/**
 * test_font_layout.c - unit tests of the Remix string layout rules (port/remix/remix_font_layout.c).
 */

#include "test_check.h"

#include <string.h>

#include "remix_font_layout.h"

#define TEST_MAX_CHARACTER 0x80
#define TEST_GLYPH_WIDTH 7

static void fillWidths(uint8_t *widths)
{
    int character;

    memset(widths, 0, TEST_MAX_CHARACTER + 1);
    for (character = 0x21; character <= TEST_MAX_CHARACTER; character++)
    {
        widths[character] = TEST_GLYPH_WIDTH;
    }
    widths[' '] = 3;
    widths['!'] = 4;
}

static void testGlyphAdvancesByWidthPlusOneAndSpaceByFour(void)
{
    uint8_t widths[TEST_MAX_CHARACTER + 1];

    fillWidths(widths);
    CHECK_EQ(8, (int)remix_font_layout_advance('A', widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(5, (int)remix_font_layout_advance('!', widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(4, (int)remix_font_layout_advance(' ', widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(6, (int)remix_font_layout_advance('A', widths, TEST_MAX_CHARACTER, 0.75f));
    CHECK_EQ(4, (int)remix_font_layout_advance(0x01, widths, TEST_MAX_CHARACTER, 1.0f));
}

static void testAlignedWidthSumsEveryAdvance(void)
{
    uint8_t widths[TEST_MAX_CHARACTER + 1];

    fillWidths(widths);
    CHECK_EQ(28, (int)remix_font_layout_aligned_width("AB A", widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(0, (int)remix_font_layout_aligned_width("", widths, TEST_MAX_CHARACTER, 1.0f));
}

static void testFitWidthEndsAtTheLastGlyphSpriteEdge(void)
{
    uint8_t widths[TEST_MAX_CHARACTER + 1];

    fillWidths(widths);
    CHECK_EQ(24, (int)remix_font_layout_fit_width("AB", widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(16, (int)remix_font_layout_fit_width("A", widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(12, (int)remix_font_layout_fit_width("AB", widths, TEST_MAX_CHARACTER, 0.5f));
}

static void testFitWidthIgnoresLeadingAndTrailingSpaces(void)
{
    uint8_t widths[TEST_MAX_CHARACTER + 1];

    fillWidths(widths);
    CHECK_EQ(24, (int)remix_font_layout_fit_width("  AB  ", widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(28, (int)remix_font_layout_fit_width("A B", widths, TEST_MAX_CHARACTER, 1.0f));
    CHECK_EQ(0, (int)remix_font_layout_fit_width("   ", widths, TEST_MAX_CHARACTER, 1.0f));
}

static void testFitFactorSqueezesOnlyWhenTooWide(void)
{
    uint8_t widths[TEST_MAX_CHARACTER + 1];

    fillWidths(widths);
    CHECK_EQ(50, (int)(100.0f * remix_font_layout_fit_factor("AB", widths, TEST_MAX_CHARACTER, 1.0f, 12.0f)));
    CHECK_EQ(100, (int)(100.0f * remix_font_layout_fit_factor("AB", widths, TEST_MAX_CHARACTER, 1.0f, 24.0f)));
    CHECK_EQ(100, (int)(100.0f * remix_font_layout_fit_factor("AB", widths, TEST_MAX_CHARACTER, 1.0f, 40.0f)));
    CHECK_EQ(100, (int)(100.0f * remix_font_layout_fit_factor("AB", widths, TEST_MAX_CHARACTER, 1.0f, 0.0f)));
    CHECK_EQ(100, (int)(100.0f * remix_font_layout_fit_factor("   ", widths, TEST_MAX_CHARACTER, 1.0f, 5.0f)));
}

void remixTestFontLayout(void)
{
    printf("font layout\n");
    RUN_TEST(testGlyphAdvancesByWidthPlusOneAndSpaceByFour);
    RUN_TEST(testAlignedWidthSumsEveryAdvance);
    RUN_TEST(testFitWidthEndsAtTheLastGlyphSpriteEdge);
    RUN_TEST(testFitWidthIgnoresLeadingAndTrailingSpaces);
    RUN_TEST(testFitFactorSqueezesOnlyWhenTooWide);
}
