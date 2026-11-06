/**
 * test_bgm_stage_rules.c - unit tests of the stage song pick (gm/gmremixbgmstagerules.c) and the VS demo Final Destination skip.
 */

#include "test_check.h"

#include "gm/gmremixbgmrules.h"
#include "gm/gmremixbgmstagerules.h"

#define NONE GMREMIX_BGM_NO_TRACK
#define MAIN_TRACK 6
#define OCCASIONAL_TRACK 100
#define RARE_TRACK 101
#define RARE_2_TRACK 102
#define NO_ROLL 0

static void testCButtonsForceTheirSlotInApplyOrder(void)
{
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_DEFAULT, gmRemixBgmStageRulesCButtonSlot(GMREMIX_BGM_STAGE_C_UP));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_OCCASIONAL, gmRemixBgmStageRulesCButtonSlot(GMREMIX_BGM_STAGE_C_LEFT));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE, gmRemixBgmStageRulesCButtonSlot(GMREMIX_BGM_STAGE_C_RIGHT));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE_2, gmRemixBgmStageRulesCButtonSlot(GMREMIX_BGM_STAGE_C_DOWN));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_NONE, gmRemixBgmStageRulesCButtonSlot(0));
}

static void testCUpWinsOverTheOtherCButtons(void)
{
    unsigned int all_c = GMREMIX_BGM_STAGE_C_UP | GMREMIX_BGM_STAGE_C_LEFT | GMREMIX_BGM_STAGE_C_RIGHT | GMREMIX_BGM_STAGE_C_DOWN;

    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_DEFAULT, gmRemixBgmStageRulesCButtonSlot(all_c));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_OCCASIONAL, gmRemixBgmStageRulesCButtonSlot(all_c & ~GMREMIX_BGM_STAGE_C_UP));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE, gmRemixBgmStageRulesCButtonSlot(GMREMIX_BGM_STAGE_C_RIGHT | GMREMIX_BGM_STAGE_C_DOWN));
}

static void testOtherButtonsDoNotForceASong(void)
{
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_NONE, gmRemixBgmStageRulesCButtonSlot(0x8000 | 0x4000 | 0x2000 | 0x1000 | 0x0020 | 0x0010));
}

static void testRollBoundariesFollowTheChances(void)
{
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_DEFAULT, gmRemixBgmStageRulesRollSlot(0));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_DEFAULT, gmRemixBgmStageRulesRollSlot(64));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_OCCASIONAL, gmRemixBgmStageRulesRollSlot(65));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_OCCASIONAL, gmRemixBgmStageRulesRollSlot(80));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE, gmRemixBgmStageRulesRollSlot(81));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE, gmRemixBgmStageRulesRollSlot(90));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE_2, gmRemixBgmStageRulesRollSlot(91));
    CHECK_EQ(GMREMIX_BGM_STAGE_SLOT_RARE_2, gmRemixBgmStageRulesRollSlot(99));
}

static void testRollIsOnlyNeededWithoutCButtonOrPreservedSong(void)
{
    CHECK_TRUE(gmRemixBgmStageRulesNeedsRoll(GMREMIX_BGM_STAGE_SLOT_NONE, NONE));
    CHECK_TRUE(!gmRemixBgmStageRulesNeedsRoll(GMREMIX_BGM_STAGE_SLOT_RARE, NONE));
    CHECK_TRUE(!gmRemixBgmStageRulesNeedsRoll(GMREMIX_BGM_STAGE_SLOT_DEFAULT, NONE));
    CHECK_TRUE(!gmRemixBgmStageRulesNeedsRoll(GMREMIX_BGM_STAGE_SLOT_NONE, OCCASIONAL_TRACK));
}

static void testCUpPlaysTheMainSongAndBlocksRandomMusic(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { OCCASIONAL_TRACK, RARE_TRACK, RARE_2_TRACK };
    GMRemixBgmStagePick pick = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_DEFAULT, NONE, NO_ROLL);

    CHECK_EQ(MAIN_TRACK, pick.track);
    CHECK_TRUE(pick.random_disabled);
}

static void testCButtonPlaysItsAlternateAndBlocksRandomMusic(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { OCCASIONAL_TRACK, RARE_TRACK, RARE_2_TRACK };
    GMRemixBgmStagePick left = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_OCCASIONAL, NONE, NO_ROLL);
    GMRemixBgmStagePick right = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_RARE, NONE, NO_ROLL);
    GMRemixBgmStagePick down = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_RARE_2, NONE, NO_ROLL);

    CHECK_EQ(OCCASIONAL_TRACK, left.track);
    CHECK_EQ(RARE_TRACK, right.track);
    CHECK_EQ(RARE_2_TRACK, down.track);
    CHECK_TRUE(left.random_disabled && right.random_disabled && down.random_disabled);
}

static void testCButtonOnAnEmptySlotKeepsTheMainSongAndAllowsRandomMusic(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { OCCASIONAL_TRACK, NONE, NONE };
    GMRemixBgmStagePick pick = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_RARE, NONE, NO_ROLL);

    CHECK_EQ(MAIN_TRACK, pick.track);
    CHECK_TRUE(!pick.random_disabled);
}

static void testRollPicksTheAlternateAndKeepsRandomMusicOpen(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { OCCASIONAL_TRACK, RARE_TRACK, RARE_2_TRACK };
    GMRemixBgmStagePick main_pick = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_NONE, NONE, 10);
    GMRemixBgmStagePick rare_pick = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_NONE, NONE, 85);

    CHECK_EQ(MAIN_TRACK, main_pick.track);
    CHECK_EQ(RARE_TRACK, rare_pick.track);
    CHECK_TRUE(!main_pick.random_disabled);
    CHECK_TRUE(!rare_pick.random_disabled);
}

static void testRollOnAnEmptySlotPlaysTheMainSong(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { NONE, NONE, NONE };

    CHECK_EQ(MAIN_TRACK, gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_NONE, NONE, 95).track);
}

static void testPreservedSongReplacesTheRoll(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { OCCASIONAL_TRACK, RARE_TRACK, RARE_2_TRACK };
    GMRemixBgmStagePick pick = gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_NONE, RARE_2_TRACK, 10);

    CHECK_EQ(RARE_2_TRACK, pick.track);
    CHECK_TRUE(!pick.random_disabled);
}

static void testCButtonWinsOverThePreservedSong(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { OCCASIONAL_TRACK, RARE_TRACK, RARE_2_TRACK };

    CHECK_EQ(OCCASIONAL_TRACK,
             gmRemixBgmStageRulesPick(MAIN_TRACK, alternates, GMREMIX_BGM_STAGE_SLOT_OCCASIONAL, RARE_2_TRACK, NO_ROLL).track);
}

static void testDemoSkipsTheFinalDestinationIntro(void)
{
    CHECK_EQ(GMREMIX_BGM_STAGE_FD_MAIN, gmRemixBgmStageRulesDemoTrack(GMREMIX_BGM_STAGE_FD_INTRO));
    CHECK_EQ(MAIN_TRACK, gmRemixBgmStageRulesDemoTrack(MAIN_TRACK));
    CHECK_EQ(GMREMIX_BGM_STAGE_FD_MAIN, gmRemixBgmStageRulesDemoTrack(GMREMIX_BGM_STAGE_FD_MAIN));
}

void remixTestBgmStageRules(void)
{
    printf("bgm stage rules\n");
    RUN_TEST(testCButtonsForceTheirSlotInApplyOrder);
    RUN_TEST(testCUpWinsOverTheOtherCButtons);
    RUN_TEST(testOtherButtonsDoNotForceASong);
    RUN_TEST(testRollBoundariesFollowTheChances);
    RUN_TEST(testRollIsOnlyNeededWithoutCButtonOrPreservedSong);
    RUN_TEST(testCUpPlaysTheMainSongAndBlocksRandomMusic);
    RUN_TEST(testCButtonPlaysItsAlternateAndBlocksRandomMusic);
    RUN_TEST(testCButtonOnAnEmptySlotKeepsTheMainSongAndAllowsRandomMusic);
    RUN_TEST(testRollPicksTheAlternateAndKeepsRandomMusicOpen);
    RUN_TEST(testRollOnAnEmptySlotPlaysTheMainSong);
    RUN_TEST(testPreservedSongReplacesTheRoll);
    RUN_TEST(testCButtonWinsOverThePreservedSong);
    RUN_TEST(testDemoSkipsTheFinalDestinationIntro);
}
