/**
 * test_gameend_wait_rules.c - unit tests of the exit wait of the VS results screen (gm/gmremixgameendwaitrules.c,
 * TimedStock.asm show_full_results_).
 */

#include "test_check.h"

#include "gm/gmremixgameendwaitrules.h"

static void testStockResultsWaitTheTimeTicsWithFullResults(void)
{
    CHECK_EQ(410, gmRemixGameEndRulesStockExitWait(1));
    CHECK_EQ(GMREMIX_GAMEEND_WAIT_TIME, gmRemixGameEndRulesStockExitWait(2));
}

static void testStockResultsKeepTheVanillaWaitWithoutFullResults(void)
{
    CHECK_EQ(370, gmRemixGameEndRulesStockExitWait(0));
}

void remixTestGameEndWaitRules(void)
{
    printf("gameend wait rules\n");
    RUN_TEST(testStockResultsWaitTheTimeTicsWithFullResults);
    RUN_TEST(testStockResultsKeepTheVanillaWaitWithoutFullResults);
}
