/**
 * test_announcer_rules.c - unit tests of the Pokemon Stadium announcer rules and call tables (if/ifremixannouncerrules.c,
 * if/ifremixannouncertables.c).
 */

#include "test_check.h"

#include "if/ifremixannouncerrules.h"

#define FAKE_DRAW_MAX 8

static int sFakeDraws[FAKE_DRAW_MAX];
static int sFakeDrawCount;
static int sFakeDrawNext;
static int sFakeDrawRange;

static void setFakeDraws(int first, int second, int count)
{
    sFakeDraws[0] = first;
    sFakeDraws[1] = second;
    sFakeDrawCount = count;
    sFakeDrawNext = 0;
    sFakeDrawRange = 0;
}

static int fakeDraw(int range)
{
    sFakeDrawRange = range;
    return sFakeDraws[sFakeDrawNext++ % sFakeDrawCount];
}

static void testHighKnockbackDrawsOverTheHighAndTheMidCalls(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallKnockbackHigh);

    ifRemixAnnouncerTablesReset();
    setFakeDraws(17, 17, 1);
    CHECK_EQ(18, table->pick_count);
    CHECK_EQ(0x493, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    CHECK_EQ(18, sFakeDrawRange);
    setFakeDraws(11, 11, 1);
    CHECK_EQ(0x48D, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
}

static void testMidKnockbackKeepsItsSixCalls(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallKnockbackMid);

    ifRemixAnnouncerTablesReset();
    setFakeDraws(0, 0, 1);
    CHECK_EQ(6, table->pick_count);
    CHECK_EQ(0x48E, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
}

static void testHighPickOfAMidCallCoolsTheMidCall(void)
{
    IFRemixAnnouncerTable *high = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallKnockbackHigh);
    IFRemixAnnouncerTable *mid = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallKnockbackMid);
    IFRemixAnnouncement *picked;

    ifRemixAnnouncerTablesReset();
    setFakeDraws(12, 12, 1);
    picked = ifRemixAnnouncerRulesPickStandard(high, fakeDraw);
    CHECK_TRUE(picked == &mid->entries[0]);
    picked->timer = 900;
    setFakeDraws(0, 0, 1);
    CHECK_TRUE(ifRemixAnnouncerRulesPickStandard(mid, fakeDraw) == &mid->entries[0]);
    CHECK_EQ(900, mid->entries[0].timer);
}

static void testSharedEntriesCountDownOncePerFrame(void)
{
    IFRemixAnnouncerTable *mid = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallKnockbackMid);
    IFRemixAnnouncerTable *burrow = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallBurrow);

    ifRemixAnnouncerTablesReset();
    mid->entries[3].timer = 10;
    burrow->entries[2].timer = 10;
    ifRemixAnnouncerTablesCountDown();
    CHECK_EQ(9, mid->entries[3].timer);
    CHECK_EQ(9, burrow->entries[2].timer);
}

static void testResetClearsTimersAndUsedFlagsOfSharedEntries(void)
{
    IFRemixAnnouncerTable *opening = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallOpening);
    IFRemixAnnouncerTable *gym = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallGymLeader);

    opening->entries[2].timer = 5;
    opening->entries[2].used = 1;
    gym->entries[0].used = 1;
    ifRemixAnnouncerTablesReset();
    CHECK_EQ(0, opening->entries[2].timer);
    CHECK_EQ(0, opening->entries[2].used);
    CHECK_EQ(0, gym->entries[0].used);
}

static void testBurrowDrawsOverTheBurrowAndTheBurrowAgainCalls(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallBurrow);

    ifRemixAnnouncerTablesReset();
    CHECK_EQ(3, table->pick_count);
    setFakeDraws(0, 0, 1);
    CHECK_EQ(0x5D1, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    setFakeDraws(1, 1, 1);
    CHECK_EQ(0x5D3, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    setFakeDraws(2, 2, 1);
    CHECK_EQ(0x5D2, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    CHECK_EQ(3, sFakeDrawRange);
}

static void testGymLeaderCastleDrawsOverItsCallAndTheFirstOpening(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallGymLeader);

    ifRemixAnnouncerTablesReset();
    CHECK_EQ(2, table->pick_count);
    setFakeDraws(0, 0, 1);
    CHECK_EQ(0x4B6, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    setFakeDraws(1, 1, 1);
    CHECK_EQ(0x4B7, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
}

static void testOpeningHasItsThreeCalls(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallOpening);

    CHECK_EQ(3, table->pick_count);
    CHECK_EQ(0x4B7, table->entries[0].fgm_id);
    CHECK_EQ(0x4C0, table->entries[1].fgm_id);
    CHECK_EQ(0x4C1, table->entries[2].fgm_id);
}

static void testBurnDrawsOverTheFirstCallOnly(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallBurn);

    ifRemixAnnouncerTablesReset();
    CHECK_EQ(1, table->pick_count);
    setFakeDraws(0, 0, 1);
    CHECK_EQ(0x478, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    CHECK_EQ(1, sFakeDrawRange);
}

static void testClangPlaysOnlyOncePerMatch(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallClang);

    ifRemixAnnouncerTablesReset();
    CHECK_EQ(0, ifRemixAnnouncerRulesIsSpent(table));
    table->entries[0].used = 1;
    CHECK_EQ(1, ifRemixAnnouncerRulesIsSpent(table));
    ifRemixAnnouncerTablesReset();
    CHECK_EQ(0, ifRemixAnnouncerRulesIsSpent(table));
}

static void testOtherCallsAreNeverSpent(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallThunder);

    table->entries[0].used = 1;
    CHECK_EQ(0, ifRemixAnnouncerRulesIsSpent(table));
    ifRemixAnnouncerTablesReset();
}

static void testStandardPickTakesTheFirstFreeDraw(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallSleep);

    ifRemixAnnouncerTablesReset();
    table->entries[0].timer = 100;
    setFakeDraws(0, 2, 2);
    CHECK_EQ(0x47F, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
}

static void testStandardPickUsesTheLastDrawWhenEveryDrawIsCoolingAndTheFlagIsClear(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallSleep);

    ifRemixAnnouncerTablesReset();
    table->entries[0].timer = 100;
    table->entries[1].timer = 100;
    table->entries[2].timer = 100;
    setFakeDraws(2, 1, 2);
    CHECK_EQ(0x47F, ifRemixAnnouncerRulesPickStandard(table, fakeDraw)->fgm_id);
    ifRemixAnnouncerTablesReset();
}

static void testStandardPickStaysSilentWhenEveryDrawIsCoolingAndTheFlagIsSet(void)
{
    IFRemixAnnouncerTable *table = ifRemixAnnouncerTablesGet(nIFRemixAnnouncerCallStun);

    ifRemixAnnouncerTablesReset();
    table->entries[0].timer = 100;
    table->entries[1].timer = 100;
    setFakeDraws(0, 1, 2);
    CHECK_TRUE(ifRemixAnnouncerRulesPickStandard(table, fakeDraw) == NULL);
    ifRemixAnnouncerTablesReset();
}

static void testSleepRequestIsOnlyReadOnFramesWhereTheGlobalTimerRan(void)
{
    CHECK_EQ(1, ifRemixAnnouncerRulesChecksSleep(120, 1));
    CHECK_EQ(1, ifRemixAnnouncerRulesChecksSleep(1, 1));
    CHECK_EQ(0, ifRemixAnnouncerRulesChecksSleep(0, 1));
    CHECK_EQ(0, ifRemixAnnouncerRulesChecksSleep(120, 0));
    CHECK_EQ(0, ifRemixAnnouncerRulesChecksSleep(0, 0));
}

void remixTestAnnouncerRules(void)
{
    printf("announcer rules\n");
    RUN_TEST(testHighKnockbackDrawsOverTheHighAndTheMidCalls);
    RUN_TEST(testMidKnockbackKeepsItsSixCalls);
    RUN_TEST(testHighPickOfAMidCallCoolsTheMidCall);
    RUN_TEST(testSharedEntriesCountDownOncePerFrame);
    RUN_TEST(testResetClearsTimersAndUsedFlagsOfSharedEntries);
    RUN_TEST(testBurrowDrawsOverTheBurrowAndTheBurrowAgainCalls);
    RUN_TEST(testGymLeaderCastleDrawsOverItsCallAndTheFirstOpening);
    RUN_TEST(testOpeningHasItsThreeCalls);
    RUN_TEST(testBurnDrawsOverTheFirstCallOnly);
    RUN_TEST(testClangPlaysOnlyOncePerMatch);
    RUN_TEST(testOtherCallsAreNeverSpent);
    RUN_TEST(testStandardPickTakesTheFirstFreeDraw);
    RUN_TEST(testStandardPickUsesTheLastDrawWhenEveryDrawIsCoolingAndTheFlagIsClear);
    RUN_TEST(testStandardPickStaysSilentWhenEveryDrawIsCoolingAndTheFlagIsSet);
    RUN_TEST(testSleepRequestIsOnlyReadOnFramesWhereTheGlobalTimerRan);
}
