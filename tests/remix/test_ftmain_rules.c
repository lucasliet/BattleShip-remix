/**
 * test_ftmain_rules.c - unit tests of the pure rules behind the ftMainSetStatus and ftMainProcPhysicsMap Remix hooks:
 * map star fighters, Marina thrown animation (ft/ftremix/ftremixmainrules.c), Sandbag landing
 * (ftremixsandbagrules.c), hit recoil (ftremixhitrecoilrules.c), Magic Kirby Hat (ftremixmagichatrules.c) and the
 * translation Size factor (ftremixcommandrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixcommandrules.h"
#include "ft/ftremix/ftremixhitrecoilrules.h"
#include "ft/ftremix/ftremixmagichatrules.h"
#include "ft/ftremix/ftremixmainrules.h"
#include "ft/ftremix/ftremixsandbagrules.h"
#include "remix/remix_roster_data.h"

#define TEST_PI 3.14159265F

static void testMapStarFightersAreKirbyJKirbyAndDedede(void)
{
    CHECK_TRUE(ftRemixMainIsMapStarFighter(REMIX_ID_KIRBY));
    CHECK_TRUE(ftRemixMainIsMapStarFighter(REMIX_ID_JKIRBY));
    CHECK_TRUE(ftRemixMainIsMapStarFighter(REMIX_ID_DEDEDE));
    CHECK_TRUE(!ftRemixMainIsMapStarFighter(REMIX_ID_NKIRBY));
    CHECK_TRUE(!ftRemixMainIsMapStarFighter(REMIX_ID_MARIO));
}

static void testMarinaThrownFiles(void)
{
    CHECK_EQ(FTREMIXMAIN_FILE_DK_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_DONKEY));
    CHECK_EQ(FTREMIXMAIN_FILE_DK_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_NDONKEY));
    CHECK_EQ(FTREMIXMAIN_FILE_DK_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_GDONKEY));
    CHECK_EQ(FTREMIXMAIN_FILE_DK_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_JDK));
    CHECK_EQ(FTREMIXMAIN_FILE_FALCON_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_CAPTAIN));
    CHECK_EQ(FTREMIXMAIN_FILE_FALCON_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_GND));
    CHECK_EQ(FTREMIXMAIN_FILE_FALCON_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_NSHEIK));
    CHECK_EQ(FTREMIXMAIN_FILE_FALCON_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_DRAGONKING));
    CHECK_EQ(FTREMIXMAIN_FILE_FALCON_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_JFALCON));
    CHECK_EQ(FTREMIXMAIN_FILE_FALCON_THROWN_MARINA, ftRemixMainMarinaThrownFile(REMIX_ID_SANDBAG));
    CHECK_EQ(0, ftRemixMainMarinaThrownFile(REMIX_ID_MARIO));
    CHECK_EQ(0, ftRemixMainMarinaThrownFile(REMIX_ID_MARINA));
}

static void testSandbagLandsOnHeadWhenUpsideDown(void)
{
    CHECK_EQ(FTREMIXSANDBAG_STATUS_HEADSTAND, ftRemixSandbagGetLandingStatus(-TEST_PI));
    CHECK_EQ(FTREMIXSANDBAG_STATUS_HEADSTAND, ftRemixSandbagGetLandingStatus(-TEST_PI * 0.85F));
    CHECK_EQ(FTREMIXSANDBAG_STATUS_HEADSTAND, ftRemixSandbagGetLandingStatus(-TEST_PI * 1.15F));
}

static void testSandbagLandsOnFeetWhenUpright(void)
{
    CHECK_EQ(FTREMIXSANDBAG_STATUS_IDLE, ftRemixSandbagGetLandingStatus(0.0F));
    CHECK_EQ(FTREMIXSANDBAG_STATUS_IDLE, ftRemixSandbagGetLandingStatus(-TEST_PI * 0.1F));
    CHECK_EQ(FTREMIXSANDBAG_STATUS_IDLE, ftRemixSandbagGetLandingStatus(-TEST_PI * 1.9F));
    CHECK_EQ(FTREMIXSANDBAG_STATUS_IDLE, ftRemixSandbagGetLandingStatus(-TEST_PI * 2.0F));
}

static void testSandbagBouncesInBetween(void)
{
    CHECK_EQ(FTREMIXSANDBAG_STATUS_NONE, ftRemixSandbagGetLandingStatus(-TEST_PI * 0.5F));
    CHECK_EQ(FTREMIXSANDBAG_STATUS_NONE, ftRemixSandbagGetLandingStatus(-TEST_PI * 1.5F));
}

static void testSandbagGetsUpWhenKnockbackXIsZero(void)
{
    CHECK_EQ(1, ftRemixSandbagGetStandWait(30, 1));
    CHECK_EQ(30, ftRemixSandbagGetStandWait(30, 0));
}

static void testSandbagDamageMulResetsInIdleAndHeadStand(void)
{
    CHECK_TRUE(ftRemixSandbagResetsDamageMul(FTREMIXSANDBAG_STATUS_IDLE));
    CHECK_TRUE(ftRemixSandbagResetsDamageMul(FTREMIXSANDBAG_STATUS_HEADSTAND));
    CHECK_TRUE(!ftRemixSandbagResetsDamageMul(0x4A));
}

static void testWarioBodySlamRecoilsOnHurtboxShieldAndOpenClang(void)
{
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_WARIO, 0xDF, FTREMIXHITRECOIL_HIT_HURTBOX, 1));
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_WARIO, 0xE0, FTREMIXHITRECOIL_HIT_SHIELD, 1));
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_WARIO, 0xDF, FTREMIXHITRECOIL_HIT_CLANG, 0));
}

static void testWarioBodySlamKeepsGoingThroughAClangWhileInvincible(void)
{
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_WARIO, 0xDF, FTREMIXHITRECOIL_HIT_CLANG, 1));
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_WARIO, 0xDF, FTREMIXHITRECOIL_HIT_SHIELD_REHIT, 0));
}

static void testWarioOtherStatusesDoNotRecoil(void)
{
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_WARIO, 0xE1, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
}

static void testKirbyWithTheWarioHatRecoilsLikeWario(void)
{
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_KIRBY, 0x138, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_JKIRBY, 0x139, FTREMIXHITRECOIL_HIT_CLANG, 0));
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_JKIRBY, 0x139, FTREMIXHITRECOIL_HIT_CLANG, 1));
}

static void testSheikAndBanjoAttacksAlwaysRecoil(void)
{
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_SHEIK, 0xF1, FTREMIXHITRECOIL_HIT_CLANG, 1));
    CHECK_EQ(nFTRemixHitRecoilRecoil, ftRemixHitRecoilGetKind(REMIX_ID_BANJO, 0xF0, FTREMIXHITRECOIL_HIT_SHIELD_REHIT, 0));
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_SHEIK, 0xF0, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_BANJO, 0xF1, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
}

static void testEbiAndKirbyEbiMakeFoodOnHurtboxOnly(void)
{
    CHECK_EQ(nFTRemixHitRecoilFood, ftRemixHitRecoilGetKind(REMIX_ID_EBI, 0xEB, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
    CHECK_EQ(nFTRemixHitRecoilFood, ftRemixHitRecoilGetKind(REMIX_ID_EBI, 0xEE, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
    CHECK_EQ(nFTRemixHitRecoilFood, ftRemixHitRecoilGetKind(REMIX_ID_KIRBY, 0x1A7, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
    CHECK_EQ(nFTRemixHitRecoilFood, ftRemixHitRecoilGetKind(REMIX_ID_JKIRBY, 0x1AA, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_EBI, 0xEB, FTREMIXHITRECOIL_HIT_SHIELD, 0));
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_EBI, 0xE6, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
}

static void testOtherFightersNeverRecoil(void)
{
    CHECK_EQ(nFTRemixHitRecoilNone, ftRemixHitRecoilGetKind(REMIX_ID_MARIO, 0xDF, FTREMIXHITRECOIL_HIT_HURTBOX, 0));
}

static void testBodySlamStatusTest(void)
{
    CHECK_TRUE(ftRemixHitRecoilIsBodySlam(REMIX_ID_WARIO, 0xDF));
    CHECK_TRUE(ftRemixHitRecoilIsBodySlam(REMIX_ID_WARIO, 0xE0));
    CHECK_TRUE(ftRemixHitRecoilIsBodySlam(REMIX_ID_KIRBY, 0x138));
    CHECK_TRUE(ftRemixHitRecoilIsBodySlam(REMIX_ID_JKIRBY, 0x139));
    CHECK_TRUE(!ftRemixHitRecoilIsBodySlam(REMIX_ID_WARIO, 0x138));
    CHECK_TRUE(!ftRemixHitRecoilIsBodySlam(REMIX_ID_KIRBY, 0xDF));
    CHECK_TRUE(!ftRemixHitRecoilIsBodySlam(REMIX_ID_MARIO, 0xDF));
}

static void testMagicHatRestoresOutsideTheCopiedSpecials(void)
{
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0x00));
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0xDC));
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0xE6));
}

static void testMagicHatKeepsTheCopiedSpecials(void)
{
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0xE7));
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0xF9));
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0xFC));
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0xFF));
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0x104));
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0x12E));
}

static void testMagicHatRestoresOnStaringAndFinalCutter(void)
{
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0xFA));
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0xFB));
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0x100));
    CHECK_TRUE(ftRemixMagicHatShouldRestore(0x103));
}

static void testMagicHatNeverRestoresInTheRemixSpecials(void)
{
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0x12F));
    CHECK_TRUE(!ftRemixMagicHatShouldRestore(0x1A7));
}

static void testMagicHatChargeRowsAndMaximums(void)
{
    CHECK_EQ(0, ftRemixMagicHatChargeSlot(REMIX_ID_DONKEY));
    CHECK_EQ(1, ftRemixMagicHatChargeSlot(REMIX_ID_SAMUS));
    CHECK_EQ(2, ftRemixMagicHatChargeSlot(REMIX_ID_DSAMUS));
    CHECK_EQ(3, ftRemixMagicHatChargeSlot(REMIX_ID_MTWO));
    CHECK_EQ(4, ftRemixMagicHatChargeSlot(REMIX_ID_SHEIK));
    CHECK_EQ(5, ftRemixMagicHatChargeSlot(REMIX_ID_PEPPY));
    CHECK_EQ(-1, ftRemixMagicHatChargeSlot(REMIX_ID_MARIO));
    CHECK_EQ(10, ftRemixMagicHatChargeMax(0));
    CHECK_EQ(7, ftRemixMagicHatChargeMax(1));
    CHECK_EQ(6, ftRemixMagicHatChargeMax(4));
    CHECK_EQ(5, ftRemixMagicHatChargeMax(5));
    CHECK_EQ(0, ftRemixMagicHatChargeMax(-1));
    CHECK_EQ(0, ftRemixMagicHatChargeMax(FTREMIXMAGICHAT_CHARGE_SLOTS));
}

static void testMagicHatChargeLivesInTheAmmoWord(void)
{
    CHECK_EQ(0x0AE8, ftRemixMagicHatAmmoOffset(REMIX_ID_DONKEY));
    CHECK_EQ(0x0AE0, ftRemixMagicHatAmmoOffset(REMIX_ID_SAMUS));
    CHECK_EQ(0x0AE0, ftRemixMagicHatAmmoOffset(REMIX_ID_PEPPY));
}

static void testMagicHatRollTable(void)
{
    CHECK_EQ(REMIX_ID_MARIO, ftRemixMagicHatFighterFromRoll(1));
    CHECK_EQ(REMIX_ID_BOWSER, ftRemixMagicHatFighterFromRoll(17));
    CHECK_EQ(REMIX_ID_EBI, ftRemixMagicHatFighterFromRoll(38));
    CHECK_EQ(REMIX_ID_SSONIC, ftRemixMagicHatFighterFromRoll(40));
    CHECK_EQ(REMIX_ID_NONE, ftRemixMagicHatFighterFromRoll(FTREMIXMAGICHAT_ROLLS));
    CHECK_EQ(REMIX_ID_KIRBY, ftRemixMagicHatFighterFromRoll(99));
}

static void testCommandSizeFactor(void)
{
    CHECK_EQ(1, ftRemixCommandGetSizeFactor(1.0F) == 1.0F);
    CHECK_EQ(1, ftRemixCommandGetSizeFactor(1.5F) == FTREMIXCOMMAND_SUPER_MULT);
    CHECK_EQ(1, ftRemixCommandGetSizeFactor(0.5F) == FTREMIXCOMMAND_POISON_MULT);
    CHECK_EQ(1, ftRemixCommandGetSizeFactor(0.0F) == FTREMIXCOMMAND_POISON_MULT);
}

void remixTestFtMainRules(void)
{
    printf("ftmain rules\n");
    RUN_TEST(testMapStarFightersAreKirbyJKirbyAndDedede);
    RUN_TEST(testMarinaThrownFiles);
    RUN_TEST(testSandbagLandsOnHeadWhenUpsideDown);
    RUN_TEST(testSandbagLandsOnFeetWhenUpright);
    RUN_TEST(testSandbagBouncesInBetween);
    RUN_TEST(testSandbagGetsUpWhenKnockbackXIsZero);
    RUN_TEST(testSandbagDamageMulResetsInIdleAndHeadStand);
    RUN_TEST(testWarioBodySlamRecoilsOnHurtboxShieldAndOpenClang);
    RUN_TEST(testWarioBodySlamKeepsGoingThroughAClangWhileInvincible);
    RUN_TEST(testWarioOtherStatusesDoNotRecoil);
    RUN_TEST(testKirbyWithTheWarioHatRecoilsLikeWario);
    RUN_TEST(testSheikAndBanjoAttacksAlwaysRecoil);
    RUN_TEST(testEbiAndKirbyEbiMakeFoodOnHurtboxOnly);
    RUN_TEST(testOtherFightersNeverRecoil);
    RUN_TEST(testBodySlamStatusTest);
    RUN_TEST(testMagicHatRestoresOutsideTheCopiedSpecials);
    RUN_TEST(testMagicHatKeepsTheCopiedSpecials);
    RUN_TEST(testMagicHatRestoresOnStaringAndFinalCutter);
    RUN_TEST(testMagicHatNeverRestoresInTheRemixSpecials);
    RUN_TEST(testMagicHatChargeRowsAndMaximums);
    RUN_TEST(testMagicHatChargeLivesInTheAmmoWord);
    RUN_TEST(testMagicHatRollTable);
    RUN_TEST(testCommandSizeFactor);
}
