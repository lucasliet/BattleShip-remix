/**
 * test_tugofwar_rules.c - unit tests of the Tug of War credit rules (gm/gmremixtugofwarrules.c).
 */

#include "test_check.h"

#include "gm/gmremixtugofwarrules.h"

#define NO_PORT GMREMIX_TUGOFWARRULES_NO_PORT

static void testTheKoerOfAnotherTeamGetsTheStock(void)
{
    CHECK_EQ(2, gmRemixTugOfWarPickCreditPort(0, 1, 2, 3));
}

static void testASelfTeamKoGivesTheStockToTheLastHit(void)
{
    CHECK_EQ(3, gmRemixTugOfWarPickCreditPort(1, 1, 2, 3));
}

static void testNoKoerGivesTheStockToTheLastHit(void)
{
    CHECK_EQ(3, gmRemixTugOfWarPickCreditPort(0, NO_PORT, NO_PORT, 3));
}

static void testNobodyToCreditStaysNobody(void)
{
    CHECK_EQ(NO_PORT, gmRemixTugOfWarPickCreditPort(0, NO_PORT, NO_PORT, NO_PORT));
}

static void testPortsOutsideZeroToThreeAreNotPorts(void)
{
    CHECK_EQ(1, gmRemixTugOfWarRulesIsValidPort(0));
    CHECK_EQ(1, gmRemixTugOfWarRulesIsValidPort(3));
    CHECK_EQ(0, gmRemixTugOfWarRulesIsValidPort(4));
    CHECK_EQ(0, gmRemixTugOfWarRulesIsValidPort(-1));
}

static void testOnlyAnEliminatedFighterRevives(void)
{
    CHECK_EQ(1, gmRemixTugOfWarShouldRevive(-1));
    CHECK_EQ(0, gmRemixTugOfWarShouldRevive(0));
    CHECK_EQ(0, gmRemixTugOfWarShouldRevive(2));
}

static void testAFighterWaitingForAStockStealDoesNotRevive(void)
{
    CHECK_EQ(0, gmRemixTugOfWarShouldRevive(-2));
}

void remixTestTugOfWarRules(void)
{
    printf("tug of war rules\n");
    RUN_TEST(testTheKoerOfAnotherTeamGetsTheStock);
    RUN_TEST(testASelfTeamKoGivesTheStockToTheLastHit);
    RUN_TEST(testNoKoerGivesTheStockToTheLastHit);
    RUN_TEST(testNobodyToCreditStaysNobody);
    RUN_TEST(testPortsOutsideZeroToThreeAreNotPorts);
    RUN_TEST(testOnlyAnEliminatedFighterRevives);
    RUN_TEST(testAFighterWaitingForAStockStealDoesNotRevive);
}
