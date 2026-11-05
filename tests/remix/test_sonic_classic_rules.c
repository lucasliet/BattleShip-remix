/**
 * test_sonic_classic_rules.c - unit tests of the Classic Sonic rules (ft/ftremix/sonic/ftremixsonicclassicrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/sonic/ftremixsonicclassicrules.h"

static void testPlayFgmSwapsTheSixModernSounds(void)
{
    CHECK_EQ(0x3DE, ftRemixSonicClassicRulesPlayFgm(0x3D8));
    CHECK_EQ(0x3E3, ftRemixSonicClassicRulesPlayFgm(0x3D9));
    CHECK_EQ(0x3E3, ftRemixSonicClassicRulesPlayFgm(0x3DB));
    CHECK_EQ(0x3E0, ftRemixSonicClassicRulesPlayFgm(0x3DC));
    CHECK_EQ(0x3E1, ftRemixSonicClassicRulesPlayFgm(0x3DD));
    CHECK_EQ(0x3FD, ftRemixSonicClassicRulesPlayFgm(0x3FC));
}

static void testPlayFgmKeepsEveryOtherSound(void)
{
    CHECK_EQ(0x3DA, ftRemixSonicClassicRulesPlayFgm(0x3DA));
    CHECK_EQ(0x3DE, ftRemixSonicClassicRulesPlayFgm(0x3DE));
    CHECK_EQ(0x100, ftRemixSonicClassicRulesPlayFgm(0x100));
}

static void testPlayLoopFgmSwapsOnlyChargeAndHomingCharge(void)
{
    CHECK_EQ(0x3DE, ftRemixSonicClassicRulesPlayLoopFgm(0x3D8));
    CHECK_EQ(0x3E2, ftRemixSonicClassicRulesPlayLoopFgm(0x3DA));
    CHECK_EQ(0x3D9, ftRemixSonicClassicRulesPlayLoopFgm(0x3D9));
    CHECK_EQ(0x3DC, ftRemixSonicClassicRulesPlayLoopFgm(0x3DC));
}

static void testRightTogglesOnlyWhenTheCycleWrapsToZero(void)
{
    CHECK_TRUE(ftRemixSonicClassicRulesShouldToggle(0, FTREMIX_SONIC_BUTTON_RIGHT, 5, 0));
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(0, FTREMIX_SONIC_BUTTON_RIGHT, 2, 3));
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(0, FTREMIX_SONIC_BUTTON_RIGHT, 0, 1));
}

static void testLeftTogglesOnlyFromCostumeZero(void)
{
    CHECK_TRUE(ftRemixSonicClassicRulesShouldToggle(0, FTREMIX_SONIC_BUTTON_LEFT, 0, 5));
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(0, FTREMIX_SONIC_BUTTON_LEFT, 3, 2));
}

static void testShadeButtonsNeverToggle(void)
{
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(0, 0, 0, 0));
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(0, 2, 0, 0));
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(1, 0, 0, 0));
    CHECK_TRUE(!ftRemixSonicClassicRulesShouldToggle(1, 2, 0, 0));
}

static void testTeamBattleTogglesOnEveryHorizontalPress(void)
{
    CHECK_TRUE(ftRemixSonicClassicRulesShouldToggle(1, FTREMIX_SONIC_BUTTON_RIGHT, 2, 3));
    CHECK_TRUE(ftRemixSonicClassicRulesShouldToggle(1, FTREMIX_SONIC_BUTTON_LEFT, 3, 2));
}

static void testYellowTeamCostumeAfterCycle(void)
{
    CHECK_EQ(1, ftRemixSonicClassicRulesCycleTeamCostume(FTREMIX_SONIC_TEAM_YELLOW, 1, 4));
    CHECK_EQ(5, ftRemixSonicClassicRulesCycleTeamCostume(FTREMIX_SONIC_TEAM_YELLOW, 0, 4));
    CHECK_EQ(4, ftRemixSonicClassicRulesCycleTeamCostume(2, 1, 4));
}

static void testTeamCostumeOnlyChangesForClassicYellow(void)
{
    CHECK_EQ(1, ftRemixSonicClassicRulesTeamCostume(FTREMIX_SONIC_TEAM_YELLOW, 1, 5));
    CHECK_EQ(5, ftRemixSonicClassicRulesTeamCostume(FTREMIX_SONIC_TEAM_YELLOW, 0, 5));
    CHECK_EQ(3, ftRemixSonicClassicRulesTeamCostume(1, 1, 3));
}

static void testComparisonFudgeAddsSixForClassic(void)
{
    CHECK_EQ(6, ftRemixSonicClassicRulesFudgeCostume(0, 1));
    CHECK_EQ(9, ftRemixSonicClassicRulesFudgeCostume(3, 1));
    CHECK_EQ(3, ftRemixSonicClassicRulesFudgeCostume(3, 0));
    CHECK_TRUE(ftRemixSonicClassicRulesFudgeCostume(0, 1) != ftRemixSonicClassicRulesFudgeCostume(0, 0));
}

static void testTrainingCostumeSplitsClassicAtSix(void)
{
    int is_classic = 1;

    CHECK_EQ(5, ftRemixSonicClassicRulesSplitTrainingCostume(5, &is_classic));
    CHECK_EQ(0, is_classic);
    CHECK_EQ(0, ftRemixSonicClassicRulesSplitTrainingCostume(6, &is_classic));
    CHECK_EQ(1, is_classic);
    CHECK_EQ(4, ftRemixSonicClassicRulesSplitTrainingCostume(10, &is_classic));
    CHECK_EQ(1, is_classic);
}

static void testTrainingMenuMaxAddsSixOnlyForSonic(void)
{
    CHECK_EQ(12, ftRemixSonicClassicRulesTrainingMenuMax(6, 1));
    CHECK_EQ(6, ftRemixSonicClassicRulesTrainingMenuMax(6, 0));
    CHECK_EQ(4, ftRemixSonicClassicRulesTrainingMenuMax(4, 0));
}

static void testSelectFrameRestartsAtTheAnimationLengthFromZero(void)
{
    CHECK_TRUE(ftRemixSonicClassicRulesSelectFrame(0.0F) == 193.0F);
    CHECK_TRUE(ftRemixSonicClassicRulesSelectFrame(42.5F) == 42.5F);
}

static void testMenuAnimFilesAreReplacedOnlyForTheTwoPoses(void)
{
    CHECK_EQ(0xDD2, ftRemixSonicClassicRulesMenuAnimFile(0xDD1));
    CHECK_EQ(0xE3C, ftRemixSonicClassicRulesMenuAnimFile(0xDD7));
    CHECK_EQ(0, ftRemixSonicClassicRulesMenuAnimFile(0xDD2));
    CHECK_EQ(0, ftRemixSonicClassicRulesMenuAnimFile(0x123));
}

static void testMenuAnimScenesAreTheIntroAndTheResults(void)
{
    CHECK_TRUE(ftRemixSonicClassicRulesIsMenuAnimScene(0x0E));
    CHECK_TRUE(ftRemixSonicClassicRulesIsMenuAnimScene(0x18));
    CHECK_TRUE(!ftRemixSonicClassicRulesIsMenuAnimScene(0x10));
    CHECK_TRUE(!ftRemixSonicClassicRulesIsMenuAnimScene(0x16));
}

static void testLoadOneClearsTheOtherPorts(void)
{
    unsigned char table[4] = { 1, 1, 1, 1 };

    ftRemixSonicClassicRulesLoadOne(table, 4, 2, 1);
    CHECK_EQ(0, table[0]);
    CHECK_EQ(0, table[1]);
    CHECK_EQ(1, table[2]);
    CHECK_EQ(0, table[3]);

    ftRemixSonicClassicRulesLoadOne(table, 4, 2, 0);
    CHECK_EQ(0, table[2]);
}

static void testLoadTwoSetsHumanAndCpuAndIgnoresBadPorts(void)
{
    unsigned char table[4] = { 1, 0, 1, 1 };

    ftRemixSonicClassicRulesLoadTwo(table, 4, 0, 1, 1, 0);
    CHECK_EQ(1, table[0]);
    CHECK_EQ(0, table[1]);
    CHECK_EQ(0, table[2]);
    CHECK_EQ(0, table[3]);

    ftRemixSonicClassicRulesLoadTwo(table, 4, 7, 1, -1, 1);
    CHECK_EQ(0, table[0]);
    CHECK_EQ(0, table[3]);
}

void remixTestSonicClassicRules(void)
{
    printf("classic sonic rules\n");
    RUN_TEST(testPlayFgmSwapsTheSixModernSounds);
    RUN_TEST(testPlayFgmKeepsEveryOtherSound);
    RUN_TEST(testPlayLoopFgmSwapsOnlyChargeAndHomingCharge);
    RUN_TEST(testRightTogglesOnlyWhenTheCycleWrapsToZero);
    RUN_TEST(testLeftTogglesOnlyFromCostumeZero);
    RUN_TEST(testShadeButtonsNeverToggle);
    RUN_TEST(testTeamBattleTogglesOnEveryHorizontalPress);
    RUN_TEST(testYellowTeamCostumeAfterCycle);
    RUN_TEST(testTeamCostumeOnlyChangesForClassicYellow);
    RUN_TEST(testComparisonFudgeAddsSixForClassic);
    RUN_TEST(testTrainingCostumeSplitsClassicAtSix);
    RUN_TEST(testTrainingMenuMaxAddsSixOnlyForSonic);
    RUN_TEST(testSelectFrameRestartsAtTheAnimationLengthFromZero);
    RUN_TEST(testMenuAnimFilesAreReplacedOnlyForTheTwoPoses);
    RUN_TEST(testMenuAnimScenesAreTheIntroAndTheResults);
    RUN_TEST(testLoadOneClearsTheOtherPorts);
    RUN_TEST(testLoadTwoSetsHumanAndCpuAndIgnoresBadPorts);
}
