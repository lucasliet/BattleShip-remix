/**
 * test_grounded_rules.c - unit tests of the Remix grounded_script table rule (ft/ftremix/ftremixgroundedrules.c):
 * which landing reset each Remix fighter runs (Character.asm:2164 and the per-fighter table patches).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixgroundedrules.h"
#include "remix/remix_roster_data.h"

static void testMarioLuigiCaptainFamiliesClearTheWord(void)
{
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_DRM));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_JMARIO));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_NCRASH));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_DRL));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_MLUIGI));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_GND));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_SHEIK));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_NMARTH));
}

static void testPuffAndSonicFamiliesClearTheWord(void)
{
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_JPUFF));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_EPUFF));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_SONIC));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_SSONIC));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_NSONIC));
}

static void testRoyAndMarthClearTheWord(void)
{
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_ROY));
    CHECK_EQ(nFTRemixGroundedScriptClearWord, ftRemixGroundedScriptKind(REMIX_ID_MARTH));
}

static void testSamusFamilyClearsTheChargeRecoil(void)
{
    CHECK_EQ(nFTRemixGroundedScriptClearRecoil, ftRemixGroundedScriptKind(REMIX_ID_DSAMUS));
    CHECK_EQ(nFTRemixGroundedScriptClearRecoil, ftRemixGroundedScriptKind(REMIX_ID_JSAMUS));
    CHECK_EQ(nFTRemixGroundedScriptClearRecoil, ftRemixGroundedScriptKind(REMIX_ID_ESAMUS));
    CHECK_EQ(nFTRemixGroundedScriptClearRecoil, ftRemixGroundedScriptKind(REMIX_ID_NDSAMUS));
}

static void testOwnRoutines(void)
{
    CHECK_EQ(nFTRemixGroundedScriptEbi, ftRemixGroundedScriptKind(REMIX_ID_EBI));
    CHECK_EQ(nFTRemixGroundedScriptKirbyHat, ftRemixGroundedScriptKind(REMIX_ID_KIRBY));
    CHECK_EQ(nFTRemixGroundedScriptKirbyHat, ftRemixGroundedScriptKind(REMIX_ID_JKIRBY));
    CHECK_EQ(nFTRemixGroundedScriptPeachFloat, ftRemixGroundedScriptKind(REMIX_ID_PEACH));
    CHECK_EQ(nFTRemixGroundedScriptPeachFloat, ftRemixGroundedScriptKind(REMIX_ID_NPEACH));
}

static void testDisabledEntriesDoNothing(void)
{
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_WARIO));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_PIANO));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_MARINA));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_DEDEDE));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_NGOEMON));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_BANJO));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_MTWO));
}

static void testVanillaIdsAreLeftToTheVanillaSwitch(void)
{
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_MARIO));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_SAMUS));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_CAPTAIN));
    CHECK_EQ(nFTRemixGroundedScriptNone, ftRemixGroundedScriptKind(REMIX_ID_JIGGLY));
}

void remixTestGroundedRules(void)
{
    printf("grounded rules\n");
    RUN_TEST(testMarioLuigiCaptainFamiliesClearTheWord);
    RUN_TEST(testPuffAndSonicFamiliesClearTheWord);
    RUN_TEST(testRoyAndMarthClearTheWord);
    RUN_TEST(testSamusFamilyClearsTheChargeRecoil);
    RUN_TEST(testOwnRoutines);
    RUN_TEST(testDisabledEntriesDoNothing);
    RUN_TEST(testVanillaIdsAreLeftToTheVanillaSwitch);
}
