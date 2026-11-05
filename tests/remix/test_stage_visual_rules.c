/**
 * test_stage_visual_rules.c - unit tests of the stage visual rules: Saffron Pokemon Rate (gr/grremix/grremixsaffronrules.c),
 * platform movement gate (grremixplatformrules.c), wallpaper kind and Big Blue scroll (grremixwallpaperrules.c) and the item
 * display link (it/itremix/itremixdisplaylinkrules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "gr/grremix/grremixplatformrules.h"
#include "gr/grremix/grremixsaffronrules.h"
#include "gr/grremix/grremixwallpaperrules.h"
#include "it/itremix/itremixdisplaylinkrules.h"

static void testSaffronDefaultRateKeepsTheVanillaWaits(void)
{
    CHECK_EQ(1500, grRemixSaffronRateSleepMonsterWait(GRREMIX_SAFFRON_RATE_DEFAULT, 1500));
    CHECK_EQ(1500, grRemixSaffronRateClosedMonsterWait(GRREMIX_SAFFRON_RATE_DEFAULT, 1500));
    CHECK_EQ(1000, grRemixSaffronRateClosedGateWait(GRREMIX_SAFFRON_RATE_DEFAULT, 1000));
}

static void testSaffronSuperAndHyperHalveThePokemonWait(void)
{
    CHECK_EQ(750, grRemixSaffronRateSleepMonsterWait(GRREMIX_SAFFRON_RATE_SUPER, 1500));
    CHECK_EQ(750, grRemixSaffronRateSleepMonsterWait(GRREMIX_SAFFRON_RATE_HYPER, 1500));
    CHECK_EQ(750, grRemixSaffronRateClosedMonsterWait(GRREMIX_SAFFRON_RATE_SUPER, 1500));
    CHECK_EQ(750, grRemixSaffronRateClosedMonsterWait(GRREMIX_SAFFRON_RATE_HYPER, 1500));
}

static void testSaffronQuickAttackUsesTheShortWaits(void)
{
    CHECK_EQ(1, grRemixSaffronRateSleepMonsterWait(GRREMIX_SAFFRON_RATE_QUICK_ATTACK, 1500));
    CHECK_EQ(15, grRemixSaffronRateClosedMonsterWait(GRREMIX_SAFFRON_RATE_QUICK_ATTACK, 1500));
}

static void testSaffronGateWaitHalvesForSuperAndOpensAtOnceOtherwise(void)
{
    CHECK_EQ(500, grRemixSaffronRateClosedGateWait(GRREMIX_SAFFRON_RATE_SUPER, 1000));
    CHECK_EQ(1, grRemixSaffronRateClosedGateWait(GRREMIX_SAFFRON_RATE_HYPER, 1000));
    CHECK_EQ(1, grRemixSaffronRateClosedGateWait(GRREMIX_SAFFRON_RATE_QUICK_ATTACK, 1000));
}

static void testPlatformMovementFollowsTheHazardToggleOnOnettAndTalTal(void)
{
    const int32_t both = GRREMIX_PLATFORM_HAZARD_TYPE_HAZARDS | GRREMIX_PLATFORM_HAZARD_TYPE_MOVEMENT;

    CHECK_EQ(0, grRemixPlatformRulesMovementIsOn(nGRRemixStage_ONETT, both, 0, 1));
    CHECK_EQ(1, grRemixPlatformRulesMovementIsOn(nGRRemixStage_ONETT, both, 1, 0));
    CHECK_EQ(0, grRemixPlatformRulesMovementIsOn(nGRRemixStage_TALTAL, 0, 0, 1));
    CHECK_EQ(0, grRemixPlatformRulesMovementIsOn(nGRRemixStage_TALTAL_REMIX, 0, 0, 1));
    CHECK_EQ(1, grRemixPlatformRulesMovementIsOn(nGRRemixStage_TALTAL_REMIX, 0, 1, 0));
}

static void testPlatformMovementFollowsTheMovementToggleWhenTheStageHasMovement(void)
{
    CHECK_EQ(0, grRemixPlatformRulesMovementIsOn(nGRRemixStage_PEACHS_CASTLE, GRREMIX_PLATFORM_HAZARD_TYPE_MOVEMENT, 1, 0));
    CHECK_EQ(1, grRemixPlatformRulesMovementIsOn(nGRRemixStage_PEACHS_CASTLE, GRREMIX_PLATFORM_HAZARD_TYPE_MOVEMENT, 0, 1));
    CHECK_EQ(0, grRemixPlatformRulesMovementIsOn(nGRRemixStage_BATTLEFIELD, 3, 1, 0));
}

static void testPlatformMovementIgnoresTheToggleWithoutTheMovementBit(void)
{
    CHECK_EQ(1, grRemixPlatformRulesMovementIsOn(nGRRemixStage_SECTOR_Z, GRREMIX_PLATFORM_HAZARD_TYPE_HAZARDS, 0, 0));
    CHECK_EQ(1, grRemixPlatformRulesMovementIsOn(nGRRemixStage_DEKU_TREE, 0, 0, 0));
}

static void testWallpaperKindOfRemixRttfIsBonus3(void)
{
    CHECK_EQ(GRREMIX_WALLPAPER_KIND_BONUS3, grRemixWallpaperRulesKind(nGRRemixStage_REMIX_RTTF, 0x80460000u));
    CHECK_EQ(GRREMIX_WALLPAPER_KIND_BONUS3, grRemixWallpaperRulesKind(nGRRemixStage_REMIX_RTTF, 0));
}

static void testWallpaperKindWithoutFunctionIsStatic(void)
{
    CHECK_EQ(GRREMIX_WALLPAPER_KIND_STATIC, grRemixWallpaperRulesKind(nGRRemixStage_BTT_DRM, 0));
    CHECK_EQ(GRREMIX_WALLPAPER_KIND_STATIC, grRemixWallpaperRulesKind(nGRRemixStage_BTP_WARIO, 0));
}

static void testWallpaperKindWithFunctionIsCommon(void)
{
    CHECK_EQ(GRREMIX_WALLPAPER_KIND_COMMON, grRemixWallpaperRulesKind(nGRRemixStage_BLUE, 0x80460000u));
    CHECK_EQ(GRREMIX_WALLPAPER_KIND_COMMON, grRemixWallpaperRulesKind(nGRRemixStage_DEKU_TREE, 0x80461234u));
}

static void testBlueBackgroundScrollsOnlyWhileTheMatchWaitsOrRuns(void)
{
    CHECK_TRUE(grRemixBlueBackgroundRulesSpeed(0) < 0.0F);
    CHECK_TRUE(grRemixBlueBackgroundRulesSpeed(1) < 0.0F);
    CHECK_TRUE(grRemixBlueBackgroundRulesSpeed(2) == 0.0F);
    CHECK_TRUE(grRemixBlueBackgroundRulesSpeed(3) == 0.0F);
    CHECK_TRUE(grRemixBlueBackgroundRulesSpeed(5) == 0.0F);
}

static void testItemDisplayLinkByKind(void)
{
    CHECK_EQ(ITREMIX_DISPLAY_LINK_HAZARD, itRemixDisplayLinkRulesForKind(0x3A));
    CHECK_EQ(ITREMIX_DISPLAY_LINK_HAZARD, itRemixDisplayLinkRulesForKind(0x3B));
    CHECK_EQ(ITREMIX_DISPLAY_LINK_HAZARD, itRemixDisplayLinkRulesForKind(0x3F));
    CHECK_EQ(ITREMIX_DISPLAY_LINK_CAR, itRemixDisplayLinkRulesForKind(0x3C));
}

static void testItemDisplayLinkDefaultsToTheVanillaLink(void)
{
    CHECK_EQ(11, itRemixDisplayLinkRulesForKind(0x00));
    CHECK_EQ(11, itRemixDisplayLinkRulesForKind(0x39));
    CHECK_EQ(11, itRemixDisplayLinkRulesForKind(0x3D));
    CHECK_EQ(11, itRemixDisplayLinkRulesForKind(0x4B));
}

void remixTestStageVisualRules(void)
{
    RUN_TEST(testSaffronDefaultRateKeepsTheVanillaWaits);
    RUN_TEST(testSaffronSuperAndHyperHalveThePokemonWait);
    RUN_TEST(testSaffronQuickAttackUsesTheShortWaits);
    RUN_TEST(testSaffronGateWaitHalvesForSuperAndOpensAtOnceOtherwise);
    RUN_TEST(testPlatformMovementFollowsTheHazardToggleOnOnettAndTalTal);
    RUN_TEST(testPlatformMovementFollowsTheMovementToggleWhenTheStageHasMovement);
    RUN_TEST(testPlatformMovementIgnoresTheToggleWithoutTheMovementBit);
    RUN_TEST(testWallpaperKindOfRemixRttfIsBonus3);
    RUN_TEST(testWallpaperKindWithoutFunctionIsStatic);
    RUN_TEST(testWallpaperKindWithFunctionIsCommon);
    RUN_TEST(testBlueBackgroundScrollsOnlyWhileTheMatchWaitsOrRuns);
    RUN_TEST(testItemDisplayLinkByKind);
    RUN_TEST(testItemDisplayLinkDefaultsToTheVanillaLink);
}
