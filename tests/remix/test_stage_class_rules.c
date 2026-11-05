/**
 * test_stage_class_rules.c - unit tests of the Remix stage class rules (gr/grremix/grremixstagerules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "gr/grremix/grremixstagerules.h"

#define SOME_HAZARD_FUNCTION 0x80109FB4u
#define BONUS_STAGE_FUNCTION 0u

static void testVanillaBattleIdsEndWithGameSetWhateverTheTable(void)
{
    CHECK_EQ(1, grRemixStageRulesEndsWithGameSet(nGRRemixStage_PEACHS_CASTLE, BONUS_STAGE_FUNCTION));
    CHECK_EQ(1, grRemixStageRulesEndsWithGameSet(nGRRemixStage_FINAL_DESTINATION, BONUS_STAGE_FUNCTION));
}

static void testBonusStageIdsEndWithFailure(void)
{
    CHECK_EQ(0, grRemixStageRulesEndsWithGameSet(nGRRemixStage_BTX_FIRST, BONUS_STAGE_FUNCTION));
    CHECK_EQ(0, grRemixStageRulesEndsWithGameSet(nGRRemixStage_BTT_MARIO, BONUS_STAGE_FUNCTION));
}

static void testRemixBattleStagesEndWithGameSetWhenTheyHaveAGroundFunction(void)
{
    CHECK_EQ(1, grRemixStageRulesEndsWithGameSet(nGRRemixStage_DRAGONKING, SOME_HAZARD_FUNCTION));
    CHECK_EQ(1, grRemixStageRulesEndsWithGameSet(nGRRemixStage_RAINBOWROAD, SOME_HAZARD_FUNCTION));
    CHECK_EQ(0, grRemixStageRulesEndsWithGameSet(nGRRemixStage_DRAGONKING, BONUS_STAGE_FUNCTION));
}

static void testHrcAndCrashStagesAlwaysEndWithFailure(void)
{
    CHECK_EQ(0, grRemixStageRulesEndsWithGameSet(nGRRemixStage_HRC, SOME_HAZARD_FUNCTION));
    CHECK_EQ(0, grRemixStageRulesEndsWithGameSet(nGRRemixStage_BTT_CRASH, SOME_HAZARD_FUNCTION));
    CHECK_EQ(0, grRemixStageRulesEndsWithGameSet(nGRRemixStage_BTP_CRASH, SOME_HAZARD_FUNCTION));
    CHECK_EQ(1, grRemixStageRulesEndsWithGameSet(nGRRemixStage_N64_REMIX, SOME_HAZARD_FUNCTION));
}

static void testVanillaBattleIdsPlayTheLargeExplosion(void)
{
    CHECK_EQ(0, grRemixStageRulesHasSmallExplosion(nGRRemixStage_PEACHS_CASTLE, 1));
    CHECK_EQ(0, grRemixStageRulesHasSmallExplosion(nGRRemixStage_FINAL_DESTINATION, 3));
}

static void testStagesOfAnotherClassPlayTheSmallExplosion(void)
{
    CHECK_EQ(1, grRemixStageRulesHasSmallExplosion(nGRRemixStage_BTT_MARIO, 3));
    CHECK_EQ(1, grRemixStageRulesHasSmallExplosion(nGRRemixStage_BTT_CRASH, 3));
    CHECK_EQ(1, grRemixStageRulesHasSmallExplosion(nGRRemixStage_BTP_CRASH, 2));
}

static void testRemixBattleClassPlaysTheLargeExplosion(void)
{
    CHECK_EQ(0, grRemixStageRulesHasSmallExplosion(nGRRemixStage_DRAGONKING, GRREMIX_STAGERULES_CLASS_BATTLE));
    CHECK_EQ(0, grRemixStageRulesHasSmallExplosion(nGRRemixStage_N64_REMIX, GRREMIX_STAGERULES_CLASS_BATTLE));
}

void remixTestStageClassRules(void)
{
    printf("stage class rules\n");
    RUN_TEST(testVanillaBattleIdsEndWithGameSetWhateverTheTable);
    RUN_TEST(testBonusStageIdsEndWithFailure);
    RUN_TEST(testRemixBattleStagesEndWithGameSetWhenTheyHaveAGroundFunction);
    RUN_TEST(testHrcAndCrashStagesAlwaysEndWithFailure);
    RUN_TEST(testVanillaBattleIdsPlayTheLargeExplosion);
    RUN_TEST(testStagesOfAnotherClassPlayTheSmallExplosion);
    RUN_TEST(testRemixBattleClassPlaysTheLargeExplosion);
}
