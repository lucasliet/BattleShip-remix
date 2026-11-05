/**
 * test_ai_combat_rules.c - unit tests of the Remix AI combat and recovery rules
 * (ft/ftremix/ftremixaicombatrules.c, ft/ftremix/ftremixairecoveryrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaicombatrules.h"
#include "ft/ftremix/ftremixairecoveryrules.h"

static void testStarTimerAtLimitIsInvulnerable(void)
{
    CHECK_TRUE(ftRemixAiCombatIsInvulnerable(112, 0));
}

static void testStarTimerBelowLimitIsNotInvulnerable(void)
{
    CHECK_EQ(0, ftRemixAiCombatIsInvulnerable(111, 0));
}

static void testRespawnTimerAtLimitIsInvulnerable(void)
{
    CHECK_TRUE(ftRemixAiCombatIsInvulnerable(0, 20));
}

static void testRespawnTimerBelowLimitIsNotInvulnerable(void)
{
    CHECK_EQ(0, ftRemixAiCombatIsInvulnerable(0, 19));
}

static void testImprovedAiSuppressesEvadeWhenInvulnerable(void)
{
    CHECK_TRUE(ftRemixAiCombatEvadeSuppressed(1, 1));
}

static void testVanillaAiKeepsEvadingWhenInvulnerable(void)
{
    CHECK_EQ(0, ftRemixAiCombatEvadeSuppressed(0, 1));
}

static void testImprovedAiKeepsEvadingWhenVulnerable(void)
{
    CHECK_EQ(0, ftRemixAiCombatEvadeSuppressed(1, 0));
}

static void testLevelBelowTenAlwaysCharges(void)
{
    CHECK_TRUE(ftRemixAiCombatChargeAllowed(9, 59));
}

static void testLevelTenChargesOnlyOnRollZero(void)
{
    CHECK_TRUE(ftRemixAiCombatChargeAllowed(10, 0));
}

static void testLevelTenSkipsChargeOnAnyOtherRoll(void)
{
    CHECK_EQ(0, ftRemixAiCombatChargeAllowed(10, 1));
}

static void testMLuigiOrGBowserIgnoresProjectiles(void)
{
    CHECK_TRUE(ftRemixAiCombatIgnoresProjectile(1, 0));
}

static void testFranklinBadgeIgnoresProjectiles(void)
{
    CHECK_TRUE(ftRemixAiCombatIgnoresProjectile(0, 1));
}

static void testOtherFightersReactToProjectiles(void)
{
    CHECK_EQ(0, ftRemixAiCombatIgnoresProjectile(0, 0));
}

static void testLevelTenAlwaysRepeatsUpB(void)
{
    CHECK_TRUE(ftRemixAiRecoveryUpBRepeatAllowed(10, 1, 0, 0));
}

static void testClassicModeNeverRepeatsUpBBelowLevelTen(void)
{
    CHECK_EQ(0, ftRemixAiRecoveryUpBRepeatAllowed(9, 1, 1, 0));
}

static void testImprovedAiRepeatsUpBBelowLevelTen(void)
{
    CHECK_TRUE(ftRemixAiRecoveryUpBRepeatAllowed(5, 0, 1, 0));
}

static void testVanillaAiKeepsUpBLimitBelowLevelTen(void)
{
    CHECK_EQ(0, ftRemixAiRecoveryUpBRepeatAllowed(5, 0, 0, 0));
}

static void testRemix1PRepeatsUpBEvenInClassicMode(void)
{
    CHECK_TRUE(ftRemixAiRecoveryUpBRepeatAllowed(3, 1, 0, 1));
}

static void testTargetInsideBlastzoneIsKept(void)
{
    CHECK_TRUE(ftRemixAiRecoveryClampTargetX(0.0F, -1000.0F, 1000.0F) == 0.0F);
}

static void testTargetNearLeftEdgeIsClampedToPaddedLeft(void)
{
    CHECK_TRUE(ftRemixAiRecoveryClampTargetX(-900.0F, -1000.0F, 1000.0F) == -800.0F);
}

static void testTargetNearRightEdgeIsClampedToPaddedRight(void)
{
    CHECK_TRUE(ftRemixAiRecoveryClampTargetX(900.0F, -1000.0F, 1000.0F) == 800.0F);
}

static void testWolfUspActionAimsRecoveryUp(void)
{
    CHECK_TRUE(ftRemixAiRecoveryIsWolfUspAction(0xE4));
}

static void testWolfIdleActionKeepsRecoveryTarget(void)
{
    CHECK_EQ(0, ftRemixAiRecoveryIsWolfUspAction(0xE5));
}

static void testNspChargeActionMatchesItsCharge(void)
{
    CHECK_TRUE(ftRemixAiRecoveryIsNspChargeAction(0xEE, 0xEE));
}

static void testNegativeChargeActionNeverMatches(void)
{
    CHECK_EQ(0, ftRemixAiRecoveryIsNspChargeAction(-1, -1));
}

static void testPikachuVanillaInClassicMode(void)
{
    CHECK_TRUE(ftRemixAiRecoveryKeepsPikachuVanilla(1, 1));
}

static void testPikachuVanillaWhenImprovedAiOff(void)
{
    CHECK_TRUE(ftRemixAiRecoveryKeepsPikachuVanilla(0, 0));
}

static void testPikachuRemixDispatchWhenImprovedAiOn(void)
{
    CHECK_EQ(0, ftRemixAiRecoveryKeepsPikachuVanilla(0, 1));
}

void remixTestAiCombatRules(void)
{
    printf("ai combat rules\n");
    RUN_TEST(testStarTimerAtLimitIsInvulnerable);
    RUN_TEST(testStarTimerBelowLimitIsNotInvulnerable);
    RUN_TEST(testRespawnTimerAtLimitIsInvulnerable);
    RUN_TEST(testRespawnTimerBelowLimitIsNotInvulnerable);
    RUN_TEST(testImprovedAiSuppressesEvadeWhenInvulnerable);
    RUN_TEST(testVanillaAiKeepsEvadingWhenInvulnerable);
    RUN_TEST(testImprovedAiKeepsEvadingWhenVulnerable);
    RUN_TEST(testLevelBelowTenAlwaysCharges);
    RUN_TEST(testLevelTenChargesOnlyOnRollZero);
    RUN_TEST(testLevelTenSkipsChargeOnAnyOtherRoll);
    RUN_TEST(testMLuigiOrGBowserIgnoresProjectiles);
    RUN_TEST(testFranklinBadgeIgnoresProjectiles);
    RUN_TEST(testOtherFightersReactToProjectiles);
    RUN_TEST(testLevelTenAlwaysRepeatsUpB);
    RUN_TEST(testClassicModeNeverRepeatsUpBBelowLevelTen);
    RUN_TEST(testImprovedAiRepeatsUpBBelowLevelTen);
    RUN_TEST(testVanillaAiKeepsUpBLimitBelowLevelTen);
    RUN_TEST(testRemix1PRepeatsUpBEvenInClassicMode);
    RUN_TEST(testTargetInsideBlastzoneIsKept);
    RUN_TEST(testTargetNearLeftEdgeIsClampedToPaddedLeft);
    RUN_TEST(testTargetNearRightEdgeIsClampedToPaddedRight);
    RUN_TEST(testWolfUspActionAimsRecoveryUp);
    RUN_TEST(testWolfIdleActionKeepsRecoveryTarget);
    RUN_TEST(testNspChargeActionMatchesItsCharge);
    RUN_TEST(testNegativeChargeActionNeverMatches);
    RUN_TEST(testPikachuVanillaInClassicMode);
    RUN_TEST(testPikachuVanillaWhenImprovedAiOff);
    RUN_TEST(testPikachuRemixDispatchWhenImprovedAiOn);
}
