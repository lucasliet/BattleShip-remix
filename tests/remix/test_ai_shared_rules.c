/**
 * test_ai_shared_rules.c - unit tests of the Remix AI family shared hooks (ft/ftremix/ftremixaisharedrules.c) and of the Zebes
 * acid stage rule (ft/ftremix/ftremixaistagerules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixids.h"
#include "ft/ftremix/ftremixaisharedrules.h"
#include "ft/ftremix/ftremixaistagerules.h"

static void testMarinaCargoStatusesAreTheCarryLoop(void)
{
    CHECK_TRUE(ftRemixAiSharedIsMarinaCargoStatus(0xEB));
    CHECK_TRUE(ftRemixAiSharedIsMarinaCargoStatus(0xEE));
    CHECK_TRUE(ftRemixAiSharedIsMarinaCargoStatus(0xEF));
}

static void testMarinaStatusesOutsideCargoAreNotCarry(void)
{
    CHECK_EQ(0, ftRemixAiSharedIsMarinaCargoStatus(0xEA));
    CHECK_EQ(0, ftRemixAiSharedIsMarinaCargoStatus(0xF0));
}

static void testKirbyWithoutDededeHatRunsTheKirbyBlock(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_KIRBY_BLOCK, ftRemixAiSharedInhaleOutcome(1, 0, 0, 0x100));
}

static void testKirbyHatBelowTheDededeHatRunsTheKirbyBlock(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_KIRBY_BLOCK, ftRemixAiSharedInhaleOutcome(1, 0, 0x1F, 0x100));
}

static void testKirbyWithDededeHatSpitsInTheInhaledStatuses(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_PRESS_NSP, ftRemixAiSharedInhaleOutcome(1, 0, 0x20, 0x170));
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_PRESS_NSP, ftRemixAiSharedInhaleOutcome(1, 0, 0x20, 0x178));
}

static void testKirbyWithDededeHatSkipsOtherStatuses(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_SKIP, ftRemixAiSharedInhaleOutcome(1, 0, 0x20, 0x171));
}

static void testNegativeHatCountsAsDededeHat(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_SKIP, ftRemixAiSharedInhaleOutcome(1, 0, -1, 0x100));
}

static void testDededeSpitsInHisInhaledStatuses(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_PRESS_NSP, ftRemixAiSharedInhaleOutcome(0, 1, 0, 0xEC));
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_PRESS_NSP, ftRemixAiSharedInhaleOutcome(0, 1, 0, 0xF4));
}

static void testDededeSkipsOtherStatuses(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_SKIP, ftRemixAiSharedInhaleOutcome(0, 1, 0, 0xE5));
}

static void testOtherFightersSkipTheInhaleLogic(void)
{
    CHECK_EQ(FTREMIX_AI_SHARED_INHALE_SKIP, ftRemixAiSharedInhaleOutcome(0, 0, 0, 0xEC));
}

static void testDededeLeavesTheWalkOnlyInUspMove(void)
{
    CHECK_TRUE(ftRemixAiSharedWalkExitsInDededeUsp(0xE5));
    CHECK_EQ(0, ftRemixAiSharedWalkExitsInDededeUsp(0xE4));
}

static void testMarinaUsesUspOnlyAfterSixJumps(void)
{
    CHECK_TRUE(ftRemixAiSharedMarinaMayUseUsp(6));
    CHECK_EQ(0, ftRemixAiSharedMarinaMayUseUsp(5));
    CHECK_EQ(0, ftRemixAiSharedMarinaMayUseUsp(0));
}

static void testPkThunderAimsAtTheLedgeWhileYoung(void)
{
    CHECK_TRUE(ftRemixAiSharedPkThunderAimsAtLedge(0));
    CHECK_TRUE(ftRemixAiSharedPkThunderAimsAtLedge(0x3B));
    CHECK_EQ(0, ftRemixAiSharedPkThunderAimsAtLedge(0x3C));
}

static void testPkThunderOffsetIsZeroWhenLedgeIsFiveHundredBelow(void)
{
    CHECK_TRUE(ftRemixAiSharedPkThunderAimOffset(0.0F, 0.0F, 1000.0F, -500.0F) == 0.0F);
}

static void testPkThunderOffsetGrowsWithLedgeHeight(void)
{
    float low = ftRemixAiSharedPkThunderAimOffset(0.0F, 0.0F, 1000.0F, 0.0F);
    float high = ftRemixAiSharedPkThunderAimOffset(0.0F, 0.0F, 1000.0F, 1000.0F);

    CHECK_TRUE(low > 0.0F);
    CHECK_TRUE(high > low);
}

static void testPkThunderOffsetIgnoresTheLedgeSide(void)
{
    float right = ftRemixAiSharedPkThunderAimOffset(0.0F, 0.0F, 1000.0F, 300.0F);
    float left = ftRemixAiSharedPkThunderAimOffset(0.0F, 0.0F, -1000.0F, 300.0F);

    CHECK_TRUE(right == left);
}

static void testPkThunderOffsetIsFiveHundredTwelveTimesAtanForEqualDistances(void)
{
    float offset = ftRemixAiSharedPkThunderAimOffset(0.0F, 0.0F, 500.0F, 0.0F);

    CHECK_TRUE((offset > 401.9F) && (offset < 402.2F));
}

static void testZebesBasedStagesHaveAcidWithHazardsOn(void)
{
    CHECK_TRUE(ftRemixAiStageHasZebesAcid(nGRRemixStage_PLANET_ZEBES, 1));
    CHECK_TRUE(ftRemixAiStageHasZebesAcid(nGRRemixStage_NORFAIR, 1));
    CHECK_TRUE(ftRemixAiStageHasZebesAcid(nGRRemixStage_ZEBES_DL, 1));
    CHECK_TRUE(ftRemixAiStageHasZebesAcid(nGRRemixStage_ZEBES_O, 1));
    CHECK_TRUE(ftRemixAiStageHasZebesAcid(nGRRemixStage_NORFAIR_REMIX, 1));
}

static void testZebesBasedStagesHaveNoAcidWithHazardsOff(void)
{
    CHECK_EQ(0, ftRemixAiStageHasZebesAcid(nGRRemixStage_PLANET_ZEBES, 0));
    CHECK_EQ(0, ftRemixAiStageHasZebesAcid(nGRRemixStage_NORFAIR, 0));
}

static void testOtherStagesHaveNoZebesAcid(void)
{
    CHECK_EQ(0, ftRemixAiStageHasZebesAcid(nGRRemixStage_PEACHS_CASTLE, 1));
    CHECK_EQ(0, ftRemixAiStageHasZebesAcid(nGRRemixStage_HYRULE_CASTLE, 1));
}

void remixTestAiSharedRules(void)
{
    printf("ai shared rules\n");
    RUN_TEST(testMarinaCargoStatusesAreTheCarryLoop);
    RUN_TEST(testMarinaStatusesOutsideCargoAreNotCarry);
    RUN_TEST(testKirbyWithoutDededeHatRunsTheKirbyBlock);
    RUN_TEST(testKirbyHatBelowTheDededeHatRunsTheKirbyBlock);
    RUN_TEST(testKirbyWithDededeHatSpitsInTheInhaledStatuses);
    RUN_TEST(testKirbyWithDededeHatSkipsOtherStatuses);
    RUN_TEST(testNegativeHatCountsAsDededeHat);
    RUN_TEST(testDededeSpitsInHisInhaledStatuses);
    RUN_TEST(testDededeSkipsOtherStatuses);
    RUN_TEST(testOtherFightersSkipTheInhaleLogic);
    RUN_TEST(testDededeLeavesTheWalkOnlyInUspMove);
    RUN_TEST(testMarinaUsesUspOnlyAfterSixJumps);
    RUN_TEST(testPkThunderAimsAtTheLedgeWhileYoung);
    RUN_TEST(testPkThunderOffsetIsZeroWhenLedgeIsFiveHundredBelow);
    RUN_TEST(testPkThunderOffsetGrowsWithLedgeHeight);
    RUN_TEST(testPkThunderOffsetIgnoresTheLedgeSide);
    RUN_TEST(testPkThunderOffsetIsFiveHundredTwelveTimesAtanForEqualDistances);
    RUN_TEST(testZebesBasedStagesHaveAcidWithHazardsOn);
    RUN_TEST(testZebesBasedStagesHaveNoAcidWithHazardsOff);
    RUN_TEST(testOtherStagesHaveNoZebesAcid);
}
