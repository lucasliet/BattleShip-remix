/**
 * test_ai_prediction_rules.c - unit tests of the level 10 Remix AI attack range and movement prediction rules
 * (ft/ftremix/ftremixaipredictionrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaipredictionrules.h"

static void testAttackViewBaseIsWidenedOnlyForLevelTen(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesAttackViewBase(10) == 2000.0F);
    CHECK_TRUE(ftRemixAiPredictionRulesAttackViewBase(9) == 1200.0F);
}

static void testRunningCandidatesKeepOnlyTheRunnableAttacks(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesRunningCandidateAllowed(1, FTREMIX_AI_PREDICTION_INPUT_GRAB));
    CHECK_TRUE(ftRemixAiPredictionRulesRunningCandidateAllowed(1, FTREMIX_AI_PREDICTION_INPUT_DASH_ATTACK));
    CHECK_EQ(0, ftRemixAiPredictionRulesRunningCandidateAllowed(1, 0x13));
}

static void testNonRunningCandidatesAreAlwaysAllowed(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesRunningCandidateAllowed(0, 0x13));
}

static void testMyXWithoutSpeedKeepsPosition(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesMyX(100.0F, 0.0F, 2.0F, 10) == 100.0F);
}

static void testMyXAddsSpeedWithoutFriction(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesMyX(0.0F, 2.0F, 0.0F, 3) == 6.0F);
}

static void testMyXStopsWhenFrictionCrossesZero(void)
{
    /* speed 2, friction 1: positions 2, then speed 1 -> 3, then speed 0 -> stop after the second frame */
    CHECK_TRUE(ftRemixAiPredictionRulesMyX(0.0F, 2.0F, 1.0F, 10) == 3.0F);
}

static void testMyXNegativeSpeedSlowsTowardsZero(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesMyX(0.0F, -2.0F, 1.0F, 10) == -3.0F);
}

static void testDeltaXIsTargetMinusPredictedCpu(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesDeltaX(10.0F, 50.0F, 2.0F, 5) == 50.0F);
}

static void testFallYStaysAboveFloorWhileFalling(void)
{
    /* vel 0 then -1 (gravity 1 applied after each frame): y 100 -> 100 -> 99; floor_dist -500 keeps it airborne */
    CHECK_TRUE(ftRemixAiPredictionRulesFallY(100.0F, 0.0F, 1.0F, -10.0F, -500.0F, 1, 2) == 99.0F);
}

static void testFallYSnapsToGroundWhenReachingFloor(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesFallY(100.0F, -20.0F, 0.0F, -50.0F, -10.0F, 1, 3) == 90.0F);
}

static void testFallYIgnoresFloorWithoutFloorLine(void)
{
    CHECK_TRUE(ftRemixAiPredictionRulesFallY(100.0F, -20.0F, 0.0F, -50.0F, -10.0F, 0, 1) == 80.0F);
}

static void testFallYCapsFallSpeed(void)
{
    /* speed -10 is capped to -5 after the first frame, so the second frame moves by -5 */
    CHECK_TRUE(ftRemixAiPredictionRulesFallY(0.0F, 0.0F, 10.0F, -5.0F, -1000.0F, 1, 2) == -5.0F);
}

void remixTestAiPredictionRules(void)
{
    testAttackViewBaseIsWidenedOnlyForLevelTen();
    testRunningCandidatesKeepOnlyTheRunnableAttacks();
    testNonRunningCandidatesAreAlwaysAllowed();
    testMyXWithoutSpeedKeepsPosition();
    testMyXAddsSpeedWithoutFriction();
    testMyXStopsWhenFrictionCrossesZero();
    testMyXNegativeSpeedSlowsTowardsZero();
    testDeltaXIsTargetMinusPredictedCpu();
    testFallYStaysAboveFloorWhileFalling();
    testFallYSnapsToGroundWhenReachingFloor();
    testFallYIgnoresFloorWithoutFloorLine();
    testFallYCapsFallSpeed();
}
