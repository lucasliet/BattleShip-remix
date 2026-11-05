/**
 * test_kirbygun_rules.c - unit tests of the Kirby held-weapon part rule (ft/ftremix/ftremixkirbygunrules.c, KirbyHats.asm
 * use_extended_special_parts_pause_ gun branch): the file offset of the special part struct of each hat.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixkirbygunrules.h"

static void testWeaponHatsMapToTheirSpecialPartStruct(void)
{
    CHECK_EQ(0x1D890, ftRemixKirbyGunFileOffset(0x1C));
    CHECK_EQ(0x1D980, ftRemixKirbyGunFileOffset(0x2C));
    CHECK_EQ(0x1D830, ftRemixKirbyGunFileOffset(0x1A));
    CHECK_EQ(0x1D8C0, ftRemixKirbyGunFileOffset(0x23));
    CHECK_EQ(0x1D8F0, ftRemixKirbyGunFileOffset(0x24));
    CHECK_EQ(0x1D920, ftRemixKirbyGunFileOffset(0x22));
    CHECK_EQ(0x1D950, ftRemixKirbyGunFileOffset(0x28));
    CHECK_EQ(0x1D9B0, ftRemixKirbyGunFileOffset(0x2E));
    CHECK_EQ(0x1D860, ftRemixKirbyGunFileOffset(0x19));
}

static void testOtherHatsHaveNoWeaponPart(void)
{
    CHECK_EQ(FTREMIX_KIRBY_GUN_NO_OFFSET, ftRemixKirbyGunFileOffset(0));
    CHECK_EQ(FTREMIX_KIRBY_GUN_NO_OFFSET, ftRemixKirbyGunFileOffset(0x0F));
    CHECK_EQ(FTREMIX_KIRBY_GUN_NO_OFFSET, ftRemixKirbyGunFileOffset(0x1B));
    CHECK_EQ(FTREMIX_KIRBY_GUN_NO_OFFSET, ftRemixKirbyGunFileOffset(-1));
}

void remixTestKirbyGunRules(void)
{
    printf("kirby gun rules\n");
    RUN_TEST(testWeaponHatsMapToTheirSpecialPartStruct);
    RUN_TEST(testOtherHatsHaveNoWeaponPart);
}
