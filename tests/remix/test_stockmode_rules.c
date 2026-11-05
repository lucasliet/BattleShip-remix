/**
 * test_stockmode_rules.c - unit tests of the Stock Mode count rules (gm/gmremixstockmoderules.c).
 */

#include "test_check.h"

#include "gm/gmremixstockmoderules.h"

static void testAnUnsetLastCountGivesTheMatchCount(void)
{
    CHECK_EQ(3, gmRemixStockModeRulesLastCount(-1, 3, 0));
}

static void testTheLastCountIsLoweredToTheMatchCount(void)
{
    CHECK_EQ(1, gmRemixStockModeRulesLastCount(1, 3, 0));
    CHECK_EQ(3, gmRemixStockModeRulesLastCount(5, 3, 0));
}

static void testTugOfWarKeepsALastCountAboveTheMatchCount(void)
{
    CHECK_EQ(5, gmRemixStockModeRulesLastCount(5, 3, 1));
}

static void testTugOfWarStillStartsFromTheMatchCountWhenUnset(void)
{
    CHECK_EQ(3, gmRemixStockModeRulesLastCount(-1, 3, 1));
}

static void testTheManualCountNeverPassesTheMatchCount(void)
{
    CHECK_EQ(1, gmRemixStockModeRulesManualCount(1, 3));
    CHECK_EQ(3, gmRemixStockModeRulesManualCount(7, 3));
}

void remixTestStockModeRules(void)
{
    printf("stock mode rules\n");
    RUN_TEST(testAnUnsetLastCountGivesTheMatchCount);
    RUN_TEST(testTheLastCountIsLoweredToTheMatchCount);
    RUN_TEST(testTugOfWarKeepsALastCountAboveTheMatchCount);
    RUN_TEST(testTugOfWarStillStartsFromTheMatchCountWhenUnset);
    RUN_TEST(testTheManualCountNeverPassesTheMatchCount);
}
