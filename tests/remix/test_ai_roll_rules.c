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

static void testGiantBowserRollSpamAppliesAtAnyLevel(void)
{
    CHECK_TRUE(ftRemixAiRollSpamApplies(1, 5));
}

static void testLevelNineRollSpamDoesNotApply(void)
{
    CHECK_EQ(0, ftRemixAiRollSpamApplies(0, 9));
}

static void testLevelTenRollSpamApplies(void)
{
    CHECK_TRUE(ftRemixAiRollSpamApplies(0, 10));
}

static void testActionAboveTurnRunResetsStick(void)
{
    CHECK_TRUE(ftRemixAiRollSpamResetsStick(0x14));
}

static void testTurnRunDoesNotResetStick(void)
{
    CHECK_EQ(0, ftRemixAiRollSpamResetsStick(0x13));
}

static void testDamageRangeIsAdvantage(void)
{
    CHECK_TRUE(ftRemixAiRollSpamIsAdvantageAction(0x25));
    CHECK_TRUE(ftRemixAiRollSpamIsAdvantageAction(0x37));
}

static void testActionBeforeDamageRangeIsNotAdvantage(void)
{
    CHECK_EQ(0, ftRemixAiRollSpamIsAdvantageAction(0x24));
}

static void testDownBounceStunAndSleepAreAdvantage(void)
{
    CHECK_TRUE(ftRemixAiRollSpamIsAdvantageAction(0x43));
    CHECK_TRUE(ftRemixAiRollSpamIsAdvantageAction(0x44));
    CHECK_TRUE(ftRemixAiRollSpamIsAdvantageAction(0xA4));
    CHECK_TRUE(ftRemixAiRollSpamIsAdvantageAction(0xA5));
}

static void testTurnActionIsNotAdvantage(void)
{
    CHECK_EQ(0, ftRemixAiRollSpamIsAdvantageAction(0x12));
}

static void testAdvantageDrawZeroShortHopsInPlace(void)
{
    CHECK_EQ(0x33, ftRemixAiRollSpamAdvantageCommand(0x2D, 0));
}

static void testAdvantageDrawOneShortHopsTowards(void)
{
    CHECK_EQ(0x31, ftRemixAiRollSpamAdvantageCommand(0x2D, 1));
}

static void testAdvantageOtherDrawsSkip(void)
{
    CHECK_EQ(FTREMIX_AI_ROLL_SPAM_SKIP, ftRemixAiRollSpamAdvantageCommand(0x2D, 2));
}

static void testNeutralDrawZeroKeepsTheRoll(void)
{
    CHECK_EQ(0x2E, ftRemixAiRollSpamNeutralCommand(0x2E, 0));
}

static void testNeutralDrawsOneToThreeAreHops(void)
{
    CHECK_EQ(0x33, ftRemixAiRollSpamNeutralCommand(0x2D, 1));
    CHECK_EQ(0x31, ftRemixAiRollSpamNeutralCommand(0x2D, 2));
    CHECK_EQ(0x32, ftRemixAiRollSpamNeutralCommand(0x2D, 3));
}

static void testNeutralDrawsFourAndFiveArePivots(void)
{
    CHECK_EQ(0x37, ftRemixAiRollSpamNeutralCommand(0x2D, 4));
    CHECK_EQ(0x37, ftRemixAiRollSpamNeutralCommand(0x2D, 5));
}

static void testNeutralHighDrawsSkip(void)
{
    CHECK_EQ(FTREMIX_AI_ROLL_SPAM_SKIP, ftRemixAiRollSpamNeutralCommand(0x2D, 6));
    CHECK_EQ(FTREMIX_AI_ROLL_SPAM_SKIP, ftRemixAiRollSpamNeutralCommand(0x2D, 15));
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
    RUN_TEST(testGiantBowserRollSpamAppliesAtAnyLevel);
    RUN_TEST(testLevelNineRollSpamDoesNotApply);
    RUN_TEST(testLevelTenRollSpamApplies);
    RUN_TEST(testActionAboveTurnRunResetsStick);
    RUN_TEST(testTurnRunDoesNotResetStick);
    RUN_TEST(testDamageRangeIsAdvantage);
    RUN_TEST(testActionBeforeDamageRangeIsNotAdvantage);
    RUN_TEST(testDownBounceStunAndSleepAreAdvantage);
    RUN_TEST(testTurnActionIsNotAdvantage);
    RUN_TEST(testAdvantageDrawZeroShortHopsInPlace);
    RUN_TEST(testAdvantageDrawOneShortHopsTowards);
    RUN_TEST(testAdvantageOtherDrawsSkip);
    RUN_TEST(testNeutralDrawZeroKeepsTheRoll);
    RUN_TEST(testNeutralDrawsOneToThreeAreHops);
    RUN_TEST(testNeutralDrawsFourAndFiveArePivots);
    RUN_TEST(testNeutralHighDrawsSkip);
}
