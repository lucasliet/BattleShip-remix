/**
 * test_css_panel_menu_rules.c - unit tests of the per-port panel menu of the Remix character select
 * (mn/mnplayers/mnremixcsspanelmenurules.c). Expected values come from the Remix asm: CharacterSelectDebugMenu.asm
 * (setup_menu_page_ 554, render_menu_page_ 644, create_item_* 788-1034, render_pagination_ 1073, hold_a_handler_ 1432,
 * handle_item_arrow_press_ 1761, handle_page_arrow_press_ 1949, add_menu_item 2477, the item order 2720-2739) and css/X.asm.
 */

#include "test_check.h"

#include <string.h>

#include "mn/mnplayers/mnremixcsspanelmenurules.h"

#define LIST_CAPACITY MNREMIX_PANELMENU_ITEM_COUNT

static int listItems(int mode, int is_cpu, int *items)
{
    return mnRemixPanelMenuRulesListItems(mode, is_cpu, items, LIST_CAPACITY);
}

static int isSame(const int *items, int count, const int *expected, int expected_count)
{
    return ((count == expected_count) && (memcmp(items, expected, (size_t)count * sizeof(int)) == 0)) ? 1 : 0;
}

static void testVsHumanListsEighteenItemsWithoutHandicapAndPractice(void)
{
    const int expected[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18 };
    int items[LIST_CAPACITY];
    int count = listItems(MNREMIX_PANELMENU_MODE_VS, 0, items);

    CHECK_TRUE(isSame(items, count, expected, 18));
}

static void testVsCpuListsHandicapAndNoHumanOnlyItems(void)
{
    const int expected[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 11, 12, 14, 17, 18 };
    int items[LIST_CAPACITY];
    int count = listItems(MNREMIX_PANELMENU_MODE_VS, 1, items);

    CHECK_TRUE(isSame(items, count, expected, 15));
}

static void testTrainingListsHaveNoStockDamagePoisonOrPractice(void)
{
    const int human[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9, 11, 12, 13, 14, 15, 16 };
    const int cpu[] = { 0, 1, 2, 3, 4, 5, 6, 8, 10, 11, 12, 14 };
    int items[LIST_CAPACITY];
    int count = listItems(MNREMIX_PANELMENU_MODE_TRAINING, 0, items);

    CHECK_TRUE(isSame(items, count, human, 15));
    count = listItems(MNREMIX_PANELMENU_MODE_TRAINING, 1, items);
    CHECK_TRUE(isSame(items, count, cpu, 12));
}

static void testOnePlayerListsPracticeAndIgnoresTheCpuBitmask(void)
{
    const int expected[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19 };
    int items[LIST_CAPACITY];
    int count = listItems(MNREMIX_PANELMENU_MODE_1P, 0, items);

    CHECK_TRUE(isSame(items, count, expected, 18));
    count = listItems(MNREMIX_PANELMENU_MODE_1P, 1, items);
    CHECK_TRUE(isSame(items, count, expected, 18));
}

static void testBonusAndAllStarLists(void)
{
    const int bonus[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18 };
    const int allstar[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9, 11, 12, 13, 14, 15, 16, 18 };
    int items[LIST_CAPACITY];
    int count = listItems(MNREMIX_PANELMENU_MODE_BONUS, 0, items);

    CHECK_TRUE(isSame(items, count, bonus, 17));
    count = listItems(MNREMIX_PANELMENU_MODE_ALLSTAR, 0, items);
    CHECK_TRUE(isSame(items, count, allstar, 16));
}

static void testListHonoursTheCapacity(void)
{
    int items[4];

    CHECK_EQ(4, mnRemixPanelMenuRulesListItems(MNREMIX_PANELMENU_MODE_VS, 0, items, 4));
    CHECK_EQ(0, mnRemixPanelMenuRulesListItems(99, 0, items, 4));
}

static void testResetListJoinsTheHumanAndCpuLists(void)
{
    const int vs[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
    const int training[] = { 0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
    int items[LIST_CAPACITY];
    int count = mnRemixPanelMenuRulesListResetItems(MNREMIX_PANELMENU_MODE_VS, items, LIST_CAPACITY);

    CHECK_TRUE(isSame(items, count, vs, 19));
    count = mnRemixPanelMenuRulesListResetItems(MNREMIX_PANELMENU_MODE_TRAINING, items, LIST_CAPACITY);
    CHECK_TRUE(isSame(items, count, training, 16));
}

static void testItemRangesAndDefaultsMatchTheItemFiles(void)
{
    CHECK_EQ(17, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_SHIELD));
    CHECK_EQ(20, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_PLAYER_TAG));
    CHECK_EQ(12, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_DELAY));
    CHECK_EQ(0x28, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_HANDICAP));
    CHECK_EQ(0x2A, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_KIRBY_HAT));
    CHECK_EQ(999, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_DAMAGE));
    CHECK_EQ(11, mnRemixPanelMenuRulesItemMax(MNREMIX_PANELMENU_ITEM_PRACTICE));
    CHECK_EQ(0, mnRemixPanelMenuRulesItemMin(MNREMIX_PANELMENU_ITEM_DAMAGE));
    CHECK_EQ(0, mnRemixPanelMenuRulesItemDefault(MNREMIX_PANELMENU_ITEM_SIZE));
    CHECK_EQ(0, mnRemixPanelMenuRulesItemMax(-1));
}

static void testOnlyDamageIsNumeric(void)
{
    int item;

    for (item = 0; item < MNREMIX_PANELMENU_ITEM_COUNT; item++)
    {
        CHECK_EQ((item == MNREMIX_PANELMENU_ITEM_DAMAGE) ? 1 : 0, mnRemixPanelMenuRulesItemIsNumeric(item));
    }
}

static void testLabelsAreTheAsmStrings(void)
{
    CHECK_EQ(0, strcmp("Initial Damage", mnRemixPanelMenuRulesItemLabel(MNREMIX_PANELMENU_ITEM_DAMAGE)));
    CHECK_EQ(0, strcmp("Taunt Btn.", mnRemixPanelMenuRulesItemLabel(MNREMIX_PANELMENU_ITEM_TAUNT_BUTTON)));
    CHECK_EQ(0, strcmp("Input HUD", mnRemixPanelMenuRulesItemLabel(MNREMIX_PANELMENU_ITEM_INPUT_HUD)));
    CHECK_EQ(0, strcmp("Delay", mnRemixPanelMenuRulesItemLabel(MNREMIX_PANELMENU_ITEM_DELAY)));
    CHECK_EQ(0, strcmp("", mnRemixPanelMenuRulesItemLabel(MNREMIX_PANELMENU_ITEM_COUNT)));
}

static void testHighScoreFlagsFollowTheItemFiles(void)
{
    const int flagged[] = { 6, 8, 10, 11, 12, 14, 15, 17, 18, 19 };
    int item;
    int index;

    for (item = 0; item < MNREMIX_PANELMENU_ITEM_COUNT; item++)
    {
        int is_flagged = 0;

        for (index = 0; index < 10; index++)
        {
            is_flagged = (flagged[index] == item) ? 1 : is_flagged;
        }
        CHECK_EQ(is_flagged, mnRemixPanelMenuRulesItemDisablesHighScores(item));
    }
}

static void testMaxPageDividesByThree(void)
{
    CHECK_EQ(0, mnRemixPanelMenuRulesMaxPage(0));
    CHECK_EQ(0, mnRemixPanelMenuRulesMaxPage(3));
    CHECK_EQ(1, mnRemixPanelMenuRulesMaxPage(4));
    CHECK_EQ(4, mnRemixPanelMenuRulesMaxPage(15));
    CHECK_EQ(5, mnRemixPanelMenuRulesMaxPage(17));
    CHECK_EQ(5, mnRemixPanelMenuRulesMaxPage(18));
}

static void testPageIsClampedWhenTheListShrinks(void)
{
    CHECK_EQ(3, mnRemixPanelMenuRulesClampPage(5, 3));
    CHECK_EQ(2, mnRemixPanelMenuRulesClampPage(2, 3));
}

static void testPageArrowsWrapAroundTheEnds(void)
{
    CHECK_EQ(5, mnRemixPanelMenuRulesStepPage(0, -1, 5));
    CHECK_EQ(0, mnRemixPanelMenuRulesStepPage(5, 1, 5));
    CHECK_EQ(3, mnRemixPanelMenuRulesStepPage(2, 1, 5));
    CHECK_EQ(1, mnRemixPanelMenuRulesStepPage(2, -1, 5));
}

static void testRowsOnPageAreTheItemsLeft(void)
{
    CHECK_EQ(3, mnRemixPanelMenuRulesRowsOnPage(0, 17));
    CHECK_EQ(2, mnRemixPanelMenuRulesRowsOnPage(5, 17));
    CHECK_EQ(3, mnRemixPanelMenuRulesRowsOnPage(5, 18));
    CHECK_EQ(0, mnRemixPanelMenuRulesRowsOnPage(6, 18));
    CHECK_EQ(0, mnRemixPanelMenuRulesRowsOnPage(0, 0));
}

static void testItemArrowsWrapBetweenMinAndMax(void)
{
    CHECK_EQ(17, mnRemixPanelMenuRulesStepValue(0, -1, 0, 17));
    CHECK_EQ(0, mnRemixPanelMenuRulesStepValue(17, 1, 0, 17));
    CHECK_EQ(1, mnRemixPanelMenuRulesStepValue(0, 1, 0, 17));
    CHECK_EQ(998, mnRemixPanelMenuRulesStepValue(999, -1, 0, 999));
}

static void testBlinkCountsZeroToNineteenAndShowsBelowEleven(void)
{
    int timer = 0;
    int visible_frames = 0;
    int frame;

    for (frame = 0; frame < MNREMIX_PANELMENU_BLINK_PERIOD; frame++)
    {
        visible_frames += mnRemixPanelMenuRulesBlinkIsVisible(timer);
        timer = mnRemixPanelMenuRulesBlinkNext(timer);
    }
    CHECK_EQ(11, visible_frames);
    CHECK_EQ(0, timer);
    CHECK_EQ(0, mnRemixPanelMenuRulesBlinkNext(19));
    CHECK_EQ(0, mnRemixPanelMenuRulesBlinkIsVisible(11));
}

static int holdFrame(MNRemixPanelMenuHold *hold)
{
    return mnRemixPanelMenuRulesHoldAdvance(hold, 1);
}

static void testHoldRepeatsEverySixthFrameAfterFifteenFrames(void)
{
    MNRemixPanelMenuHold hold = { 0, 0 };
    int frame;

    for (frame = 1; frame <= 19; frame++)
    {
        CHECK_EQ(0, holdFrame(&hold));
    }
    CHECK_EQ(1, holdFrame(&hold));
    for (frame = 21; frame <= 25; frame++)
    {
        CHECK_EQ(0, holdFrame(&hold));
    }
    CHECK_EQ(1, holdFrame(&hold));
}

static void testHoldRepeatsEveryFrameAfterNinetyFrames(void)
{
    MNRemixPanelMenuHold hold = { 0, 0 };
    int frame;

    for (frame = 1; frame < 90; frame++)
    {
        holdFrame(&hold);
    }
    for (frame = 90; frame < 100; frame++)
    {
        CHECK_EQ(1, holdFrame(&hold));
    }
}

static void testReleasingAResetsTheHoldCounters(void)
{
    MNRemixPanelMenuHold hold = { 0, 0 };
    int frame;

    for (frame = 0; frame < 30; frame++)
    {
        holdFrame(&hold);
    }
    CHECK_EQ(0, mnRemixPanelMenuRulesHoldAdvance(&hold, 0));
    CHECK_EQ(0, hold.held_frames);
    CHECK_EQ(0, hold.repeat_timer);
}

static void testPageArrowRestartsTheHoldWait(void)
{
    MNRemixPanelMenuHold hold = { 40, 3 };

    mnRemixPanelMenuRulesHoldRestart(&hold);
    CHECK_EQ(0, hold.held_frames);
    CHECK_EQ(3, hold.repeat_timer);
}

static void testRowsAreTwentyFourUnitsApart(void)
{
    CHECK_EQ(18, (int)mnRemixPanelMenuRulesRowTextY(0, 0));
    CHECK_EQ(66, (int)mnRemixPanelMenuRulesRowTextY(2, 0));
    CHECK_EQ(30, (int)mnRemixPanelMenuRulesRowTextY(0, 1));
    CHECK_EQ(54, (int)mnRemixPanelMenuRulesRowTextY(1, 1));
    CHECK_EQ(32, (int)mnRemixPanelMenuRulesRowArrowY(0));
    CHECK_EQ(80, (int)mnRemixPanelMenuRulesRowArrowY(2));
}

static void testArrowsSitAtTheCardEdges(void)
{
    CHECK_EQ(27, (int)mnRemixPanelMenuRulesItemArrowX(25.0F, 64, 0));
    CHECK_EQ(84, (int)mnRemixPanelMenuRulesItemArrowX(25.0F, 64, 1));
    CHECK_EQ(69, (int)mnRemixPanelMenuRulesPageArrowX(25.0F, 64, 0));
    CHECK_EQ(84, (int)mnRemixPanelMenuRulesPageArrowX(25.0F, 64, 1));
}

static void testStaminaDamageWrapsOnlyAboveTheHp(void)
{
    CHECK_EQ(150, mnRemixPanelMenuRulesStaminaDamage(150, 1, 199));
    CHECK_EQ(199, mnRemixPanelMenuRulesStaminaDamage(199, -1, 199));
    CHECK_EQ(0, mnRemixPanelMenuRulesStaminaDamage(200, 1, 199));
    CHECK_EQ(199, mnRemixPanelMenuRulesStaminaDamage(999, -1, 199));
}

static void testStockStaysDefaultOutsideTheModesThatUseIt(void)
{
    CHECK_EQ(2, mnRemixPanelMenuRulesStockValue(1, 2));
    CHECK_EQ(0, mnRemixPanelMenuRulesStockValue(0, 2));
}

static MNRemixPanelMenuHighScoreGlobals makeDefaultHighScoreGlobals(void)
{
    MNRemixPanelMenuHighScoreGlobals globals = { { 0, 0 }, MNREMIX_PANELMENU_POKEMON_WORD_DEFAULT, 0 };

    return globals;
}

static void testHighScoresStayOnWhileFlaggedItemsAreDefault(void)
{
    int values[MNREMIX_PANELMENU_ITEM_COUNT] = { 0 };
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    CHECK_EQ(1, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_1P, values, &globals));
    values[MNREMIX_PANELMENU_ITEM_SHIELD] = 5;
    values[MNREMIX_PANELMENU_ITEM_DELAY] = 3;
    CHECK_EQ(1, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_1P, values, &globals));
    values[MNREMIX_PANELMENU_ITEM_SIZE] = 1;
    CHECK_EQ(0, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_1P, values, &globals));
}

static void testHighScoresIgnoreItemsTheModeDoesNotList(void)
{
    int values[MNREMIX_PANELMENU_ITEM_COUNT] = { 0 };
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    values[MNREMIX_PANELMENU_ITEM_PRACTICE] = 4;
    CHECK_EQ(0, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_1P, values, &globals));
    CHECK_EQ(1, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_BONUS, values, &globals));
}

static void testHighScoresTurnOffWithAnEnemyPort(void)
{
    int values[MNREMIX_PANELMENU_ITEM_COUNT] = { 0 };
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    globals.enemy_port = 2;
    CHECK_EQ(0, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_1P, values, &globals));
    globals.enemy_port = 0;
    CHECK_EQ(1, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_1P, values, &globals));
}

static void testToggleBitWidthFollowsTheHighestValue(void)
{
    CHECK_EQ(1, mnRemixPanelMenuRulesToggleBitWidth(0));
    CHECK_EQ(1, mnRemixPanelMenuRulesToggleBitWidth(1));
    CHECK_EQ(2, mnRemixPanelMenuRulesToggleBitWidth(2));
    CHECK_EQ(2, mnRemixPanelMenuRulesToggleBitWidth(3));
    CHECK_EQ(3, mnRemixPanelMenuRulesToggleBitWidth(4));
    CHECK_EQ(3, mnRemixPanelMenuRulesToggleBitWidth(7));
    CHECK_EQ(4, mnRemixPanelMenuRulesToggleBitWidth(11));
    CHECK_EQ(4, mnRemixPanelMenuRulesToggleBitWidth(15));
    CHECK_EQ(5, mnRemixPanelMenuRulesToggleBitWidth(16));
}

/* The first Gameplay toggles of Toggles.asm 2419-2428 (Hitstun .. Improved AI) as (highest value) and the bit each one starts at. */
static void testPackingPutsImprovedAiOnBit20AndJapaneseSoundsOnBits6And7(void)
{
    int maxima[10] = { 1, 4, 2, 2, 1, 1, 4, 4, 11, 1 };
    int values[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 };
    unsigned int words[2];

    mnRemixPanelMenuRulesPackToggles(values, maxima, 10, words, 2);
    CHECK_EQ(1U << 20, words[0]);
    CHECK_EQ(0, words[1]);

    values[9] = 0;
    values[3] = 2;
    mnRemixPanelMenuRulesPackToggles(values, maxima, 10, words, 2);
    CHECK_EQ(1U << 7, words[0]);

    values[3] = 1;
    mnRemixPanelMenuRulesPackToggles(values, maxima, 10, words, 2);
    CHECK_EQ(1U << 6, words[0]);
}

static void testPackingStartsANewWordWhenAToggleEndsOnTheBoundary(void)
{
    int maxima[33];
    int values[33];
    unsigned int words[2];
    int index;

    for (index = 0; index < 33; index++)
    {
        maxima[index] = 1;
        values[index] = 0;
    }
    values[31] = 1;
    values[32] = 1;
    mnRemixPanelMenuRulesPackToggles(values, maxima, 33, words, 2);
    CHECK_EQ(0, words[0]);
    CHECK_EQ(3U, words[1]);
}

static void testPackingDropsTogglesBeyondTheGivenWords(void)
{
    int maxima[40];
    int values[40];
    unsigned int words[1];
    int index;

    for (index = 0; index < 40; index++)
    {
        maxima[index] = 1;
        values[index] = 1;
    }
    mnRemixPanelMenuRulesPackToggles(values, maxima, 40, words, 1);
    CHECK_EQ(0x7FFFFFFFU, words[0]);
}

static void testPokemonDefaultsPackToTheAsmDefaultWord(void)
{
    int maxima[14] = { 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    int values[14] = { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    unsigned int words[1];

    mnRemixPanelMenuRulesPackToggles(values, maxima, 14, words, 1);
    CHECK_EQ(MNREMIX_PANELMENU_POKEMON_WORD_DEFAULT, words[0]);
}

static void testTogglesAtDefaultKeepTheHighScores(void)
{
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    CHECK_EQ(1, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
}

static void testImprovedAiAndJapaneseNeverSoundsDoNotTurnTheHighScoresOff(void)
{
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    globals.gameplay_words[0] = (1U << 20) | (1U << 7);
    CHECK_EQ(1, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
    globals.gameplay_words[0] = 1U << 6;
    CHECK_EQ(0, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
}

static void testAnyOtherGameplayToggleTurnsTheHighScoresOff(void)
{
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    globals.gameplay_words[0] = 1U << 1;
    CHECK_EQ(0, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
    globals.gameplay_words[0] = 0;
    globals.gameplay_words[1] = 1U << 3;
    CHECK_EQ(0, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
}

static void testPokemonSoundsAreIgnoredButEachPokemonCounts(void)
{
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    globals.pokemon_word = MNREMIX_PANELMENU_POKEMON_WORD_DEFAULT | 0x3U;
    CHECK_EQ(1, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
    globals.pokemon_word = MNREMIX_PANELMENU_POKEMON_WORD_DEFAULT & ~(1U << 5);
    CHECK_EQ(0, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
    globals.pokemon_word = MNREMIX_PANELMENU_POKEMON_WORD_DEFAULT | (1U << 15);
    CHECK_EQ(0, mnRemixPanelMenuRulesTogglesKeepHighScores(&globals));
}

static void testHighScoresTurnOffWithATogglePastItsDefault(void)
{
    int values[MNREMIX_PANELMENU_ITEM_COUNT] = { 0 };
    MNRemixPanelMenuHighScoreGlobals globals = makeDefaultHighScoreGlobals();

    globals.gameplay_words[0] = 1U << 9;
    CHECK_EQ(0, mnRemixPanelMenuRulesHighScoresEnabled(MNREMIX_PANELMENU_MODE_BONUS, values, &globals));
}

void remixTestCssPanelMenuRules(void)
{
    RUN_TEST(testVsHumanListsEighteenItemsWithoutHandicapAndPractice);
    RUN_TEST(testVsCpuListsHandicapAndNoHumanOnlyItems);
    RUN_TEST(testTrainingListsHaveNoStockDamagePoisonOrPractice);
    RUN_TEST(testOnePlayerListsPracticeAndIgnoresTheCpuBitmask);
    RUN_TEST(testBonusAndAllStarLists);
    RUN_TEST(testListHonoursTheCapacity);
    RUN_TEST(testResetListJoinsTheHumanAndCpuLists);
    RUN_TEST(testItemRangesAndDefaultsMatchTheItemFiles);
    RUN_TEST(testOnlyDamageIsNumeric);
    RUN_TEST(testLabelsAreTheAsmStrings);
    RUN_TEST(testHighScoreFlagsFollowTheItemFiles);
    RUN_TEST(testMaxPageDividesByThree);
    RUN_TEST(testPageIsClampedWhenTheListShrinks);
    RUN_TEST(testPageArrowsWrapAroundTheEnds);
    RUN_TEST(testRowsOnPageAreTheItemsLeft);
    RUN_TEST(testItemArrowsWrapBetweenMinAndMax);
    RUN_TEST(testBlinkCountsZeroToNineteenAndShowsBelowEleven);
    RUN_TEST(testHoldRepeatsEverySixthFrameAfterFifteenFrames);
    RUN_TEST(testHoldRepeatsEveryFrameAfterNinetyFrames);
    RUN_TEST(testReleasingAResetsTheHoldCounters);
    RUN_TEST(testPageArrowRestartsTheHoldWait);
    RUN_TEST(testRowsAreTwentyFourUnitsApart);
    RUN_TEST(testArrowsSitAtTheCardEdges);
    RUN_TEST(testStaminaDamageWrapsOnlyAboveTheHp);
    RUN_TEST(testStockStaysDefaultOutsideTheModesThatUseIt);
    RUN_TEST(testHighScoresStayOnWhileFlaggedItemsAreDefault);
    RUN_TEST(testHighScoresIgnoreItemsTheModeDoesNotList);
    RUN_TEST(testHighScoresTurnOffWithAnEnemyPort);
    RUN_TEST(testToggleBitWidthFollowsTheHighestValue);
    RUN_TEST(testPackingPutsImprovedAiOnBit20AndJapaneseSoundsOnBits6And7);
    RUN_TEST(testPackingStartsANewWordWhenAToggleEndsOnTheBoundary);
    RUN_TEST(testPackingDropsTogglesBeyondTheGivenWords);
    RUN_TEST(testPokemonDefaultsPackToTheAsmDefaultWord);
    RUN_TEST(testTogglesAtDefaultKeepTheHighScores);
    RUN_TEST(testImprovedAiAndJapaneseNeverSoundsDoNotTurnTheHighScoresOff);
    RUN_TEST(testAnyOtherGameplayToggleTurnsTheHighScoresOff);
    RUN_TEST(testPokemonSoundsAreIgnoredButEachPokemonCounts);
    RUN_TEST(testHighScoresTurnOffWithATogglePastItsDefault);
}
