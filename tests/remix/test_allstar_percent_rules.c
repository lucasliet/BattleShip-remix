/**
 * test_allstar_percent_rules.c - unit tests of the All-Star carried percent and Rest Area afk exit rules
 * (sc/sc1pmode/sc1premixallstarpercentrules.c). Expected values come from SinglePlayerModes.asm player_percent, record_percent and
 * rest_area_routine.
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixallstarpercentrules.h"

static void testTheEnemyPortIsTheNextPortAndWrapsAfterPortFour(void)
{
    CHECK_EQ(1, sc1PRemixAllstarEnemyPort(0));
    CHECK_EQ(2, sc1PRemixAllstarEnemyPort(1));
    CHECK_EQ(3, sc1PRemixAllstarEnemyPort(2));
    CHECK_EQ(0, sc1PRemixAllstarEnemyPort(3));
}

static void testEveryHumanTakesThePercentWithoutAnEnemyPort(void)
{
    CHECK_TRUE(sc1PRemixAllstarTakesPercent(0, 0, 0));
    CHECK_TRUE(sc1PRemixAllstarTakesPercent(0, 0, 1));
    CHECK_TRUE(sc1PRemixAllstarTakesPercent(0, 2, 3));
}

static void testTheEnemyHumanStartsAtZeroWhenAnEnemyPortIsSet(void)
{
    CHECK_EQ(0, sc1PRemixAllstarTakesPercent(2, 0, 1));
    CHECK_EQ(0, sc1PRemixAllstarTakesPercent(4, 3, 0));
    CHECK_EQ(0, sc1PRemixAllstarTakesPercent(1, 1, 2));
}

static void testThePlayingHumanKeepsThePercentWhenAnEnemyPortIsSet(void)
{
    CHECK_TRUE(sc1PRemixAllstarTakesPercent(2, 0, 0));
    CHECK_TRUE(sc1PRemixAllstarTakesPercent(4, 3, 3));
}

static void testALimboRunRecordsZeroAndOtherwiseThePercent(void)
{
    CHECK_EQ(0, sc1PRemixAllstarRecordedPercent(87, 1));
    CHECK_EQ(87, sc1PRemixAllstarRecordedPercent(87, 0));
    CHECK_EQ(0, sc1PRemixAllstarRecordedPercent(0, 0));
}

static void testTheRestAreaNeedsTheFighterOnTheGround(void)
{
    CHECK_EQ(0, sc1PRemixAllstarRestIsLeft(0, 1, 0, 0));
    CHECK_EQ(0, sc1PRemixAllstarRestIsLeft(0, 1, 1, 500));
}

static void testTheRestAreaIsLeftOnTheExitClippingWithoutAfk(void)
{
    CHECK_TRUE(sc1PRemixAllstarRestIsLeft(1, 1, 0, 0));
    CHECK_EQ(0, sc1PRemixAllstarRestIsLeft(1, 0, 0, 5000));
}

static void testAfkLeavesTheRestAreaAfterOneHundredTwentyFramesAnywhere(void)
{
    CHECK_EQ(120, SC1PREMIX_ALLSTAR_AFK_REST_FRAMES);
    CHECK_EQ(0, sc1PRemixAllstarRestIsLeft(1, 0, 1, 119));
    CHECK_TRUE(sc1PRemixAllstarRestIsLeft(1, 0, 1, 120));
    CHECK_TRUE(sc1PRemixAllstarRestIsLeft(1, 0, 1, 900));
}

static void testAfkStillLeavesOnTheExitClippingBeforeTheWait(void)
{
    CHECK_TRUE(sc1PRemixAllstarRestIsLeft(1, 1, 1, 10));
}

void remixTestAllstarPercentRules(void)
{
    printf("all-star percent rules\n");
    RUN_TEST(testTheEnemyPortIsTheNextPortAndWrapsAfterPortFour);
    RUN_TEST(testEveryHumanTakesThePercentWithoutAnEnemyPort);
    RUN_TEST(testTheEnemyHumanStartsAtZeroWhenAnEnemyPortIsSet);
    RUN_TEST(testThePlayingHumanKeepsThePercentWhenAnEnemyPortIsSet);
    RUN_TEST(testALimboRunRecordsZeroAndOtherwiseThePercent);
    RUN_TEST(testTheRestAreaNeedsTheFighterOnTheGround);
    RUN_TEST(testTheRestAreaIsLeftOnTheExitClippingWithoutAfk);
    RUN_TEST(testAfkLeavesTheRestAreaAfterOneHundredTwentyFramesAnywhere);
    RUN_TEST(testAfkStillLeavesOnTheExitClippingBeforeTheWait);
}
