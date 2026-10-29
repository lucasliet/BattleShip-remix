/**
 * test_timedstock_rules.c - unit tests of the Timed Stock score arithmetic (gm/gmremixtimedstockrules.c).
 */

#include "test_check.h"

#include "gm/gmremixtimedstockrules.h"

static void testNoStocksLeftScoresZero(void)
{
    CHECK_EQ(0, gmRemixTimedStockRulesStockScore(0, 150));
}

static void testStocksWeighMoreThanAnyDamage(void)
{
    CHECK_TRUE(gmRemixTimedStockRulesStockScore(2, 999) > gmRemixTimedStockRulesStockScore(1, 0));
}

static void testDamageLowersTheScoreOfTheSameStocks(void)
{
    CHECK_EQ((3 << GMREMIX_TIMEDSTOCK_STOCK_SHIFT) - 40, gmRemixTimedStockRulesStockScore(3, 40));
    CHECK_TRUE(gmRemixTimedStockRulesStockScore(3, 10) > gmRemixTimedStockRulesStockScore(3, 90));
}

static void testTugOfWarCountsKosMinusFalls(void)
{
    int base = gmRemixTimedStockRulesTugScore(2, 3, 1, 0);

    CHECK_EQ(base + (1 << GMREMIX_TIMEDSTOCK_STOCK_SHIFT), gmRemixTimedStockRulesTugScore(2, 4, 1, 0));
    CHECK_EQ(base - 25, gmRemixTimedStockRulesTugScore(2, 3, 1, 25));
}

static void testTugOfWarStocksOutweighKos(void)
{
    CHECK_TRUE(gmRemixTimedStockRulesTugScore(3, 0, 0, 500) > gmRemixTimedStockRulesTugScore(2, 40, 0, 0));
}

static void testAnInfiniteLimitNeverRunsOut(void)
{
    CHECK_EQ(0, gmRemixTimedStockRulesIsTimeUp(100, 0, 100));
    CHECK_EQ(1, gmRemixTimedStockRulesIsTimeUp(5, 0, 100));
    CHECK_EQ(0, gmRemixTimedStockRulesIsTimeUp(5, 60, 100));
}

void remixTestTimedStockRules(void)
{
    printf("timed stock rules\n");
    RUN_TEST(testNoStocksLeftScoresZero);
    RUN_TEST(testStocksWeighMoreThanAnyDamage);
    RUN_TEST(testDamageLowersTheScoreOfTheSameStocks);
    RUN_TEST(testTugOfWarCountsKosMinusFalls);
    RUN_TEST(testTugOfWarStocksOutweighKos);
    RUN_TEST(testAnInfiniteLimitNeverRunsOut);
}
