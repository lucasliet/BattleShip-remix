/**
 * test_ness_rules.c - unit tests of the Ness family patch rules (ft/ftremix/ftremixnessrules.c): the J Ness and Lucas PK Fire
 * and PK Thunder values of nessshared.asm and the Lucas PSI Magnet and PK Thunder multi-hit rules of Lucas.asm.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixnessrules.h"
#include "remix/remix_roster_data.h"

#define VANILLA_SPEED_GROUND 73.0F
#define VANILLA_SPEED_AIR 95.0F
#define VANILLA_GRAVITY 0.45F
#define VANILLA_TVEL 55.0F
#define VANILLA_LANDING_LAG 0.17F
#define VANILLA_DRIFT 0.6F
#define VANILLA_END_DELAY 30

static void testOnlyJNessAndLucasUseTheirOwnFiles(void)
{
    CHECK_TRUE(ftRemixNessRulesUsesOwnFiles(REMIX_ID_JNESS));
    CHECK_TRUE(ftRemixNessRulesUsesOwnFiles(REMIX_ID_LUCAS));
    CHECK_TRUE(!ftRemixNessRulesUsesOwnFiles(REMIX_ID_NESS));
    CHECK_TRUE(!ftRemixNessRulesUsesOwnFiles(REMIX_ID_NLUCAS));
    CHECK_TRUE(!ftRemixNessRulesUsesOwnFiles(-1));
}

static void testLucasPKFireIsAlwaysTheGroundedLaunch(void)
{
    CHECK_EQ(0, ftRemixNessRulesPKFireIsAir(REMIX_ID_LUCAS, 1));
    CHECK_EQ(0, ftRemixNessRulesPKFireIsAir(REMIX_ID_LUCAS, 0));
}

static void testOtherPKFireLaunchesKeepTheAirFlag(void)
{
    CHECK_EQ(1, ftRemixNessRulesPKFireIsAir(REMIX_ID_NESS, 1));
    CHECK_EQ(1, ftRemixNessRulesPKFireIsAir(REMIX_ID_JNESS, 1));
    CHECK_EQ(0, ftRemixNessRulesPKFireIsAir(REMIX_ID_JNESS, 0));
}

static void testJNessAndLucasPKFireSpeedsAre90AndTheAirSpeed120(void)
{
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_JNESS, 0, VANILLA_SPEED_GROUND) == 90.0F);
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_JNESS, 1, VANILLA_SPEED_AIR) == 120.0F);
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_LUCAS, 0, VANILLA_SPEED_GROUND) == 90.0F);
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_LUCAS, 1, VANILLA_SPEED_AIR) == 120.0F);
}

static void testNessPKFireSpeedsAreTheVanillaOnes(void)
{
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_NESS, 0, VANILLA_SPEED_GROUND) == VANILLA_SPEED_GROUND);
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_NESS, 1, VANILLA_SPEED_AIR) == VANILLA_SPEED_AIR);
    CHECK_TRUE(ftRemixNessRulesPKFireSpeed(REMIX_ID_NNESS, 1, VANILLA_SPEED_AIR) == VANILLA_SPEED_AIR);
}

static void testOnlyLucasPKFireExplodes(void)
{
    CHECK_TRUE(ftRemixNessRulesPKFireExplodes(REMIX_ID_LUCAS));
    CHECK_TRUE(!ftRemixNessRulesPKFireExplodes(REMIX_ID_JNESS));
    CHECK_TRUE(!ftRemixNessRulesPKFireExplodes(REMIX_ID_NESS));
}

static void testOnlyJNessPillarHasItsOwnFileAndPhysics(void)
{
    CHECK_TRUE(ftRemixNessRulesPKFirePillarUsesOwnFile(REMIX_ID_JNESS));
    CHECK_TRUE(!ftRemixNessRulesPKFirePillarUsesOwnFile(REMIX_ID_NESS));
    CHECK_TRUE(ftRemixNessRulesPKFirePillarGravity(REMIX_ID_JNESS, VANILLA_GRAVITY) == 0.4F);
    CHECK_TRUE(ftRemixNessRulesPKFirePillarTVel(REMIX_ID_JNESS, VANILLA_TVEL) == 50.0F);
    CHECK_TRUE(ftRemixNessRulesPKFirePillarGravity(REMIX_ID_LUCAS, VANILLA_GRAVITY) == VANILLA_GRAVITY);
    CHECK_TRUE(ftRemixNessRulesPKFirePillarTVel(REMIX_ID_NESS, VANILLA_TVEL) == VANILLA_TVEL);
}

static void testPKThunderLandingLagPerFighter(void)
{
    CHECK_TRUE(ftRemixNessRulesPKThunderLandingLag(REMIX_ID_JNESS, VANILLA_LANDING_LAG) == 0.24F);
    CHECK_TRUE(ftRemixNessRulesPKThunderLandingLag(REMIX_ID_LUCAS, VANILLA_LANDING_LAG) == 0.3F);
    CHECK_TRUE(ftRemixNessRulesPKThunderLandingLag(REMIX_ID_NESS, VANILLA_LANDING_LAG) == VANILLA_LANDING_LAG);
}

static void testOnlyLucasPKThunderLandingDriftIsFaster(void)
{
    CHECK_TRUE(ftRemixNessRulesPKThunderDrift(REMIX_ID_LUCAS, VANILLA_DRIFT) == 0.8203125F);
    CHECK_TRUE(ftRemixNessRulesPKThunderDrift(REMIX_ID_JNESS, VANILLA_DRIFT) == VANILLA_DRIFT);
    CHECK_TRUE(ftRemixNessRulesPKThunderDrift(REMIX_ID_NESS, VANILLA_DRIFT) == VANILLA_DRIFT);
}

static void testOnlyJNessPKThunderEndsAfter20Frames(void)
{
    CHECK_EQ(20, ftRemixNessRulesPKThunderEndDelay(REMIX_ID_JNESS, VANILLA_END_DELAY));
    CHECK_EQ(VANILLA_END_DELAY, ftRemixNessRulesPKThunderEndDelay(REMIX_ID_LUCAS, VANILLA_END_DELAY));
    CHECK_EQ(VANILLA_END_DELAY, ftRemixNessRulesPKThunderEndDelay(REMIX_ID_NESS, VANILLA_END_DELAY));
}

static void testLucasPsiMagnetHealsTwoAndAHalfTimes(void)
{
    CHECK_TRUE(ftRemixNessRulesPsiMagnetHealMultiplier(REMIX_ID_LUCAS) == 2.5F);
    CHECK_TRUE(ftRemixNessRulesPsiMagnetHealMultiplier(REMIX_ID_NESS) == 2.0F);
    CHECK_TRUE(ftRemixNessRulesPsiMagnetHealMultiplier(REMIX_ID_JNESS) == 2.0F);
}

static void testOnlyLucasPsiMagnetKeepsTheFacing(void)
{
    CHECK_TRUE(ftRemixNessRulesPsiMagnetKeepsFacing(REMIX_ID_LUCAS));
    CHECK_TRUE(!ftRemixNessRulesPsiMagnetKeepsFacing(REMIX_ID_NESS));
    CHECK_TRUE(!ftRemixNessRulesPsiMagnetKeepsFacing(REMIX_ID_JNESS));
}

static void testPKThunderHitRefreshesOnMultiplesOfEightFrames(void)
{
    CHECK_TRUE(ftRemixNessRulesPKThunderHitRefreshDue(8));
    CHECK_TRUE(ftRemixNessRulesPKThunderHitRefreshDue(160));
    CHECK_TRUE(ftRemixNessRulesPKThunderHitRefreshDue(16));
    CHECK_TRUE(!ftRemixNessRulesPKThunderHitRefreshDue(7));
    CHECK_TRUE(!ftRemixNessRulesPKThunderHitRefreshDue(9));
    CHECK_TRUE(!ftRemixNessRulesPKThunderHitRefreshDue(158));
}

static void testPKThunderHitDoesNotRefreshWithoutLifetime(void)
{
    CHECK_TRUE(!ftRemixNessRulesPKThunderHitRefreshDue(0));
    CHECK_TRUE(!ftRemixNessRulesPKThunderHitRefreshDue(-8));
}

void remixTestNessRules(void)
{
    printf("ness rules\n");
    RUN_TEST(testOnlyJNessAndLucasUseTheirOwnFiles);
    RUN_TEST(testLucasPKFireIsAlwaysTheGroundedLaunch);
    RUN_TEST(testOtherPKFireLaunchesKeepTheAirFlag);
    RUN_TEST(testJNessAndLucasPKFireSpeedsAre90AndTheAirSpeed120);
    RUN_TEST(testNessPKFireSpeedsAreTheVanillaOnes);
    RUN_TEST(testOnlyLucasPKFireExplodes);
    RUN_TEST(testOnlyJNessPillarHasItsOwnFileAndPhysics);
    RUN_TEST(testPKThunderLandingLagPerFighter);
    RUN_TEST(testOnlyLucasPKThunderLandingDriftIsFaster);
    RUN_TEST(testOnlyJNessPKThunderEndsAfter20Frames);
    RUN_TEST(testLucasPsiMagnetHealsTwoAndAHalfTimes);
    RUN_TEST(testOnlyLucasPsiMagnetKeepsTheFacing);
    RUN_TEST(testPKThunderHitRefreshesOnMultiplesOfEightFrames);
    RUN_TEST(testPKThunderHitDoesNotRefreshWithoutLifetime);
}
