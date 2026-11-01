/**
 * test_gameend_input_rules.c - unit tests of the exact button match of the match end (gm/gmremixgameendinputrules.c).
 */

#include "test_check.h"

#include "gm/gmremixgameendinputrules.h"

#define TEST_BUTTON_A 0x8000
#define TEST_BUTTON_B 0x4000
#define TEST_BUTTON_Z 0x2000
#define TEST_BUTTON_START 0x1000
#define TEST_BUTTON_L 0x0020
#define TEST_BUTTON_R 0x0010

static void testWholeCombinationMatches(void)
{
    unsigned int salty = TEST_BUTTON_A | TEST_BUTTON_B | TEST_BUTTON_Z | TEST_BUTTON_R | TEST_BUTTON_START;

    CHECK_EQ(1, gmRemixGameEndInputMatchesExactly(salty, salty));
}

static void testMissingButtonDoesNotMatch(void)
{
    unsigned int salty = TEST_BUTTON_A | TEST_BUTTON_B | TEST_BUTTON_Z | TEST_BUTTON_R | TEST_BUTTON_START;

    CHECK_EQ(0, gmRemixGameEndInputMatchesExactly(salty & ~TEST_BUTTON_START, salty));
    CHECK_EQ(0, gmRemixGameEndInputMatchesExactly(TEST_BUTTON_A, salty));
}

static void testExtraButtonDoesNotMatch(void)
{
    unsigned int results = TEST_BUTTON_R | TEST_BUTTON_L;

    CHECK_EQ(0, gmRemixGameEndInputMatchesExactly(results | TEST_BUTTON_A, results));
}

static void testRAloneDoesNotForceTheResultsScreen(void)
{
    unsigned int results = TEST_BUTTON_R | TEST_BUTTON_L;

    CHECK_EQ(0, gmRemixGameEndInputMatchesExactly(TEST_BUTTON_R, results));
    CHECK_EQ(1, gmRemixGameEndInputMatchesExactly(results, results));
}

static void testEmptyMaskNeverMatches(void)
{
    CHECK_EQ(0, gmRemixGameEndInputMatchesExactly(0, 0));
}

void remixTestGameEndInputRules(void)
{
    printf("match end button rules\n");
    RUN_TEST(testWholeCombinationMatches);
    RUN_TEST(testMissingButtonDoesNotMatch);
    RUN_TEST(testExtraButtonDoesNotMatch);
    RUN_TEST(testRAloneDoesNotForceTheResultsScreen);
    RUN_TEST(testEmptyMaskNeverMatches);
}
