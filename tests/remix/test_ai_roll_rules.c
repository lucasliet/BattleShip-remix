/**
 * test_ai_roll_rules.c - unit tests of the Remix AI level 10 routine rules (ft/ftremix/ftremixairollrules.c):
 * double jump while falling and platform drop.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixairollrules.h"

static void testLevelTenIsTheOnlyAdvancedLevel(void)
{
    CHECK_TRUE(ftRemixAiRollIsLevelTen(10));
}

static void testLevelNineIsNotAdvanced(void)
{
    CHECK_EQ(0, ftRemixAiRollIsLevelTen(9));
}

static void testLevelTenMayJumpWhileFallingOnAerialProc(void)
{
    CHECK_TRUE(ftRemixAiRollCanJumpWhileFalling(10, 1, 0));
}

static void testLevelNineMayNotJumpOnAerialProcAlone(void)
{
    CHECK_EQ(0, ftRemixAiRollCanJumpWhileFalling(9, 1, 0));
}

static void testFallSpecialStatusJumpsAtAnyLevel(void)
{
    CHECK_TRUE(ftRemixAiRollCanJumpWhileFalling(9, 0, 1));
}

static void testLevelTenWithoutAerialProcOrStatusDoesNotJump(void)
{
    CHECK_EQ(0, ftRemixAiRollCanJumpWhileFalling(10, 0, 0));
}

static void testIdleLevelTenShieldDrops(void)
{
    CHECK_TRUE(ftRemixAiRollShieldDrops(10, 1));
}

static void testNonIdleLevelTenPlatformDrops(void)
{
    CHECK_EQ(0, ftRemixAiRollShieldDrops(10, 0));
}

static void testIdleLevelNinePlatformDrops(void)
{
    CHECK_EQ(0, ftRemixAiRollShieldDrops(9, 1));
}

void remixTestAiRollRules(void)
{
    printf("ai roll rules\n");
    RUN_TEST(testLevelTenIsTheOnlyAdvancedLevel);
    RUN_TEST(testLevelNineIsNotAdvanced);
    RUN_TEST(testLevelTenMayJumpWhileFallingOnAerialProc);
    RUN_TEST(testLevelNineMayNotJumpOnAerialProcAlone);
    RUN_TEST(testFallSpecialStatusJumpsAtAnyLevel);
    RUN_TEST(testLevelTenWithoutAerialProcOrStatusDoesNotJump);
    RUN_TEST(testIdleLevelTenShieldDrops);
    RUN_TEST(testNonIdleLevelTenPlatformDrops);
    RUN_TEST(testIdleLevelNinePlatformDrops);
}
