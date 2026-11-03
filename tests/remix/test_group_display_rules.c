/**
 * test_group_display_rules.c - unit tests of the display group flag rules (gm/gmremixgroupdisplayrules.c).
 */

#include "test_check.h"

#include "gm/gmremixgroupdisplayrules.h"

static void testSetWritesTheHideArgumentAsTheWholeWord(void)
{
    CHECK_EQ(1, (int)gmRemixGroupDisplayRulesSetFlags(1));
    CHECK_EQ(0, (int)gmRemixGroupDisplayRulesSetFlags(0));
}

static void testTempHideSavesTheOldWordInTheUpperHalf(void)
{
    CHECK_EQ(0x00010001, (int)gmRemixGroupDisplayRulesTempFlags(0x1, 1));
    CHECK_EQ(0x00420001, (int)gmRemixGroupDisplayRulesTempFlags(0x42, 1));
    CHECK_EQ(0x00000001, (int)gmRemixGroupDisplayRulesTempFlags(0x0, 1));
}

static void testTempShowRestoresTheSavedWord(void)
{
    CHECK_EQ(0x42, (int)gmRemixGroupDisplayRulesTempFlags(0x00420001, 0));
    CHECK_EQ(0x0, (int)gmRemixGroupDisplayRulesTempFlags(0x00000001, 0));
}

static void testTempHideThenShowIsARoundTrip(void)
{
    uint32_t flags = 0x2;

    flags = gmRemixGroupDisplayRulesTempFlags(flags, 1);
    CHECK_EQ(1, (int)(flags & 1));
    flags = gmRemixGroupDisplayRulesTempFlags(flags, 0);
    CHECK_EQ(0x2, (int)flags);
}

void remixTestGroupDisplayRules(void)
{
    printf("group display rules\n");
    RUN_TEST(testSetWritesTheHideArgumentAsTheWholeWord);
    RUN_TEST(testTempHideSavesTheOldWordInTheUpperHalf);
    RUN_TEST(testTempShowRestoresTheSavedWord);
    RUN_TEST(testTempHideThenShowIsARoundTrip);
}
