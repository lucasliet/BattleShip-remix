/**
 * test_koth_rules.c - unit tests of the King of the Hill king rules (gm/gmremixkothrules.c).
 */

#include "test_check.h"

#include "gm/gmremixkothrules.h"

#define BUFFER 30

static void testAnEmptyHillHasNoKing(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 0, 0, 0 }, { 0, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_NO_KING, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(0, eligible[0]);
}

static void testOneFighterOnTheHillIsTheCandidate(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 0, 0, 0 }, { 1, 0, 1 } };
    int eligible[2];

    CHECK_EQ(1, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(1, eligible[1]);
}

static void testTeammatesShareTheHill(void)
{
    GMRemixKothRulesFighter fighters[3] = { { 1, 0, 2 }, { 1, 0, 2 }, { 0, 0, 0 } };
    int eligible[3];

    CHECK_EQ(2, gmRemixKothRulesCandidate(fighters, 3, eligible));
    CHECK_EQ(1, eligible[0]);
    CHECK_EQ(1, eligible[1]);
}

static void testTwoTeamsOnTheHillContestIt(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 1, 0, 0 }, { 1, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesCandidate(fighters, 2, eligible));
}

static void testHoldingAnEnemyContestsTheHillAndLosesTheClaim(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 1, 1, 0 }, { 0, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(0, eligible[0]);
}

static void testAContestStartsTheBuffer(void)
{
    int buffer = 0;

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(GMREMIX_KOTHRULES_CONTESTED, &buffer, BUFFER));
    CHECK_EQ(BUFFER, buffer);
}

static void testTheBufferHoldsTheHillUntilItRunsOut(void)
{
    int buffer = 2;

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(1, &buffer, BUFFER));
    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(1, &buffer, BUFFER));
    CHECK_EQ(1, gmRemixKothRulesSettle(1, &buffer, BUFFER));
}

void remixTestKothRules(void)
{
    printf("koth rules\n");
    RUN_TEST(testAnEmptyHillHasNoKing);
    RUN_TEST(testOneFighterOnTheHillIsTheCandidate);
    RUN_TEST(testTeammatesShareTheHill);
    RUN_TEST(testTwoTeamsOnTheHillContestIt);
    RUN_TEST(testHoldingAnEnemyContestsTheHillAndLosesTheClaim);
    RUN_TEST(testAContestStartsTheBuffer);
    RUN_TEST(testTheBufferHoldsTheHillUntilItRunsOut);
}
