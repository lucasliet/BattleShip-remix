/**
 * test_ai_longrange_rules.c - unit tests of the Remix LONG_RANGE rules (ft/ftremix/ftremixailongrangerules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixailongrangerules.h"

static void testVanillaFightersKeepTheVanillaSwitch(void)
{
    CHECK_EQ(0, ftRemixAiLongRangeRulesUsesTable(0x00));
    CHECK_EQ(0, ftRemixAiLongRangeRulesUsesTable(0x0D));
    CHECK_EQ(0, ftRemixAiLongRangeRulesUsesTable(0x1A));
    CHECK_EQ(0, ftRemixAiLongRangeRulesUsesTable(0x1D));
}

static void testRemixFightersReadTheTable(void)
{
    CHECK_TRUE(ftRemixAiLongRangeRulesUsesTable(0x0E));
    CHECK_TRUE(ftRemixAiLongRangeRulesUsesTable(0x1E));
    CHECK_TRUE(ftRemixAiLongRangeRulesUsesTable(0x44));
}

static void testPolygonVariantsAreDetected(void)
{
    CHECK_TRUE(ftRemixAiLongRangeRulesIsPolygon(0x0E));
    CHECK_TRUE(ftRemixAiLongRangeRulesIsPolygon(0x19));
    CHECK_TRUE(ftRemixAiLongRangeRulesIsPolygon(0x4D));
    CHECK_TRUE(ftRemixAiLongRangeRulesIsPolygon(0x60));
}

static void testNonPolygonFightersAreNotPolygons(void)
{
    CHECK_EQ(0, ftRemixAiLongRangeRulesIsPolygon(0x0D));
    CHECK_EQ(0, ftRemixAiLongRangeRulesIsPolygon(0x1A));
    CHECK_EQ(0, ftRemixAiLongRangeRulesIsPolygon(0x1D));
    CHECK_EQ(0, ftRemixAiLongRangeRulesIsPolygon(0x4C));
}

static void testVanillaRoutineAddressesMapToKinds(void)
{
    CHECK_EQ(FTREMIX_AI_LONG_RANGE_KIND_NSP_SHOOT, ftRemixAiLongRangeRulesRoutineKind(0x80138D24u));
    CHECK_EQ(FTREMIX_AI_LONG_RANGE_KIND_PHYSICAL_SHOOT, ftRemixAiLongRangeRulesRoutineKind(0x80138CD4u));
    CHECK_EQ(FTREMIX_AI_LONG_RANGE_KIND_NONE, ftRemixAiLongRangeRulesRoutineKind(0x80138ECCu));
}

static void testUnknownRoutineAddressIsNotMapped(void)
{
    CHECK_EQ(FTREMIX_AI_LONG_RANGE_KIND_UNKNOWN, ftRemixAiLongRangeRulesRoutineKind(0u));
    CHECK_EQ(FTREMIX_AI_LONG_RANGE_KIND_UNKNOWN, ftRemixAiLongRangeRulesRoutineKind(0x80138D28u));
}

static void testNonZeroTableCommandReplacesTheDefault(void)
{
    CHECK_EQ(0x12, ftRemixAiLongRangeRulesCustomCommand(0x12, 0x0A));
}

static void testZeroTableCommandKeepsTheDefault(void)
{
    CHECK_EQ(0x0A, ftRemixAiLongRangeRulesCustomCommand(0, 0x0A));
}

void remixTestAiLongRangeRules(void)
{
    printf("ai long range rules\n");
    RUN_TEST(testVanillaFightersKeepTheVanillaSwitch);
    RUN_TEST(testRemixFightersReadTheTable);
    RUN_TEST(testPolygonVariantsAreDetected);
    RUN_TEST(testNonPolygonFightersAreNotPolygons);
    RUN_TEST(testVanillaRoutineAddressesMapToKinds);
    RUN_TEST(testUnknownRoutineAddressIsNotMapped);
    RUN_TEST(testNonZeroTableCommandReplacesTheDefault);
    RUN_TEST(testZeroTableCommandKeepsTheDefault);
}
