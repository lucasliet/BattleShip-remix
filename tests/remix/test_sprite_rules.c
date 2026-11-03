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

static void testEveryButtonLivesInFileC5(void)
{
    int id;

    for (id = 0; id < IFREMIX_SPRITE_COUNT; id++)
    {
        CHECK_EQ(0xC5, (int)ifRemixSpriteRulesGetEntry(id)->file_id);
    }
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
    RUN_TEST(testEveryButtonLivesInFileC5);
    RUN_TEST(testOutOfRangeIdHasNoEntry);
    RUN_TEST(testUnpackColorSplitsTheBytes);
}
