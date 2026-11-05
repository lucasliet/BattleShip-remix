/**
 * test_kirbyspawn_rules.c - unit tests of the CSS Kirby Hat rules (ft/ftremix/ftremixkirbyspawnrules.c): the fighter behind each
 * panel value (KirbyHats.asm spawn_with_table_), the magic hat and taunt_loses_power flags a spawn sets (Size.asm 358-381) and
 * the weapon part offsets of the hats with a weapon (KirbyHats.asm use_extended_special_parts_pause_).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixkirbyspawnrules.h"
#include "remix/remix_roster_data.h"

static void testPanelNoneGivesKirby(void)
{
    CHECK_EQ(REMIX_ID_KIRBY, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_NONE, 0));
}

static void testPanelValuesFollowTheSpawnTable(void)
{
    CHECK_EQ(REMIX_ID_MARIO, ftRemixKirbySpawnPanelFighter(1, 0));
    CHECK_EQ(REMIX_ID_FOX, ftRemixKirbySpawnPanelFighter(2, 0));
    CHECK_EQ(REMIX_ID_SAMUS, ftRemixKirbySpawnPanelFighter(4, 0));
    CHECK_EQ(REMIX_ID_BOWSER, ftRemixKirbySpawnPanelFighter(17, 0));
    CHECK_EQ(REMIX_ID_MTWO, ftRemixKirbySpawnPanelFighter(20, 0));
    CHECK_EQ(REMIX_ID_DSAMUS, ftRemixKirbySpawnPanelFighter(30, 0));
    CHECK_EQ(REMIX_ID_SSONIC, ftRemixKirbySpawnPanelFighter(40, 0));
}

static void testMagicPanelGivesTheMagicPower(void)
{
    CHECK_EQ(REMIX_ID_NONE, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_MAGIC, 0));
}

static void testRandomRollSkipsTheNoneRow(void)
{
    CHECK_EQ(REMIX_ID_MARIO, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM, 0));
    CHECK_EQ(REMIX_ID_SSONIC, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM, FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT - 1));
}

static void testRandomRollNeverGivesTheMagicPower(void)
{
    int roll;

    for (roll = 0; roll < FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT; roll++)
    {
        CHECK_TRUE(ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM, roll) != REMIX_ID_NONE);
        CHECK_TRUE(ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM, roll) != REMIX_ID_KIRBY);
    }
}

static void testOutOfRangeValuesGiveKirby(void)
{
    CHECK_EQ(REMIX_ID_KIRBY, ftRemixKirbySpawnPanelFighter(-1, 0));
    CHECK_EQ(REMIX_ID_KIRBY, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM + 1, 0));
    CHECK_EQ(REMIX_ID_KIRBY, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM, -1));
    CHECK_EQ(REMIX_ID_KIRBY, ftRemixKirbySpawnPanelFighter(FTREMIX_KIRBYSPAWN_PANEL_RANDOM, FTREMIX_KIRBYSPAWN_PANEL_FIGHTER_COUNT));
}

static void testMagicHatStartsOnForTheMagicPowerOrThePanel(void)
{
    CHECK_TRUE(ftRemixKirbySpawnMagicHatStartsOn(REMIX_ID_NONE, FTREMIX_KIRBYSPAWN_PANEL_NONE));
    CHECK_TRUE(ftRemixKirbySpawnMagicHatStartsOn(REMIX_ID_KIRBY, FTREMIX_KIRBYSPAWN_PANEL_MAGIC));
}

static void testMagicHatStaysOffOtherwise(void)
{
    CHECK_TRUE(!ftRemixKirbySpawnMagicHatStartsOn(REMIX_ID_KIRBY, FTREMIX_KIRBYSPAWN_PANEL_NONE));
    CHECK_TRUE(!ftRemixKirbySpawnMagicHatStartsOn(REMIX_ID_MARIO, 5));
    CHECK_TRUE(!ftRemixKirbySpawnMagicHatStartsOn(REMIX_ID_KIRBY, FTREMIX_KIRBYSPAWN_PANEL_RANDOM));
}

static void testPowerIsLostOnlyWithNoHatChosen(void)
{
    CHECK_TRUE(ftRemixKirbySpawnTauntLosesPower(FTREMIX_KIRBYSPAWN_PANEL_NONE));
    CHECK_TRUE(!ftRemixKirbySpawnTauntLosesPower(1));
    CHECK_TRUE(!ftRemixKirbySpawnTauntLosesPower(FTREMIX_KIRBYSPAWN_PANEL_MAGIC));
    CHECK_TRUE(!ftRemixKirbySpawnTauntLosesPower(FTREMIX_KIRBYSPAWN_PANEL_RANDOM));
}

static void testGunHatsHaveTheirModelFileOffsets(void)
{
    CHECK_EQ(0x1D830u, ftRemixKirbyGunHatOffset(0x1A));
    CHECK_EQ(0x1D860u, ftRemixKirbyGunHatOffset(0x19));
    CHECK_EQ(0x1D890u, ftRemixKirbyGunHatOffset(0x1C));
    CHECK_EQ(0x1D8C0u, ftRemixKirbyGunHatOffset(0x23));
    CHECK_EQ(0x1D8F0u, ftRemixKirbyGunHatOffset(0x24));
    CHECK_EQ(0x1D920u, ftRemixKirbyGunHatOffset(0x22));
    CHECK_EQ(0x1D950u, ftRemixKirbyGunHatOffset(0x28));
    CHECK_EQ(0x1D980u, ftRemixKirbyGunHatOffset(0x2C));
    CHECK_EQ(0x1D9B0u, ftRemixKirbyGunHatOffset(0x2E));
}

static void testHatsWithoutAWeaponHaveNoOffset(void)
{
    CHECK_EQ(0u, ftRemixKirbyGunHatOffset(0));
    CHECK_EQ(0u, ftRemixKirbyGunHatOffset(0x13));
    CHECK_EQ(0u, ftRemixKirbyGunHatOffset(-1));
}

void remixTestKirbySpawnRules(void)
{
    printf("kirby spawn rules\n");
    RUN_TEST(testPanelNoneGivesKirby);
    RUN_TEST(testPanelValuesFollowTheSpawnTable);
    RUN_TEST(testMagicPanelGivesTheMagicPower);
    RUN_TEST(testRandomRollSkipsTheNoneRow);
    RUN_TEST(testRandomRollNeverGivesTheMagicPower);
    RUN_TEST(testOutOfRangeValuesGiveKirby);
    RUN_TEST(testMagicHatStartsOnForTheMagicPowerOrThePanel);
    RUN_TEST(testMagicHatStaysOffOtherwise);
    RUN_TEST(testPowerIsLostOnlyWithNoHatChosen);
    RUN_TEST(testGunHatsHaveTheirModelFileOffsets);
    RUN_TEST(testHatsWithoutAWeaponHaveNoOffset);
}
