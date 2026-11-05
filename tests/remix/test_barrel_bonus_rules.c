/**
 * test_barrel_bonus_rules.c - unit tests of the Remix barrel stage rules (gr/grremix/grremixbarrelrules.c) and of the bonus stage
 * hazard rules (gr/grremix/grremixbonusrules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixbarrelrules.h"
#include "gr/grremix/grremixbonusrules.h"
#include "gr/grremix/grremixids.h"

static void testCongoFamilyAndFallsMakeTheHazardBarrel(void)
{
    CHECK_EQ(GRREMIX_BARREL_STAGE_HAZARD, grRemixBarrelRulesGetStageKind(nGRRemixStage_CONGO_JUNGLE));
    CHECK_EQ(GRREMIX_BARREL_STAGE_HAZARD, grRemixBarrelRulesGetStageKind(nGRRemixStage_CONGOJ_DL));
    CHECK_EQ(GRREMIX_BARREL_STAGE_HAZARD, grRemixBarrelRulesGetStageKind(nGRRemixStage_CONGOJ_O));
    CHECK_EQ(GRREMIX_BARREL_STAGE_HAZARD, grRemixBarrelRulesGetStageKind(nGRRemixStage_FALLS));
}

static void testSmashketballHasItsOwnBarrelKind(void)
{
    CHECK_EQ(GRREMIX_BARREL_STAGE_SMASHKETBALL, grRemixBarrelRulesGetStageKind(nGRRemixStage_SMASHKETBALL));
}

static void testOtherStagesMakeNoBarrel(void)
{
    CHECK_EQ(GRREMIX_BARREL_STAGE_NONE, grRemixBarrelRulesGetStageKind(nGRRemixStage_PEACHS_CASTLE));
    CHECK_EQ(GRREMIX_BARREL_STAGE_NONE, grRemixBarrelRulesGetStageKind(nGRRemixStage_BTP_BANJO));
}

static void testSmashketballBarrelsFollowTheModeAndTheToggle(void)
{
    CHECK_EQ(1, grRemixBarrelRulesSmashketballIsMade(1, 0));
    CHECK_EQ(1, grRemixBarrelRulesSmashketballIsMade(0, 1));
    CHECK_EQ(0, grRemixBarrelRulesSmashketballIsMade(0, 0));
}

static void testSmashketballBarrelsAreHigherInTheMode(void)
{
    CHECK_EQ(2944.0F, grRemixBarrelRulesGetSmashketballY(1));
    CHECK_EQ(1775.0F, grRemixBarrelRulesGetSmashketballY(0));
}

static void testRollingBombSpawnersOfTheThreeBonusStages(void)
{
    float x = 0.0F;
    float y = 0.0F;
    int wait = 0;

    CHECK_EQ(1, grRemixBonusRulesGetRollingBomb(nGRRemixStage_BTT_MARINA, &x, &y, &wait));
    CHECK_EQ(4050.0F, x);
    CHECK_EQ(6000.0F, y);
    CHECK_EQ(GRREMIX_BONUS_BOMB_WAIT_MARINA, wait);

    CHECK_EQ(1, grRemixBonusRulesGetRollingBomb(nGRRemixStage_BTP_MTWO, &x, &y, &wait));
    CHECK_EQ(4800.0F, x);
    CHECK_EQ(4200.0F, y);
    CHECK_EQ(GRREMIX_BONUS_BOMB_WAIT_DEFAULT, wait);

    CHECK_EQ(1, grRemixBonusRulesGetRollingBomb(nGRRemixStage_BTP_CRASH, &x, &y, &wait));
    CHECK_EQ(3150.0F, x);
    CHECK_EQ(8100.0F, y);
    CHECK_EQ(GRREMIX_BONUS_BOMB_WAIT_DEFAULT, wait);
}

static void testOtherBonusStagesHaveNoRollingBomb(void)
{
    float x = 7.0F;
    float y = 8.0F;
    int wait = 9;

    CHECK_EQ(0, grRemixBonusRulesGetRollingBomb(nGRRemixStage_BTT_MARIO, &x, &y, &wait));
    CHECK_EQ(0, grRemixBonusRulesGetRollingBomb(nGRRemixStage_BTP_BANJO, &x, &y, &wait));
    CHECK_EQ(7.0F, x);
    CHECK_EQ(8.0F, y);
    CHECK_EQ(9, wait);
}

static void testPeachBreakTheTargetsAlwaysTakesTheSidePipe(void)
{
    CHECK_EQ(1125, grRemixBonusRulesGetDokanSideThreshold(nGRRemixStage_BTT_PEACH) * 1000.0F);
    CHECK_EQ(250, grRemixBonusRulesGetDokanSideThreshold(nGRRemixStage_BTT_MARIO) * 1000.0F);
    CHECK_EQ(250, grRemixBonusRulesGetDokanSideThreshold(nGRRemixStage_PEACHS_CASTLE) * 1000.0F);
}

void remixTestBarrelBonusRules(void)
{
    printf("barrel and bonus hazard rules\n");
    RUN_TEST(testCongoFamilyAndFallsMakeTheHazardBarrel);
    RUN_TEST(testSmashketballHasItsOwnBarrelKind);
    RUN_TEST(testOtherStagesMakeNoBarrel);
    RUN_TEST(testSmashketballBarrelsFollowTheModeAndTheToggle);
    RUN_TEST(testSmashketballBarrelsAreHigherInTheMode);
    RUN_TEST(testRollingBombSpawnersOfTheThreeBonusStages);
    RUN_TEST(testOtherBonusStagesHaveNoRollingBomb);
    RUN_TEST(testPeachBreakTheTargetsAlwaysTakesTheSidePipe);
}
