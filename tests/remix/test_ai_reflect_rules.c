/**
 * test_ai_reflect_rules.c - unit tests of the Remix CPU reflect and absorb rules (ft/ftremix/ftremixaireflectrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaireflectrules.h"
#include "remix/remix_roster_data.h"

static void testMarinaHasTheLongerDetectionWindow(void)
{
    CHECK_EQ(25, (int)ftRemixAiReflectRulesDetectionFrames(REMIX_ID_MARINA));
    CHECK_EQ(15, (int)ftRemixAiReflectRulesDetectionFrames(REMIX_ID_FALCO));
    CHECK_EQ(15, (int)ftRemixAiReflectRulesDetectionFrames(REMIX_ID_MARIO));
}

static void testFoxLikeFightersAlwaysRaiseTheFlagForProjectiles(void)
{
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_FOX, 0x23, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_NFOX, 0x24, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_NESS, 0x23, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_JFOX, 0x23, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_FALCO, 0x01, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_SLIPPY, 0x01, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_WOLF, 0x01, 0));
}

static void testAbsorbersIgnoreSheikNeedlesAndConkerNuts(void)
{
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_JNESS, 0x23, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_LUCAS, 0x24, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_NLUCAS, 0x23, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_PIANO, 0x24, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_NNESS, 0x23, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_JNESS, 0x01, 0));
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_PIANO, 0x02, 0));
}

static void testMarinaAbsorbsOnlyWhenFreeAndNotAtMaxCharge(void)
{
    CHECK_TRUE(ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_MARINA, 0x01, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_MARINA, 0x01, 1));
    CHECK_TRUE(ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_MARINA, 1, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_MARINA, 1, 1));
}

static void testNonReflectorsNeverRaiseTheFlagForProjectiles(void)
{
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_MARIO, 0x01, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesProjectileRaisesFlag(REMIX_ID_DEDEDE, 0x01, 0));
}

static void testItemFlagFollowsTheReflectTable(void)
{
    CHECK_TRUE(ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_DEDEDE, 1, 0));
    CHECK_TRUE(ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_WOLF, 1, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_MARIO, 0, 0));
}

static void testVanillaFoxNFoxAndNessBypassTheItemTable(void)
{
    CHECK_TRUE(ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_FOX, 0, 0));
    CHECK_TRUE(ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_NFOX, 0, 0));
    CHECK_TRUE(ftRemixAiReflectRulesItemRaisesFlag(REMIX_ID_NESS, 0, 0));
}

static void testMaintainKindMapsTheRemixReflectors(void)
{
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_FOX, ftRemixAiReflectRulesMaintainKind(REMIX_ID_WOLF));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_FOX, ftRemixAiReflectRulesMaintainKind(REMIX_ID_JFOX));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_NESS, ftRemixAiReflectRulesMaintainKind(REMIX_ID_LUCAS));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_NESS, ftRemixAiReflectRulesMaintainKind(REMIX_ID_NLUCAS));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_PIANO, ftRemixAiReflectRulesMaintainKind(REMIX_ID_PIANO));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_MARINA, ftRemixAiReflectRulesMaintainKind(REMIX_ID_MARINA));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_NONE, ftRemixAiReflectRulesMaintainKind(REMIX_ID_DEDEDE));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_NONE, ftRemixAiReflectRulesMaintainKind(REMIX_ID_FOX));
}

static void testApplyKindSkipsNLucasLikeTheAsm(void)
{
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_NESS, ftRemixAiReflectRulesApplyKind(REMIX_ID_LUCAS));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_NONE, ftRemixAiReflectRulesApplyKind(REMIX_ID_NLUCAS));
    CHECK_EQ(FTREMIX_AI_REFLECT_KIND_FOX, ftRemixAiReflectRulesApplyKind(REMIX_ID_FALCO));
}

static void testAbsorbActionRanges(void)
{
    CHECK_TRUE(ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_PIANO, 0x0E5));
    CHECK_TRUE(ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_PIANO, 0x0ED));
    CHECK_EQ(0, ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_PIANO, 0x0EE));
    CHECK_EQ(0, ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_PIANO, 0x0E4));
    CHECK_TRUE(ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_MARINA, 0x0FE));
    CHECK_TRUE(ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_MARINA, 0x109));
    CHECK_EQ(0, ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_MARINA, 0x10A));
    CHECK_EQ(0, ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_MARINA, 0x0FD));
    CHECK_EQ(0, ftRemixAiReflectRulesIsAbsorbAction(FTREMIX_AI_REFLECT_KIND_FOX, 0x0FE));
}

static void testNspCandidateYieldsOnlyForChaseAgainstReflectors(void)
{
    CHECK_TRUE(ftRemixAiReflectRulesSkipsNspCandidate(1, 1));
    CHECK_EQ(0, ftRemixAiReflectRulesSkipsNspCandidate(1, 0));
    CHECK_EQ(0, ftRemixAiReflectRulesSkipsNspCandidate(0, 1));
    CHECK_EQ(0, ftRemixAiReflectRulesSkipsNspCandidate(0, 0));
}

void remixTestAiReflectRules(void)
{
    printf("ai reflect rules\n");
    RUN_TEST(testMarinaHasTheLongerDetectionWindow);
    RUN_TEST(testFoxLikeFightersAlwaysRaiseTheFlagForProjectiles);
    RUN_TEST(testAbsorbersIgnoreSheikNeedlesAndConkerNuts);
    RUN_TEST(testMarinaAbsorbsOnlyWhenFreeAndNotAtMaxCharge);
    RUN_TEST(testNonReflectorsNeverRaiseTheFlagForProjectiles);
    RUN_TEST(testItemFlagFollowsTheReflectTable);
    RUN_TEST(testVanillaFoxNFoxAndNessBypassTheItemTable);
    RUN_TEST(testMaintainKindMapsTheRemixReflectors);
    RUN_TEST(testApplyKindSkipsNLucasLikeTheAsm);
    RUN_TEST(testAbsorbActionRanges);
    RUN_TEST(testNspCandidateYieldsOnlyForChaseAgainstReflectors);
}
