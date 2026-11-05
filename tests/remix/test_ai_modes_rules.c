/**
 * test_ai_modes_rules.c - unit tests of the CPU rules of the Remix game modes (ft/ftremix/ftremixaimodesrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaimodesrules.h"

static void testHomeTeamSeeksBehindTheBallOnTheLeft(void)
{
    CHECK_EQ(-400, (int)ftRemixAiModesBallSeekOffset(1, 1));
}

static void testAwayTeamSeeksBehindTheBallOnTheRight(void)
{
    CHECK_EQ(400, (int)ftRemixAiModesBallSeekOffset(0, 1));
    CHECK_EQ(400, (int)ftRemixAiModesBallSeekOffset(2, 1));
}

static void testSquaredDistanceIsThePythagoreanSum(void)
{
    CHECK_EQ(25, (int)ftRemixAiModesSquaredDistance(0.0F, 0.0F, 3.0F, 4.0F));
    CHECK_EQ(25, (int)ftRemixAiModesSquaredDistance(3.0F, 4.0F, 0.0F, 0.0F));
    CHECK_EQ(0, (int)ftRemixAiModesSquaredDistance(7.0F, -2.0F, 7.0F, -2.0F));
}

static void testHillTargetIsFiveHundredBelowTheMarker(void)
{
    CHECK_EQ(100, (int)ftRemixAiModesHillTargetY(600.0F));
    CHECK_EQ(-500, (int)ftRemixAiModesHillTargetY(0.0F));
}

void remixTestAiModesRules(void)
{
    printf("ai modes rules\n");
    RUN_TEST(testHomeTeamSeeksBehindTheBallOnTheLeft);
    RUN_TEST(testAwayTeamSeeksBehindTheBallOnTheRight);
    RUN_TEST(testSquaredDistanceIsThePythagoreanSum);
    RUN_TEST(testHillTargetIsFiveHundredBelowTheMarker);
}
