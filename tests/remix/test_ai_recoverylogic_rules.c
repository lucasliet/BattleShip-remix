/**
 * test_ai_recoverylogic_rules.c - unit tests of the Remix AI recovery_logic routines
 * (ft/ftremix/ftremixairecoverylogic*rules.c): the shared families and the single fighters.
 */

#include <string.h>

#include "test_check.h"

#include "ft/ftremix/ftremixairecoverylogicids.h"
#include "ft/ftremix/ftremixairecoverylogicrules.h"

/* A fighter at (-4000, -100) falling, with the nearest ledge (left) at (-1000, 0), 3000 units away and 100 above. */
static FTRemixAiRecoveryLogicState makeState(void)
{
    FTRemixAiRecoveryLogicState state;

    memset(&state, 0, sizeof(state));
    state.level_ten = 1;
    state.lr = 1;
    state.jumps_max = 2;
    state.x = -4000.0F;
    state.y = -100.0F;
    state.vel_y = -10.0F;
    state.air_speed_max_x = 100.0F;
    state.air_friction = 3.0F;
    state.cliff_catch_y = 20.0F;
    state.left_x = -1000.0F;
    state.left_y = 0.0F;
    state.right_x = 1000.0F;
    state.right_y = 0.0F;
    state.target_x = -1000.0F;
    state.target_y = 0.0F;
    return state;
}

static FTRemixAiRecoveryLogicResult run(int kind, const FTRemixAiRecoveryLogicState *state)
{
    FTRemixAiRecoveryLogicResult result;

    ftRemixAiRecoveryLogicRun(kind, state, &result);
    return result;
}

static void testNearestLedgeIsTheLeftOneOnATie(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    float ledge_x;
    float ledge_y;

    state.x = 0.0F;
    state.left_x = -500.0F;
    state.right_x = 500.0F;
    state.left_y = 7.0F;
    ftRemixAiRecoveryLogicNearestLedge(&state, &ledge_x, &ledge_y);
    CHECK_TRUE(ledge_x == -500.0F);
    CHECK_TRUE(ledge_y == 7.0F);
}

static void testNearestLedgeIsTheRightOneWhenCloser(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    float ledge_x;
    float ledge_y;

    state.x = 900.0F;
    ftRemixAiRecoveryLogicNearestLedge(&state, &ledge_x, &ledge_y);
    CHECK_TRUE(ledge_x == 1000.0F);
}

static void testUnknownKindDoesNothing(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_NONE, &state).command);
}

static void testSharedFamiliesNeedLevelTen(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.level_ten = 0;
    state.y = 5000.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state).command);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_LINK, &state).command);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF, &state).command);
}

static void testMarioFarBelowUsesNspTowardsAndTargetsTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.y = 3000.0F;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, result.command);
    CHECK_TRUE(result.sets_target);
    CHECK_TRUE(result.target_x == -1000.0F);
}

static void testMarioFarAndUnderTheLedgeUsesDsp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -3600.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state).command);
}

static void testMarioKeepsTheTornadoOnceUsed(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -3600.0F;
    state.word_adc = 1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state).command);
}

static void testMarioDoesNotUseDspWhenCloseThanTheMinimum(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -2000.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state).command);
}

static void testMarioTornadoMashesBOnOddFramesAndReleasesOnEven(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult odd;
    FTRemixAiRecoveryLogicResult even;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_MARIO_TORNADO_AIR;
    state.status_frame = 10;
    state.buttons = 0x0001;
    state.frame_is_odd = 1;
    odd = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state);
    state.frame_is_odd = 0;
    even = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state);

    CHECK_EQ(0x4001, odd.buttons);
    CHECK_EQ(0, even.buttons);
    CHECK_TRUE(odd.sets_buttons && even.sets_buttons);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET, odd.command);
}

static void testMarioTornadoStopsMashingFromFrameFifty(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_MARIO_TORNADO_AIR;
    state.status_frame = 50;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARIO, &state).command);
}

static void testPikachuFirstZipWithinReachPointsAtTheTarget(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_PIKACHU_QUICK_ATTACK_START_AIR;
    state.x = -1500.0F;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PIKACHU, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET, result.command);
    CHECK_EQ(0, result.sets_target);
}

static void testPikachuFirstZipFarAimsAboveTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_PIKACHU_QUICK_ATTACK_START_AIR;
    state.x = -6000.0F;
    state.y = 0.0F;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PIKACHU, &state);
    CHECK_TRUE(result.sets_target);
    CHECK_TRUE(result.target_x > state.x);
    CHECK_TRUE(result.target_y > state.y);
}

static void testPikachuSecondZipPointsAtTheTarget(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_PIKACHU_QUICK_ATTACK_AIR;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PIKACHU, &state).command);
}

static void testPuffPoundHoldsUpAndTowardTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JIGGLY_POUND_AIR;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NULL, result.command);
    CHECK_EQ(80, result.stick_y);
    CHECK_EQ(80, result.stick_x);
}

static void testPuffPoundHoldsLeftWhenTheLedgeIsOnTheLeft(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JIGGLY_POUND_AIR;
    state.x = 0.0F;
    CHECK_EQ(-80, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF, &state).stick_x);
}

static void testPuffMidJumpFarFromTheLedgeUsesNsp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JIGGLY_JUMP2;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF, &state).command);
}

static void testPuffLastJumpDoesNotUseNsp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JIGGLY_JUMP5 + 1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PUFF, &state).command);
}

static void testYoshiUsesDspWhenTheGrabBoxMissesTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F;
    state.vel_y = 10.0F;
    state.x = -1500.0F;
    state.y = -300.0F;
    state.cliff_catch_y = 600.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI, &state).command);
}

static void testYoshiNeedsTheDoubleJumpAndRisingSpeed(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.vel_y = 10.0F;
    state.x = -1500.0F;
    state.y = -300.0F;
    state.cliff_catch_y = 600.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI, &state).command);
    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F;
    state.vel_y = -1.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_YOSHI, &state).command);
}

static void testCaptainFalconPunchHoldsUp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_CAPTAIN_FALCON_PUNCH_AIR;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_CAPTAIN, &state);
    CHECK_EQ(80, result.stick_y);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NULL, result.command);
}

static void testCaptainUsesNspRightAfterTheDoubleJumpAtMaxSpeed(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F;
    state.x = -6000.0F;
    state.y = 500.0F;
    state.vel_x = 100.0F;
    state.status_frame = 3;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_CAPTAIN, &state).command);
}

static void testCaptainSkipsNspBelowMaxAirSpeed(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F;
    state.x = -6000.0F;
    state.y = 500.0F;
    state.vel_x = 50.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_CAPTAIN, &state).command);
}

static void testLinkSkipsDspWhileHoldingAnItem(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -6000.0F;
    state.y = 3000.0F;
    state.has_item = 1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_LINK, &state).command);
    state.has_item = 0;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_LINK, &state).command);
}

static void testFoxFirstUsesDspOnAOneInTwentyRoll(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, result.command);
    CHECK_EQ(20, result.roll_modulus);
}

static void testFoxIgnoresLedgesBeyondReachWithNoJumps(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -4800.0F;
    state.jumps_used = 2;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX, &state).command);
    state.jumps_used = 0;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX, &state).command);
}

static void testFoxLeavesFireFoxAloneAndWaitsToFall(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_FOX_FIRE_FOX_FIRST;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX, &state).command);
    state.action_id = 0;
    state.vel_y = 5.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FOX, &state).command);
}

static void testBanjoFairsWhenFacingTheExpectedWay(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.y = -500.0F;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_FTILT, result.command);
    CHECK_TRUE(result.sets_stick_y);
    CHECK_EQ(0, result.stick_y);
    state.lr = -1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO, &state).command);
}

static void testBanjoSkipsDuringFairAndWhileJumping(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.y = -500.0F;
    state.action_id = FTREMIX_AI_RLOGIC_ACTION_ATTACK_AIR_F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO, &state).command);
    state.action_id = 0;
    state.jump_command_active = 1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO, &state).command);
}

static void testBanjoRunsAtAnyLevel(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.level_ten = 0;
    state.y = -500.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_FTILT, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_BANJO, &state).command);
}

static void testWarioBodySlamHoldsUp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_WARIO_BODY_SLAM_AIR;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_WARIO, &state);
    CHECK_EQ(80, result.stick_y);
}

static void testWarioUsesNspFarAndNotMuchBelowTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -6000.0F;
    state.y = -100.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_WARIO, &state).command);
}

static void testMarthPointsTheStickDuringTheFirstNsp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_MARTH_NSPA_1;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARTH, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_POINT_STICK_TO_TARGET, result.command);
    CHECK_TRUE(result.sets_target);
}

static void testMarthNeedsMaxAirSpeedForNsp(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.vel_x = 50.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARTH, &state).command);
    state.vel_x = 100.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARTH, &state).command);
}

static void testCrashNeedsTheDoubleJumpAndDistance(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_CRASH, &state).command);
    state.action_id = FTREMIX_AI_RLOGIC_ACTION_JUMP_AERIAL_F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_CRASH, &state).command);
    state.x = -2500.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_CRASH, &state).command);
}

static void testDededeUsesDspFarBelowUnlessBothMinionsAreOut(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -3000.0F;
    state.y = 2000.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE, &state).command);
    state.minions_full = 1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE, &state).command);
}

static void testDededeSkipsTooFarLedges(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -5000.0F;
    state.y = 2000.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_DEDEDE, &state).command);
}

static void testFalcoPhantasmHoldsB(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_FALCO_PHANTASM_AIR;
    state.buttons = 0x0001;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FALCO, &state);
    CHECK_EQ(0x4001, result.buttons);
    CHECK_TRUE(result.sets_buttons);
}

static void testFalcoUsesNspOnAOneInEightRollWhenJustBelowTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult result;

    state.x = -2500.0F;
    state.y = -10.0F;
    state.cliff_catch_y = 30.0F;
    result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FALCO, &state);
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, result.command);
    CHECK_EQ(8, result.roll_modulus);
}

static void testFalcoSkipsWhenTheGrabBoxStaysBelowTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -2500.0F;
    state.y = -300.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_FALCO, &state).command);
}

static void testSheikDspsOnceWhenFacingTheFarLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -6000.0F;
    state.left_x = -1000.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_DSP, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_SHEIK, &state).command);
    state.word_adc = 1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_SHEIK, &state).command);
}

static void testSheikFacingAwayFromTheLedgeDoesNothing(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.x = -6000.0F;
    state.lr = -1;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_SHEIK, &state).command);
}

static void testMarinaUsesNspWhenFarAndFarAboveTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.y = 2500.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARINA, &state).command);
    state.vel_y = 5.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_MARINA, &state).command);
}

static void testSonicUsesNspAboveTheLedgeHeightWithAJumpLeft(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.y = 10.0F;
    state.jumps_used = 0;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_SONIC, &state).command);
    state.y = -100.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_COMMAND_NONE, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_SONIC, &state).command);
}

static void testSonicWithoutJumpsNeedsToBeWellAboveTheLedge(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.y = 1600.0F;
    state.jumps_used = 2;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_SONIC, &state).command);
}

static void testPeachFloatsTowardFarLedgesAndReleasesNearBy(void)
{
    FTRemixAiRecoveryLogicState state = makeState();
    FTRemixAiRecoveryLogicResult far_result;
    FTRemixAiRecoveryLogicResult near_result;

    state.action_id = FTREMIX_AI_RLOGIC_ACTION_PEACH_FLOAT;
    state.buttons = 0x0005;
    far_result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PEACH, &state);
    state.x = -1500.0F;
    near_result = run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PEACH, &state);

    CHECK_EQ(0x000D, far_result.buttons);
    CHECK_EQ(0x0000, near_result.buttons);
}

static void testPeachUsesNspFarAboveTheLedgeWhenNotFloating(void)
{
    FTRemixAiRecoveryLogicState state = makeState();

    state.y = 2500.0F;
    CHECK_EQ(FTREMIX_AI_RECOVERY_LOGIC_ROUTINE_NSP_TOWARDS, run(FTREMIX_AI_RECOVERY_LOGIC_KIND_PEACH, &state).command);
}

void remixTestAiRecoveryLogicRules(void)
{
    printf("ai recovery logic rules\n");
    RUN_TEST(testNearestLedgeIsTheLeftOneOnATie);
    RUN_TEST(testNearestLedgeIsTheRightOneWhenCloser);
    RUN_TEST(testUnknownKindDoesNothing);
    RUN_TEST(testSharedFamiliesNeedLevelTen);
    RUN_TEST(testMarioFarBelowUsesNspTowardsAndTargetsTheLedge);
    RUN_TEST(testMarioFarAndUnderTheLedgeUsesDsp);
    RUN_TEST(testMarioKeepsTheTornadoOnceUsed);
    RUN_TEST(testMarioDoesNotUseDspWhenCloseThanTheMinimum);
    RUN_TEST(testMarioTornadoMashesBOnOddFramesAndReleasesOnEven);
    RUN_TEST(testMarioTornadoStopsMashingFromFrameFifty);
    RUN_TEST(testPikachuFirstZipWithinReachPointsAtTheTarget);
    RUN_TEST(testPikachuFirstZipFarAimsAboveTheLedge);
    RUN_TEST(testPikachuSecondZipPointsAtTheTarget);
    RUN_TEST(testPuffPoundHoldsUpAndTowardTheLedge);
    RUN_TEST(testPuffPoundHoldsLeftWhenTheLedgeIsOnTheLeft);
    RUN_TEST(testPuffMidJumpFarFromTheLedgeUsesNsp);
    RUN_TEST(testPuffLastJumpDoesNotUseNsp);
    RUN_TEST(testYoshiUsesDspWhenTheGrabBoxMissesTheLedge);
    RUN_TEST(testYoshiNeedsTheDoubleJumpAndRisingSpeed);
    RUN_TEST(testCaptainFalconPunchHoldsUp);
    RUN_TEST(testCaptainUsesNspRightAfterTheDoubleJumpAtMaxSpeed);
    RUN_TEST(testCaptainSkipsNspBelowMaxAirSpeed);
    RUN_TEST(testLinkSkipsDspWhileHoldingAnItem);
    RUN_TEST(testFoxFirstUsesDspOnAOneInTwentyRoll);
    RUN_TEST(testFoxIgnoresLedgesBeyondReachWithNoJumps);
    RUN_TEST(testFoxLeavesFireFoxAloneAndWaitsToFall);
    RUN_TEST(testBanjoFairsWhenFacingTheExpectedWay);
    RUN_TEST(testBanjoSkipsDuringFairAndWhileJumping);
    RUN_TEST(testBanjoRunsAtAnyLevel);
    RUN_TEST(testWarioBodySlamHoldsUp);
    RUN_TEST(testWarioUsesNspFarAndNotMuchBelowTheLedge);
    RUN_TEST(testMarthPointsTheStickDuringTheFirstNsp);
    RUN_TEST(testMarthNeedsMaxAirSpeedForNsp);
    RUN_TEST(testCrashNeedsTheDoubleJumpAndDistance);
    RUN_TEST(testDededeUsesDspFarBelowUnlessBothMinionsAreOut);
    RUN_TEST(testDededeSkipsTooFarLedges);
    RUN_TEST(testFalcoPhantasmHoldsB);
    RUN_TEST(testFalcoUsesNspOnAOneInEightRollWhenJustBelowTheLedge);
    RUN_TEST(testFalcoSkipsWhenTheGrabBoxStaysBelowTheLedge);
    RUN_TEST(testSheikDspsOnceWhenFacingTheFarLedge);
    RUN_TEST(testSheikFacingAwayFromTheLedgeDoesNothing);
    RUN_TEST(testMarinaUsesNspWhenFarAndFarAboveTheLedge);
    RUN_TEST(testSonicUsesNspAboveTheLedgeHeightWithAJumpLeft);
    RUN_TEST(testSonicWithoutJumpsNeedsToBeWellAboveTheLedge);
    RUN_TEST(testPeachFloatsTowardFarLedgesAndReleasesNearBy);
    RUN_TEST(testPeachUsesNspFarAboveTheLedgeWhenNotFloating);
}
