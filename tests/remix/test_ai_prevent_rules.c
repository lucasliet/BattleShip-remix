/**
 * test_ai_prevent_rules.c - unit tests of the Remix PREVENT_ATTACK veto rules (ft/ftremix/ftremixaipreventrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaipreventrules.h"

static void testBowserVetoesDspAndUspOnly(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesIsDspOrUsp(FTREMIX_AI_PREVENT_INPUT_DSPG));
    CHECK_TRUE(ftRemixAiPreventRulesIsDspOrUsp(FTREMIX_AI_PREVENT_INPUT_USPG));
    CHECK_EQ(0, ftRemixAiPreventRulesIsDspOrUsp(FTREMIX_AI_PREVENT_INPUT_NSPG));
}

static void testSingleInputRuleMatchesOnlyTheSameInput(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesIsInput(FTREMIX_AI_PREVENT_INPUT_USPG, FTREMIX_AI_PREVENT_INPUT_USPG));
    CHECK_EQ(0, ftRemixAiPreventRulesIsInput(FTREMIX_AI_PREVENT_INPUT_DSPG, FTREMIX_AI_PREVENT_INPUT_USPG));
}

static void testWarioVetoesDspAndBothNspTowardsInputs(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesIsWarioRisky(FTREMIX_AI_PREVENT_INPUT_DSPG));
    CHECK_TRUE(ftRemixAiPreventRulesIsWarioRisky(FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS));
    CHECK_TRUE(ftRemixAiPreventRulesIsWarioRisky(FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS_JUMP));
    CHECK_EQ(0, ftRemixAiPreventRulesIsWarioRisky(FTREMIX_AI_PREVENT_INPUT_NSPG));
}

static void testFalcoNspIsVetoedOnlyWhileAirborne(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesFalcoNspVeto(FTREMIX_AI_PREVENT_INPUT_NSPG, 1));
    CHECK_EQ(0, ftRemixAiPreventRulesFalcoNspVeto(FTREMIX_AI_PREVENT_INPUT_NSPG, 0));
}

static void testFalcoNspRuleIgnoresOtherInputs(void)
{
    CHECK_EQ(0, ftRemixAiPreventRulesFalcoNspVeto(FTREMIX_AI_PREVENT_INPUT_USPG, 1));
}

static void testFoxUspIsVetoedAtLevelTenOnly(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesFoxUspVeto(FTREMIX_AI_PREVENT_INPUT_USPG, 10));
    CHECK_EQ(0, ftRemixAiPreventRulesFoxUspVeto(FTREMIX_AI_PREVENT_INPUT_USPG, 9));
    CHECK_EQ(0, ftRemixAiPreventRulesFoxUspVeto(FTREMIX_AI_PREVENT_INPUT_DSPG, 10));
}

static void testConkerGrenadeVetoesOnlyWhileAGrenadeIsOut(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesConkerGrenadeVeto(FTREMIX_AI_PREVENT_INPUT_DSPG, 1));
    CHECK_TRUE(ftRemixAiPreventRulesConkerGrenadeVeto(FTREMIX_AI_PREVENT_INPUT_SMASH_DSP, 1));
    CHECK_EQ(0, ftRemixAiPreventRulesConkerGrenadeVeto(FTREMIX_AI_PREVENT_INPUT_DSPG, 0));
}

static void testConkerGrenadeRuleIgnoresOtherInputs(void)
{
    CHECK_EQ(0, ftRemixAiPreventRulesConkerGrenadeVeto(FTREMIX_AI_PREVENT_INPUT_NSPG, 1));
}

static void testCrashVetoesDspAndUspOnlyOnASoftPlatform(void)
{
    CHECK_TRUE(ftRemixAiPreventRulesCrashSoftPlatformVeto(FTREMIX_AI_PREVENT_INPUT_DSPG, 1));
    CHECK_TRUE(ftRemixAiPreventRulesCrashSoftPlatformVeto(FTREMIX_AI_PREVENT_INPUT_USPG, 1));
    CHECK_EQ(0, ftRemixAiPreventRulesCrashSoftPlatformVeto(FTREMIX_AI_PREVENT_INPUT_DSPG, 0));
    CHECK_EQ(0, ftRemixAiPreventRulesCrashSoftPlatformVeto(FTREMIX_AI_PREVENT_INPUT_NSPG, 1));
}

void remixTestAiPreventRules(void)
{
    printf("ai prevent rules\n");
    RUN_TEST(testBowserVetoesDspAndUspOnly);
    RUN_TEST(testSingleInputRuleMatchesOnlyTheSameInput);
    RUN_TEST(testWarioVetoesDspAndBothNspTowardsInputs);
    RUN_TEST(testFalcoNspIsVetoedOnlyWhileAirborne);
    RUN_TEST(testFalcoNspRuleIgnoresOtherInputs);
    RUN_TEST(testFoxUspIsVetoedAtLevelTenOnly);
    RUN_TEST(testConkerGrenadeVetoesOnlyWhileAGrenadeIsOut);
    RUN_TEST(testConkerGrenadeRuleIgnoresOtherInputs);
    RUN_TEST(testCrashVetoesDspAndUspOnlyOnASoftPlatform);
}
