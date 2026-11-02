/**
 * test_gameend_rows_rules.c - unit tests of the extra results row rules (gm/gmremixgameendrowsrules.c).
 */

#include "test_check.h"

#include "gm/gmremixgameendrowsrules.h"

static void testKothTimeSplitsIntoMinutesAndSeconds(void)
{
    CHECK_EQ(0, gmRemixGameEndRowsMinutes(3599));
    CHECK_EQ(1, gmRemixGameEndRowsMinutes(3600));
    CHECK_EQ(59, gmRemixGameEndRowsSecondsDigits(3599));
    CHECK_EQ(5, gmRemixGameEndRowsSecondsDigits(65 * 60 + 12));
    CHECK_EQ(1, gmRemixGameEndRowsMinutes(65 * 60 + 12));
    CHECK_EQ(125, gmRemixGameEndRowsSeconds(125 * 60 + 59));
}

static void testGoalsAreCappedLikeTheKoColumn(void)
{
    CHECK_EQ(7, gmRemixGameEndRowsClampCount(7));
    CHECK_EQ(999, gmRemixGameEndRowsClampCount(999));
    CHECK_EQ(999, gmRemixGameEndRowsClampCount(1000));
    CHECK_EQ(999, gmRemixGameEndRowsClampCount(1500));
}

static void testPlaceRowMovesDownOnlyWithTheExtraRow(void)
{
    CHECK_EQ(124, gmRemixGameEndRowsPlaceY(124, 0));
    CHECK_EQ(140, gmRemixGameEndRowsPlaceY(124, 1));
}

static void testTensDigitIsForcedOnlyOnTheKothTimeRow(void)
{
    CHECK_EQ(1, gmRemixGameEndRowsForcesTensDigit(1));
    CHECK_EQ(0, gmRemixGameEndRowsForcesTensDigit(0));
}

void remixTestGameEndRowsRules(void)
{
    printf("results extra row rules\n");
    RUN_TEST(testKothTimeSplitsIntoMinutesAndSeconds);
    RUN_TEST(testGoalsAreCappedLikeTheKoColumn);
    RUN_TEST(testPlaceRowMovesDownOnlyWithTheExtraRow);
    RUN_TEST(testTensDigitIsForcedOnlyOnTheKothTimeRow);
}
