/**
 * test_results_rules.c - unit tests of the Remix VS results winner rules (mn/mnvsmode/mnremixresultsrules.c). Expected values come
 * from src/resultsscreen.asm: victory_animations_ (C buttons), get_str_wins_lx_ ("WIN!" for J sound and Banjo), the float32 table
 * words of add_to_results_screen and the add_ampersand_ constants.
 */

#include "test_check.h"

#include <string.h>

#include "mn/mnvsmode/mnremixresultsrules.h"
#include "remix/remix_roster_data.h"

#define BUTTON_A 0x8000
#define BUTTON_C_UP 0x0008
#define BUTTON_C_DOWN 0x0004

static const char *sFakeImageText;

static unsigned int readFakeImageByte(unsigned int ram_addr)
{
    return (unsigned char)sFakeImageText[ram_addr - 0x80400000U];
}

static void testOnlyTheFkindsAboveTheTwelveFightersUseTheRemixTables(void)
{
    CHECK_EQ(0, mnRemixResultsRulesIsExtendedFkind(0));
    CHECK_EQ(0, mnRemixResultsRulesIsExtendedFkind(11));
    CHECK_TRUE(mnRemixResultsRulesIsExtendedFkind(12));
    CHECK_TRUE(mnRemixResultsRulesIsExtendedFkind(REMIX_ID_METAL));
    CHECK_TRUE(mnRemixResultsRulesIsExtendedFkind(REMIX_ID_GDONKEY));
    CHECK_TRUE(mnRemixResultsRulesIsExtendedFkind(REMIX_ID_BANJO));
}

static void testNoCButtonKeepsTheRandomVictoryAnimation(void)
{
    CHECK_EQ(0, mnRemixResultsRulesPickVictoryIndex(0, 0));
    CHECK_EQ(1, mnRemixResultsRulesPickVictoryIndex(1, 0));
    CHECK_EQ(2, mnRemixResultsRulesPickVictoryIndex(2, 0));
    CHECK_EQ(1, mnRemixResultsRulesPickVictoryIndex(1, BUTTON_A));
}

static void testCRightPicksAnimationThree(void)
{
    CHECK_EQ(2, mnRemixResultsRulesPickVictoryIndex(0, MNREMIX_RESULTS_C_RIGHT));
    CHECK_EQ(2, mnRemixResultsRulesPickVictoryIndex(1, MNREMIX_RESULTS_C_RIGHT | BUTTON_A));
}

static void testCLeftPicksAnimationTwo(void)
{
    CHECK_EQ(1, mnRemixResultsRulesPickVictoryIndex(0, MNREMIX_RESULTS_C_LEFT));
    CHECK_EQ(1, mnRemixResultsRulesPickVictoryIndex(2, MNREMIX_RESULTS_C_LEFT | BUTTON_C_UP));
}

static void testCUpOrCDownPicksAnimationOne(void)
{
    CHECK_EQ(0, mnRemixResultsRulesPickVictoryIndex(2, BUTTON_C_UP));
    CHECK_EQ(0, mnRemixResultsRulesPickVictoryIndex(1, BUTTON_C_DOWN));
}

static void testCRightWinsOverCLeft(void)
{
    CHECK_EQ(2, mnRemixResultsRulesPickVictoryIndex(0, MNREMIX_RESULTS_C_RIGHT | MNREMIX_RESULTS_C_LEFT));
}

static void testJapaneseSoundFightersSaySingularWin(void)
{
    CHECK_TRUE(mnRemixResultsRulesUsesSingularWin(REMIX_ID_JMARIO, 1));
    CHECK_EQ(0, mnRemixResultsRulesUsesSingularWin(REMIX_ID_FALCO, 0));
    CHECK_EQ(0, mnRemixResultsRulesUsesSingularWin(REMIX_ID_EPIKA, 0));
}

static void testBanjoSaysSingularWinWithTheUnitedStatesSound(void)
{
    CHECK_TRUE(mnRemixResultsRulesUsesSingularWin(REMIX_ID_BANJO, 0));
}

static void testDragonKingKeepsPluralWinWhateverItsSoundType(void)
{
    CHECK_EQ(0, mnRemixResultsRulesUsesSingularWin(REMIX_ID_DRAGONKING, 1));
    CHECK_EQ(0, mnRemixResultsRulesUsesSingularWin(REMIX_ID_DRAGONKING, 0));
}

static void testTableWordsDecodeToTheAsmFloats(void)
{
    CHECK_TRUE(mnRemixResultsRulesBitsToFloat(0x43390000U) == 185.0F);
    CHECK_TRUE(mnRemixResultsRulesBitsToFloat(0x3F800000U) == 1.0F);
    CHECK_TRUE(mnRemixResultsRulesBitsToFloat(0x3F000000U) == 0.5F);
    CHECK_TRUE(mnRemixResultsRulesBitsToFloat(0x41F00000U) == 30.0F);
}

static void testAmpersandGlyphConstantsFollowAddAmpersand(void)
{
    CHECK_EQ(0x1D, MNREMIX_RESULTS_AMPERSAND_INDEX);
    CHECK_EQ(0x8348 + 0x10, MNREMIX_RESULTS_AMPERSAND_SPRITE_OFFSET);
    CHECK_TRUE(mnRemixResultsRulesBitsToFloat(0x42080000U) == MNREMIX_RESULTS_AMPERSAND_WIDTH);
}

static void testNameIsCopiedUpToTheTerminator(void)
{
    char name[MNREMIX_RESULTS_NAME_BUFFER_SIZE];

    sFakeImageText = "BAN1JO2&2KAZOOI1E";

    CHECK_EQ(17, mnRemixResultsRulesCopyName(readFakeImageByte, 0x80400000U, name, MNREMIX_RESULTS_NAME_BUFFER_SIZE));
    CHECK_EQ(0, strcmp(name, "BAN1JO2&2KAZOOI1E"));
}

static void testLongNameIsCutAndTerminated(void)
{
    char name[8];

    sFakeImageText = "POLY D. SAMUS";

    CHECK_EQ(7, mnRemixResultsRulesCopyName(readFakeImageByte, 0x80400000U, name, sizeof(name)));
    CHECK_EQ(0, strcmp(name, "POLY D."));
}

static void testEmptyNameCopiesNothing(void)
{
    char name[MNREMIX_RESULTS_NAME_BUFFER_SIZE] = "x";

    sFakeImageText = "";

    CHECK_EQ(0, mnRemixResultsRulesCopyName(readFakeImageByte, 0x80400000U, name, MNREMIX_RESULTS_NAME_BUFFER_SIZE));
    CHECK_EQ(0, name[0]);
}

static void testUnusableArgumentsCopyNothing(void)
{
    char name[4];

    CHECK_EQ(0, mnRemixResultsRulesCopyName(NULL, 0x80400000U, name, sizeof(name)));
    CHECK_EQ(0, mnRemixResultsRulesCopyName(readFakeImageByte, 0x80400000U, NULL, sizeof(name)));
    CHECK_EQ(0, mnRemixResultsRulesCopyName(readFakeImageByte, 0x80400000U, name, 0));
}

void remixTestResultsRules(void)
{
    printf("results rules\n");
    RUN_TEST(testOnlyTheFkindsAboveTheTwelveFightersUseTheRemixTables);
    RUN_TEST(testNoCButtonKeepsTheRandomVictoryAnimation);
    RUN_TEST(testCRightPicksAnimationThree);
    RUN_TEST(testCLeftPicksAnimationTwo);
    RUN_TEST(testCUpOrCDownPicksAnimationOne);
    RUN_TEST(testCRightWinsOverCLeft);
    RUN_TEST(testJapaneseSoundFightersSaySingularWin);
    RUN_TEST(testBanjoSaysSingularWinWithTheUnitedStatesSound);
    RUN_TEST(testDragonKingKeepsPluralWinWhateverItsSoundType);
    RUN_TEST(testTableWordsDecodeToTheAsmFloats);
    RUN_TEST(testAmpersandGlyphConstantsFollowAddAmpersand);
    RUN_TEST(testNameIsCopiedUpToTheTerminator);
    RUN_TEST(testLongNameIsCutAndTerminated);
    RUN_TEST(testEmptyNameCopiesNothing);
    RUN_TEST(testUnusableArgumentsCopyNothing);
}
