/**
 * test_ai_drift_rules.c - unit tests of the Remix AI level 10 recovery drift (ft/ftremix/ftremixairecoverydriftrules.c).
 */

#include <string.h>

#include "test_check.h"

#include "ft/ftremix/ftremixairecoverydriftrules.h"

/* A fighter off the left side of the stage at (-4000, -100) facing right, with grabbable ledges at (-1000, 0) and (1000, 0). */
static FTRemixAiDriftState makeState(void)
{
    FTRemixAiDriftState state;

    memset(&state, 0, sizeof(state));
    state.x = -4000.0F;
    state.y = -100.0F;
    state.lr = 1;
    state.left_x = -1000.0F;
    state.left_y = 0.0F;
    state.right_x = 1000.0F;
    state.right_y = 0.0F;
    state.left_grabbable = 1;
    state.right_grabbable = 1;
    state.grab_x = 18.0F;
    state.grab_y = 20.0F;
    state.bounds_left = -3000.0F;
    state.bounds_right = 3000.0F;
    return state;
}

static FTRemixAiDriftResult compute(const FTRemixAiDriftState *state)
{
    FTRemixAiDriftResult result;

    ftRemixAiDriftComputeTarget(state, &result);
    return result;
}

static void testGateNeedsLevelTen(void)
{
    CHECK_EQ(0, ftRemixAiDriftGateOpens(0, 1, 2, 2, 1, -1));
}

static void testGateOpensInSpecialFall(void)
{
    CHECK_TRUE(ftRemixAiDriftGateOpens(1, 1, 0, 2, 0, 5));
}

static void testGateOpensWithAllJumpsUsed(void)
{
    CHECK_TRUE(ftRemixAiDriftGateOpens(1, 0, 2, 2, 0, 5));
}

static void testGateOpensWhenRecoveringWithNoFloorBelow(void)
{
    CHECK_TRUE(ftRemixAiDriftGateOpens(1, 0, 0, 2, 1, -1));
}

static void testGateStaysClosedWithFloorBelowOrOtherObjective(void)
{
    CHECK_EQ(0, ftRemixAiDriftGateOpens(1, 0, 0, 2, 1, 5));
    CHECK_EQ(0, ftRemixAiDriftGateOpens(1, 0, 0, 2, 0, -1));
}

static void testEdgeInsideTheShrunkBlastzoneIsOnScreen(void)
{
    CHECK_TRUE(ftRemixAiDriftEdgeIsOnScreen(0.0F, 0.0F, 5000.0F, -5000.0F, 6000.0F, -6000.0F));
}

static void testEdgeNearTheBlastzoneIsOffScreen(void)
{
    CHECK_EQ(0, ftRemixAiDriftEdgeIsOnScreen(0.0F, 4900.0F, 5000.0F, -5000.0F, 6000.0F, -6000.0F));
    CHECK_EQ(0, ftRemixAiDriftEdgeIsOnScreen(0.0F, -4900.0F, 5000.0F, -5000.0F, 6000.0F, -6000.0F));
    CHECK_EQ(0, ftRemixAiDriftEdgeIsOnScreen(-5900.0F, 0.0F, 5000.0F, -5000.0F, 6000.0F, -6000.0F));
    CHECK_EQ(0, ftRemixAiDriftEdgeIsOnScreen(5900.0F, 0.0F, 5000.0F, -5000.0F, 6000.0F, -6000.0F));
}

static void testLedgeAboveTheFighterLooksFartherInY(void)
{
    float above = ftRemixAiDriftLedgeDistance(0.0F, 0.0F, 0.0F, 100.0F);
    float below = ftRemixAiDriftLedgeDistance(0.0F, 100.0F, 0.0F, 0.0F);

    CHECK_TRUE(below == 100.0F);
    CHECK_TRUE(above == 150.0F);
}

static void testFacingAGrabbableLedgeBelowAimsAtTheGrabBox(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result = compute(&state);

    CHECK_EQ(0, result.skipped);
    CHECK_TRUE(result.target_x == -1018.0F);
    CHECK_TRUE(result.target_y == -20.0F);
}

static void testNotFacingTheLedgeBelowAimsOutsideIt(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.lr = -1;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -1300.0F);
    CHECK_TRUE(result.target_y == 0.0F);
}

static void testSlowJumpersAimOutsideTheLedgeEvenWhenFacingIt(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.is_slow_jumper = 1;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -1300.0F);
}

static void testUngrabbableLedgeBelowAimsOutsideIt(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.left_grabbable = 0;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -1300.0F);
}

static void testAboveTheLedgeFacingItLowersTheTargetByTheGrabBox(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.y = 500.0F;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -1000.0F);
    CHECK_TRUE(result.target_y == -20.0F);
}

static void testAboveTheLedgeNotFacingItAimsInsideIt(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.y = 500.0F;
    state.lr = -1;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -700.0F);
    CHECK_TRUE(result.target_y == 0.0F);
}

static void testRightLedgeIsChosenFromTheFacingBias(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.x = 900.0F;
    state.y = 500.0F;
    state.lr = -1;
    result = compute(&state);
    CHECK_TRUE(result.target_x == 1000.0F);
    CHECK_TRUE(result.target_y == -20.0F);
}

static void testUspBelowTheLedgeFacingItJustPointsAtTheLedge(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.special_flag_set = 1;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -1000.0F);
    CHECK_TRUE(result.target_y == 0.0F);
}

static void testUspBelowTheLedgeNotFacingItPushesTheTargetTowardTheLedgeSide(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.special_flag_set = 1;
    state.lr = -1;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -3000.0F);
}

static void testUspAboveTheLedgeAimsInsideIt(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.special_flag_set = 1;
    state.y = 500.0F;
    result = compute(&state);
    CHECK_TRUE(result.target_x == -700.0F);
}

static void testDamageStatusClearsTheFlagBeforeItIsUsed(void)
{
    FTRemixAiDriftState state = makeState();
    FTRemixAiDriftResult result;

    state.special_flag_set = 1;
    state.resets_special_flag = 1;
    result = compute(&state);
    CHECK_TRUE(result.clears_special_flag);
    CHECK_TRUE(result.target_x == -1018.0F);
}

static void testInnerGapWellAboveTheLedgesIsSkipped(void)
{
    FTRemixAiDriftState state = makeState();

    state.x = 0.0F;
    state.y = 2000.0F;
    CHECK_TRUE(compute(&state).skipped);
}

static void testInnerGapIsNotSkippedWithUspOrSpecialFall(void)
{
    FTRemixAiDriftState state = makeState();

    state.x = 0.0F;
    state.y = 2000.0F;
    state.special_flag_set = 1;
    CHECK_EQ(0, compute(&state).skipped);
    state.special_flag_set = 0;
    state.is_fall_special = 1;
    CHECK_EQ(0, compute(&state).skipped);
}

static void testOffstageIsNeverSkipped(void)
{
    FTRemixAiDriftState state = makeState();

    state.y = 5000.0F;
    CHECK_EQ(0, compute(&state).skipped);
}

void remixTestAiDriftRules(void)
{
    printf("ai drift rules\n");
    RUN_TEST(testGateNeedsLevelTen);
    RUN_TEST(testGateOpensInSpecialFall);
    RUN_TEST(testGateOpensWithAllJumpsUsed);
    RUN_TEST(testGateOpensWhenRecoveringWithNoFloorBelow);
    RUN_TEST(testGateStaysClosedWithFloorBelowOrOtherObjective);
    RUN_TEST(testEdgeInsideTheShrunkBlastzoneIsOnScreen);
    RUN_TEST(testEdgeNearTheBlastzoneIsOffScreen);
    RUN_TEST(testLedgeAboveTheFighterLooksFartherInY);
    RUN_TEST(testFacingAGrabbableLedgeBelowAimsAtTheGrabBox);
    RUN_TEST(testNotFacingTheLedgeBelowAimsOutsideIt);
    RUN_TEST(testSlowJumpersAimOutsideTheLedgeEvenWhenFacingIt);
    RUN_TEST(testUngrabbableLedgeBelowAimsOutsideIt);
    RUN_TEST(testAboveTheLedgeFacingItLowersTheTargetByTheGrabBox);
    RUN_TEST(testAboveTheLedgeNotFacingItAimsInsideIt);
    RUN_TEST(testRightLedgeIsChosenFromTheFacingBias);
    RUN_TEST(testUspBelowTheLedgeFacingItJustPointsAtTheLedge);
    RUN_TEST(testUspBelowTheLedgeNotFacingItPushesTheTargetTowardTheLedgeSide);
    RUN_TEST(testUspAboveTheLedgeAimsInsideIt);
    RUN_TEST(testDamageStatusClearsTheFlagBeforeItIsUsed);
    RUN_TEST(testInnerGapWellAboveTheLedgesIsSkipped);
    RUN_TEST(testInnerGapIsNotSkippedWithUspOrSpecialFall);
    RUN_TEST(testOffstageIsNeverSkipped);
}
