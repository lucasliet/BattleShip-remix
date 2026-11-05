/**
 * test_css_rules.c - unit tests of the Remix VS character select rules (mn/mnplayers/mnremixcssrules.c).
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixcssrules.h"
#include "remix/remix_roster_data.h"

#define SLOT_COUNT 30

static void testCostumeButtonsAlwaysWorkWithoutTeams(void)
{
    CHECK_EQ(1, mnRemixCssCostumeButtonsActive(0, 0, 2));
    CHECK_EQ(1, mnRemixCssCostumeButtonsActive(0, 1, -1));
    CHECK_EQ(1, mnRemixCssCostumeButtonsActive(0, 0, -1));
}

static void testCostumeButtonsInTeamsNeedASelectedFighterWithoutAHeldToken(void)
{
    CHECK_EQ(1, mnRemixCssCostumeButtonsActive(1, 1, -1));
    CHECK_EQ(0, mnRemixCssCostumeButtonsActive(1, 0, -1));
    CHECK_EQ(0, mnRemixCssCostumeButtonsActive(1, 1, 0));
    CHECK_EQ(0, mnRemixCssCostumeButtonsActive(1, 1, 3));
    CHECK_EQ(0, mnRemixCssCostumeButtonsActive(1, 0, 2));
}

static void testBonusBookendEdges(void)
{
    CHECK_EQ(1, mnRemixCssBonusBookendContains(24, 44));
    CHECK_EQ(1, mnRemixCssBonusBookendContains(38, 115));
    CHECK_EQ(0, mnRemixCssBonusBookendContains(23, 44));
    CHECK_EQ(0, mnRemixCssBonusBookendContains(39, 44));
    CHECK_EQ(0, mnRemixCssBonusBookendContains(24, 43));
    CHECK_EQ(0, mnRemixCssBonusBookendContains(24, 116));
}

static void testBonusStepWrapsAtBothEnds(void)
{
    CHECK_EQ(1, mnRemixCssBonusStep(0, 1));
    CHECK_EQ(9, mnRemixCssBonusStep(0, -1));
    CHECK_EQ(0, mnRemixCssBonusStep(9, 1));
    CHECK_EQ(8, mnRemixCssBonusStep(9, -1));
    CHECK_EQ(5, mnRemixCssBonusStep(5, 0));
}

static void testBonusHeldStepPrefersZ(void)
{
    CHECK_EQ(-1, mnRemixCssBonusHeldStep(1, 0));
    CHECK_EQ(1, mnRemixCssBonusHeldStep(0, 1));
    CHECK_EQ(-1, mnRemixCssBonusHeldStep(1, 1));
    CHECK_EQ(0, mnRemixCssBonusHeldStep(0, 0));
}

static void testBonusRepeatChangesOnPressThenAfterThirtyFramesEveryEight(void)
{
    int timer = MNREMIX_CSS_BONUS_REPEAT_IDLE;
    int frame;
    int changes[40];

    for (frame = 0; frame < 40; frame++)
    {
        changes[frame] = mnRemixCssBonusRepeat(&timer, 1);
    }
    CHECK_EQ(1, changes[0]);

    for (frame = 1; frame < 30; frame++)
    {
        CHECK_EQ(0, changes[frame]);
    }
    CHECK_EQ(1, changes[30]);
    CHECK_EQ(0, changes[31]);
    CHECK_EQ(0, changes[37]);
    CHECK_EQ(1, changes[38]);
}

static void testBonusRepeatReleaseRearmsTheNextPress(void)
{
    int timer = MNREMIX_CSS_BONUS_REPEAT_IDLE;

    CHECK_EQ(1, mnRemixCssBonusRepeat(&timer, 1));
    CHECK_EQ(0, mnRemixCssBonusRepeat(&timer, 1));
    CHECK_EQ(0, mnRemixCssBonusRepeat(&timer, 0));
    CHECK_EQ(MNREMIX_CSS_BONUS_REPEAT_IDLE, timer);
    CHECK_EQ(1, mnRemixCssBonusRepeat(&timer, 1));
}

static void testRandomPoolGrowsWithTheBonusFightersWhenTheToggleIsNotDefault(void)
{
    CHECK_EQ(SLOT_COUNT, mnRemixCssRandomPoolSize(0, SLOT_COUNT));
    CHECK_EQ(SLOT_COUNT + 10, mnRemixCssRandomPoolSize(1, SLOT_COUNT));
    CHECK_EQ(SLOT_COUNT + 10, mnRemixCssRandomPoolSize(2, SLOT_COUNT));
    CHECK_EQ(SLOT_COUNT + 10, mnRemixCssRandomPoolSize(3, SLOT_COUNT));
}

static void testRandomPoolBonusEntriesStartPastTheGrid(void)
{
    CHECK_EQ(0, mnRemixCssRandomPoolIsBonus(0, SLOT_COUNT));
    CHECK_EQ(0, mnRemixCssRandomPoolIsBonus(29, SLOT_COUNT));
    CHECK_EQ(1, mnRemixCssRandomPoolIsBonus(30, SLOT_COUNT));
    CHECK_EQ(1, mnRemixCssRandomPoolIsBonus(39, SLOT_COUNT));
}

static void testDpadPanelsAreSixtyNineApart(void)
{
    CHECK_EQ(34, mnRemixCssDpadPanelX(0));
    CHECK_EQ(103, mnRemixCssDpadPanelX(1));
    CHECK_EQ(241, mnRemixCssDpadPanelX(3));
}

static void testDpadIconOffsetsPerDirection(void)
{
    int x;
    int y;

    mnRemixCssDpadIconOffset(MNREMIX_CSS_DPAD_UP, &x, &y);
    CHECK_EQ(4, x);
    CHECK_EQ(-9, y);
    mnRemixCssDpadIconOffset(MNREMIX_CSS_DPAD_DOWN, &x, &y);
    CHECK_EQ(4, x);
    CHECK_EQ(14, y);
    mnRemixCssDpadIconOffset(MNREMIX_CSS_DPAD_LEFT, &x, &y);
    CHECK_EQ(-10, x);
    CHECK_EQ(4, y);
    mnRemixCssDpadIconOffset(MNREMIX_CSS_DPAD_RIGHT, &x, &y);
    CHECK_EQ(16, x);
    CHECK_EQ(4, y);
}

static void testDpadMarkOffsetsPerDirection(void)
{
    int x;
    int y;

    mnRemixCssDpadMarkOffset(MNREMIX_CSS_DPAD_UP, &x, &y);
    CHECK_EQ(7, x);
    CHECK_EQ(3, y);
    mnRemixCssDpadMarkOffset(MNREMIX_CSS_DPAD_DOWN, &x, &y);
    CHECK_EQ(7, x);
    CHECK_EQ(11, y);
    mnRemixCssDpadMarkOffset(MNREMIX_CSS_DPAD_LEFT, &x, &y);
    CHECK_EQ(3, x);
    CHECK_EQ(7, y);
    mnRemixCssDpadMarkOffset(MNREMIX_CSS_DPAD_RIGHT, &x, &y);
    CHECK_EQ(11, x);
    CHECK_EQ(7, y);
}

static void testVariantIconByTypeBeforeById(void)
{
    CHECK_EQ(MNREMIX_CSS_ICON_FLAG_J, mnRemixCssVariantIconKind(REMIX_VARIANT_J, REMIX_ID_METAL));
    CHECK_EQ(MNREMIX_CSS_ICON_FLAG_E, mnRemixCssVariantIconKind(REMIX_VARIANT_E, 0x33));
    CHECK_EQ(MNREMIX_CSS_ICON_POLYGON, mnRemixCssVariantIconKind(REMIX_VARIANT_POLYGON, 0x0E));
}

static void testVariantIconByFighterId(void)
{
    CHECK_EQ(MNREMIX_CSS_ICON_METAL, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_METAL));
    CHECK_EQ(MNREMIX_CSS_ICON_MLUIGI, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_MLUIGI));
    CHECK_EQ(MNREMIX_CSS_ICON_MASTER_HAND, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_BOSS));
    CHECK_EQ(MNREMIX_CSS_ICON_PEPPY, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_PEPPY));
    CHECK_EQ(MNREMIX_CSS_ICON_SLIPPY, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_SLIPPY));
    CHECK_EQ(MNREMIX_CSS_ICON_EBI, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_EBI));
    CHECK_EQ(MNREMIX_CSS_ICON_GBOWSER, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_GBOWSER));
    CHECK_EQ(MNREMIX_CSS_ICON_PIANO, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_PIANO));
    CHECK_EQ(MNREMIX_CSS_ICON_SSONIC, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_SSONIC));
    CHECK_EQ(MNREMIX_CSS_ICON_DRAGONKING, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_DRAGONKING));
    CHECK_EQ(MNREMIX_CSS_ICON_DSAMUS, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_DSAMUS));
    CHECK_EQ(MNREMIX_CSS_ICON_LUCAS, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_LUCAS));
    CHECK_EQ(MNREMIX_CSS_ICON_ROY, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_ROY));
}

static void testVariantWithoutAnIconUsesTheDonkeyKongStockIcon(void)
{
    CHECK_EQ(MNREMIX_CSS_ICON_DONKEY_STOCK, mnRemixCssVariantIconKind(REMIX_VARIANT_SPECIAL, REMIX_ID_GDONKEY));
}

static void testVariantIconScales(void)
{
    CHECK_EQ(1, mnRemixCssVariantIconScale(MNREMIX_CSS_ICON_FLAG_J) == 0.625F);
    CHECK_EQ(1, mnRemixCssVariantIconScale(MNREMIX_CSS_ICON_FLAG_E) == 0.625F);
    CHECK_EQ(1, mnRemixCssVariantIconScale(MNREMIX_CSS_ICON_MASTER_HAND) == 0.5625F);
    CHECK_EQ(1, mnRemixCssVariantIconScale(MNREMIX_CSS_ICON_METAL) == 1.0F);
    CHECK_EQ(1, mnRemixCssVariantIconScale(MNREMIX_CSS_ICON_POLYGON) == 1.0F);
    CHECK_EQ(1, mnRemixCssVariantIconScale(MNREMIX_CSS_ICON_DONKEY_STOCK) == 1.0F);
}

static void testVariantIconOffsetsPointAtTheSpriteHeaders(void)
{
    CHECK_EQ(0x558, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_FLAG_J));
    CHECK_EQ(0x3B8, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_FLAG_E));
    CHECK_EQ(0x13A8, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_POLYGON));
    CHECK_EQ(0x2458, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_METAL));
    CHECK_EQ(0x2800, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_MASTER_HAND));
    CHECK_EQ(0x2A98, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_MLUIGI));
    CHECK_EQ(0x2ED0, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_ROY));
    CHECK_EQ(0, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_DONKEY_STOCK));
    CHECK_EQ(0, mnRemixCssVariantIconOffset(MNREMIX_CSS_ICON_NONE));
}

static void testHandicapIconSourcesFollowTheTable(void)
{
    CHECK_EQ(REMIX_ID_YOSHI, mnRemixCssHandicapIconSource(0));
    CHECK_EQ(REMIX_ID_YOSHI, mnRemixCssHandicapIconSource(4));
    CHECK_EQ(REMIX_ID_KIRBY, mnRemixCssHandicapIconSource(5));
    CHECK_EQ(REMIX_ID_KIRBY, mnRemixCssHandicapIconSource(9));
    CHECK_EQ(MNREMIX_CSS_HANDICAP_SOURCE_POLYGON, mnRemixCssHandicapIconSource(10));
    CHECK_EQ(REMIX_ID_DONKEY, mnRemixCssHandicapIconSource(15));
    CHECK_EQ(MNREMIX_CSS_HANDICAP_SOURCE_METAL, mnRemixCssHandicapIconSource(20));
    CHECK_EQ(MNREMIX_CSS_HANDICAP_SOURCE_MASTER_HAND, mnRemixCssHandicapIconSource(29));
    CHECK_EQ(REMIX_ID_SAMUS, mnRemixCssHandicapIconSource(30));
    CHECK_EQ(MNREMIX_CSS_HANDICAP_SOURCE_NONE, mnRemixCssHandicapIconSource(31));
    CHECK_EQ(MNREMIX_CSS_HANDICAP_SOURCE_NONE, mnRemixCssHandicapIconSource(-1));
}

static void testHandicapIconLevelsRunOneToFivePerSource(void)
{
    CHECK_EQ(1, mnRemixCssHandicapIconLevel(0));
    CHECK_EQ(5, mnRemixCssHandicapIconLevel(4));
    CHECK_EQ(1, mnRemixCssHandicapIconLevel(5));
    CHECK_EQ(3, mnRemixCssHandicapIconLevel(27));
    CHECK_EQ(5, mnRemixCssHandicapIconLevel(29));
    CHECK_EQ(0, mnRemixCssHandicapIconLevel(30));
    CHECK_EQ(0, mnRemixCssHandicapIconLevel(31));
}

static void testHandicapLevelColors(void)
{
    CHECK_EQ(0x416FE4FF, mnRemixCssHandicapLevelColor(1));
    CHECK_EQ(0xE44141FF, mnRemixCssHandicapLevelColor(5));
    CHECK_EQ(0, mnRemixCssHandicapLevelColor(0));
    CHECK_EQ(0, mnRemixCssHandicapLevelColor(6));
}

static void testHasVariantsNeedsOnlyOneVariant(void)
{
    const int none = REMIX_ID_NONE;
    const int mario[4] = { REMIX_ID_METAL, 0x0E, 0x2A, none };
    const int fox_without_special[4] = { none, 0x0F, 0x29, none };
    const int only_right[4] = { none, none, none, 0x33 };
    const int nothing[4] = { none, none, none, none };

    CHECK_EQ(1, mnRemixCssHasVariants(mario, none));
    CHECK_EQ(1, mnRemixCssHasVariants(fox_without_special, none));
    CHECK_EQ(1, mnRemixCssHasVariants(only_right, none));
    CHECK_EQ(0, mnRemixCssHasVariants(nothing, none));
}

void remixTestCssRules(void)
{
    RUN_TEST(testCostumeButtonsAlwaysWorkWithoutTeams);
    RUN_TEST(testCostumeButtonsInTeamsNeedASelectedFighterWithoutAHeldToken);
    RUN_TEST(testBonusBookendEdges);
    RUN_TEST(testBonusStepWrapsAtBothEnds);
    RUN_TEST(testBonusHeldStepPrefersZ);
    RUN_TEST(testBonusRepeatChangesOnPressThenAfterThirtyFramesEveryEight);
    RUN_TEST(testBonusRepeatReleaseRearmsTheNextPress);
    RUN_TEST(testRandomPoolGrowsWithTheBonusFightersWhenTheToggleIsNotDefault);
    RUN_TEST(testRandomPoolBonusEntriesStartPastTheGrid);
    RUN_TEST(testDpadPanelsAreSixtyNineApart);
    RUN_TEST(testDpadIconOffsetsPerDirection);
    RUN_TEST(testDpadMarkOffsetsPerDirection);
    RUN_TEST(testVariantIconByTypeBeforeById);
    RUN_TEST(testVariantIconByFighterId);
    RUN_TEST(testVariantWithoutAnIconUsesTheDonkeyKongStockIcon);
    RUN_TEST(testVariantIconScales);
    RUN_TEST(testVariantIconOffsetsPointAtTheSpriteHeaders);
    RUN_TEST(testHandicapIconSourcesFollowTheTable);
    RUN_TEST(testHandicapIconLevelsRunOneToFivePerSource);
    RUN_TEST(testHandicapLevelColors);
    RUN_TEST(testHasVariantsNeedsOnlyOneVariant);
}
