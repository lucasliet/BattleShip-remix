/**
 * test_training_port_rules.c - unit tests of the Training per-port chain rules (sc/sc1pmode/sc1ptrainingportrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1ptrainingportrules.h"

static void testAutoGivesTheMenuOwnerHumanAndTheSelectCpuCpu(void)
{
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_HUMAN, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_AUTO, 0, 0, 1));
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_CPU, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_AUTO, 1, 0, 1));
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_DISABLED, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_AUTO, 2, 0, 1));
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_DISABLED, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_AUTO, 3, 0, 1));
}

static void testExplicitTypesOverrideAutoOnOtherPorts(void)
{
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_CPU, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_CPU, 2, 0, 1));
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_DISABLED, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_DISABLED, 1, 0, 1));
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_HUMAN, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_HUMAN, 3, 0, 1));
}

static void testTheMenuOwnerCannotBeDisabledOrCpu(void)
{
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_HUMAN, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_DISABLED, 2, 2, 0));
    CHECK_EQ(SC1PTRAININGPORTRULES_TYPE_HUMAN, sc1PTrainingPortRulesResolveType(SC1PTRAININGPORTRULES_TYPE_CPU, 0, 0, 1));
}

static void testTheDummyIsTheFirstCpu(void)
{
    int32_t types[SC1PTRAININGPORTRULES_PORT_COUNT] = { SC1PTRAININGPORTRULES_TYPE_HUMAN, SC1PTRAININGPORTRULES_TYPE_DISABLED,
                                                        SC1PTRAININGPORTRULES_TYPE_CPU, SC1PTRAININGPORTRULES_TYPE_CPU };

    CHECK_EQ(2, sc1PTrainingPortRulesPickDummy(types, 0));
}

static void testWithoutACpuTheDummyIsTheMenuOwner(void)
{
    int32_t types[SC1PTRAININGPORTRULES_PORT_COUNT] = { SC1PTRAININGPORTRULES_TYPE_DISABLED, SC1PTRAININGPORTRULES_TYPE_HUMAN,
                                                        SC1PTRAININGPORTRULES_TYPE_DISABLED, SC1PTRAININGPORTRULES_TYPE_DISABLED };

    CHECK_EQ(1, sc1PTrainingPortRulesPickDummy(types, 1));
}

static void testCountsPerType(void)
{
    int32_t types[SC1PTRAININGPORTRULES_PORT_COUNT] = { SC1PTRAININGPORTRULES_TYPE_HUMAN, SC1PTRAININGPORTRULES_TYPE_CPU,
                                                        SC1PTRAININGPORTRULES_TYPE_CPU, SC1PTRAININGPORTRULES_TYPE_DISABLED };

    CHECK_EQ(1, sc1PTrainingPortRulesCountType(types, SC1PTRAININGPORTRULES_TYPE_HUMAN));
    CHECK_EQ(2, sc1PTrainingPortRulesCountType(types, SC1PTRAININGPORTRULES_TYPE_CPU));
    CHECK_EQ(1, sc1PTrainingPortRulesCountType(types, SC1PTRAININGPORTRULES_TYPE_DISABLED));
}

static void testPickKeepsTheAutoValueForEntryZero(void)
{
    CHECK_EQ(8, sc1PTrainingPortRulesPick(0, 8));
    CHECK_EQ(0, sc1PTrainingPortRulesPick(1, 8));
    CHECK_EQ(5, sc1PTrainingPortRulesPick(6, 8));
    CHECK_EQ(8, sc1PTrainingPortRulesPick(-3, 8));
}

static void testSpawnFollowsThePortUnlessChosen(void)
{
    CHECK_EQ(2, sc1PTrainingPortRulesSpawnPort(SC1PTRAININGPORTRULES_SPAWN_AUTO, 2));
    CHECK_EQ(0, sc1PTrainingPortRulesSpawnPort(1, 3));
    CHECK_EQ(3, sc1PTrainingPortRulesSpawnPort(4, 0));
    CHECK_EQ(1, sc1PTrainingPortRulesSpawnPort(5, 1));
    CHECK_EQ(1, sc1PTrainingPortRulesSpawnPort(9, 1));
}

void remixTestTrainingPortRules(void)
{
    printf("training per-port chain rules\n");
    RUN_TEST(testAutoGivesTheMenuOwnerHumanAndTheSelectCpuCpu);
    RUN_TEST(testExplicitTypesOverrideAutoOnOtherPorts);
    RUN_TEST(testTheMenuOwnerCannotBeDisabledOrCpu);
    RUN_TEST(testTheDummyIsTheFirstCpu);
    RUN_TEST(testWithoutACpuTheDummyIsTheMenuOwner);
    RUN_TEST(testCountsPerType);
    RUN_TEST(testPickKeepsTheAutoValueForEntryZero);
    RUN_TEST(testSpawnFollowsThePortUnlessChosen);
}
