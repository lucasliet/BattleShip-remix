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

static int verdictOf(int routine_kind, int input_kind, int cpu_level, int is_airborne)
{
    FTRemixAiPreventContext context = { input_kind, cpu_level, is_airborne, 0, 0 };

    return ftRemixAiPreventRulesVerdict(routine_kind, &context);
}

static void testVanillaAddressesMapToTheirRoutineKinds(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_NONE, ftRemixAiPreventRulesVanillaKind(0x80133520u));
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_MARIO, ftRemixAiPreventRulesVanillaKind(0x801334ECu));
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_KIRBY, ftRemixAiPreventRulesVanillaKind(0x80133500u));
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_YOSHI_FALCON, ftRemixAiPreventRulesVanillaKind(0x80133510u));
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_SKIP_ATTACK, ftRemixAiPreventRulesVanillaKind(0x80133A14u));
}

static void testRemixCodeAddressIsUnknownToTheVanillaMap(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN, ftRemixAiPreventRulesVanillaKind(0x8051A4D4u));
    CHECK_EQ(FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN, ftRemixAiPreventRulesVanillaKind(0u));
}

static void testMarioRoutineChecksTheCliffOnlyForUsp(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_MARIO, FTREMIX_AI_PREVENT_INPUT_USPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_MARIO, FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0));
}

static void testKirbyRoutineChecksTheCliffForUspAndDsp(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_KIRBY, FTREMIX_AI_PREVENT_INPUT_USPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_KIRBY, FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_KIRBY, FTREMIX_AI_PREVENT_INPUT_NSPG, 5, 0));
}

static void testYoshiFalconRoutineChecksTheCliffForDspOnly(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_YOSHI_FALCON, FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_YOSHI_FALCON, FTREMIX_AI_PREVENT_INPUT_USPG, 5, 0));
}

static void testNoneAndUnknownRoutinesAllowEverything(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW, verdictOf(FTREMIX_AI_PREVENT_ROUTINE_NONE, FTREMIX_AI_PREVENT_INPUT_DSPG, 10, 1));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_UNKNOWN, FTREMIX_AI_PREVENT_INPUT_USPG, 10, 1));
}

static void testSkipAttackRoutineDropsEveryCandidate(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_SKIP, verdictOf(FTREMIX_AI_PREVENT_ROUTINE_SKIP_ATTACK, FTREMIX_AI_PREVENT_INPUT_NSPG, 1, 0));
}

static void testFoxUspRoutineDropsTheUspAtLevelTen(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_SKIP, verdictOf(FTREMIX_AI_PREVENT_ROUTINE_FOX_USP, FTREMIX_AI_PREVENT_INPUT_USPG, 10, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW, verdictOf(FTREMIX_AI_PREVENT_ROUTINE_FOX_USP, FTREMIX_AI_PREVENT_INPUT_USPG, 9, 0));
}

static void testFalcoRoutineChecksTheCliffForAirborneNspAndDropsLevelTenUsp(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP, FTREMIX_AI_PREVENT_INPUT_NSPG, 5, 1));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP, FTREMIX_AI_PREVENT_INPUT_NSPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_SKIP,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP, FTREMIX_AI_PREVENT_INPUT_USPG, 10, 1));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_FALCO_NSP, FTREMIX_AI_PREVENT_INPUT_USPG, 9, 1));
}

static void testLedgeRoutinesCheckTheCliffForTheirInputs(void)
{
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_BOWSER_USP_DSP, FTREMIX_AI_PREVENT_INPUT_USPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_GBOWSER, FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_GBOWSER, FTREMIX_AI_PREVENT_INPUT_USPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_USP, FTREMIX_AI_PREVENT_INPUT_USPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_SONIC_DSP, FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_WARIO, FTREMIX_AI_PREVENT_INPUT_WARIO_NSP_TOWARDS, 5, 0));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK,
             verdictOf(FTREMIX_AI_PREVENT_ROUTINE_MARINA_NSP, FTREMIX_AI_PREVENT_INPUT_NSPG, 5, 0));
}

static void testConkerRoutineDropsTheGrenadeWhileOneIsOut(void)
{
    FTRemixAiPreventContext out = { FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0, 0, 1 };
    FTRemixAiPreventContext ready = { FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0, 0, 0 };

    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_SKIP, ftRemixAiPreventRulesVerdict(FTREMIX_AI_PREVENT_ROUTINE_CONKER_GRENADE, &out));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW, ftRemixAiPreventRulesVerdict(FTREMIX_AI_PREVENT_ROUTINE_CONKER_GRENADE, &ready));
}

static void testCrashRoutineChecksTheCliffOnASoftPlatform(void)
{
    FTRemixAiPreventContext soft = { FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0, 1, 0 };
    FTRemixAiPreventContext solid = { FTREMIX_AI_PREVENT_INPUT_DSPG, 5, 0, 0, 0 };

    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_CLIFF_CHECK, ftRemixAiPreventRulesVerdict(FTREMIX_AI_PREVENT_ROUTINE_CRASH, &soft));
    CHECK_EQ(FTREMIX_AI_PREVENT_VERDICT_ALLOW, ftRemixAiPreventRulesVerdict(FTREMIX_AI_PREVENT_ROUTINE_CRASH, &solid));
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
    RUN_TEST(testVanillaAddressesMapToTheirRoutineKinds);
    RUN_TEST(testRemixCodeAddressIsUnknownToTheVanillaMap);
    RUN_TEST(testMarioRoutineChecksTheCliffOnlyForUsp);
    RUN_TEST(testKirbyRoutineChecksTheCliffForUspAndDsp);
    RUN_TEST(testYoshiFalconRoutineChecksTheCliffForDspOnly);
    RUN_TEST(testNoneAndUnknownRoutinesAllowEverything);
    RUN_TEST(testSkipAttackRoutineDropsEveryCandidate);
    RUN_TEST(testFoxUspRoutineDropsTheUspAtLevelTen);
    RUN_TEST(testFalcoRoutineChecksTheCliffForAirborneNspAndDropsLevelTenUsp);
    RUN_TEST(testLedgeRoutinesCheckTheCliffForTheirInputs);
    RUN_TEST(testConkerRoutineDropsTheGrenadeWhileOneIsOut);
    RUN_TEST(testCrashRoutineChecksTheCliffOnASoftPlatform);
}
