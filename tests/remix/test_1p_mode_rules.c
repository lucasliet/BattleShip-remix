/**
 * test_1p_mode_rules.c - unit tests of the state-free Remix 1P rules (sc/sc1pmode/sc1premixmoderules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixmoderules.h"

static void testBonusModesLeaveTheBonusCSSFor1P(void)
{
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_BONUS3));
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_MULTIMAN));
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_CRUEL));
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_HRC));
}

static void testOtherModesKeepTheirOwnFlow(void)
{
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(-1));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_COUNT));
}

static void testSeedAdvanceIsOneToSixtyFourByFrameCount(void)
{
    CHECK_EQ(1, (int)sc1PRemixModeSeedAdvanceCount(0));
    CHECK_EQ(2, (int)sc1PRemixModeSeedAdvanceCount(1));
    CHECK_EQ(64, (int)sc1PRemixModeSeedAdvanceCount(63));
    CHECK_EQ(1, (int)sc1PRemixModeSeedAdvanceCount(64));
    CHECK_EQ(64, (int)sc1PRemixModeSeedAdvanceCount(0xFFFFFFFF));
}

static void testHordeModesAreMultimanAndCruelOnly(void)
{
    CHECK_TRUE(sc1PRemixModeIsMultiman(SC1PREMIX_MODE_MULTIMAN));
    CHECK_TRUE(sc1PRemixModeIsMultiman(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(0, sc1PRemixModeIsMultiman(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(0, sc1PRemixModeIsMultiman(SC1PREMIX_MODE_HRC));
    CHECK_EQ(0, sc1PRemixModeIsMultiman(SC1PREMIX_MODE_STANDARD));
}

static void testHandicapsFollowTheAsmKnockbackRatios(void)
{
    CHECK_EQ(0x15, sc1PRemixModeEnemyHandicap(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(0x18, sc1PRemixModeEnemyHandicap(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(0, sc1PRemixModeEnemyHandicap(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0x16, sc1PRemixModeHumanHandicap(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(0, sc1PRemixModeHumanHandicap(SC1PREMIX_MODE_MULTIMAN));
}

static void testKosCountUntilThePlayerIsKod(void)
{
    SC1PRemixMultimanKos counter;

    sc1PRemixMultimanKosReset(&counter);
    sc1PRemixMultimanKosAddEnemy(&counter);
    sc1PRemixMultimanKosAddEnemy(&counter);
    CHECK_EQ(2, counter.kos);

    sc1PRemixMultimanKosSetPlayerKo(&counter);
    sc1PRemixMultimanKosAddEnemy(&counter);
    CHECK_EQ(2, counter.kos);
}

static void testResetClearsTheKosAndThePlayerKo(void)
{
    SC1PRemixMultimanKos counter = { 7, 1 };

    sc1PRemixMultimanKosReset(&counter);
    CHECK_EQ(0, counter.kos);
    CHECK_EQ(0, counter.is_player_ko);
    sc1PRemixMultimanKosAddEnemy(&counter);
    CHECK_EQ(1, counter.kos);
}

static void testOnlyTheResetButtonRestartsTheRun(void)
{
    CHECK_EQ(1, sc1PRemixMultimanRestartsRun(1));
    CHECK_EQ(0, sc1PRemixMultimanRestartsRun(0));
}

static void testMultimanHudMovesDownOnlyOnTheAlwaysSetting(void)
{
    float polygon_y;
    float cross_y;
    float counter_y;

    sc1PRemixMultimanHudY(0, &polygon_y, &cross_y, &counter_y);
    CHECK_EQ(20, (int)polygon_y);
    CHECK_EQ(18, (int)cross_y);
    CHECK_EQ(18, (int)counter_y);
    sc1PRemixMultimanHudY(1, &polygon_y, &cross_y, &counter_y);
    CHECK_EQ(214, (int)polygon_y);
    CHECK_EQ(212, (int)cross_y);
    CHECK_EQ(212, (int)counter_y);
    sc1PRemixMultimanHudY(2, &polygon_y, &cross_y, &counter_y);
    CHECK_EQ(18, (int)counter_y);
}

static void testBonusStartCheerIsSkippedOnlyWhenTheToggleIsBonus(void)
{
    CHECK_EQ(0, sc1PRemixModeSkipsBonusStartCheer(0, SC1PREMIX_MODE_HRC, 1));
    CHECK_EQ(0, sc1PRemixModeSkipsBonusStartCheer(1, SC1PREMIX_MODE_BONUS3, 1));
    CHECK_EQ(1, sc1PRemixModeSkipsBonusStartCheer(2, SC1PREMIX_MODE_BONUS3, 0));
    CHECK_EQ(1, sc1PRemixModeSkipsBonusStartCheer(2, SC1PREMIX_MODE_HRC, 0));
}

static void testBonusStartCheerFollowsThePreviousScreen(void)
{
    CHECK_EQ(1, sc1PRemixModeSkipsBonusStartCheer(2, SC1PREMIX_MODE_STANDARD, 1));
    CHECK_EQ(1, sc1PRemixModeSkipsBonusStartCheer(2, SC1PREMIX_MODE_MULTIMAN, 1));
    CHECK_EQ(0, sc1PRemixModeSkipsBonusStartCheer(2, SC1PREMIX_MODE_STANDARD, 0));
    CHECK_EQ(0, sc1PRemixModeSkipsBonusStartCheer(2, SC1PREMIX_MODE_REMIX_1P, 0));
}

void remixTest1PModeRules(void)
{
    printf("1p mode rules\n");
    RUN_TEST(testBonusModesLeaveTheBonusCSSFor1P);
    RUN_TEST(testOtherModesKeepTheirOwnFlow);
    RUN_TEST(testSeedAdvanceIsOneToSixtyFourByFrameCount);
    RUN_TEST(testHordeModesAreMultimanAndCruelOnly);
    RUN_TEST(testHandicapsFollowTheAsmKnockbackRatios);
    RUN_TEST(testKosCountUntilThePlayerIsKod);
    RUN_TEST(testResetClearsTheKosAndThePlayerKo);
    RUN_TEST(testOnlyTheResetButtonRestartsTheRun);
    RUN_TEST(testMultimanHudMovesDownOnlyOnTheAlwaysSetting);
    RUN_TEST(testBonusStartCheerIsSkippedOnlyWhenTheToggleIsBonus);
    RUN_TEST(testBonusStartCheerFollowsThePreviousScreen);
}
