/**
 * test_stage_select_rules.c - unit tests of the Remix stage select rules: the Hazards, Movement and Layout lines
 * (mn/mnmaps/mnremixstagepanelrules.c), the tournament layout (mnremixstagetournamentrules.c) and the Random Stage Roulette
 * timer (mnremixstageroulette.c).
 */

#include <string.h>

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "mn/mnmaps/mnremixstagepanelrules.h"
#include "mn/mnmaps/mnremixstageroulette.h"
#include "mn/mnmaps/mnremixstagetournamentrules.h"

static MNRemixStagePanelInput makeInput(int stage_id, int hazard_type, int hazard_mode)
{
    MNRemixStagePanelInput input = { 0 };

    input.stage_id = stage_id;
    input.hazard_type = hazard_type;
    input.hazard_mode = hazard_mode;

    return input;
}

static void testStageWithBothTypesShowsBothLinesOn(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_PEACHS_CASTLE, MNREMIX_HAZARD_TYPE_BOTH, 0);
    MNRemixStagePanelView view;

    mnRemixStagePanelRulesCompute(&input, &view);

    CHECK_EQ(1, view.show_hazards);
    CHECK_EQ(1, view.show_movement);
    CHECK_EQ(0, strcmp("On", view.hazards_value));
    CHECK_EQ(0, strcmp("On", view.movement_value));
    CHECK_EQ(0, view.cursor_color_index);
}

static void testLinesFollowTheHazardType(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_HYRULE_CASTLE, MNREMIX_HAZARD_TYPE_HAZARDS, 0);
    MNRemixStagePanelView view;

    mnRemixStagePanelRulesCompute(&input, &view);
    CHECK_EQ(1, view.show_hazards);
    CHECK_EQ(0, view.show_movement);

    input.hazard_type = MNREMIX_HAZARD_TYPE_MOVEMENT;
    mnRemixStagePanelRulesCompute(&input, &view);
    CHECK_EQ(0, view.show_hazards);
    CHECK_EQ(1, view.show_movement);

    input.hazard_type = MNREMIX_HAZARD_TYPE_NONE;
    mnRemixStagePanelRulesCompute(&input, &view);
    CHECK_EQ(0, view.show_hazards);
    CHECK_EQ(0, view.show_movement);
}

static void testValuesReadOffWhenTheModeBitsAreSet(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_PEACHS_CASTLE, MNREMIX_HAZARD_TYPE_BOTH, MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON);
    MNRemixStagePanelView view;

    mnRemixStagePanelRulesCompute(&input, &view);
    CHECK_EQ(0, strcmp("Off", view.hazards_value));
    CHECK_EQ(0, strcmp("On", view.movement_value));

    input.hazard_mode = MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF;
    mnRemixStagePanelRulesCompute(&input, &view);
    CHECK_EQ(0, strcmp("On", view.hazards_value));
    CHECK_EQ(0, strcmp("Off", view.movement_value));

    input.hazard_mode = MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF;
    mnRemixStagePanelRulesCompute(&input, &view);
    CHECK_EQ(0, strcmp("Off", view.hazards_value));
    CHECK_EQ(0, strcmp("Off", view.movement_value));
}

static void testWhispyModeMarksTheDreamLandStages(void)
{
    static const int stages[] = { nGRRemixStage_DREAM_LAND, nGRRemixStage_WINTER_DL, nGRRemixStage_DREAM_LAND_O, nGRRemixStage_DREAM_LAND_SR };
    static const char *const marks[] = { "On", "On (J)", "On (S)", "On (H)" };
    MNRemixStagePanelView view;
    int stage;
    int whispy_mode;

    for (stage = 0; stage < 4; stage++)
    {
        for (whispy_mode = 0; whispy_mode < 4; whispy_mode++)
        {
            MNRemixStagePanelInput input = makeInput(stages[stage], MNREMIX_HAZARD_TYPE_HAZARDS, 0);

            input.whispy_mode = whispy_mode;
            mnRemixStagePanelRulesCompute(&input, &view);
            CHECK_EQ(0, strcmp(marks[whispy_mode], view.hazards_value));
        }
    }
}

static void testWhispyModeLeavesOtherStagesUnmarked(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_PEACHS_CASTLE, MNREMIX_HAZARD_TYPE_BOTH, 0);
    MNRemixStagePanelView view;

    input.whispy_mode = MNREMIX_WHISPY_MODE_HYPER;
    mnRemixStagePanelRulesCompute(&input, &view);

    CHECK_EQ(0, strcmp("On", view.hazards_value));
}

static void testSaffronPokemonRateMarksSaffronCity(void)
{
    static const char *const marks[] = { "On", "On (S)", "On (H)", "On (Q)" };
    MNRemixStagePanelView view;
    int rate;

    for (rate = 0; rate < 4; rate++)
    {
        MNRemixStagePanelInput input = makeInput(nGRRemixStage_SAFFRON_CITY, MNREMIX_HAZARD_TYPE_BOTH, 0);

        input.saffron_poke_rate = rate;
        mnRemixStagePanelRulesCompute(&input, &view);
        CHECK_EQ(0, strcmp(marks[rate], view.hazards_value));
    }
}

static void testWhispyModeWinsOverSaffronRateOnDreamLand(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_DREAM_LAND, MNREMIX_HAZARD_TYPE_HAZARDS, 0);
    MNRemixStagePanelView view;

    input.whispy_mode = MNREMIX_WHISPY_MODE_JAPANESE;
    input.saffron_poke_rate = MNREMIX_SAFFRON_RATE_QUICK_ATTACK;
    mnRemixStagePanelRulesCompute(&input, &view);

    CHECK_EQ(0, strcmp("On (J)", view.hazards_value));
}

static void testRandomShowsBothLinesWithTheCurrentMode(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_RANDOM, MNREMIX_HAZARD_TYPE_NONE, MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF);
    MNRemixStagePanelView view;

    input.is_random = 1;
    mnRemixStagePanelRulesCompute(&input, &view);

    CHECK_EQ(1, view.show_hazards);
    CHECK_EQ(1, view.show_movement);
    CHECK_EQ(0, strcmp("Off", view.hazards_value));
    CHECK_EQ(0, strcmp("Off", view.movement_value));
    CHECK_EQ(3, view.cursor_color_index);
}

static void testRandomOnTheTournamentLayoutReadsOnOnWithARedCursor(void)
{
    MNRemixStagePanelInput input = makeInput(nGRRemixStage_RANDOM, MNREMIX_HAZARD_TYPE_NONE, MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF);
    MNRemixStagePanelView view;

    input.is_random = 1;
    input.is_tournament = 1;
    mnRemixStagePanelRulesCompute(&input, &view);

    CHECK_EQ(0, strcmp("On", view.hazards_value));
    CHECK_EQ(0, strcmp("On", view.movement_value));
    CHECK_EQ(0, view.cursor_color_index);
}

static void testCursorColorFollowsTheHazardMode(void)
{
    MNRemixStagePanelView view;
    MNRemixStagePanelInput hazards = makeInput(nGRRemixStage_HYRULE_CASTLE, MNREMIX_HAZARD_TYPE_HAZARDS, MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON);
    MNRemixStagePanelInput movement = makeInput(nGRRemixStage_DREAM_LAND_BETA_2, MNREMIX_HAZARD_TYPE_MOVEMENT, MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON);
    MNRemixStagePanelInput none = makeInput(nGRRemixStage_FINAL_DESTINATION, MNREMIX_HAZARD_TYPE_NONE, MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF);
    MNRemixStagePanelInput both = makeInput(nGRRemixStage_PEACHS_CASTLE, MNREMIX_HAZARD_TYPE_BOTH, MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF);

    mnRemixStagePanelRulesCompute(&hazards, &view);
    CHECK_EQ(0, view.cursor_color_index);
    hazards.hazard_mode = MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON;
    mnRemixStagePanelRulesCompute(&hazards, &view);
    CHECK_EQ(1, view.cursor_color_index);

    mnRemixStagePanelRulesCompute(&movement, &view);
    CHECK_EQ(0, view.cursor_color_index);
    movement.hazard_mode = MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF;
    mnRemixStagePanelRulesCompute(&movement, &view);
    CHECK_EQ(2, view.cursor_color_index);

    mnRemixStagePanelRulesCompute(&none, &view);
    CHECK_EQ(0, view.cursor_color_index);

    mnRemixStagePanelRulesCompute(&both, &view);
    CHECK_EQ(2, view.cursor_color_index);
}

static void testHazardModeStepsThroughTheFourModesOnBothAndRandom(void)
{
    int next_mode = -1;
    int mode;

    for (mode = 0; mode < 4; mode++)
    {
        CHECK_EQ(1, mnRemixStagePanelRulesNextHazardMode(mode, MNREMIX_HAZARD_TYPE_BOTH, 0, &next_mode));
        CHECK_EQ((mode + 1) % 4, next_mode);
        CHECK_EQ(1, mnRemixStagePanelRulesNextHazardMode(mode, MNREMIX_HAZARD_TYPE_NONE, 1, &next_mode));
        CHECK_EQ((mode + 1) % 4, next_mode);
    }
}

static void testHazardModeFlipsOnlyTheBitOfTheStageType(void)
{
    int next_mode = -1;

    CHECK_EQ(1, mnRemixStagePanelRulesNextHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF, MNREMIX_HAZARD_TYPE_HAZARDS, 0, &next_mode));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF, next_mode);
    CHECK_EQ(1, mnRemixStagePanelRulesNextHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF, MNREMIX_HAZARD_TYPE_HAZARDS, 0, &next_mode));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF, next_mode);
    CHECK_EQ(1, mnRemixStagePanelRulesNextHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, MNREMIX_HAZARD_TYPE_MOVEMENT, 0, &next_mode));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF, next_mode);
    CHECK_EQ(1, mnRemixStagePanelRulesNextHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF, MNREMIX_HAZARD_TYPE_MOVEMENT, 0, &next_mode));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, next_mode);
}

static void testHazardModeStaysWhenTheStageHasNothingToSwitch(void)
{
    int next_mode = 99;

    CHECK_EQ(0, mnRemixStagePanelRulesNextHazardMode(0, MNREMIX_HAZARD_TYPE_NONE, 0, &next_mode));
    CHECK_EQ(99, next_mode);
}

static void testLayoutLineShowsOnlyForVariantStagesOnTheNormalLayout(void)
{
    CHECK_EQ(1, mnRemixStagePanelRulesShowLayout(0, 1, 0));
    CHECK_EQ(0, mnRemixStagePanelRulesShowLayout(0, 0, 0));
    CHECK_EQ(0, mnRemixStagePanelRulesShowLayout(1, 0, 0));
    CHECK_EQ(0, mnRemixStagePanelRulesShowLayout(0, 1, 1));
}

static void testLayoutLabelNamesTheVariantType(void)
{
    CHECK_EQ(0, strcmp("Def.", mnRemixStagePanelRulesLayoutLabel(0)));
    CHECK_EQ(0, strcmp("DL", mnRemixStagePanelRulesLayoutLabel(1)));
    CHECK_EQ(0, strcmp("\x7F", mnRemixStagePanelRulesLayoutLabel(2)));
    CHECK_EQ(0, strcmp("Remix", mnRemixStagePanelRulesLayoutLabel(3)));
    CHECK_EQ(0, strcmp("Remix", mnRemixStagePanelRulesLayoutLabel(4)));
    CHECK_EQ(0, strcmp("Remix", mnRemixStagePanelRulesLayoutLabel(5)));
}

static void testTournamentSourcePageIsTheEighteenSquareGridPage(void)
{
    CHECK_EQ(0, mnRemixTournamentRulesSourcePage(0));
    CHECK_EQ(0, mnRemixTournamentRulesSourcePage(17));
    CHECK_EQ(1, mnRemixTournamentRulesSourcePage(18));
    CHECK_EQ(1, mnRemixTournamentRulesSourcePage(35));
    CHECK_EQ(2, mnRemixTournamentRulesSourcePage(36));
    CHECK_EQ(2, mnRemixTournamentRulesSourcePage(53));
}

static void testTournamentDreamLandIsHazardsOffOnThePageThreeCopy(void)
{
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, nGRRemixStage_DREAM_LAND, 0));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, nGRRemixStage_DREAM_LAND, 2));
}

static void testTournamentYoshiIslandDLIsHazardsOnOnThePagesOneAndTwo(void)
{
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, nGRRemixStage_YOSHI_ISLAND_DL, 1));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, nGRRemixStage_YOSHI_ISLAND_DL, 2));
}

static void testTournamentSaffronDLIsMovementOffOnThePageThreeCopy(void)
{
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, nGRRemixStage_SAFFRON_DL, 0));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_OFF, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_ON_MOVEMENT_ON, nGRRemixStage_SAFFRON_DL, 2));
}

static void testTournamentOtherStagesTakeTheTableMode(void)
{
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_OFF, nGRRemixStage_PEACHS_CASTLE, 0));
    CHECK_EQ(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, mnRemixTournamentRulesHazardMode(MNREMIX_HAZARD_MODE_HAZARDS_OFF_MOVEMENT_ON, nGRRemixStage_PEACHS_CASTLE, 2));
}

static void testRouletteJumpsOnTheFirstFrameThenEveryDelay(void)
{
    MNRemixRoulette roulette;
    int frame;
    int jumps = 0;

    mnRemixRouletteReset(&roulette);
    mnRemixRouletteStart(&roulette);

    for (frame = 0; frame < (3 * MNREMIX_ROULETTE_DELAY); frame++)
    {
        int jumped = mnRemixRouletteTick(&roulette);

        CHECK_EQ((frame % MNREMIX_ROULETTE_DELAY) == 0, jumped);
        jumps += jumped;
    }
    CHECK_EQ(3, jumps);
}

static void testRouletteIsIdleUntilStartedAndAfterStop(void)
{
    MNRemixRoulette roulette;

    mnRemixRouletteReset(&roulette);
    CHECK_EQ(0, mnRemixRouletteIsActive(&roulette));

    mnRemixRouletteStart(&roulette);
    CHECK_EQ(1, mnRemixRouletteIsActive(&roulette));

    mnRemixRouletteStop(&roulette);
    CHECK_EQ(0, mnRemixRouletteIsActive(&roulette));
}

static void testRouletteSwallowsTheSelectPressAndTakesItOnTheNextFrame(void)
{
    MNRemixRoulette roulette;

    mnRemixRouletteReset(&roulette);
    mnRemixRouletteStart(&roulette);
    mnRemixRouletteMarkSelect(&roulette);

    CHECK_EQ(1, mnRemixRouletteIsActive(&roulette));
    CHECK_EQ(1, mnRemixRouletteTakeSelect(&roulette));
    CHECK_EQ(0, mnRemixRouletteIsActive(&roulette));
    CHECK_EQ(0, mnRemixRouletteTakeSelect(&roulette));
}

static void testRouletteTakeSelectIgnoresARunningRoulette(void)
{
    MNRemixRoulette roulette;

    mnRemixRouletteReset(&roulette);
    CHECK_EQ(0, mnRemixRouletteTakeSelect(&roulette));

    mnRemixRouletteStart(&roulette);
    CHECK_EQ(0, mnRemixRouletteTakeSelect(&roulette));
    CHECK_EQ(1, mnRemixRouletteIsActive(&roulette));
}

static void testRouletteSlotPickingWalksTheSetBitsInOrder(void)
{
    unsigned int slot_mask = (1U << 0) | (1U << 3) | (1U << 4) | (1U << 8);

    CHECK_EQ(4, mnRemixRouletteCountSlots(slot_mask));
    CHECK_EQ(0, mnRemixRouletteNthSlot(slot_mask, 0));
    CHECK_EQ(3, mnRemixRouletteNthSlot(slot_mask, 1));
    CHECK_EQ(4, mnRemixRouletteNthSlot(slot_mask, 2));
    CHECK_EQ(8, mnRemixRouletteNthSlot(slot_mask, 3));
    CHECK_EQ(-1, mnRemixRouletteNthSlot(slot_mask, 4));
    CHECK_EQ(-1, mnRemixRouletteNthSlot(slot_mask, -1));
}

static void testRouletteSlotPickingOfAnEmptyMask(void)
{
    CHECK_EQ(0, mnRemixRouletteCountSlots(0));
    CHECK_EQ(-1, mnRemixRouletteNthSlot(0, 0));
}

void remixTestStageSelectRules(void)
{
    RUN_TEST(testStageWithBothTypesShowsBothLinesOn);
    RUN_TEST(testLinesFollowTheHazardType);
    RUN_TEST(testValuesReadOffWhenTheModeBitsAreSet);
    RUN_TEST(testWhispyModeMarksTheDreamLandStages);
    RUN_TEST(testWhispyModeLeavesOtherStagesUnmarked);
    RUN_TEST(testSaffronPokemonRateMarksSaffronCity);
    RUN_TEST(testWhispyModeWinsOverSaffronRateOnDreamLand);
    RUN_TEST(testRandomShowsBothLinesWithTheCurrentMode);
    RUN_TEST(testRandomOnTheTournamentLayoutReadsOnOnWithARedCursor);
    RUN_TEST(testCursorColorFollowsTheHazardMode);
    RUN_TEST(testHazardModeStepsThroughTheFourModesOnBothAndRandom);
    RUN_TEST(testHazardModeFlipsOnlyTheBitOfTheStageType);
    RUN_TEST(testHazardModeStaysWhenTheStageHasNothingToSwitch);
    RUN_TEST(testLayoutLineShowsOnlyForVariantStagesOnTheNormalLayout);
    RUN_TEST(testLayoutLabelNamesTheVariantType);
    RUN_TEST(testTournamentSourcePageIsTheEighteenSquareGridPage);
    RUN_TEST(testTournamentDreamLandIsHazardsOffOnThePageThreeCopy);
    RUN_TEST(testTournamentYoshiIslandDLIsHazardsOnOnThePagesOneAndTwo);
    RUN_TEST(testTournamentSaffronDLIsMovementOffOnThePageThreeCopy);
    RUN_TEST(testTournamentOtherStagesTakeTheTableMode);
    RUN_TEST(testRouletteJumpsOnTheFirstFrameThenEveryDelay);
    RUN_TEST(testRouletteIsIdleUntilStartedAndAfterStop);
    RUN_TEST(testRouletteSwallowsTheSelectPressAndTakesItOnTheNextFrame);
    RUN_TEST(testRouletteTakeSelectIgnoresARunningRoulette);
    RUN_TEST(testRouletteSlotPickingWalksTheSetBitsInOrder);
    RUN_TEST(testRouletteSlotPickingOfAnEmptyMask);
}
