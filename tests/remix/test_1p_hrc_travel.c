/**
 * test_1p_hrc_travel.c - unit tests of the Home-Run Contest object travel (sc/sc1pmode/sc1premixhrctravelrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixhrctravelrules.h"

static void testObjectsBelowTheThresholdStayWhereTheyAre(void)
{
    SC1PRemixHrcTraveler traveler = { 0, 0.0F, 0.0F };

    CHECK_EQ(-5000, (int)sc1PRemixHrcTravelerStep(&traveler, -5000.0F, -5000.0F, 9120.0F));
    CHECK_EQ(0, traveler.is_tracking);
    CHECK_EQ(-5000, (int)traveler.previous_x);
}

static void testWithoutAWarpAnObjectPastTheThresholdIsNotMoved(void)
{
    SC1PRemixHrcTraveler traveler = { 0, 0.0F, 0.0F };

    CHECK_EQ(-1000, (int)sc1PRemixHrcTravelerStep(&traveler, -1000.0F, -1100.0F, 0.0F));
    CHECK_EQ(1, traveler.is_tracking);
    CHECK_EQ(152, (int)traveler.distance);
    CHECK_EQ(-900, (int)sc1PRemixHrcTravelerStep(&traveler, -900.0F, -1000.0F, 0.0F));
    CHECK_EQ(252, (int)traveler.distance);
}

static void testAWarpMovesTheObjectBackWithTheCamera(void)
{
    SC1PRemixHrcTraveler traveler = { 0, 0.0F, 0.0F };

    CHECK_EQ(3880, (int)sc1PRemixHrcTravelerStep(&traveler, 13000.0F, 3400.0F, 9120.0F));
}

static void testAnObjectFarBehindTheCameraIsHeldAtTheThreshold(void)
{
    SC1PRemixHrcTraveler traveler = { 0, 0.0F, 0.0F };

    CHECK_EQ(-1152, (int)sc1PRemixHrcTravelerStep(&traveler, -1000.0F, 0.0F, 9120.0F));
    CHECK_EQ(-1152, (int)traveler.previous_x);
}

static void testTheDistanceFollowsTheMovementOfTheObjectNotItsShift(void)
{
    SC1PRemixHrcTraveler traveler = { 0, 0.0F, 0.0F };
    float shifted_x = sc1PRemixHrcTravelerStep(&traveler, 13000.0F, 3400.0F, 9120.0F);

    sc1PRemixHrcTravelerStep(&traveler, shifted_x + 50.0F, 3400.0F, 9120.0F);
    CHECK_EQ(14202, (int)traveler.distance);
}

void remixTest1PHrcTravel(void)
{
    printf("1p hrc travel\n");
    RUN_TEST(testObjectsBelowTheThresholdStayWhereTheyAre);
    RUN_TEST(testWithoutAWarpAnObjectPastTheThresholdIsNotMoved);
    RUN_TEST(testAWarpMovesTheObjectBackWithTheCamera);
    RUN_TEST(testAnObjectFarBehindTheCameraIsHeldAtTheThreshold);
    RUN_TEST(testTheDistanceFollowsTheMovementOfTheObjectNotItsShift);
}
