/**
 * test_ai_postprocess_rules.c - unit tests of the per-fighter Remix AI post process rules
 * (ft/ftremix/ftremixaipostrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaipostrules.h"

static void testActionSetsMatchTheAsmIds(void)
{
    CHECK_TRUE(ftRemixAiPostIsWhirlingFortress(0xDE));
    CHECK_EQ(0, ftRemixAiPostIsWhirlingFortress(0xDF));
    CHECK_TRUE(ftRemixAiPostIsDragonKingUsp(0xEF));
    CHECK_TRUE(ftRemixAiPostIsDragonKingUsp(0xF1));
    CHECK_EQ(0, ftRemixAiPostIsDragonKingUsp(0xF0));
    CHECK_TRUE(ftRemixAiPostIsDiveGrab(0xEB));
    CHECK_TRUE(ftRemixAiPostIsDiveGrab(0xEE));
    CHECK_EQ(0, ftRemixAiPostIsDiveGrab(0xEC));
    CHECK_TRUE(ftRemixAiPostIsWolfFlashStart(0xE3));
    CHECK_TRUE(ftRemixAiPostIsWolfFlashStart(0xE4));
    CHECK_EQ(0, ftRemixAiPostIsWolfFlashStart(0xE5));
    CHECK_TRUE(ftRemixAiPostIsDededeCharge(0xF9));
    CHECK_TRUE(ftRemixAiPostIsDededeCharge(0xFC));
    CHECK_EQ(0, ftRemixAiPostIsDededeCharge(0xFA));
    CHECK_TRUE(ftRemixAiPostIsMarinaPull(0xE2));
    CHECK_TRUE(ftRemixAiPostIsMarinaPull(0xE7));
    CHECK_TRUE(ftRemixAiPostIsMarinaCargo(0xEB));
    CHECK_TRUE(ftRemixAiPostIsMarinaCargo(0xF1));
}

static void testFoxFamilyUspCoversStartTravelAndAir(void)
{
    CHECK_EQ(0, ftRemixAiPostIsFoxFamilyUsp(0xE2));
    CHECK_TRUE(ftRemixAiPostIsFoxFamilyUsp(0xE3));
    CHECK_TRUE(ftRemixAiPostIsFoxFamilyUsp(0xE8));
    CHECK_EQ(0, ftRemixAiPostIsFoxFamilyUsp(0xE9));
}

static void testQuickAttackCoversTheSixPikachuActions(void)
{
    CHECK_EQ(0, ftRemixAiPostIsQuickAttack(0xE7));
    CHECK_TRUE(ftRemixAiPostIsQuickAttack(0xE8));
    CHECK_TRUE(ftRemixAiPostIsQuickAttack(0xED));
    CHECK_EQ(0, ftRemixAiPostIsQuickAttack(0xEE));
}

static void testDkActions(void)
{
    CHECK_TRUE(ftRemixAiPostIsHandSlap(0xE8));
    CHECK_TRUE(ftRemixAiPostIsHandSlap(0xEA));
    CHECK_EQ(0, ftRemixAiPostIsHandSlap(0xEB));
    CHECK_TRUE(ftRemixAiPostIsSpinningKong(0xE6));
    CHECK_TRUE(ftRemixAiPostIsSpinningKong(0xE7));
    CHECK_EQ(0, ftRemixAiPostIsSpinningKong(0xE8));
}

static void testKirbyStoneActions(void)
{
    CHECK_EQ(0, ftRemixAiPostIsKirbyStone(0x105));
    CHECK_TRUE(ftRemixAiPostIsKirbyStone(0x106));
    CHECK_EQ(0, ftRemixAiPostIsKirbyStone(0x107));
    CHECK_TRUE(ftRemixAiPostIsKirbyStone(0x109));
    CHECK_TRUE(ftRemixAiPostIsKirbyStone(0x10A));
    CHECK_TRUE(ftRemixAiPostIsKirbyStone(0x10B));
    CHECK_EQ(0, ftRemixAiPostIsKirbyStone(0x10C));
}

static void testDiveTurnFacingTargetKeepsPointing(void)
{
    CHECK_EQ(0, ftRemixAiPostDiveTurnStickX(100.0F, 1));
    CHECK_EQ(0, ftRemixAiPostDiveTurnStickX(-100.0F, -1));
}

static void testDiveTurnAwayFromTargetHoldsTowardIt(void)
{
    CHECK_EQ(FTREMIX_AI_POST_STICK_MAX, ftRemixAiPostDiveTurnStickX(100.0F, -1));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MIN, ftRemixAiPostDiveTurnStickX(-100.0F, 1));
}

static void testDiveTurnTargetAtSameXIsTreatedAsRight(void)
{
    CHECK_EQ(0, ftRemixAiPostDiveTurnStickX(0.0F, 1));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MAX, ftRemixAiPostDiveTurnStickX(0.0F, -1));
}

static void testLandingProbeGoesTowardTheTarget(void)
{
    CHECK_TRUE(ftRemixAiPostLandingProbeOffset(0.0F, 500.0F) == 2000.0F);
    CHECK_TRUE(ftRemixAiPostLandingProbeOffset(500.0F, 0.0F) == -2000.0F);
}

static void testLandingProbeSameXGoesLeft(void)
{
    CHECK_TRUE(ftRemixAiPostLandingProbeOffset(300.0F, 300.0F) == -2000.0F);
}

static void testWolfStickYBands(void)
{
    CHECK_EQ(FTREMIX_AI_POST_STICK_MAX, ftRemixAiPostWolfStickY(1096.0F));
    CHECK_EQ(0, ftRemixAiPostWolfStickY(1095.0F));
    CHECK_EQ(0, ftRemixAiPostWolfStickY(700.0F));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MIN, ftRemixAiPostWolfStickY(699.0F));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MIN, ftRemixAiPostWolfStickY(-300.0F));
}

static void testDkProbeFollowsTheSpeedSign(void)
{
    CHECK_TRUE(ftRemixAiPostDkProbeOffset(3.0F) == 400.0F);
    CHECK_TRUE(ftRemixAiPostDkProbeOffset(-3.0F) == -400.0F);
    CHECK_TRUE(ftRemixAiPostDkProbeOffset(0.0F) == 0.0F);
}

static void testDkHoldsAgainstTheSpeed(void)
{
    CHECK_EQ(FTREMIX_AI_POST_STICK_MIN, ftRemixAiPostDkHoldStickX(3.0F));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MAX, ftRemixAiPostDkHoldStickX(-3.0F));
}

static void testDededeHoldsBFromEightHundred(void)
{
    CHECK_TRUE(ftRemixAiPostDededeHoldsB(800.0F));
    CHECK_EQ(0, ftRemixAiPostDededeHoldsB(799.0F));
}

static void testMewtwoSkipsDspWhenTargetCannotBeGrabbed(void)
{
    CHECK_TRUE(ftRemixAiPostMewtwoSkipsDsp(1, 1, 0, 0x0A));
    CHECK_TRUE(ftRemixAiPostMewtwoSkipsDsp(1, -1, 1, 0x0A));
    CHECK_TRUE(ftRemixAiPostMewtwoSkipsDsp(1, -1, 0, 0xA4));
    CHECK_TRUE(ftRemixAiPostMewtwoSkipsDsp(1, -1, 0, 0xA5));
    CHECK_EQ(0, ftRemixAiPostMewtwoSkipsDsp(1, -1, 0, 0x0A));
}

static void testPuffSkipsDspAgainstShieldOrLowDamage(void)
{
    CHECK_TRUE(ftRemixAiPostPuffSkipsDsp(0x98, 80));
    CHECK_TRUE(ftRemixAiPostPuffSkipsDsp(0x99, 80));
    CHECK_TRUE(ftRemixAiPostPuffSkipsDsp(0x9B, 80));
    CHECK_TRUE(ftRemixAiPostPuffSkipsDsp(0x0A, 29));
    CHECK_EQ(0, ftRemixAiPostPuffSkipsDsp(0x0A, 30));
}

static void testYoshiSkipsNspAgainstEgg(void)
{
    CHECK_TRUE(ftRemixAiPostYoshiSkipsNsp(0xB1));
    CHECK_TRUE(ftRemixAiPostYoshiSkipsNsp(0xB2));
    CHECK_EQ(0, ftRemixAiPostYoshiSkipsNsp(0xB3));
}

static void testKirbyStoneCancelConditions(void)
{
    CHECK_TRUE(ftRemixAiPostKirbyCancelsStone(0, 0, 0));
    CHECK_TRUE(ftRemixAiPostKirbyCancelsStone(1, 1, 0));
    CHECK_TRUE(ftRemixAiPostKirbyCancelsStone(1, 0, 1));
    CHECK_EQ(0, ftRemixAiPostKirbyCancelsStone(1, 0, 0));
}

static void testKirbyMashPressesBOnOddFramesOnly(void)
{
    CHECK_EQ(0x4001, ftRemixAiPostKirbyMashButtons(0x0001, 3));
    CHECK_EQ(0, ftRemixAiPostKirbyMashButtons(0x0001, 4));
}

static void testSwordNspActionRange(void)
{
    CHECK_EQ(0, ftRemixAiPostIsSwordNsp(0xDF));
    CHECK_TRUE(ftRemixAiPostIsSwordNsp(0xE0));
    CHECK_TRUE(ftRemixAiPostIsSwordNsp(0xE3));
    CHECK_EQ(0, ftRemixAiPostIsSwordNsp(0xE4));
}

static void testSwordNspStepTwoRepeatsItsHeight(void)
{
    CHECK_EQ(FTREMIX_AI_POST_INPUT_USP, ftRemixAiPostSwordNspInput(0xE1, 0x0A, 50, 0));
    CHECK_EQ(FTREMIX_AI_POST_INPUT_NSP, ftRemixAiPostSwordNspInput(0xE2, 0x0A, 50, 0));
    CHECK_EQ(FTREMIX_AI_POST_INPUT_DSP, ftRemixAiPostSwordNspInput(0xE3, 0x0A, 50, 0));
}

static void testSwordNspAgainstShieldGoesLow(void)
{
    CHECK_EQ(FTREMIX_AI_POST_INPUT_DSP, ftRemixAiPostSwordNspInput(0xE0, 0x99, 150, 0));
    CHECK_EQ(FTREMIX_AI_POST_INPUT_DSP, ftRemixAiPostSwordNspInput(0xE0, 0x9B, 150, 0));
}

static void testSwordNspShieldOnDoesNotCount(void)
{
    CHECK_EQ(FTREMIX_AI_POST_INPUT_NSP, ftRemixAiPostSwordNspInput(0xE0, 0x98, 150, 0));
}

static void testSwordNspByDamage(void)
{
    CHECK_EQ(FTREMIX_AI_POST_INPUT_DSP, ftRemixAiPostSwordNspInput(0xE0, 0x0A, 29, 0));
    CHECK_EQ(FTREMIX_AI_POST_INPUT_NSP, ftRemixAiPostSwordNspInput(0xE0, 0x0A, 101, 2));
}

static void testSwordNspRollBetweenThirtyAndOneHundred(void)
{
    CHECK_EQ(FTREMIX_AI_POST_INPUT_NSP, ftRemixAiPostSwordNspInput(0xE0, 0x0A, 30, 0));
    CHECK_EQ(FTREMIX_AI_POST_INPUT_USP, ftRemixAiPostSwordNspInput(0xE0, 0x0A, 100, 1));
    CHECK_EQ(FTREMIX_AI_POST_INPUT_DSP, ftRemixAiPostSwordNspInput(0xE0, 0x0A, 100, 2));
}

static void testSwordNspDrawsTheRollOnlyInTheRandomBlock(void)
{
    CHECK_TRUE(ftRemixAiPostSwordNspNeedsRoll(0xE0, 0x0A, 30));
    CHECK_TRUE(ftRemixAiPostSwordNspNeedsRoll(0xE0, 0x0A, 100));
    CHECK_EQ(0, ftRemixAiPostSwordNspNeedsRoll(0xE0, 0x0A, 29));
    CHECK_EQ(0, ftRemixAiPostSwordNspNeedsRoll(0xE0, 0x0A, 101));
    CHECK_EQ(0, ftRemixAiPostSwordNspNeedsRoll(0xE1, 0x0A, 50));
    CHECK_EQ(0, ftRemixAiPostSwordNspNeedsRoll(0xE0, 0x99, 50));
    CHECK_EQ(0, ftRemixAiPostSwordNspNeedsRoll(0xE0, 0x9B, 50));
}

static void testMarinaSnatchChoice(void)
{
    CHECK_EQ(FTREMIX_AI_POST_MARINA_SNATCH_NONE, ftRemixAiPostMarinaSnatchChoice(0));
    CHECK_EQ(FTREMIX_AI_POST_MARINA_SNATCH_NONE, ftRemixAiPostMarinaSnatchChoice(2));
    CHECK_EQ(FTREMIX_AI_POST_MARINA_SNATCH_BACK, ftRemixAiPostMarinaSnatchChoice(3));
    CHECK_EQ(FTREMIX_AI_POST_MARINA_SNATCH_BACK, ftRemixAiPostMarinaSnatchChoice(4));
    CHECK_EQ(FTREMIX_AI_POST_MARINA_SNATCH_UP, ftRemixAiPostMarinaSnatchChoice(5));
    CHECK_EQ(FTREMIX_AI_POST_MARINA_SNATCH_UP, ftRemixAiPostMarinaSnatchChoice(9));
}

static void testMarinaBackThrowGoesAwayFromTarget(void)
{
    CHECK_EQ(FTREMIX_AI_POST_STICK_MIN, ftRemixAiPostMarinaBackStickX(0.0F, 100.0F));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MAX, ftRemixAiPostMarinaBackStickX(100.0F, 0.0F));
    CHECK_EQ(FTREMIX_AI_POST_STICK_MAX, ftRemixAiPostMarinaBackStickX(50.0F, 50.0F));
}

static void testMarinaCargoShakesFromSixteen(void)
{
    CHECK_TRUE(ftRemixAiPostMarinaCargoShakes(0x10));
    CHECK_EQ(0, ftRemixAiPostMarinaCargoShakes(0x0F));
}

static void testMarinaDspDistanceAndOdds(void)
{
    CHECK_TRUE(ftRemixAiPostMarinaDspInDistance(1000.0F));
    CHECK_EQ(0, ftRemixAiPostMarinaDspInDistance(999.0F));
    CHECK_EQ(500, ftRemixAiPostMarinaDspRollRange(1));
    CHECK_EQ(125, ftRemixAiPostMarinaDspRollRange(4));
}

void remixTestAiPostProcessRules(void)
{
    printf("ai post process rules\n");
    RUN_TEST(testActionSetsMatchTheAsmIds);
    RUN_TEST(testFoxFamilyUspCoversStartTravelAndAir);
    RUN_TEST(testQuickAttackCoversTheSixPikachuActions);
    RUN_TEST(testDkActions);
    RUN_TEST(testKirbyStoneActions);
    RUN_TEST(testDiveTurnFacingTargetKeepsPointing);
    RUN_TEST(testDiveTurnAwayFromTargetHoldsTowardIt);
    RUN_TEST(testDiveTurnTargetAtSameXIsTreatedAsRight);
    RUN_TEST(testLandingProbeGoesTowardTheTarget);
    RUN_TEST(testLandingProbeSameXGoesLeft);
    RUN_TEST(testWolfStickYBands);
    RUN_TEST(testDkProbeFollowsTheSpeedSign);
    RUN_TEST(testDkHoldsAgainstTheSpeed);
    RUN_TEST(testDededeHoldsBFromEightHundred);
    RUN_TEST(testMewtwoSkipsDspWhenTargetCannotBeGrabbed);
    RUN_TEST(testPuffSkipsDspAgainstShieldOrLowDamage);
    RUN_TEST(testYoshiSkipsNspAgainstEgg);
    RUN_TEST(testKirbyStoneCancelConditions);
    RUN_TEST(testKirbyMashPressesBOnOddFramesOnly);
    RUN_TEST(testSwordNspActionRange);
    RUN_TEST(testSwordNspStepTwoRepeatsItsHeight);
    RUN_TEST(testSwordNspAgainstShieldGoesLow);
    RUN_TEST(testSwordNspShieldOnDoesNotCount);
    RUN_TEST(testSwordNspByDamage);
    RUN_TEST(testSwordNspRollBetweenThirtyAndOneHundred);
    RUN_TEST(testSwordNspDrawsTheRollOnlyInTheRandomBlock);
    RUN_TEST(testMarinaSnatchChoice);
    RUN_TEST(testMarinaBackThrowGoesAwayFromTarget);
    RUN_TEST(testMarinaCargoShakesFromSixteen);
    RUN_TEST(testMarinaDspDistanceAndOdds);
}
