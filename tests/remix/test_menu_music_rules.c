/**
 * test_menu_music_rules.c - unit tests of the Menu Music rules (gm/gmremixmenumusicrules.c): the trap easter egg, the RANDOM
 * step of BGM.asm alt_menu_music_ and the Play Music resets of Toggles.asm _exit_sub_menu and update_bgm_volume.
 */

#include "test_check.h"

#include "gm/gmremixmenumusicrules.h"

static void testFourthPressInARowActivatesTheTrap(void)
{
    int count = 0;

    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(1, count);
    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(2, count);
    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(3, count);
    CHECK_TRUE(!gmRemixMenuMusicRulesTrapIsActive(count));
    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(GMREMIX_MENU_MUSIC_TRAP_ACTIVE, count);
    CHECK_TRUE(gmRemixMenuMusicRulesTrapIsActive(count));
}

static void testPressAfterTheTrapRestartsTheCountAtOne(void)
{
    int count = gmRemixMenuMusicRulesCountTrapPress(GMREMIX_MENU_MUSIC_TRAP_ACTIVE);

    CHECK_EQ(1, count);
    CHECK_TRUE(!gmRemixMenuMusicRulesTrapIsActive(count));
}

static void testTrapStaysActiveUntilAnotherPressCounts(void)
{
    CHECK_TRUE(gmRemixMenuMusicRulesTrapIsActive(GMREMIX_MENU_MUSIC_TRAP_ACTIVE));
    CHECK_TRUE(!gmRemixMenuMusicRulesTrapIsActive(0));
}

#define TEST_MENU_MUSIC_RANDOM 19
#define TEST_MENU_MUSIC_RANDOM_CLASSICS 20
#define TEST_MENU_MUSIC_MELEE 2
#define TEST_BGM_MAIN_MELEE 71

static void testRandomPressLandingOnSm64CountsFromOne(void)
{
    CHECK_EQ(1, gmRemixMenuMusicRulesCountAuditionPress(TEST_MENU_MUSIC_RANDOM, GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64, 3));
    CHECK_EQ(1, gmRemixMenuMusicRulesCountAuditionPress(TEST_MENU_MUSIC_RANDOM_CLASSICS, GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64,
                                                        GMREMIX_MENU_MUSIC_TRAP_ACTIVE));
}

static void testDirectSm64PressesStillReachTheTrap(void)
{
    CHECK_EQ(GMREMIX_MENU_MUSIC_TRAP_ACTIVE,
             gmRemixMenuMusicRulesCountAuditionPress(GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64,
                                                     GMREMIX_MENU_MUSIC_RULES_FILESELECT_SM64, 3));
}

static void testPressOnAnotherTrackClearsTheCount(void)
{
    CHECK_EQ(0, gmRemixMenuMusicRulesCountAuditionPress(TEST_MENU_MUSIC_RANDOM, TEST_MENU_MUSIC_MELEE, 3));
    CHECK_EQ(0, gmRemixMenuMusicRulesCountAuditionPress(TEST_MENU_MUSIC_MELEE, TEST_MENU_MUSIC_MELEE, GMREMIX_MENU_MUSIC_TRAP_ACTIVE));
}

static void testRandomFollowsTheScreenTrackWhileA64TrackPlays(void)
{
    CHECK_EQ(GMREMIX_MENU_MUSIC_RANDOM_SCREEN_TRACK, gmRemixMenuMusicRulesRandomStep(GMREMIX_MENU_MUSIC_RULES_BGM_MAIN));
    CHECK_EQ(GMREMIX_MENU_MUSIC_RANDOM_SCREEN_TRACK,
             gmRemixMenuMusicRulesRandomStep(GMREMIX_MENU_MUSIC_RULES_BGM_CHARACTER_SELECT));
}

static void testRandomRerollsAfterTheDataThemeOrSilence(void)
{
    CHECK_EQ(GMREMIX_MENU_MUSIC_RANDOM_REROLL, gmRemixMenuMusicRulesRandomStep(GMREMIX_MENU_MUSIC_RULES_BGM_DATA));
    CHECK_EQ(GMREMIX_MENU_MUSIC_RANDOM_REROLL, gmRemixMenuMusicRulesRandomStep(-1));
}

static void testRandomKeepsAnyOtherTrack(void)
{
    CHECK_EQ(GMREMIX_MENU_MUSIC_RANDOM_KEEP, gmRemixMenuMusicRulesRandomStep(TEST_BGM_MAIN_MELEE));
}

static void testSettingsCloseResetsWhenMusicIsOffOrSilent(void)
{
    CHECK_TRUE(gmRemixMenuMusicRulesSettingsCloseResets(0, TEST_BGM_MAIN_MELEE));
    CHECK_TRUE(gmRemixMenuMusicRulesSettingsCloseResets(1, -1));
    CHECK_TRUE(!gmRemixMenuMusicRulesSettingsCloseResets(1, TEST_BGM_MAIN_MELEE));
}

static void testBgmVolumeResetsOnlyWhenMusicIsOnAndSilent(void)
{
    CHECK_TRUE(gmRemixMenuMusicRulesVolumeResets(1, -1));
    CHECK_TRUE(!gmRemixMenuMusicRulesVolumeResets(0, -1));
    CHECK_TRUE(!gmRemixMenuMusicRulesVolumeResets(1, TEST_BGM_MAIN_MELEE));
}

static void testResultsBgmReplaysOnlyWhileTheScreenRuns(void)
{
    CHECK_TRUE(gmRemixMenuMusicRulesResultsBgmShouldReplay(1, 0));
    CHECK_TRUE(!gmRemixMenuMusicRulesResultsBgmShouldReplay(0, 0));
    CHECK_TRUE(!gmRemixMenuMusicRulesResultsBgmShouldReplay(1, 1));
    CHECK_TRUE(!gmRemixMenuMusicRulesResultsBgmShouldReplay(0, 1));
}

void remixTestMenuMusicRules(void)
{
    printf("menu music rules\n");
    RUN_TEST(testFourthPressInARowActivatesTheTrap);
    RUN_TEST(testPressAfterTheTrapRestartsTheCountAtOne);
    RUN_TEST(testTrapStaysActiveUntilAnotherPressCounts);
    RUN_TEST(testRandomPressLandingOnSm64CountsFromOne);
    RUN_TEST(testDirectSm64PressesStillReachTheTrap);
    RUN_TEST(testPressOnAnotherTrackClearsTheCount);
    RUN_TEST(testRandomFollowsTheScreenTrackWhileA64TrackPlays);
    RUN_TEST(testRandomRerollsAfterTheDataThemeOrSilence);
    RUN_TEST(testRandomKeepsAnyOtherTrack);
    RUN_TEST(testSettingsCloseResetsWhenMusicIsOffOrSilent);
    RUN_TEST(testBgmVolumeResetsOnlyWhenMusicIsOnAndSilent);
    RUN_TEST(testResultsBgmReplaysOnlyWhileTheScreenRuns);
}
