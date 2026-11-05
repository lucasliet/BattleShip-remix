/**
 * test_stage_grid_rules.c - unit tests of the Remix stage select grid (mn/mnmaps/mnremixstagegridrules.c), of the list the RANDOM
 * cell draws from (mnremixstagerandomrules.c) and of the generated Random Stage Toggle table. Expected values come from the asm
 * of src/Stages.asm (NUM_ROWS 3, NUM_COLUMNS 6, ICON_WIDTH 40, ICON_HEIGHT 30, right_, left_, down_, up_, right_check_,
 * random_stage_roulette, add_stage_to_random_list_), not from the code under test.
 */

#include <stdint.h>

#include "test_check.h"

#include "mn/mnmaps/mnremixstagegridrules.h"
#include "mn/mnmaps/mnremixstagerandomrules.h"
#include "remix/remix_symbols.h"

typedef struct StageToggleRow
{
    unsigned int stage_id;
    uint32_t entry_addr;

} StageToggleRow;

static const StageToggleRow sStageToggleRows[] =
{
#include "mn/mnmaps/mnremixstagerandomtable.inc"
};

static void testGridHasThreeRowsOfSixIcons(void)
{
    CHECK_EQ(6, MNREMIX_GRID_COLUMNS);
    CHECK_EQ(3, MNREMIX_GRID_ROWS);
    CHECK_EQ(18, MNREMIX_GRID_CELLS);
    CHECK_EQ(5, MNREMIX_GRID_PAGES_MAX);
}

static void testCellIsRowTimesColumnsPlusColumn(void)
{
    CHECK_EQ(0, mnRemixStageGridRulesGetCell(0, 0));
    CHECK_EQ(5, mnRemixStageGridRulesGetCell(0, 5));
    CHECK_EQ(6, mnRemixStageGridRulesGetCell(1, 0));
    CHECK_EQ(10, mnRemixStageGridRulesGetCell(1, 4));
    CHECK_EQ(17, mnRemixStageGridRulesGetCell(2, 5));
}

static void testRowAndColumnInvertTheCell(void)
{
    int cell;

    for (cell = 0; cell < MNREMIX_GRID_CELLS; cell++)
    {
        CHECK_EQ(cell, mnRemixStageGridRulesGetCell(mnRemixStageGridRulesGetRow(cell), mnRemixStageGridRulesGetColumn(cell)));
    }
    CHECK_EQ(2, mnRemixStageGridRulesGetRow(14));
    CHECK_EQ(2, mnRemixStageGridRulesGetColumn(14));
}

static void testRightMovesOneColumnAndWrapsInsideTheRow(void)
{
    CHECK_EQ(1, mnRemixStageGridRulesMoveRight(0));
    CHECK_EQ(5, mnRemixStageGridRulesMoveRight(4));
    CHECK_EQ(0, mnRemixStageGridRulesMoveRight(5));
    CHECK_EQ(6, mnRemixStageGridRulesMoveRight(11));
    CHECK_EQ(12, mnRemixStageGridRulesMoveRight(17));
}

static void testLeftMovesOneColumnAndWrapsInsideTheRow(void)
{
    CHECK_EQ(0, mnRemixStageGridRulesMoveLeft(1));
    CHECK_EQ(5, mnRemixStageGridRulesMoveLeft(0));
    CHECK_EQ(11, mnRemixStageGridRulesMoveLeft(6));
    CHECK_EQ(17, mnRemixStageGridRulesMoveLeft(12));
    CHECK_EQ(6, mnRemixStageGridRulesMoveLeft(7));
}

static void testDownMovesOneRowAndWrapsInsideTheColumn(void)
{
    CHECK_EQ(6, mnRemixStageGridRulesMoveDown(0));
    CHECK_EQ(12, mnRemixStageGridRulesMoveDown(6));
    CHECK_EQ(0, mnRemixStageGridRulesMoveDown(12));
    CHECK_EQ(5, mnRemixStageGridRulesMoveDown(17));
    CHECK_EQ(10, mnRemixStageGridRulesMoveDown(4));
}

static void testUpMovesOneRowAndWrapsInsideTheColumn(void)
{
    CHECK_EQ(0, mnRemixStageGridRulesMoveUp(6));
    CHECK_EQ(6, mnRemixStageGridRulesMoveUp(12));
    CHECK_EQ(12, mnRemixStageGridRulesMoveUp(0));
    CHECK_EQ(17, mnRemixStageGridRulesMoveUp(5));
    CHECK_EQ(4, mnRemixStageGridRulesMoveUp(10));
}

static void testFourMovesInOneDirectionComeBack(void)
{
    int cell;

    for (cell = 0; cell < MNREMIX_GRID_CELLS; cell++)
    {
        int right = cell;
        int down = cell;
        int step;

        for (step = 0; step < MNREMIX_GRID_COLUMNS; step++)
        {
            right = mnRemixStageGridRulesMoveRight(right);
        }
        for (step = 0; step < MNREMIX_GRID_ROWS; step++)
        {
            down = mnRemixStageGridRulesMoveDown(down);
        }
        CHECK_EQ(cell, right);
        CHECK_EQ(cell, down);
    }
}

static void testRSwitchesToTheNextPageAndWrapsToTheFirst(void)
{
    CHECK_EQ(1, mnRemixStageGridRulesNextPage(0, 5));
    CHECK_EQ(4, mnRemixStageGridRulesNextPage(3, 5));
    CHECK_EQ(0, mnRemixStageGridRulesNextPage(4, 5));
    CHECK_EQ(0, mnRemixStageGridRulesNextPage(2, 3));
    CHECK_EQ(0, mnRemixStageGridRulesNextPage(0, 1));
}

static void testZSwitchesToThePreviousPageAndWrapsToTheLast(void)
{
    CHECK_EQ(0, mnRemixStageGridRulesPreviousPage(1, 5));
    CHECK_EQ(3, mnRemixStageGridRulesPreviousPage(4, 5));
    CHECK_EQ(4, mnRemixStageGridRulesPreviousPage(0, 5));
    CHECK_EQ(2, mnRemixStageGridRulesPreviousPage(0, 3));
    CHECK_EQ(0, mnRemixStageGridRulesPreviousPage(0, 1));
}

static void testPageBeyondTheLayoutGoesBackToTheFirst(void)
{
    CHECK_EQ(4, mnRemixStageGridRulesClampPage(4, 5));
    CHECK_EQ(2, mnRemixStageGridRulesClampPage(2, 3));
    CHECK_EQ(0, mnRemixStageGridRulesClampPage(3, 3));
    CHECK_EQ(0, mnRemixStageGridRulesClampPage(4, 3));
    CHECK_EQ(0, mnRemixStageGridRulesClampPage(0, 3));
}

static void testStageTableIndexAddsOnePageOfCellsPerPage(void)
{
    CHECK_EQ(0, mnRemixStageGridRulesGetTableIndex(0, 0));
    CHECK_EQ(17, mnRemixStageGridRulesGetTableIndex(0, 17));
    CHECK_EQ(18, mnRemixStageGridRulesGetTableIndex(1, 0));
    CHECK_EQ(35, mnRemixStageGridRulesGetTableIndex(1, 17));
    CHECK_EQ(89, mnRemixStageGridRulesGetTableIndex(4, 17));
}

static void testRouletteRerollsOnlyTheBottomRightCell(void)
{
    int row;
    int column;
    int rerolled = 0;

    for (row = 0; row < MNREMIX_GRID_ROWS; row++)
    {
        for (column = 0; column < MNREMIX_GRID_COLUMNS; column++)
        {
            rerolled += mnRemixStageGridRulesRouletteRerolls(row, column);
        }
    }
    CHECK_EQ(1, rerolled);
    CHECK_EQ(1, mnRemixStageGridRulesRouletteRerolls(2, 5));
    CHECK_EQ(0, mnRemixStageGridRulesRouletteRerolls(2, 4));
    CHECK_EQ(0, mnRemixStageGridRulesRouletteRerolls(1, 5));
}

static void testIconsStartAt30And20WithStep42And32(void)
{
    CHECK_EQ(30, mnRemixStageGridRulesGetIconX(0));
    CHECK_EQ(72, mnRemixStageGridRulesGetIconX(1));
    CHECK_EQ(240, mnRemixStageGridRulesGetIconX(5));
    CHECK_EQ(20, mnRemixStageGridRulesGetIconY(0));
    CHECK_EQ(52, mnRemixStageGridRulesGetIconY(1));
    CHECK_EQ(84, mnRemixStageGridRulesGetIconY(2));
}

static void testCursorSitsSixPixelsUpAndLeftOfTheIcon(void)
{
    int index;

    CHECK_EQ(24, mnRemixStageGridRulesGetCursorX(0));
    CHECK_EQ(66, mnRemixStageGridRulesGetCursorX(1));
    CHECK_EQ(234, mnRemixStageGridRulesGetCursorX(5));
    CHECK_EQ(14, mnRemixStageGridRulesGetCursorY(0));
    CHECK_EQ(46, mnRemixStageGridRulesGetCursorY(1));
    CHECK_EQ(78, mnRemixStageGridRulesGetCursorY(2));
    for (index = 0; index < MNREMIX_GRID_COLUMNS; index++)
    {
        CHECK_EQ(6, mnRemixStageGridRulesGetIconX(index) - mnRemixStageGridRulesGetCursorX(index));
    }
    for (index = 0; index < MNREMIX_GRID_ROWS; index++)
    {
        CHECK_EQ(6, mnRemixStageGridRulesGetIconY(index) - mnRemixStageGridRulesGetCursorY(index));
    }
}

static void testPageNumberIsOneBased(void)
{
    CHECK_EQ(1, mnRemixStageGridRulesGetPageNumber(0));
    CHECK_EQ(5, mnRemixStageGridRulesGetPageNumber(4));
}

static void testRandomListKeepsTheTogglesThatAreOnInOrder(void)
{
    MNRemixStageRandomEntry entries[] = { { 0x00, 1, 1 }, { 0x01, 0, 1 }, { 0x2A, 1, 0 }, { 0x2B, 0, 0 }, { 0x30, 1, 1 } };
    int list[5];

    CHECK_EQ(3, mnRemixStageRandomRulesBuildList(entries, 5, 0, list));
    CHECK_EQ(0x00, list[0]);
    CHECK_EQ(0x2A, list[1]);
    CHECK_EQ(0x30, list[2]);
}

static void testTournamentLayoutKeepsTheLegalStagesOnly(void)
{
    MNRemixStageRandomEntry entries[] = { { 0x00, 1, 1 }, { 0x01, 1, 0 }, { 0x2A, 1, 0 }, { 0x30, 1, 1 } };
    int list[4];

    CHECK_EQ(2, mnRemixStageRandomRulesBuildList(entries, 4, 1, list));
    CHECK_EQ(0x00, list[0]);
    CHECK_EQ(0x30, list[1]);
}

static void testListWithEveryToggleOffFallsBackToAllStages(void)
{
    MNRemixStageRandomEntry entries[] = { { 0x00, 0, 1 }, { 0x01, 0, 0 }, { 0x2A, 0, 1 } };
    int list[3];

    CHECK_EQ(3, mnRemixStageRandomRulesBuildList(entries, 3, 0, list));
    CHECK_EQ(0x00, list[0]);
    CHECK_EQ(0x01, list[1]);
    CHECK_EQ(0x2A, list[2]);
}

static void testFallbackStillFiltersTheTournamentLegality(void)
{
    MNRemixStageRandomEntry entries[] = { { 0x00, 0, 1 }, { 0x01, 0, 0 }, { 0x2A, 0, 1 } };
    int list[3];

    CHECK_EQ(2, mnRemixStageRandomRulesBuildList(entries, 3, 1, list));
    CHECK_EQ(0x00, list[0]);
    CHECK_EQ(0x2A, list[1]);
}

static void testListIsEmptyWhenNothingIsLegal(void)
{
    MNRemixStageRandomEntry entries[] = { { 0x00, 1, 0 }, { 0x01, 0, 0 } };
    int list[2];

    CHECK_EQ(0, mnRemixStageRandomRulesBuildList(entries, 2, 1, list));
    CHECK_EQ(0, mnRemixStageRandomRulesBuildList(entries, 0, 0, list));
}

static void testGeneratedToggleTableHoldsEveryTogglableCustomStageOnce(void)
{
    int count = (int)(sizeof(sStageToggleRows) / sizeof(sStageToggleRows[0]));
    int seen[0xD8] = { 0 };
    int index;

    CHECK_EQ(128, count);
    for (index = 0; index < count; index++)
    {
        CHECK_TRUE((sStageToggleRows[index].stage_id >= 0x29) && (sStageToggleRows[index].stage_id <= 0xD7));
        CHECK_EQ(0, seen[sStageToggleRows[index].stage_id]);
        seen[sStageToggleRows[index].stage_id] = 1;
    }
}

static void testGeneratedToggleTableFollowsTheMenuEntryChain(void)
{
    int count = (int)(sizeof(sStageToggleRows) / sizeof(sStageToggleRows[0]));
    int index;

    for (index = 1; index < count; index++)
    {
        CHECK_TRUE(sStageToggleRows[index].entry_addr > sStageToggleRows[index - 1].entry_addr);
    }
    CHECK_TRUE(sStageToggleRows[0].entry_addr >= REMIX_SYM_Toggles__entry_random_stage_first_custom);
}

void remixTestStageGridRules(void)
{
    RUN_TEST(testGridHasThreeRowsOfSixIcons);
    RUN_TEST(testCellIsRowTimesColumnsPlusColumn);
    RUN_TEST(testRowAndColumnInvertTheCell);
    RUN_TEST(testRightMovesOneColumnAndWrapsInsideTheRow);
    RUN_TEST(testLeftMovesOneColumnAndWrapsInsideTheRow);
    RUN_TEST(testDownMovesOneRowAndWrapsInsideTheColumn);
    RUN_TEST(testUpMovesOneRowAndWrapsInsideTheColumn);
    RUN_TEST(testFourMovesInOneDirectionComeBack);
    RUN_TEST(testRSwitchesToTheNextPageAndWrapsToTheFirst);
    RUN_TEST(testZSwitchesToThePreviousPageAndWrapsToTheLast);
    RUN_TEST(testPageBeyondTheLayoutGoesBackToTheFirst);
    RUN_TEST(testStageTableIndexAddsOnePageOfCellsPerPage);
    RUN_TEST(testRouletteRerollsOnlyTheBottomRightCell);
    RUN_TEST(testIconsStartAt30And20WithStep42And32);
    RUN_TEST(testCursorSitsSixPixelsUpAndLeftOfTheIcon);
    RUN_TEST(testPageNumberIsOneBased);
    RUN_TEST(testRandomListKeepsTheTogglesThatAreOnInOrder);
    RUN_TEST(testTournamentLayoutKeepsTheLegalStagesOnly);
    RUN_TEST(testListWithEveryToggleOffFallsBackToAllStages);
    RUN_TEST(testFallbackStillFiltersTheTournamentLegality);
    RUN_TEST(testListIsEmptyWhenNothingIsLegal);
    RUN_TEST(testGeneratedToggleTableHoldsEveryTogglableCustomStageOnce);
    RUN_TEST(testGeneratedToggleTableFollowsTheMenuEntryChain);
}
