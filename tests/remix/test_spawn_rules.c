/**
 * test_spawn_rules.c - unit tests of the Remix spawn table rules (gm/gmremixspawnrules.c, Spawn.asm load_spawn_).
 */

#include "test_check.h"

#include "gm/gmremixspawnrules.h"

static void testNeutralTableNeedsTheToggleAndASlot(void)
{
    CHECK_EQ(1, gmRemixSpawnRulesVsUsesNeutral(1, 0));
    CHECK_EQ(1, gmRemixSpawnRulesVsUsesNeutral(1, 3));
    CHECK_EQ(0, gmRemixSpawnRulesVsUsesNeutral(0, 2));
    CHECK_EQ(0, gmRemixSpawnRulesVsUsesNeutral(1, -1));
    CHECK_EQ(0, gmRemixSpawnRulesVsUsesNeutral(0, -1));
}

static void testVsSlotIsTheNeutralSlotOrThePlayer(void)
{
    CHECK_EQ(2, gmRemixSpawnRulesVsSlot(1, 1, 2));
    CHECK_EQ(1, gmRemixSpawnRulesVsSlot(1, 0, 2));
    CHECK_EQ(3, gmRemixSpawnRulesVsSlot(3, 0, -1));
}

static void testTrainingTakesTheOriginalSlotOfPortSpawnIds(void)
{
    CHECK_EQ(0, gmRemixSpawnRulesTrainingSlot(0));
    CHECK_EQ(3, gmRemixSpawnRulesTrainingSlot(3));
}

static void testTrainingCustomSpawnHasNoTableSlot(void)
{
    CHECK_EQ(-1, gmRemixSpawnRulesTrainingSlot(4));
    CHECK_EQ(-1, gmRemixSpawnRulesTrainingSlot(-1));
}

static void testTableOffsetIsStageRowPlusSlot(void)
{
    CHECK_EQ(0, gmRemixSpawnRulesTableOffset(0, 0));
    CHECK_EQ(8, gmRemixSpawnRulesTableOffset(0, 1));
    CHECK_EQ(0x20 + 0x18, gmRemixSpawnRulesTableOffset(1, 3));
    CHECK_EQ(0xD7 * 0x20, gmRemixSpawnRulesTableOffset(0xD7, 0));
}

static void testTableOffsetRejectsOutOfRangeStageOrSlot(void)
{
    CHECK_EQ(-1, gmRemixSpawnRulesTableOffset(0xD8, 0));
    CHECK_EQ(-1, gmRemixSpawnRulesTableOffset(-1, 0));
    CHECK_EQ(-1, gmRemixSpawnRulesTableOffset(5, 4));
    CHECK_EQ(-1, gmRemixSpawnRulesTableOffset(5, -1));
}

void remixTestSpawnRules(void)
{
    printf("spawn table rules\n");
    RUN_TEST(testNeutralTableNeedsTheToggleAndASlot);
    RUN_TEST(testVsSlotIsTheNeutralSlotOrThePlayer);
    RUN_TEST(testTrainingTakesTheOriginalSlotOfPortSpawnIds);
    RUN_TEST(testTrainingCustomSpawnHasNoTableSlot);
    RUN_TEST(testTableOffsetIsStageRowPlusSlot);
    RUN_TEST(testTableOffsetRejectsOutOfRangeStageOrSlot);
}
