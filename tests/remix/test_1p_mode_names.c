/**
 * test_1p_mode_names.c - unit tests of the Remix 1P mode names (sc/sc1pmode/sc1premixmodename.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixmodename.h"

static void testEveryModeNameMapsToItsAsmId(void)
{
    CHECK_EQ(SC1PREMIX_MODE_STANDARD, sc1PRemixModeFromName("standard"));
    CHECK_EQ(1, sc1PRemixModeFromName("bonus3"));
    CHECK_EQ(2, sc1PRemixModeFromName("multiman"));
    CHECK_EQ(3, sc1PRemixModeFromName("cruel"));
    CHECK_EQ(4, sc1PRemixModeFromName("remix1p"));
    CHECK_EQ(5, sc1PRemixModeFromName("allstar"));
    CHECK_EQ(6, sc1PRemixModeFromName("hrc"));
}

static void testUnknownNamesAreRejected(void)
{
    CHECK_EQ(-1, sc1PRemixModeFromName(NULL));
    CHECK_EQ(-1, sc1PRemixModeFromName(""));
    CHECK_EQ(-1, sc1PRemixModeFromName("Multiman"));
    CHECK_EQ(-1, sc1PRemixModeFromName("multiman "));
}

void remixTest1PModeNames(void)
{
    printf("1p mode names\n");
    RUN_TEST(testEveryModeNameMapsToItsAsmId);
    RUN_TEST(testUnknownNamesAreRejected);
}
