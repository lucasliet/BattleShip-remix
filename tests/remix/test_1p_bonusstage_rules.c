/**
 * test_1p_bonusstage_rules.c - unit tests of the Remix Bonus stage and bonus timer rules (sc/sc1pmode/sc1pbonusstagerules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1pbonusstagerules.h"

#define FIRST_ADDED_FKIND 12
#define NO_STAGE 0xFF

static void testNewRecordNeedsACompleteStoredRun(void)
{
    CHECK_EQ(0, sc1PBonusStageRulesIsNewRecord(0, 0xFFFFFFFFu, 100));
    CHECK_EQ(1, sc1PBonusStageRulesIsNewRecord(1, 500, 499));
}

static void testNewRecordNeedsAStrictlyFasterTime(void)
{
    CHECK_EQ(0, sc1PBonusStageRulesIsNewRecord(1, 500, 500));
    CHECK_EQ(0, sc1PBonusStageRulesIsNewRecord(1, 500, 501));
}

static void testOriginalFightersPlayTheirVanillaStageInThe1PGame(void)
{
    CHECK_EQ(0x11, sc1PBonusStageRulesGkindFor1PGame(0, FIRST_ADDED_FKIND, 0x40, NO_STAGE, 0x11));
    CHECK_EQ(0x1C, sc1PBonusStageRulesGkindFor1PGame(11, FIRST_ADDED_FKIND, 0x40, NO_STAGE, 0x1C));
}

static void testAddedFightersPlayTheirDefaultTableStageInThe1PGame(void)
{
    CHECK_EQ(0x40, sc1PBonusStageRulesGkindFor1PGame(FIRST_ADDED_FKIND, FIRST_ADDED_FKIND, 0x40, NO_STAGE, 0x1D));
    CHECK_EQ(0x41, sc1PBonusStageRulesGkindFor1PGame(30, FIRST_ADDED_FKIND, 0x41, NO_STAGE, 0x2B));
}

static void testAddedFighterWithoutAStageFallsBackToTheVanillaStage(void)
{
    CHECK_EQ(0x1D, sc1PBonusStageRulesGkindFor1PGame(FIRST_ADDED_FKIND, FIRST_ADDED_FKIND, NO_STAGE, NO_STAGE, 0x1D));
}

static void testBonusTimerBranchIsBonus3HrcOrPractice(void)
{
    CHECK_EQ(1, sc1PBonusStageRulesUsesCountUpTimer(1, 0, 0));
    CHECK_EQ(1, sc1PBonusStageRulesUsesCountUpTimer(0, 1, 0));
    CHECK_EQ(1, sc1PBonusStageRulesUsesCountUpTimer(0, 0, 1));
    CHECK_EQ(0, sc1PBonusStageRulesUsesCountUpTimer(0, 0, 0));
}

static void testHrcTimerCountsDownTenSeconds(void)
{
    CHECK_EQ(600, sc1PBonusStageRulesHrcTimerTics(0));
    CHECK_EQ(1, sc1PBonusStageRulesHrcTimerTics(599));
    CHECK_EQ(0, sc1PBonusStageRulesHrcTimerTics(600));
}

static void testHrcTimerStopsWhenTheTimeIsSpent(void)
{
    CHECK_EQ(-1, sc1PBonusStageRulesHrcTimerTics(601));
    CHECK_EQ(-1, sc1PBonusStageRulesHrcTimerTics(0xFFFFFFFFu));
}

void remixTest1PBonusStageRules(void)
{
    printf("1p bonus stage rules\n");
    RUN_TEST(testNewRecordNeedsACompleteStoredRun);
    RUN_TEST(testNewRecordNeedsAStrictlyFasterTime);
    RUN_TEST(testOriginalFightersPlayTheirVanillaStageInThe1PGame);
    RUN_TEST(testAddedFightersPlayTheirDefaultTableStageInThe1PGame);
    RUN_TEST(testAddedFighterWithoutAStageFallsBackToTheVanillaStage);
    RUN_TEST(testBonusTimerBranchIsBonus3HrcOrPractice);
    RUN_TEST(testHrcTimerCountsDownTenSeconds);
    RUN_TEST(testHrcTimerStopsWhenTheTimeIsSpent);
}
