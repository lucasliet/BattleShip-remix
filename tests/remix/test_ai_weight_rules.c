/**
 * test_ai_weight_rules.c - unit tests of the Remix lv10 attack option weighting rules (ft/ftremix/ftremixaiweightrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaiweightrules.h"

static void testLandingNeedsAFloorBelowAndAirborne(void)
{
    CHECK_EQ(0, ftRemixAiWeightRulesCanLandOn(-1, 1));
    CHECK_EQ(0, ftRemixAiWeightRulesCanLandOn(3, 0));
    CHECK_TRUE(ftRemixAiWeightRulesCanLandOn(3, 1));
}

static void testFallSpeedIsCappedAtMaxFall(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesCapFallSpeed(-5.0F, -4.0F) == -4.0F);
    CHECK_TRUE(ftRemixAiWeightRulesCapFallSpeed(-2.0F, -4.0F) == -2.0F);
}

static void testFighterLandsBeforeTheMoveComesOut(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesLandsBeforeMove(10, -5.0F, -1.0F, 0.0F, -4.0F));
}

static void testFighterFarAboveGroundDoesNotLand(void)
{
    CHECK_EQ(0, ftRemixAiWeightRulesLandsBeforeMove(3, -100.0F, -1.0F, 0.0F, -4.0F));
}

static void testHitstunStaysWhenNoLandingInside(void)
{
    CHECK_EQ(12, ftRemixAiWeightRulesHitstunLeft(5, 12, -100.0F, -1.0F, 0.0F, -4.0F));
}

static void testHitstunBecomesFramesUntilLandingMinusOne(void)
{
    CHECK_EQ(1, ftRemixAiWeightRulesHitstunLeft(10, 20, -2.5F, -1.0F, 0.0F, -4.0F));
}

static void testTechActionRangeIsDamageFlyHighToTumble(void)
{
    CHECK_EQ(0, ftRemixAiWeightRulesIsTechAction(0x32));
    CHECK_TRUE(ftRemixAiWeightRulesIsTechAction(0x33));
    CHECK_TRUE(ftRemixAiWeightRulesIsTechAction(0x39));
    CHECK_EQ(0, ftRemixAiWeightRulesIsTechAction(0x3A));
}

static void testComboNeedsHitstun(void)
{
    CHECK_EQ(0, ftRemixAiWeightRulesIsCombo(5, 0, 20, 0x00, 0));
}

static void testComboWhenMoveHitsInsideHitstunWindow(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesIsCombo(10, 1, 8, 0x00, 0));
    CHECK_EQ(0, ftRemixAiWeightRulesIsCombo(11, 1, 8, 0x00, 0));
}

static void testShieldStunComboUsesShieldstunFrames(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesIsCombo(12, 1, 0, 0x9B, 12));
    CHECK_EQ(0, ftRemixAiWeightRulesIsCombo(13, 1, 0, 0x9B, 12));
}

static void testComboRollNeedsASpanOfTwoOrMore(void)
{
    CHECK_EQ(0, ftRemixAiWeightRulesComboNeedsRoll(1));
    CHECK_TRUE(ftRemixAiWeightRulesComboNeedsRoll(2));
    CHECK_TRUE(ftRemixAiWeightRulesComboRollPasses(0));
    CHECK_EQ(0, ftRemixAiWeightRulesComboRollPasses(3));
}

static void testComboChanceIsMultipliedByAThousand(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesComboChance(0.5F) == 500.0F);
}

static void testEarlyNotComboMoveAlwaysKept(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesNotComboKeeps(14, 0.99F));
}

static void testSlowMoveThresholdsByStartFrame(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesNotComboKeeps(15, 0.1F));
    CHECK_EQ(0, ftRemixAiWeightRulesNotComboKeeps(15, 0.2F));
    CHECK_TRUE(ftRemixAiWeightRulesNotComboKeeps(20, 0.03F));
    CHECK_EQ(0, ftRemixAiWeightRulesNotComboKeeps(20, 0.04F));
    CHECK_TRUE(ftRemixAiWeightRulesNotComboKeeps(30, 0.01F));
    CHECK_EQ(0, ftRemixAiWeightRulesNotComboKeeps(30, 0.02F));
}

static void testOffstageOpponentIsAdvantage(void)
{
    CHECK_EQ(FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE, ftRemixAiWeightRulesSlowMoveKind(0x00, -1, 0));
}

static void testStunnedOpponentIsAdvantage(void)
{
    CHECK_EQ(FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE, ftRemixAiWeightRulesSlowMoveKind(0xA4, 2, 0));
}

static void testTechingOpponentIsSmallAdvantage(void)
{
    CHECK_EQ(FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE, ftRemixAiWeightRulesSlowMoveKind(0x49, 2, 0));
}

static void testPercentDecidesTheRestOfTheStates(void)
{
    CHECK_EQ(FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE, ftRemixAiWeightRulesSlowMoveKind(0x00, 2, 150));
    CHECK_EQ(FTREMIX_AI_WEIGHT_SLOW_NORMAL, ftRemixAiWeightRulesSlowMoveKind(0x00, 2, 151));
    CHECK_EQ(FTREMIX_AI_WEIGHT_SLOW_NORMAL, ftRemixAiWeightRulesSlowMoveKind(0x00, 2, 90));
}

static void testSlowMoveChanceFormulas(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesSlowMoveChance(2.0F, 4, FTREMIX_AI_WEIGHT_SLOW_NORMAL) == 0.125F);
    CHECK_TRUE(ftRemixAiWeightRulesSlowMoveChance(2.0F, 4, FTREMIX_AI_WEIGHT_SLOW_SMALL_ADVANTAGE) == 8.0F);
    CHECK_TRUE(ftRemixAiWeightRulesSlowMoveChance(2.0F, 4, FTREMIX_AI_WEIGHT_SLOW_ADVANTAGE) == 32.0F);
}

static void testGrabAgainstShieldIsMultiplied(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesGrabVsShieldChance(0.5F, 28, 0x99) == 500.0F);
    CHECK_TRUE(ftRemixAiWeightRulesGrabVsShieldChance(0.5F, 0x45, 0x9B) == 500.0F);
}

static void testNonGrabOrOpenOpponentKeepsTheChance(void)
{
    CHECK_TRUE(ftRemixAiWeightRulesGrabVsShieldChance(0.5F, 28, 0x00) == 0.5F);
    CHECK_TRUE(ftRemixAiWeightRulesGrabVsShieldChance(0.5F, 0x02, 0x99) == 0.5F);
}

static void testTruncationSaturatesOutOfRange(void)
{
    CHECK_EQ(7, ftRemixAiWeightRulesTruncFrames(7.9F));
    CHECK_EQ(0x7FFFFFFF, ftRemixAiWeightRulesTruncFrames(3.0e9F));
}

void remixTestAiWeightRules(void)
{
    printf("ai weight rules\n");
    RUN_TEST(testLandingNeedsAFloorBelowAndAirborne);
    RUN_TEST(testFallSpeedIsCappedAtMaxFall);
    RUN_TEST(testFighterLandsBeforeTheMoveComesOut);
    RUN_TEST(testFighterFarAboveGroundDoesNotLand);
    RUN_TEST(testHitstunStaysWhenNoLandingInside);
    RUN_TEST(testHitstunBecomesFramesUntilLandingMinusOne);
    RUN_TEST(testTechActionRangeIsDamageFlyHighToTumble);
    RUN_TEST(testComboNeedsHitstun);
    RUN_TEST(testComboWhenMoveHitsInsideHitstunWindow);
    RUN_TEST(testShieldStunComboUsesShieldstunFrames);
    RUN_TEST(testComboRollNeedsASpanOfTwoOrMore);
    RUN_TEST(testComboChanceIsMultipliedByAThousand);
    RUN_TEST(testEarlyNotComboMoveAlwaysKept);
    RUN_TEST(testSlowMoveThresholdsByStartFrame);
    RUN_TEST(testOffstageOpponentIsAdvantage);
    RUN_TEST(testStunnedOpponentIsAdvantage);
    RUN_TEST(testTechingOpponentIsSmallAdvantage);
    RUN_TEST(testPercentDecidesTheRestOfTheStates);
    RUN_TEST(testSlowMoveChanceFormulas);
    RUN_TEST(testGrabAgainstShieldIsMultiplied);
    RUN_TEST(testNonGrabOrOpenOpponentKeepsTheChance);
    RUN_TEST(testTruncationSaturatesOutOfRange);
}
