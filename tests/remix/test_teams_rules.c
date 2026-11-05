/**
 * test_teams_rules.c - unit tests of the Remix yellow team rules (mn/mnplayers/mnremixteamsrules.c).
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixteamsrules.h"

static void testTheTeamButtonCyclesThroughFourTeams(void)
{
    CHECK_EQ(1, mnRemixTeamsRulesNextTeam(0));
    CHECK_EQ(2, mnRemixTeamsRulesNextTeam(1));
    CHECK_EQ(3, mnRemixTeamsRulesNextTeam(2));
    CHECK_EQ(0, mnRemixTeamsRulesNextTeam(3));
}

static void testOnlyYellowIsAnAddedTeam(void)
{
    CHECK_EQ(0, mnRemixTeamsRulesIsAdded(0));
    CHECK_EQ(0, mnRemixTeamsRulesIsAdded(2));
    CHECK_EQ(1, mnRemixTeamsRulesIsAdded(MNREMIX_TEAM_YELLOW));
    CHECK_EQ(0, mnRemixTeamsRulesIsAdded(4));
    CHECK_EQ(0, mnRemixTeamsRulesIsAdded(-1));
}

static void testVanillaColorsKeepGreenOnIndexThree(void)
{
    CHECK_EQ(0, mnRemixTeamsRulesVanillaColor(0));
    CHECK_EQ(1, mnRemixTeamsRulesVanillaColor(1));
    CHECK_EQ(3, mnRemixTeamsRulesVanillaColor(2));
    CHECK_EQ(-1, mnRemixTeamsRulesVanillaColor(3));
    CHECK_EQ(-1, mnRemixTeamsRulesVanillaColor(-1));
}

static void testCostumeIndexIsOneByteAfterTheCharacterRow(void)
{
    CHECK_EQ(0x3B, mnRemixTeamsRulesCostumeIndex(0x3B, MNREMIX_TEAM_YELLOW));
    CHECK_EQ(0, mnRemixTeamsRulesCostumeIndex(0, MNREMIX_TEAM_YELLOW));
    CHECK_EQ(-1, mnRemixTeamsRulesCostumeIndex(0x3B, 2));
    CHECK_EQ(-1, mnRemixTeamsRulesCostumeIndex(-1, MNREMIX_TEAM_YELLOW));
}

static void testBigTextEntriesAreSixBytes(void)
{
    CHECK_EQ(0, mnRemixTeamsRulesBigTextOffset(0));
    CHECK_EQ(30, mnRemixTeamsRulesBigTextOffset(5));
}

static void testWordsAreFloatBitPatterns(void)
{
    CHECK_TRUE(mnRemixTeamsRulesWordToFloat(0x41D00000u) == 26.0F);
    CHECK_TRUE(mnRemixTeamsRulesWordToFloat(0x3F600000u) == 0.875F);
    CHECK_TRUE(mnRemixTeamsRulesWordToFloat(0x3F800000u) == 1.0F);
}

static void testDefaultShieldFollowsTheTeamOnlyInAVsTeamBattle(void)
{
    CHECK_EQ(MNREMIX_TEAMS_SHIELD_TEAM, mnRemixTeamsRulesShieldSource(1, 0));
    CHECK_EQ(MNREMIX_TEAMS_SHIELD_TEAM, mnRemixTeamsRulesShieldSource(1, 1));
    CHECK_EQ(MNREMIX_TEAMS_SHIELD_CPU, mnRemixTeamsRulesShieldSource(0, 1));
    CHECK_EQ(MNREMIX_TEAMS_SHIELD_PORT, mnRemixTeamsRulesShieldSource(0, 0));
}

static void testTheResultsCountYellow(void)
{
    CHECK_EQ(3, mnRemixTeamsRulesLastResultTeam());
}

void remixTestTeamsRules(void)
{
    printf("yellow team rules\n");
    RUN_TEST(testTheTeamButtonCyclesThroughFourTeams);
    RUN_TEST(testOnlyYellowIsAnAddedTeam);
    RUN_TEST(testVanillaColorsKeepGreenOnIndexThree);
    RUN_TEST(testCostumeIndexIsOneByteAfterTheCharacterRow);
    RUN_TEST(testBigTextEntriesAreSixBytes);
    RUN_TEST(testWordsAreFloatBitPatterns);
    RUN_TEST(testDefaultShieldFollowsTheTeamOnlyInAVsTeamBattle);
    RUN_TEST(testTheResultsCountYellow);
}
