/**
 * test_smashketball_rules.c - unit tests of the Smashketball scoring rules (gm/gmremixsmashketballrules.c).
 */

#include "test_check.h"

#include "gm/gmremixsmashketballrules.h"

#define NO_TEAM GMREMIX_SMASHKETBALL_RULES_NO_TEAM
#define HOME 0
#define AWAY 1
#define PLAYERS GMREMIX_SMASHKETBALL_RULES_PLAYERS

static void testTheLeftHoopCreditsAwayAndTheRightOneHome(void)
{
    CHECK_EQ(AWAY, gmRemixSmashketballRulesScoringTeam(-1, HOME, AWAY));
    CHECK_EQ(HOME, gmRemixSmashketballRulesScoringTeam(1, HOME, AWAY));
}

static void testAGoldenBallIsWorthThreePointsAndANormalOneWorthOne(void)
{
    CHECK_EQ(GMREMIX_SMASHKETBALL_RULES_GOLDEN_POINTS, gmRemixSmashketballRulesBasketPoints(1));
    CHECK_EQ(3, gmRemixSmashketballRulesBasketPoints(1));
    CHECK_EQ(1, gmRemixSmashketballRulesBasketPoints(0));
}

static void testOnlyAnOwnerOfTheScoringTeamScoresForThePlayer(void)
{
    CHECK_EQ(1, gmRemixSmashketballRulesIsOwnerScoring(1, HOME, HOME));
    CHECK_EQ(0, gmRemixSmashketballRulesIsOwnerScoring(1, AWAY, HOME));
    CHECK_EQ(0, gmRemixSmashketballRulesIsOwnerScoring(0, NO_TEAM, HOME));
}

static void testTheWinnerShareFollowsTheTableByTeammates(void)
{
    CHECK_EQ(6, gmRemixSmashketballRulesWinnerShare(GMREMIX_SMASHKETBALL_RULES_ATTRIBUTED_SHARE, 1));
    CHECK_EQ(6, gmRemixSmashketballRulesWinnerShare(1, 1));
    CHECK_EQ(3, gmRemixSmashketballRulesWinnerShare(2, 1));
    CHECK_EQ(2, gmRemixSmashketballRulesWinnerShare(3, 1));
}

static void testTheWinnerShareScalesWithThePoints(void)
{
    CHECK_EQ(18, gmRemixSmashketballRulesWinnerShare(GMREMIX_SMASHKETBALL_RULES_ATTRIBUTED_SHARE, 3));
    CHECK_EQ(6, gmRemixSmashketballRulesWinnerShare(2, 2));
}

static void testATeamScoreIsZeroWithoutATeam(void)
{
    int team_scores[PLAYERS] = { 4, 7, 0, 0 };

    CHECK_EQ(4, gmRemixSmashketballRulesTeamScore(HOME, team_scores));
    CHECK_EQ(7, gmRemixSmashketballRulesTeamScore(AWAY, team_scores));
    CHECK_EQ(0, gmRemixSmashketballRulesTeamScore(NO_TEAM, team_scores));
}

static void testTheFirstFighterSetsTheHomeTeam(void)
{
    int home_team = NO_TEAM;
    int away_team = NO_TEAM;

    gmRemixSmashketballRulesRecordTeam(&home_team, &away_team, AWAY);

    CHECK_EQ(AWAY, home_team);
    CHECK_EQ(NO_TEAM, away_team);
}

static void testAFighterOfTheSameTeamDoesNotSetTheAwayTeam(void)
{
    int home_team = HOME;
    int away_team = NO_TEAM;

    gmRemixSmashketballRulesRecordTeam(&home_team, &away_team, HOME);

    CHECK_EQ(HOME, home_team);
    CHECK_EQ(NO_TEAM, away_team);
}

static void testAFighterOfAnotherTeamSetsTheAwayTeam(void)
{
    int home_team = HOME;
    int away_team = NO_TEAM;

    gmRemixSmashketballRulesRecordTeam(&home_team, &away_team, AWAY);

    CHECK_EQ(HOME, home_team);
    CHECK_EQ(AWAY, away_team);
}

static void testTheHomeTeamIsNotChangedByLaterFighters(void)
{
    int home_team = HOME;
    int away_team = NO_TEAM;

    gmRemixSmashketballRulesRecordTeam(&home_team, &away_team, AWAY);
    gmRemixSmashketballRulesRecordTeam(&home_team, &away_team, HOME);

    CHECK_EQ(HOME, home_team);
    CHECK_EQ(AWAY, away_team);
}

static void testAnAttributedBasketResetsTheOtherStreaks(void)
{
    int streaks[PLAYERS] = { 2, 3, 0, 1 };

    gmRemixSmashketballRulesRecordBasket(streaks, 1);

    CHECK_EQ(0, streaks[0]);
    CHECK_EQ(4, streaks[1]);
    CHECK_EQ(0, streaks[2]);
    CHECK_EQ(0, streaks[3]);
}

static void testTheFirstBasketStartsTheStreakOfThePlayer(void)
{
    int streaks[PLAYERS] = { 0, 0, 0, 0 };

    gmRemixSmashketballRulesRecordBasket(streaks, 2);

    CHECK_EQ(1, streaks[2]);
    CHECK_EQ(0, streaks[0]);
}

static void testTeamColorsAreTheColorTagValuesOfTheAsm(void)
{
    CHECK_EQ(0xED3636FF, gmRemixSmashketballRulesTeamColor(0));
    CHECK_EQ(0x4E4EE9FF, gmRemixSmashketballRulesTeamColor(1));
    CHECK_EQ(0x4EB94EFF, gmRemixSmashketballRulesTeamColor(2));
    CHECK_EQ(0xFFDF1AFF, gmRemixSmashketballRulesTeamColor(3));
}

static void testTheCrowdChantsFromThreeBasketsInARow(void)
{
    CHECK_EQ(0, gmRemixSmashketballRulesIsChantStreak(2));
    CHECK_EQ(1, gmRemixSmashketballRulesIsChantStreak(3));
    CHECK_EQ(1, gmRemixSmashketballRulesIsChantStreak(5));
}

static void testTheCameraGrowsToTheBallWithPaddingAndAlwaysShowsTheFloor(void)
{
    float left = -500.0F;
    float right = 500.0F;
    float bottom = -200.0F;
    float top = 800.0F;

    gmRemixSmashketballRulesFollowBall(2000.0F, 1500.0F, &left, &right, &bottom, &top);

    CHECK_EQ(-500, (int)left);
    CHECK_EQ(3000, (int)right);
    CHECK_EQ(-1000, (int)bottom);
    CHECK_EQ(2500, (int)top);
}

static void testTheCameraKeepsABottomBelowTheFloorAndOnlyTheOuterBoundsGrow(void)
{
    float left = -5000.0F;
    float right = 5000.0F;
    float bottom = -3000.0F;
    float top = 4000.0F;

    gmRemixSmashketballRulesFollowBall(100.0F, 100.0F, &left, &right, &bottom, &top);

    CHECK_EQ(-5000, (int)left);
    CHECK_EQ(5000, (int)right);
    CHECK_EQ(-3000, (int)bottom);
    CHECK_EQ(4000, (int)top);
}

void remixTestSmashketballRules(void)
{
    printf("smashketball rules\n");
    RUN_TEST(testTheLeftHoopCreditsAwayAndTheRightOneHome);
    RUN_TEST(testAGoldenBallIsWorthThreePointsAndANormalOneWorthOne);
    RUN_TEST(testOnlyAnOwnerOfTheScoringTeamScoresForThePlayer);
    RUN_TEST(testTheWinnerShareFollowsTheTableByTeammates);
    RUN_TEST(testTheWinnerShareScalesWithThePoints);
    RUN_TEST(testATeamScoreIsZeroWithoutATeam);
    RUN_TEST(testTheFirstFighterSetsTheHomeTeam);
    RUN_TEST(testAFighterOfTheSameTeamDoesNotSetTheAwayTeam);
    RUN_TEST(testAFighterOfAnotherTeamSetsTheAwayTeam);
    RUN_TEST(testTheHomeTeamIsNotChangedByLaterFighters);
    RUN_TEST(testAnAttributedBasketResetsTheOtherStreaks);
    RUN_TEST(testTheFirstBasketStartsTheStreakOfThePlayer);
    RUN_TEST(testTeamColorsAreTheColorTagValuesOfTheAsm);
    RUN_TEST(testTheCrowdChantsFromThreeBasketsInARow);
    RUN_TEST(testTheCameraGrowsToTheBallWithPaddingAndAlwaysShowsTheFloor);
    RUN_TEST(testTheCameraKeepsABottomBelowTheFloorAndOnlyTheOuterBoundsGrow);
}
