/**
 * test_1p_hrc_signs.c - unit tests of the Home-Run Contest sign digits (sc/sc1pmode/sc1premixhrcsignrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixhrcsignrules.h"

static SC1PRemixHrcSign signAfterWarps(int warps)
{
    SC1PRemixHrcSign sign;
    int i;

    sc1PRemixHrcSignReset(&sign);
    for (i = 0; i < warps; i++)
    {
        sc1PRemixHrcSignAdvance(&sign);
    }
    return sign;
}

static void testResetShowsThreeHundredWithBlankThousands(void)
{
    SC1PRemixHrcSign sign = signAfterWarps(0);

    CHECK_EQ(SC1PREMIX_HRC_SIGN_STATE_UNDER_10K, sign.state);
    CHECK_EQ(0, sign.thousands);
    CHECK_EQ(3, sign.hundreds);
    CHECK_EQ(0, sign.hundred_pair);
}

static void testEachWarpAddsOneHundredBelowTenThousand(void)
{
    SC1PRemixHrcSign sign = signAfterWarps(3);

    CHECK_EQ(6, sign.hundreds);
    CHECK_EQ(0, sign.thousands);
}

static void testSeventhWarpRollsTheHundredsIntoTheThousands(void)
{
    SC1PRemixHrcSign sign = signAfterWarps(7);

    CHECK_EQ(0, sign.hundreds);
    CHECK_EQ(1, sign.thousands);
    CHECK_EQ(SC1PREMIX_HRC_SIGN_STATE_UNDER_10K, sign.state);
}

static void testTenThousandStartsTheSecondRange(void)
{
    SC1PRemixHrcSign sign = signAfterWarps(97);

    CHECK_EQ(SC1PREMIX_HRC_SIGN_STATE_UNDER_100K, sign.state);
    CHECK_EQ(1, sign.thousands);
    CHECK_EQ(0, sign.hundreds);
    CHECK_EQ(1, sign.hundred_pair);
}

static void testSecondRangeCountsThePair(void)
{
    SC1PRemixHrcSign sign = signAfterWarps(99);

    CHECK_EQ(SC1PREMIX_HRC_SIGN_STATE_UNDER_100K, sign.state);
    CHECK_EQ(3, sign.hundred_pair);
}

static void testSignStopsAtNinetyNineThousandNineNinetyNine(void)
{
    SC1PRemixHrcSign maxed = signAfterWarps(2000);
    SC1PRemixHrcSign later = signAfterWarps(3000);

    CHECK_EQ(SC1PREMIX_HRC_SIGN_STATE_MAXED, maxed.state);
    CHECK_EQ(9, maxed.thousands);
    CHECK_EQ(9, maxed.hundreds);
    CHECK_EQ(11, maxed.hundred_pair);
    CHECK_EQ(maxed.state, later.state);
    CHECK_EQ(maxed.hundred_pair, later.hundred_pair);
}

void remixTest1PHrcSigns(void)
{
    printf("1p hrc signs\n");
    RUN_TEST(testResetShowsThreeHundredWithBlankThousands);
    RUN_TEST(testEachWarpAddsOneHundredBelowTenThousand);
    RUN_TEST(testSeventhWarpRollsTheHundredsIntoTheThousands);
    RUN_TEST(testTenThousandStartsTheSecondRange);
    RUN_TEST(testSecondRangeCountsThePair);
    RUN_TEST(testSignStopsAtNinetyNineThousandNineNinetyNine);
}
