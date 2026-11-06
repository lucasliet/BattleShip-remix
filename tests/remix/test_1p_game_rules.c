/**
 * test_1p_game_rules.c - unit tests of the 1P game rules of the Remix modes (sc/sc1pmode/sc1premixmusicrules.c,
 * sc1premixgamerules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "remix/remix_roster_data.h"
#include "sc/sc1pmode/sc1premixgamerules.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixmusicrules.h"

static void testHordeModesAndTheMiniBossStartTheMusicAtOnce(void)
{
    CHECK_TRUE(sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_MULTIMAN, 12));
    CHECK_TRUE(sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_CRUEL, 12));
    CHECK_TRUE(sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_HRC, 0));
    CHECK_TRUE(sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS));
}

static void testOtherRunsKeepTheMusicAtGo(void)
{
    CHECK_EQ(0, sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_STANDARD, SC1PREMIX_MUSIC_STAGE_MINI_BOSS));
    CHECK_EQ(0, sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_REMIX_1P, 12));
    CHECK_EQ(0, sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_FINAL));
    CHECK_EQ(0, sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_ALLSTAR, SC1PREMIX_MUSIC_STAGE_MINI_BOSS));
    CHECK_EQ(0, sc1PRemixModePlaysMusicAtOnce(SC1PREMIX_MODE_BONUS3, 11));
}

static void testMultimanPicksOneOfTwoSongs(void)
{
    CHECK_EQ(SC1PREMIX_MUSIC_MULTIMAN_ROLLS, sc1PRemixMusicRollCount(SC1PREMIX_MODE_MULTIMAN, 12, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(SC1PREMIX_BGM_MULTIMAN_2, sc1PRemixMusicPick(SC1PREMIX_MODE_MULTIMAN, 12, nGRRemixStage_BATTLEFIELD, 0));
    CHECK_EQ(SC1PREMIX_BGM_MULTIMAN, sc1PRemixMusicPick(SC1PREMIX_MODE_MULTIMAN, 12, nGRRemixStage_BATTLEFIELD, 1));
}

static void testCruelAndHrcForceTheirSongWithoutARoll(void)
{
    CHECK_EQ(0, sc1PRemixMusicRollCount(SC1PREMIX_MODE_CRUEL, 12, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(0x7F, sc1PRemixMusicPick(SC1PREMIX_MODE_CRUEL, 12, nGRRemixStage_BATTLEFIELD, 0));
    CHECK_EQ(0, sc1PRemixMusicRollCount(SC1PREMIX_MODE_HRC, 0, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(0x8E, sc1PRemixMusicPick(SC1PREMIX_MODE_HRC, 0, nGRRemixStage_BATTLEFIELD, 0));
}

static void testRemix1PFinalStagePlaysUltimateBowser(void)
{
    CHECK_EQ(0x4D, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_FINAL, nGRRemixStage_BATTLEFIELD, 0));
    CHECK_EQ(0, sc1PRemixMusicRollCount(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_FINAL, nGRRemixStage_BATTLEFIELD));
}

static void testSuperSonicStagesPickOneOfThreeSongs(void)
{
    int gkind = nGRRemixStage_GHZ;

    CHECK_EQ(SC1PREMIX_MUSIC_SONIC_ROLLS, sc1PRemixMusicRollCount(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, gkind));
    CHECK_EQ(SC1PREMIX_BGM_LIVE_AND_LEARN, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, gkind, 0));
    CHECK_EQ(SC1PREMIX_BGM_OPEN_YOUR_HEART, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, gkind, 1));
    CHECK_EQ(SC1PREMIX_BGM_EVERYTHING, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, gkind, 2));
    CHECK_EQ(SC1PREMIX_MUSIC_SONIC_ROLLS, sc1PRemixMusicRollCount(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, nGRRemixStage_CASINO));
    CHECK_EQ(SC1PREMIX_MUSIC_SONIC_ROLLS, sc1PRemixMusicRollCount(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, nGRRemixStage_MMADNESS));
}

static void testMadPianoStageKeepsItsHeaderSong(void)
{
    CHECK_EQ(0, sc1PRemixMusicRollCount(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(SC1PREMIX_MUSIC_KEEP_STAGE, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, nGRRemixStage_BATTLEFIELD, 0));
}

static void testDuoStagesForceTheirPairSong(void)
{
    CHECK_EQ(SC1PREMIX_BGM_ALOOF_SOLDIER, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_EDO, 0));
    CHECK_EQ(SC1PREMIX_BGM_DANGEROUS_FOE, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_ONETT, 0));
    CHECK_EQ(SC1PREMIX_BGM_DANGEROUS_FOE, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_NPC, 0));
    CHECK_EQ(SC1PREMIX_BGM_PIKA_CUP, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_POKEMON_STADIUM_2, 0));
    CHECK_EQ(SC1PREMIX_BGM_PIKA_CUP, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_GYM_LEADER_CASTLE, 0));
    CHECK_EQ(SC1PREMIX_BGM_PIKA_CUP, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_SAFFRON_DL, 0));
    CHECK_EQ(SC1PREMIX_BGM_WIZPIG, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_SPIRALM, 0));
    CHECK_EQ(SC1PREMIX_BGM_WIZPIG, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_MADMM, 0));
    CHECK_EQ(SC1PREMIX_BGM_WIZPIG, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_FROSTY, 0));
    CHECK_EQ(SC1PREMIX_BGM_WIZPIG, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_WINDY, 0));
    CHECK_EQ(SC1PREMIX_BGM_BATTLE_AMONG_FRIENDS, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_MT_DEDEDE, 0));
    CHECK_EQ(SC1PREMIX_BGM_BATTLE_AMONG_FRIENDS, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_DREAM_LAND_SR, 0));
    CHECK_EQ(SC1PREMIX_MUSIC_KEEP_STAGE, sc1PRemixMusicPick(SC1PREMIX_MODE_REMIX_1P, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_BATTLEFIELD, 0));
}

static void testStandardAndAllStarKeepTheHeaderSong(void)
{
    CHECK_EQ(SC1PREMIX_MUSIC_KEEP_STAGE, sc1PRemixMusicPick(SC1PREMIX_MODE_STANDARD, SC1PREMIX_MUSIC_STAGE_FINAL, nGRRemixStage_EDO, 0));
    CHECK_EQ(SC1PREMIX_MUSIC_KEEP_STAGE, sc1PRemixMusicPick(SC1PREMIX_MODE_ALLSTAR, SC1PREMIX_MUSIC_STAGE_DUO, nGRRemixStage_EDO, 0));
    CHECK_EQ(SC1PREMIX_MUSIC_KEEP_STAGE, sc1PRemixMusicPick(SC1PREMIX_MODE_BONUS3, 11, nGRRemixStage_REMIX_RTTF, 0));
    CHECK_EQ(0, sc1PRemixMusicRollCount(SC1PREMIX_MODE_ALLSTAR, SC1PREMIX_MUSIC_STAGE_MINI_BOSS, nGRRemixStage_GHZ));
}

static void testEveryFighterSpawnsFacingLeftOnTheRemixRaceCourse(void)
{
    CHECK_EQ(-1, sc1PRemixModeSpawnLR(nGRRemixStage_REMIX_RTTF, 1));
    CHECK_EQ(-1, sc1PRemixModeSpawnLR(nGRRemixStage_REMIX_RTTF, -1));
    CHECK_EQ(1, sc1PRemixModeSpawnLR(nGRRemixStage_BATTLEFIELD, 1));
    CHECK_EQ(-1, sc1PRemixModeSpawnLR(nGRRemixStage_BATTLEFIELD, -1));
}

static void testScreenFlashIsDimmedInBonus3AndTheHordeModes(void)
{
    CHECK_EQ(0x80, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_BONUS3, 0xFF));
    CHECK_EQ(0x80, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_MULTIMAN, 0xFF));
    CHECK_EQ(0x80, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_CRUEL, 0xFF));
    CHECK_EQ(0xFF, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_STANDARD, 0xFF));
    CHECK_EQ(0xFF, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_REMIX_1P, 0xFF));
    CHECK_EQ(0xFF, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_ALLSTAR, 0xFF));
    CHECK_EQ(0xFF, sc1PRemixModeFlashAlpha(SC1PREMIX_MODE_HRC, 0xFF));
}

static void testEnemyTraitFollowsTheMode(void)
{
    CHECK_EQ(8, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_BONUS3, 11, 0));
    CHECK_EQ(4, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_MULTIMAN, 12, 0));
    CHECK_EQ(4, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_CRUEL, 12, 0));
    CHECK_EQ(3, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_REMIX_1P, 6, 3));
    CHECK_EQ(5, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_STANDARD, 4, 5));
    CHECK_EQ(1, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_HRC, 0, 1));
}

static void testAllStarTripleBattleForcesTheStandardTrait(void)
{
    CHECK_EQ(0, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_ALLSTAR, 5, 0x1F));
    CHECK_EQ(2, sc1PRemixModeEnemyTrait(SC1PREMIX_MODE_ALLSTAR, 4, 2));
}

static void testHordeLevelsAreFourAndNine(void)
{
    CHECK_EQ(4, sc1PRemixModeHordeEnemyLevel(SC1PREMIX_MODE_MULTIMAN, 7));
    CHECK_EQ(9, sc1PRemixModeHordeEnemyLevel(SC1PREMIX_MODE_CRUEL, 1));
    CHECK_EQ(7, sc1PRemixModeHordeEnemyLevel(SC1PREMIX_MODE_STANDARD, 7));
    CHECK_EQ(7, sc1PRemixModeHordeEnemyLevel(SC1PREMIX_MODE_BONUS3, 7));
    CHECK_EQ(7, sc1PRemixModeHordeEnemyLevel(SC1PREMIX_MODE_REMIX_1P, 7));
}

static void testEnemyKosNeverCountInTheEndlessModes(void)
{
    CHECK_TRUE(sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_BONUS3));
    CHECK_TRUE(sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_MULTIMAN));
    CHECK_TRUE(sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_CRUEL));
    CHECK_TRUE(sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_HRC));
    CHECK_EQ(0, sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, sc1PRemixModeKeepsEnemyStocks(SC1PREMIX_MODE_ALLSTAR));
}

static void testVanillaRunsKeepOnlyTheTwentyVanillaItems(void)
{
    CHECK_EQ(0x000FFFFFu, sc1PRemixModeItemToggles(SC1PREMIX_MODE_STANDARD, 0xFFFFFFFFu));
    CHECK_EQ(0x000FFFFFu, sc1PRemixModeItemToggles(SC1PREMIX_MODE_MULTIMAN, 0xFFFFFFFFu));
    CHECK_EQ(0x00012345u, sc1PRemixModeItemToggles(SC1PREMIX_MODE_BONUS3, 0xABC12345u));
    CHECK_EQ(0, sc1PRemixModeExtendedItemToggles(SC1PREMIX_MODE_STANDARD, 0x7, 1));
    CHECK_EQ(0, sc1PRemixModeExtendedItemToggles(SC1PREMIX_MODE_HRC, 1, 1));
}

static void testRemix1PKeepsTheCustomItems(void)
{
    CHECK_EQ(0xFFFFFFFFu, sc1PRemixModeItemToggles(SC1PREMIX_MODE_REMIX_1P, 0xFFFFFFFFu));
    CHECK_EQ(0x7, sc1PRemixModeExtendedItemToggles(SC1PREMIX_MODE_REMIX_1P, 0x7, 1));
}

static void testAllStarRemovesTheHealingItemsAndUsesDefaultExtendedItems(void)
{
    CHECK_EQ(0xDFFFFFCFu, sc1PRemixModeItemToggles(SC1PREMIX_MODE_ALLSTAR, 0xFFFFFFFFu));
    CHECK_EQ(1, sc1PRemixModeExtendedItemToggles(SC1PREMIX_MODE_ALLSTAR, 0x7, 1));
}

static void testPolygonHumanOnAPolygonStageMakesTheEnemiesWearCostumeOne(void)
{
    CHECK_EQ(1, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NMARIO, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(1, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NNESS, nGRRemixStage_DUEL_ZONE));
    CHECK_EQ(1, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NWARIO, nGRRemixStage_RACE_TO_THE_FINISH));
    CHECK_EQ(1, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NCRASH, nGRRemixStage_REMIX_RTTF));
}

static void testPolygonCostumeNeedsAPolygonHumanWithTheDefaultCostume(void)
{
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(1, REMIX_ID_NMARIO, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, 0x00, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NMARIO - 1, nGRRemixStage_BATTLEFIELD));
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NNESS + 1, nGRRemixStage_BATTLEFIELD));
}

static void testPolygonCostumeNeedsAPolygonStage(void)
{
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NMARIO, nGRRemixStage_META_CRYSTAL));
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NMARIO, nGRRemixStage_FINAL_DESTINATION));
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NMARIO, nGRRemixStage_BATTLEFIELD_DL));
    CHECK_EQ(0, sc1PRemixModePolygonTeamCostume(0, REMIX_ID_NMARIO, nGRRemixStage_PEACHS_CASTLE));
}

static void testAllStarFinalWinBecomesTheGameClearStage(void)
{
    CHECK_TRUE(sc1PRemixModeEndsAllstarRun(SC1PREMIX_MODE_ALLSTAR, 0x0C, 1));
    CHECK_EQ(0, sc1PRemixModeEndsAllstarRun(SC1PREMIX_MODE_ALLSTAR, 0x0C, 0));
    CHECK_EQ(0, sc1PRemixModeEndsAllstarRun(SC1PREMIX_MODE_ALLSTAR, 4, 1));
    CHECK_EQ(0, sc1PRemixModeEndsAllstarRun(SC1PREMIX_MODE_REMIX_1P, 0x0C, 1));
    CHECK_EQ(0, sc1PRemixModeEndsAllstarRun(SC1PREMIX_MODE_STANDARD, 0x0C, 1));
}

static void testAllStarAwardsNeitherNoDamageNorCalamity(void)
{
    CHECK_EQ(0, sc1PRemixModeAwardsNoDamage(SC1PREMIX_MODE_ALLSTAR));
    CHECK_TRUE(sc1PRemixModeAwardsNoDamage(SC1PREMIX_MODE_STANDARD));
    CHECK_TRUE(sc1PRemixModeAwardsNoDamage(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_ALLSTAR));
}

static void testOnlyTheStandardRunAwardsBrosCalamity(void)
{
    CHECK_TRUE(sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(0, sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(0, sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(0, sc1PRemixModeAwardsBrosCalamity(SC1PREMIX_MODE_HRC));
}

void remixTest1PGameRules(void)
{
    printf("1p game rules\n");
    RUN_TEST(testHordeModesAndTheMiniBossStartTheMusicAtOnce);
    RUN_TEST(testOtherRunsKeepTheMusicAtGo);
    RUN_TEST(testMultimanPicksOneOfTwoSongs);
    RUN_TEST(testCruelAndHrcForceTheirSongWithoutARoll);
    RUN_TEST(testRemix1PFinalStagePlaysUltimateBowser);
    RUN_TEST(testSuperSonicStagesPickOneOfThreeSongs);
    RUN_TEST(testMadPianoStageKeepsItsHeaderSong);
    RUN_TEST(testDuoStagesForceTheirPairSong);
    RUN_TEST(testStandardAndAllStarKeepTheHeaderSong);
    RUN_TEST(testEveryFighterSpawnsFacingLeftOnTheRemixRaceCourse);
    RUN_TEST(testScreenFlashIsDimmedInBonus3AndTheHordeModes);
    RUN_TEST(testEnemyTraitFollowsTheMode);
    RUN_TEST(testAllStarTripleBattleForcesTheStandardTrait);
    RUN_TEST(testHordeLevelsAreFourAndNine);
    RUN_TEST(testEnemyKosNeverCountInTheEndlessModes);
    RUN_TEST(testVanillaRunsKeepOnlyTheTwentyVanillaItems);
    RUN_TEST(testRemix1PKeepsTheCustomItems);
    RUN_TEST(testAllStarRemovesTheHealingItemsAndUsesDefaultExtendedItems);
    RUN_TEST(testPolygonHumanOnAPolygonStageMakesTheEnemiesWearCostumeOne);
    RUN_TEST(testPolygonCostumeNeedsAPolygonHumanWithTheDefaultCostume);
    RUN_TEST(testPolygonCostumeNeedsAPolygonStage);
    RUN_TEST(testAllStarFinalWinBecomesTheGameClearStage);
    RUN_TEST(testAllStarAwardsNeitherNoDamageNorCalamity);
    RUN_TEST(testOnlyTheStandardRunAwardsBrosCalamity);
}
