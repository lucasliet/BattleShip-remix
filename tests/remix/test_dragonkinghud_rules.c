/**
 * test_dragonkinghud_rules.c - unit tests of the Dragon King HUD rules (if/ifremixdragonkinghudrules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "if/ifremixdragonkinghudrules.h"

static void testTheToggleOffNeverAppliesTheLayout(void)
{
    CHECK_EQ(0, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_OFF, nGRRemixStage_DRAGONKING));
    CHECK_EQ(0, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_OFF, nGRRemixStage_PEACHS_CASTLE));
}

static void testTheToggleAlwaysAppliesTheLayoutOnEveryStage(void)
{
    CHECK_EQ(1, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_ALWAYS, nGRRemixStage_PEACHS_CASTLE));
    CHECK_EQ(1, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_ALWAYS, nGRRemixStage_HRC));
}

static void testTheDefaultToggleAppliesTheLayoutOnTheDragonKingStagesOnly(void)
{
    CHECK_EQ(1, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_DRAGONKING));
    CHECK_EQ(1, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_DRAGONKING_REMIX));
    CHECK_EQ(0, ifRemixDragonKingHudRulesIsOn(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_FINAL_DESTINATION));
}

static void testTheDamageNumbersMoveUpOnlyWhenTheLayoutApplies(void)
{
    CHECK_EQ(38, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_ALWAYS, nGRRemixStage_PEACHS_CASTLE, 210, 0));
    CHECK_EQ(38, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_DRAGONKING, 150, 0));
    CHECK_EQ(210, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_PEACHS_CASTLE, 210, 0));
    CHECK_EQ(210, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_OFF, nGRRemixStage_DRAGONKING, 210, 0));
}

static void testTheOnePlayerGameKeepsTheVanillaYOnTheDragonKingRemixStage(void)
{
    CHECK_EQ(210, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_DRAGONKING_REMIX, 210, 1));
    CHECK_EQ(38, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_DRAGONKING_REMIX, 210, 0));
    CHECK_EQ(38, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_ALWAYS, nGRRemixStage_DRAGONKING_REMIX, 210, 1));
    CHECK_EQ(38, ifRemixDragonKingHudRulesDamageY(IFREMIX_DKHUD_TOGGLE_STAGES, nGRRemixStage_DRAGONKING, 210, 1));
}

static void testTheDigitTablesFollowTheAsm(void)
{
    CHECK_EQ(0x17B8, ifRemixDragonKingHudRulesDigitOffset(0));
    CHECK_EQ(0x2C58, ifRemixDragonKingHudRulesDigitOffset(9));
    CHECK_EQ(0x3198, ifRemixDragonKingHudRulesDigitOffset(11));
    CHECK_EQ(0, ifRemixDragonKingHudRulesDigitOffset(12));
    CHECK_EQ(0x11, ifRemixDragonKingHudRulesDigitWidth(0));
    CHECK_EQ(0x0C, ifRemixDragonKingHudRulesDigitWidth(1));
    CHECK_EQ(0x17, ifRemixDragonKingHudRulesDigitWidth(11));
    CHECK_EQ(0, ifRemixDragonKingHudRulesDigitWidth(-1));
}

static void testHumansInAFreeForAllUseThePortColor(void)
{
    IFRemixDragonKingHudColorInput input = { 2, 0, 0, 0 };
    unsigned char rgb[3];

    ifRemixDragonKingHudRulesColor(&input, rgb);

    CHECK_EQ(0xEB, rgb[0]);
    CHECK_EQ(0xE7, rgb[1]);
    CHECK_EQ(0x8D, rgb[2]);
}

static void testCpusInAFreeForAllAreWhite(void)
{
    IFRemixDragonKingHudColorInput input = { 1, 0, 1, 1 };
    unsigned char rgb[3];

    ifRemixDragonKingHudRulesColor(&input, rgb);

    CHECK_EQ(0xFF, rgb[0]);
    CHECK_EQ(0xFF, rgb[1]);
    CHECK_EQ(0xFF, rgb[2]);
}

static void testTeamBattlesUseTheTeamColorEvenForCpus(void)
{
    IFRemixDragonKingHudColorInput input = { 0, 1, 2, 1 };
    unsigned char rgb[3];

    ifRemixDragonKingHudRulesColor(&input, rgb);

    CHECK_EQ(0xC3, rgb[0]);
    CHECK_EQ(0xFB, rgb[1]);
    CHECK_EQ(0xCE, rgb[2]);
}

static void testTheTimerIsLoweredByTheDragonKingHudThenByTheInputDisplayAtTheTop(void)
{
    CHECK_EQ(212, (int)ifRemixDragonKingHudRulesTimerY(1, 0));
    CHECK_EQ(212, (int)ifRemixDragonKingHudRulesTimerY(1, 1));
    CHECK_EQ(52, (int)ifRemixDragonKingHudRulesTimerY(0, 1));
    CHECK_EQ(30, (int)ifRemixDragonKingHudRulesTimerY(0, 0));
    CHECK_EQ(30, (int)ifRemixDragonKingHudRulesTimerY(0, 2));
}

void remixTestDragonKingHudRules(void)
{
    printf("dragon king hud rules\n");
    RUN_TEST(testTheToggleOffNeverAppliesTheLayout);
    RUN_TEST(testTheToggleAlwaysAppliesTheLayoutOnEveryStage);
    RUN_TEST(testTheDefaultToggleAppliesTheLayoutOnTheDragonKingStagesOnly);
    RUN_TEST(testTheDamageNumbersMoveUpOnlyWhenTheLayoutApplies);
    RUN_TEST(testTheOnePlayerGameKeepsTheVanillaYOnTheDragonKingRemixStage);
    RUN_TEST(testTheDigitTablesFollowTheAsm);
    RUN_TEST(testHumansInAFreeForAllUseThePortColor);
    RUN_TEST(testCpusInAFreeForAllAreWhite);
    RUN_TEST(testTeamBattlesUseTheTeamColorEvenForCpus);
    RUN_TEST(testTheTimerIsLoweredByTheDragonKingHudThenByTheInputDisplayAtTheTop);
}
