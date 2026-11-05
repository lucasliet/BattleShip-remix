/**
 * test_sprite_rules.c - unit tests of the Remix extension sprite table (if/ifremixspriterules.c).
 */

#include "test_check.h"

#include "if/ifremixspriterules.h"

static void testButtonOffsetsMatchTheAsmConstants(void)
{
    CHECK_EQ(0x0958, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_A)->sprite_offset);
    CHECK_EQ(0x0A88, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_B)->sprite_offset);
    CHECK_EQ(0x18C8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_L)->sprite_offset);
    CHECK_EQ(0x0CF8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_R)->sprite_offset);
    CHECK_EQ(0x0BD8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_Z)->sprite_offset);
    CHECK_EQ(0x04D8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_PLUS)->sprite_offset);
    CHECK_EQ(0x1B10, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_BUTTON_L_THIN)->sprite_offset);
}

static void testButtonsAndLegendWordLiveInFileC5(void)
{
    int id;

    for (id = IFREMIX_SPRITE_BUTTON_A; id <= IFREMIX_SPRITE_LEGEND_WORD; id++)
    {
        CHECK_EQ(0xC5, (int)ifRemixSpriteRulesGetEntry(id)->file_id);
    }
    CHECK_EQ(0x19F8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_LEGEND_WORD)->sprite_offset);
}

static void testDpadAndCStickArrowsLiveInTheCssImagesFile(void)
{
    int id;

    for (id = IFREMIX_SPRITE_DPAD; id <= IFREMIX_SPRITE_C_ARROW_D; id++)
    {
        CHECK_EQ(0xA06, (int)ifRemixSpriteRulesGetEntry(id)->file_id);
    }
    CHECK_EQ(0x218, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_DPAD)->sprite_offset);
    CHECK_EQ(0x1BB8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_C_ARROW_D)->sprite_offset);
}

static void testKothMeterFrameLivesInTheKothGfxFile(void)
{
    CHECK_EQ(0x14F3, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_KOTH_METER)->file_id);
    CHECK_EQ(0xAA8, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_KOTH_METER)->sprite_offset);
}

static void testMultimanIconAndCrossFiles(void)
{
    CHECK_EQ(0x19, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_MULTIMAN_POLYGON)->file_id);
    CHECK_EQ(0x80, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_MULTIMAN_POLYGON)->sprite_offset);
    CHECK_EQ(0x24, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_MULTIMAN_CROSS)->file_id);
    CHECK_EQ(0x828, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_MULTIMAN_CROSS)->sprite_offset);
}

static void testStartButtonLivesInTheSoundTestFile(void)
{
    CHECK_EQ(0xC4, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_START_BUTTON)->file_id);
    CHECK_EQ(0x1D50, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_START_BUTTON)->sprite_offset);
}

static void testMenuArrowLivesInTheMainMenuImagesFile(void)
{
    CHECK_EQ(0x00, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_MENU_ARROW)->file_id);
    CHECK_EQ(0xDD90, (int)ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_MENU_ARROW)->sprite_offset);
}

static void testOutOfRangeIdHasNoEntry(void)
{
    CHECK_TRUE(ifRemixSpriteRulesGetEntry(-1) == NULL);
    CHECK_TRUE(ifRemixSpriteRulesGetEntry(IFREMIX_SPRITE_COUNT) == NULL);
}

static void testUnpackColorSplitsTheBytes(void)
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t alpha;

    ifRemixSpriteRulesUnpackColor(0x50A8FFFF, &red, &green, &blue, &alpha);
    CHECK_EQ(0x50, red);
    CHECK_EQ(0xA8, green);
    CHECK_EQ(0xFF, blue);
    CHECK_EQ(0xFF, alpha);
    ifRemixSpriteRulesUnpackColor(0x00300012, &red, &green, &blue, &alpha);
    CHECK_EQ(0x00, red);
    CHECK_EQ(0x30, green);
    CHECK_EQ(0x00, blue);
    CHECK_EQ(0x12, alpha);
}

void remixTestSpriteRules(void)
{
    printf("sprite rules\n");
    RUN_TEST(testButtonOffsetsMatchTheAsmConstants);
    RUN_TEST(testButtonsAndLegendWordLiveInFileC5);
    RUN_TEST(testDpadAndCStickArrowsLiveInTheCssImagesFile);
    RUN_TEST(testKothMeterFrameLivesInTheKothGfxFile);
    RUN_TEST(testMultimanIconAndCrossFiles);
    RUN_TEST(testStartButtonLivesInTheSoundTestFile);
    RUN_TEST(testMenuArrowLivesInTheMainMenuImagesFile);
    RUN_TEST(testOutOfRangeIdHasNoEntry);
    RUN_TEST(testUnpackColorSplitsTheBytes);
}
