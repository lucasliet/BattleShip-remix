/**
 * test_partimage_rules.c - unit tests of the Remix model part image rules (ft/ftremix/ftremixpartimagerules.c): extra costume
 * indexing and the C-button cycle (Costumes.asm), the prim color pick and the Yoshi egg palette index (Shield.asm).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixpartimagerules.h"

/* Mario: costumes 0 to 4 are vanilla, 3 extra costumes (5 to 7), none skipped. */
#define MARIO_NUM 4
#define MARIO_SKIPPED 0
#define MARIO_EXTRA 3

/* A character with skipped ids: vanilla 0 to 3, ids 4 and 5 skipped, 4 extra costumes (6 to 9). */
#define GAP_NUM 3
#define GAP_SKIPPED 2
#define GAP_EXTRA 4

static void testFirstExtraCostumeFollowsTheSkippedIds(void)
{
    CHECK_EQ(5, ftRemixPartImageRulesFirstExtraCostume(MARIO_NUM, MARIO_SKIPPED));
    CHECK_EQ(6, ftRemixPartImageRulesFirstExtraCostume(GAP_NUM, GAP_SKIPPED));
}

static void testExtraIndexCountsFromTheFirstExtraCostume(void)
{
    CHECK_EQ(0, ftRemixPartImageRulesExtraIndex(5, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(2, ftRemixPartImageRulesExtraIndex(7, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(0, ftRemixPartImageRulesExtraIndex(6, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(3, ftRemixPartImageRulesExtraIndex(9, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
}

static void testVanillaSkippedAndPastTheEndIdsAreNotExtra(void)
{
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(0, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(4, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(8, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(4, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(5, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(10, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
}

static void testCharacterWithoutExtraCostumesNeverHasAnExtraIndex(void)
{
    CHECK_EQ(FTREMIX_PARTIMAGE_NO_EXTRA, ftRemixPartImageRulesExtraIndex(5, MARIO_NUM, 0, 0));
    CHECK_EQ(MARIO_NUM, ftRemixPartImageRulesMaxCostume(MARIO_NUM, 0, 0));
}

static void testMaxCostumeAddsSkippedAndExtraIds(void)
{
    CHECK_EQ(7, ftRemixPartImageRulesMaxCostume(MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(9, ftRemixPartImageRulesMaxCostume(GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
}

static void testNextCostumeWalksIntoTheExtraCostumesAndWraps(void)
{
    CHECK_EQ(1, ftRemixPartImageRulesNextCostume(0, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(5, ftRemixPartImageRulesNextCostume(4, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(7, ftRemixPartImageRulesNextCostume(6, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(0, ftRemixPartImageRulesNextCostume(7, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
}

static void testNextCostumeJumpsOverTheSkippedIds(void)
{
    CHECK_EQ(3, ftRemixPartImageRulesNextCostume(2, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(6, ftRemixPartImageRulesNextCostume(3, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(0, ftRemixPartImageRulesNextCostume(9, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
}

static void testPrevCostumeWalksBackAndWrapsToTheLastExtraCostume(void)
{
    CHECK_EQ(0, ftRemixPartImageRulesPrevCostume(1, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(4, ftRemixPartImageRulesPrevCostume(5, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
    CHECK_EQ(7, ftRemixPartImageRulesPrevCostume(0, MARIO_NUM, MARIO_SKIPPED, MARIO_EXTRA));
}

static void testPrevCostumeJumpsOverTheSkippedIds(void)
{
    CHECK_EQ(3, ftRemixPartImageRulesPrevCostume(6, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(6, ftRemixPartImageRulesPrevCostume(7, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
    CHECK_EQ(9, ftRemixPartImageRulesPrevCostume(0, GAP_NUM, GAP_SKIPPED, GAP_EXTRA));
}

static void testCycleWithoutExtraCostumesIsTheVanillaCycle(void)
{
    CHECK_EQ(4, ftRemixPartImageRulesNextCostume(3, MARIO_NUM, 0, 0));
    CHECK_EQ(0, ftRemixPartImageRulesNextCostume(4, MARIO_NUM, 0, 0));
    CHECK_EQ(4, ftRemixPartImageRulesPrevCostume(0, MARIO_NUM, 0, 0));
}

static void testRandomPickSkipsOnlyTheSkippedIds(void)
{
    CHECK_EQ(1, ftRemixPartImageRulesIsLegalRandomCostume(3, GAP_NUM, GAP_SKIPPED));
    CHECK_EQ(0, ftRemixPartImageRulesIsLegalRandomCostume(4, GAP_NUM, GAP_SKIPPED));
    CHECK_EQ(0, ftRemixPartImageRulesIsLegalRandomCostume(5, GAP_NUM, GAP_SKIPPED));
    CHECK_EQ(1, ftRemixPartImageRulesIsLegalRandomCostume(6, GAP_NUM, GAP_SKIPPED));
    CHECK_EQ(1, ftRemixPartImageRulesIsLegalRandomCostume(5, MARIO_NUM, MARIO_SKIPPED));
}

static void testOnlyPositiveSpecialPartIndexUsesTheSpecialArray(void)
{
    CHECK_EQ(0, ftRemixPartImageRulesUsesSpecialArray(-1));
    CHECK_EQ(0, ftRemixPartImageRulesUsesSpecialArray(0));
    CHECK_EQ(1, ftRemixPartImageRulesUsesSpecialArray(1));
}

static void testLowPolyPrimColorFallsBackToHighPolyWhenUnset(void)
{
    CHECK_EQ(0xAABBCCDDu, ftRemixPartImageRulesPickPrimColor(0xAABBCCDDu, 0x11223344u, 0));
    CHECK_EQ(0x11223344u, ftRemixPartImageRulesPickPrimColor(0xAABBCCDDu, 0x11223344u, 1));
    CHECK_EQ(0xAABBCCDDu, ftRemixPartImageRulesPickPrimColor(0xAABBCCDDu, 0, 1));
}

static void testAccessoryImageIndexStartsAtTheTableByte(void)
{
    CHECK_EQ(0, ftRemixPartImageRulesAccessoryImageIndex(0, 1));
    CHECK_EQ(1, ftRemixPartImageRulesAccessoryImageIndex(0, 0));
    CHECK_EQ(3, ftRemixPartImageRulesAccessoryImageIndex(3, 1));
    CHECK_EQ(4, ftRemixPartImageRulesAccessoryImageIndex(3, 0));
}

static void testEggUsesTheCostumeColorOnlyForTheCostumeOption(void)
{
    CHECK_EQ(9, ftRemixPartImageRulesEggPaletteIndex(FTREMIX_PARTIMAGE_SHIELD_COSTUME, 9));
    CHECK_EQ(0, ftRemixPartImageRulesEggPaletteIndex(FTREMIX_PARTIMAGE_SHIELD_VANILLA, 9));
    CHECK_EQ(0, ftRemixPartImageRulesEggPaletteIndex(0, 9));
    CHECK_EQ(0, ftRemixPartImageRulesEggPaletteIndex(5, 9));
}

static void testEggPaletteIndexStaysInsideTheSixteenPalettes(void)
{
    CHECK_EQ(15, ftRemixPartImageRulesEggPaletteIndex(FTREMIX_PARTIMAGE_SHIELD_COSTUME, 15));
    CHECK_EQ(0, ftRemixPartImageRulesEggPaletteIndex(FTREMIX_PARTIMAGE_SHIELD_COSTUME, 16));
    CHECK_EQ(0, ftRemixPartImageRulesEggPaletteIndex(FTREMIX_PARTIMAGE_SHIELD_COSTUME, -1));
}

void remixTestPartImageRules(void)
{
    printf("part image rules\n");
    RUN_TEST(testFirstExtraCostumeFollowsTheSkippedIds);
    RUN_TEST(testExtraIndexCountsFromTheFirstExtraCostume);
    RUN_TEST(testVanillaSkippedAndPastTheEndIdsAreNotExtra);
    RUN_TEST(testCharacterWithoutExtraCostumesNeverHasAnExtraIndex);
    RUN_TEST(testMaxCostumeAddsSkippedAndExtraIds);
    RUN_TEST(testNextCostumeWalksIntoTheExtraCostumesAndWraps);
    RUN_TEST(testNextCostumeJumpsOverTheSkippedIds);
    RUN_TEST(testPrevCostumeWalksBackAndWrapsToTheLastExtraCostume);
    RUN_TEST(testPrevCostumeJumpsOverTheSkippedIds);
    RUN_TEST(testCycleWithoutExtraCostumesIsTheVanillaCycle);
    RUN_TEST(testRandomPickSkipsOnlyTheSkippedIds);
    RUN_TEST(testOnlyPositiveSpecialPartIndexUsesTheSpecialArray);
    RUN_TEST(testLowPolyPrimColorFallsBackToHighPolyWhenUnset);
    RUN_TEST(testAccessoryImageIndexStartsAtTheTableByte);
    RUN_TEST(testEggUsesTheCostumeColorOnlyForTheCostumeOption);
    RUN_TEST(testEggPaletteIndexStaysInsideTheSixteenPalettes);
}
