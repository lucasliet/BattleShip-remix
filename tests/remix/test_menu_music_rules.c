/**
 * test_menu_music_rules.c - unit tests of the Menu Music trap easter egg (gm/gmremixmenumusicrules.c).
 */

#include "test_check.h"

#include "gm/gmremixmenumusicrules.h"

static void testFourthPressInARowActivatesTheTrap(void)
{
    int count = 0;

    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(1, count);
    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(2, count);
    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(3, count);
    CHECK_TRUE(!gmRemixMenuMusicRulesTrapIsActive(count));
    count = gmRemixMenuMusicRulesCountTrapPress(count);
    CHECK_EQ(GMREMIX_MENU_MUSIC_TRAP_ACTIVE, count);
    CHECK_TRUE(gmRemixMenuMusicRulesTrapIsActive(count));
}

static void testPressAfterTheTrapRestartsTheCountAtOne(void)
{
    int count = gmRemixMenuMusicRulesCountTrapPress(GMREMIX_MENU_MUSIC_TRAP_ACTIVE);

    CHECK_EQ(1, count);
    CHECK_TRUE(!gmRemixMenuMusicRulesTrapIsActive(count));
}

static void testTrapStaysActiveUntilAnotherPressCounts(void)
{
    CHECK_TRUE(gmRemixMenuMusicRulesTrapIsActive(GMREMIX_MENU_MUSIC_TRAP_ACTIVE));
    CHECK_TRUE(!gmRemixMenuMusicRulesTrapIsActive(0));
}

void remixTestMenuMusicRules(void)
{
    printf("menu music rules\n");
    RUN_TEST(testFourthPressInARowActivatesTheTrap);
    RUN_TEST(testPressAfterTheTrapRestartsTheCountAtOne);
    RUN_TEST(testTrapStaysActiveUntilAnotherPressCounts);
}
