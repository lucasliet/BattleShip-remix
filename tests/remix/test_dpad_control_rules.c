/**
 * test_dpad_control_rules.c - unit tests of the Remix D-pad Control action rules (ft/ftremix/ftremixdpadcontrolrules.c):
 * the stick jump, run up smash/special, dash attack and shield mask of css/DpadControl.asm, and the menu side of the option
 * (ft/ftremix/ftremixdpadmenurules.c): css_cursor_and_dpad_controls_ Stick Swap (with the Training DR turbo), the stick Stickless
 * clears, the paused Training menu and the CSS D-pad cursor.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixdpadcontrolrules.h"
#include "ft/ftremix/ftremixdpadmenurules.h"

static void testStandardModesAreStickSwapAndStickless(void)
{
    CHECK_EQ(0, ftRemixDpadControlIsStandardMode(FTREMIX_DPAD_CONTROL_OFF));
    CHECK_EQ(1, ftRemixDpadControlIsStandardMode(FTREMIX_DPAD_CONTROL_STICK_SWAP));
    CHECK_EQ(1, ftRemixDpadControlIsStandardMode(FTREMIX_DPAD_CONTROL_STICKLESS));
    CHECK_EQ(0, ftRemixDpadControlIsStandardMode(FTREMIX_DPAD_CONTROL_STICK_SWAP_J));
    CHECK_EQ(0, ftRemixDpadControlIsStandardMode(FTREMIX_DPAD_CONTROL_STICKLESS_J));
}

static void testJModesAndOffKeepTheVanillaRunStickJump(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpVanilla, ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_OFF, 0, FTREMIX_DPAD_CONTROL_BUTTON_B));
    CHECK_EQ(nFTRemixDpadControlRunUpVanilla, ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP_J, 0, FTREMIX_DPAD_CONTROL_BUTTON_A));
    CHECK_EQ(nFTRemixDpadControlRunUpVanilla, ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICKLESS_J, 0, 0));
}

static void testRunUpWithoutButtonsNeitherJumpsNorAttacks(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpNone, ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0, 0));
    CHECK_EQ(nFTRemixDpadControlRunUpNone, ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICKLESS, 0, FTREMIX_DPAD_CONTROL_BUTTON_L));
}

static void testRunUpWithBHeldStartsTheUpSpecial(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpSpecial,
             ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0, FTREMIX_DPAD_CONTROL_BUTTON_B));
}

static void testRunUpWithAHeldStartsTheUpSmash(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpSmash,
             ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICKLESS, 0, FTREMIX_DPAD_CONTROL_BUTTON_A));
}

static void testRunUpWithBPressedThisFrameFallsToTheSmashCheck(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpNone,
             ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP, FTREMIX_DPAD_CONTROL_BUTTON_B, FTREMIX_DPAD_CONTROL_BUTTON_B));
    CHECK_EQ(nFTRemixDpadControlRunUpSmash,
             ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP, FTREMIX_DPAD_CONTROL_BUTTON_B,
                                           FTREMIX_DPAD_CONTROL_BUTTON_A | FTREMIX_DPAD_CONTROL_BUTTON_B));
}

static void testRunUpWithAPressedThisFrameDoesNothing(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpNone,
             ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP, FTREMIX_DPAD_CONTROL_BUTTON_A, FTREMIX_DPAD_CONTROL_BUTTON_A));
}

static void testRunUpSpecialWinsOverSmashWhenBothAreHeld(void)
{
    CHECK_EQ(nFTRemixDpadControlRunUpSpecial,
             ftRemixDpadControlRunUpAction(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0, FTREMIX_DPAD_CONTROL_BUTTON_A | FTREMIX_DPAD_CONTROL_BUTTON_B));
}

static void testDashAttackIsSkippedWithTheStickUpInStandardModes(void)
{
    CHECK_EQ(1, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1));
    CHECK_EQ(1, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_STICKLESS, 80));
}

static void testDashAttackStaysWithTheStickLevelOrDown(void)
{
    CHECK_EQ(0, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0));
    CHECK_EQ(0, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_STICK_SWAP, -80));
}

static void testDashAttackStaysInJModesAndOff(void)
{
    CHECK_EQ(0, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_STICK_SWAP_J, 80));
    CHECK_EQ(0, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_STICKLESS_J, 80));
    CHECK_EQ(0, ftRemixDpadControlSkipsDashAttack(FTREMIX_DPAD_CONTROL_OFF, 80));
}

static void testShieldMovesToLWithTheControlOn(void)
{
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_L, ftRemixDpadControlShieldMask(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0));
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_L, ftRemixDpadControlShieldMask(FTREMIX_DPAD_CONTROL_STICKLESS_J, 0));
}

static void testShieldKeepsZWithTheControlOff(void)
{
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_Z, ftRemixDpadControlShieldMask(FTREMIX_DPAD_CONTROL_OFF, 0));
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_Z, ftRemixDpadControlShieldMask(FTREMIX_DPAD_CONTROL_OFF, 1));
}

static void testShieldAnswersToLAndZWhenTheTauntButtonIsCustom(void)
{
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_L | FTREMIX_DPAD_CONTROL_BUTTON_Z, ftRemixDpadControlShieldMask(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1));
}

static void testDefaultTauntMovesToZWithTheControlOn(void)
{
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_Z, ftRemixDpadControlTauntMask(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0, FTREMIX_DPAD_CONTROL_BUTTON_L));
}

static void testCustomTauntAndControlOffKeepTheTauntMask(void)
{
    CHECK_EQ(0x0008, ftRemixDpadControlTauntMask(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1, 0x0008));
    CHECK_EQ(FTREMIX_DPAD_CONTROL_BUTTON_L, ftRemixDpadControlTauntMask(FTREMIX_DPAD_CONTROL_OFF, 0, FTREMIX_DPAD_CONTROL_BUTTON_L));
}

#define MENU_NO_BUTTONS 0

static int menuActionOn(int mode, int match, int title, int paused, int training, int css, int toggle)
{
    FTRemixDpadMenuScreen screen;

    screen.is_match_scene = match;
    screen.is_title_scene = title;
    screen.is_paused = paused;
    screen.is_training = training;
    screen.is_css_scene = css;
    screen.is_cursor_toggle_on = toggle;
    return ftRemixDpadMenuPickAction(mode, &screen);
}

#define MENU_ACTION(mode, match, paused, training, css, toggle) menuActionOn(mode, match, 0, paused, training, css, toggle)
#define MENU_ACTION_TITLE(mode) menuActionOn(mode, 0, 1, 0, 0, 0, 0)

static FTRemixDpadMenuSample menuSample(unsigned int hold, int stick_x, int stick_y)
{
    FTRemixDpadMenuSample sample;

    sample.hold = hold;
    sample.tap = hold;
    sample.update = hold;
    sample.stick_x = stick_x;
    sample.stick_y = stick_y;
    return sample;
}

static void testMenuStickSwapSwapsOnEveryScreenButAnUnpausedMatch(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionSwap, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP, 0, 0, 0, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionSwap, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP_J, 0, 0, 0, 1, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1, 0, 0, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionSwap, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1, 1, 0, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionSwapTraining, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1, 1, 1, 0, 0));
}

static void testMenuStickSwapOnTheTrainingScreenSwapsTheRunningMatchToo(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionSwapTraining, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP, 1, 0, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionSwapTraining, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP_J, 1, 0, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICK_SWAP_J, 1, 0, 0, 0, 0));
}

static void testMenuSticklessZeroesTheStickOfUnpausedMatchesAndTheTitle(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionClearStick, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 1, 0, 0, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionClearStick, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 1, 0, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionClearStick, MENU_ACTION_TITLE(FTREMIX_DPAD_CONTROL_STICKLESS));
}

static void testMenuSticklessJKeepsTheStickOfUnpausedMatchesAndTheTitle(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS_J, 1, 0, 0, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS_J, 1, 0, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION_TITLE(FTREMIX_DPAD_CONTROL_STICKLESS_J));
}

static void testMenuSticklessKeepsTheStickOfAPausedMatchOutsideTraining(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 1, 1, 0, 0, 0));
}

static void testMenuOffNeverTouchesAMatchOrTheTitle(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_OFF, 1, 0, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION_TITLE(FTREMIX_DPAD_CONTROL_OFF));
}

static void testMenuSticklessOpensThePausedTrainingMenuToTheDpad(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionTrainingPause, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 1, 1, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionTrainingPause, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS_J, 1, 1, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 1, 1, 0, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionClearStick, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 1, 0, 1, 0, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_OFF, 1, 1, 1, 0, 1));
}

static void testMenuCursorNeedsACssSceneAndTheOptionOrTheToggle(void)
{
    CHECK_EQ(nFTRemixDpadMenuActionCursor, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 0, 0, 0, 1, 0));
    CHECK_EQ(nFTRemixDpadMenuActionCursor, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS_J, 0, 0, 0, 1, 0));
    CHECK_EQ(nFTRemixDpadMenuActionCursor, MENU_ACTION(FTREMIX_DPAD_CONTROL_OFF, 0, 0, 0, 1, 1));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_OFF, 0, 0, 0, 1, 0));
    CHECK_EQ(nFTRemixDpadMenuActionNone, MENU_ACTION(FTREMIX_DPAD_CONTROL_STICKLESS, 0, 0, 0, 0, 1));
}

static void testMenuSwapPressesTheDpadOnTheFirstFrameOfAStickPush(void)
{
    FTRemixDpadMenuSample sample = menuSample(MENU_NO_BUTTONS, 60, 0);

    sample.tap = MENU_NO_BUTTONS;
    sample.update = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwap(&sample, MENU_NO_BUTTONS);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_R, (int)sample.hold);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_R, (int)sample.tap);
    CHECK_EQ(0, sample.stick_x);
}

static void testMenuSwapKeepsHoldingWithoutANewPress(void)
{
    FTRemixDpadMenuSample sample = menuSample(MENU_NO_BUTTONS, -60, 0);

    sample.tap = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwap(&sample, FTREMIX_DPAD_MENU_BUTTON_L);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_L, (int)sample.hold);
    CHECK_EQ(0, (int)sample.tap);
    CHECK_EQ(0, sample.stick_x);
}

static void testMenuSwapDeadZoneIs40OnXAnd30OnY(void)
{
    FTRemixDpadMenuSample inside = menuSample(MENU_NO_BUTTONS, -39, 29);
    FTRemixDpadMenuSample edge = menuSample(MENU_NO_BUTTONS, -40, -30);

    inside.tap = MENU_NO_BUTTONS;
    edge.tap = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwap(&inside, MENU_NO_BUTTONS);
    ftRemixDpadMenuSwap(&edge, MENU_NO_BUTTONS);
    CHECK_EQ(0, (int)inside.hold);
    CHECK_EQ(0, inside.stick_x);
    CHECK_EQ(0, inside.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_L | FTREMIX_DPAD_MENU_BUTTON_D, (int)edge.hold);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_L | FTREMIX_DPAD_MENU_BUTTON_D, (int)edge.tap);
}

static void testMenuSwapOnYTestsUpAndDownHeldBeforeAPress(void)
{
    FTRemixDpadMenuSample sample = menuSample(MENU_NO_BUTTONS, 0, 70);

    sample.tap = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwap(&sample, FTREMIX_DPAD_MENU_BUTTON_D);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_U, (int)sample.hold);
    CHECK_EQ(0, (int)sample.tap);

    sample = menuSample(MENU_NO_BUTTONS, 0, 70);
    sample.tap = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwap(&sample, FTREMIX_DPAD_MENU_BUTTON_L);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_U, (int)sample.tap);
}

static void testMenuSwapMovesTheStickWithTheDpadAndDropsTheDpadBits(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_L | FTREMIX_DPAD_MENU_BUTTON_U | 0x8000, 0, 0);

    ftRemixDpadMenuSwap(&sample, MENU_NO_BUTTONS);
    CHECK_EQ(0x8000, (int)sample.hold);
    CHECK_EQ(0x8000, (int)sample.tap);
    CHECK_EQ(0x8000, (int)sample.update);
    CHECK_EQ(-80, sample.stick_x);
    CHECK_EQ(80, sample.stick_y);
}

static void testMenuSwapRightAndUpWinOverLeftAndDown(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTONS, 0, 0);

    ftRemixDpadMenuSwap(&sample, MENU_NO_BUTTONS);
    CHECK_EQ(80, sample.stick_x);
    CHECK_EQ(80, sample.stick_y);
}

static void testMenuDpadAsStickKeepsTheStickWithoutTheDpad(void)
{
    FTRemixDpadMenuSample sample = menuSample(0x4000, -33, 44);

    ftRemixDpadMenuDpadAsStick(&sample);
    CHECK_EQ(0x4000, (int)sample.hold);
    CHECK_EQ(-33, sample.stick_x);
    CHECK_EQ(44, sample.stick_y);
}

static void testMenuDpadAsStickReplacesTheAxesOfTheHeldDirections(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_D | FTREMIX_DPAD_MENU_BUTTON_R, -33, 44);

    ftRemixDpadMenuDpadAsStick(&sample);
    CHECK_EQ(0, (int)sample.hold);
    CHECK_EQ(80, sample.stick_x);
    CHECK_EQ(-80, sample.stick_y);
}

static void testMenuClearStickZeroesBothAxesAndKeepsTheButtons(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_U | 0x8000, -33, 44);

    ftRemixDpadMenuClearStick(&sample);
    CHECK_EQ(0, sample.stick_x);
    CHECK_EQ(0, sample.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_U | 0x8000, (int)sample.hold);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_U | 0x8000, (int)sample.tap);
}

static void testMenuTurboStepFiresOnEveryFifthCall(void)
{
    int turbo_frames = 0;
    int fired = 0;
    int call;

    for (call = 1; call <= 10; call++)
    {
        fired = ftRemixDpadMenuStepTurbo(&turbo_frames);
        CHECK_EQ((call % FTREMIX_DPAD_MENU_TURBO_FRAMES) == 0 ? 1 : 0, fired);
    }
    CHECK_EQ(0, turbo_frames);
}

static void testMenuSwapTrainingAddsDrToTheRepeatEveryFifthFrameOfStickRight(void)
{
    FTRemixDpadMenuSample sample;
    int turbo_frames = 0;
    int frame;

    for (frame = 1; frame <= 6; frame++)
    {
        sample = menuSample(MENU_NO_BUTTONS, 60, 0);
        sample.tap = MENU_NO_BUTTONS;
        sample.update = MENU_NO_BUTTONS;
        ftRemixDpadMenuSwapTraining(&sample, FTREMIX_DPAD_MENU_BUTTON_R, &turbo_frames);
        CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_R, (int)sample.hold);
        CHECK_EQ(0, (int)sample.tap);
        CHECK_EQ((frame == 5) ? FTREMIX_DPAD_MENU_BUTTON_R : 0, (int)sample.update);
    }
}

static void testMenuSwapTrainingFirstFramePressesWithoutTheRepeatUntilTheTimerFires(void)
{
    FTRemixDpadMenuSample sample = menuSample(MENU_NO_BUTTONS, 60, 0);
    int turbo_frames = 0;

    sample.tap = MENU_NO_BUTTONS;
    sample.update = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwapTraining(&sample, MENU_NO_BUTTONS, &turbo_frames);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_R, (int)sample.tap);
    CHECK_EQ(0, (int)sample.update);
    CHECK_EQ(1, turbo_frames);
}

static void testMenuSwapTrainingTurboNeedsTheStickRightPastTheDeadZone(void)
{
    FTRemixDpadMenuSample left = menuSample(MENU_NO_BUTTONS, -60, 0);
    FTRemixDpadMenuSample inside = menuSample(MENU_NO_BUTTONS, 39, 0);
    FTRemixDpadMenuSample vertical = menuSample(MENU_NO_BUTTONS, 0, 70);
    int turbo_frames = FTREMIX_DPAD_MENU_TURBO_FRAMES - 1;

    left.update = MENU_NO_BUTTONS;
    inside.update = MENU_NO_BUTTONS;
    vertical.update = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwapTraining(&left, MENU_NO_BUTTONS, &turbo_frames);
    ftRemixDpadMenuSwapTraining(&inside, MENU_NO_BUTTONS, &turbo_frames);
    ftRemixDpadMenuSwapTraining(&vertical, MENU_NO_BUTTONS, &turbo_frames);
    CHECK_EQ(0, (int)left.update);
    CHECK_EQ(0, (int)inside.update);
    CHECK_EQ(0, (int)vertical.update);
    CHECK_EQ(FTREMIX_DPAD_MENU_TURBO_FRAMES - 1, turbo_frames);
}

static void testMenuSwapTrainingKeepsTheTimerWhenTheStickIsReleased(void)
{
    FTRemixDpadMenuSample sample = menuSample(MENU_NO_BUTTONS, 0, 0);
    int turbo_frames = 3;

    sample.update = MENU_NO_BUTTONS;
    ftRemixDpadMenuSwapTraining(&sample, MENU_NO_BUTTONS, &turbo_frames);
    CHECK_EQ(3, turbo_frames);
    CHECK_EQ(0, (int)sample.update);
}

static void testMenuSwapTrainingStillSwapsTheDpadToTheStick(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_L | 0x8000, 0, 0);
    int turbo_frames = 0;

    sample.update = sample.hold;
    ftRemixDpadMenuSwapTraining(&sample, MENU_NO_BUTTONS, &turbo_frames);
    CHECK_EQ(0x8000, (int)sample.hold);
    CHECK_EQ(0x8000, (int)sample.update);
    CHECK_EQ(-80, sample.stick_x);
    CHECK_EQ(0, turbo_frames);
}

static void testMenuSwapOutsideTrainingNeverAddsTheTurbo(void)
{
    FTRemixDpadMenuSample sample;
    int frame;

    for (frame = 1; frame <= FTREMIX_DPAD_MENU_TURBO_FRAMES; frame++)
    {
        sample = menuSample(MENU_NO_BUTTONS, 60, 0);
        sample.update = MENU_NO_BUTTONS;
        ftRemixDpadMenuSwap(&sample, FTREMIX_DPAD_MENU_BUTTON_R);
        CHECK_EQ(0, (int)sample.update);
    }
}

static void testMenuCursorFirstFrameOfAPressOnlyArmsTheDelay(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_R, 0, 0);
    int timer = 0;

    ftRemixDpadMenuCursor(&sample, MENU_NO_BUTTONS, 0, &timer, 0);
    CHECK_EQ(FTREMIX_DPAD_MENU_CURSOR_DELAY, timer);
    CHECK_EQ(0, sample.stick_x);
    ftRemixDpadMenuCursor(&sample, MENU_NO_BUTTONS, 1, &timer, 0);
    CHECK_EQ(FTREMIX_DPAD_MENU_CURSOR_DELAY_STICKLESS, timer);
}

static void testMenuCursorMovesAtOnceWhenNoTokenIsOverAFighter(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_L, 0, 0);
    int timer = FTREMIX_DPAD_MENU_CURSOR_DELAY;

    ftRemixDpadMenuCursor(&sample, FTREMIX_DPAD_MENU_BUTTON_L, 0, &timer, 0);
    CHECK_EQ(-48, sample.stick_x);
    CHECK_EQ(0, sample.stick_y);
    CHECK_EQ(0, timer);
}

static void testMenuCursorWaitsOutTheDelayWhileATokenIsOverAFighter(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_U, 0, 0);
    int timer = 2;

    ftRemixDpadMenuCursor(&sample, FTREMIX_DPAD_MENU_BUTTON_U, 1, &timer, 1);
    CHECK_EQ(1, timer);
    CHECK_EQ(0, sample.stick_y);
    ftRemixDpadMenuCursor(&sample, FTREMIX_DPAD_MENU_BUTTON_U, 1, &timer, 1);
    CHECK_EQ(0, timer);
    CHECK_EQ(48, sample.stick_y);
}

static void testMenuCursorKeepsMovingOnceTheDelayIsOver(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_D, 0, 0);
    int timer = 0;

    ftRemixDpadMenuCursor(&sample, FTREMIX_DPAD_MENU_BUTTON_D, 0, &timer, 1);
    CHECK_EQ(-48, sample.stick_y);
    CHECK_EQ(0, timer);
}

static void testMenuCursorRightAndUpWinOverLeftAndDown(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTONS, 0, 0);
    int timer = 0;

    ftRemixDpadMenuCursor(&sample, FTREMIX_DPAD_MENU_BUTTONS, 0, &timer, 0);
    CHECK_EQ(48, sample.stick_x);
    CHECK_EQ(48, sample.stick_y);
}

static void testMenuCursorReadsThePreviousFrameDpadAndKeepsTheStick(void)
{
    FTRemixDpadMenuSample sample = menuSample(FTREMIX_DPAD_MENU_BUTTON_R, 10, -20);
    int timer = 0;

    ftRemixDpadMenuCursor(&sample, MENU_NO_BUTTONS, 0, &timer, 0);
    CHECK_EQ(10, sample.stick_x);
    CHECK_EQ(-20, sample.stick_y);
    CHECK_EQ(FTREMIX_DPAD_MENU_BUTTON_R, (int)sample.hold);
}

void remixTestDpadControlRules(void)
{
    printf("dpad control rules\n");
    RUN_TEST(testStandardModesAreStickSwapAndStickless);
    RUN_TEST(testJModesAndOffKeepTheVanillaRunStickJump);
    RUN_TEST(testRunUpWithoutButtonsNeitherJumpsNorAttacks);
    RUN_TEST(testRunUpWithBHeldStartsTheUpSpecial);
    RUN_TEST(testRunUpWithAHeldStartsTheUpSmash);
    RUN_TEST(testRunUpWithBPressedThisFrameFallsToTheSmashCheck);
    RUN_TEST(testRunUpWithAPressedThisFrameDoesNothing);
    RUN_TEST(testRunUpSpecialWinsOverSmashWhenBothAreHeld);
    RUN_TEST(testDashAttackIsSkippedWithTheStickUpInStandardModes);
    RUN_TEST(testDashAttackStaysWithTheStickLevelOrDown);
    RUN_TEST(testDashAttackStaysInJModesAndOff);
    RUN_TEST(testShieldMovesToLWithTheControlOn);
    RUN_TEST(testShieldKeepsZWithTheControlOff);
    RUN_TEST(testShieldAnswersToLAndZWhenTheTauntButtonIsCustom);
    RUN_TEST(testDefaultTauntMovesToZWithTheControlOn);
    RUN_TEST(testCustomTauntAndControlOffKeepTheTauntMask);
    RUN_TEST(testMenuStickSwapSwapsOnEveryScreenButAnUnpausedMatch);
    RUN_TEST(testMenuStickSwapOnTheTrainingScreenSwapsTheRunningMatchToo);
    RUN_TEST(testMenuSticklessZeroesTheStickOfUnpausedMatchesAndTheTitle);
    RUN_TEST(testMenuSticklessJKeepsTheStickOfUnpausedMatchesAndTheTitle);
    RUN_TEST(testMenuSticklessKeepsTheStickOfAPausedMatchOutsideTraining);
    RUN_TEST(testMenuOffNeverTouchesAMatchOrTheTitle);
    RUN_TEST(testMenuSticklessOpensThePausedTrainingMenuToTheDpad);
    RUN_TEST(testMenuCursorNeedsACssSceneAndTheOptionOrTheToggle);
    RUN_TEST(testMenuSwapPressesTheDpadOnTheFirstFrameOfAStickPush);
    RUN_TEST(testMenuSwapKeepsHoldingWithoutANewPress);
    RUN_TEST(testMenuSwapDeadZoneIs40OnXAnd30OnY);
    RUN_TEST(testMenuSwapOnYTestsUpAndDownHeldBeforeAPress);
    RUN_TEST(testMenuSwapMovesTheStickWithTheDpadAndDropsTheDpadBits);
    RUN_TEST(testMenuSwapRightAndUpWinOverLeftAndDown);
    RUN_TEST(testMenuDpadAsStickKeepsTheStickWithoutTheDpad);
    RUN_TEST(testMenuDpadAsStickReplacesTheAxesOfTheHeldDirections);
    RUN_TEST(testMenuClearStickZeroesBothAxesAndKeepsTheButtons);
    RUN_TEST(testMenuTurboStepFiresOnEveryFifthCall);
    RUN_TEST(testMenuSwapTrainingAddsDrToTheRepeatEveryFifthFrameOfStickRight);
    RUN_TEST(testMenuSwapTrainingFirstFramePressesWithoutTheRepeatUntilTheTimerFires);
    RUN_TEST(testMenuSwapTrainingTurboNeedsTheStickRightPastTheDeadZone);
    RUN_TEST(testMenuSwapTrainingKeepsTheTimerWhenTheStickIsReleased);
    RUN_TEST(testMenuSwapTrainingStillSwapsTheDpadToTheStick);
    RUN_TEST(testMenuSwapOutsideTrainingNeverAddsTheTurbo);
    RUN_TEST(testMenuCursorFirstFrameOfAPressOnlyArmsTheDelay);
    RUN_TEST(testMenuCursorMovesAtOnceWhenNoTokenIsOverAFighter);
    RUN_TEST(testMenuCursorWaitsOutTheDelayWhileATokenIsOverAFighter);
    RUN_TEST(testMenuCursorKeepsMovingOnceTheDelayIsOver);
    RUN_TEST(testMenuCursorRightAndUpWinOverLeftAndDown);
    RUN_TEST(testMenuCursorReadsThePreviousFrameDpadAndKeepsTheStick);
}
