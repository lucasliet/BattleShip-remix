/**
 * test_bgm_rules.c - unit tests of the Remix BGM rules (gm/gmremixbgmrules.c).
 */

#include "test_check.h"

#include "gm/gmremixbgmrules.h"

#define NONE GMREMIX_BGM_NO_TRACK
#define DEFAULT_TRACK 6

static void testItemMusicIsProtected(void)
{
    CHECK_TRUE(gmRemixBgmRulesIsProtected(GMREMIX_BGM_HAMMER));
    CHECK_TRUE(gmRemixBgmRulesIsProtected(GMREMIX_BGM_INVINCIBLE));
    CHECK_TRUE(!gmRemixBgmRulesIsProtected(DEFAULT_TRACK));
}

static void testDefaultStepsThroughTheAlternatesInOrder(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { 100, 101, 102 };

    CHECK_EQ(100, gmRemixBgmRulesNextAlternate(DEFAULT_TRACK, DEFAULT_TRACK, alternates));
    CHECK_EQ(101, gmRemixBgmRulesNextAlternate(100, DEFAULT_TRACK, alternates));
    CHECK_EQ(102, gmRemixBgmRulesNextAlternate(101, DEFAULT_TRACK, alternates));
    CHECK_EQ(DEFAULT_TRACK, gmRemixBgmRulesNextAlternate(102, DEFAULT_TRACK, alternates));
}

static void testEmptySlotsAreSkipped(void)
{
    int rare_only[GMREMIX_BGM_ALT_SLOTS] = { NONE, 101, NONE };
    int occasional_only[GMREMIX_BGM_ALT_SLOTS] = { 100, NONE, NONE };
    int none[GMREMIX_BGM_ALT_SLOTS] = { NONE, NONE, NONE };

    CHECK_EQ(101, gmRemixBgmRulesNextAlternate(DEFAULT_TRACK, DEFAULT_TRACK, rare_only));
    CHECK_EQ(DEFAULT_TRACK, gmRemixBgmRulesNextAlternate(101, DEFAULT_TRACK, rare_only));
    CHECK_EQ(100, gmRemixBgmRulesNextAlternate(DEFAULT_TRACK, DEFAULT_TRACK, occasional_only));
    CHECK_EQ(DEFAULT_TRACK, gmRemixBgmRulesNextAlternate(100, DEFAULT_TRACK, occasional_only));
    CHECK_EQ(DEFAULT_TRACK, gmRemixBgmRulesNextAlternate(DEFAULT_TRACK, DEFAULT_TRACK, none));
}

static void testRareTwoIsOnlyReachedFromRare(void)
{
    int rare_two_only[GMREMIX_BGM_ALT_SLOTS] = { NONE, NONE, 102 };

    CHECK_EQ(DEFAULT_TRACK, gmRemixBgmRulesNextAlternate(DEFAULT_TRACK, DEFAULT_TRACK, rare_two_only));
}

static void testAnyOtherTrackReturnsToTheDefault(void)
{
    int alternates[GMREMIX_BGM_ALT_SLOTS] = { 100, 101, 102 };

    CHECK_EQ(DEFAULT_TRACK, gmRemixBgmRulesNextAlternate(250, DEFAULT_TRACK, alternates));
}

static void testTimeTwisterChangesOnlyOnItsOwnSong(void)
{
    CHECK_TRUE(gmRemixBgmRulesTimeTwisterChanges(317));
    CHECK_TRUE(!gmRemixBgmRulesTimeTwisterChanges(318));
}

static void testTimeTwisterTracksFollowTheBackgrounds(void)
{
    CHECK_EQ(318, gmRemixBgmRulesTimeTwisterTrack(0));
    CHECK_EQ(359, gmRemixBgmRulesTimeTwisterTrack(1));
    CHECK_EQ(360, gmRemixBgmRulesTimeTwisterTrack(2));
    CHECK_EQ(317, gmRemixBgmRulesTimeTwisterTrack(3));
    CHECK_EQ(NONE, gmRemixBgmRulesTimeTwisterTrack(4));
    CHECK_EQ(NONE, gmRemixBgmRulesTimeTwisterTrack(-1));
}

void remixTestBgmRules(void)
{
    printf("bgm rules\n");
    RUN_TEST(testItemMusicIsProtected);
    RUN_TEST(testDefaultStepsThroughTheAlternatesInOrder);
    RUN_TEST(testEmptySlotsAreSkipped);
    RUN_TEST(testRareTwoIsOnlyReachedFromRare);
    RUN_TEST(testAnyOtherTrackReturnsToTheDefault);
    RUN_TEST(testTimeTwisterChangesOnlyOnItsOwnSong);
    RUN_TEST(testTimeTwisterTracksFollowTheBackgrounds);
}
