/**
 * test_1p_hrc_rules.c - unit tests of the state-free Home-Run Contest rules (sc/sc1pmode/sc1premixhrcrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixhrcrules.h"

static SC1PRemixHrcAttempt attemptStoppedAfterTheLine(void)
{
    SC1PRemixHrcAttempt attempt = { 100, 1, 0, 0, 0, 0, 0 };

    return attempt;
}

static void testResetClearsTheDistanceAndStartsTheEndTimer(void)
{
    SC1PRemixHrcState state = { 5.0F, 7, 1 };

    sc1PRemixHrcStateReset(&state);
    CHECK_EQ(0, (int)state.warp_distance);
    CHECK_EQ(0, state.distance);
    CHECK_EQ(45, state.complete_timer);
}

static void testCameraWaitsAtTheDefaultXBeforeTheStartLine(void)
{
    SC1PRemixHrcState state;
    SC1PRemixHrcMove move;

    sc1PRemixHrcStateReset(&state);
    sc1PRemixHrcTrack(&state, -20000.0F, -21000.0F, &move);
    CHECK_EQ(0, move.is_past_start);
    CHECK_EQ(-21312, (int)move.camera_x);
    CHECK_EQ(0, (int)move.sandbag_shift);
    CHECK_EQ(0, state.distance);
}

static void testDistanceIsInTenthsOfAFootFromTheStartLine(void)
{
    SC1PRemixHrcState state;
    SC1PRemixHrcMove move;

    sc1PRemixHrcStateReset(&state);
    sc1PRemixHrcTrack(&state, -19392.0F + 9120.0F, -21312.0F, &move);
    CHECK_EQ(1, move.is_past_start);
    CHECK_EQ(1000, state.distance);
}

static void testCameraPansAThirdOfTheWayToTheSandbag(void)
{
    SC1PRemixHrcState state;
    SC1PRemixHrcMove move;

    sc1PRemixHrcStateReset(&state);
    sc1PRemixHrcTrack(&state, -15000.0F, -18000.0F, &move);
    CHECK_EQ(-17000, (int)move.camera_x);
    CHECK_EQ(0, (int)move.sandbag_shift);
}

static void testCameraWarpMovesTheCameraAndTheSandbagBack(void)
{
    SC1PRemixHrcState state;
    SC1PRemixHrcMove move;

    sc1PRemixHrcStateReset(&state);
    sc1PRemixHrcTrack(&state, 14000.0F, 14000.0F, &move);
    CHECK_EQ(14000 - 9120, (int)move.camera_x);
    CHECK_EQ(-9120, (int)move.sandbag_shift);
    CHECK_EQ(9120, (int)state.warp_distance);
}

static void testDistanceKeepsCountingAcrossAWarp(void)
{
    SC1PRemixHrcState state;
    SC1PRemixHrcMove move;

    sc1PRemixHrcStateReset(&state);
    sc1PRemixHrcTrack(&state, 14000.0F, 14000.0F, &move);
    sc1PRemixHrcTrack(&state, 14000.0F - 9120.0F + 912.0F, move.camera_x, &move);
    CHECK_EQ((int)((14000.0F + 19392.0F + 912.0F) / 9.12F), state.distance);
}

static void testTheAttemptContinuesOverThePlatformWithTimeLeft(void)
{
    SC1PRemixHrcAttempt attempt = attemptStoppedAfterTheLine();

    attempt.is_past_start = 0;
    attempt.is_over_platform = 1;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_KEEP_GOING, sc1PRemixHrcAttemptVerdict(&attempt));
}

static void testAStoppedSandbagPastTheLineEndsTheAttempt(void)
{
    SC1PRemixHrcAttempt attempt = attemptStoppedAfterTheLine();

    CHECK_EQ(SC1PREMIX_HRC_VERDICT_END, sc1PRemixHrcAttemptVerdict(&attempt));
}

static void testAFlyingSandbagKeepsTheAttemptGoing(void)
{
    SC1PRemixHrcAttempt attempt = attemptStoppedAfterTheLine();

    attempt.is_sandbag_aerial = 1;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_KEEP_GOING, sc1PRemixHrcAttemptVerdict(&attempt));
    attempt.time_passed = 600;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS, sc1PRemixHrcAttemptVerdict(&attempt));
}

static void testASlidingSandbagCutsTheInputsButGoesOn(void)
{
    SC1PRemixHrcAttempt attempt = attemptStoppedAfterTheLine();

    attempt.is_sandbag_moving = 1;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS, sc1PRemixHrcAttemptVerdict(&attempt));
}

static void testTimeUpBeforeTheLineWaitsForTheHumanToRest(void)
{
    SC1PRemixHrcAttempt attempt = attemptStoppedAfterTheLine();

    attempt.time_passed = 600;
    attempt.is_past_start = 0;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_DISABLE_INPUTS, sc1PRemixHrcAttemptVerdict(&attempt));
    attempt.is_human_resting = 1;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_END, sc1PRemixHrcAttemptVerdict(&attempt));
}

static void testTimeUpWithTheHumanStuckInAHoldEndsAtOnce(void)
{
    SC1PRemixHrcAttempt attempt = attemptStoppedAfterTheLine();

    attempt.time_passed = 600;
    attempt.is_sandbag_aerial = 1;
    attempt.is_human_stuck_in_hold = 1;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_END, sc1PRemixHrcAttemptVerdict(&attempt));
    attempt.time_passed = 599;
    CHECK_EQ(SC1PREMIX_HRC_VERDICT_KEEP_GOING, sc1PRemixHrcAttemptVerdict(&attempt));
}

static void testCounterStaysInTheCornerWhileTheAttemptIsOn(void)
{
    SC1PRemixHrcCounterLayout layout;

    sc1PRemixHrcCounterAt(0, 0, 40.0F, &layout);
    CHECK_EQ(278, (int)layout.counter_x);
    CHECK_EQ(44, (int)layout.counter_y);
    CHECK_EQ(282, (int)layout.label_x);
    CHECK_EQ(47, (int)layout.label_y);
    CHECK_EQ(90, (int)(layout.counter_scale * 100.0F + 0.5F));
    CHECK_EQ(62, (int)(layout.label_scale * 100.0F));
}

static void testCounterSlidesToTheCentreAndGrowsAtComplete(void)
{
    SC1PRemixHrcCounterLayout layout;
    float final_right_edge = 160.0F + 0.725F * 40.0F - 12.5F;

    sc1PRemixHrcCounterAt(45, 1, 40.0F, &layout);
    /* HRC.asm _complete animates X, Y and scale once per frame for timer 45..0: 46 steps of each. */
    CHECK_EQ((int)(final_right_edge * 10.0F + 0.5F), (int)(layout.counter_x * 10.0F + 0.5F));
    CHECK_EQ(67, (int)layout.counter_y);
    CHECK_EQ(136, (int)(layout.counter_scale * 100.0F + 0.5F));
    CHECK_EQ(109, (int)(layout.label_scale * 100.0F + 0.5F));
    CHECK_EQ((int)(layout.label_x - layout.counter_x), 4);
}

static void testCounterMovesOneStepInTheFirstTicOfComplete(void)
{
    SC1PRemixHrcCounterLayout layout;
    float step = (278.0F - (160.0F + 0.725F * 40.0F - 12.5F)) / 46.0F;

    sc1PRemixHrcCounterAt(0, 1, 40.0F, &layout);
    CHECK_EQ((int)((278.0F - step) * 100.0F), (int)(layout.counter_x * 100.0F));
    CHECK_EQ(44, (int)layout.counter_y);
}

static void testCounterClampsTheTicsToTheEndTimer(void)
{
    SC1PRemixHrcCounterLayout layout;

    sc1PRemixHrcCounterAt(-5, 1, 40.0F, &layout);
    CHECK_EQ(221, (int)((278.0F - layout.counter_x) * 100.0F + 0.5F));
    sc1PRemixHrcCounterAt(900, 1, 40.0F, &layout);
    CHECK_EQ(136, (int)(layout.counter_scale * 100.0F + 0.5F));
}

static void testCloudsStandStillBeforeTheStartLine(void)
{
    CHECK_EQ(0, (int)sc1PRemixHrcCloudSpeed(500.0F, 0));
}

static void testCloudsScrollAgainstTheSandbagPastTheStartLine(void)
{
    CHECK_EQ(-5, (int)sc1PRemixHrcCloudSpeed(5000.0F, 1));
}

static void testCloudsKeepScrollingBetweenTheLimits(void)
{
    CHECK_EQ(-101, (int)sc1PRemixHrcCloudStep(-100.0F, -1.0F));
}

static void testCloudsJumpBackWhenTheyScrollPastTheThreshold(void)
{
    CHECK_EQ(-8, (int)sc1PRemixHrcCloudStep(-342.0F, -1.0F));
}

static void testTheFirstDistanceIsSavedQuietly(void)
{
    CHECK_EQ(SC1PREMIX_HRC_RECORD_FIRST, sc1PRemixHrcRecordVerdict(0, 2308));
}

static void testAZeroDistanceNeverMakesARecord(void)
{
    CHECK_EQ(SC1PREMIX_HRC_RECORD_KEEP, sc1PRemixHrcRecordVerdict(0, 0));
}

static void testALongerDistanceBeatsTheRecord(void)
{
    CHECK_EQ(SC1PREMIX_HRC_RECORD_BEATEN, sc1PRemixHrcRecordVerdict(2308, 2309));
}

static void testAnEqualOrShorterDistanceKeepsTheRecord(void)
{
    CHECK_EQ(SC1PREMIX_HRC_RECORD_KEEP, sc1PRemixHrcRecordVerdict(2308, 2308));
    CHECK_EQ(SC1PREMIX_HRC_RECORD_KEEP, sc1PRemixHrcRecordVerdict(2308, 100));
}

void remixTest1PHrcRules(void)
{
    printf("1p hrc rules\n");
    RUN_TEST(testResetClearsTheDistanceAndStartsTheEndTimer);
    RUN_TEST(testCameraWaitsAtTheDefaultXBeforeTheStartLine);
    RUN_TEST(testDistanceIsInTenthsOfAFootFromTheStartLine);
    RUN_TEST(testCameraPansAThirdOfTheWayToTheSandbag);
    RUN_TEST(testCameraWarpMovesTheCameraAndTheSandbagBack);
    RUN_TEST(testDistanceKeepsCountingAcrossAWarp);
    RUN_TEST(testTheAttemptContinuesOverThePlatformWithTimeLeft);
    RUN_TEST(testAStoppedSandbagPastTheLineEndsTheAttempt);
    RUN_TEST(testAFlyingSandbagKeepsTheAttemptGoing);
    RUN_TEST(testASlidingSandbagCutsTheInputsButGoesOn);
    RUN_TEST(testTimeUpBeforeTheLineWaitsForTheHumanToRest);
    RUN_TEST(testTimeUpWithTheHumanStuckInAHoldEndsAtOnce);
    RUN_TEST(testCounterStaysInTheCornerWhileTheAttemptIsOn);
    RUN_TEST(testCounterSlidesToTheCentreAndGrowsAtComplete);
    RUN_TEST(testCounterMovesOneStepInTheFirstTicOfComplete);
    RUN_TEST(testCounterClampsTheTicsToTheEndTimer);
    RUN_TEST(testTheFirstDistanceIsSavedQuietly);
    RUN_TEST(testAZeroDistanceNeverMakesARecord);
    RUN_TEST(testALongerDistanceBeatsTheRecord);
    RUN_TEST(testAnEqualOrShorterDistanceKeepsTheRecord);
    RUN_TEST(testCloudsStandStillBeforeTheStartLine);
    RUN_TEST(testCloudsScrollAgainstTheSandbagPastTheStartLine);
    RUN_TEST(testCloudsKeepScrollingBetweenTheLimits);
    RUN_TEST(testCloudsJumpBackWhenTheyScrollPastTheThreshold);
}
