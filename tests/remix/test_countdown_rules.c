/**
 * test_countdown_rules.c - unit tests of the stoplight countdown sound rules (if/ifremixcountdownrules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "if/ifremixcountdownrules.h"

static void testBigBlueReplacesEachVoiceWithTheFZeroVoice(void)
{
    IFRemixCountdownSounds sounds = ifRemixCountdownRulesGetSounds(nGRRemixStage_BLUE, nIFRemixCountdownStepThree);

    CHECK_EQ(0x5CB, sounds.extra_fgm);
    CHECK_EQ(1, sounds.is_voice_replaced);
    CHECK_EQ(0x5CC, ifRemixCountdownRulesGetSounds(nGRRemixStage_BLUE, nIFRemixCountdownStepTwo).extra_fgm);
    CHECK_EQ(0x5CD, ifRemixCountdownRulesGetSounds(nGRRemixStage_BLUE, nIFRemixCountdownStepOne).extra_fgm);
    CHECK_EQ(0x5CE, ifRemixCountdownRulesGetSounds(nGRRemixStage_BLUE, nIFRemixCountdownStepGo).extra_fgm);
}

static void testMarioKartStagesAddTheNoiseAfterTheVoiceOfTheNumbers(void)
{
    IFRemixCountdownSounds sounds = ifRemixCountdownRulesGetSounds(nGRRemixStage_TOADSTURNPIKE, nIFRemixCountdownStepTwo);

    CHECK_EQ(0x405, sounds.extra_fgm);
    CHECK_EQ(0, sounds.is_voice_replaced);
    CHECK_EQ(0, sounds.is_extra_first);
    CHECK_EQ(0x405, ifRemixCountdownRulesGetSounds(nGRRemixStage_RAINBOWROAD, nIFRemixCountdownStepThree).extra_fgm);
    CHECK_EQ(0x405, ifRemixCountdownRulesGetSounds(nGRRemixStage_RAINBOWROAD, nIFRemixCountdownStepOne).extra_fgm);
}

static void testMarioKartStagesPlayTheGoNoiseBeforeTheGoVoice(void)
{
    IFRemixCountdownSounds sounds = ifRemixCountdownRulesGetSounds(nGRRemixStage_RAINBOWROAD, nIFRemixCountdownStepGo);

    CHECK_EQ(0x406, sounds.extra_fgm);
    CHECK_EQ(0, sounds.is_voice_replaced);
    CHECK_EQ(1, sounds.is_extra_first);
}

static void testOtherStagesKeepTheVanillaVoice(void)
{
    CHECK_EQ(IFREMIX_COUNTDOWN_NO_FGM, ifRemixCountdownRulesGetSounds(nGRRemixStage_PEACHS_CASTLE, nIFRemixCountdownStepGo).extra_fgm);
    CHECK_EQ(IFREMIX_COUNTDOWN_NO_FGM, ifRemixCountdownRulesGetSounds(nGRRemixStage_DRAGONKING, nIFRemixCountdownStepThree).extra_fgm);
}

void remixTestCountdownRules(void)
{
    printf("countdown rules\n");
    RUN_TEST(testBigBlueReplacesEachVoiceWithTheFZeroVoice);
    RUN_TEST(testMarioKartStagesAddTheNoiseAfterTheVoiceOfTheNumbers);
    RUN_TEST(testMarioKartStagesPlayTheGoNoiseBeforeTheGoVoice);
    RUN_TEST(testOtherStagesKeepTheVanillaVoice);
}
