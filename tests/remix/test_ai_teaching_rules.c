/**
 * test_ai_teaching_rules.c - unit tests of the Remix AI teching and Z-cancel rules
 * (ft/ftremix/ftremixaiteachingrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaiteachingrules.h"

static void testImprovedAiUsesRollForAdvancedCpu(void)
{
    CHECK_TRUE(ftRemixAiTechUsesImprovedRoll(0, 1, 0, 0, 0));
}

static void testTrainingAlwaysUsesRoll(void)
{
    CHECK_TRUE(ftRemixAiTechUsesImprovedRoll(1, 0, 1, 0, 0));
}

static void testVanillaCpuWithoutToggleKeepsVanillaTech(void)
{
    CHECK_EQ(0, ftRemixAiTechUsesImprovedRoll(0, 0, 0, 0, 0));
}

static void testClassicModeKeepsVanillaTechEvenWithToggle(void)
{
    CHECK_EQ(0, ftRemixAiTechUsesImprovedRoll(0, 0, 1, 1, 0));
}

static void testImprovedToggleUsesRollForLowLevelCpu(void)
{
    CHECK_TRUE(ftRemixAiTechUsesImprovedRoll(0, 0, 0, 1, 0));
}

static void testExcludedScreenKeepsVanillaTechForAdvancedCpu(void)
{
    CHECK_EQ(0, ftRemixAiTechUsesImprovedRoll(0, 1, 0, 0, 1));
}

static void testSingleButtonOffAllowsTech(void)
{
    CHECK_TRUE(ftRemixAiTechSingleButtonAllows(0));
}

static void testSingleButtonRAllowsTech(void)
{
    CHECK_TRUE(ftRemixAiTechSingleButtonAllows(3));
}

static void testSingleButtonRPlusCAllowsTech(void)
{
    CHECK_TRUE(ftRemixAiTechSingleButtonAllows(6));
}

static void testSingleButtonAAndBBlockTech(void)
{
    CHECK_EQ(0, ftRemixAiTechSingleButtonAllows(1));
    CHECK_EQ(0, ftRemixAiTechSingleButtonAllows(2));
}

static void testSingleButtonCVariantsOfAAndBBlockTech(void)
{
    CHECK_EQ(0, ftRemixAiTechSingleButtonAllows(4));
    CHECK_EQ(0, ftRemixAiTechSingleButtonAllows(5));
}

static void testForwardRollBelowThirty(void)
{
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_FORWARD, ftRemixAiTechRollOutcome(29, 0));
}

static void testBackwardRollFromThirty(void)
{
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_BACKWARD, ftRemixAiTechRollOutcome(30, 0));
}

static void testInPlaceRollFromSixty(void)
{
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_IN_PLACE, ftRemixAiTechRollOutcome(60, 0));
}

static void testFailRollFromNinety(void)
{
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_FAIL, ftRemixAiTechRollOutcome(90, 0));
}

static void testOffStageNeverRollsForwardOrBackward(void)
{
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_IN_PLACE, ftRemixAiTechRollOutcome(0, 1));
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_IN_PLACE, ftRemixAiTechRollOutcome(89, 1));
}

static void testOffStageFailsFromNinety(void)
{
    CHECK_EQ(FTREMIX_AI_TECH_OUTCOME_FAIL, ftRemixAiTechRollOutcome(90, 1));
}

static void testZCancelGateOpenOnImprovedCpu(void)
{
    CHECK_TRUE(ftRemixAiZCancelGateOpen(1, 0, 1, 1, 0));
}

static void testZCancelGateClosedForHuman(void)
{
    CHECK_EQ(0, ftRemixAiZCancelGateOpen(1, 0, 1, 0, 0));
}

static void testZCancelGateClosedWithoutImprovedPath(void)
{
    CHECK_EQ(0, ftRemixAiZCancelGateOpen(0, 0, 1, 1, 0));
}

static void testZCancelGateClosedOnExcludedScreen(void)
{
    CHECK_EQ(0, ftRemixAiZCancelGateOpen(1, 1, 1, 1, 0));
}

static void testZCancelGateClosedByButtonMode(void)
{
    CHECK_EQ(0, ftRemixAiZCancelGateOpen(1, 0, 0, 1, 0));
}

static void testZCancelGateClosedInClassicMode(void)
{
    CHECK_EQ(0, ftRemixAiZCancelGateOpen(1, 0, 1, 1, 1));
}

static void testZCancelRollBelowNinetyFiveSucceeds(void)
{
    CHECK_TRUE(ftRemixAiZCancelRollSucceeds(94));
}

static void testZCancelRollFromNinetyFiveFails(void)
{
    CHECK_EQ(0, ftRemixAiZCancelRollSucceeds(95));
}

void remixTestAiTeachingRules(void)
{
    printf("ai teaching rules\n");
    RUN_TEST(testImprovedAiUsesRollForAdvancedCpu);
    RUN_TEST(testTrainingAlwaysUsesRoll);
    RUN_TEST(testVanillaCpuWithoutToggleKeepsVanillaTech);
    RUN_TEST(testClassicModeKeepsVanillaTechEvenWithToggle);
    RUN_TEST(testImprovedToggleUsesRollForLowLevelCpu);
    RUN_TEST(testExcludedScreenKeepsVanillaTechForAdvancedCpu);
    RUN_TEST(testSingleButtonOffAllowsTech);
    RUN_TEST(testSingleButtonRAllowsTech);
    RUN_TEST(testSingleButtonRPlusCAllowsTech);
    RUN_TEST(testSingleButtonAAndBBlockTech);
    RUN_TEST(testSingleButtonCVariantsOfAAndBBlockTech);
    RUN_TEST(testForwardRollBelowThirty);
    RUN_TEST(testBackwardRollFromThirty);
    RUN_TEST(testInPlaceRollFromSixty);
    RUN_TEST(testFailRollFromNinety);
    RUN_TEST(testOffStageNeverRollsForwardOrBackward);
    RUN_TEST(testOffStageFailsFromNinety);
    RUN_TEST(testZCancelGateOpenOnImprovedCpu);
    RUN_TEST(testZCancelGateClosedForHuman);
    RUN_TEST(testZCancelGateClosedWithoutImprovedPath);
    RUN_TEST(testZCancelGateClosedOnExcludedScreen);
    RUN_TEST(testZCancelGateClosedByButtonMode);
    RUN_TEST(testZCancelGateClosedInClassicMode);
    RUN_TEST(testZCancelRollBelowNinetyFiveSucceeds);
    RUN_TEST(testZCancelRollFromNinetyFiveFails);
}
