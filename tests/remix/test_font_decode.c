/**
 * test_font_decode.c - unit tests of the Remix HUD font decoder (port/remix/remix_font_decode.c) on a synthetic font file.
 */

#include "test_check.h"

#include <string.h>

#include "remix_font_decode.h"

#define TEST_FILE_SIZE 0x4000
#define TEST_FIRST_PIXELS 0x08
#define TEST_BITMAP_OF_FIRST_GLYPH 0x108

static void putBigEndian32(unsigned char *file, size_t at, unsigned value)
{
    file[at] = (unsigned char)(value >> 24);
    file[at + 1] = (unsigned char)(value >> 16);
    file[at + 2] = (unsigned char)(value >> 8);
    file[at + 3] = (unsigned char)value;
}

static void buildFirstGlyph(unsigned char *file)
{
    int index;

    memset(file, 0, TEST_FILE_SIZE);
    file[TEST_BITMAP_OF_FIRST_GLYPH + 1] = REMIX_FONT_GLYPH_SIZE;
    putBigEndian32(file, TEST_BITMAP_OF_FIRST_GLYPH + 8, TEST_FIRST_PIXELS / 4);
    for (index = 0; index < REMIX_FONT_GLYPH_SIZE * REMIX_FONT_GLYPH_SIZE; index++)
    {
        file[TEST_FIRST_PIXELS + index] = (unsigned char)((index & 0x0F) | 0xF0);
    }
}

static void testEvenRowsAreReadInStoredOrder(void)
{
    unsigned char file[TEST_FILE_SIZE];
    unsigned char rgba[REMIX_FONT_GLYPH_RGBA_BYTES];

    buildFirstGlyph(file);
    CHECK_EQ(1, remix_font_decode_glyph(file, sizeof(file), REMIX_FONT_FIRST_CHARACTER, rgba));
    CHECK_EQ(0, rgba[3]);
    CHECK_EQ(3 * 17, rgba[3 * 4 + 3]);
    CHECK_EQ(15 * 17, rgba[15 * 4 + 3]);
}

static void testOddRowsSwapTheFourByteHalves(void)
{
    unsigned char file[TEST_FILE_SIZE];
    unsigned char rgba[REMIX_FONT_GLYPH_RGBA_BYTES];
    const int row = 1;

    buildFirstGlyph(file);
    remix_font_decode_glyph(file, sizeof(file), REMIX_FONT_FIRST_CHARACTER, rgba);
    CHECK_EQ(4 * 17, rgba[(row * REMIX_FONT_GLYPH_SIZE + 0) * 4 + 3]);
    CHECK_EQ(0 * 17, rgba[(row * REMIX_FONT_GLYPH_SIZE + 4) * 4 + 3]);
    CHECK_EQ(12 * 17, rgba[(row * REMIX_FONT_GLYPH_SIZE + 8) * 4 + 3]);
    CHECK_EQ(8 * 17, rgba[(row * REMIX_FONT_GLYPH_SIZE + 12) * 4 + 3]);
}

static void testPixelsAreWhiteSoTheTintCarriesTheColour(void)
{
    unsigned char file[TEST_FILE_SIZE];
    unsigned char rgba[REMIX_FONT_GLYPH_RGBA_BYTES];

    buildFirstGlyph(file);
    remix_font_decode_glyph(file, sizeof(file), REMIX_FONT_FIRST_CHARACTER, rgba);
    CHECK_EQ(0xFF, rgba[8 * 4]);
    CHECK_EQ(0xFF, rgba[8 * 4 + 1]);
    CHECK_EQ(0xFF, rgba[8 * 4 + 2]);
}

static void testRejectsCharactersAndFilesOutOfRange(void)
{
    unsigned char file[TEST_FILE_SIZE];
    unsigned char rgba[REMIX_FONT_GLYPH_RGBA_BYTES];

    buildFirstGlyph(file);
    CHECK_EQ(0, remix_font_decode_glyph(file, sizeof(file), REMIX_FONT_FIRST_CHARACTER - 1, rgba));
    CHECK_EQ(0, remix_font_decode_glyph(file, sizeof(file), REMIX_FONT_LAST_CHARACTER + 1, rgba));
    CHECK_EQ(0, remix_font_decode_glyph(file, 0x100, REMIX_FONT_FIRST_CHARACTER, rgba));
    CHECK_EQ(0, remix_font_decode_glyph(file, sizeof(file), REMIX_FONT_LAST_CHARACTER, rgba));
    CHECK_EQ(0, remix_font_decode_glyph(NULL, sizeof(file), REMIX_FONT_FIRST_CHARACTER, rgba));
}

void remixTestFontDecode(void)
{
    printf("Remix HUD font decoder\n");
    RUN_TEST(testEvenRowsAreReadInStoredOrder);
    RUN_TEST(testOddRowsSwapTheFourByteHalves);
    RUN_TEST(testPixelsAreWhiteSoTheTintCarriesTheColour);
    RUN_TEST(testRejectsCharactersAndFilesOutOfRange);
}
