/**
 * test_1p_afk_rules.c - unit tests of the "afk" player tag rules (sc/sc1pmode/sc1premixenemyafkrules.c). Expected values come from
 * SinglePlayerEnemy.asm _afk_1p and Credits.asm remix_credits_speed_assist_ (tag word 0x61666B00, name id 0x55, CPU level 10).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixenemyafkrules.h"

static void testOnlyTheExactTextAfkSwitchesTheAutomationOn(void)
{
    CHECK_TRUE(sc1PRemixAfkIsTagName("afk"));
    CHECK_EQ(0, sc1PRemixAfkIsTagName("AFK"));
    CHECK_EQ(0, sc1PRemixAfkIsTagName("Afk"));
    CHECK_EQ(0, sc1PRemixAfkIsTagName("afk "));
    CHECK_EQ(0, sc1PRemixAfkIsTagName("afkk"));
    CHECK_EQ(0, sc1PRemixAfkIsTagName("af"));
    CHECK_EQ(0, sc1PRemixAfkIsTagName(""));
    CHECK_EQ(0, sc1PRemixAfkIsTagName(0));
}

static void testTagTwoIsTheOneThatCounts(void)
{
    CHECK_EQ(2, SC1PREMIX_AFK_TAG_NUMBER);
}

static void testPlayerSlotBecomesABotOutsideTheContest(void)
{
    CHECK_TRUE(sc1PRemixAfkMakesPlayerBot(1, 1, 0));
    CHECK_EQ(0, sc1PRemixAfkMakesPlayerBot(0, 1, 0));
    CHECK_EQ(0, sc1PRemixAfkMakesPlayerBot(1, 0, 0));
    CHECK_EQ(0, sc1PRemixAfkMakesPlayerBot(1, 1, 1));
    CHECK_EQ(10, SC1PREMIX_AFK_BOT_LEVEL);
}

static void testSpeedAssistFiresOnlyAtTheFirstRemixNameWhileFast(void)
{
    CHECK_TRUE(sc1PRemixAfkSpeedAssistFires(1, 0x55, 0.11F, 0.00375F));
    CHECK_EQ(0, sc1PRemixAfkSpeedAssistFires(1, 0x55, 0.00375F, 0.00375F));
    CHECK_EQ(0, sc1PRemixAfkSpeedAssistFires(1, 0x54, 0.11F, 0.00375F));
    CHECK_EQ(0, sc1PRemixAfkSpeedAssistFires(1, 0x56, 0.11F, 0.00375F));
    CHECK_EQ(0, sc1PRemixAfkSpeedAssistFires(0, 0x55, 0.11F, 0.00375F));
}

static void testSoundTestTimerCountsOnlyWhileASongPlaysAndNoAIsTapped(void)
{
    CHECK_EQ(1, (int)sc1PRemixAfkSoundTestNextTimer(0, 0, 1));
    CHECK_EQ(101, (int)sc1PRemixAfkSoundTestNextTimer(100, 0, 1));
    CHECK_EQ(0, (int)sc1PRemixAfkSoundTestNextTimer(100, 1, 1));
    CHECK_EQ(0, (int)sc1PRemixAfkSoundTestNextTimer(100, 0, 0));
}

static void testSoundTestPressesAAfterSevenMinutes(void)
{
    CHECK_EQ(25200, SC1PREMIX_AFK_SOUNDTEST_WAIT_TIME);
    CHECK_EQ(0, sc1PRemixAfkSoundTestFiresA(25197, 0, 1));
    CHECK_EQ(0, sc1PRemixAfkSoundTestFiresA(25198, 0, 1));
    CHECK_TRUE(sc1PRemixAfkSoundTestFiresA(25199, 0, 1));
    CHECK_EQ(0, sc1PRemixAfkSoundTestFiresA(25199, 1, 1));
    CHECK_EQ(0, sc1PRemixAfkSoundTestFiresA(25199, 0, 0));
    CHECK_EQ(0, (int)sc1PRemixAfkSoundTestNextTimer(25199, 0, 1));
}

static void testSoundTestPressesRightOneFrameBeforeA(void)
{
    CHECK_EQ(0, sc1PRemixAfkSoundTestRightIsDue(1, 25198));
    CHECK_TRUE(sc1PRemixAfkSoundTestRightIsDue(1, 25199));
    CHECK_TRUE(sc1PRemixAfkSoundTestRightIsDue(1, 25200));
    CHECK_EQ(0, sc1PRemixAfkSoundTestRightIsDue(0, 25199));
}

static void testCreditsShootByItselfOnlyWhileAfk(void)
{
    CHECK_TRUE(sc1PRemixAfkCreditsShootPressed(1, 0));
    CHECK_TRUE(sc1PRemixAfkCreditsShootPressed(0, 1));
    CHECK_EQ(0, sc1PRemixAfkCreditsShootPressed(0, 0));
}

static void testCreditsAutomaticShotIsSilentUnlessTheAIsManual(void)
{
    CHECK_TRUE(sc1PRemixAfkCreditsShootIsSilent(1, 0));
    CHECK_EQ(0, sc1PRemixAfkCreditsShootIsSilent(1, 1));
    CHECK_EQ(0, sc1PRemixAfkCreditsShootIsSilent(0, 0));
    CHECK_EQ(0, sc1PRemixAfkCreditsShootIsSilent(0, 1));
}

void remixTest1PAfkRules(void)
{
    printf("1p afk rules\n");
    RUN_TEST(testSoundTestTimerCountsOnlyWhileASongPlaysAndNoAIsTapped);
    RUN_TEST(testSoundTestPressesAAfterSevenMinutes);
    RUN_TEST(testSoundTestPressesRightOneFrameBeforeA);
    RUN_TEST(testCreditsShootByItselfOnlyWhileAfk);
    RUN_TEST(testCreditsAutomaticShotIsSilentUnlessTheAIsManual);
    RUN_TEST(testOnlyTheExactTextAfkSwitchesTheAutomationOn);
    RUN_TEST(testTagTwoIsTheOneThatCounts);
    RUN_TEST(testPlayerSlotBecomesABotOutsideTheContest);
    RUN_TEST(testSpeedAssistFiresOnlyAtTheFirstRemixNameWhileFast);
}
