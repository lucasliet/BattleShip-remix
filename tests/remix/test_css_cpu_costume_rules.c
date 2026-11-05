/**
 * test_css_cpu_costume_rules.c - unit tests of the CPU costume buttons and the Random bookend of the Remix VS character select
 * (mn/mnplayers/mnremixcssrules.c). Expected values come from the Remix asm: Costumes.asm cpu_costumes_, CharacterSelect.asm
 * check_press_, _recent_randoms_lookup and save_random_char_._vs.
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixcssrules.h"

#define PANELS 4

static void testCursorPressesImageAddsTwentyAndThreeToTheCursor(void)
{
    CHECK_EQ(1, mnRemixCssCursorPressesImage(80.0F, 47.0F, 100.0F, 50.0F, 40, 20));
    CHECK_EQ(0, mnRemixCssCursorPressesImage(79.0F, 47.0F, 100.0F, 50.0F, 40, 20));
    CHECK_EQ(0, mnRemixCssCursorPressesImage(80.0F, 46.0F, 100.0F, 50.0F, 40, 20));
}

static void testCursorPressesImageIncludesTheFarEdges(void)
{
    CHECK_EQ(1, mnRemixCssCursorPressesImage(120.0F, 67.0F, 100.0F, 50.0F, 40, 20));
    CHECK_EQ(0, mnRemixCssCursorPressesImage(121.0F, 67.0F, 100.0F, 50.0F, 40, 20));
    CHECK_EQ(0, mnRemixCssCursorPressesImage(120.0F, 68.0F, 100.0F, 50.0F, 40, 20));
}

static void testCpuCostumePanelIsTheCpuPanelUnderTheCursor(void)
{
    const int is_cpu[PANELS] = { 0, 1, 1, 0 };
    const int over_second_cpu[PANELS] = { 0, 0, 1, 0 };

    CHECK_EQ(2, mnRemixCssCpuCostumePanel(0, is_cpu, over_second_cpu, PANELS));
}

static void testCpuCostumePanelTakesTheFirstCpuPanelInPortOrder(void)
{
    const int is_cpu[PANELS] = { 0, 1, 1, 0 };
    const int over_both[PANELS] = { 0, 1, 1, 0 };

    CHECK_EQ(1, mnRemixCssCpuCostumePanel(0, is_cpu, over_both, PANELS));
}

static void testCpuCostumePanelSkipsTheOwnPanel(void)
{
    const int is_cpu[PANELS] = { 0, 1, 1, 0 };
    const int over_own_only[PANELS] = { 0, 1, 0, 0 };

    CHECK_EQ(1, mnRemixCssCpuCostumePanel(1, is_cpu, over_own_only, PANELS));
}

static void testCpuCostumePanelSkipsHumanPanels(void)
{
    const int is_cpu[PANELS] = { 0, 1, 0, 1 };
    const int over_human_and_cpu[PANELS] = { 0, 0, 1, 1 };
    const int over_human_only[PANELS] = { 0, 0, 1, 0 };

    CHECK_EQ(3, mnRemixCssCpuCostumePanel(0, is_cpu, over_human_and_cpu, PANELS));
    CHECK_EQ(0, mnRemixCssCpuCostumePanel(0, is_cpu, over_human_only, PANELS));
}

static void testCpuCostumePanelIsTheOwnPanelWhenNothingIsUnderTheCursor(void)
{
    const int is_cpu[PANELS] = { 0, 1, 1, 0 };
    const int over_nothing[PANELS] = { 0, 0, 0, 0 };

    CHECK_EQ(2, mnRemixCssCpuCostumePanel(2, is_cpu, over_nothing, PANELS));
}

static void testRandomBookendEdges(void)
{
    CHECK_EQ(1, mnRemixCssRandomBookendContains(279, 44));
    CHECK_EQ(1, mnRemixCssRandomBookendContains(293, 115));
    CHECK_EQ(0, mnRemixCssRandomBookendContains(278, 44));
    CHECK_EQ(0, mnRemixCssRandomBookendContains(294, 44));
    CHECK_EQ(0, mnRemixCssRandomBookendContains(279, 43));
    CHECK_EQ(0, mnRemixCssRandomBookendContains(279, 116));
}

static void testRandomBookendDoesNotOverlapTheBonusBookend(void)
{
    CHECK_EQ(0, mnRemixCssRandomBookendContains(30, 80));
    CHECK_EQ(0, mnRemixCssBonusBookendContains(285, 80));
}

static void testRandomTokenSitsInsideItsBookend(void)
{
    CHECK_EQ(1, mnRemixCssRandomBookendContains((int)MNREMIX_CSS_RANDOM_TOKEN_X + 13, (int)MNREMIX_CSS_RANDOM_TOKEN_Y + 12));
}

static void testRecentRandomsStartEmpty(void)
{
    MNRemixCssRecentRandoms recent;
    int slot;

    mnRemixCssRecentRandomsReset(&recent);

    for (slot = 0; slot < MNREMIX_CSS_RECENT_RANDOMS_SLOTS; slot++)
    {
        CHECK_EQ(-1, recent.entries[slot]);
    }
    CHECK_EQ(0, recent.index);
}

static void testNewFighterNeedsNoDice(void)
{
    MNRemixCssRecentRandoms recent;
    int matches;

    mnRemixCssRecentRandomsReset(&recent);
    matches = mnRemixCssRecentRandomsStore(&recent, 5);

    CHECK_EQ(1, matches);
    CHECK_EQ(0, mnRemixCssRecentRandomsDiceRange(matches));
    CHECK_EQ(5, recent.entries[0]);
}

static void testKeptRollMovesTheIndexAndWrapsAfterTheSixthSlot(void)
{
    MNRemixCssRecentRandoms recent;
    int roll;

    mnRemixCssRecentRandomsReset(&recent);

    for (roll = 0; roll < MNREMIX_CSS_RECENT_RANDOMS_SLOTS; roll++)
    {
        CHECK_EQ(roll, recent.index);
        mnRemixCssRecentRandomsStore(&recent, 10 + roll);
        mnRemixCssRecentRandomsCommit(&recent);
    }
    CHECK_EQ(0, recent.index);
}

static void testRepeatedFighterRollsADiceOfItsMatchCount(void)
{
    MNRemixCssRecentRandoms recent;
    int matches;

    mnRemixCssRecentRandomsReset(&recent);
    mnRemixCssRecentRandomsStore(&recent, 5);
    mnRemixCssRecentRandomsCommit(&recent);

    matches = mnRemixCssRecentRandomsStore(&recent, 5);
    CHECK_EQ(2, matches);
    CHECK_EQ(2, mnRemixCssRecentRandomsDiceRange(matches));

    mnRemixCssRecentRandomsCommit(&recent);
    matches = mnRemixCssRecentRandomsStore(&recent, 5);
    CHECK_EQ(3, matches);
    CHECK_EQ(3, mnRemixCssRecentRandomsDiceRange(matches));
}

static void testRejectedRollIsOverwrittenByTheNextOne(void)
{
    MNRemixCssRecentRandoms recent;

    mnRemixCssRecentRandomsReset(&recent);
    mnRemixCssRecentRandomsStore(&recent, 5);
    mnRemixCssRecentRandomsCommit(&recent);
    mnRemixCssRecentRandomsStore(&recent, 5);

    CHECK_EQ(5, recent.entries[1]);
    CHECK_EQ(1, mnRemixCssRecentRandomsStore(&recent, 9));
    CHECK_EQ(9, recent.entries[1]);
    CHECK_EQ(1, recent.index);
}

static void testTeamShadeCountsEarlierTeammatesWithTheSameFighter(void)
{
    const int fkinds[4] = { 3, 3, 3, 7 };
    const int teams[4] = { 0, 0, 1, 0 };

    CHECK_EQ(0, mnRemixCssRandomTeamShade(0, fkinds, teams));
    CHECK_EQ(1, mnRemixCssRandomTeamShade(1, fkinds, teams));
    CHECK_EQ(0, mnRemixCssRandomTeamShade(2, fkinds, teams));
    CHECK_EQ(0, mnRemixCssRandomTeamShade(3, fkinds, teams));
}

static void testTeamShadeGrowsWithEachEarlierMatch(void)
{
    const int fkinds[4] = { 3, 3, 3, 3 };
    const int teams[4] = { 1, 1, 1, 1 };

    CHECK_EQ(2, mnRemixCssRandomTeamShade(2, fkinds, teams));
    CHECK_EQ(3, mnRemixCssRandomTeamShade(3, fkinds, teams));
}

void remixTestCssCpuCostumeRules(void)
{
    RUN_TEST(testCursorPressesImageAddsTwentyAndThreeToTheCursor);
    RUN_TEST(testCursorPressesImageIncludesTheFarEdges);
    RUN_TEST(testCpuCostumePanelIsTheCpuPanelUnderTheCursor);
    RUN_TEST(testCpuCostumePanelTakesTheFirstCpuPanelInPortOrder);
    RUN_TEST(testCpuCostumePanelSkipsTheOwnPanel);
    RUN_TEST(testCpuCostumePanelSkipsHumanPanels);
    RUN_TEST(testCpuCostumePanelIsTheOwnPanelWhenNothingIsUnderTheCursor);
    RUN_TEST(testRandomBookendEdges);
    RUN_TEST(testRandomBookendDoesNotOverlapTheBonusBookend);
    RUN_TEST(testRandomTokenSitsInsideItsBookend);
    RUN_TEST(testRecentRandomsStartEmpty);
    RUN_TEST(testNewFighterNeedsNoDice);
    RUN_TEST(testKeptRollMovesTheIndexAndWrapsAfterTheSixthSlot);
    RUN_TEST(testRepeatedFighterRollsADiceOfItsMatchCount);
    RUN_TEST(testRejectedRollIsOverwrittenByTheNextOne);
    RUN_TEST(testTeamShadeCountsEarlierTeammatesWithTheSameFighter);
    RUN_TEST(testTeamShadeGrowsWithEachEarlierMatch);
}
