/**
 * test_1p_remix_rules.c - unit tests of the Remix 1P random picks (sc/sc1pmode/sc1premix1prules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premix1prules.h"

static int sScriptedDraws[32];
static int sScriptedCount;
static int sScriptedNext;

static void scriptDraws(const int *draws, int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        sScriptedDraws[i] = draws[i];
    }
    sScriptedCount = count;
    sScriptedNext = 0;
}

static int scriptedRandom(int range)
{
    int draw = (sScriptedNext < sScriptedCount) ? sScriptedDraws[sScriptedNext++] : 0;

    return draw % range;
}

static int firstRandom(int range)
{
    (void)range;
    return 0;
}

static void testAFighterIsPickedAmongTheFreeOnes(void)
{
    int is_used[4] = { 1, 0, 1, 0 };
    int draws[] = { 1 };

    scriptDraws(draws, 1);
    CHECK_EQ(3, sc1PRemix1PPickFighter(scriptedRandom, is_used, 4));
    CHECK_EQ(1, is_used[3]);
}

static void testNoFighterIsPickedWhenAllAreTaken(void)
{
    int is_used[2] = { 1, 1 };

    CHECK_EQ(-1, sc1PRemix1PPickFighter(firstRandom, is_used, 2));
}

static void testASlotNeverRepeatsAFighter(void)
{
    int is_used[3] = { 0, 0, 0 };
    int a = sc1PRemix1PPickFighter(firstRandom, is_used, 3);
    int b = sc1PRemix1PPickFighter(firstRandom, is_used, 3);
    int c = sc1PRemix1PPickFighter(firstRandom, is_used, 3);

    CHECK_EQ(0, a);
    CHECK_EQ(1, b);
    CHECK_EQ(2, c);
    CHECK_EQ(-1, sc1PRemix1PPickFighter(firstRandom, is_used, 3));
}

static void testAStageIsPickedFromTheOptionsNotUsedYet(void)
{
    int options[3] = { 5, 6, 7 };
    int is_stage_used[16] = { 0 };
    int draws[] = { 1 };

    is_stage_used[5] = 1;
    scriptDraws(draws, 1);
    CHECK_EQ(7, sc1PRemix1PPickStage(scriptedRandom, options, is_stage_used, 16));
    CHECK_EQ(1, is_stage_used[7]);
}

static void testAFighterWithEveryStageUsedGivesNoStage(void)
{
    int options[3] = { 5, 5, 5 };
    int is_stage_used[16] = { 0 };

    is_stage_used[5] = 1;
    CHECK_EQ(-1, sc1PRemix1PPickStage(firstRandom, options, is_stage_used, 16));
}

static void testTheSameStageInAllThreeOptionsIsUsedOnce(void)
{
    int options[3] = { 9, 9, 9 };
    int is_stage_used[16] = { 0 };

    CHECK_EQ(9, sc1PRemix1PPickStage(firstRandom, options, is_stage_used, 16));
    CHECK_EQ(-1, sc1PRemix1PPickStage(firstRandom, options, is_stage_used, 16));
}

static void testDistinctPicksNeverRepeat(void)
{
    int picks[7];
    int seen[22] = { 0 };
    int i;

    sc1PRemix1PPickDistinct(firstRandom, 22, picks, 7);
    for (i = 0; i < 7; i++)
    {
        CHECK_EQ(0, seen[picks[i]]);
        seen[picks[i]] = 1;
    }
}

static void testTheBossIsOneOfThree(void)
{
    int draws[] = { 0, 1, 2 };

    scriptDraws(draws, 3);
    CHECK_EQ(SC1PREMIX_1P_BOSS_MAD_PIANO, sc1PRemix1PPickBoss(scriptedRandom));
    CHECK_EQ(SC1PREMIX_1P_BOSS_METAL_BROS, sc1PRemix1PPickBoss(scriptedRandom));
    CHECK_EQ(SC1PREMIX_1P_BOSS_SUPER_SONIC, sc1PRemix1PPickBoss(scriptedRandom));
}

static void testSonicFightsInGreenHillTwiceAsOftenAsCasino(void)
{
    int draws[] = { 0, 1, 2 };

    scriptDraws(draws, 3);
    CHECK_EQ(SC1PREMIX_1P_SONIC_STAGE_GHZ, sc1PRemix1PPickSonicStage(scriptedRandom));
    CHECK_EQ(SC1PREMIX_1P_SONIC_STAGE_CASINO, sc1PRemix1PPickSonicStage(scriptedRandom));
    CHECK_EQ(SC1PREMIX_1P_SONIC_STAGE_GHZ, sc1PRemix1PPickSonicStage(scriptedRandom));
}

void remixTest1PRemixRules(void)
{
    printf("1p remix rules\n");
    RUN_TEST(testAFighterIsPickedAmongTheFreeOnes);
    RUN_TEST(testNoFighterIsPickedWhenAllAreTaken);
    RUN_TEST(testASlotNeverRepeatsAFighter);
    RUN_TEST(testAStageIsPickedFromTheOptionsNotUsedYet);
    RUN_TEST(testAFighterWithEveryStageUsedGivesNoStage);
    RUN_TEST(testTheSameStageInAllThreeOptionsIsUsedOnce);
    RUN_TEST(testDistinctPicksNeverRepeat);
    RUN_TEST(testTheBossIsOneOfThree);
    RUN_TEST(testSonicFightsInGreenHillTwiceAsOftenAsCasino);
}
