/**
 * test_training_menu_rules.c - unit tests of the Training Z menu navigation rules (sc/sc1pmode/sc1ptrainingmenurules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1ptrainingmenurules.h"

#define ENTRY_COUNT 23

static void testStickStepsOnTheFirstFrameThenRepeats(void)
{
    CHECK_EQ(0, sc1PTrainingMenuRulesIsStickStep(0));
    CHECK_EQ(1, sc1PTrainingMenuRulesIsStickStep(1));
    CHECK_EQ(0, sc1PTrainingMenuRulesIsStickStep(2));
    CHECK_EQ(0, sc1PTrainingMenuRulesIsStickStep(27));
    CHECK_EQ(1, sc1PTrainingMenuRulesIsStickStep(28));
    CHECK_EQ(0, sc1PTrainingMenuRulesIsStickStep(29));
    CHECK_EQ(0, sc1PTrainingMenuRulesIsStickStep(31));
    CHECK_EQ(1, sc1PTrainingMenuRulesIsStickStep(32));
    CHECK_EQ(1, sc1PTrainingMenuRulesIsStickStep(100));
}

static void testPageAndRow(void)
{
    CHECK_EQ(0, sc1PTrainingMenuRulesGetPage(11));
    CHECK_EQ(1, sc1PTrainingMenuRulesGetPage(12));
    CHECK_EQ(11, sc1PTrainingMenuRulesGetRow(11));
    CHECK_EQ(0, sc1PTrainingMenuRulesGetRow(12));
    CHECK_EQ(10, sc1PTrainingMenuRulesGetRow(22));
}

static void testDownMovesWithinThePageAndWrapsToItsTop(void)
{
    CHECK_EQ(1, sc1PTrainingMenuRulesMoveDown(0, ENTRY_COUNT));
    CHECK_EQ(0, sc1PTrainingMenuRulesMoveDown(11, ENTRY_COUNT));
    CHECK_EQ(13, sc1PTrainingMenuRulesMoveDown(12, ENTRY_COUNT));
}

static void testDownFromTheLastEntryGoesToTheTopOfItsPage(void)
{
    CHECK_EQ(12, sc1PTrainingMenuRulesMoveDown(22, ENTRY_COUNT));
}

static void testDownOnASinglePageWrapsToTheFirstEntry(void)
{
    CHECK_EQ(0, sc1PTrainingMenuRulesMoveDown(4, 5));
    CHECK_EQ(0, sc1PTrainingMenuRulesMoveDown(11, 12));
}

static void testUpMovesWithinThePageAndWrapsToItsBottom(void)
{
    CHECK_EQ(4, sc1PTrainingMenuRulesMoveUp(5, ENTRY_COUNT));
    CHECK_EQ(11, sc1PTrainingMenuRulesMoveUp(0, ENTRY_COUNT));
    CHECK_EQ(22, sc1PTrainingMenuRulesMoveUp(12, ENTRY_COUNT));
    CHECK_EQ(4, sc1PTrainingMenuRulesMoveUp(0, 5));
}

static void testNextPageKeepsTheRowAndWrapsToTheFirstPage(void)
{
    CHECK_EQ(15, sc1PTrainingMenuRulesNextPage(3, ENTRY_COUNT));
    CHECK_EQ(3, sc1PTrainingMenuRulesNextPage(15, ENTRY_COUNT));
}

static void testNextPageStaysInsideAShortLastPage(void)
{
    CHECK_EQ(22, sc1PTrainingMenuRulesNextPage(11, ENTRY_COUNT));
    CHECK_EQ(10, sc1PTrainingMenuRulesNextPage(22, ENTRY_COUNT));
}

static void testPrevPageKeepsTheRowAndWrapsToTheLastPage(void)
{
    CHECK_EQ(3, sc1PTrainingMenuRulesPrevPage(15, ENTRY_COUNT));
    CHECK_EQ(15, sc1PTrainingMenuRulesPrevPage(3, ENTRY_COUNT));
    CHECK_EQ(12, sc1PTrainingMenuRulesPrevPage(0, ENTRY_COUNT));
}

static void testPrevPageStaysInsideAShortLastPage(void)
{
    CHECK_EQ(22, sc1PTrainingMenuRulesPrevPage(11, ENTRY_COUNT));
}

static void testPrevPageKeepsTheSelectionOnASinglePage(void)
{
    CHECK_EQ(2, sc1PTrainingMenuRulesPrevPage(2, 5));
}

static void testStepValueWrapsAroundTheRange(void)
{
    CHECK_EQ(3, sc1PTrainingMenuRulesStepValue(2, 0, 5, SC1PTRAININGMENURULES_STEP_RIGHT));
    CHECK_EQ(0, sc1PTrainingMenuRulesStepValue(5, 0, 5, SC1PTRAININGMENURULES_STEP_RIGHT));
    CHECK_EQ(1, sc1PTrainingMenuRulesStepValue(2, 0, 5, SC1PTRAININGMENURULES_STEP_LEFT));
    CHECK_EQ(5, sc1PTrainingMenuRulesStepValue(0, 0, 5, SC1PTRAININGMENURULES_STEP_LEFT));
}

static void testStepValueRespectsAMinimumAboveZero(void)
{
    CHECK_EQ(99, sc1PTrainingMenuRulesStepValue(1, 1, 99, SC1PTRAININGMENURULES_STEP_LEFT));
    CHECK_EQ(1, sc1PTrainingMenuRulesStepValue(99, 1, 99, SC1PTRAININGMENURULES_STEP_RIGHT));
}

static void testStepValueOnATitleKeepsZero(void)
{
    CHECK_EQ(0, sc1PTrainingMenuRulesStepValue(0, 0, 0, SC1PTRAININGMENURULES_STEP_RIGHT));
    CHECK_EQ(0, sc1PTrainingMenuRulesStepValue(0, 0, 0, SC1PTRAININGMENURULES_STEP_LEFT));
}

static void testClampLowersOnlyAValueAboveTheMaximum(void)
{
    CHECK_EQ(3, sc1PTrainingMenuRulesClampValue(6, 3));
    CHECK_EQ(2, sc1PTrainingMenuRulesClampValue(2, 3));
}

void remixTestTrainingMenuRules(void)
{
    printf("training Z menu navigation rules\n");
    RUN_TEST(testStickStepsOnTheFirstFrameThenRepeats);
    RUN_TEST(testPageAndRow);
    RUN_TEST(testDownMovesWithinThePageAndWrapsToItsTop);
    RUN_TEST(testDownFromTheLastEntryGoesToTheTopOfItsPage);
    RUN_TEST(testDownOnASinglePageWrapsToTheFirstEntry);
    RUN_TEST(testUpMovesWithinThePageAndWrapsToItsBottom);
    RUN_TEST(testNextPageKeepsTheRowAndWrapsToTheFirstPage);
    RUN_TEST(testNextPageStaysInsideAShortLastPage);
    RUN_TEST(testPrevPageKeepsTheRowAndWrapsToTheLastPage);
    RUN_TEST(testPrevPageStaysInsideAShortLastPage);
    RUN_TEST(testPrevPageKeepsTheSelectionOnASinglePage);
    RUN_TEST(testStepValueWrapsAroundTheRange);
    RUN_TEST(testStepValueRespectsAMinimumAboveZero);
    RUN_TEST(testStepValueOnATitleKeepsZero);
    RUN_TEST(testClampLowersOnlyAValueAboveTheMaximum);
}
