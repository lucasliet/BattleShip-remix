/**
 * test_vsstats_rules.c - unit tests of the VS match stats arithmetic (mn/mnvsmode/mnremixvsstatsrules.c).
 */

#include "test_check.h"

#include "mn/mnvsmode/mnremixvsstatsrules.h"

static void testPercentIsZeroWithoutEvents(void)
{
    CHECK_EQ(0, mnRemixVSStatsRulesPercent(0, 0));
    CHECK_EQ(0, mnRemixVSStatsRulesPercent(3, 0));
    CHECK_EQ(0, mnRemixVSStatsRulesPercent(3, -1));
}

static void testPercentOfAllAndNoneSuccesses(void)
{
    CHECK_EQ(100, mnRemixVSStatsRulesPercent(5, 5));
    CHECK_EQ(0, mnRemixVSStatsRulesPercent(0, 5));
}

static void testPercentRoundsToNearestWithTiesToEven(void)
{
    CHECK_EQ(12, mnRemixVSStatsRulesPercent(1, 8));
    CHECK_EQ(38, mnRemixVSStatsRulesPercent(3, 8));
    CHECK_EQ(33, mnRemixVSStatsRulesPercent(1, 3));
    CHECK_EQ(67, mnRemixVSStatsRulesPercent(2, 3));
}

static void testHighestKeepsTheLargerValue(void)
{
    CHECK_EQ(40, mnRemixVSStatsRulesHighest(40, 12));
    CHECK_EQ(55, mnRemixVSStatsRulesHighest(40, 55));
    CHECK_EQ(7, mnRemixVSStatsRulesHighest(7, 7));
}

static void testNextPageWrapsAfterTheLastPage(void)
{
    CHECK_EQ(1, mnRemixVSStatsRulesNextPage(0));
    CHECK_EQ(0, mnRemixVSStatsRulesNextPage(MNREMIX_VSSTATS_RULES_TOTAL_PAGES - 1));
}

static void testPromptRevealsBeforeTheExitWaitEnds(void)
{
    CHECK_EQ(0, mnRemixVSStatsRulesIsPromptActive(0, 300));
    CHECK_EQ(0, mnRemixVSStatsRulesIsPromptActive(194, 300));
    CHECK_EQ(1, mnRemixVSStatsRulesIsPromptActive(195, 300));
    CHECK_EQ(1, mnRemixVSStatsRulesIsPromptActive(400, 300));
}

static void testBufferCountsDownAndStopsAtZero(void)
{
    CHECK_EQ(14, mnRemixVSStatsRulesTickBuffer(MNREMIX_VSSTATS_RULES_PRESS_BUFFER_PERIOD));
    CHECK_EQ(0, mnRemixVSStatsRulesTickBuffer(1));
    CHECK_EQ(0, mnRemixVSStatsRulesTickBuffer(0));
}

void remixTestVsStatsRules(void)
{
    printf("vs stats rules\n");
    RUN_TEST(testPercentIsZeroWithoutEvents);
    RUN_TEST(testPercentOfAllAndNoneSuccesses);
    RUN_TEST(testPercentRoundsToNearestWithTiesToEven);
    RUN_TEST(testHighestKeepsTheLargerValue);
    RUN_TEST(testNextPageWrapsAfterTheLastPage);
    RUN_TEST(testPromptRevealsBeforeTheExitWaitEnds);
    RUN_TEST(testBufferCountsDownAndStopsAtZero);
}
