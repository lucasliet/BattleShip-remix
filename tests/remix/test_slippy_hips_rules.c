/**
 * test_slippy_hips_rules.c - unit tests of the Slippy hip translation table rule (ft/ftremix/slippy/ftremixslippyhipsrules.c,
 * Slippy.asm hips_y_translation_): which table word an action reads.
 */

#include "test_check.h"

#include "ft/ftremix/slippy/ftremixslippyhipsrules.h"

#define TABLE 0x805763A0u
#define MENU_TABLE 0x80576780u

static void testActionReadsItsWordInTheMatchTable(void)
{
    unsigned int address = 0;

    CHECK_EQ(1, ftRemixSlippyHipFindWordAddress(0, TABLE, MENU_TABLE, &address));
    CHECK_EQ(TABLE, address);
    CHECK_EQ(1, ftRemixSlippyHipFindWordAddress(5, TABLE, MENU_TABLE, &address));
    CHECK_EQ(TABLE + 20, address);
}

static void testLastActionOfTheTableIsInside(void)
{
    unsigned int address = 0;

    CHECK_EQ(1, ftRemixSlippyHipFindWordAddress(FTREMIX_SLIPPY_HIP_TABLE_SIZE - 1, TABLE, MENU_TABLE, &address));
    CHECK_EQ(TABLE + ((FTREMIX_SLIPPY_HIP_TABLE_SIZE - 1) * 4), address);
}

static void testActionPastTheTableHasNoEntry(void)
{
    unsigned int address = 0;

    CHECK_EQ(0, ftRemixSlippyHipFindWordAddress(FTREMIX_SLIPPY_HIP_TABLE_SIZE, TABLE, MENU_TABLE, &address));
    CHECK_EQ(0, ftRemixSlippyHipFindWordAddress(0x1F3, TABLE, MENU_TABLE, &address));
}

static void testMenuActionReadsTheMenuTable(void)
{
    unsigned int address = 0;

    CHECK_EQ(1, ftRemixSlippyHipFindWordAddress(0x10000, TABLE, MENU_TABLE, &address));
    CHECK_EQ(MENU_TABLE, address);
    CHECK_EQ(1, ftRemixSlippyHipFindWordAddress(0x10005, TABLE, MENU_TABLE, &address));
    CHECK_EQ(MENU_TABLE + 20, address);
}

static void testMenuActionPastTheMenuTableHasNoEntry(void)
{
    unsigned int address = 0;

    CHECK_EQ(0, ftRemixSlippyHipFindWordAddress(0x10000 + FTREMIX_SLIPPY_HIP_MENU_TABLE_SIZE, TABLE, MENU_TABLE, &address));
}

void remixTestSlippyHipsRules(void)
{
    printf("slippy hips rules\n");
    RUN_TEST(testActionReadsItsWordInTheMatchTable);
    RUN_TEST(testLastActionOfTheTableIsInside);
    RUN_TEST(testActionPastTheTableHasNoEntry);
    RUN_TEST(testMenuActionReadsTheMenuTable);
    RUN_TEST(testMenuActionPastTheMenuTableHasNoEntry);
}
