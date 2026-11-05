/**
 * test_chargeflash_rules.c - unit tests of the Remix gfx_routine_end rule (ft/ftremix/ftremixchargeflashrules.c): which
 * charge glow each fighter restarts when another color animation ends (Character.asm:2179 and the charge_gfx_routine_ rows).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixchargeflashrules.h"
#include "remix/remix_roster_data.h"

#define WORD_LEVEL 0x0ADC
#define WORD_RECOIL 0x0AE0
#define WORD_DEDEDE 0x0AE4

static FTRemixChargeFlash findFlash(int fkind)
{
    FTRemixChargeFlash flash = { 0, 0, 0 };

    CHECK_EQ(1, ftRemixChargeFlashFind(fkind, &flash));
    return flash;
}

static void testSamusClonesUseTheCommonGlowAtLevelSeven(void)
{
    FTRemixChargeFlash flash = findFlash(REMIX_ID_DSAMUS);

    CHECK_EQ(WORD_LEVEL, flash.n64_offset);
    CHECK_EQ(7, flash.full_level);
    CHECK_EQ(FTREMIX_CHARGE_FLASH_COMMON, flash.colanim_id);
    CHECK_EQ(FTREMIX_CHARGE_FLASH_COMMON, findFlash(REMIX_ID_JSAMUS).colanim_id);
    CHECK_EQ(FTREMIX_CHARGE_FLASH_COMMON, findFlash(REMIX_ID_ESAMUS).colanim_id);
    CHECK_EQ(FTREMIX_CHARGE_FLASH_COMMON, findFlash(REMIX_ID_NDSAMUS).colanim_id);
}

static void testJDKUsesTheGiantPunchLevel(void)
{
    FTRemixChargeFlash flash = findFlash(REMIX_ID_JDK);

    CHECK_EQ(WORD_LEVEL, flash.n64_offset);
    CHECK_EQ(10, flash.full_level);
    CHECK_EQ(FTREMIX_CHARGE_FLASH_COMMON, flash.colanim_id);
}

static void testMewtwoAndMarinaReadTheLevelWord(void)
{
    FTRemixChargeFlash mewtwo = findFlash(REMIX_ID_MTWO);
    FTRemixChargeFlash marina = findFlash(REMIX_ID_MARINA);

    CHECK_EQ(WORD_LEVEL, mewtwo.n64_offset);
    CHECK_EQ(7, mewtwo.full_level);
    CHECK_EQ(0x58, mewtwo.colanim_id);
    CHECK_EQ(WORD_LEVEL, marina.n64_offset);
    CHECK_EQ(4, marina.full_level);
    CHECK_EQ(0x72, marina.colanim_id);
}

static void testSheikAndPeppyReadTheRecoilWordAndShareTheSheikGlow(void)
{
    FTRemixChargeFlash sheik = findFlash(REMIX_ID_SHEIK);
    FTRemixChargeFlash peppy = findFlash(REMIX_ID_PEPPY);

    CHECK_EQ(WORD_RECOIL, sheik.n64_offset);
    CHECK_EQ(6, sheik.full_level);
    CHECK_EQ(0x6D, sheik.colanim_id);
    CHECK_EQ(WORD_RECOIL, peppy.n64_offset);
    CHECK_EQ(5, peppy.full_level);
    CHECK_EQ(0x6D, peppy.colanim_id);
}

static void testDededeReadsItsOwnWord(void)
{
    FTRemixChargeFlash flash = findFlash(REMIX_ID_DEDEDE);

    CHECK_EQ(WORD_DEDEDE, flash.n64_offset);
    CHECK_EQ(2, flash.full_level);
    CHECK_EQ(0x74, flash.colanim_id);
}

static void testFightersWithoutAGlowHaveNoRow(void)
{
    FTRemixChargeFlash flash;

    CHECK_EQ(0, ftRemixChargeFlashFind(REMIX_ID_LUCAS, &flash));
    CHECK_EQ(0, ftRemixChargeFlashFind(REMIX_ID_NLUCAS, &flash));
    CHECK_EQ(0, ftRemixChargeFlashFind(REMIX_ID_NSHEIK, &flash));
    CHECK_EQ(0, ftRemixChargeFlashFind(REMIX_ID_NDEDEDE, &flash));
    CHECK_EQ(0, ftRemixChargeFlashFind(REMIX_ID_SAMUS, &flash));
}

void remixTestChargeFlashRules(void)
{
    printf("charge flash rules\n");
    RUN_TEST(testSamusClonesUseTheCommonGlowAtLevelSeven);
    RUN_TEST(testJDKUsesTheGiantPunchLevel);
    RUN_TEST(testMewtwoAndMarinaReadTheLevelWord);
    RUN_TEST(testSheikAndPeppyReadTheRecoilWordAndShareTheSheikGlow);
    RUN_TEST(testDededeReadsItsOwnWord);
    RUN_TEST(testFightersWithoutAGlowHaveNoRow);
}
