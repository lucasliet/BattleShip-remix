/**
 * test_1p_leftover_rules.c - unit tests of the leftover rules of the Remix 1P modes: Bonus 3 (sc1premixbonus3rules.c), the credits song
 * of the own ending (sc1premixendingrules.c) and the Rest Area icons of All-Star (sc1premixallstarrestrules.c). Expected values come
 * from SinglePlayerModes.asm (_time_save, _new_record, _opponent_cpu, _opponent_knockback, load_player_stocks, bonus3_timer,
 * rest_area_setup) and BGM.asm (remix_1P_ending_song).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixallstarrestrules.h"
#include "sc/sc1pmode/sc1premixbonus3rules.h"
#include "sc/sc1pmode/sc1premixendingrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixmoderules.h"

#define CHECK_NEAR(expected, actual) CHECK_TRUE((((actual) - (expected)) < 0.001F) && (((expected) - (actual)) < 0.001F))

#define COMPLETE_FGM 0x1CB
#define NEW_RECORD_FGM 0x1D0

static void testBonus3UsesTheStrongestPolygonsAndOneStock(void)
{
    CHECK_EQ(9, SC1PREMIX_BONUS3_ENEMY_LEVEL);
    CHECK_EQ(9, SC1PREMIX_BONUS3_ENEMY_HANDICAP);
    CHECK_EQ(0, SC1PREMIX_BONUS3_PLAYER_STOCKS);
}

static void testBonus3LeavesTheBonusCssForTheRaceStage(void)
{
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_REMIX_1P));
}

static void testEmptyRecordSlotTakesTheTimeWithoutAFanfare(void)
{
    CHECK_EQ(SC1PREMIX_BONUS3_RECORD_FIRST, sc1PRemixBonus3RecordVerdict(0, 5000));
    CHECK_EQ(SC1PREMIX_BONUS3_RECORD_FIRST, sc1PRemixBonus3RecordVerdict(SC1PREMIX_BONUS3_RECORD_FILLER, 5000));
    CHECK_EQ(COMPLETE_FGM, sc1PRemixBonus3CompleteFgm(SC1PREMIX_BONUS3_RECORD_FIRST, COMPLETE_FGM, NEW_RECORD_FGM));
}

static void testOnlyAStrictlyFasterTimeBeatsTheRecord(void)
{
    CHECK_EQ(SC1PREMIX_BONUS3_RECORD_BEATEN, sc1PRemixBonus3RecordVerdict(5000, 4999));
    CHECK_EQ(SC1PREMIX_BONUS3_RECORD_KEEP, sc1PRemixBonus3RecordVerdict(5000, 5000));
    CHECK_EQ(SC1PREMIX_BONUS3_RECORD_KEEP, sc1PRemixBonus3RecordVerdict(5000, 5001));
}

static void testNewRecordReplacesTheCompleteSound(void)
{
    CHECK_EQ(NEW_RECORD_FGM, sc1PRemixBonus3CompleteFgm(SC1PREMIX_BONUS3_RECORD_BEATEN, COMPLETE_FGM, NEW_RECORD_FGM));
    CHECK_EQ(COMPLETE_FGM, sc1PRemixBonus3CompleteFgm(SC1PREMIX_BONUS3_RECORD_KEEP, COMPLETE_FGM, NEW_RECORD_FGM));
}

static void testRecordIsSavedOnlyWhenTheCpuPlaysTheEnemy(void)
{
    CHECK_TRUE(sc1PRemixBonus3CanSaveRecord(0));
    CHECK_EQ(0, sc1PRemixBonus3CanSaveRecord(1));
    CHECK_EQ(0, sc1PRemixBonus3CanSaveRecord(4));
}

static void testTimerMovesDownWithTheDragonKingHud(void)
{
    float digit_y;
    float symbol_y;

    sc1PRemixBonus3TimerY(0, &digit_y, &symbol_y);
    CHECK_NEAR(30.0F, digit_y);
    CHECK_NEAR(20.0F, symbol_y);

    sc1PRemixBonus3TimerY(1, &digit_y, &symbol_y);
    CHECK_NEAR(210.0F, digit_y);
    CHECK_NEAR(200.0F, symbol_y);

    sc1PRemixBonus3TimerY(2, &digit_y, &symbol_y);
    CHECK_NEAR(30.0F, digit_y);
}

static void testCreditsPlayTheFighterSongOnlyInRemix1PAndAllStar(void)
{
    CHECK_TRUE(sc1PRemixEndingUsesFighterSong(SC1PREMIX_MODE_REMIX_1P));
    CHECK_TRUE(sc1PRemixEndingUsesFighterSong(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(0, sc1PRemixEndingUsesFighterSong(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, sc1PRemixEndingUsesFighterSong(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(0, sc1PRemixEndingUsesFighterSong(SC1PREMIX_MODE_HRC));
}

static void testZeroTableEntryKeepsTheStandardCreditsSong(void)
{
    CHECK_EQ(0x24, sc1PRemixEndingPickSong(0x24, 0x28));
    CHECK_EQ(0x28, sc1PRemixEndingPickSong(0, 0x28));
}

static void testRestAreaShowsTheFightersLeft(void)
{
    CHECK_EQ(30, sc1PRemixAllstarRestIconCount(30, 0));
    CHECK_EQ(26, sc1PRemixAllstarRestIconCount(30, 4));
    CHECK_EQ(3, sc1PRemixAllstarRestIconCount(30, 27));
    CHECK_EQ(0, sc1PRemixAllstarRestIconCount(30, 30));
    CHECK_EQ(0, sc1PRemixAllstarRestIconCount(30, -1));
}

static void testRestAreaIconsFillTenToARow(void)
{
    float x;
    float y;

    sc1PRemixAllstarRestIconPosition(0, 0, &x, &y);
    CHECK_NEAR(30.0F, x);
    CHECK_NEAR(30.0F, y);

    sc1PRemixAllstarRestIconPosition(9, 0, &x, &y);
    CHECK_NEAR(111.0F, x);
    CHECK_NEAR(30.0F, y);

    sc1PRemixAllstarRestIconPosition(10, 0, &x, &y);
    CHECK_NEAR(30.0F, x);
    CHECK_NEAR(41.0F, y);

    sc1PRemixAllstarRestIconPosition(29, 0, &x, &y);
    CHECK_NEAR(111.0F, x);
    CHECK_NEAR(52.0F, y);
}

static void testRestAreaIconsMoveDownWithTheDragonKingHud(void)
{
    float x;
    float y;

    sc1PRemixAllstarRestIconPosition(0, 1, &x, &y);
    CHECK_NEAR(180.0F, y);

    sc1PRemixAllstarRestIconPosition(11, 1, &x, &y);
    CHECK_NEAR(39.0F, x);
    CHECK_NEAR(191.0F, y);
}

void remixTest1PLeftoverRules(void)
{
    printf("1p leftover rules\n");
    RUN_TEST(testBonus3UsesTheStrongestPolygonsAndOneStock);
    RUN_TEST(testBonus3LeavesTheBonusCssForTheRaceStage);
    RUN_TEST(testEmptyRecordSlotTakesTheTimeWithoutAFanfare);
    RUN_TEST(testOnlyAStrictlyFasterTimeBeatsTheRecord);
    RUN_TEST(testNewRecordReplacesTheCompleteSound);
    RUN_TEST(testRecordIsSavedOnlyWhenTheCpuPlaysTheEnemy);
    RUN_TEST(testTimerMovesDownWithTheDragonKingHud);
    RUN_TEST(testCreditsPlayTheFighterSongOnlyInRemix1PAndAllStar);
    RUN_TEST(testZeroTableEntryKeepsTheStandardCreditsSong);
    RUN_TEST(testRestAreaShowsTheFightersLeft);
    RUN_TEST(testRestAreaIconsFillTenToARow);
    RUN_TEST(testRestAreaIconsMoveDownWithTheDragonKingHud);
}
