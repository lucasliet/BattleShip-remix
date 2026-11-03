/**
 * test_twelvecb_custom.c - unit tests of the 12-Character Battle Custom character set (mn/mnplayers/mnremixtwelvecbcustom.c).
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixtwelvecbcustom.h"
#include "mn/mnplayers/mnremixtwelvecblayout.h"

static const int sRoster[] = { 0, 1, 2, 60, 3, 4 };
#define ROSTER_COUNT 6
#define SANDBAG 60

static int sPickCalls;

static int pickCounting(void)
{
    return 40 + (sPickCalls++);
}

static void testStartsOnTheDefaultSetForBothPlayers(void)
{
    int slot;

    mnRemixTwelveCBCustomReset();

    for (slot = 0; slot < MNREMIX_TCB_SLOTS; slot++)
    {
        CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, slot), mnRemixTwelveCBCustomFkind(slot));
        CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, slot), mnRemixTwelveCBLayoutFkind(MNREMIX_TCB_CUSTOM_SET, slot));
    }
}

static void testBothHalvesOfARowShareTheEntryOfTheOwner(void)
{
    mnRemixTwelveCBCustomReset();

    CHECK_EQ(mnRemixTwelveCBCustomIndexOfSlot(1), mnRemixTwelveCBCustomIndexOfSlot(5));
    CHECK_EQ(0, mnRemixTwelveCBCustomIndexOfSlot(0));
    CHECK_EQ(4, mnRemixTwelveCBCustomIndexOfSlot(8));
    CHECK_EQ(11, mnRemixTwelveCBCustomIndexOfSlot(23));
    CHECK_EQ(-1, mnRemixTwelveCBCustomIndexOfSlot(24));
    CHECK_EQ(-1, mnRemixTwelveCBCustomIndexOfSlot(-1));
}

static void testCycleMovesOnePortraitOfTheOwnerOnly(void)
{
    mnRemixTwelveCBCustomReset();

    CHECK_EQ(1, mnRemixTwelveCBCustomCycle(0, 1, sRoster, ROSTER_COUNT, SANDBAG));

    CHECK_EQ(0, mnRemixTwelveCBCustomFkind(0));
    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 1), mnRemixTwelveCBCustomFkind(1));
    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 4), mnRemixTwelveCBCustomFkind(4));
}

static void testCycleWrapsAndSkipsTheSandbag(void)
{
    mnRemixTwelveCBCustomReset();
    mnRemixTwelveCBCustomApplyAll(0, 2);

    mnRemixTwelveCBCustomCycle(0, 1, sRoster, ROSTER_COUNT, SANDBAG);
    CHECK_EQ(3, mnRemixTwelveCBCustomFkind(0));

    mnRemixTwelveCBCustomCycle(0, -1, sRoster, ROSTER_COUNT, SANDBAG);
    CHECK_EQ(2, mnRemixTwelveCBCustomFkind(0));

    mnRemixTwelveCBCustomApplyAll(0, 0);
    mnRemixTwelveCBCustomCycle(0, -1, sRoster, ROSTER_COUNT, SANDBAG);
    CHECK_EQ(4, mnRemixTwelveCBCustomFkind(0));
    mnRemixTwelveCBCustomCycle(0, 1, sRoster, ROSTER_COUNT, SANDBAG);
    CHECK_EQ(0, mnRemixTwelveCBCustomFkind(0));
}

static void testCycleRejectsAnInvalidSlotOrAnEmptyRoster(void)
{
    mnRemixTwelveCBCustomReset();

    CHECK_EQ(0, mnRemixTwelveCBCustomCycle(24, 1, sRoster, ROSTER_COUNT, SANDBAG));
    CHECK_EQ(0, mnRemixTwelveCBCustomCycle(0, 1, sRoster, 0, SANDBAG));
    CHECK_EQ(0, mnRemixTwelveCBCustomCycle(0, 0, sRoster, ROSTER_COUNT, SANDBAG));
}

static void testApplyAllFillsOnlyThatPlayer(void)
{
    mnRemixTwelveCBCustomReset();

    mnRemixTwelveCBCustomApplyAll(1, 9);

    CHECK_EQ(9, mnRemixTwelveCBCustomFkind(4));
    CHECK_EQ(9, mnRemixTwelveCBCustomFkind(23));
    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 0), mnRemixTwelveCBCustomFkind(0));
    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 19), mnRemixTwelveCBCustomFkind(19));
}

static void testPresetWalksTheFourSetsAndComesBack(void)
{
    int step;

    mnRemixTwelveCBCustomReset();

    for (step = 0; step < 5; step++)
    {
        int set = step % MNREMIX_TCB_CUSTOM_PRESETS;

        mnRemixTwelveCBCustomPreset(0);

        CHECK_EQ(mnRemixTwelveCBLayoutFkind(set, 0), mnRemixTwelveCBCustomFkind(0));
        CHECK_EQ(mnRemixTwelveCBLayoutFkind(set, 17), mnRemixTwelveCBCustomFkind(17));
    }
    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 4), mnRemixTwelveCBCustomFkind(4));
}

static void testCopyTakesTheOpponentsEntries(void)
{
    mnRemixTwelveCBCustomReset();
    mnRemixTwelveCBCustomApplyAll(0, 7);

    mnRemixTwelveCBCustomCopyOpponent(1);

    CHECK_EQ(7, mnRemixTwelveCBCustomFkind(4));
    CHECK_EQ(7, mnRemixTwelveCBCustomFkind(14));
    CHECK_EQ(7, mnRemixTwelveCBCustomFkind(0));
}

static void testRandomizeAsksForOneCharacterPerEntry(void)
{
    mnRemixTwelveCBCustomReset();
    sPickCalls = 0;

    mnRemixTwelveCBCustomRandomize(0, pickCounting);

    CHECK_EQ(MNREMIX_TCB_CUSTOM_ENTRIES, sPickCalls);
    CHECK_EQ(40, mnRemixTwelveCBCustomFkind(0));
    CHECK_EQ(41, mnRemixTwelveCBCustomFkind(1));
    CHECK_EQ(44, mnRemixTwelveCBCustomFkind(8));
    CHECK_EQ(51, mnRemixTwelveCBCustomFkind(19));
    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 4), mnRemixTwelveCBCustomFkind(4));
}

static void testInvalidPlayerChangesNothing(void)
{
    mnRemixTwelveCBCustomReset();

    mnRemixTwelveCBCustomApplyAll(2, 9);
    mnRemixTwelveCBCustomPreset(-1);
    mnRemixTwelveCBCustomCopyOpponent(5);

    CHECK_EQ(mnRemixTwelveCBLayoutFkind(0, 0), mnRemixTwelveCBCustomFkind(0));
}

static void testRepeatCyclesAtOnceThenAfterTheDelayEveryEightFrames(void)
{
    int timer = MNREMIX_TCB_CUSTOM_REPEAT_IDLE;
    int frame;
    int cycles = 0;
    int first_repeat_frame = -1;

    for (frame = 0; frame < 60; frame++)
    {
        if (mnRemixTwelveCBCustomRepeat(&timer, 1) != 0)
        {
            cycles++;
            if ((frame > 0) && (first_repeat_frame < 0))
            {
                first_repeat_frame = frame;
            }
        }
    }
    CHECK_EQ(30, first_repeat_frame);
    CHECK_EQ(1 + 1 + ((59 - 30) / 8), cycles);
}

static void testRepeatRestartsAfterTheButtonIsLetGo(void)
{
    int timer = MNREMIX_TCB_CUSTOM_REPEAT_IDLE;

    CHECK_EQ(1, mnRemixTwelveCBCustomRepeat(&timer, 1));
    CHECK_EQ(0, mnRemixTwelveCBCustomRepeat(&timer, 1));
    CHECK_EQ(0, mnRemixTwelveCBCustomRepeat(&timer, 0));
    CHECK_EQ(MNREMIX_TCB_CUSTOM_REPEAT_IDLE, timer);
    CHECK_EQ(1, mnRemixTwelveCBCustomRepeat(&timer, 1));
}

static void testDpadUpShortPressChangesThePreset(void)
{
    int timer = 0;
    int frame;

    for (frame = 0; frame < 10; frame++)
    {
        CHECK_EQ(MNREMIX_TCB_CUSTOM_DPAD_UP_NONE, mnRemixTwelveCBCustomDpadUp(&timer, 1, 0));
    }
    CHECK_EQ(MNREMIX_TCB_CUSTOM_DPAD_UP_PRESET, mnRemixTwelveCBCustomDpadUp(&timer, 0, 1));
    CHECK_EQ(0, timer);
}

static void testDpadUpHeldCopiesOnceAndTheReleaseDoesNothing(void)
{
    int timer = 0;
    int frame;
    int copies = 0;

    for (frame = 0; frame < 100; frame++)
    {
        if (mnRemixTwelveCBCustomDpadUp(&timer, 1, 0) == MNREMIX_TCB_CUSTOM_DPAD_UP_COPY)
        {
            copies++;
            CHECK_EQ(59, frame);
        }
    }
    CHECK_EQ(1, copies);
    CHECK_EQ(MNREMIX_TCB_CUSTOM_DPAD_UP_NONE, mnRemixTwelveCBCustomDpadUp(&timer, 0, 1));
    CHECK_EQ(0, timer);
}

void remixTestTwelveCBCustom(void)
{
    printf("twelvecb custom set\n");
    RUN_TEST(testStartsOnTheDefaultSetForBothPlayers);
    RUN_TEST(testBothHalvesOfARowShareTheEntryOfTheOwner);
    RUN_TEST(testCycleMovesOnePortraitOfTheOwnerOnly);
    RUN_TEST(testCycleWrapsAndSkipsTheSandbag);
    RUN_TEST(testCycleRejectsAnInvalidSlotOrAnEmptyRoster);
    RUN_TEST(testApplyAllFillsOnlyThatPlayer);
    RUN_TEST(testPresetWalksTheFourSetsAndComesBack);
    RUN_TEST(testCopyTakesTheOpponentsEntries);
    RUN_TEST(testRandomizeAsksForOneCharacterPerEntry);
    RUN_TEST(testInvalidPlayerChangesNothing);
    RUN_TEST(testRepeatCyclesAtOnceThenAfterTheDelayEveryEightFrames);
    RUN_TEST(testRepeatRestartsAfterTheButtonIsLetGo);
    RUN_TEST(testDpadUpShortPressChangesThePreset);
    RUN_TEST(testDpadUpHeldCopiesOnceAndTheReleaseDoesNothing);
}
