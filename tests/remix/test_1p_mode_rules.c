/**
 * test_1p_mode_rules.c - unit tests of the state-free Remix 1P rules (sc/sc1pmode/sc1premixmoderules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixmoderules.h"

static void testBonusModesLeaveTheBonusCSSFor1P(void)
{
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_BONUS3));
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_MULTIMAN));
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_CRUEL));
    CHECK_TRUE(sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_HRC));
}

static void testOtherModesKeepTheirOwnFlow(void)
{
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(-1));
    CHECK_EQ(0, sc1PRemixModeLeavesBonusCSSFor1P(SC1PREMIX_MODE_COUNT));
}

static void testSeedAdvanceIsOneToSixtyFourByFrameCount(void)
{
    CHECK_EQ(1, (int)sc1PRemixModeSeedAdvanceCount(0));
    CHECK_EQ(2, (int)sc1PRemixModeSeedAdvanceCount(1));
    CHECK_EQ(64, (int)sc1PRemixModeSeedAdvanceCount(63));
    CHECK_EQ(1, (int)sc1PRemixModeSeedAdvanceCount(64));
    CHECK_EQ(64, (int)sc1PRemixModeSeedAdvanceCount(0xFFFFFFFF));
}

void remixTest1PModeRules(void)
{
    printf("1p mode rules\n");
    RUN_TEST(testBonusModesLeaveTheBonusCSSFor1P);
    RUN_TEST(testOtherModesKeepTheirOwnFlow);
    RUN_TEST(testSeedAdvanceIsOneToSixtyFourByFrameCount);
}
