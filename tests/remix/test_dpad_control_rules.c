/**
 * test_dpad_control_rules.c - unit tests of the Remix D-pad Control action rules (ft/ftremix/ftremixdpadcontrolrules.c):
 * the stick jump, run up smash/special, dash attack and shield mask of css/DpadControl.asm.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixdpadcontrolrules.h"

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
}
