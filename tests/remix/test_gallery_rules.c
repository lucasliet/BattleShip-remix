/**
 * test_gallery_rules.c - unit tests of the Remix Gallery rules (mn/mncommon/mnremixgalleryrules.c).
 */

#include "test_check.h"

#include "mn/mncommon/mnremixgalleryrules.h"
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

void remixTestGalleryRules(void)
{
    printf("gallery rules\n");
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
