/**
 * test_ai_lv10_rules.c - unit tests of the level 10 CPU rules (ft/ftremix/ftremixailv10rules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixailv10rules.h"

static void testLevelTenIsLevelTen(void)
{
    CHECK_TRUE(ftRemixAiIsLevelTen(10));
}

static void testLevelNineIsNotLevelTen(void)
{
    CHECK_EQ(0, ftRemixAiIsLevelTen(9));
}

static void testStarmanEnemyCountsAtAnyLevel(void)
{
    CHECK_TRUE(ftRemixAiCountsAsInvulnerableEnemy(1, 0, 3));
}

static void testRespawnInvincibilityCountsForLevelTen(void)
{
    CHECK_TRUE(ftRemixAiCountsAsInvulnerableEnemy(0, 5, 10));
}

static void testRespawnInvincibilityIgnoredBelowLevelTen(void)
{
    CHECK_EQ(0, ftRemixAiCountsAsInvulnerableEnemy(0, 5, 9));
}

static void testVulnerableEnemyNeverCounts(void)
{
    CHECK_EQ(0, ftRemixAiCountsAsInvulnerableEnemy(0, 0, 10));
}

static void testCliffRollZeroGoesLetGo(void)
{
    CHECK_EQ(FTREMIX_AI_LV10_ROUTINE_CLIFF_LET_GO, ftRemixAiCliffGetupInputFromRoll(0, 4));
}

static void testCliffRollOneGoesLetGoAsInAsm(void)
{
    CHECK_EQ(FTREMIX_AI_LV10_ROUTINE_CLIFF_LET_GO, ftRemixAiCliffGetupInputFromRoll(1, 4));
}

static void testStartMashPressesInFramesWithBitSet(void)
{
    CHECK_EQ(0x1000 | 0x0002, ftRemixAiStartMashButtons(0x0002, 0x10));
}

static void testStartMashReleasesInFramesWithBitClear(void)
{
    CHECK_EQ(0, ftRemixAiStartMashButtons(0x0002, 0x00));
}

static void testDashAttackCancelledWhileDashing(void)
{
    CHECK_TRUE(ftRemixAiDashAttackCancelNeeded(1, 0));
}

static void testDashAttackKeptWhileIdle(void)
{
    CHECK_EQ(0, ftRemixAiDashAttackCancelNeeded(1, 1));
}

static void testOtherCommandsNeverCancelled(void)
{
    CHECK_EQ(0, ftRemixAiDashAttackCancelNeeded(0, 0));
}

void remixTestAiLv10Rules(void)
{
    printf("ai lv10 rules\n");
    RUN_TEST(testLevelTenIsLevelTen);
    RUN_TEST(testLevelNineIsNotLevelTen);
    RUN_TEST(testStarmanEnemyCountsAtAnyLevel);
    RUN_TEST(testRespawnInvincibilityCountsForLevelTen);
    RUN_TEST(testRespawnInvincibilityIgnoredBelowLevelTen);
    RUN_TEST(testVulnerableEnemyNeverCounts);
    RUN_TEST(testCliffRollZeroGoesLetGo);
    RUN_TEST(testCliffRollOneGoesLetGoAsInAsm);
    RUN_TEST(testStartMashPressesInFramesWithBitSet);
    RUN_TEST(testStartMashReleasesInFramesWithBitClear);
    RUN_TEST(testDashAttackCancelledWhileDashing);
    RUN_TEST(testDashAttackKeptWhileIdle);
    RUN_TEST(testOtherCommandsNeverCancelled);
}
