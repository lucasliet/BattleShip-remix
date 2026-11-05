/**
 * test_ai_cpuinput_rules.c - unit tests of the CPU-only inputs inside shared statuses
 * (ft/ftremix/ftremixaicpuinputrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaicpuinputrules.h"

static void testLevelTenCpuGrabsDuringTheJabFromTheThirdFrame(void)
{
    CHECK_TRUE(ftRemixAiJabGrabRulesApplies(1, 10, 3, 0));
    CHECK_TRUE(ftRemixAiJabGrabRulesApplies(1, 10, 40, 0));
}

static void testJabGrabWaitsForTheThirdFrame(void)
{
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 10, 2, 0));
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 10, 0, 0));
}

static void testJabGrabIgnoresHumansAndLowerLevels(void)
{
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(0, 10, 10, 0));
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 9, 10, 0));
}

static void testJabGrabFollowsSingleButtonMode(void)
{
    CHECK_TRUE(ftRemixAiJabGrabRulesApplies(1, 10, 10, 3));
    CHECK_TRUE(ftRemixAiJabGrabRulesApplies(1, 10, 10, 6));
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 10, 10, 1));
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 10, 10, 2));
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 10, 10, 4));
    CHECK_EQ(0, ftRemixAiJabGrabRulesApplies(1, 10, 10, 5));
}

static void testOnlyACpuGigaBowserWalksInsteadOfDashing(void)
{
    CHECK_TRUE(ftRemixAiDashRulesUsesWalk(1, 1));
    CHECK_EQ(0, ftRemixAiDashRulesUsesWalk(1, 0));
    CHECK_EQ(0, ftRemixAiDashRulesUsesWalk(0, 1));
    CHECK_EQ(0, ftRemixAiDashRulesUsesWalk(0, 0));
}

void remixTestAiCpuInputRules(void)
{
    printf("ai cpu input rules\n");
    RUN_TEST(testLevelTenCpuGrabsDuringTheJabFromTheThirdFrame);
    RUN_TEST(testJabGrabWaitsForTheThirdFrame);
    RUN_TEST(testJabGrabIgnoresHumansAndLowerLevels);
    RUN_TEST(testJabGrabFollowsSingleButtonMode);
    RUN_TEST(testOnlyACpuGigaBowserWalksInsteadOfDashing);
}
