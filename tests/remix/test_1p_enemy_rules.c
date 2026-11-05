/**
 * test_1p_enemy_rules.c - unit tests of the Single Player Enemy rules (sc/sc1pmode/sc1premixenemyrules.c) and of the Training gate
 * of Single Button Mode (sc/sc1pmode/sc1ptrainingsbmrules.c).
 */

#include "test_check.h"

#include "sc/sc1pmode/sc1premixenemyrules.h"
#include "sc/sc1pmode/sc1premixmodename.h"
#include "sc/sc1pmode/sc1ptrainingsbmrules.h"

static void testZOfAnotherPortPicksTheEnemyAndLaughsOnce(void)
{
    SC1PRemixEnemySelection selection = { 0, 0 };

    CHECK_EQ(1, sc1PRemixEnemySelectOnZ(&selection, 2, 0));
    CHECK_EQ(2, selection.enemy_port);
    CHECK_EQ(0, sc1PRemixEnemySelectOnZ(&selection, 4, 0));
    CHECK_EQ(4, selection.enemy_port);
    CHECK_EQ(1, selection.is_selected);
}

static void testZOfThePlayingPortOrNobodyChangesNothing(void)
{
    SC1PRemixEnemySelection selection = { 0, 0 };

    CHECK_EQ(0, sc1PRemixEnemySelectOnZ(&selection, 1, 0));
    CHECK_EQ(0, sc1PRemixEnemySelectOnZ(&selection, 0, 0));
    CHECK_EQ(0, sc1PRemixEnemySelectOnZ(&selection, 3, 2));
    CHECK_EQ(0, selection.enemy_port);
    CHECK_EQ(0, selection.is_selected);
}

static void testBOfAnotherPortClearsTheEnemy(void)
{
    CHECK_TRUE(sc1PRemixEnemyIsClearedByB(0, 0x2));
    CHECK_TRUE(sc1PRemixEnemyIsClearedByB(1, 0x9));
    CHECK_EQ(0, sc1PRemixEnemyIsClearedByB(0, 0x1));
    CHECK_EQ(0, sc1PRemixEnemyIsClearedByB(2, 0x4));
    CHECK_EQ(0, sc1PRemixEnemyIsClearedByB(0, 0));
}

static void testOnlyTheFirstEnemyIsHandedToThePort(void)
{
    CHECK_EQ(1, sc1PRemixEnemyControlPort(2, SC1PREMIX_MODE_STANDARD, 1));
    CHECK_EQ(3, sc1PRemixEnemyControlPort(4, SC1PREMIX_MODE_ALLSTAR, 1));
    CHECK_EQ(-1, sc1PRemixEnemyControlPort(2, SC1PREMIX_MODE_STANDARD, 0));
    CHECK_EQ(-1, sc1PRemixEnemyControlPort(0, SC1PREMIX_MODE_STANDARD, 1));
}

static void testTheHomeRunContestKeepsItsCpu(void)
{
    CHECK_EQ(-1, sc1PRemixEnemyControlPort(2, SC1PREMIX_MODE_HRC, 1));
    CHECK_EQ(1, sc1PRemixEnemyRespawnPort(2, 1));
}

static void testRespawnFollowsTheSameChoice(void)
{
    CHECK_EQ(0, sc1PRemixEnemyRespawnPort(1, 1));
    CHECK_EQ(-1, sc1PRemixEnemyRespawnPort(1, 0));
    CHECK_EQ(-1, sc1PRemixEnemyRespawnPort(0, 1));
}

static void testShieldTakesTheColourOfTheControllingPort(void)
{
    CHECK_EQ(2, sc1PRemixEnemyShieldPort(1, 3, 1, 2, 1));
    CHECK_EQ(0, sc1PRemixEnemyShieldPort(1, 1, 1, 0, 1));
}

static void testShieldKeepsItsColourOutsideTheEnemyCase(void)
{
    CHECK_EQ(1, sc1PRemixEnemyShieldPort(0, 3, 1, 2, 1));
    CHECK_EQ(1, sc1PRemixEnemyShieldPort(1, 0, 1, 2, 1));
    CHECK_EQ(1, sc1PRemixEnemyShieldPort(1, 3, 0, 2, 1));
    CHECK_EQ(1, sc1PRemixEnemyShieldPort(1, 3, 1, -1, 1));
    CHECK_EQ(1, sc1PRemixEnemyShieldPort(1, 3, 1, 4, 1));
}

static void testBlinkRunsEightyFramesAndShowsTheFirstForty(void)
{
    int timer = 0;
    int shown_frames = 0;
    int frame;

    for (frame = 0; frame < 80; frame++)
    {
        shown_frames += sc1PRemixEnemyBlinkIsOn(timer);
        timer = sc1PRemixEnemyBlinkNext(timer);
    }
    CHECK_EQ(0, timer);
    CHECK_EQ(40, shown_frames);
    CHECK_EQ(1, sc1PRemixEnemyBlinkNext(0));
    CHECK_EQ(0, sc1PRemixEnemyBlinkNext(79));
    CHECK_EQ(0, sc1PRemixEnemyBlinkIsOn(79));
}

static void testLabelsFollowTheEnemyPort(void)
{
    CHECK_TRUE(sc1PRemixEnemyTextIsVisible(0, 1));
    CHECK_EQ(0, sc1PRemixEnemyTextIsVisible(0, 0));
    CHECK_EQ(0, sc1PRemixEnemyTextIsVisible(2, 1));
    CHECK_TRUE(sc1PRemixEnemyPortLabelIsVisible(2, 2));
    CHECK_EQ(0, sc1PRemixEnemyPortLabelIsVisible(2, 3));
    CHECK_EQ(0, sc1PRemixEnemyPortLabelIsVisible(0, 1));
}

static void testTrainingMenuIsUpFromTheStatusOrTheZMenu(void)
{
    CHECK_EQ(0, sc1PTrainingSbmIsMenuUp(sc1PTrainingSbmGetToggleMenu(0, 0)));
    CHECK_EQ(0, sc1PTrainingSbmIsMenuUp(sc1PTrainingSbmGetToggleMenu(SC1PTRAININGSBM_MENU_BOTH_DOWN, 0)));
    CHECK_TRUE(sc1PTrainingSbmIsMenuUp(sc1PTrainingSbmGetToggleMenu(SC1PTRAININGSBM_MENU_SSB_UP, 0)));
    CHECK_TRUE(sc1PTrainingSbmIsMenuUp(sc1PTrainingSbmGetToggleMenu(SC1PTRAININGSBM_MENU_SSB_UP, 1)));
    CHECK_EQ(SC1PTRAININGSBM_MENU_CUSTOM_UP, sc1PTrainingSbmGetToggleMenu(SC1PTRAININGSBM_MENU_SSB_UP, 1));
}

void remixTest1PEnemyRules(void)
{
    printf("1p enemy rules\n");
    RUN_TEST(testZOfAnotherPortPicksTheEnemyAndLaughsOnce);
    RUN_TEST(testZOfThePlayingPortOrNobodyChangesNothing);
    RUN_TEST(testBOfAnotherPortClearsTheEnemy);
    RUN_TEST(testOnlyTheFirstEnemyIsHandedToThePort);
    RUN_TEST(testTheHomeRunContestKeepsItsCpu);
    RUN_TEST(testRespawnFollowsTheSameChoice);
    RUN_TEST(testShieldTakesTheColourOfTheControllingPort);
    RUN_TEST(testShieldKeepsItsColourOutsideTheEnemyCase);
    RUN_TEST(testBlinkRunsEightyFramesAndShowsTheFirstForty);
    RUN_TEST(testLabelsFollowTheEnemyPort);
    RUN_TEST(testTrainingMenuIsUpFromTheStatusOrTheZMenu);
}
