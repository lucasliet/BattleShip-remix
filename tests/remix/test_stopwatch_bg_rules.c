/**
 * test_stopwatch_bg_rules.c - unit tests of the Stopwatch backfire background rules (it/itremix/itremixstopwatchbgrules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixstopwatchbgrules.h"

static void testSpeedsBelowSevenUseTheRedColor(void)
{
    CHECK_EQ(ITREMIX_STOPWATCH_BG_COLOR_FAST, itRemixStopwatchBgGetColor(6));
    CHECK_EQ(ITREMIX_STOPWATCH_BG_COLOR_FAST, itRemixStopwatchBgGetColor(1));
}

static void testSevenAndAboveUseTheBlueColor(void)
{
    CHECK_EQ(ITREMIX_STOPWATCH_BG_COLOR_SLOW, itRemixStopwatchBgGetColor(7));
    CHECK_EQ(ITREMIX_STOPWATCH_BG_COLOR_SLOW, itRemixStopwatchBgGetColor(12));
}

static void testNoFadeWhileTheTimerIsAtLeastTwoHundredForty(void)
{
    CHECK_EQ(0xA1333399U, itRemixStopwatchBgFadeColor(0xA1333399U, 240));
    CHECK_EQ(0xA1333399U, itRemixStopwatchBgFadeColor(0xA1333399U, 600));
}

static void testFadeOnlyOnEveryFourthFrame(void)
{
    CHECK_EQ(0xA1333399U, itRemixStopwatchBgFadeColor(0xA1333399U, 237));
    CHECK_EQ(0xA1333399U, itRemixStopwatchBgFadeColor(0xA1333399U, 238));
    CHECK_EQ(0xA1333399U, itRemixStopwatchBgFadeColor(0xA1333399U, 239));
    CHECK_EQ(0xA1333396U, itRemixStopwatchBgFadeColor(0xA1333399U, 236));
}

static void testHighAlphaDropsByThreeAndKeepsTheRgb(void)
{
    CHECK_EQ(0x0066AA74U, itRemixStopwatchBgFadeColor(0x0066AA77U, 100));
}

static void testAlphaBelowSixtySixDropsByOne(void)
{
    CHECK_EQ(0x00112264U, itRemixStopwatchBgFadeColor(0x00112265U, 8));
    CHECK_EQ(0x00112233U, itRemixStopwatchBgFadeColor(0x00112234U, 8));
}

static void testAlphaBelowTheFloorStops(void)
{
    CHECK_EQ(0x00112232U, itRemixStopwatchBgFadeColor(0x00112232U, 8));
    CHECK_EQ(0x00000000U, itRemixStopwatchBgFadeColor(0x00000000U, 8));
}

static void testFullFadeOfTheRedColorEndsAtAlpha0x39(void)
{
    uint32_t color = ITREMIX_STOPWATCH_BG_COLOR_FAST;
    int32_t timer;

    for (timer = 239; timer >= 0; timer--)
    {
        color = itRemixStopwatchBgFadeColor(color, timer);
    }
    CHECK_EQ(0x39U, color & 0xFF);
}

void remixTestStopwatchBgRules(void)
{
    printf("stopwatch background rules\n");
    RUN_TEST(testSpeedsBelowSevenUseTheRedColor);
    RUN_TEST(testSevenAndAboveUseTheBlueColor);
    RUN_TEST(testNoFadeWhileTheTimerIsAtLeastTwoHundredForty);
    RUN_TEST(testFadeOnlyOnEveryFourthFrame);
    RUN_TEST(testHighAlphaDropsByThreeAndKeepsTheRgb);
    RUN_TEST(testAlphaBelowSixtySixDropsByOne);
    RUN_TEST(testAlphaBelowTheFloorStops);
    RUN_TEST(testFullFadeOfTheRedColorEndsAtAlpha0x39);
}
