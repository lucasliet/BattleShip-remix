/**
 * test_ai_level_rules.c - unit tests of the Remix AI level clamp rules (ft/ftremix/ftremixailevelrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixailevelrules.h"

static void testLevelNineIsNotClamped(void)
{
    CHECK_EQ(9, ftRemixAiLevelClamp(9));
}

static void testLevelTenIsClampedToNine(void)
{
    CHECK_EQ(9, ftRemixAiLevelClamp(10));
}

static void testLevelOneIsNotClamped(void)
{
    CHECK_EQ(1, ftRemixAiLevelClamp(1));
}

static void testWalkStopThresholdAtLevelNineIsZero(void)
{
    CHECK_EQ(0, ftRemixAiLevelWalkStopThreshold(9));
}

static void testWalkStopThresholdAtLevelFiveIsEight(void)
{
    CHECK_EQ(8, ftRemixAiLevelWalkStopThreshold(5));
}

static void testWalkStopThresholdAtLevelTenIsFlooredAtZero(void)
{
    CHECK_EQ(0, ftRemixAiLevelWalkStopThreshold(10));
}

static void testCollisionScaleBelowTenIsOne(void)
{
    CHECK_TRUE(ftRemixAiLevelCollisionScale(9) == 1.0F);
}

static void testCollisionScaleAtTenUnderestimates(void)
{
    CHECK_TRUE(ftRemixAiLevelCollisionScale(10) == 0.875F);
}

void remixTestAiLevelRules(void)
{
    printf("ai level rules\n");
    RUN_TEST(testLevelNineIsNotClamped);
    RUN_TEST(testLevelTenIsClampedToNine);
    RUN_TEST(testLevelOneIsNotClamped);
    RUN_TEST(testWalkStopThresholdAtLevelNineIsZero);
    RUN_TEST(testWalkStopThresholdAtLevelFiveIsEight);
    RUN_TEST(testWalkStopThresholdAtLevelTenIsFlooredAtZero);
    RUN_TEST(testCollisionScaleBelowTenIsOne);
    RUN_TEST(testCollisionScaleAtTenUnderestimates);
}
