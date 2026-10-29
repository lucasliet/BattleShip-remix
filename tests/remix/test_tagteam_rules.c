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

static void testASwapIsNeededOnlyForAnotherFighter(void)
{
    CHECK_EQ(0, gmRemixTagTeamRulesNeedsSwap(0x1F, 0x1F));
    CHECK_EQ(1, gmRemixTagTeamRulesNeedsSwap(0x1F, 0x20));
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
    RUN_TEST(testASwapIsNeededOnlyForAnotherFighter);
    RUN_TEST(testTheRandomListHasThirtyFighters);
    RUN_TEST(testTheRandomIndexPicksTheFighterOfTheList);
    RUN_TEST(testARandomIndexOutOfRangeIsClamped);
}
