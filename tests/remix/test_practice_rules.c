/**
 * test_practice_rules.c - unit tests of the Remix Practice_1P rules (gm/gmremixpracticerules.c).
 */

#include "test_check.h"

#include "gm/gmremixpracticerules.h"

#define MODE_BONUS3 1
#define MODE_MULTIMAN 2
#define MODE_CRUEL 3
#define MODE_ALLSTAR 5
#define MODE_HRC 6

static void testStandardAndRemix1PRunsWithAStagePractice(void)
{
    CHECK_EQ(1, gmRemixPracticeIsRunActive(GMREMIX_PRACTICE_MODE_STANDARD, 4));
    CHECK_EQ(1, gmRemixPracticeIsRunActive(GMREMIX_PRACTICE_MODE_REMIX_1P, 11));
}

static void testOffPanelValueIsNotPractice(void)
{
    CHECK_EQ(0, gmRemixPracticeIsRunActive(GMREMIX_PRACTICE_MODE_STANDARD, 0));
    CHECK_EQ(0, gmRemixPracticeIsRunActive(GMREMIX_PRACTICE_MODE_REMIX_1P, 0));
}

static void testOtherModesAreNeverPractice(void)
{
    CHECK_EQ(0, gmRemixPracticeIsRunActive(MODE_BONUS3, 3));
    CHECK_EQ(0, gmRemixPracticeIsRunActive(MODE_MULTIMAN, 3));
    CHECK_EQ(0, gmRemixPracticeIsRunActive(MODE_CRUEL, 3));
    CHECK_EQ(0, gmRemixPracticeIsRunActive(MODE_ALLSTAR, 3));
    CHECK_EQ(0, gmRemixPracticeIsRunActive(MODE_HRC, 3));
}

static void testRetryIsHeldOnlyWithL(void)
{
    CHECK_EQ(1, gmRemixPracticeIsRetryHeld(GMREMIX_PRACTICE_BUTTON_L));
    CHECK_EQ(1, gmRemixPracticeIsRetryHeld(GMREMIX_PRACTICE_BUTTON_L | 0x8000));
    CHECK_EQ(0, gmRemixPracticeIsRetryHeld(0x8000 | 0x2000));
    CHECK_EQ(0, gmRemixPracticeIsRetryHeld(0));
}

static void testPauseRetryNeedsPracticeScenePressAndL(void)
{
    CHECK_EQ(1, gmRemixPracticeIsPauseRetry(1, 1, GMREMIX_PRACTICE_BUTTON_L));
    CHECK_EQ(0, gmRemixPracticeIsPauseRetry(0, 1, GMREMIX_PRACTICE_BUTTON_L));
    CHECK_EQ(0, gmRemixPracticeIsPauseRetry(1, 0, GMREMIX_PRACTICE_BUTTON_L));
    CHECK_EQ(0, gmRemixPracticeIsPauseRetry(1, 1, 0x8000));
}

void remixTestPracticeRules(void)
{
    printf("practice rules\n");
    RUN_TEST(testStandardAndRemix1PRunsWithAStagePractice);
    RUN_TEST(testOffPanelValueIsNotPractice);
    RUN_TEST(testOtherModesAreNeverPractice);
    RUN_TEST(testRetryIsHeldOnlyWithL);
    RUN_TEST(testPauseRetryNeedsPracticeScenePressAndL);
}
