/**
 * test_1p_bonus_modes_rules.c - unit tests of what the Bonus 2 character select shows for the Remix 1P modes
 * (mnremixbonusmodesrules.c). Expected values come from SinglePlayerMenus.asm (header_graphic, counter_graphic, _ko_amount_or_feet,
 * _timedisplay_skip, total_kos_or_feet_, draw_hrc_high_score_decimal_) and Bonus.asm (arrow_routine_, handle_arrow_press_).
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixbonusmodesrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"

static void testEachBonusModeHasItsOwnTitle(void)
{
    CHECK_EQ(0x1DD8, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_BONUS3));
    CHECK_EQ(0x14D8, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(0x1958, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(0x24C8, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_HRC));
}

static void testStandardBonusAndTheOtherModesKeepTheVanillaTitle(void)
{
    CHECK_EQ(0, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, mnRemixBonusModesHeaderOffset(SC1PREMIX_MODE_ALLSTAR));
}

static void testKoAndFeetRecordsUseTheirOwnLabelAndDigits(void)
{
    CHECK_EQ(0x34E8, mnRemixBonusModesCounterOffset(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(0x34E8, mnRemixBonusModesCounterOffset(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(0x3C08, mnRemixBonusModesCounterOffset(SC1PREMIX_MODE_HRC));
    CHECK_EQ(0, mnRemixBonusModesCounterOffset(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, mnRemixBonusModesCounterOffset(SC1PREMIX_MODE_BONUS3));

    CHECK_EQ(5, mnRemixBonusModesRecordDigits(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(5, mnRemixBonusModesRecordDigits(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(7, mnRemixBonusModesRecordDigits(SC1PREMIX_MODE_HRC));
    CHECK_EQ(2, mnRemixBonusModesRecordDigits(SC1PREMIX_MODE_STANDARD));
}

static void testOnlyTheHomeRunContestHasADecimalPoint(void)
{
    CHECK_TRUE(mnRemixBonusModesHasDecimal(SC1PREMIX_MODE_HRC));
    CHECK_EQ(0, mnRemixBonusModesHasDecimal(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(0, mnRemixBonusModesHasDecimal(SC1PREMIX_MODE_STANDARD));
}

static void testBonus3AlwaysShowsATimeAndTheOtherModesACount(void)
{
    CHECK_TRUE(mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_BONUS3, 0, 0));
    CHECK_TRUE(mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_BONUS3, 1, 0));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_MULTIMAN, 0, 1));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_CRUEL, 0, 1));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_HRC, 0, 1));
}

static void testStandardBonusShowsATimeOnlyForAFinishedFighter(void)
{
    CHECK_TRUE(mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_STANDARD, 0, 1));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_STANDARD, 0, 0));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTime(SC1PREMIX_MODE_STANDARD, 1, 1));
}

static void testHordeAndContestModesAlwaysShowTheTotal(void)
{
    CHECK_TRUE(mnRemixBonusModesRulesShowsTotal(SC1PREMIX_MODE_MULTIMAN, 0));
    CHECK_TRUE(mnRemixBonusModesRulesShowsTotal(SC1PREMIX_MODE_CRUEL, 0));
    CHECK_TRUE(mnRemixBonusModesRulesShowsTotal(SC1PREMIX_MODE_HRC, 0));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTotal(SC1PREMIX_MODE_BONUS3, 0));
    CHECK_TRUE(mnRemixBonusModesRulesShowsTotal(SC1PREMIX_MODE_BONUS3, 1));
    CHECK_EQ(0, mnRemixBonusModesRulesShowsTotal(SC1PREMIX_MODE_STANDARD, 0));
}

static void testTotalDigitsFollowTheMode(void)
{
    CHECK_EQ(6, mnRemixBonusModesTotalDigits(SC1PREMIX_MODE_MULTIMAN));
    CHECK_EQ(6, mnRemixBonusModesTotalDigits(SC1PREMIX_MODE_CRUEL));
    CHECK_EQ(8, mnRemixBonusModesTotalDigits(SC1PREMIX_MODE_HRC));
    CHECK_EQ(0, mnRemixBonusModesTotalDigits(SC1PREMIX_MODE_STANDARD));
}

static void testTotalAddsTheRecordsAndIgnoresTheConsoleFiller(void)
{
    unsigned int records[4] = { 10, 0xAAAAAAAAu, 25, 7 };

    CHECK_EQ(42, mnRemixBonusModesSumRecords(records, 4));
    CHECK_EQ(10, mnRemixBonusModesSumRecords(records, 1));
    CHECK_EQ(0, mnRemixBonusModesSumRecords(records, 0));
}

static void testCourseChoiceStopsAtBothEnds(void)
{
    CHECK_EQ(1, mnRemixBonusModesToggleStep(0, 1));
    CHECK_EQ(0, mnRemixBonusModesToggleStep(1, -1));
    CHECK_EQ(1, mnRemixBonusModesToggleStep(1, 1));
    CHECK_EQ(0, mnRemixBonusModesToggleStep(0, -1));
}

static void testOnlyTheArrowsThatCanMoveExist(void)
{
    CHECK_EQ(0, mnRemixBonusModesToggleArrowExists(0, 0));
    CHECK_TRUE(mnRemixBonusModesToggleArrowExists(0, 1));
    CHECK_TRUE(mnRemixBonusModesToggleArrowExists(1, 0));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowExists(1, 1));
}

static void testArrowsBlinkElevenFramesOfTwenty(void)
{
    int timer = 0;
    int visible = 0;
    int frame;

    for (frame = 0; frame < MNREMIX_BONUSMODES_BLINK_PERIOD; frame++)
    {
        visible += mnRemixBonusModesToggleBlinkIsOn(timer);
        timer = mnRemixBonusModesToggleBlinkNext(timer);
    }
    CHECK_EQ(11, visible);
    CHECK_EQ(0, timer);
    CHECK_EQ(0, mnRemixBonusModesToggleBlinkNext(19));
}

static void testArrowPressUsesThePaddingOnTheSideFacingTheOtherArrow(void)
{
    CHECK_TRUE(mnRemixBonusModesToggleArrowHit(212, 160, 212, 157, 8, 10, 0));
    CHECK_TRUE(mnRemixBonusModesToggleArrowHit(228, 160, 212, 157, 8, 10, 0));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowHit(211, 160, 212, 157, 8, 10, 0));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowHit(229, 160, 212, 157, 8, 10, 0));

    CHECK_TRUE(mnRemixBonusModesToggleArrowHit(272, 160, 280, 157, 8, 10, 1));
    CHECK_TRUE(mnRemixBonusModesToggleArrowHit(288, 160, 280, 157, 8, 10, 1));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowHit(271, 160, 280, 157, 8, 10, 1));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowHit(289, 160, 280, 157, 8, 10, 1));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowHit(284, 156, 280, 157, 8, 10, 1));
    CHECK_EQ(0, mnRemixBonusModesToggleArrowHit(284, 168, 280, 157, 8, 10, 1));
}

void remixTest1PBonusModesRules(void)
{
    printf("1p bonus modes rules\n");
    RUN_TEST(testEachBonusModeHasItsOwnTitle);
    RUN_TEST(testStandardBonusAndTheOtherModesKeepTheVanillaTitle);
    RUN_TEST(testKoAndFeetRecordsUseTheirOwnLabelAndDigits);
    RUN_TEST(testOnlyTheHomeRunContestHasADecimalPoint);
    RUN_TEST(testBonus3AlwaysShowsATimeAndTheOtherModesACount);
    RUN_TEST(testStandardBonusShowsATimeOnlyForAFinishedFighter);
    RUN_TEST(testHordeAndContestModesAlwaysShowTheTotal);
    RUN_TEST(testTotalDigitsFollowTheMode);
    RUN_TEST(testTotalAddsTheRecordsAndIgnoresTheConsoleFiller);
    RUN_TEST(testCourseChoiceStopsAtBothEnds);
    RUN_TEST(testOnlyTheArrowsThatCanMoveExist);
    RUN_TEST(testArrowsBlinkElevenFramesOfTwenty);
    RUN_TEST(testArrowPressUsesThePaddingOnTheSideFacingTheOtherArrow);
}
