/**
 * test_bgm_swap_rules.c - unit tests of the Final Destination intro and Hurry Up swaps (gm/gmremixbgmswaprules.c).
 */

#include "test_check.h"

#include "gm/gmremixbgmrules.h"
#include "gm/gmremixbgmstagerules.h"
#include "gm/gmremixbgmswaprules.h"

#define NONE GMREMIX_BGM_NO_TRACK

static void testFinalDestinationStagesAreTheThreeClones(void)
{
    CHECK_TRUE(gmRemixBgmSwapRulesIsFinalDestination(0x10));
    CHECK_TRUE(gmRemixBgmSwapRulesIsFinalDestination(0x8F));
    CHECK_TRUE(gmRemixBgmSwapRulesIsFinalDestination(0x90));
    CHECK_TRUE(!gmRemixBgmSwapRulesIsFinalDestination(0x00));
    CHECK_TRUE(!gmRemixBgmSwapRulesIsFinalDestination(0x11));
}

static void testOnly1PStandardAndRemixRunBossFinalStage(void)
{
    CHECK_TRUE(gmRemixBgmSwapRulesIs1PFinalStage(GMREMIX_BGM_SWAP_1P_MODE_STANDARD, 13));
    CHECK_TRUE(gmRemixBgmSwapRulesIs1PFinalStage(GMREMIX_BGM_SWAP_1P_MODE_REMIX, 13));
    CHECK_TRUE(!gmRemixBgmSwapRulesIs1PFinalStage(GMREMIX_BGM_SWAP_1P_MODE_STANDARD, 12));
    CHECK_TRUE(!gmRemixBgmSwapRulesIs1PFinalStage(2, 13));
    CHECK_TRUE(!gmRemixBgmSwapRulesIs1PFinalStage(5, 13));
}

static void testIntroEndsExactly840UpdatesAfterTheSongStarted(void)
{
    CHECK_TRUE(gmRemixBgmSwapRulesFdIntroEnds(840, 0, GMREMIX_BGM_STAGE_FD_INTRO));
    CHECK_TRUE(gmRemixBgmSwapRulesFdIntroEnds(1240, 400, GMREMIX_BGM_STAGE_FD_INTRO));
    CHECK_TRUE(!gmRemixBgmSwapRulesFdIntroEnds(839, 0, GMREMIX_BGM_STAGE_FD_INTRO));
    CHECK_TRUE(!gmRemixBgmSwapRulesFdIntroEnds(841, 0, GMREMIX_BGM_STAGE_FD_INTRO));
}

static void testIntroDoesNotEndWhenTheStageSongChanged(void)
{
    CHECK_TRUE(!gmRemixBgmSwapRulesFdIntroEnds(840, 0, GMREMIX_BGM_STAGE_FD_MAIN));
    CHECK_TRUE(!gmRemixBgmSwapRulesFdIntroEnds(840, 0, 317));
}

static void testHurryTrackFollowsThePlayingSong(void)
{
    CHECK_EQ(3, gmRemixBgmSwapRulesHurryTrack(2, 2));
    CHECK_EQ(292, gmRemixBgmSwapRulesHurryTrack(179, 179));
    CHECK_EQ(316, gmRemixBgmSwapRulesHurryTrack(315, 315));
}

static void testHurryTrackIgnoresTheStageIdentity(void)
{
    CHECK_EQ(3, gmRemixBgmSwapRulesHurryTrack(2, 6));
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(6, 6));
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(0, 0));
}

static void testHurryTrackWaitsForItemMusicToEnd(void)
{
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(45, 2));
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(46, 315));
}

static void testHurryTrackDoesNotRepeat(void)
{
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(2, 3));
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(179, 292));
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(315, 316));
    CHECK_EQ(NONE, gmRemixBgmSwapRulesHurryTrack(3, 3));
}

void remixTestBgmSwapRules(void)
{
    printf("bgm swap rules\n");
    RUN_TEST(testFinalDestinationStagesAreTheThreeClones);
    RUN_TEST(testOnly1PStandardAndRemixRunBossFinalStage);
    RUN_TEST(testIntroEndsExactly840UpdatesAfterTheSongStarted);
    RUN_TEST(testIntroDoesNotEndWhenTheStageSongChanged);
    RUN_TEST(testHurryTrackFollowsThePlayingSong);
    RUN_TEST(testHurryTrackIgnoresTheStageIdentity);
    RUN_TEST(testHurryTrackWaitsForItemMusicToEnd);
    RUN_TEST(testHurryTrackDoesNotRepeat);
}
