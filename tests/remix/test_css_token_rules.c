/**
 * test_css_token_rules.c - unit tests of the Remix character select token rules (mn/mnplayers/mnremixcsstokenrules.c) and of the
 * stock picker ranges (mnremixstockrangerules.c). Expected values come from CharacterSelect.asm place_token_from_id_,
 * token_autoposition_, adjust_scaled_token_position_ and TwelveCharBattle.asm handle_stock_count_change_.
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixbonusmodesrules.h"
#include "mn/mnplayers/mnremixcssafkrules.h"
#include "mn/mnplayers/mnremixcssrules.h"
#include "mn/mnplayers/mnremixcssunplugrules.h"
#include "mn/mnplayers/mnremixcsstokenrules.h"
#include "mn/mnplayers/mnremixstockrangerules.h"
#include "mn/mnvsmode/mnremixvsstaminarules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

#define DEFAULT_STOCK_MAX 98

static void testGridTokenRestsAtStartXPlus12AndStartYPlus14(void)
{
    float pos_x = 0.0F;
    float pos_y = 0.0F;

    mnRemixCssTokenRestPosition(0, &pos_x, &pos_y);
    CHECK_EQ(41, pos_x);
    CHECK_EQ(48, pos_y);

    mnRemixCssTokenRestPosition(13, &pos_x, &pos_y);
    CHECK_EQ(41 + (24 * 3), pos_x);
    CHECK_EQ(48 + 24, pos_y);

    mnRemixCssTokenRestPosition(29, &pos_x, &pos_y);
    CHECK_EQ(41 + (24 * 9), pos_x);
    CHECK_EQ(48 + (24 * 2), pos_y);
}

static void testBookendTokensRestAtTheirFixedSpots(void)
{
    float pos_x = 0.0F;
    float pos_y = 0.0F;

    mnRemixCssTokenRestPosition(MNREMIX_CSS_TOKEN_BONUS_SLOT, &pos_x, &pos_y);
    CHECK_EQ(21, pos_x);
    CHECK_EQ(70, pos_y);

    mnRemixCssTokenRestPosition(MNREMIX_CSS_TOKEN_RANDOM_SLOT, &pos_x, &pos_y);
    CHECK_EQ(278, pos_x);
    CHECK_EQ(70, pos_y);
}

static void testSlotOutOfRangeReadsAsTheFirstCell(void)
{
    float pos_x = 0.0F;
    float pos_y = 0.0F;

    mnRemixCssTokenRestPosition(-1, &pos_x, &pos_y);
    CHECK_EQ(41, pos_x);
    CHECK_EQ(48, pos_y);

    mnRemixCssTokenRestPosition(32, &pos_x, &pos_y);
    CHECK_EQ(41, pos_x);
    CHECK_EQ(48, pos_y);
}

static void testGridEdgeBoxIsA24PixelCellFromStartXPlus12(void)
{
    float edge_x, edge_y, width, height;

    mnRemixCssTokenEdgeBox(0, &edge_x, &edge_y, &width, &height);
    CHECK_EQ(41, edge_x);
    CHECK_EQ(46, edge_y);
    CHECK_EQ(24, width);
    CHECK_EQ(24, height);

    mnRemixCssTokenEdgeBox(24, &edge_x, &edge_y, &width, &height);
    CHECK_EQ(41 + (24 * 4), edge_x);
    CHECK_EQ(46 + (24 * 2), edge_y);
}

static void testBookendEdgeBoxesAre15By72(void)
{
    float edge_x, edge_y, width, height;

    mnRemixCssTokenEdgeBox(MNREMIX_CSS_TOKEN_BONUS_SLOT, &edge_x, &edge_y, &width, &height);
    CHECK_EQ(24, edge_x);
    CHECK_EQ(46, edge_y);
    CHECK_EQ(15, width);
    CHECK_EQ(72, height);

    mnRemixCssTokenEdgeBox(MNREMIX_CSS_TOKEN_RANDOM_SLOT, &edge_x, &edge_y, &width, &height);
    CHECK_EQ(279, edge_x);
    CHECK_EQ(46, edge_y);
    CHECK_EQ(15, width);
    CHECK_EQ(72, height);
}

static void testHeldTokenFollowsTheHandAt12AndMinus10(void)
{
    CHECK_EQ(12, MNREMIX_CSS_TOKEN_FOLLOW_X);
    CHECK_EQ(-10, MNREMIX_CSS_TOKEN_FOLLOW_Y);
}

static void testCursorReachesY212AndOverlapRadiusIs10(void)
{
    CHECK_EQ(212, MNREMIX_CSS_CURSOR_MAX_Y);
    CHECK_EQ(10, MNREMIX_CSS_TOKEN_OVERLAP_RADIUS);
}

static void testDefaultStockRangeStepsAndWrapsBetween1And99(void)
{
    CHECK_EQ(1, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_DEFAULT, 0, DEFAULT_STOCK_MAX));
    CHECK_EQ(0, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_DEFAULT, DEFAULT_STOCK_MAX, DEFAULT_STOCK_MAX));
    CHECK_EQ(DEFAULT_STOCK_MAX, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_DEFAULT, 0, DEFAULT_STOCK_MAX));
    CHECK_EQ(2, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_DEFAULT, 3, DEFAULT_STOCK_MAX));
}

static void testDefaultStockRangeHonoursAScreenLimit(void)
{
    CHECK_EQ(0, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_DEFAULT, 4, 4));
    CHECK_EQ(4, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_DEFAULT, 0, 4));
}

static void testTagTeamStockRangeIs2To6Stocks(void)
{
    CHECK_EQ(2, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TAG_TEAM, 1, DEFAULT_STOCK_MAX));
    CHECK_EQ(5, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TAG_TEAM, 4, DEFAULT_STOCK_MAX));
    CHECK_EQ(1, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TAG_TEAM, 5, DEFAULT_STOCK_MAX));
    CHECK_EQ(5, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_TAG_TEAM, 1, DEFAULT_STOCK_MAX));
    CHECK_EQ(1, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_TAG_TEAM, 2, DEFAULT_STOCK_MAX));
}

static void testTagTeamStockRangePullsOutOfRangeValuesIn(void)
{
    CHECK_EQ(5, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_TAG_TEAM, 0, DEFAULT_STOCK_MAX));
    CHECK_EQ(1, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TAG_TEAM, 98, DEFAULT_STOCK_MAX));
}

static void testTugOfWarStockRangeIs1To25Stocks(void)
{
    CHECK_EQ(1, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TUG_OF_WAR, 0, DEFAULT_STOCK_MAX));
    CHECK_EQ(24, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TUG_OF_WAR, 23, DEFAULT_STOCK_MAX));
    CHECK_EQ(0, mnRemixStockRangeNext(MNREMIX_STOCK_RANGE_TUG_OF_WAR, 24, DEFAULT_STOCK_MAX));
    CHECK_EQ(24, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_TUG_OF_WAR, 0, DEFAULT_STOCK_MAX));
    CHECK_EQ(0, mnRemixStockRangePrev(MNREMIX_STOCK_RANGE_TUG_OF_WAR, 1, DEFAULT_STOCK_MAX));
}

static void testMasterHandVariantNeedsTheTwentiethTagNamed(void)
{
    CHECK_EQ(0, mnRemixCssVariantIsPickable(28, 28, 12, 1));
    CHECK_EQ(0, mnRemixCssVariantIsPickable(12, 28, 12, 0));
    CHECK_EQ(1, mnRemixCssVariantIsPickable(12, 28, 12, 1));
    CHECK_EQ(1, mnRemixCssVariantIsPickable(5, 28, 12, 0));
}

static void testStaminaMenuButtonsStepOneAndWrap(void)
{
    CHECK_EQ(51, mnRemixVSStaminaRulesMenuStep(50, 1, 0));
    CHECK_EQ(49, mnRemixVSStaminaRulesMenuStep(50, 0, 0));
    CHECK_EQ(1, mnRemixVSStaminaRulesMenuStep(999, 1, 0));
    CHECK_EQ(999, mnRemixVSStaminaRulesMenuStep(1, 0, 0));
}

static void testStaminaMenuStickStepsTenWithSnaps(void)
{
    CHECK_EQ(10, mnRemixVSStaminaRulesMenuStep(1, 1, 1));
    CHECK_EQ(10, mnRemixVSStaminaRulesMenuStep(9, 1, 1));
    CHECK_EQ(20, mnRemixVSStaminaRulesMenuStep(10, 1, 1));
    CHECK_EQ(999, mnRemixVSStaminaRulesMenuStep(989, 1, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesMenuStep(990, 1, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesMenuStep(999, 1, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesMenuStep(10, 0, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesMenuStep(2, 0, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesMenuStep(11, 0, 1));
    CHECK_EQ(90, mnRemixVSStaminaRulesMenuStep(100, 0, 1));
    CHECK_EQ(980, mnRemixVSStaminaRulesMenuStep(990, 0, 1));
    CHECK_EQ(990, mnRemixVSStaminaRulesMenuStep(991, 0, 1));
    CHECK_EQ(990, mnRemixVSStaminaRulesMenuStep(999, 0, 1));
    CHECK_EQ(999, mnRemixVSStaminaRulesMenuStep(1, 0, 1));
}

static void testStaminaCssStepsTenAndSnapsToTheTens(void)
{
    CHECK_EQ(10, mnRemixVSStaminaRulesCssStep(1, 1));
    CHECK_EQ(20, mnRemixVSStaminaRulesCssStep(10, 1));
    CHECK_EQ(210, mnRemixVSStaminaRulesCssStep(200, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesCssStep(10, 0));
    CHECK_EQ(10, mnRemixVSStaminaRulesCssStep(20, 0));
    CHECK_EQ(190, mnRemixVSStaminaRulesCssStep(200, 0));
}

static void testStaminaCssSnapsNearTheTopAndWrapsAtBothEnds(void)
{
    CHECK_EQ(990, mnRemixVSStaminaRulesCssStep(979, 1));
    CHECK_EQ(990, mnRemixVSStaminaRulesCssStep(980, 1));
    CHECK_EQ(999, mnRemixVSStaminaRulesCssStep(990, 1));
    CHECK_EQ(990, mnRemixVSStaminaRulesCssStep(999, 0));
    CHECK_EQ(1, mnRemixVSStaminaRulesCssStep(999, 1));
    CHECK_EQ(1, mnRemixVSStaminaRulesCssStep(995, 1));
    CHECK_EQ(999, mnRemixVSStaminaRulesCssStep(5, 0));
    CHECK_EQ(999, mnRemixVSStaminaRulesCssStep(1, 0));
}

static void testAfkTimerCountsOnlyWhileReadyAfterTheFirstSecond(void)
{
    CHECK_EQ(0, mnRemixCssAfkRulesNextTimer(50, 0, 200));
    CHECK_EQ(0, mnRemixCssAfkRulesNextTimer(50, 1, 60));
    CHECK_EQ(51, mnRemixCssAfkRulesNextTimer(50, 1, 61));
}

static void testAfkPressesStartAfter120ReadyFramesWithNoButtonHeld(void)
{
    CHECK_EQ(0, mnRemixCssAfkRulesShouldStart(119, 0));
    CHECK_EQ(1, mnRemixCssAfkRulesShouldStart(120, 0));
    CHECK_EQ(0, mnRemixCssAfkRulesShouldStart(300, 1));
}

static void testOnePlayerGameHeaderAndVoiceFollowTheRemixMode(void)
{
    CHECK_EQ(0x3758, mnRemixBonusModesGameHeaderOffset(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0x39C8, mnRemixBonusModesGameHeaderOffset(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(0, mnRemixBonusModesGameHeaderOffset(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(1014, mnRemixBonusModesGameAnnounceFgm(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(0, mnRemixBonusModesGameAnnounceFgm(SC1PREMIX_MODE_REMIX_1P));
}

static void testBonusVoiceFollowsTheRemixMode(void)
{
    CHECK_EQ(0x1EF, mnRemixBonusModesBonusAnnounceFgm(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(0x3C7, mnRemixBonusModesBonusAnnounceFgm(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(0x3C8, mnRemixBonusModesBonusAnnounceFgm(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(1015, mnRemixBonusModesBonusAnnounceFgm(SC1PREMIX_MODE_HRC));
    CHECK_EQ(0, mnRemixBonusModesBonusAnnounceFgm(SC1PREMIX_MODE_STANDARD));
}

static void testBonusHeaderCyclesBonus1Bonus2Bonus3(void)
{
    int mode, kind;

    CHECK_EQ(1, mnRemixBonusModesCycleHeader(SC1PREMIX_MODE_STANDARD, 0, &mode, &kind));
    CHECK_EQ(SC1PREMIX_MODE_STANDARD, mode);
    CHECK_EQ(1, kind);

    CHECK_EQ(1, mnRemixBonusModesCycleHeader(SC1PREMIX_MODE_STANDARD, 1, &mode, &kind));
    CHECK_EQ(SC1PREMIX_MODE_BONUS3, mode);
    CHECK_EQ(1, kind);

    CHECK_EQ(1, mnRemixBonusModesCycleHeader(SC1PREMIX_MODE_BONUS3, 1, &mode, &kind));
    CHECK_EQ(SC1PREMIX_MODE_STANDARD, mode);
    CHECK_EQ(0, kind);
}

static void testBonusHeaderTogglesMultiManAndCruelAndSkipsHrc(void)
{
    int mode, kind;

    CHECK_EQ(1, mnRemixBonusModesCycleHeader(SC1PREMIX_MODE_MULTIMAN, 1, &mode, &kind));
    CHECK_EQ(SC1PREMIX_MODE_CRUEL, mode);
    CHECK_EQ(1, kind);

    CHECK_EQ(1, mnRemixBonusModesCycleHeader(SC1PREMIX_MODE_CRUEL, 1, &mode, &kind));
    CHECK_EQ(SC1PREMIX_MODE_MULTIMAN, mode);

    CHECK_EQ(0, mnRemixBonusModesCycleHeader(SC1PREMIX_MODE_HRC, 1, &mode, &kind));
    CHECK_EQ(SC1PREMIX_MODE_HRC, mode);
    CHECK_EQ(1, kind);
}

static void testUnpluggedPlayerBacksOutWithBFromAnyPort(void)
{
    const unsigned int taps[4] = { 0, 0x4000, 0, 0 };
    const unsigned int no_taps[4] = { 0, 0, 0, 0 };

    CHECK_EQ(1, mnRemixCssUnplugRulesBackTapped(0, 0, taps, 4, 0x4000));
    CHECK_EQ(0, mnRemixCssUnplugRulesBackTapped(0, 0, no_taps, 4, 0x4000));
    CHECK_EQ(0, mnRemixCssUnplugRulesBackTapped(1, 0, taps, 4, 0x4000));
    CHECK_EQ(1, mnRemixCssUnplugRulesBackTapped(1, 0x4000, taps, 4, 0x4000));
}

void remixTestCssTokenRules(void)
{
    RUN_TEST(testGridTokenRestsAtStartXPlus12AndStartYPlus14);
    RUN_TEST(testBookendTokensRestAtTheirFixedSpots);
    RUN_TEST(testSlotOutOfRangeReadsAsTheFirstCell);
    RUN_TEST(testGridEdgeBoxIsA24PixelCellFromStartXPlus12);
    RUN_TEST(testBookendEdgeBoxesAre15By72);
    RUN_TEST(testHeldTokenFollowsTheHandAt12AndMinus10);
    RUN_TEST(testCursorReachesY212AndOverlapRadiusIs10);
    RUN_TEST(testDefaultStockRangeStepsAndWrapsBetween1And99);
    RUN_TEST(testDefaultStockRangeHonoursAScreenLimit);
    RUN_TEST(testTagTeamStockRangeIs2To6Stocks);
    RUN_TEST(testTagTeamStockRangePullsOutOfRangeValuesIn);
    RUN_TEST(testTugOfWarStockRangeIs1To25Stocks);
    RUN_TEST(testMasterHandVariantNeedsTheTwentiethTagNamed);
    RUN_TEST(testUnpluggedPlayerBacksOutWithBFromAnyPort);
    RUN_TEST(testOnePlayerGameHeaderAndVoiceFollowTheRemixMode);
    RUN_TEST(testBonusVoiceFollowsTheRemixMode);
    RUN_TEST(testBonusHeaderCyclesBonus1Bonus2Bonus3);
    RUN_TEST(testBonusHeaderTogglesMultiManAndCruelAndSkipsHrc);
    RUN_TEST(testAfkTimerCountsOnlyWhileReadyAfterTheFirstSecond);
    RUN_TEST(testAfkPressesStartAfter120ReadyFramesWithNoButtonHeld);
    RUN_TEST(testStaminaMenuButtonsStepOneAndWrap);
    RUN_TEST(testStaminaMenuStickStepsTenWithSnaps);
    RUN_TEST(testStaminaCssStepsTenAndSnapsToTheTens);
    RUN_TEST(testStaminaCssSnapsNearTheTopAndWrapsAtBothEnds);
}
