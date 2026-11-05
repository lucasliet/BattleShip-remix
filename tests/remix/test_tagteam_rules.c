/**
 * test_tagteam_rules.c - unit tests of the Tag Team queue rules (gm/gmremixtagteamrules.c).
 */

#include "test_check.h"

#include "gm/gmremixtagteamrules.h"

#define QUEUE_MAX 6
#define FIRST_RANDOM_FIGHTER 0x3F
#define LAST_RANDOM_FIGHTER 0x44

static void testTheSlotIsTheStocksSetMinusTheStocksLeft(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesGetSlot(4, 4, QUEUE_MAX));
    CHECK_EQ(2, gmRemixTagTeamRulesGetSlot(4, 2, QUEUE_MAX));
    CHECK_EQ(4, gmRemixTagTeamRulesGetSlot(4, 0, QUEUE_MAX));
}

static void testTheSlotIsClampedToTheLastQueueSlot(void)
{
    CHECK_EQ(QUEUE_MAX - 1, gmRemixTagTeamRulesGetSlot(QUEUE_MAX, 0, QUEUE_MAX));
    CHECK_EQ(QUEUE_MAX - 1, gmRemixTagTeamRulesGetSlot(10, 0, QUEUE_MAX));
}

static void testTheSlotIsClampedToZeroWhenStocksLeftAreMore(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesGetSlot(2, 5, QUEUE_MAX));
}

static void testNoStocksAreSkippedOutsideStockModeLast(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesSkippedStocks(0, 4, 1));
}

static void testStockModeLastSkipsTheStocksBelowTheSetting(void)
{
    CHECK_EQ(3, gmRemixTagTeamRulesSkippedStocks(1, 4, 1));
    CHECK_EQ(0, gmRemixTagTeamRulesSkippedStocks(1, 4, 4));
}

static void testALowerSettingThanTheStoredCountSkipsNothing(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesSkippedStocks(1, 1, 3));
}

static void testTheQueueIndexFollowsTheInitialStockCount(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesQueueIndex(2, 2, 0, 0, QUEUE_MAX));
    CHECK_EQ(1, gmRemixTagTeamRulesQueueIndex(2, 1, 0, 0, QUEUE_MAX));
}

static void testManualStockCountStartsAtTheFirstSlot(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesQueueIndex(1, 1, 0, 0, QUEUE_MAX));
    CHECK_EQ(1, gmRemixTagTeamRulesQueueIndex(1, 0, 0, 0, QUEUE_MAX));
}

static void testStolenStocksAndSkippedStocksMoveTheQueueIndex(void)
{
    CHECK_EQ(1, gmRemixTagTeamRulesQueueIndex(3, 1, 1, 0, QUEUE_MAX));
    CHECK_EQ(3, gmRemixTagTeamRulesQueueIndex(1, 0, 0, 2, QUEUE_MAX));
}

static void testTheShadeCountsEarlierTeammatesWithTheSameFighter(void)
{
    const int fkinds[] = { 0x1F, 0x20, 0x1F };
    const int teams[] = { 0, 0, 1 };

    CHECK_EQ(1, gmRemixTagTeamRulesShade(0x1F, 0, fkinds, teams, 3));
    CHECK_EQ(1, gmRemixTagTeamRulesShade(0x1F, 1, fkinds, teams, 3));
    CHECK_EQ(0, gmRemixTagTeamRulesShade(0x21, 0, fkinds, teams, 3));
    CHECK_EQ(0, gmRemixTagTeamRulesShade(0x1F, 0, fkinds, teams, 0));
}

static void testTheRightmostIconIsTheCurrentFighter(void)
{
    CHECK_EQ(1, gmRemixTagTeamRulesIconSlot(1, 3, 2, QUEUE_MAX));
}

static void testTheLeftmostIconIsTheLastCharacterOfTheQueue(void)
{
    CHECK_EQ(3, gmRemixTagTeamRulesIconSlot(1, 3, 0, QUEUE_MAX));
    CHECK_EQ(2, gmRemixTagTeamRulesIconSlot(1, 3, 1, QUEUE_MAX));
}

static void testAnIconSlotIsClampedToTheQueue(void)
{
    CHECK_EQ(QUEUE_MAX - 1, gmRemixTagTeamRulesIconSlot(4, 6, 0, QUEUE_MAX));
    CHECK_EQ(0, gmRemixTagTeamRulesIconSlot(0, 1, 3, QUEUE_MAX));
}

static void testASwapIsNeededOnlyForAnotherFighter(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesNeedsSwap(0x1F, 0, 0, 0, 0x1F, 0, 0, 0));
    CHECK_EQ(1, gmRemixTagTeamRulesNeedsSwap(0x1F, 0, 0, 0, 0x20, 0, 0, 0));
}

static void testASwapIsNeededWhenClassicSonicChanges(void)
{
    CHECK_EQ(1, gmRemixTagTeamRulesNeedsSwap(0x3B, 0, 0, 1, 0x3B, 0, 0, 0));
    CHECK_EQ(1, gmRemixTagTeamRulesNeedsSwap(0x3B, 0, 0, 0, 0x3B, 0, 0, 1));
    CHECK_EQ(0, gmRemixTagTeamRulesNeedsSwap(0x3B, 0, 0, 1, 0x3B, 0, 0, 1));
}

static void testASwapIsNeededForAnotherCostumeOrShade(void)
{
    CHECK_EQ(1, gmRemixTagTeamRulesNeedsSwap(0x1F, 2, 0, 0, 0x1F, 0, 0, 0));
    CHECK_EQ(1, gmRemixTagTeamRulesNeedsSwap(0x1F, 0, 1, 0, 0x1F, 0, 0, 0));
}

static void testTheRandomListHasThirtyFighters(void)
{
    CHECK_EQ(30, gmRemixTagTeamRulesRandomCount());
}

static void testTheRandomIndexPicksTheFighterOfTheList(void)
{
    CHECK_EQ(FIRST_RANDOM_FIGHTER, gmRemixTagTeamRulesRandomFighter(0));
    CHECK_EQ(0x20, gmRemixTagTeamRulesRandomFighter(1));
    CHECK_EQ(LAST_RANDOM_FIGHTER, gmRemixTagTeamRulesRandomFighter(29));
}

static void testARandomIndexOutOfRangeIsClamped(void)
{
    CHECK_EQ(FIRST_RANDOM_FIGHTER, gmRemixTagTeamRulesRandomFighter(-1));
    CHECK_EQ(LAST_RANDOM_FIGHTER, gmRemixTagTeamRulesRandomFighter(30));
}

void remixTestTagTeamRules(void)
{
    printf("tag team rules\n");
    RUN_TEST(testTheSlotIsTheStocksSetMinusTheStocksLeft);
    RUN_TEST(testTheSlotIsClampedToTheLastQueueSlot);
    RUN_TEST(testTheSlotIsClampedToZeroWhenStocksLeftAreMore);
    RUN_TEST(testNoStocksAreSkippedOutsideStockModeLast);
    RUN_TEST(testStockModeLastSkipsTheStocksBelowTheSetting);
    RUN_TEST(testALowerSettingThanTheStoredCountSkipsNothing);
    RUN_TEST(testTheQueueIndexFollowsTheInitialStockCount);
    RUN_TEST(testManualStockCountStartsAtTheFirstSlot);
    RUN_TEST(testStolenStocksAndSkippedStocksMoveTheQueueIndex);
    RUN_TEST(testTheShadeCountsEarlierTeammatesWithTheSameFighter);
    RUN_TEST(testTheRightmostIconIsTheCurrentFighter);
    RUN_TEST(testTheLeftmostIconIsTheLastCharacterOfTheQueue);
    RUN_TEST(testAnIconSlotIsClampedToTheQueue);
    RUN_TEST(testASwapIsNeededOnlyForAnotherFighter);
    RUN_TEST(testASwapIsNeededForAnotherCostumeOrShade);
    RUN_TEST(testASwapIsNeededWhenClassicSonicChanges);
    RUN_TEST(testTheRandomListHasThirtyFighters);
    RUN_TEST(testTheRandomIndexPicksTheFighterOfTheList);
    RUN_TEST(testARandomIndexOutOfRangeIsClamped);
}
