/**
 * test_gallery_rules.c - unit tests of the Remix Gallery rules (mn/mncommon/mnremixgalleryrules.c).
 */

#include "test_check.h"

#include "mn/mncommon/mnremixgalleryaudio.h"
#include "mn/mncommon/mnremixgalleryrules.h"
#include "mn/mncommon/mnremixsplashrules.h"
#include "remix/remix_roster_data.h"

static void testGalleryHasFortyPagesInAsmOrder(void)
{
    CHECK_EQ(40, MN_REMIX_GALLERY_ENTRY_COUNT);
    CHECK_EQ(REMIX_ID_MARIO, mnRemixGalleryRulesGetCharacterId(0));
    CHECK_EQ(REMIX_ID_DONKEY, mnRemixGalleryRulesGetCharacterId(1));
    CHECK_EQ(REMIX_ID_JIGGLY, mnRemixGalleryRulesGetCharacterId(11));
    CHECK_EQ(REMIX_ID_GND, mnRemixGalleryRulesGetCharacterId(12));
    CHECK_EQ(REMIX_ID_CRASH, mnRemixGalleryRulesGetCharacterId(28));
    CHECK_EQ(REMIX_ID_EBI, mnRemixGalleryRulesGetCharacterId(37));
    CHECK_EQ(REMIX_ID_PIANO, mnRemixGalleryRulesGetCharacterId(38));
    CHECK_EQ(REMIX_ID_BOSS, mnRemixGalleryRulesGetCharacterId(39));
}

static void testOutOfRangePagesHaveNoCharacter(void)
{
    CHECK_EQ(-1, mnRemixGalleryRulesGetCharacterId(-1));
    CHECK_EQ(-1, mnRemixGalleryRulesGetCharacterId(MN_REMIX_GALLERY_ENTRY_COUNT));
}

static void testEveryPageHasADistinctCharacter(void)
{
    int first;
    int second;

    for (first = 0; first < MN_REMIX_GALLERY_ENTRY_COUNT; first++)
    {
        for (second = first + 1; second < MN_REMIX_GALLERY_ENTRY_COUNT; second++)
        {
            CHECK_TRUE(mnRemixGalleryRulesGetCharacterId(first) != mnRemixGalleryRulesGetCharacterId(second));
        }
    }
}

static void testPagingWrapsAtBothEnds(void)
{
    CHECK_EQ(1, mnRemixGalleryRulesStepIndex(0, 1));
    CHECK_EQ(0, mnRemixGalleryRulesStepIndex(39, 1));
    CHECK_EQ(39, mnRemixGalleryRulesStepIndex(0, -1));
    CHECK_EQ(38, mnRemixGalleryRulesStepIndex(39, -1));
}

static void testVanillaCharactersUseTheVanillaPictureTable(void)
{
    CHECK_EQ(0, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_MARIO));
    CHECK_EQ(0, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_NESS));
}

static void testRemixCharactersUseTheirOwnPicture(void)
{
    CHECK_EQ(0xF1A, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_FALCO));
    CHECK_EQ(0x12C7, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_BANJO));
    CHECK_EQ(0x13FC, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_CRASH));
    CHECK_EQ(0xEE8, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_MTWO));
}

static void testLastEndingImageCallWins(void)
{
    CHECK_EQ(0x152F, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_LANKY));
}

static void testVariantsShareThePictureOfTheirBase(void)
{
    CHECK_EQ(mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_BOWSER), mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_GBOWSER));
    CHECK_EQ(mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_SLIPPY), mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_PEPPY));
    CHECK_EQ(0xB8, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_GDONKEY));
    CHECK_EQ(0xBA, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_METAL));
}

static void testCharactersWithoutAPictureUseTheDefault(void)
{
    CHECK_EQ(MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_BOSS));
    CHECK_EQ(MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_NMARIO));
    CHECK_EQ(MN_REMIX_VICTORY_IMAGE_DEFAULT_FILE, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_NCRASH));
}

static void testEveryGalleryPageHasAPicture(void)
{
    int page;
    int character_id;

    for (page = 0; page < MN_REMIX_GALLERY_ENTRY_COUNT; page++)
    {
        character_id = mnRemixGalleryRulesGetCharacterId(page);

        if (character_id >= MN_REMIX_GALLERY_VANILLA_CHARACTER_COUNT)
        {
            CHECK_TRUE(mnRemixGalleryRulesGetVictoryFileId(character_id) != 0);
        }
    }
}

static void testSplashImageIsTheRemixSplashFile(void)
{
    CHECK_EQ(MN_REMIX_SPLASH_IMAGE_FILE, mnRemixGalleryRulesGetVictoryFileId(REMIX_ID_NONE));
    CHECK_EQ(0x95E, MN_REMIX_SPLASH_IMAGE_FILE);
}

static void testFadesAndIdleTimeMatchTheAsmConstants(void)
{
    CHECK_EQ(45, MN_REMIX_GALLERY_FADE_NORMAL);
    CHECK_EQ(4, MN_REMIX_GALLERY_FADE_QUICK);
    CHECK_EQ(100, MN_REMIX_GALLERY_FADE_IDLE);
    CHECK_EQ(9000, MN_REMIX_GALLERY_IDLE_FRAMES);
}

static void testStartCyclesTheIdleMode(void)
{
    CHECK_EQ(MN_REMIX_GALLERY_IDLE_SHUFFLE, mnRemixGalleryRulesNextIdleMode(MN_REMIX_GALLERY_IDLE_OFF));
    CHECK_EQ(MN_REMIX_GALLERY_IDLE_RANDOM_SONG, mnRemixGalleryRulesNextIdleMode(MN_REMIX_GALLERY_IDLE_SHUFFLE));
    CHECK_EQ(MN_REMIX_GALLERY_IDLE_SHUFFLE, mnRemixGalleryRulesNextIdleMode(MN_REMIX_GALLERY_IDLE_RANDOM_SONG));
}

static void testIdleIndexStartsAtZeroAfterStartAndWraps(void)
{
    CHECK_EQ(0, mnRemixGalleryRulesNextIdleIndex(-1));
    CHECK_EQ(1, mnRemixGalleryRulesNextIdleIndex(0));
    CHECK_EQ(0, mnRemixGalleryRulesNextIdleIndex(MN_REMIX_GALLERY_ENTRY_COUNT - 1));
}

static int sFakeRandomState;

static int fakeRandomRange(int range)
{
    sFakeRandomState = (sFakeRandomState * 1103515245 + 12345) & 0x7FFFFFFF;
    return (sFakeRandomState >> 8) % range;
}

static void testIdleOrderHoldsEveryPageOnce(void)
{
    int order[MN_REMIX_GALLERY_ENTRY_COUNT];
    int seen[MN_REMIX_GALLERY_ENTRY_COUNT] = { 0 };
    int position;

    sFakeRandomState = 7;
    mnRemixGalleryRulesBuildIdleOrder(order, fakeRandomRange);

    for (position = 0; position < MN_REMIX_GALLERY_ENTRY_COUNT; position++)
    {
        CHECK_TRUE((order[position] >= 0) && (order[position] < MN_REMIX_GALLERY_ENTRY_COUNT));
        seen[order[position]]++;
    }
    for (position = 0; position < MN_REMIX_GALLERY_ENTRY_COUNT; position++)
    {
        CHECK_EQ(1, seen[position]);
    }
}

static void testEveryPageHasSongsAndTheLastNumberWraps(void)
{
    int page;

    for (page = 0; page < MN_REMIX_GALLERY_ENTRY_COUNT; page++)
    {
        int count = mnRemixGalleryAudioGetSongCount(page);

        CHECK_TRUE(count > 0);
        CHECK_TRUE(mnRemixGalleryAudioGetSong(page, count) > 0);
        CHECK_EQ(-1, mnRemixGalleryAudioGetSong(page, count + 1));
        CHECK_EQ(1, mnRemixGalleryAudioNextSongNumber(page, count));
        CHECK_EQ(1, mnRemixGalleryAudioNextSongNumber(page, 0));
    }
}

static void testSongCountsMatchTheAsmArrays(void)
{
    CHECK_EQ(12, mnRemixGalleryAudioGetSongCount(0));
    CHECK_EQ(10, mnRemixGalleryAudioGetSongCount(8));
    CHECK_EQ(13, mnRemixGalleryAudioGetSongCount(22));
    CHECK_EQ(1, mnRemixGalleryAudioGetSongCount(34));
    CHECK_EQ(1, mnRemixGalleryAudioGetSongCount(37));
    CHECK_EQ(11, mnRemixGalleryAudioGetSongCount(39));
    CHECK_EQ(0, mnRemixGalleryAudioGetSongCount(MN_REMIX_GALLERY_ENTRY_COUNT));
}

static void testSongNumberZeroIsTheGalleryBgm(void)
{
    CHECK_EQ(MN_REMIX_GALLERY_BGM_GALLERY, mnRemixGalleryAudioGetSong(5, 0));
    CHECK_EQ(281, MN_REMIX_GALLERY_BGM_GALLERY);
}

static void testFirstSongsComeFromTheAsmArrays(void)
{
    CHECK_EQ(6, mnRemixGalleryAudioGetSong(0, 1));
    CHECK_EQ(335, mnRemixGalleryAudioGetSong(0, 12));
    CHECK_EQ(213, mnRemixGalleryAudioGetSong(36, 1));
    CHECK_EQ(142, mnRemixGalleryAudioGetSong(39, 11));
}

static void testSongPicturesFollowTheAddBgmToGalleryCalls(void)
{
    CHECK_EQ(5, mnRemixGalleryAudioGetPageForBgm(0));
    CHECK_EQ(0, mnRemixGalleryAudioGetPageForBgm(6));
    CHECK_EQ(12, mnRemixGalleryAudioGetPageForBgm(47));
    CHECK_EQ(22, mnRemixGalleryAudioGetPageForBgm(194));
    CHECK_EQ(36, mnRemixGalleryAudioGetPageForBgm(129));
}

static void testSongsWithoutACharacterShowTheRemixPicture(void)
{
    CHECK_EQ(39, mnRemixGalleryAudioGetPageForBgm(10));
    CHECK_EQ(39, mnRemixGalleryAudioGetPageForBgm(46));
    CHECK_EQ(39, mnRemixGalleryAudioGetPageForBgm(213));
    CHECK_EQ(39, mnRemixGalleryAudioGetPageForBgm(281));
}

static void testDrumKitHasEighteenEntriesAndWraps(void)
{
    CHECK_EQ(18, MN_REMIX_GALLERY_DRUM_KIT_COUNT);
    CHECK_EQ(0x020, mnRemixGalleryAudioGetDrumFgm(0, 0));
    CHECK_EQ(0x01F, mnRemixGalleryAudioGetDrumFgm(0, 1));
    CHECK_EQ(0x547, mnRemixGalleryAudioGetDrumFgm(17, 0));
    CHECK_EQ(0x549, mnRemixGalleryAudioGetDrumFgm(17, 1));
    CHECK_EQ(-1, mnRemixGalleryAudioGetDrumFgm(18, 0));
    CHECK_EQ(1, mnRemixGalleryAudioNextDrumKit(0));
    CHECK_EQ(0, mnRemixGalleryAudioNextDrumKit(17));
}

static void testSplashJingleFollowsTheHeldLButtons(void)
{
    CHECK_EQ(0x0A0, mnRemixSplashRulesGetJingleFgm(0));
    CHECK_EQ(0x2C2, mnRemixSplashRulesGetJingleFgm(1));
    CHECK_EQ(0x2C2, mnRemixSplashRulesGetJingleFgm(3));
    CHECK_EQ(0x2F9, mnRemixSplashRulesGetJingleFgm(4));
}

static void testSplashLastsFiveHundredTwelveFramesThenEnds(void)
{
    int frames_left = MN_REMIX_SPLASH_FRAMES;
    int ticks = 0;

    CHECK_EQ(512, MN_REMIX_SPLASH_FRAMES);
    while (mnRemixSplashRulesTickCountdown(&frames_left) == 0)
    {
        ticks++;
    }
    CHECK_EQ(512, ticks);
    CHECK_TRUE(mnRemixSplashRulesTickCountdown(&frames_left));
}

void remixTestGalleryRules(void)
{
    printf("gallery rules\n");
    RUN_TEST(testSplashJingleFollowsTheHeldLButtons);
    RUN_TEST(testSplashLastsFiveHundredTwelveFramesThenEnds);
    RUN_TEST(testSplashImageIsTheRemixSplashFile);
    RUN_TEST(testFadesAndIdleTimeMatchTheAsmConstants);
    RUN_TEST(testStartCyclesTheIdleMode);
    RUN_TEST(testIdleIndexStartsAtZeroAfterStartAndWraps);
    RUN_TEST(testIdleOrderHoldsEveryPageOnce);
    RUN_TEST(testEveryPageHasSongsAndTheLastNumberWraps);
    RUN_TEST(testSongCountsMatchTheAsmArrays);
    RUN_TEST(testSongNumberZeroIsTheGalleryBgm);
    RUN_TEST(testFirstSongsComeFromTheAsmArrays);
    RUN_TEST(testSongPicturesFollowTheAddBgmToGalleryCalls);
    RUN_TEST(testSongsWithoutACharacterShowTheRemixPicture);
    RUN_TEST(testDrumKitHasEighteenEntriesAndWraps);
    RUN_TEST(testGalleryHasFortyPagesInAsmOrder);
    RUN_TEST(testOutOfRangePagesHaveNoCharacter);
    RUN_TEST(testEveryPageHasADistinctCharacter);
    RUN_TEST(testPagingWrapsAtBothEnds);
    RUN_TEST(testVanillaCharactersUseTheVanillaPictureTable);
    RUN_TEST(testRemixCharactersUseTheirOwnPicture);
    RUN_TEST(testLastEndingImageCallWins);
    RUN_TEST(testVariantsShareThePictureOfTheirBase);
    RUN_TEST(testCharactersWithoutAPictureUseTheDefault);
    RUN_TEST(testEveryGalleryPageHasAPicture);
}
