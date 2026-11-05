/**
 * test_camerapan_rules.c - unit tests of the pause camera pan rules (gm/gmremixcamerapanrules.c).
 */

#include "test_check.h"

#include "gm/gmremixcamerapanrules.h"

static GMRemixCameraPanOffsets makeOffsets(void)
{
    GMRemixCameraPanOffsets offsets = { 0.0F, 0.0F, 0.0F, 0.0F };

    return offsets;
}

static void testNoButtonLeavesTheOffsets(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();

    gmRemixCameraPanRulesStep(&offsets, 0);

    CHECK_EQ(0, (int)offsets.x);
    CHECK_EQ(0, (int)offsets.y);
    CHECK_EQ(0, (int)offsets.z);
    CHECK_EQ(0, (int)offsets.fov);
}

static void testCUpAndCDownMoveYAndCUpWinsWhenBothAreHeld(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();

    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_C_UP);
    CHECK_EQ(32, (int)offsets.y);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_C_DOWN);
    CHECK_EQ(0, (int)offsets.y);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_C_UP | GMREMIX_CAMERAPAN_BUTTON_C_DOWN);
    CHECK_EQ(32, (int)offsets.y);
}

static void testCLeftAndCRightMoveXAndCLeftWinsWhenBothAreHeld(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();

    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_C_RIGHT);
    CHECK_EQ(32, (int)offsets.x);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_C_LEFT);
    CHECK_EQ(0, (int)offsets.x);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_C_LEFT | GMREMIX_CAMERAPAN_BUTTON_C_RIGHT);
    CHECK_EQ(-32, (int)offsets.x);
}

static void testBMovesTheDistanceAwayAndAMovesItCloser(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();

    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_B);
    CHECK_EQ(80, (int)offsets.z);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_A);
    CHECK_EQ(0, (int)offsets.z);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_A | GMREMIX_CAMERAPAN_BUTTON_B);
    CHECK_EQ(80, (int)offsets.z);
}

static void testTheDistanceStopsAtTheCloseLimitButNotAtTheFarOne(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();
    int frame;

    for (frame = 0; frame < 20; frame++)
    {
        gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_A);
    }
    CHECK_EQ(-1024, (int)offsets.z);

    for (frame = 0; frame < 100; frame++)
    {
        gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_B);
    }
    CHECK_EQ(6976, (int)offsets.z);
}

static void testZMovesTheFovInsteadOfTheDistance(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();

    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_B | GMREMIX_CAMERAPAN_BUTTON_Z);

    CHECK_EQ(0, (int)offsets.z);
    CHECK_TRUE(offsets.fov > 0.79F && offsets.fov < 0.80F);
    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_A | GMREMIX_CAMERAPAN_BUTTON_Z);
    CHECK_TRUE(offsets.fov > -0.001F && offsets.fov < 0.001F);
}

static void testTheFovOffsetStaysBetweenItsLimits(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();
    int frame;

    for (frame = 0; frame < 300; frame++)
    {
        gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_B | GMREMIX_CAMERAPAN_BUTTON_Z);
    }
    CHECK_EQ(124, (int)offsets.fov);

    for (frame = 0; frame < 300; frame++)
    {
        gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_A | GMREMIX_CAMERAPAN_BUTTON_Z);
    }
    CHECK_EQ(-28, (int)offsets.fov);
}

static void testZAloneDoesNothing(void)
{
    GMRemixCameraPanOffsets offsets = makeOffsets();

    gmRemixCameraPanRulesStep(&offsets, GMREMIX_CAMERAPAN_BUTTON_Z);

    CHECK_EQ(0, (int)offsets.z);
    CHECK_EQ(0, (int)offsets.fov);
}

void remixTestCameraPanRules(void)
{
    printf("camera pan rules\n");
    RUN_TEST(testNoButtonLeavesTheOffsets);
    RUN_TEST(testCUpAndCDownMoveYAndCUpWinsWhenBothAreHeld);
    RUN_TEST(testCLeftAndCRightMoveXAndCLeftWinsWhenBothAreHeld);
    RUN_TEST(testBMovesTheDistanceAwayAndAMovesItCloser);
    RUN_TEST(testTheDistanceStopsAtTheCloseLimitButNotAtTheFarOne);
    RUN_TEST(testZMovesTheFovInsteadOfTheDistance);
    RUN_TEST(testTheFovOffsetStaysBetweenItsLimits);
    RUN_TEST(testZAloneDoesNothing);
}
