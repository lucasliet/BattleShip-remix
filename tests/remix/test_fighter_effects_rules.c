/**
 * test_fighter_effects_rules.c - unit tests of the fighter effect rules (ft/ftremix/ftremixeffectsrules.c): the Dedede inhale
 * wind and captured timer (Dedede.asm), the Falcon punch gfx routing (captainshared.asm get_punch_anim_struct_) and the Wolf
 * and Slippy reflector tables (Wolf.asm wolf_reflect_graphic).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixeffectsrules.h"
#include "remix/remix_roster_data.h"

static void testOnlyDededeMovesTheInhaleWind(void)
{
    CHECK_EQ(1000, ftRemixEffectsRulesInhaleWindOffsetX(REMIX_ID_DEDEDE));
    CHECK_EQ(800, ftRemixEffectsRulesInhaleWindOffsetX(REMIX_ID_KIRBY));
    CHECK_EQ(800, ftRemixEffectsRulesInhaleWindOffsetX(REMIX_ID_NDEDEDE));
}

static void testDededeInhaleBreakoutIsHalfPlusDamage(void)
{
    CHECK_EQ(250, ftRemixEffectsRulesInhaleBreakoutWait(REMIX_ID_DEDEDE, 0, 500));
    CHECK_EQ(337, ftRemixEffectsRulesInhaleBreakoutWait(REMIX_ID_DEDEDE, 87, 500));
}

static void testOtherAbsorbersKeepTheVanillaBreakout(void)
{
    CHECK_EQ(500, ftRemixEffectsRulesInhaleBreakoutWait(REMIX_ID_KIRBY, 87, 500));
    CHECK_EQ(500, ftRemixEffectsRulesInhaleBreakoutWait(REMIX_ID_NDEDEDE, 87, 500));
}

static void testPunchGfxRoutesTheFalconClonesAndWolf(void)
{
    CHECK_EQ(nFTRemixEffectsPunchGfxOwnFile8, ftRemixEffectsRulesPunchGfx(REMIX_ID_GND));
    CHECK_EQ(nFTRemixEffectsPunchGfxOwnFile8, ftRemixEffectsRulesPunchGfx(REMIX_ID_JFALCON));
    CHECK_EQ(nFTRemixEffectsPunchGfxOwnFile8, ftRemixEffectsRulesPunchGfx(REMIX_ID_DRAGONKING));
    CHECK_EQ(nFTRemixEffectsPunchGfxWolfSlash, ftRemixEffectsRulesPunchGfx(REMIX_ID_WOLF));
    CHECK_EQ(nFTRemixEffectsPunchGfxFalcon, ftRemixEffectsRulesPunchGfx(REMIX_ID_KIRBY));
    CHECK_EQ(nFTRemixEffectsPunchGfxFalcon, ftRemixEffectsRulesPunchGfx(REMIX_ID_NWOLF));
}

static void testPunchAnnouncesExceptGndWolfAndMewtwo(void)
{
    CHECK_TRUE(ftRemixEffectsRulesPunchAnnounces(REMIX_ID_JFALCON));
    CHECK_TRUE(ftRemixEffectsRulesPunchAnnounces(REMIX_ID_DRAGONKING));
    CHECK_TRUE(ftRemixEffectsRulesPunchAnnounces(REMIX_ID_KIRBY));
    CHECK_TRUE(!ftRemixEffectsRulesPunchAnnounces(REMIX_ID_GND));
    CHECK_TRUE(!ftRemixEffectsRulesPunchAnnounces(REMIX_ID_WOLF));
    CHECK_TRUE(!ftRemixEffectsRulesPunchAnnounces(REMIX_ID_MTWO));
}

static void testReflectorOwnFileIsWolfAndSlippyOnly(void)
{
    CHECK_TRUE(ftRemixEffectsRulesReflectorUsesOwnFile(REMIX_ID_WOLF));
    CHECK_TRUE(ftRemixEffectsRulesReflectorUsesOwnFile(REMIX_ID_SLIPPY));
    CHECK_TRUE(!ftRemixEffectsRulesReflectorUsesOwnFile(REMIX_ID_FOX));
    CHECK_TRUE(!ftRemixEffectsRulesReflectorUsesOwnFile(REMIX_ID_NWOLF));
}

static void testReflectorDescOffsets(void)
{
    CHECK_EQ(0x288, ftRemixEffectsRulesReflectorDObjSetup(REMIX_ID_WOLF, 0x111));
    CHECK_EQ(0x41C, ftRemixEffectsRulesReflectorFirstAnimJoint(REMIX_ID_WOLF, 0x111));
    CHECK_EQ(0x2B0, ftRemixEffectsRulesReflectorDObjSetup(REMIX_ID_SLIPPY, 0x111));
    CHECK_EQ(0x340, ftRemixEffectsRulesReflectorFirstAnimJoint(REMIX_ID_SLIPPY, 0x111));
    CHECK_EQ(0x111, ftRemixEffectsRulesReflectorDObjSetup(REMIX_ID_FOX, 0x111));
    CHECK_EQ(0x111, ftRemixEffectsRulesReflectorFirstAnimJoint(REMIX_ID_FOX, 0x111));
}

static void testReflectorAnimJointTables(void)
{
    CHECK_EQ(0x41C, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_WOLF, 0, 7));
    CHECK_EQ(0x4C4, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_WOLF, 1, 7));
    CHECK_EQ(0x5E8, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_WOLF, 2, 7));
    CHECK_EQ(0x6FC, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_WOLF, 3, 7));
    CHECK_EQ(0x320, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_SLIPPY, 0, 7));
    CHECK_EQ(0x4C0, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_SLIPPY, 1, 7));
    CHECK_EQ(0x3A0, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_SLIPPY, 2, 7));
    CHECK_EQ(0x430, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_SLIPPY, 3, 7));
    CHECK_EQ(7, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_FOX, 2, 7));
    CHECK_EQ(7, ftRemixEffectsRulesReflectorAnimJoint(REMIX_ID_WOLF, 4, 7));
}

static void testPsiMagnetFile6BelongsToJNessAndTheLucasFamily(void)
{
    CHECK_TRUE(ftRemixEffectsRulesPsiMagnetUsesFile6(REMIX_ID_JNESS));
    CHECK_TRUE(ftRemixEffectsRulesPsiMagnetUsesFile6(REMIX_ID_LUCAS));
    CHECK_TRUE(ftRemixEffectsRulesPsiMagnetUsesFile6(REMIX_ID_NLUCAS));
    CHECK_TRUE(!ftRemixEffectsRulesPsiMagnetUsesFile6(REMIX_ID_NESS));
}

void remixTestFighterEffectsRules(void)
{
    printf("fighter effects rules\n");
    RUN_TEST(testOnlyDededeMovesTheInhaleWind);
    RUN_TEST(testDededeInhaleBreakoutIsHalfPlusDamage);
    RUN_TEST(testOtherAbsorbersKeepTheVanillaBreakout);
    RUN_TEST(testPunchGfxRoutesTheFalconClonesAndWolf);
    RUN_TEST(testPunchAnnouncesExceptGndWolfAndMewtwo);
    RUN_TEST(testReflectorOwnFileIsWolfAndSlippyOnly);
    RUN_TEST(testReflectorDescOffsets);
    RUN_TEST(testReflectorAnimJointTables);
    RUN_TEST(testPsiMagnetFile6BelongsToJNessAndTheLucasFamily);
}
