/**
 * test_surface_rules.c - unit tests of the Remix surface knockback rules (gr/grremix/grremixsurfacerules.c).
 */

#include "test_check.h"

#include "gr/grremix/grremixsurfacerules.h"

static const GRRemixSurfaceKnockback* knockbackOf(unsigned int material)
{
    return grRemixSurfaceRulesGetRow(grRemixSurfaceRulesFindRow(material));
}

static void testOriginalStructsKeepTheVanillaMaterials(void)
{
    CHECK_EQ(4, knockbackOf(0x07)->kind);
    CHECK_EQ(5, knockbackOf(0x08)->kind);
    CHECK_EQ(6, knockbackOf(0x09)->kind);
    CHECK_EQ(7, knockbackOf(0x0A)->kind);
    CHECK_EQ(8, knockbackOf(0x0B)->kind);
    CHECK_EQ(9, knockbackOf(0x0F)->kind);
    CHECK_EQ(-1, knockbackOf(0x0F)->fgm_id);
}

static void testMaterialsWithoutAKnockbackStructHaveNoRow(void)
{
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x00));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x06));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x0C));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x0E));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x11));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x1F));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x23));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(GRREMIX_SURFACE_RULES_BOUNCE_MATERIAL));
    CHECK_EQ(-1, grRemixSurfaceRulesFindRow(0x25));
}

static void testEveryRowIsFoundByItsOwnMaterial(void)
{
    int row;

    for (row = 0; row < GRREMIX_SURFACE_RULES_ROW_COUNT; row++)
    {
        CHECK_EQ(row, grRemixSurfaceRulesFindRow(grRemixSurfaceRulesGetRow(row)->material));
    }
    CHECK_TRUE(grRemixSurfaceRulesGetRow(-1) == 0);
    CHECK_TRUE(grRemixSurfaceRulesGetRow(GRREMIX_SURFACE_RULES_ROW_COUNT) == 0);
}

static void testFailedZCancelSurfacesAreCruelWithTheirOwnSound(void)
{
    const GRRemixSurfaceKnockback *on = knockbackOf(0x12);
    const GRRemixSurfaceKnockback *lava = knockbackOf(0x1E);

    CHECK_EQ(0x00010008, on->kind);
    CHECK_EQ(7, on->damage);
    CHECK_EQ(90, on->angle);
    CHECK_EQ(0x20, on->base);
    CHECK_EQ(0x38, on->fgm_id);
    CHECK_TRUE(grRemixSurfaceRulesIsCruel(on));
    CHECK_EQ(0x00010005, lava->kind);
    CHECK_EQ(10, lava->damage);
    CHECK_EQ(200, lava->fixed);
    CHECK_EQ(0x11E, lava->fgm_id);
    CHECK_TRUE(grRemixSurfaceRulesIsCruel(lava));
}

static void testStageSurfacesAreNotCruel(void)
{
    CHECK_TRUE(!grRemixSurfaceRulesIsCruel(knockbackOf(0x10)));
    CHECK_TRUE(!grRemixSurfaceRulesIsCruel(knockbackOf(0x16)));
    CHECK_TRUE(!grRemixSurfaceRulesIsCruel(knockbackOf(0x1D)));
    CHECK_TRUE(!grRemixSurfaceRulesIsCruel(knockbackOf(0x0A)));
}

static void testBigBlueCorneriaAndMuteCityValues(void)
{
    CHECK_EQ(105, knockbackOf(0x10)->angle);
    CHECK_EQ(125, knockbackOf(0x10)->base);
    CHECK_EQ(90, knockbackOf(0x13)->angle);
    CHECK_EQ(130, knockbackOf(0x13)->base);
    CHECK_EQ(60, knockbackOf(0x14)->damage);
    CHECK_EQ(0x3C, knockbackOf(0x14)->fgm_id);
    CHECK_EQ(30, knockbackOf(0x15)->damage);
    CHECK_EQ(0x16, knockbackOf(0x15)->fgm_id);
    CHECK_EQ(20, knockbackOf(0x16)->kind);
    CHECK_EQ(15, knockbackOf(0x16)->damage);
    CHECK_EQ(0x1F, knockbackOf(0x16)->fgm_id);
}

static void testCasinoBumpersShareTheBumperSound(void)
{
    unsigned int material;

    for (material = 0x17; material <= 0x1C; material++)
    {
        CHECK_EQ(0x3D6, knockbackOf(material)->fgm_id);
        CHECK_EQ(10, knockbackOf(material)->damage);
        CHECK_EQ(65, knockbackOf(material)->base);
    }
    CHECK_EQ(315, knockbackOf(0x17)->angle);
    CHECK_EQ(225, knockbackOf(0x1A)->angle);
    CHECK_EQ(80, knockbackOf(0x1B)->growth);
    CHECK_EQ(0, knockbackOf(0x1C)->growth);
}

static void testTurnpikeCarKnocksUpWithFixedKnockback(void)
{
    CHECK_EQ(8, knockbackOf(0x1D)->kind);
    CHECK_EQ(20, knockbackOf(0x1D)->damage);
    CHECK_EQ(200, knockbackOf(0x1D)->fixed);
    CHECK_EQ(0x11F, knockbackOf(0x1D)->fgm_id);
}

static void testHazardModeOffBlocksOnlyTheNonCruelStructs(void)
{
    CHECK_TRUE(grRemixSurfaceRulesIsBlockedByHazardMode(knockbackOf(0x08), 0));
    CHECK_TRUE(grRemixSurfaceRulesIsBlockedByHazardMode(knockbackOf(0x16), 0));
    CHECK_TRUE(!grRemixSurfaceRulesIsBlockedByHazardMode(knockbackOf(0x08), 1));
    CHECK_TRUE(!grRemixSurfaceRulesIsBlockedByHazardMode(knockbackOf(0x12), 0));
    CHECK_TRUE(!grRemixSurfaceRulesIsBlockedByHazardMode(knockbackOf(0x1E), 0));
}

static void testBumperWallsBelowTheFirstBumperAreIgnored(void)
{
    CHECK_EQ(-1, grRemixSurfaceRulesBumperMaterial(0x00, 1, 1));
    CHECK_EQ(-1, grRemixSurfaceRulesBumperMaterial(0x16, 1, -1));
    CHECK_EQ(-1, grRemixSurfaceRulesBumperMaterial(0x16, 0, 1));
}

static void testBumperMaterialFollowsTheFacingOfTheFighter(void)
{
    CHECK_EQ(0x1A, grRemixSurfaceRulesBumperMaterial(0x1A, 1, -1));
    CHECK_EQ(0x17, grRemixSurfaceRulesBumperMaterial(0x1A, 1, 1));
    CHECK_EQ(0x17, grRemixSurfaceRulesBumperMaterial(0x17, 0, -1));
    CHECK_EQ(0x1A, grRemixSurfaceRulesBumperMaterial(0x17, 0, 1));
    CHECK_EQ(0x1C, grRemixSurfaceRulesBumperMaterial(0x19, 0, 1));
}

static void testBumperIgnoresFightersInDamageStatuses(void)
{
    CHECK_TRUE(grRemixSurfaceRulesBumperStatusApplies(0x24));
    CHECK_TRUE(!grRemixSurfaceRulesBumperStatusApplies(0x25));
    CHECK_TRUE(!grRemixSurfaceRulesBumperStatusApplies(0x30));
    CHECK_TRUE(!grRemixSurfaceRulesBumperStatusApplies(0x37));
    CHECK_TRUE(grRemixSurfaceRulesBumperStatusApplies(0x38));
    CHECK_TRUE(grRemixSurfaceRulesBumperStatusApplies(0x0A));
}

void remixTestSurfaceRules(void)
{
    RUN_TEST(testOriginalStructsKeepTheVanillaMaterials);
    RUN_TEST(testMaterialsWithoutAKnockbackStructHaveNoRow);
    RUN_TEST(testEveryRowIsFoundByItsOwnMaterial);
    RUN_TEST(testFailedZCancelSurfacesAreCruelWithTheirOwnSound);
    RUN_TEST(testStageSurfacesAreNotCruel);
    RUN_TEST(testBigBlueCorneriaAndMuteCityValues);
    RUN_TEST(testCasinoBumpersShareTheBumperSound);
    RUN_TEST(testTurnpikeCarKnocksUpWithFixedKnockback);
    RUN_TEST(testHazardModeOffBlocksOnlyTheNonCruelStructs);
    RUN_TEST(testBumperWallsBelowTheFirstBumperAreIgnored);
    RUN_TEST(testBumperMaterialFollowsTheFacingOfTheFighter);
    RUN_TEST(testBumperIgnoresFightersInDamageStatuses);
}
