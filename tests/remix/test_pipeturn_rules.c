/**
 * test_pipeturn_rules.c - unit tests of the Remix pipe_turn table rule (ft/ftremix/ftremixpipeturnrules.c): the fighters that
 * turn like Mario in a warp pipe (marioshared.asm:157-199, DrLuigi.asm:150).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixpipeturnrules.h"
#include "remix/remix_roster_data.h"

static void testTheTwelveTableEntriesTurnLikeMario(void)
{
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_NDRM));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_NWARIO));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_NCONKER));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_JMARIO));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_JLUIGI));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_MLUIGI));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_WARIO));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_GOEMON));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_EBI));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_DRM));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_CONKER));
    CHECK_TRUE(ftRemixPipeTurnRulesHasEntry(REMIX_ID_DRL));
}

static void testOtherFightersHaveNoEntry(void)
{
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_FOX));
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_NMARIO));
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_NLUIGI));
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_NGOEMON));
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_NCRASH));
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(-1));
}

static void testMarioAndVanillaLuigiAreHandledByTheVanillaCheck(void)
{
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_MARIO));
    CHECK_TRUE(!ftRemixPipeTurnRulesHasEntry(REMIX_ID_LUIGI));
}

void remixTestPipeTurnRules(void)
{
    printf("pipe turn rules\n");
    RUN_TEST(testTheTwelveTableEntriesTurnLikeMario);
    RUN_TEST(testOtherFightersHaveNoEntry);
    RUN_TEST(testMarioAndVanillaLuigiAreHandledByTheVanillaCheck);
}
