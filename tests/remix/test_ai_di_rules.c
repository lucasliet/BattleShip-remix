/**
 * test_ai_di_rules.c - unit tests of the Remix CPU DI rules (ft/ftremix/ftremixaidirules.c).
 */

#include "test_check.h"

#include <string.h>

#include "ft/ftremix/ftremixaidirules.h"

static int32_t sFakeRandomValue;
static int32_t sFakeRandomCalls;

static int32_t fakeRandom(int32_t range)
{
    sFakeRandomCalls++;

    return (sFakeRandomValue < range) ? sFakeRandomValue : (range - 1);
}

static void resetRandom(int32_t value)
{
    sFakeRandomValue = value;
    sFakeRandomCalls = 0;
}

static FTRemixAiDiIn makeTrainingInput(int32_t type, int32_t strength, int32_t direction)
{
    FTRemixAiDiIn in;

    memset(&in, 0, sizeof(in));
    in.is_training = 1;
    in.menu_type = type;
    in.menu_strength = strength;
    in.menu_direction = direction;
    in.menu_first_hit = 2;
    in.hit_count = 2;
    in.is_in_hitlag = 1;
    in.is_damage_status = 1;
    in.is_facing_right = 1;

    return in;
}

static FTRemixAiDiIn makeVersusInput(int32_t level)
{
    FTRemixAiDiIn in;

    memset(&in, 0, sizeof(in));
    in.hit_count = 3;
    in.is_in_hitlag = 1;
    in.status_frame = 20;
    in.is_damage_status = 1;
    in.cpu_level = level;
    in.uses_advanced_ai = 1;

    return in;
}

static const int8_t sNotSet[FTREMIXAIDI_PARAM_COUNT] = { -1, -1, -1, -1 };

static void testTrainingMenuPicksTheExactRoutine(void)
{
    FTRemixAiDiIn in = makeTrainingInput(2, 1, 1);
    FTRemixAiDiOut out;

    resetRandom(0);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(1, out.has_routine);
    CHECK_EQ(0, out.is_slide);
    CHECK_EQ(FTREMIXAIDI_STRENGTH_HIGH, out.strength);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_LEFT, out.direction);
    CHECK_EQ(0, sFakeRandomCalls);
}

static void testTrainingNoneTypeDoesNothing(void)
{
    FTRemixAiDiIn in = makeTrainingInput(FTREMIXAIDI_MENU_TYPE_NONE, 1, 1);
    FTRemixAiDiOut out;

    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
}

static void testTrainingWaitsForTheFirstHit(void)
{
    FTRemixAiDiIn in = makeTrainingInput(2, 1, 1);
    FTRemixAiDiOut out;

    in.hit_count = 1;
    in.menu_first_hit = 2;
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
}

static void testAwayAndTowardFollowTheFacing(void)
{
    FTRemixAiDiIn in = makeTrainingInput(3, 2, FTREMIXAIDI_MENU_DIRECTION_AWAY);
    FTRemixAiDiOut out;

    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_LEFT, out.direction);
    CHECK_EQ(1, out.is_slide);
    CHECK_EQ(FTREMIXAIDI_STRENGTH_MEDIUM, out.strength);
    in.menu_direction = FTREMIXAIDI_MENU_DIRECTION_AWAY + 1;
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_RIGHT, out.direction);
    in.is_facing_right = 0;
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_LEFT, out.direction);
}

static void testRandomTypeRollsOnceAndStoresIt(void)
{
    FTRemixAiDiIn in = makeTrainingInput(FTREMIXAIDI_MENU_TYPE_RANDOM, 1, 1);
    FTRemixAiDiOut out;

    resetRandom(1);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(1, out.is_slide);
    CHECK_EQ(1, out.params[FTREMIXAIDI_PARAM_TYPE]);
    CHECK_EQ(1, sFakeRandomCalls);
}

static void testSavedComboParametersAreReusedWithoutRolling(void)
{
    FTRemixAiDiIn in = makeTrainingInput(FTREMIXAIDI_MENU_TYPE_RANDOM, 0, FTREMIXAIDI_MENU_DIRECTION_RANDOM);
    int8_t saved[FTREMIXAIDI_PARAM_COUNT] = { 0, 2, 3, -1 };
    FTRemixAiDiOut out;

    resetRandom(1);
    ftRemixAiDiDecide(&in, saved, fakeRandom, &out);
    CHECK_EQ(0, sFakeRandomCalls);
    CHECK_EQ(0, out.is_slide);
    CHECK_EQ(FTREMIXAIDI_STRENGTH_LOW, out.strength);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_DOWN, out.direction);
}

static void testParametersResetWhenNoComboIsRunning(void)
{
    FTRemixAiDiIn in = makeTrainingInput(2, 1, 1);
    int8_t saved[FTREMIXAIDI_PARAM_COUNT] = { 1, 1, 1, 1 };
    FTRemixAiDiOut out;

    in.hit_count = 0;
    in.menu_first_hit = 1;
    in.is_in_hitlag = 0;
    ftRemixAiDiDecide(&in, saved, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
    CHECK_EQ(-1, out.params[FTREMIXAIDI_PARAM_ADVANCED]);
    CHECK_EQ(-1, out.params[FTREMIXAIDI_PARAM_TYPE]);
}

static void testLevelTenUsesHighStrengthRoutines(void)
{
    FTRemixAiDiIn in = makeVersusInput(10);
    FTRemixAiDiOut out;

    resetRandom(3);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(1, out.has_routine);
    CHECK_EQ(0, out.is_slide);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_UP, out.direction);
    CHECK_EQ(FTREMIXAIDI_STRENGTH_HIGH, out.strength);
    CHECK_EQ(3, out.params[FTREMIXAIDI_PARAM_ADVANCED]);
    resetRandom(6);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(1, out.is_slide);
    CHECK_EQ(FTREMIXAIDI_DIRECTION_RIGHT, out.direction);
}

static void testAdvancedRollOfZeroMeansNoDi(void)
{
    FTRemixAiDiIn in = makeVersusInput(10);
    FTRemixAiDiOut out;

    resetRandom(0);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
}

static void testAdvancedAiScalesStrengthWithTheLevel(void)
{
    FTRemixAiDiIn in = makeVersusInput(3);
    FTRemixAiDiOut out;

    resetRandom(1);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
    in.cpu_level = 5;
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(FTREMIXAIDI_STRENGTH_LOW, out.strength);
    in.cpu_level = 8;
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(FTREMIXAIDI_STRENGTH_MEDIUM, out.strength);
}

static void testWithoutAdvancedAiNothingRollsBelowLevelTen(void)
{
    FTRemixAiDiIn in = makeVersusInput(8);
    FTRemixAiDiOut out;

    in.uses_advanced_ai = 0;
    resetRandom(2);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
    CHECK_EQ(0, sFakeRandomCalls);
}

static void testEarlyHitNeedsTenFramesOfHitlagStatus(void)
{
    FTRemixAiDiIn in = makeVersusInput(10);
    FTRemixAiDiOut out;

    in.hit_count = 1;
    in.status_frame = 9;
    resetRandom(2);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
    in.status_frame = 10;
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(1, out.has_routine);
}

static void testNonDamageStatusSkipsTheDi(void)
{
    FTRemixAiDiIn in = makeVersusInput(10);
    FTRemixAiDiOut out;

    in.is_damage_status = 0;
    resetRandom(2);
    ftRemixAiDiDecide(&in, sNotSet, fakeRandom, &out);
    CHECK_EQ(0, out.has_routine);
}

static void testSmashScriptBytes(void)
{
    static const uint8_t expected[] = { 0xB0, 0x00, 0xA1, 0xB0, 0xB0, 0x00, 0xA1, 0x00, 0xFF };
    uint8_t script[FTREMIXAIDI_SCRIPT_SIZE];
    int32_t length = ftRemixAiDiBuildScript(0, FTREMIXAIDI_STRENGTH_HIGH, FTREMIXAIDI_DIRECTION_LEFT, script);

    CHECK_EQ((int32_t)sizeof(expected), length);
    CHECK_EQ(0, memcmp(expected, script, sizeof(expected)));
}

static void testSlideScriptSweepsTheOtherAxis(void)
{
    static const uint8_t expected_up[] =
    {
        0xB0, 0x50, 0xA1, 0x00, 0xB0, 0x50, 0xA1, 0xB0, 0xB0, 0x50, 0xA1, 0x00, 0xB0, 0x50, 0xA1, 0x50, 0xFF
    };
    static const uint8_t expected_left_low[] =
    {
        0xB0, 0x00, 0xA5, 0xB0, 0xB0, 0xB0, 0xA5, 0xB0, 0xB0, 0x00, 0xA5, 0xB0, 0xB0, 0x50, 0xA5, 0xB0, 0xFF
    };
    uint8_t script[FTREMIXAIDI_SCRIPT_SIZE];
    int32_t length = ftRemixAiDiBuildScript(1, FTREMIXAIDI_STRENGTH_HIGH, FTREMIXAIDI_DIRECTION_UP, script);

    CHECK_EQ((int32_t)sizeof(expected_up), length);
    CHECK_EQ(0, memcmp(expected_up, script, sizeof(expected_up)));
    length = ftRemixAiDiBuildScript(1, FTREMIXAIDI_STRENGTH_LOW, FTREMIXAIDI_DIRECTION_LEFT, script);
    CHECK_EQ((int32_t)sizeof(expected_left_low), length);
    CHECK_EQ(0, memcmp(expected_left_low, script, sizeof(expected_left_low)));
}

static void testMediumStrengthWaitsThreeFrames(void)
{
    uint8_t script[FTREMIXAIDI_SCRIPT_SIZE];

    ftRemixAiDiBuildScript(0, FTREMIXAIDI_STRENGTH_MEDIUM, FTREMIXAIDI_DIRECTION_DOWN, script);
    CHECK_EQ(0xA3, script[2]);
    CHECK_EQ(0xB0, script[1]);
}

void remixTestAiDiRules(void)
{
    printf("ai cpu di rules\n");
    RUN_TEST(testTrainingMenuPicksTheExactRoutine);
    RUN_TEST(testTrainingNoneTypeDoesNothing);
    RUN_TEST(testTrainingWaitsForTheFirstHit);
    RUN_TEST(testAwayAndTowardFollowTheFacing);
    RUN_TEST(testRandomTypeRollsOnceAndStoresIt);
    RUN_TEST(testSavedComboParametersAreReusedWithoutRolling);
    RUN_TEST(testParametersResetWhenNoComboIsRunning);
    RUN_TEST(testLevelTenUsesHighStrengthRoutines);
    RUN_TEST(testAdvancedRollOfZeroMeansNoDi);
    RUN_TEST(testAdvancedAiScalesStrengthWithTheLevel);
    RUN_TEST(testWithoutAdvancedAiNothingRollsBelowLevelTen);
    RUN_TEST(testEarlyHitNeedsTenFramesOfHitlagStatus);
    RUN_TEST(testNonDamageStatusSkipsTheDi);
    RUN_TEST(testSmashScriptBytes);
    RUN_TEST(testSlideScriptSweepsTheOtherAxis);
    RUN_TEST(testMediumStrengthWaitsThreeFrames);
}
