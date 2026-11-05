/**
 * test_dpad_map_rules.c - unit tests of the Remix D-pad Map rules (ft/ftremix/ftremixdpadmaprules.c): the Smash, Tilt
 * and Special press macros and the held hook of css/DpadFunctions.asm.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixdpadmaprules.h"

#define STATUS_WAIT 0x0A
#define BUTTON_L 0x0020
#define BUTTON_Z 0x2000

static FTRemixDpadMapResult pressMacro(int map_mode, unsigned int hold, unsigned int hold_prev, int status_id)
{
    FTRemixDpadMapInput input;
    FTRemixDpadMapResult result;

    input.map_mode = map_mode;
    input.hold = hold;
    input.hold_prev = hold_prev;
    input.status_id = status_id;
    ftRemixDpadMapPress(&input, &result);
    return result;
}

static void testNoDpadHeldLeavesTheFrameAlone(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, BUTTON_Z, 0, STATUS_WAIT);

    CHECK_EQ(0, result.is_applied);
    CHECK_EQ(0, result.is_stick_set);
    CHECK_EQ(BUTTON_Z, result.hold);
}

static void testMapOffLeavesTheFrameAlone(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_OFF, FTREMIX_DPAD_MAP_DPAD_UP, 0, STATUS_WAIT);

    CHECK_EQ(0, result.is_applied);
}

static void testSmashUpPressesAWithFullStickUp(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_UP, 0, STATUS_WAIT);

    CHECK_TRUE(result.is_applied);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_A, result.tap_buttons);
    CHECK_TRUE(result.is_stick_set);
    CHECK_EQ(0, result.stick_x);
    CHECK_EQ(80, result.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_A, result.hold);
}

static void testSmashDownLeftRightUseFullSticks(void)
{
    FTRemixDpadMapResult down = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_DOWN, 0, STATUS_WAIT);
    FTRemixDpadMapResult left = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_LEFT, 0, STATUS_WAIT);
    FTRemixDpadMapResult right = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_RIGHT, 0, STATUS_WAIT);

    CHECK_EQ(-80, down.stick_y);
    CHECK_EQ(0, down.stick_x);
    CHECK_EQ(-80, left.stick_x);
    CHECK_EQ(0, left.stick_y);
    CHECK_EQ(80, right.stick_x);
    CHECK_EQ(0, right.stick_y);
}

static void testSmashDoesNotRepeatWhileTheDpadStaysHeld(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_LEFT,
                                             FTREMIX_DPAD_MAP_DPAD_LEFT, STATUS_WAIT);

    CHECK_EQ(0, result.is_applied);
}

static void testSmashUpWhileDashingJumpsWithCUp(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_UP, 0,
                                             FTREMIX_DPAD_MAP_STATUS_DASH);

    CHECK_TRUE(result.is_applied);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_C_UP, result.tap_buttons);
    CHECK_EQ(0, result.is_stick_set);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP, result.hold);
}

static void testSmashUpWhileRunningAndShieldingJumpsEvenWhenHeld(void)
{
    FTRemixDpadMapResult running = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_UP,
                                              FTREMIX_DPAD_MAP_DPAD_UP, FTREMIX_DPAD_MAP_STATUS_RUN);
    FTRemixDpadMapResult shielding = pressMacro(FTREMIX_DPAD_MAP_SPECIAL, FTREMIX_DPAD_MAP_DPAD_UP, 0,
                                                FTREMIX_DPAD_MAP_STATUS_SHIELD);

    CHECK_TRUE(running.is_applied);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_C_UP, running.tap_buttons);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_C_UP, shielding.tap_buttons);
    CHECK_EQ(0, shielding.is_stick_set);
}

static void testSmashUpInJumpSquatPressesEvenWhenHeld(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_UP,
                                             FTREMIX_DPAD_MAP_DPAD_UP, FTREMIX_DPAD_MAP_STATUS_JUMP_SQUAT);
    FTRemixDpadMapResult shield_squat = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_UP,
                                                   FTREMIX_DPAD_MAP_DPAD_UP, FTREMIX_DPAD_MAP_STATUS_SHIELD_JUMP_SQUAT);

    CHECK_TRUE(result.is_applied);
    CHECK_EQ(80, result.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_A, result.tap_buttons);
    CHECK_TRUE(shield_squat.is_applied);
    CHECK_EQ(80, shield_squat.stick_y);
}

static void testSmashDownWhileDashingStillSmashes(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_DOWN, 0,
                                             FTREMIX_DPAD_MAP_STATUS_DASH);

    CHECK_EQ(-80, result.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_DOWN | FTREMIX_DPAD_MAP_BUTTON_A, result.tap_buttons);
}

static void testSmashTapMaskKeepsTheOtherHeldButtons(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_RIGHT | BUTTON_Z, 0, STATUS_WAIT);

    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_RIGHT | BUTTON_Z | FTREMIX_DPAD_MAP_BUTTON_A, result.tap_buttons);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_RIGHT | BUTTON_Z | FTREMIX_DPAD_MAP_BUTTON_A, result.hold);
}

static void testSpecialPressesBButHoldsA(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SPECIAL, FTREMIX_DPAD_MAP_DPAD_RIGHT, 0, STATUS_WAIT);

    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_RIGHT | FTREMIX_DPAD_MAP_BUTTON_B, result.tap_buttons);
    CHECK_EQ(80, result.stick_x);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_RIGHT | FTREMIX_DPAD_MAP_BUTTON_A, result.hold);
}

static void testSmashUpPriorityOverDownWhenBothHeld(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_SMASH, FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_DPAD_DOWN, 0,
                                             STATUS_WAIT);

    CHECK_EQ(80, result.stick_y);
}

static void testTiltPressesAWithHalfStick(void)
{
    FTRemixDpadMapResult up = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_UP, 0, STATUS_WAIT);
    FTRemixDpadMapResult down = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_DOWN, 0, STATUS_WAIT);
    FTRemixDpadMapResult left = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_LEFT, 0, STATUS_WAIT);
    FTRemixDpadMapResult right = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_RIGHT, 0, STATUS_WAIT);

    CHECK_EQ(40, up.stick_y);
    CHECK_EQ(-40, down.stick_y);
    CHECK_EQ(-40, left.stick_x);
    CHECK_EQ(40, right.stick_x);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_A, up.tap_buttons);
}

static void testTiltDiagonalSetsBothAxes(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_DPAD_RIGHT, 0,
                                             STATUS_WAIT);

    CHECK_EQ(40, result.stick_x);
    CHECK_EQ(40, result.stick_y);
}

static void testTiltOppositeDirectionsOrTheirValues(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_DPAD_DOWN, 0,
                                             STATUS_WAIT);

    CHECK_EQ(-8, result.stick_y);
}

static void testTiltDoesNotRepeatWhileTheDpadStaysHeld(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_DPAD_LEFT,
                                             FTREMIX_DPAD_MAP_DPAD_UP, STATUS_WAIT);

    CHECK_EQ(0, result.is_applied);
}

static void testTiltIgnoresJumpActions(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_UP, 0, FTREMIX_DPAD_MAP_STATUS_DASH);

    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_A, result.tap_buttons);
    CHECK_EQ(40, result.stick_y);
}

static void testShieldTiltUpUsesStickFiftyThreeWithoutAPress(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_UP, 0, FTREMIX_DPAD_MAP_STATUS_SHIELD);

    CHECK_TRUE(result.is_applied);
    CHECK_EQ(53, result.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP, result.tap_buttons);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_A, result.hold);
}

static void testShieldTiltDownPressesAWithHalfStick(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_DOWN, 0, FTREMIX_DPAD_MAP_STATUS_SHIELD_ON);

    CHECK_EQ(-40, result.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_DOWN | FTREMIX_DPAD_MAP_BUTTON_A, result.tap_buttons);
}

static void testShieldTiltSidewaysUsesFullStickWithoutAPress(void)
{
    FTRemixDpadMapResult left = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_LEFT, 0, FTREMIX_DPAD_MAP_STATUS_ROLL_F);
    FTRemixDpadMapResult right = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_RIGHT, 0, FTREMIX_DPAD_MAP_STATUS_ROLL_B);

    CHECK_EQ(-80, left.stick_x);
    CHECK_EQ(80, right.stick_x);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_LEFT, left.tap_buttons);
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_RIGHT | FTREMIX_DPAD_MAP_BUTTON_A, right.hold);
}

static void testShieldTiltDoesNotRepeatWhileTheDpadStaysHeld(void)
{
    FTRemixDpadMapResult result = pressMacro(FTREMIX_DPAD_MAP_TILT, FTREMIX_DPAD_MAP_DPAD_LEFT, FTREMIX_DPAD_MAP_DPAD_LEFT,
                                             FTREMIX_DPAD_MAP_STATUS_SHIELD);

    CHECK_EQ(0, result.is_applied);
}

static void testEnabledNeedsAnActiveMap(void)
{
    CHECK_EQ(0, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_OFF, 0, 0, 0, 0));
    CHECK_EQ(1, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_SMASH, 0, 0, 0, 0));
}

static void testEnabledWithStickSwapButNotStickless(void)
{
    CHECK_EQ(1, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_TILT, 1, 0, 0, 0));
    CHECK_EQ(0, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_TILT, 2, 0, 0, 0));
    CHECK_EQ(1, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_TILT, 3, 0, 0, 0));
    CHECK_EQ(0, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_TILT, 4, 0, 0, 0));
}

static void testEnabledNotWithSingleButtonMode(void)
{
    CHECK_EQ(0, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_SPECIAL, 0, 3, 0, 0));
}

static void testEnabledInTrainingOnlyWithTheDpadMenuOff(void)
{
    CHECK_EQ(0, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_SMASH, 0, 0, 1, 0));
    CHECK_EQ(1, ftRemixDpadMapIsEnabled(FTREMIX_DPAD_MAP_SMASH, 0, 0, 1, 1));
}

static void testHeldSpecialAddsBWhileTheDpadIsHeld(void)
{
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_UP | FTREMIX_DPAD_MAP_BUTTON_B,
             ftRemixDpadMapHeld(FTREMIX_DPAD_MAP_SPECIAL, 0, FTREMIX_DPAD_MAP_DPAD_UP));
    CHECK_EQ(BUTTON_L, ftRemixDpadMapHeld(FTREMIX_DPAD_MAP_SPECIAL, 0, BUTTON_L));
}

static void testHeldSmashAddsAOnlyWithChargedSmashes(void)
{
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_LEFT | FTREMIX_DPAD_MAP_BUTTON_A,
             ftRemixDpadMapHeld(FTREMIX_DPAD_MAP_SMASH, 1, FTREMIX_DPAD_MAP_DPAD_LEFT));
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_LEFT, ftRemixDpadMapHeld(FTREMIX_DPAD_MAP_SMASH, 0, FTREMIX_DPAD_MAP_DPAD_LEFT));
}

static void testHeldTiltAddsNothing(void)
{
    CHECK_EQ(FTREMIX_DPAD_MAP_DPAD_LEFT, ftRemixDpadMapHeld(FTREMIX_DPAD_MAP_TILT, 1, FTREMIX_DPAD_MAP_DPAD_LEFT));
}

void remixTestDpadMapRules(void)
{
    printf("dpad map rules\n");
    RUN_TEST(testNoDpadHeldLeavesTheFrameAlone);
    RUN_TEST(testMapOffLeavesTheFrameAlone);
    RUN_TEST(testSmashUpPressesAWithFullStickUp);
    RUN_TEST(testSmashDownLeftRightUseFullSticks);
    RUN_TEST(testSmashDoesNotRepeatWhileTheDpadStaysHeld);
    RUN_TEST(testSmashUpWhileDashingJumpsWithCUp);
    RUN_TEST(testSmashUpWhileRunningAndShieldingJumpsEvenWhenHeld);
    RUN_TEST(testSmashUpInJumpSquatPressesEvenWhenHeld);
    RUN_TEST(testSmashDownWhileDashingStillSmashes);
    RUN_TEST(testSmashTapMaskKeepsTheOtherHeldButtons);
    RUN_TEST(testSpecialPressesBButHoldsA);
    RUN_TEST(testSmashUpPriorityOverDownWhenBothHeld);
    RUN_TEST(testTiltPressesAWithHalfStick);
    RUN_TEST(testTiltDiagonalSetsBothAxes);
    RUN_TEST(testTiltOppositeDirectionsOrTheirValues);
    RUN_TEST(testTiltDoesNotRepeatWhileTheDpadStaysHeld);
    RUN_TEST(testTiltIgnoresJumpActions);
    RUN_TEST(testShieldTiltUpUsesStickFiftyThreeWithoutAPress);
    RUN_TEST(testShieldTiltDownPressesAWithHalfStick);
    RUN_TEST(testShieldTiltSidewaysUsesFullStickWithoutAPress);
    RUN_TEST(testShieldTiltDoesNotRepeatWhileTheDpadStaysHeld);
    RUN_TEST(testEnabledNeedsAnActiveMap);
    RUN_TEST(testEnabledWithStickSwapButNotStickless);
    RUN_TEST(testEnabledNotWithSingleButtonMode);
    RUN_TEST(testEnabledInTrainingOnlyWithTheDpadMenuOff);
    RUN_TEST(testHeldSpecialAddsBWhileTheDpadIsHeld);
    RUN_TEST(testHeldSmashAddsAOnlyWithChargedSmashes);
    RUN_TEST(testHeldTiltAddsNothing);
}
