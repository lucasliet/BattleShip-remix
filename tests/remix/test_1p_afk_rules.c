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

void remixTest1PAfkRules(void)
{
    printf("1p afk rules\n");
    RUN_TEST(testOnlyTheExactTextAfkSwitchesTheAutomationOn);
    RUN_TEST(testTagTwoIsTheOneThatCounts);
    RUN_TEST(testPlayerSlotBecomesABotOutsideTheContest);
    RUN_TEST(testSpeedAssistFiresOnlyAtTheFirstRemixNameWhileFast);
}
