/**
 * test_css_vsmodes_rules.c - unit tests of the Remix VS modes on the VS character select: the header and announcer of each
 * mode (mn/mnplayers/mnremixcssmoderules.c), the goal picker and its Time toggle (mnremixgoalrules.c) and the slide-in of the
 * portraits (mnremixcsssliderules.c). Expected values come from the Remix asm: TwelveCharBattle.asm update_css_header_ and
 * update_announcer_on_entry_, Smashketball.asm prevent_start_unless_two_teams_, VsRemixMenu.asm change_time_options,
 * CharacterSelect.asm check_picker_press_ and draw_portraits_. The Tag Team CSS queue (mnremixtagteamcssrules.c) follows
 * TagTeam.asm css.update_slot_, move_to_next_slot_, clear_slot_and_go_to_previous_slot_,
 * adjust_current_slot_on_stock_count_change_ and prevent_start_with_empty_slots_.
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixcssmoderules.h"
#include "mn/mnplayers/mnremixcsssliderules.h"
#include "mn/mnplayers/mnremixgoalrules.h"
#include "mn/mnplayers/mnremixtagteamcssrules.h"
#include "mn/mnvsmode/mnremixvsmodereturnrules.h"

static void testTitleOffsetPerMode(void)
{
    CHECK_EQ(0, mnRemixCssModeRulesTitleOffset(0, 0));
    CHECK_EQ(0x2048, mnRemixCssModeRulesTitleOffset(1, 0));
    CHECK_EQ(0x2738, mnRemixCssModeRulesTitleOffset(2, 0));
    CHECK_EQ(0x29A8, mnRemixCssModeRulesTitleOffset(3, 0));
    CHECK_EQ(0x2C18, mnRemixCssModeRulesTitleOffset(4, 0));
    CHECK_EQ(0x30F0, mnRemixCssModeRulesTitleOffset(5, 0));
}

static void testSmashketballTitleFollowsTheType(void)
{
    CHECK_EQ(0x2C18, mnRemixCssModeRulesTitleOffset(4, 0));
    CHECK_EQ(0x2E88, mnRemixCssModeRulesTitleOffset(4, 1));
    CHECK_EQ(0x29A8, mnRemixCssModeRulesTitleOffset(3, 1));
}

static void testTitleOffsetOutOfRangeKeepsTheVanillaHeader(void)
{
    CHECK_EQ(0, mnRemixCssModeRulesTitleOffset(-1, 0));
    CHECK_EQ(0, mnRemixCssModeRulesTitleOffset(MNREMIX_CSS_MODE_COUNT, 0));
}

static void testAnnouncerPerMode(void)
{
    CHECK_EQ(0, mnRemixCssModeRulesAnnouncerFgm(0));
    CHECK_EQ(1016, mnRemixCssModeRulesAnnouncerFgm(1));
    CHECK_EQ(783, mnRemixCssModeRulesAnnouncerFgm(2));
    CHECK_EQ(775, mnRemixCssModeRulesAnnouncerFgm(3));
    CHECK_EQ(776, mnRemixCssModeRulesAnnouncerFgm(4));
    CHECK_EQ(479, mnRemixCssModeRulesAnnouncerFgm(5));
    CHECK_EQ(0, mnRemixCssModeRulesAnnouncerFgm(6));
}

static void testTwoTeamsNeedsExactlyTwoTeamsPresent(void)
{
    const int teams[4] = { 0, 1, 0, 1 };
    const int all_present[4] = { 1, 1, 1, 1 };
    const int only_red[4] = { 1, 0, 1, 0 };
    const int three_teams[4] = { 0, 1, 2, 0 };

    CHECK_EQ(1, mnRemixCssModeRulesHasTwoTeams(teams, all_present, 4));
    CHECK_EQ(0, mnRemixCssModeRulesHasTwoTeams(teams, only_red, 4));
    CHECK_EQ(0, mnRemixCssModeRulesHasTwoTeams(three_teams, all_present, 4));
}

static void testTwoTeamsIgnoresPanelsThatAreNotInPlay(void)
{
    const int teams[4] = { 0, 1, 2, 3 };
    const int present[4] = { 1, 1, 0, 0 };
    const int none[4] = { 0, 0, 0, 0 };

    CHECK_EQ(1, mnRemixCssModeRulesHasTwoTeams(teams, present, 4));
    CHECK_EQ(0, mnRemixCssModeRulesHasTwoTeams(teams, none, 4));
}

static void testTwoTeamsIgnoresATeamOutOfRange(void)
{
    const int teams[4] = { 0, 1, 7, -1 };
    const int present[4] = { 1, 0, 1, 1 };

    CHECK_EQ(0, mnRemixCssModeRulesHasTwoTeams(teams, present, 4));
}

static void testTimeValuesAreTheRemixOptions(void)
{
    const int *values = mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_TIME);

    CHECK_EQ(1, values[0]);
    CHECK_EQ(8, values[4]);
    CHECK_EQ(12, values[6]);
    CHECK_EQ(99, values[11]);
    CHECK_EQ(100, values[12]);
}

static void testGoalTablesAreSorted(void)
{
    int table;
    int i;

    for (table = MNREMIX_GOAL_TABLE_TIME; table <= MNREMIX_GOAL_TABLE_SMASHKETBALL; table++)
    {
        const int *values = mnRemixGoalRulesValues(table);

        for (i = 1; i < MNREMIX_GOAL_VALUE_COUNT; i++)
        {
            CHECK_TRUE(values[i - 1] < values[i]);
        }
    }
    CHECK_TRUE(mnRemixGoalRulesValues(3) == 0);
}

static void testNextTakesTheFirstValueAboveAndWraps(void)
{
    const int *koth = mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_KOTH);

    CHECK_EQ(10, mnRemixGoalRulesNext(koth, 5));
    CHECK_EQ(45, mnRemixGoalRulesNext(koth, 30));
    CHECK_EQ(60, mnRemixGoalRulesNext(koth, 46));
    CHECK_EQ(5, mnRemixGoalRulesNext(koth, 960));
    CHECK_EQ(5, mnRemixGoalRulesNext(koth, 5000));
}

static void testPrevTakesTheLastValueBelowAndWraps(void)
{
    const int *smashketball = mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_SMASHKETBALL);

    CHECK_EQ(7, mnRemixGoalRulesPrev(smashketball, 10));
    CHECK_EQ(10, mnRemixGoalRulesPrev(smashketball, 11));
    CHECK_EQ(100, mnRemixGoalRulesPrev(smashketball, 1));
    CHECK_EQ(100, mnRemixGoalRulesPrev(smashketball, 0));
}

static void testNumberMovesRightFromThreeDigits(void)
{
    CHECK_EQ(212, (int)mnRemixGoalRulesNumberX(99));
    CHECK_EQ(218, (int)mnRemixGoalRulesNumberX(100));
    CHECK_EQ(218, (int)mnRemixGoalRulesNumberX(960));
}

static void testPickerPressZoneTakesSixteenPixelsOffEachSide(void)
{
    /* picker at (140, 22), 96 wide and 24 high: the hot spot is the cursor plus (20, 3), the zone is x 156..220 and y 22..46 */
    CHECK_EQ(1, mnRemixGoalRulesPickerPressed(140.0F, 20.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(1, mnRemixGoalRulesPickerPressed(136.0F, 19.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(0, mnRemixGoalRulesPickerPressed(135.0F, 20.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(1, mnRemixGoalRulesPickerPressed(200.0F, 20.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(0, mnRemixGoalRulesPickerPressed(201.0F, 20.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(0, mnRemixGoalRulesPickerPressed(140.0F, 18.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(1, mnRemixGoalRulesPickerPressed(140.0F, 43.0F, 140.0F, 22.0F, 96, 24));
    CHECK_EQ(0, mnRemixGoalRulesPickerPressed(140.0F, 44.0F, 140.0F, 22.0F, 96, 24));
}

static void testToggleShowsTheVsTimeAndKeepsTheGoal(void)
{
    MNRemixGoalState state = { 0, 0, 8 };

    CHECK_EQ(8, mnRemixGoalRulesToggleGoal(&state, 30));
    CHECK_EQ(1, state.is_showing_time);
    CHECK_EQ(30, state.saved_goal);
}

static void testToggleBackSavesTheTimeAndRestoresTheGoal(void)
{
    MNRemixGoalState state = { 0, 0, 8 };

    mnRemixGoalRulesToggleGoal(&state, 30);

    CHECK_EQ(30, mnRemixGoalRulesToggleGoal(&state, 12));
    CHECK_EQ(0, state.is_showing_time);
    CHECK_EQ(12, state.vs_time);
}

static void testSettleWithTheGoalShowingKeepsThePickerValue(void)
{
    MNRemixGoalState state = { 0, 0, 8 };

    CHECK_EQ(45, mnRemixGoalRulesSettleGoal(&state, 45));
    CHECK_EQ(8, state.vs_time);
}

static void testSettleWithTheTimeShowingSplitsTimeAndGoal(void)
{
    MNRemixGoalState state = { 0, 0, 8 };

    mnRemixGoalRulesToggleGoal(&state, 30);

    CHECK_EQ(30, mnRemixGoalRulesSettleGoal(&state, 15));
    CHECK_EQ(0, state.is_showing_time);
    CHECK_EQ(15, state.vs_time);
}

static void testSlideColumnOfEachPortrait(void)
{
    CHECK_EQ(0, mnRemixCssSlideRulesColumn(0));
    CHECK_EQ(9, mnRemixCssSlideRulesColumn(9));
    CHECK_EQ(0, mnRemixCssSlideRulesColumn(10));
    CHECK_EQ(9, mnRemixCssSlideRulesColumn(29));
    CHECK_EQ(MNREMIX_CSS_SLIDE_BONUS_COLUMN, mnRemixCssSlideRulesColumn(30));
    CHECK_EQ(MNREMIX_CSS_SLIDE_RANDOM_COLUMN, mnRemixCssSlideRulesColumn(31));
    CHECK_EQ(-1, mnRemixCssSlideRulesColumn(32));
    CHECK_EQ(-1, mnRemixCssSlideRulesColumn(-1));
}

static void testSlideStartsOffScreenOnTheSideOfItsHalf(void)
{
    CHECK_EQ(-20, (int)mnRemixCssSlideRulesStartX(0));
    CHECK_EQ(-20, (int)mnRemixCssSlideRulesStartX(4));
    CHECK_EQ(310, (int)mnRemixCssSlideRulesStartX(5));
    CHECK_EQ(310, (int)mnRemixCssSlideRulesStartX(9));
    CHECK_EQ(-20, (int)mnRemixCssSlideRulesStartX(MNREMIX_CSS_SLIDE_BONUS_COLUMN));
    CHECK_EQ(310, (int)mnRemixCssSlideRulesStartX(MNREMIX_CSS_SLIDE_RANDOM_COLUMN));
}

static void testSlideMovesByTheSpeedOfTheColumn(void)
{
    CHECK_TRUE(mnRemixCssSlideRulesNextX(0, -20.0F) > -18.2F);
    CHECK_TRUE(mnRemixCssSlideRulesNextX(0, -20.0F) < -18.0F);
    CHECK_TRUE(mnRemixCssSlideRulesNextX(9, 310.0F) < 308.2F);
    CHECK_TRUE(mnRemixCssSlideRulesNextX(9, 310.0F) > 308.0F);
}

static void testSlideStopsAtTheCell(void)
{
    CHECK_EQ(39, (int)mnRemixCssSlideRulesNextX(0, 38.0F));
    CHECK_EQ(39, (int)mnRemixCssSlideRulesNextX(0, 39.0F));
    CHECK_EQ(255, (int)mnRemixCssSlideRulesNextX(9, 256.0F));
    CHECK_EQ(24, (int)mnRemixCssSlideRulesNextX(MNREMIX_CSS_SLIDE_BONUS_COLUMN, 23.5F));
    CHECK_EQ(279, (int)mnRemixCssSlideRulesNextX(MNREMIX_CSS_SLIDE_RANDOM_COLUMN, 279.5F));
}

static void testEveryPortraitReachesItsCell(void)
{
    int column;

    for (column = 0; column <= MNREMIX_CSS_SLIDE_RANDOM_COLUMN; column++)
    {
        float x = mnRemixCssSlideRulesStartX(column);
        float previous_x = x + 1.0F;
        int frames = 0;

        while ((x != previous_x) && (frames < 400))
        {
            previous_x = x;
            x = mnRemixCssSlideRulesNextX(column, x);
            frames++;
        }
        CHECK_TRUE(frames < 400);
        CHECK_TRUE((x >= 24.0F) && (x <= 279.0F));
    }
}


static void fillSlots(MNRemixTagTeamCssQueue *queue, int count)
{
    int slot;

    for (slot = 0; slot < count; slot++)
    {
        queue->current_slot = slot;
        mnRemixTagTeamCssRulesWrite(queue, 10 + slot, slot, 0, 0, 0);
    }
}

static void testQueueStartsEmptyOnTheFirstSlot(void)
{
    MNRemixTagTeamCssQueue queue;
    int slot;

    mnRemixTagTeamCssRulesReset(&queue);

    CHECK_EQ(0, queue.current_slot);

    for (slot = 0; slot < MNREMIX_TAGTEAM_CSS_SLOTS; slot++)
    {
        CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[slot].fkind);
    }
}

static void testWriteFillsTheCurrentSlotOnly(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    queue.current_slot = 2;
    mnRemixTagTeamCssRulesWrite(&queue, 7, 3, 1, 1, 0);

    CHECK_EQ(7, queue.slots[2].fkind);
    CHECK_EQ(3, queue.slots[2].costume);
    CHECK_EQ(1, queue.slots[2].shade);
    CHECK_EQ(1, queue.slots[2].is_classic);
    CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[1].fkind);
    CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[3].fkind);
}

static void testWriteWithoutAFighterEmptiesTheSlot(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    mnRemixTagTeamCssRulesWrite(&queue, 7, 3, 1, 0, 0);
    mnRemixTagTeamCssRulesWrite(&queue, MNREMIX_TAGTEAM_CSS_NONE, 0, 0, 0, 0);

    CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[0].fkind);
}

static void testNextSlotStopsOnTheLastSlot(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    mnRemixTagTeamCssRulesNextSlot(&queue, 2);
    mnRemixTagTeamCssRulesNextSlot(&queue, 2);
    mnRemixTagTeamCssRulesNextSlot(&queue, 2);

    CHECK_EQ(2, queue.current_slot);
}

static void testNextSlotBringsASlotPastTheLastOneBack(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    queue.current_slot = 4;
    mnRemixTagTeamCssRulesNextSlot(&queue, 1);

    CHECK_EQ(1, queue.current_slot);
}

static void testNextSlotNeverLeavesTheSixSlots(void)
{
    MNRemixTagTeamCssQueue queue;
    int i;

    mnRemixTagTeamCssRulesReset(&queue);

    for (i = 0; i < 20; i++)
    {
        mnRemixTagTeamCssRulesNextSlot(&queue, 98);
    }
    CHECK_EQ(MNREMIX_TAGTEAM_CSS_SLOTS - 1, queue.current_slot);
}

static void testStepBackEmptiesTheSlotBeingPickedAndGoesBack(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    fillSlots(&queue, 3);

    CHECK_EQ(1, mnRemixTagTeamCssRulesStepBack(&queue));
    CHECK_EQ(1, queue.current_slot);
    CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[2].fkind);
    CHECK_EQ(11, queue.slots[1].fkind);
    CHECK_EQ(10, queue.slots[0].fkind);
}

static void testStepBackOnTheFirstSlotDoesNothing(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    mnRemixTagTeamCssRulesWrite(&queue, 7, 0, 0, 0, 0);

    CHECK_EQ(0, mnRemixTagTeamCssRulesStepBack(&queue));
    CHECK_EQ(0, queue.current_slot);
    CHECK_EQ(7, queue.slots[0].fkind);
}

static void testClampSlotsEmptiesTheSlotsAboveTheNewStockCount(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    fillSlots(&queue, 5);

    CHECK_EQ(1, mnRemixTagTeamCssRulesClampSlots(&queue, 2));
    CHECK_EQ(2, queue.current_slot);
    CHECK_EQ(12, queue.slots[2].fkind);
    CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[3].fkind);
    CHECK_EQ(MNREMIX_TAGTEAM_CSS_NONE, queue.slots[4].fkind);
}

static void testClampSlotsKeepsAQueueThatStillFits(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    fillSlots(&queue, 3);

    CHECK_EQ(0, mnRemixTagTeamCssRulesClampSlots(&queue, 2));
    CHECK_EQ(2, queue.current_slot);
    CHECK_EQ(0, mnRemixTagTeamCssRulesClampSlots(&queue, 5));
}

static void testQueueIsCompleteWhenTheLastSlotIsFilled(void)
{
    MNRemixTagTeamCssQueue queue;

    mnRemixTagTeamCssRulesReset(&queue);
    fillSlots(&queue, 2);

    CHECK_EQ(0, mnRemixTagTeamCssRulesIsComplete(&queue, 2));
    CHECK_EQ(1, mnRemixTagTeamCssRulesIsComplete(&queue, 1));

    fillSlots(&queue, 3);

    CHECK_EQ(1, mnRemixTagTeamCssRulesIsComplete(&queue, 2));
}

static void testIconRowFollowsThePanel(void)
{
    CHECK_EQ(27, (int)mnRemixTagTeamCssRulesIconX(22.0F, 0));
    CHECK_EQ(37, (int)mnRemixTagTeamCssRulesIconX(22.0F, 1));
    CHECK_EQ(96, (int)mnRemixTagTeamCssRulesIconX(91.0F, 0));
    CHECK_EQ(117, MNREMIX_TAGTEAM_CSS_ICON_Y);
}

static void testIconCountIsTheStockCountPlusOne(void)
{
    CHECK_EQ(4, mnRemixTagTeamCssRulesIconCount(3));
    CHECK_EQ(1, mnRemixTagTeamCssRulesIconCount(0));
    CHECK_EQ(6, mnRemixTagTeamCssRulesIconCount(5));
    CHECK_EQ(6, mnRemixTagTeamCssRulesIconCount(98));
    CHECK_EQ(0, mnRemixTagTeamCssRulesIconCount(-1));
}

static void testReturnFromTheCssOpensTheRemixPageOnTheModePlayed(void)
{
    CHECK_EQ(0, mnRemixVSModeReturnCursor(1, 1));
    CHECK_EQ(1, mnRemixVSModeReturnCursor(1, 2));
    CHECK_EQ(2, mnRemixVSModeReturnCursor(1, 3));
    CHECK_EQ(3, mnRemixVSModeReturnCursor(1, 4));
    CHECK_EQ(4, mnRemixVSModeReturnCursor(1, 5));
}

static void testReturnAfterVsStartOpensPageOne(void)
{
    CHECK_EQ(MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR, mnRemixVSModeReturnCursor(1, 0));
}

static void testReturnFromAnyOtherSceneOpensPageOne(void)
{
    CHECK_EQ(MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR, mnRemixVSModeReturnCursor(0, 1));
    CHECK_EQ(MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR, mnRemixVSModeReturnCursor(0, 5));
}

static void testReturnWithAModeOutOfRangeOpensPageOne(void)
{
    CHECK_EQ(MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR, mnRemixVSModeReturnCursor(1, -1));
    CHECK_EQ(MNREMIX_VSMODE_RETURN_NO_MODE_CURSOR, mnRemixVSModeReturnCursor(1, 6));
}

void remixTestCssVsModesRules(void)
{
    RUN_TEST(testTitleOffsetPerMode);
    RUN_TEST(testSmashketballTitleFollowsTheType);
    RUN_TEST(testTitleOffsetOutOfRangeKeepsTheVanillaHeader);
    RUN_TEST(testAnnouncerPerMode);
    RUN_TEST(testTwoTeamsNeedsExactlyTwoTeamsPresent);
    RUN_TEST(testTwoTeamsIgnoresPanelsThatAreNotInPlay);
    RUN_TEST(testTwoTeamsIgnoresATeamOutOfRange);
    RUN_TEST(testTimeValuesAreTheRemixOptions);
    RUN_TEST(testGoalTablesAreSorted);
    RUN_TEST(testNextTakesTheFirstValueAboveAndWraps);
    RUN_TEST(testPrevTakesTheLastValueBelowAndWraps);
    RUN_TEST(testNumberMovesRightFromThreeDigits);
    RUN_TEST(testPickerPressZoneTakesSixteenPixelsOffEachSide);
    RUN_TEST(testToggleShowsTheVsTimeAndKeepsTheGoal);
    RUN_TEST(testToggleBackSavesTheTimeAndRestoresTheGoal);
    RUN_TEST(testSettleWithTheGoalShowingKeepsThePickerValue);
    RUN_TEST(testSettleWithTheTimeShowingSplitsTimeAndGoal);
    RUN_TEST(testSlideColumnOfEachPortrait);
    RUN_TEST(testSlideStartsOffScreenOnTheSideOfItsHalf);
    RUN_TEST(testSlideMovesByTheSpeedOfTheColumn);
    RUN_TEST(testSlideStopsAtTheCell);
    RUN_TEST(testEveryPortraitReachesItsCell);
    RUN_TEST(testQueueStartsEmptyOnTheFirstSlot);
    RUN_TEST(testWriteFillsTheCurrentSlotOnly);
    RUN_TEST(testWriteWithoutAFighterEmptiesTheSlot);
    RUN_TEST(testNextSlotStopsOnTheLastSlot);
    RUN_TEST(testNextSlotBringsASlotPastTheLastOneBack);
    RUN_TEST(testNextSlotNeverLeavesTheSixSlots);
    RUN_TEST(testStepBackEmptiesTheSlotBeingPickedAndGoesBack);
    RUN_TEST(testStepBackOnTheFirstSlotDoesNothing);
    RUN_TEST(testClampSlotsEmptiesTheSlotsAboveTheNewStockCount);
    RUN_TEST(testClampSlotsKeepsAQueueThatStillFits);
    RUN_TEST(testQueueIsCompleteWhenTheLastSlotIsFilled);
    RUN_TEST(testIconRowFollowsThePanel);
    RUN_TEST(testIconCountIsTheStockCountPlusOne);
    RUN_TEST(testReturnFromTheCssOpensTheRemixPageOnTheModePlayed);
    RUN_TEST(testReturnAfterVsStartOpensPageOne);
    RUN_TEST(testReturnFromAnyOtherSceneOpensPageOne);
    RUN_TEST(testReturnWithAModeOutOfRangeOpensPageOne);
}
