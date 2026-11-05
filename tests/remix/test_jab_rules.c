/**
 * test_jab_rules.c - unit tests of the Remix jab 3 and rapid jab rules (ft/ftremix/ftremixjabrules.c): the table words of
 * Character.asm (jab_3, jab_3_timer, jab_3_action, rapid_jab, rapid_jab_begin/loop/ending_action, rapid_jab_unknown) and the
 * Mewtwo and Slippy rapid jab patches. The words are the ones the RAM image of Remix 2.0.1 holds.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixjabrules.h"
#include "remix/remix_roster_data.h"

#define WORD_JAB_3_ENABLED 0x8014F054u
#define WORD_JAB_3_DISABLED 0x8014F0B4u
#define WORD_RAPID_ENABLED 0x8014F55Cu
#define WORD_RAPID_DISABLED 0x8014F65Cu

#define WORD_TIMER_MARIO 0x8014EB54u
#define WORD_TIMER_DISABLED 0x8014EBA4u
#define WORD_TIMER_LANKY 0x8058C0D0u

#define WORD_ACTION_MARIO 0x8014EBF8u
#define WORD_ACTION_DISABLED 0x8014EC30u
#define WORD_ACTION_BANJO 0x8057AB14u

#define WORD_BEGIN_DISABLED 0x8014F174u
#define WORD_MEWTWO_BEGIN 0x8055C5FCu

static const unsigned int sBeginRows[FTREMIX_RAPID_JAB_STYLE_COUNT] =
{
    0x8014F13Cu, 0x8014F148u, 0x8014F154u, 0x8014F160u, 0x8014F16Cu
};

static const unsigned int sLoopRows[FTREMIX_RAPID_JAB_STYLE_COUNT] =
{
    0x8014F3F4u, 0x8014F400u, 0x8014F40Cu, 0x8014F418u, 0x8014F424u
};

static const unsigned int sEndRows[FTREMIX_RAPID_JAB_STYLE_COUNT] =
{
    0x8014F490u, 0x8014F49Cu, 0x8014F4A8u, 0x8014F4B4u, 0x8014F4C0u
};

static const unsigned int sUnknownRows[FTREMIX_RAPID_JAB_STYLE_COUNT] =
{
    0x8014F5B0u, 0x8014F5C4u, 0x8014F5D8u, 0x8014F5ECu, 0x8014F600u
};

static void testVanillaRowsOrderIsFoxLinkKirbyPurinCaptain(void)
{
    CHECK_EQ(REMIX_ID_FOX, dFTRemixJabRapidStyleFkinds[nFTRemixJabRapidStyleFox - 1]);
    CHECK_EQ(REMIX_ID_LINK, dFTRemixJabRapidStyleFkinds[nFTRemixJabRapidStyleLink - 1]);
    CHECK_EQ(REMIX_ID_KIRBY, dFTRemixJabRapidStyleFkinds[nFTRemixJabRapidStyleKirby - 1]);
    CHECK_EQ(REMIX_ID_JIGGLY, dFTRemixJabRapidStyleFkinds[nFTRemixJabRapidStylePurin - 1]);
    CHECK_EQ(REMIX_ID_CAPTAIN, dFTRemixJabRapidStyleFkinds[nFTRemixJabRapidStyleCaptain - 1]);
}

static void testJab3GateOpensOnlyOnTheEnabledWord(void)
{
    CHECK_TRUE(ftRemixJabRulesIsJab3Gate(WORD_JAB_3_ENABLED));
    CHECK_TRUE(!ftRemixJabRulesIsJab3Gate(WORD_JAB_3_DISABLED));
    CHECK_TRUE(!ftRemixJabRulesIsJab3Gate(0));
}

static void testRapidGateOpensOnlyOnTheEnabledWord(void)
{
    CHECK_TRUE(ftRemixJabRulesIsRapidGate(WORD_RAPID_ENABLED));
    CHECK_TRUE(!ftRemixJabRulesIsRapidGate(WORD_RAPID_DISABLED));
    CHECK_TRUE(!ftRemixJabRulesIsRapidGate(0));
}

static void testFollowupKeepsTheWindowWhenTheTimerIsDisabled(void)
{
    CHECK_EQ(nFTRemixJabFollowupKeep, ftRemixJabRulesFollowupKind(WORD_TIMER_DISABLED));
}

static void testFollowupUsesTheDefaultForAVanillaCase(void)
{
    CHECK_EQ(nFTRemixJabFollowupDefault, ftRemixJabRulesFollowupKind(WORD_TIMER_MARIO));
    CHECK_EQ(nFTRemixJabFollowupDefault, ftRemixJabRulesFollowupKind(0x8014EB94u));
}

static void testFollowupUsesTheLongWindowForARemixRoutine(void)
{
    CHECK_EQ(nFTRemixJabFollowupLong, ftRemixJabRulesFollowupKind(WORD_TIMER_LANKY));
}

static void testJab3StatusOfAVanillaCaseIsTheFirstSpecialStatus(void)
{
    CHECK_EQ(0xDC, ftRemixJabRulesJab3Status(REMIX_ID_DRM, WORD_ACTION_MARIO));
    CHECK_EQ(0xDC, ftRemixJabRulesJab3Status(REMIX_ID_SHEIK, 0x8014EC10u));
}

static void testJab3StatusIsNoneWhenTheActionIsDisabled(void)
{
    CHECK_EQ(FTREMIX_JAB_STATUS_NONE, ftRemixJabRulesJab3Status(REMIX_ID_MARTH, WORD_ACTION_DISABLED));
    CHECK_EQ(FTREMIX_JAB_STATUS_NONE, ftRemixJabRulesJab3Status(REMIX_ID_WARIO, WORD_ACTION_DISABLED));
}

static void testJab3StatusOfTheRemixRoutinesIsTheOwnJab3(void)
{
    CHECK_EQ(0xDE, ftRemixJabRulesJab3Status(REMIX_ID_BANJO, WORD_ACTION_BANJO));
    CHECK_EQ(0xDE, ftRemixJabRulesJab3Status(REMIX_ID_NBANJO, WORD_ACTION_BANJO));
    CHECK_EQ(0xFE, ftRemixJabRulesJab3Status(REMIX_ID_DEDEDE, 0x8056FFA8u));
    CHECK_EQ(0xFE, ftRemixJabRulesJab3Status(REMIX_ID_NDEDEDE, 0x8056FFA8u));
    CHECK_EQ(0xE9, ftRemixJabRulesJab3Status(REMIX_ID_BOWSER, 0x8055004Cu));
    CHECK_EQ(0xE9, ftRemixJabRulesJab3Status(REMIX_ID_NBOWSER, 0x8055004Cu));
    CHECK_EQ(0xE9, ftRemixJabRulesJab3Status(REMIX_ID_GBOWSER, 0x805517F4u));
}

static void testJab3StatusOfAnUnknownRemixRoutineIsNone(void)
{
    CHECK_EQ(FTREMIX_JAB_STATUS_NONE, ftRemixJabRulesJab3Status(REMIX_ID_DRM, 0x80600000u));
}

static void testRapidStyleMatchesTheVanillaRows(void)
{
    CHECK_EQ(nFTRemixJabRapidStyleFox, ftRemixJabRulesRapidStyle(0x8014F13Cu, sBeginRows));
    CHECK_EQ(nFTRemixJabRapidStyleLink, ftRemixJabRulesRapidStyle(0x8014F148u, sBeginRows));
    CHECK_EQ(nFTRemixJabRapidStyleKirby, ftRemixJabRulesRapidStyle(0x8014F154u, sBeginRows));
    CHECK_EQ(nFTRemixJabRapidStylePurin, ftRemixJabRulesRapidStyle(0x8014F160u, sBeginRows));
    CHECK_EQ(nFTRemixJabRapidStyleCaptain, ftRemixJabRulesRapidStyle(0x8014F16Cu, sBeginRows));
}

static void testRapidStyleIsNoneForDisabledZeroAndRemixWords(void)
{
    CHECK_EQ(nFTRemixJabRapidStyleNone, ftRemixJabRulesRapidStyle(WORD_BEGIN_DISABLED, sBeginRows));
    CHECK_EQ(nFTRemixJabRapidStyleNone, ftRemixJabRulesRapidStyle(WORD_MEWTWO_BEGIN, sBeginRows));
}

static void testRapidStyleIsNoneWhenTheImageIsNotLoaded(void)
{
    static const unsigned int emptyRows[FTREMIX_RAPID_JAB_STYLE_COUNT] = { 0, 0, 0, 0, 0 };

    CHECK_EQ(nFTRemixJabRapidStyleNone, ftRemixJabRulesRapidStyle(0, emptyRows));
}

static void testFoxKirbyAndPurinRapidJabUseTheFirstSpecialStatuses(void)
{
    CHECK_EQ(0xDC, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_FALCO, sBeginRows[0], sBeginRows));
    CHECK_EQ(0xDD, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseLoop, REMIX_ID_FALCO, sLoopRows[0], sLoopRows));
    CHECK_EQ(0xDE, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseEnd, REMIX_ID_FALCO, sEndRows[0], sEndRows));
    CHECK_EQ(0xDC, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_JKIRBY, sBeginRows[2], sBeginRows));
    CHECK_EQ(0xDE, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseEnd, REMIX_ID_JPUFF, sEndRows[3], sEndRows));
}

static void testLinkAndCaptainRapidJabUseTheSecondSpecialStatuses(void)
{
    CHECK_EQ(0xDD, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_YLINK, sBeginRows[1], sBeginRows));
    CHECK_EQ(0xDE, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseLoop, REMIX_ID_GND, sLoopRows[4], sLoopRows));
    CHECK_EQ(0xDF, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseEnd, REMIX_ID_BANJO, sEndRows[4], sEndRows));
}

static void testSheikBeginsLikeFoxWhileItsParentIsCaptain(void)
{
    CHECK_EQ(0xDC, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_SHEIK, 0x8014F13Cu, sBeginRows));
    CHECK_EQ(nFTRemixJabRapidStyleCaptain, ftRemixJabRulesRapidStyle(0x8014F600u, sUnknownRows));
}

static void testMewtwoRapidJabUsesItsOwnStatuses(void)
{
    CHECK_EQ(0xE7, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_MTWO, WORD_MEWTWO_BEGIN, sBeginRows));
    CHECK_EQ(0xE8, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseLoop, REMIX_ID_MTWO, 0x8055C608u, sLoopRows));
    CHECK_EQ(0xE9, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseEnd, REMIX_ID_NMTWO, 0x8055C614u, sEndRows));
}

static void testRapidStatusIsNoneForADisabledWord(void)
{
    CHECK_EQ(FTREMIX_JAB_STATUS_NONE, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_MARTH, WORD_BEGIN_DISABLED, sBeginRows));
    CHECK_EQ(FTREMIX_JAB_STATUS_NONE, ftRemixJabRulesRapidStatus(nFTRemixJabRapidPhaseBegin, REMIX_ID_DRM, WORD_MEWTWO_BEGIN, sBeginRows));
}

static void testRapidInputsMinFollowTheStyle(void)
{
    CHECK_EQ(4, ftRemixJabRulesRapidInputsMin(nFTRemixJabRapidStyleFox));
    CHECK_EQ(5, ftRemixJabRulesRapidInputsMin(nFTRemixJabRapidStyleLink));
    CHECK_EQ(4, ftRemixJabRulesRapidInputsMin(nFTRemixJabRapidStyleKirby));
    CHECK_EQ(4, ftRemixJabRulesRapidInputsMin(nFTRemixJabRapidStylePurin));
    CHECK_EQ(6, ftRemixJabRulesRapidInputsMin(nFTRemixJabRapidStyleCaptain));
    CHECK_EQ(0, ftRemixJabRulesRapidInputsMin(nFTRemixJabRapidStyleNone));
}

static void testOnlyMewtwoStartsTheRapidJabAtJab2(void)
{
    CHECK_TRUE(ftRemixJabRulesStartsRapidJabAtJab2(REMIX_ID_MTWO));
    CHECK_TRUE(ftRemixJabRulesStartsRapidJabAtJab2(REMIX_ID_NMTWO));
    CHECK_TRUE(!ftRemixJabRulesStartsRapidJabAtJab2(REMIX_ID_FALCO));
    CHECK_TRUE(!ftRemixJabRulesStartsRapidJabAtJab2(REMIX_ID_FOX));
}

static void testOnlySlippyRapidJabRestartsJab1(void)
{
    CHECK_TRUE(ftRemixJabRulesRapidJabRestartsJab1(REMIX_ID_SLIPPY));
    CHECK_TRUE(!ftRemixJabRulesRapidJabRestartsJab1(REMIX_ID_PEPPY));
    CHECK_TRUE(!ftRemixJabRulesRapidJabRestartsJab1(REMIX_ID_FOX));
}

void remixTestJabRules(void)
{
    printf("jab rules\n");
    RUN_TEST(testVanillaRowsOrderIsFoxLinkKirbyPurinCaptain);
    RUN_TEST(testJab3GateOpensOnlyOnTheEnabledWord);
    RUN_TEST(testRapidGateOpensOnlyOnTheEnabledWord);
    RUN_TEST(testFollowupKeepsTheWindowWhenTheTimerIsDisabled);
    RUN_TEST(testFollowupUsesTheDefaultForAVanillaCase);
    RUN_TEST(testFollowupUsesTheLongWindowForARemixRoutine);
    RUN_TEST(testJab3StatusOfAVanillaCaseIsTheFirstSpecialStatus);
    RUN_TEST(testJab3StatusIsNoneWhenTheActionIsDisabled);
    RUN_TEST(testJab3StatusOfTheRemixRoutinesIsTheOwnJab3);
    RUN_TEST(testJab3StatusOfAnUnknownRemixRoutineIsNone);
    RUN_TEST(testRapidStyleMatchesTheVanillaRows);
    RUN_TEST(testRapidStyleIsNoneForDisabledZeroAndRemixWords);
    RUN_TEST(testRapidStyleIsNoneWhenTheImageIsNotLoaded);
    RUN_TEST(testFoxKirbyAndPurinRapidJabUseTheFirstSpecialStatuses);
    RUN_TEST(testLinkAndCaptainRapidJabUseTheSecondSpecialStatuses);
    RUN_TEST(testSheikBeginsLikeFoxWhileItsParentIsCaptain);
    RUN_TEST(testMewtwoRapidJabUsesItsOwnStatuses);
    RUN_TEST(testRapidStatusIsNoneForADisabledWord);
    RUN_TEST(testRapidInputsMinFollowTheStyle);
    RUN_TEST(testOnlyMewtwoStartsTheRapidJabAtJab2);
    RUN_TEST(testOnlySlippyRapidJabRestartsJab1);
}
