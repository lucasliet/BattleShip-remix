/**
 * test_ai_movement_rules.c - unit tests of the level 10 Remix AI movement rules (ft/ftremix/ftremixaimovementrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaimovementrules.h"

static void testFastfallDeniedWhenAllJumpsUsed(void)
{
    CHECK_EQ(0, ftRemixAiMovementRulesFastfallAllowed(2, 2, 0, 0, -3000.0F));
}

static void testFastfallDeniedWhenOpponentOffstage(void)
{
    CHECK_EQ(0, ftRemixAiMovementRulesFastfallAllowed(0, 2, 1, -1, -3000.0F));
}

static void testFastfallDeniedNearGround(void)
{
    CHECK_EQ(0, ftRemixAiMovementRulesFastfallAllowed(0, 2, 1, 5, -1999.0F));
    CHECK_EQ(0, ftRemixAiMovementRulesFastfallAllowed(0, 2, 1, 5, -2000.0F));
}

static void testFastfallAllowedMoreThan2000AboveGround(void)
{
    CHECK_TRUE(ftRemixAiMovementRulesFastfallAllowed(0, 2, 1, 5, -2001.0F));
}

static void testFastfallAllowedWithoutOpponent(void)
{
    CHECK_TRUE(ftRemixAiMovementRulesFastfallAllowed(0, 2, 0, -1, -3000.0F));
}

static void testShorthopPicksInputByHeightGap(void)
{
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK, ftRemixAiMovementRulesShorthopInput(300.0F, 4));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS, ftRemixAiMovementRulesShorthopInput(301.0F, 4));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS, ftRemixAiMovementRulesShorthopInput(600.0F, 4));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_FULL_C_JUMP_TOWARDS, ftRemixAiMovementRulesShorthopInput(900.0F, 4));
}

static void testShorthopKeepsVanillaFullHopAboveTheLastGap(void)
{
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_FULL_HOP, ftRemixAiMovementRulesShorthopInput(901.0F, FTREMIX_AI_MOVEMENT_INPUT_FULL_HOP));
}

static void testJumpInNeutralMapsRollsToInputs(void)
{
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS, ftRemixAiMovementRulesJumpInNeutralInput(0));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_TOWARDS, ftRemixAiMovementRulesJumpInNeutralInput(2));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_SHORT_HOP_IN_PLACE, ftRemixAiMovementRulesJumpInNeutralInput(3));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_FULL_C_JUMP_TOWARDS, ftRemixAiMovementRulesJumpInNeutralInput(4));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_FULL_C_JUMP_TOWARDS, ftRemixAiMovementRulesJumpInNeutralInput(5));
}

static void testJumpInNeutralDefaultsToPointStick(void)
{
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK, ftRemixAiMovementRulesJumpInNeutralInput(6));
    CHECK_EQ(FTREMIX_AI_MOVEMENT_INPUT_POINT_STICK, ftRemixAiMovementRulesJumpInNeutralInput(199));
}

static void testClampMovesOnlyTargetsBeyondTheEdges(void)
{
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetX(-100.0F, -500.0F, 500.0F) == -100.0F);
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetX(-900.0F, -500.0F, 500.0F) == -800.0F);
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetX(900.0F, -500.0F, 500.0F) == 800.0F);
}

static void testClampLeavesTargetsInsideTheStage(void)
{
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetX(0.0F, -500.0F, 500.0F) == 0.0F);
}

static void testClampYSubtractsPaddingAndStopsAtZero(void)
{
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetY(500.0F) == 300.0F);
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetY(200.0F) == 0.0F);
    CHECK_TRUE(ftRemixAiMovementRulesClampTargetY(-50.0F) == 0.0F);
}

static void testOffstageTargetsOnlyForLevelTen(void)
{
    CHECK_TRUE(ftRemixAiMovementRulesOffstageTargetAllowed(10));
    CHECK_EQ(0, ftRemixAiMovementRulesOffstageTargetAllowed(9));
}

void remixTestAiMovementRules(void)
{
    testFastfallDeniedWhenAllJumpsUsed();
    testFastfallDeniedWhenOpponentOffstage();
    testFastfallDeniedNearGround();
    testFastfallAllowedMoreThan2000AboveGround();
    testFastfallAllowedWithoutOpponent();
    testShorthopPicksInputByHeightGap();
    testShorthopKeepsVanillaFullHopAboveTheLastGap();
    testJumpInNeutralMapsRollsToInputs();
    testJumpInNeutralDefaultsToPointStick();
    testClampMovesOnlyTargetsBeyondTheEdges();
    testClampLeavesTargetsInsideTheStage();
    testClampYSubtractsPaddingAndStopsAtZero();
    testOffstageTargetsOnlyForLevelTen();
}
