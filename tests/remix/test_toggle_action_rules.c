/**
 * test_toggle_action_rules.c - unit tests of the A-press routines of the Remix settings (gm/gmremixtoggleactionrules.c). Expected
 * values come from Toggles.asm punish_fgm_, update_bgm_volume, play_menu_music_, toggle_all_ and preview_bgm_.
 */

#include "test_check.h"

#include "gm/gmremixtoggleactionrules.h"

static void testEveryPunishValueHasItsOwnSound(void)
{
    CHECK_EQ(0x1AB, gmRemixToggleActionGetPunishFgm(1, 0));
    CHECK_EQ(0x1A4, gmRemixToggleActionGetPunishFgm(2, 0));
    CHECK_EQ(0x339, gmRemixToggleActionGetPunishFgm(3, 0));
    CHECK_EQ(0x4E5, gmRemixToggleActionGetPunishFgm(4, 0));
    CHECK_EQ(0x151, gmRemixToggleActionGetPunishFgm(5, 0));
    CHECK_EQ(0x437, gmRemixToggleActionGetPunishFgm(6, 0));
    CHECK_EQ(0x312, gmRemixToggleActionGetPunishFgm(7, 0));
    CHECK_EQ(0x24B, gmRemixToggleActionGetPunishFgm(8, 0));
    CHECK_EQ(0x23C, gmRemixToggleActionGetPunishFgm(9, 0));
    CHECK_EQ(0x456, gmRemixToggleActionGetPunishFgm(10, 0));
    CHECK_EQ(0x03A, gmRemixToggleActionGetPunishFgm(GMREMIX_PUNISH_MODE_RANDOM, 0));
}

static void testPunishOffIsSilentUnlessTheMenuMusicIsBlastCorps(void)
{
    CHECK_EQ(0, gmRemixToggleActionGetPunishFgm(0, 0));
    CHECK_EQ(0, gmRemixToggleActionGetPunishFgm(0, 7));
    CHECK_EQ(0x537, gmRemixToggleActionGetPunishFgm(0, GMREMIX_PUNISH_MENU_MUSIC_BLASTCORPS));
}

static void testPunishValuesOutOfRangeAreSilent(void)
{
    CHECK_EQ(0, gmRemixToggleActionGetPunishFgm(-1, GMREMIX_PUNISH_MENU_MUSIC_BLASTCORPS));
    CHECK_EQ(0, gmRemixToggleActionGetPunishFgm(GMREMIX_PUNISH_MODE_RANDOM + 1, 0));
}

static void testVolumeActionLevelMatchesTheAsm(void)
{
    CHECK_EQ(0x7800, GMREMIX_VOLUME_ACTION_LEVEL);
    CHECK_EQ(0x020, GMREMIX_VOLUME_ACTION_TEST_FGM);
}

static void testAuditionDefaultKeepsThe64TrackBelowTheChance(void)
{
    CHECK_EQ(GMREMIX_AUDITION_VALUE_64, gmRemixToggleActionGetAuditionDefaultValue(0));
    CHECK_EQ(GMREMIX_AUDITION_VALUE_64, gmRemixToggleActionGetAuditionDefaultValue(GMREMIX_AUDITION_CHANCE_64 - 1));
}

static void testAuditionDefaultPicksTheThreeSmashTracksAboveTheChance(void)
{
    CHECK_EQ(2, gmRemixToggleActionGetAuditionDefaultValue(GMREMIX_AUDITION_CHANCE_64));
    CHECK_EQ(3, gmRemixToggleActionGetAuditionDefaultValue(GMREMIX_AUDITION_CHANCE_64 + 1));
    CHECK_EQ(4, gmRemixToggleActionGetAuditionDefaultValue(GMREMIX_AUDITION_DEFAULT_ROLL_RANGE - 1));
}

static void testAuditionRollRangeIsTheChancePlusThreeTracks(void)
{
    CHECK_EQ(85, GMREMIX_AUDITION_CHANCE_64);
    CHECK_EQ(GMREMIX_AUDITION_CHANCE_64 + 3, GMREMIX_AUDITION_DEFAULT_ROLL_RANGE);
}

static void testToggleAllWritesTheOppositeOfTheFirstValue(void)
{
    CHECK_EQ(1, gmRemixToggleActionGetToggleAllValue(0));
    CHECK_EQ(0, gmRemixToggleActionGetToggleAllValue(1));
}

static void testPreviewRemembersTheTrackPlayingBeforeTheFirstPreview(void)
{
    CHECK_EQ(71, gmRemixToggleActionGetPreviewRemembered(GMREMIX_PREVIEW_IDLE, 71));
    CHECK_EQ(-1, gmRemixToggleActionGetPreviewRemembered(GMREMIX_PREVIEW_IDLE, -1));
}

static void testPreviewKeepsTheFirstRememberedTrackAcrossPreviews(void)
{
    CHECK_EQ(71, gmRemixToggleActionGetPreviewRemembered(71, 12));
    CHECK_EQ(-1, gmRemixToggleActionGetPreviewRemembered(-1, 12));
}

static void testPreviewRestoreStepFollowsWhatPlayedBefore(void)
{
    CHECK_EQ(GMREMIX_PREVIEW_RESTORE_NOTHING, gmRemixToggleActionGetPreviewRestoreStep(GMREMIX_PREVIEW_IDLE));
    CHECK_EQ(GMREMIX_PREVIEW_RESTORE_STOP, gmRemixToggleActionGetPreviewRestoreStep(-1));
    CHECK_EQ(GMREMIX_PREVIEW_RESTORE_PLAY, gmRemixToggleActionGetPreviewRestoreStep(0));
    CHECK_EQ(GMREMIX_PREVIEW_RESTORE_PLAY, gmRemixToggleActionGetPreviewRestoreStep(224));
}

void remixTestToggleActionRules(void)
{
    printf("toggle action rules\n");
    RUN_TEST(testEveryPunishValueHasItsOwnSound);
    RUN_TEST(testPunishOffIsSilentUnlessTheMenuMusicIsBlastCorps);
    RUN_TEST(testPunishValuesOutOfRangeAreSilent);
    RUN_TEST(testVolumeActionLevelMatchesTheAsm);
    RUN_TEST(testAuditionDefaultKeepsThe64TrackBelowTheChance);
    RUN_TEST(testAuditionDefaultPicksTheThreeSmashTracksAboveTheChance);
    RUN_TEST(testAuditionRollRangeIsTheChancePlusThreeTracks);
    RUN_TEST(testToggleAllWritesTheOppositeOfTheFirstValue);
    RUN_TEST(testPreviewRemembersTheTrackPlayingBeforeTheFirstPreview);
    RUN_TEST(testPreviewKeepsTheFirstRememberedTrackAcrossPreviews);
    RUN_TEST(testPreviewRestoreStepFollowsWhatPlayedBefore);
}
