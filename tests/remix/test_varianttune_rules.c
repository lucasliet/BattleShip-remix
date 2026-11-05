/**
 * test_varianttune_rules.c - unit tests of the small per-fighter tuning rules (ft/ftremix/ftremixvarianttunerules.c): JDK Spinning
 * Kong, J Falcon Falcon Dive drift, Peppy Firefox pitch and the entry facing of Sonic.asm sonic_facing_.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixvarianttunerules.h"
#include "remix/remix_roster_data.h"

static void testJdkSpinningKongStartsAtEighteen(void)
{
    CHECK_EQ(180, (int)(ftRemixVariantTuneRulesSpinningKongAirVelY(REMIX_ID_JDK, 20.3F) * 10.0F));
    CHECK_EQ(203, (int)(ftRemixVariantTuneRulesSpinningKongAirVelY(REMIX_ID_DONKEY, 20.3F) * 10.0F));
}

static void testJFalconFalconDiveDriftUsesTheJapaneseMultipliers(void)
{
    CHECK_EQ(120, (int)(ftRemixVariantTuneRulesFalconDiveAirAccelMul(REMIX_ID_JFALCON, 1.1F) * 100.0F));
    CHECK_EQ(84, (int)(ftRemixVariantTuneRulesFalconDiveAirSpeedMaxMul(REMIX_ID_JFALCON, 0.8F) * 100.0F));
}

static void testOtherFalconsKeepTheVanillaFalconDiveDrift(void)
{
    CHECK_EQ(110, (int)(ftRemixVariantTuneRulesFalconDiveAirAccelMul(REMIX_ID_CAPTAIN, 1.1F) * 100.0F));
    CHECK_EQ(80, (int)(ftRemixVariantTuneRulesFalconDiveAirSpeedMaxMul(REMIX_ID_GND, 0.8F) * 100.0F));
}

static void testPeppyFirefoxHasNoPitchOffset(void)
{
    CHECK_EQ(0, (int)ftRemixVariantTuneRulesFireFoxPitchOffset(REMIX_ID_PEPPY, 1.5707964F));
    CHECK_EQ(15707, (int)(ftRemixVariantTuneRulesFireFoxPitchOffset(REMIX_ID_FOX, 1.5707964F) * 10000.0F));
}

static void testVanillaFightersClearTheEntryFacing(void)
{
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_MARIO));
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_NESS));
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_BOSS));
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_METAL));
}

static void testPolygonFightersKeepTheEntryFacing(void)
{
    CHECK_TRUE(ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_NMARIO));
    CHECK_TRUE(ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_NNESS));
    CHECK_TRUE(ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_NWARIO));
    CHECK_TRUE(ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_NCRASH));
}

static void testSonicAndSuperSonicKeepTheEntryFacing(void)
{
    CHECK_TRUE(ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_SONIC));
    CHECK_TRUE(ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_SSONIC));
}

static void testOtherRemixFightersClearTheEntryFacing(void)
{
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_GDONKEY));
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_FALCO));
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_SANDBAG));
    CHECK_TRUE(!ftRemixVariantTuneRulesEntryKeepsFacing(REMIX_ID_LANKY));
}

void remixTestVariantTuneRules(void)
{
    printf("variant tune rules\n");
    RUN_TEST(testJdkSpinningKongStartsAtEighteen);
    RUN_TEST(testJFalconFalconDiveDriftUsesTheJapaneseMultipliers);
    RUN_TEST(testOtherFalconsKeepTheVanillaFalconDiveDrift);
    RUN_TEST(testPeppyFirefoxHasNoPitchOffset);
    RUN_TEST(testVanillaFightersClearTheEntryFacing);
    RUN_TEST(testPolygonFightersKeepTheEntryFacing);
    RUN_TEST(testSonicAndSuperSonicKeepTheEntryFacing);
    RUN_TEST(testOtherRemixFightersClearTheEntryFacing);
}
