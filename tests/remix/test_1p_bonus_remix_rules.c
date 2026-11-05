/**
 * test_1p_bonus_remix_rules.c - unit tests of the Remix bonus stage tables and the Remix 1P score tables (sc1pbonusremixrules.c,
 * sc1premixscorerules.c). Expected values come from Bonus.asm (get_bonus_stage_index_, handle_arrow_press_, check_input_,
 * update_stage_and_icons_), SinglePlayer.asm (extend_high_score_btt/btp_*, set_btt_stage_id_, extend_high_score_1p_*) and Stages.asm
 * (bonus_fix_3_).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1pbonusremixrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixscorerules.h"

#define CHECK_NEAR(expected, actual) CHECK_TRUE((((actual) - (expected)) < 0.001F) && (((expected) - (actual)) < 0.001F))

#define FIRST_ADDED_FKIND 12

static void testStageIndexIsTheTablePositionOrMinusOne(void)
{
    unsigned char stages[4] = { 0x11, 0x12, 0x4D, 0x53 };

    CHECK_EQ(0, sc1PBonusRemixStageIndexOf(stages, 4, 0x11));
    CHECK_EQ(3, sc1PBonusRemixStageIndexOf(stages, 4, 0x53));
    CHECK_EQ(-1, sc1PBonusRemixStageIndexOf(stages, 4, 0x99));
    CHECK_EQ(-1, sc1PBonusRemixStageIndexOf(stages, 3, 0x53));
}

static void testOwnStageUsesTheStandardRecords(void)
{
    CHECK_TRUE(sc1PBonusRemixIsDefaultStage(0x53, 0x53));
    CHECK_EQ(0, sc1PBonusRemixIsDefaultStage(0x11, 0x53));
}

static void testStageArrowsWrapOverTheTable(void)
{
    CHECK_EQ(1, sc1PBonusRemixStageStep(0, 1, SC1PBONUS_REMIX_STAGE_COUNT));
    CHECK_EQ(32, sc1PBonusRemixStageStep(0, -1, SC1PBONUS_REMIX_STAGE_COUNT));
    CHECK_EQ(0, sc1PBonusRemixStageStep(32, 1, SC1PBONUS_REMIX_STAGE_COUNT));
    CHECK_EQ(10, sc1PBonusRemixStageStep(11, -1, SC1PBONUS_REMIX_STAGE_COUNT));
}

static void testFailedRunKeepsTheHighestCount(void)
{
    CHECK_EQ(7, sc1PBonusRemixCountAfterRun(5, 7));
    CHECK_EQ(7, sc1PBonusRemixCountAfterRun(7, 5));
    CHECK_EQ(10, sc1PBonusRemixCountAfterRun(10, 3));
    CHECK_EQ(4, sc1PBonusRemixCountAfterRun(0, 4));
}

static void testUnfinishedRecordComparesAgainstTheUnsetTime(void)
{
    CHECK_EQ(SC1PBONUS_REMIX_TIME_UNSET, sc1PBonusRemixRecordTime(9, 123));
    CHECK_EQ(123, sc1PBonusRemixRecordTime(10, 123));
    CHECK_TRUE(sc1PBonusRemixIsTimeBeaten(0, 0, 5000));
    CHECK_EQ(0, sc1PBonusRemixIsTimeBeaten(0, 0, SC1PBONUS_REMIX_TIME_UNSET));
}

static void testFinishedRecordNeedsAStrictlyFasterTime(void)
{
    CHECK_TRUE(sc1PBonusRemixIsTimeBeaten(10, 3000, 2999));
    CHECK_EQ(0, sc1PBonusRemixIsTimeBeaten(10, 3000, 3000));
    CHECK_EQ(0, sc1PBonusRemixIsTimeBeaten(10, 3000, 3001));
}

static void testStageSourceFollowsTheMode(void)
{
    CHECK_EQ(SC1PBONUS_REMIX_SOURCE_REMIX_1P_TABLE, sc1PBonusRemixStageSource(1, 1, 3, FIRST_ADDED_FKIND));
    CHECK_EQ(SC1PBONUS_REMIX_SOURCE_SELECTED, sc1PBonusRemixStageSource(0, 1, 3, FIRST_ADDED_FKIND));
    CHECK_EQ(SC1PBONUS_REMIX_SOURCE_DEFAULT_TABLE, sc1PBonusRemixStageSource(0, 0, 40, FIRST_ADDED_FKIND));
    CHECK_EQ(SC1PBONUS_REMIX_SOURCE_VANILLA, sc1PBonusRemixStageSource(0, 0, 3, FIRST_ADDED_FKIND));
    CHECK_EQ(SC1PBONUS_REMIX_SOURCE_DEFAULT_TABLE, sc1PBonusRemixStageSource(0, 0, FIRST_ADDED_FKIND, FIRST_ADDED_FKIND));
}

static void testPlatformStagesAreFoundByClassOutsideTheVanillaRange(void)
{
    CHECK_TRUE(sc1PBonusRemixRulesIsPlatformStage(1, 1, 0));
    CHECK_EQ(0, sc1PBonusRemixRulesIsPlatformStage(0, 1, SC1PBONUS_REMIX_CLASS_BTP));
    CHECK_TRUE(sc1PBonusRemixRulesIsPlatformStage(0, 0, SC1PBONUS_REMIX_CLASS_BTP));
    CHECK_EQ(0, sc1PBonusRemixRulesIsPlatformStage(0, 0, SC1PBONUS_REMIX_CLASS_BTT));
}

static void testSetStageShowsOnlyWithTheRemixChoiceAndAHeldToken(void)
{
    CHECK_TRUE(sc1PBonusRemixRulesSetStageIsVisible(1, 1, 1, 1));
    CHECK_EQ(0, sc1PBonusRemixRulesSetStageIsVisible(0, 1, 1, 1));
    CHECK_EQ(0, sc1PBonusRemixRulesSetStageIsVisible(1, 0, 1, 1));
    CHECK_EQ(0, sc1PBonusRemixRulesSetStageIsVisible(1, 1, 0, 1));
    CHECK_EQ(0, sc1PBonusRemixRulesSetStageIsVisible(1, 1, 1, 0));
}

static void testLoneIconIsCentredAndAPairIsSpread(void)
{
    CHECK_NEAR(245.0F, sc1PBonusRemixIconX(1, 0));
    CHECK_NEAR(240.0F, sc1PBonusRemixIconX(2, 0));
    CHECK_NEAR(250.0F, sc1PBonusRemixIconX(2, 1));
}

static void testScoreTableFollowsTheMode(void)
{
    CHECK_EQ(SC1PREMIX_SCORE_TABLE_REMIX_1P, sc1PRemixScoreTableOf(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(SC1PREMIX_SCORE_TABLE_ALLSTAR, sc1PRemixScoreTableOf(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(SC1PREMIX_SCORE_TABLE_STANDARD, sc1PRemixScoreTableOf(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(SC1PREMIX_SCORE_TABLE_STANDARD, sc1PRemixScoreTableOf(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(SC1PREMIX_SCORE_TABLE_STANDARD, sc1PRemixScoreTableOf(SC1PREMIX_MODE_HRC));
}

static void testOnlyAHigherScoreReplacesTheStoredOne(void)
{
    CHECK_TRUE(sc1PRemixScoreReplaces(100, 101));
    CHECK_EQ(0, sc1PRemixScoreReplaces(100, 100));
    CHECK_EQ(0, sc1PRemixScoreReplaces(100, 99));
    CHECK_TRUE(sc1PRemixScoreReplaces(0, 1));
}

void remixTest1PBonusRemixRules(void)
{
    printf("1p bonus remix rules\n");
    RUN_TEST(testStageIndexIsTheTablePositionOrMinusOne);
    RUN_TEST(testOwnStageUsesTheStandardRecords);
    RUN_TEST(testStageArrowsWrapOverTheTable);
    RUN_TEST(testFailedRunKeepsTheHighestCount);
    RUN_TEST(testUnfinishedRecordComparesAgainstTheUnsetTime);
    RUN_TEST(testFinishedRecordNeedsAStrictlyFasterTime);
    RUN_TEST(testStageSourceFollowsTheMode);
    RUN_TEST(testPlatformStagesAreFoundByClassOutsideTheVanillaRange);
    RUN_TEST(testSetStageShowsOnlyWithTheRemixChoiceAndAHeldToken);
    RUN_TEST(testLoneIconIsCentredAndAPairIsSpread);
    RUN_TEST(testScoreTableFollowsTheMode);
    RUN_TEST(testOnlyAHigherScoreReplacesTheStoredOne);
}
