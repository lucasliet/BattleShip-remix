/**
 * test_twelvecb_rules.c - unit tests of the 12-Character Battle rules (gm/gmremixtwelvecbrules.c).
 */

#include "test_check.h"

#include "gm/gmremixtwelvecbrules.h"

static void playGame(GMRemixTwelveCBState *state, int32_t fkind0, int32_t fkind1, int32_t portrait0, int32_t portrait1,
                     int32_t stocks0, int32_t stocks1)
{
    const int32_t fkinds[GMREMIX_TWELVECB_PLAYERS] = { fkind0, fkind1 };
    const int32_t stocks[GMREMIX_TWELVECB_PLAYERS] = { stocks0, stocks1 };
    const int32_t was_random[GMREMIX_TWELVECB_PLAYERS] = { 0, 0 };

    gmRemixTwelveCBSetNextPortrait(state, 0, portrait0);
    gmRemixTwelveCBSetNextPortrait(state, 1, portrait1);
    gmRemixTwelveCBStartGame(state, fkinds, stocks, was_random);
}

static void testInitGivesFourStocksToEveryCharacter(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);

    CHECK_EQ(GMREMIX_TWELVECB_STATUS_NOT_STARTED, state.status);
    CHECK_EQ(-1, state.current_game);
    CHECK_EQ(48, state.players[0].stocks_remaining);
    CHECK_EQ(48, state.players[1].stocks_remaining);
    CHECK_EQ(4, gmRemixTwelveCBGetPortraitStocks(&state, 0));
    CHECK_EQ(4, gmRemixTwelveCBGetPortraitStocks(&state, 23));
}

static void testUpdateStocksRestartsPoolsOnlyWhenChanged(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBUpdateStocks(&state, 0);
    CHECK_EQ(12, state.players[0].stocks_remaining);
    CHECK_EQ(1, gmRemixTwelveCBGetPortraitStocks(&state, 7));

    state.players[0].stocks_remaining = 5;
    gmRemixTwelveCBUpdateStocks(&state, 0);
    CHECK_EQ(5, state.players[0].stocks_remaining);

    gmRemixTwelveCBUpdateStocks(&state, 98);
    CHECK_EQ(1188, state.players[0].stocks_remaining);
    CHECK_EQ(99, gmRemixTwelveCBGetPortraitStocks(&state, 0));
}

static void testPortraitOutOfRangeCountsAsDefeated(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);

    CHECK_EQ(0, gmRemixTwelveCBGetPortraitStocks(&state, -1));
    CHECK_EQ(0, gmRemixTwelveCBGetPortraitStocks(&state, GMREMIX_TWELVECB_PORTRAITS));
}

static void testFirstGameStartsWithThePortraitStocks(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBSetNextPortrait(&state, 0, 4);
    gmRemixTwelveCBSetNextPortrait(&state, 1, 9);

    CHECK_EQ(3, gmRemixTwelveCBGetInitialStocks(&state, 0));
    CHECK_EQ(3, gmRemixTwelveCBGetInitialStocks(&state, 1));
}

static void testKoTakesOneStockFromThePoolAndThePortrait(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    playGame(&state, 1, 2, 0, 0, 3, 3);
    gmRemixTwelveCBRecordKo(&state, 1, 2);

    CHECK_EQ(GMREMIX_TWELVECB_STATUS_STARTED, state.status);
    CHECK_EQ(47, state.players[1].stocks_remaining);
    CHECK_EQ(48, state.players[0].stocks_remaining);
    CHECK_EQ(2, state.players[1].matches[0].stocks_end);
    CHECK_EQ(3, gmRemixTwelveCBGetPortraitStocks(&state, 0));
}

static void testBattleCompletesWhenThePoolIsEmpty(void)
{
    GMRemixTwelveCBState state;
    int32_t ko;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBUpdateStocks(&state, 0);
    playGame(&state, 1, 2, 0, 0, 0, 0);

    for (ko = 0; ko < 11; ko++)
    {
        gmRemixTwelveCBRecordKo(&state, 0, -1);
    }
    CHECK_EQ(GMREMIX_TWELVECB_STATUS_STARTED, state.status);

    gmRemixTwelveCBRecordKo(&state, 0, -1);
    CHECK_EQ(GMREMIX_TWELVECB_STATUS_COMPLETE, state.status);
    CHECK_EQ(0, state.players[0].stocks_remaining);

    gmRemixTwelveCBRecordKo(&state, 0, -1);
    CHECK_EQ(0, state.players[0].stocks_remaining);
}

static void testWinnerKeepsTheStocksOfThePreviousGame(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    playGame(&state, 1, 2, 0, 0, 3, 3);
    gmRemixTwelveCBRecordKo(&state, 1, 2);
    gmRemixTwelveCBRecordKo(&state, 1, 1);
    gmRemixTwelveCBRecordKo(&state, 1, 0);
    gmRemixTwelveCBRecordKo(&state, 1, -1);

    gmRemixTwelveCBSetNextPortrait(&state, 0, 0);
    gmRemixTwelveCBSetNextPortrait(&state, 1, 5);

    CHECK_EQ(3, gmRemixTwelveCBGetInitialStocks(&state, 0));
    CHECK_EQ(3, gmRemixTwelveCBGetInitialStocks(&state, 1));

    state.players[0].matches[0].stocks_end = 1;
    CHECK_EQ(1, gmRemixTwelveCBGetInitialStocks(&state, 0));
}

static void testLastStockLeftFallsBackToThePortraitStocks(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    playGame(&state, 1, 2, 0, 0, 3, 3);
    state.players[0].matches[0].stocks_end = 0;

    gmRemixTwelveCBSetNextPortrait(&state, 0, 2);

    CHECK_EQ(3, gmRemixTwelveCBGetInitialStocks(&state, 0));
}

static void testDefeatedPortraitIsNotReusedWithZeroStocks(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    playGame(&state, 1, 2, 0, 0, 3, 3);
    gmRemixTwelveCBRecordKo(&state, 1, 2);
    gmRemixTwelveCBRecordKo(&state, 1, 1);
    gmRemixTwelveCBRecordKo(&state, 1, 0);
    gmRemixTwelveCBRecordKo(&state, 1, -1);

    CHECK_EQ(0, gmRemixTwelveCBGetPortraitStocks(&state, 0));
    CHECK_EQ(4, gmRemixTwelveCBGetPortraitStocks(&state, 1));
}

static void testBestCharacterTakesTheMostStocksAndBreaksTiesByFewerLost(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    playGame(&state, 1, 2, 0, 0, 3, 3);
    state.players[0].matches[0].stocks_end = 3;
    state.players[1].matches[0].stocks_end = GMREMIX_TWELVECB_NO_STOCKS;

    playGame(&state, 1, 3, 0, 5, 3, 3);
    state.players[0].matches[1].stocks_end = 1;
    state.players[1].matches[1].stocks_end = GMREMIX_TWELVECB_NO_STOCKS;

    playGame(&state, 1, 2, 0, 1, 1, 3);
    state.players[0].matches[2].stocks_end = GMREMIX_TWELVECB_NO_STOCKS;
    state.players[1].matches[2].stocks_end = 2;

    gmRemixTwelveCBCalculateBest(&state);

    CHECK_EQ(1, state.players[0].best_fkind);
    CHECK_EQ(9, state.players[0].best_tkos_for);
    CHECK_EQ(4, state.players[0].best_tkos_against);
    CHECK_EQ(2, state.players[1].best_fkind);
    CHECK_EQ(2, state.players[1].best_tkos_for);
    CHECK_EQ(1, state.players[1].best_tkos_against);
}

static void testBestCharacterIsUnsetBeforeTheBattleStarts(void)
{
    GMRemixTwelveCBState state;

    gmRemixTwelveCBInit(&state);
    gmRemixTwelveCBCalculateBest(&state);

    CHECK_EQ(GMREMIX_TWELVECB_NO_FKIND, state.players[0].best_fkind);
    CHECK_EQ(-1, state.players[0].best_tkos_for);
}

static void testGameRecordsStopAtTheLimitInsteadOfOverrunning(void)
{
    GMRemixTwelveCBState state;
    int32_t game;

    gmRemixTwelveCBInit(&state);
    for (game = 0; game < GMREMIX_TWELVECB_GAMES + 3; game++)
    {
        playGame(&state, 1, 2, 0, 0, 3, 3);
    }
    CHECK_EQ(GMREMIX_TWELVECB_GAMES - 1, state.current_game);

    gmRemixTwelveCBCalculateBest(&state);
    CHECK_EQ(1, state.players[0].best_fkind);
}

void remixTestTwelveCBRules(void)
{
    printf("twelvecb rules\n");
    RUN_TEST(testInitGivesFourStocksToEveryCharacter);
    RUN_TEST(testUpdateStocksRestartsPoolsOnlyWhenChanged);
    RUN_TEST(testPortraitOutOfRangeCountsAsDefeated);
    RUN_TEST(testFirstGameStartsWithThePortraitStocks);
    RUN_TEST(testKoTakesOneStockFromThePoolAndThePortrait);
    RUN_TEST(testBattleCompletesWhenThePoolIsEmpty);
    RUN_TEST(testWinnerKeepsTheStocksOfThePreviousGame);
    RUN_TEST(testLastStockLeftFallsBackToThePortraitStocks);
    RUN_TEST(testDefeatedPortraitIsNotReusedWithZeroStocks);
    RUN_TEST(testBestCharacterTakesTheMostStocksAndBreaksTiesByFewerLost);
    RUN_TEST(testBestCharacterIsUnsetBeforeTheBattleStarts);
    RUN_TEST(testGameRecordsStopAtTheLimitInsteadOfOverrunning);
}
