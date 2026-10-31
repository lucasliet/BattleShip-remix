/**
 * test_training_rules.c - unit tests of the Training custom menu rules (sc/sc1pmode/sc1ptrainingrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1ptrainingrules.h"

static SC1PTrainingRulesShieldIn makeInput(int32_t mode, int32_t status)
{
    SC1PTrainingRulesShieldIn in = { 0 };

    in.mode = mode;
    in.status = status;
    in.oos_option = 0;

    return in;
}

static void testModeOffLeavesTheControllerAlone(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_OFF, SC1PTRAININGRULES_STATUS_SHIELD);
    SC1PTrainingRulesShieldOut out;

    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_NONE, out.action);
    CHECK_EQ(0, out.max_shield);
}

static void testInfiniteModeRefillsAndHoldsShield(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_INFINITE, SC1PTRAININGRULES_STATUS_SHIELD);
    SC1PTrainingRulesShieldOut out;

    in.is_in_shield_stun = 1;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_FORCE_SHIELD, out.action);
    CHECK_EQ(1, out.max_shield);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_SHIELD, out.new_status);
}

static void testBreakModeRefillsUntilHit(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_SHIELD);
    SC1PTrainingRulesShieldOut out;

    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_FORCE_SHIELD, out.action);
    CHECK_EQ(1, out.max_shield);
}

static void testShieldStunMovesToStunWithoutRefill(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_SHIELD);
    SC1PTrainingRulesShieldOut out;

    in.is_in_shield_stun = 1;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_STUN, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_FORCE_SHIELD, out.action);
    CHECK_EQ(0, out.max_shield);
}

static void testStunKeepsShieldingUntilTheLastFrame(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_STUN);
    SC1PTrainingRulesShieldOut out;

    in.is_in_shield_stun = 1;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_FORCE_SHIELD, out.action);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_STUN, out.new_status);
}

static void testLeavingShieldStunReturnsToShieldWithoutInput(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_STUN);
    SC1PTrainingRulesShieldOut out;

    in.is_in_shield_stun = 0;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_SHIELD, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_NONE, out.action);
}

static void testLastStunFramePressesTheFirstOosInput(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_STUN);
    SC1PTrainingRulesShieldOut out;

    in.is_in_shield_stun = 1;
    in.is_shield_stun_ending = 1;
    in.oos_option = 3;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_OOS_INPUT, out.action);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_OOS_FRAME_1, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_Z | SC1PTRAININGRULES_BUTTON_A, out.button_mask);
}

static void testSecondOosFrameUsesTheSecondTable(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_OOS_FRAME_1);
    SC1PTrainingRulesShieldOut out;

    in.oos_option = 4;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_OOS_FRAME_2, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_A, out.button_mask);
    CHECK_EQ(80, out.stick_y);
}

static void testDownSpecialWaitsForTheLastJumpsquatFrame(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_OOS_FRAME_1);
    SC1PTrainingRulesShieldOut out;

    in.oos_option = SC1PTRAININGRULES_OOS_DSP;
    in.is_in_shield_jumpsquat = 1;
    in.is_last_jumpsquat_frame = 0;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_OOS_FRAME_1, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_Z, out.button_mask);
    in.is_last_jumpsquat_frame = 1;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_OOS_FRAME_2, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_B, out.button_mask);
}

static void testNeutralAirOutsideJumpsquatAdvancesAtOnce(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_OOS_FRAME_1);
    SC1PTrainingRulesShieldOut out;

    in.oos_option = SC1PTRAININGRULES_OOS_NAIR;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_OOS_FRAME_2, out.new_status);
}

static void testEndOosReleasesTheButtonAndReturnsToShield(void)
{
    SC1PTrainingRulesShieldIn in = makeInput(SC1PTRAININGRULES_SHIELD_BREAK_ON, SC1PTRAININGRULES_STATUS_OOS_FRAME_2);
    SC1PTrainingRulesShieldOut out;

    in.oos_option = 6;
    sc1PTrainingRulesShieldStep(&in, &out);
    CHECK_EQ(SC1PTRAININGRULES_ACTION_END_OOS, out.action);
    CHECK_EQ(SC1PTRAININGRULES_STATUS_SHIELD, out.new_status);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_B, out.button_mask);
    CHECK_EQ(-80, out.stick_y);
}

static void testOosTablesMatchTheAsmEntries(void)
{
    int32_t x;
    int32_t y;

    CHECK_EQ(SC1PTRAININGRULES_BUTTON_Z, sc1PTrainingRulesGetOosInput(1, 0, &x, &y));
    CHECK_EQ(-80, x);
    CHECK_EQ(0, y);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_Z, sc1PTrainingRulesGetOosInput(2, 1, &x, &y));
    CHECK_EQ(80, x);
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_Z, sc1PTrainingRulesGetOosInput(11, 0, &x, &y));
    CHECK_EQ(SC1PTRAININGRULES_BUTTON_Z, sc1PTrainingRulesGetOosInput(99, 1, &x, &y));
}

static void testSpamTimerReload(void)
{
    CHECK_EQ(5, sc1PTrainingRulesSpamReloadTimer(0, 5, 77));
    CHECK_EQ(77, sc1PTrainingRulesSpamReloadTimer(1, 5, 77));
    CHECK_EQ(999 & 0xFF, sc1PTrainingRulesSpamReloadTimer(0, 999, 0));
}

static void testSpamTimerOnlyCountsDownForACpuOutsideHitlag(void)
{
    CHECK_EQ(4, sc1PTrainingRulesSpamTickTimer(5, 1, 0));
    CHECK_EQ(5, sc1PTrainingRulesSpamTickTimer(5, 0, 0));
    CHECK_EQ(5, sc1PTrainingRulesSpamTickTimer(5, 1, 1));
    CHECK_EQ(0, sc1PTrainingRulesSpamTickTimer(0, 1, 0));
}

void remixTestTrainingRules(void)
{
    printf("training custom menu rules\n");
    RUN_TEST(testModeOffLeavesTheControllerAlone);
    RUN_TEST(testInfiniteModeRefillsAndHoldsShield);
    RUN_TEST(testBreakModeRefillsUntilHit);
    RUN_TEST(testShieldStunMovesToStunWithoutRefill);
    RUN_TEST(testStunKeepsShieldingUntilTheLastFrame);
    RUN_TEST(testLeavingShieldStunReturnsToShieldWithoutInput);
    RUN_TEST(testLastStunFramePressesTheFirstOosInput);
    RUN_TEST(testSecondOosFrameUsesTheSecondTable);
    RUN_TEST(testDownSpecialWaitsForTheLastJumpsquatFrame);
    RUN_TEST(testNeutralAirOutsideJumpsquatAdvancesAtOnce);
    RUN_TEST(testEndOosReleasesTheButtonAndReturnsToShield);
    RUN_TEST(testOosTablesMatchTheAsmEntries);
    RUN_TEST(testSpamTimerReload);
    RUN_TEST(testSpamTimerOnlyCountsDownForACpuOutsideHitlag);
}
