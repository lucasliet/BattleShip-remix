/**
 * test_training_leftover_rules.c - unit tests of the Training leftover rules (sc/sc1pmode/sc1ptrainingleftoverrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1ptrainingleftoverrules.h"

static void testTheFrameCountGrowsOutsideHitlag(void)
{
    CHECK_EQ(6, sc1PTrainingLeftoverRulesNextFrameCount(10, 10, 0, 0, 5));
}

static void testTheFrameCountCountsTheFirstHitlagFrameOnly(void)
{
    CHECK_EQ(6, sc1PTrainingLeftoverRulesNextFrameCount(10, 10, 0, 4, 5));
    CHECK_EQ(5, sc1PTrainingLeftoverRulesNextFrameCount(10, 10, 4, 3, 5));
}

static void testTheFrameCountCountsTheFrameAfterHitlagEnds(void)
{
    CHECK_EQ(6, sc1PTrainingLeftoverRulesNextFrameCount(10, 10, 1, 0, 5));
}

static void testANewActionRestartsTheCountAtOne(void)
{
    CHECK_EQ(1, sc1PTrainingLeftoverRulesNextFrameCount(10, 11, 0, 0, 5));
    CHECK_EQ(1, sc1PTrainingLeftoverRulesNextFrameCount(10, 11, 4, 3, 5));
}

static void testActionsBelow0xDCUseTheSharedTable(void)
{
    CHECK_EQ(1, sc1PTrainingLeftoverRulesIsSharedAction(0));
    CHECK_EQ(1, sc1PTrainingLeftoverRulesIsSharedAction(0xDB));
    CHECK_EQ(0, sc1PTrainingLeftoverRulesIsSharedAction(0xDC));
    CHECK_EQ(0, sc1PTrainingLeftoverRulesIsSharedAction(0x120));
}

static void testTheCharacterTableStartsAt0xDC(void)
{
    CHECK_EQ(0, sc1PTrainingLeftoverRulesGetUniqueIndex(0xDC));
    CHECK_EQ(5, sc1PTrainingLeftoverRulesGetUniqueIndex(0xE1));
}

static void testHoldExitCountsHeldFramesAndExitsOnThe33rd(void)
{
    int32_t held = 0;
    int32_t frame;

    for (frame = 1; frame < SC1PTRAININGLEFTOVERRULES_HOLD_EXIT_FRAMES; frame++)
    {
        CHECK_EQ(0, sc1PTrainingLeftoverRulesHoldExitStep(held, 1, &held));
        CHECK_EQ(frame, held);
    }
    CHECK_EQ(1, sc1PTrainingLeftoverRulesHoldExitStep(held, 1, &held));
    CHECK_EQ(0, held);
}

static void testHoldExitDoesNotExitOnThe32ndFrame(void)
{
    int32_t held = 31;

    CHECK_EQ(0, sc1PTrainingLeftoverRulesHoldExitStep(held, 1, &held));
    CHECK_EQ(32, held);
}

static void testReleasingAClearsTheHeldFrames(void)
{
    int32_t held = 20;

    CHECK_EQ(0, sc1PTrainingLeftoverRulesHoldExitStep(held, 0, &held));
    CHECK_EQ(0, held);
}

static void testTheHoldBarGrowsByTwoUpTo64(void)
{
    int32_t width = 0;
    int32_t frame;

    for (frame = 1; frame <= 32; frame++)
    {
        width = sc1PTrainingLeftoverRulesGetHoldBarWidth(width, 1);
        CHECK_EQ(frame * 2, width);
    }
    CHECK_EQ(64, sc1PTrainingLeftoverRulesGetHoldBarWidth(width, 1));
}

static void testTheHoldBarEmptiesWhenAIsReleased(void)
{
    CHECK_EQ(0, sc1PTrainingLeftoverRulesGetHoldBarWidth(40, 0));
}

static void testOnlyTheRemixStandardItemsHaveAHeldName(void)
{
    CHECK_EQ(0, sc1PTrainingLeftoverRulesGetHeldItemEntry(0x2D, 0x2D, 13));
    CHECK_EQ(12, sc1PTrainingLeftoverRulesGetHeldItemEntry(0x39, 0x2D, 13));
    CHECK_EQ(-1, sc1PTrainingLeftoverRulesGetHeldItemEntry(0x3A, 0x2D, 13));
    CHECK_EQ(-1, sc1PTrainingLeftoverRulesGetHeldItemEntry(0x2C, 0x2D, 13));
    CHECK_EQ(-1, sc1PTrainingLeftoverRulesGetHeldItemEntry(0x05, 0x2D, 13));
}

void remixTestTrainingLeftoverRules(void)
{
    printf("training leftover rules\n");
    RUN_TEST(testTheFrameCountGrowsOutsideHitlag);
    RUN_TEST(testTheFrameCountCountsTheFirstHitlagFrameOnly);
    RUN_TEST(testTheFrameCountCountsTheFrameAfterHitlagEnds);
    RUN_TEST(testANewActionRestartsTheCountAtOne);
    RUN_TEST(testActionsBelow0xDCUseTheSharedTable);
    RUN_TEST(testTheCharacterTableStartsAt0xDC);
    RUN_TEST(testHoldExitCountsHeldFramesAndExitsOnThe33rd);
    RUN_TEST(testHoldExitDoesNotExitOnThe32ndFrame);
    RUN_TEST(testReleasingAClearsTheHeldFrames);
    RUN_TEST(testTheHoldBarGrowsByTwoUpTo64);
    RUN_TEST(testTheHoldBarEmptiesWhenAIsReleased);
    RUN_TEST(testOnlyTheRemixStandardItemsHaveAHeldName);
}
