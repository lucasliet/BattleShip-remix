/**
 * test_twelvecb_modes_rules.c - unit tests of the 12-Character Battle and Stock Mode rules of the modes package: Last mode stock
 * count after a KO (update_stocks_remaining_), CSS announcer of a finished battle (update_announcer_on_entry_), token pickup
 * (prevent_token_pickup_), start block (treat_selected_defeated_chars_as_unselected_), CSS port setup (setup_ports_and_characters_),
 * defeated pose rows (defeated_action_override_) and the DK Mode easter egg (DKMode.asm _check_portraits).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixtwelvecbdefeatedrules.h"
#include "gm/gmremixstockmoderules.h"
#include "gm/gmremixtwelvecbrules.h"
#include "mn/mnplayers/mnremixcssmoderules.h"
#include "mn/mnplayers/mnremixtwelvecbcustom.h"
#include "mn/mnplayers/mnremixtwelvecbdkmoderules.h"

#define TEST_NONE_FKIND 0x1C
#define TEST_FKIND_MARIO 0
#define TEST_FKIND_FOX 1
#define TEST_FKIND_LINK 5

static void testAKoTakesOneStockFromTheLastCount(void)
{
    CHECK_EQ(2, gmRemixStockModeRulesCountAfterKo(3, 0));
    CHECK_EQ(0, gmRemixStockModeRulesCountAfterKo(1, 0));
    CHECK_EQ(-1, gmRemixStockModeRulesCountAfterKo(0, 0));
}

static void testAStaminaFighterOutOfStocksStaysAtMinusOne(void)
{
    CHECK_EQ(-1, gmRemixStockModeRulesCountAfterKo(-1, 1));
    CHECK_EQ(-2, gmRemixStockModeRulesCountAfterKo(-1, 0));
    CHECK_EQ(-1, gmRemixStockModeRulesCountAfterKo(0, 1));
}

static void testTheWinnerAnnouncementNamesPlayerOneUnlessHisPoolIsEmpty(void)
{
    CHECK_EQ(503, mnRemixCssModeRulesTwelveCBWinnerFgm(7));
    CHECK_EQ(504, mnRemixCssModeRulesTwelveCBWinnerFgm(0));
}

static void testWinsIsSaidOnlyOnFrameThirtyFourOfAFinishedBattle(void)
{
    CHECK_EQ(0x34, MNREMIX_CSS_MODE_WINS_FRAME);
    CHECK_EQ(533, mnRemixCssModeRulesTwelveCBWinsFgm(1, 0x34));
    CHECK_EQ(0, mnRemixCssModeRulesTwelveCBWinsFgm(1, 0x33));
    CHECK_EQ(0, mnRemixCssModeRulesTwelveCBWinsFgm(1, 0x35));
    CHECK_EQ(0, mnRemixCssModeRulesTwelveCBWinsFgm(0, 0x34));
}

static void startGame(GMRemixTwelveCBState *state, int stocks_p1_end, int stocks_p2_end)
{
    int32_t fkinds[GMREMIX_TWELVECB_PLAYERS] = { TEST_FKIND_MARIO, TEST_FKIND_FOX };
    int32_t stocks[GMREMIX_TWELVECB_PLAYERS] = { 3, 3 };
    int32_t was_random[GMREMIX_TWELVECB_PLAYERS] = { 0, 0 };

    gmRemixTwelveCBSetNextPortrait(state, 0, 0);
    gmRemixTwelveCBSetNextPortrait(state, 1, 5);
    gmRemixTwelveCBStartGame(state, fkinds, stocks, was_random);
    state->players[0].matches[state->current_game].stocks_end = (uint8_t)stocks_p1_end;
    state->players[1].matches[state->current_game].stocks_end = (uint8_t)stocks_p2_end;
}

static void testNoGameMeansNoStocksLeftFromTheLastGame(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    CHECK_EQ(0, gmRemixTwelveCBLastGameHadStocks(&state, 0));
}

static void testTheLastGameHadStocksUnlessThePlayerWasDefeated(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    startGame(&state, 2, GMREMIX_TWELVECB_NO_STOCKS);
    CHECK_TRUE(gmRemixTwelveCBLastGameHadStocks(&state, 0));
    CHECK_EQ(0, gmRemixTwelveCBLastGameHadStocks(&state, 1));
    CHECK_EQ(0, gmRemixTwelveCBLastGameHadStocks(&state, 2));
}

static void testACpuTokenWithStocksLeftCannotBePickedUpAfterAGame(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    CHECK_TRUE(gmRemixTwelveCBCanPickUpToken(&state, 0, 0));
    startGame(&state, 2, GMREMIX_TWELVECB_NO_STOCKS);
    CHECK_EQ(0, gmRemixTwelveCBCanPickUpToken(&state, 0, 0));
    CHECK_TRUE(gmRemixTwelveCBCanPickUpToken(&state, 1, 0));
    CHECK_TRUE(gmRemixTwelveCBCanPickUpToken(&state, 0, 1));
}

static void testACompleteBattleBlocksTheStartOfAPlayerWithAnEmptyPool(void)
{
    CHECK_TRUE(gmRemixTwelveCBSlotMayStart(2, 5));
    CHECK_EQ(0, gmRemixTwelveCBSlotMayStart(0, 5));
    CHECK_EQ(0, gmRemixTwelveCBSlotMayStart(2, 0));
    CHECK_EQ(0, gmRemixTwelveCBSlotMayStart(0, 0));
}

static int32_t sFindValidPortraitResult;
static int sFindValidPortraitCalls;

static int32_t fakeFindValidPortrait(int32_t port, int32_t fkind)
{
    (void)port;
    (void)fkind;
    sFindValidPortraitCalls++;
    return sFindValidPortraitResult;
}

static int32_t fakeIsFkindValid(int32_t port, int32_t fkind)
{
    (void)port;
    return (fkind != TEST_FKIND_LINK) ? 1 : 0;
}

static const GMRemixTwelveCBPortOps sPortOps = { fakeFindValidPortrait, fakeIsFkindValid };

static void testAPortBeforeTheFirstGameKeepsAValidCharacter(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBSetupPort(&state, 0, GMREMIX_TWELVECB_PKIND_HUMAN, TEST_FKIND_MARIO, -1, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(GMREMIX_TWELVECB_PKIND_HUMAN, setup.pkind);
    CHECK_EQ(TEST_FKIND_MARIO, setup.fkind);
}

static void testACharacterOutsideTheSetClearsTheSlotToAHumanWithNoCharacter(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBSetupPort(&state, 1, GMREMIX_TWELVECB_PKIND_CPU, TEST_FKIND_LINK, -1, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(GMREMIX_TWELVECB_PKIND_HUMAN, setup.pkind);
    CHECK_EQ(TEST_NONE_FKIND, setup.fkind);
}

static void testAnAbsentPortIsLeftAlone(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBSetupPort(&state, 0, GMREMIX_TWELVECB_PKIND_NONE, TEST_FKIND_LINK, 4, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(GMREMIX_TWELVECB_PKIND_NONE, setup.pkind);
    CHECK_EQ(TEST_FKIND_LINK, setup.fkind);
    CHECK_EQ(4, setup.portrait);
}

static void testTheWinnerComesBackOnTheCharacterAndPortraitHeWonWith(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    startGame(&state, 2, GMREMIX_TWELVECB_NO_STOCKS);
    gmRemixTwelveCBSetupPort(&state, 0, GMREMIX_TWELVECB_PKIND_HUMAN, TEST_FKIND_FOX, -1, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(TEST_FKIND_MARIO, setup.fkind);
    CHECK_EQ(0, setup.portrait);
    CHECK_EQ(GMREMIX_TWELVECB_PKIND_HUMAN, setup.pkind);
}

static void testADefeatedHumanKeepsACharacterThatStillHasAPortraitWithStocks(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    startGame(&state, 2, GMREMIX_TWELVECB_NO_STOCKS);
    sFindValidPortraitResult = 9;
    sFindValidPortraitCalls = 0;
    gmRemixTwelveCBSetupPort(&state, 1, GMREMIX_TWELVECB_PKIND_HUMAN, TEST_FKIND_FOX, -1, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(1, sFindValidPortraitCalls);
    CHECK_EQ(TEST_FKIND_FOX, setup.fkind);
    CHECK_EQ(9, setup.portrait);
}

static void testADefeatedHumanWithNoPortraitLeftLosesTheCharacter(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    startGame(&state, 2, GMREMIX_TWELVECB_NO_STOCKS);
    sFindValidPortraitResult = -1;
    gmRemixTwelveCBSetupPort(&state, 1, GMREMIX_TWELVECB_PKIND_HUMAN, TEST_FKIND_FOX, -1, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(TEST_NONE_FKIND, setup.fkind);
    CHECK_EQ(GMREMIX_TWELVECB_PKIND_HUMAN, setup.pkind);
}

static void testADefeatedCpuKeepsItsCharacterAndTheRememberedPortrait(void)
{
    GMRemixTwelveCBState state;
    GMRemixTwelveCBPortSetup setup;

    gmRemixTwelveCBInit(&state);
    startGame(&state, 2, GMREMIX_TWELVECB_NO_STOCKS);
    sFindValidPortraitCalls = 0;
    gmRemixTwelveCBSetupPort(&state, 1, GMREMIX_TWELVECB_PKIND_CPU, TEST_FKIND_FOX, 7, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(0, sFindValidPortraitCalls);
    CHECK_EQ(GMREMIX_TWELVECB_PKIND_CPU, setup.pkind);
    CHECK_EQ(TEST_FKIND_FOX, setup.fkind);
    CHECK_EQ(7, setup.portrait);
    gmRemixTwelveCBSetupPort(&state, 1, GMREMIX_TWELVECB_PKIND_CPU, TEST_FKIND_FOX, -1, TEST_NONE_FKIND, &sPortOps, &setup);
    CHECK_EQ(5, setup.portrait);
}

static void testTheDefeatedRowIsTwelveBytesPerCharacter(void)
{
    CHECK_EQ(0x1000, ftRemixTwelveCBDefeatedRulesRowAddress(0x1000, 0));
    CHECK_EQ(0x1000 + 12, ftRemixTwelveCBDefeatedRulesRowAddress(0x1000, 1));
    CHECK_EQ(0x1000 + (12 * FTREMIX_TWELVECB_DEFEATED_LAST_FKIND), ftRemixTwelveCBDefeatedRulesRowAddress(0x1000, FTREMIX_TWELVECB_DEFEATED_LAST_FKIND));
    CHECK_EQ(0, ftRemixTwelveCBDefeatedRulesRowAddress(0x1000, FTREMIX_TWELVECB_DEFEATED_LAST_FKIND + 1));
    CHECK_EQ(0, ftRemixTwelveCBDefeatedRulesRowAddress(0x1000, -1));
}

static void testTheDefeatedPoseReplacesTheMotionOnlyOnTheVsSelectOfADefeatedCharacter(void)
{
    CHECK_TRUE(ftRemixTwelveCBDefeatedRulesReplacesMotion(1, 1, 0x222));
    CHECK_EQ(0, ftRemixTwelveCBDefeatedRulesReplacesMotion(0, 1, 0x222));
    CHECK_EQ(0, ftRemixTwelveCBDefeatedRulesReplacesMotion(1, 0, 0x222));
    CHECK_EQ(0, ftRemixTwelveCBDefeatedRulesReplacesMotion(1, 1, 0));
}

static void fillTable(int *table, const char *rows[3], int ink_fkind, int other_fkind)
{
    int row;
    int column;

    for (row = 0; row < 3; row++)
    {
        for (column = 0; column < 8; column++)
        {
            table[(row * 8) + column] = (rows[row][column % 4] == 'X') ? ink_fkind : other_fkind;
        }
    }
}

static void testPlayerOneSpellingDAndPlayerTwoSpellingKSwitchesDkModeOn(void)
{
    const char *letter_d[3] = { "XX..", "X.X.", "XX.." };
    const char *letter_k[3] = { "X.X.", "XX..", "X.X." };
    int player_one[MNREMIX_TCB_DK_MODE_SLOTS];
    int player_two[MNREMIX_TCB_DK_MODE_SLOTS];

    fillTable(player_one, letter_d, 3, 9);
    fillTable(player_two, letter_k, 4, 8);
    CHECK_EQ(MNREMIX_TCB_DK_MODE_DK, mnRemixTwelveCBDKModeSelect(player_one, player_two));
}

static void testPlayerOneSpellingLAndPlayerTwoSpellingKSwitchesLankyModeOn(void)
{
    const char *letter_l[3] = { "X...", "X...", "XXX." };
    const char *letter_k[3] = { "X.X.", "XX..", "X.X." };
    int player_one[MNREMIX_TCB_DK_MODE_SLOTS];
    int player_two[MNREMIX_TCB_DK_MODE_SLOTS];

    fillTable(player_one, letter_l, 3, 9);
    fillTable(player_two, letter_k, 4, 8);
    CHECK_EQ(MNREMIX_TCB_DK_MODE_LANKY, mnRemixTwelveCBDKModeSelect(player_one, player_two));
}

static void testTheEasterEggNeedsBothLetters(void)
{
    const char *letter_d[3] = { "XX..", "X.X.", "XX.." };
    const char *letter_k[3] = { "X.X.", "XX..", "X.X." };
    int player_one[MNREMIX_TCB_DK_MODE_SLOTS];
    int player_two[MNREMIX_TCB_DK_MODE_SLOTS];
    int flat[MNREMIX_TCB_DK_MODE_SLOTS];
    int slot;

    for (slot = 0; slot < MNREMIX_TCB_DK_MODE_SLOTS; slot++)
    {
        flat[slot] = 2;
    }
    fillTable(player_one, letter_d, 3, 9);
    fillTable(player_two, letter_k, 4, 8);
    CHECK_EQ(MNREMIX_TCB_DK_MODE_OFF, mnRemixTwelveCBDKModeSelect(player_one, flat));
    CHECK_EQ(MNREMIX_TCB_DK_MODE_OFF, mnRemixTwelveCBDKModeSelect(flat, player_two));
    CHECK_EQ(MNREMIX_TCB_DK_MODE_OFF, mnRemixTwelveCBDKModeSelect(player_two, player_one));
    CHECK_EQ(MNREMIX_TCB_DK_MODE_OFF, mnRemixTwelveCBDKModeSelect(flat, flat));
}

static void testAnotherCharacterInAnInkCellBreaksTheLetter(void)
{
    const char *letter_d[3] = { "XX..", "X.X.", "XX.." };
    const char *letter_k[3] = { "X.X.", "XX..", "X.X." };
    int player_one[MNREMIX_TCB_DK_MODE_SLOTS];
    int player_two[MNREMIX_TCB_DK_MODE_SLOTS];

    fillTable(player_one, letter_d, 3, 9);
    fillTable(player_two, letter_k, 4, 8);
    player_one[8] = 9;
    CHECK_EQ(MNREMIX_TCB_DK_MODE_OFF, mnRemixTwelveCBDKModeSelect(player_one, player_two));
}

static void testTheCustomTableOfAPlayerHoldsAllTwentyFourSlots(void)
{
    mnRemixTwelveCBCustomReset();
    CHECK_EQ(mnRemixTwelveCBCustomFkindOf(0, 0), mnRemixTwelveCBCustomFkindOf(0, 4));
    CHECK_EQ(mnRemixTwelveCBCustomFkindOf(0, 3), mnRemixTwelveCBCustomFkindOf(0, 7));
    CHECK_EQ(-1, mnRemixTwelveCBCustomFkindOf(2, 0));
    CHECK_EQ(-1, mnRemixTwelveCBCustomFkindOf(0, 24));
    CHECK_EQ(mnRemixTwelveCBCustomFkindOf(0, 5), mnRemixTwelveCBCustomFkind(5));
    CHECK_EQ(mnRemixTwelveCBCustomFkindOf(1, 5), mnRemixTwelveCBCustomFkind(5));
}

void remixTestTwelveCBModesRules(void)
{
    printf("12-character battle modes rules\n");
    RUN_TEST(testAKoTakesOneStockFromTheLastCount);
    RUN_TEST(testAStaminaFighterOutOfStocksStaysAtMinusOne);
    RUN_TEST(testTheWinnerAnnouncementNamesPlayerOneUnlessHisPoolIsEmpty);
    RUN_TEST(testWinsIsSaidOnlyOnFrameThirtyFourOfAFinishedBattle);
    RUN_TEST(testNoGameMeansNoStocksLeftFromTheLastGame);
    RUN_TEST(testTheLastGameHadStocksUnlessThePlayerWasDefeated);
    RUN_TEST(testACpuTokenWithStocksLeftCannotBePickedUpAfterAGame);
    RUN_TEST(testACompleteBattleBlocksTheStartOfAPlayerWithAnEmptyPool);
    RUN_TEST(testAPortBeforeTheFirstGameKeepsAValidCharacter);
    RUN_TEST(testACharacterOutsideTheSetClearsTheSlotToAHumanWithNoCharacter);
    RUN_TEST(testAnAbsentPortIsLeftAlone);
    RUN_TEST(testTheWinnerComesBackOnTheCharacterAndPortraitHeWonWith);
    RUN_TEST(testADefeatedHumanKeepsACharacterThatStillHasAPortraitWithStocks);
    RUN_TEST(testADefeatedHumanWithNoPortraitLeftLosesTheCharacter);
    RUN_TEST(testADefeatedCpuKeepsItsCharacterAndTheRememberedPortrait);
    RUN_TEST(testTheDefeatedRowIsTwelveBytesPerCharacter);
    RUN_TEST(testTheDefeatedPoseReplacesTheMotionOnlyOnTheVsSelectOfADefeatedCharacter);
    RUN_TEST(testPlayerOneSpellingDAndPlayerTwoSpellingKSwitchesDkModeOn);
    RUN_TEST(testPlayerOneSpellingLAndPlayerTwoSpellingKSwitchesLankyModeOn);
    RUN_TEST(testTheEasterEggNeedsBothLetters);
    RUN_TEST(testAnotherCharacterInAnInkCellBreaksTheLetter);
    RUN_TEST(testTheCustomTableOfAPlayerHoldsAllTwentyFourSlots);
}
