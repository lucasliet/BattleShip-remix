/**
 * test_1p_retry_rules.c - unit tests of the L retry rules of the Remix bonus runs (sc/sc1pmode/sc1premixretryrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1premixretryrules.h"

#define BUTTON_L 0x0020
#define BUTTON_A 0x8000

static void testFakeScreenModesAreTheBonusRuns(void)
{
    CHECK_TRUE(sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_BONUS3));
    CHECK_TRUE(sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_MULTIMAN));
    CHECK_TRUE(sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_CRUEL));
    CHECK_TRUE(sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_HRC));
}

static void testOtherModesAreNotFakeScreenModes(void)
{
    CHECK_EQ(0, sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_STANDARD));
    CHECK_EQ(0, sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_REMIX_1P));
    CHECK_EQ(0, sc1PRemixRetryIsFakeScreenMode(SC1PREMIX_MODE_ALLSTAR));
    CHECK_EQ(0, sc1PRemixRetryIsFakeScreenMode(-1));
}

static void testPauseRetryNeedsLInAFakeScreenRun(void)
{
    CHECK_EQ(1, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_MULTIMAN, 1, BUTTON_L));
    CHECK_EQ(1, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_HRC, 1, BUTTON_L | BUTTON_A));
    CHECK_EQ(0, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_HRC, 1, BUTTON_A));
    CHECK_EQ(0, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_HRC, 1, 0));
}

static void testPauseRetryIgnoresOtherModesAndScenes(void)
{
    CHECK_EQ(0, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_STANDARD, 1, BUTTON_L));
    CHECK_EQ(0, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_ALLSTAR, 1, BUTTON_L));
    CHECK_EQ(0, sc1PRemixRetryIsPauseRetry(SC1PREMIX_MODE_BONUS3, 0, BUTTON_L));
}

static void testGameSetRetryNeedsLHeldInAFakeScreenRun(void)
{
    CHECK_EQ(1, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_BONUS3, 1, 0, BUTTON_L));
    CHECK_EQ(1, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_CRUEL, 1, 0, BUTTON_L | BUTTON_A));
    CHECK_EQ(0, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_CRUEL, 1, 0, BUTTON_A));
    CHECK_EQ(0, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_CRUEL, 1, 0, 0));
}

static void testGameSetRetryWorksInABonusPracticeRun(void)
{
    CHECK_EQ(1, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_STANDARD, 0, 1, BUTTON_L));
    CHECK_EQ(0, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_STANDARD, 0, 1, BUTTON_A));
}

static void testGameSetRetryLeavesTheOtherRunsAlone(void)
{
    CHECK_EQ(0, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_ALLSTAR, 1, 0, BUTTON_L));
    CHECK_EQ(0, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_STANDARD, 1, 0, BUTTON_L));
    CHECK_EQ(0, sc1PRemixRetryIsGameSetRetry(SC1PREMIX_MODE_HRC, 0, 0, BUTTON_L));
}

void remixTest1PRetryRules(void)
{
    printf("1p retry rules\n");
    RUN_TEST(testFakeScreenModesAreTheBonusRuns);
    RUN_TEST(testOtherModesAreNotFakeScreenModes);
    RUN_TEST(testPauseRetryNeedsLInAFakeScreenRun);
    RUN_TEST(testPauseRetryIgnoresOtherModesAndScenes);
    RUN_TEST(testGameSetRetryNeedsLHeldInAFakeScreenRun);
    RUN_TEST(testGameSetRetryWorksInABonusPracticeRun);
    RUN_TEST(testGameSetRetryLeavesTheOtherRunsAlone);
}
