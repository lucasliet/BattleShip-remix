/**
 * test_koth_rules.c - unit tests of the King of the Hill king rules (gm/gmremixkothrules.c).
 */

#include "test_check.h"

#include "gm/gmremixkothrules.h"

#define BUFFER 30

static void testAnEmptyHillHasNoKing(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 0, 0, 0 }, { 0, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_NO_KING, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(0, eligible[0]);
}

static void testOneFighterOnTheHillIsTheCandidate(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 0, 0, 0 }, { 1, 0, 1 } };
    int eligible[2];

    CHECK_EQ(1, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(1, eligible[1]);
}

static void testTeammatesShareTheHill(void)
{
    GMRemixKothRulesFighter fighters[3] = { { 1, 0, 2 }, { 1, 0, 2 }, { 0, 0, 0 } };
    int eligible[3];

    CHECK_EQ(2, gmRemixKothRulesCandidate(fighters, 3, eligible));
    CHECK_EQ(1, eligible[0]);
    CHECK_EQ(1, eligible[1]);
}

static void testTwoTeamsOnTheHillContestIt(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 1, 0, 0 }, { 1, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesCandidate(fighters, 2, eligible));
}

static void testHoldingAnEnemyContestsTheHillAndLosesTheClaim(void)
{
    GMRemixKothRulesFighter fighters[2] = { { 1, 1, 0 }, { 0, 0, 1 } };
    int eligible[2];

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesCandidate(fighters, 2, eligible));
    CHECK_EQ(0, eligible[0]);
}

static void testAContestStartsTheBuffer(void)
{
    int buffer = 0;

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(GMREMIX_KOTHRULES_CONTESTED, &buffer, BUFFER));
    CHECK_EQ(BUFFER, buffer);
}

static void testTheBufferHoldsTheHillUntilItRunsOut(void)
{
    int buffer = 2;

    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(1, &buffer, BUFFER));
    CHECK_EQ(GMREMIX_KOTHRULES_CONTESTED, gmRemixKothRulesSettle(1, &buffer, BUFFER));
    CHECK_EQ(1, gmRemixKothRulesSettle(1, &buffer, BUFFER));
}

static void testMeterSitsAtTheBottomUnlessTheDragonKingHudApplies(void)
{
    float frame_y;
    float bar_y;

    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_DEFAULT, 0, &frame_y, &bar_y);
    CHECK_EQ(180, (int)frame_y);
    CHECK_EQ(188, (int)bar_y);
    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_DEFAULT, 1, &frame_y, &bar_y);
    CHECK_EQ(8, (int)frame_y);
    CHECK_EQ(16, (int)bar_y);
    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_ALWAYS, 0, &frame_y, &bar_y);
    CHECK_EQ(16, (int)bar_y);
    gmRemixKothRulesMeterY(GMREMIX_KOTHRULES_DK_HUD_OFF, 1, &frame_y, &bar_y);
    CHECK_EQ(188, (int)bar_y);
}

static void testMeterXFollowsThePort(void)
{
    CHECK_EQ(40, (int)gmRemixKothRulesMeterX(0));
    CHECK_EQ(110, (int)gmRemixKothRulesMeterX(1));
    CHECK_EQ(250, (int)gmRemixKothRulesMeterX(3));
}

static void testFfaTagsFollowTheColorOrderOfTheAsmAndACpuIsGray(void)
{
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_RED, gmRemixKothRulesTag(0, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_BLUE, gmRemixKothRulesTag(1, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_YELLOW, gmRemixKothRulesTag(2, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_GREEN, gmRemixKothRulesTag(3, 0, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_GRAY, gmRemixKothRulesTag(2, 0, 1));
}

static void testTeamTagsUseTheTeamColorOrderAndNeverGray(void)
{
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_RED, gmRemixKothRulesTag(0, 1, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_BLUE, gmRemixKothRulesTag(1, 1, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_GREEN, gmRemixKothRulesTag(2, 1, 0));
    CHECK_EQ(GMREMIX_KOTHRULES_TAG_YELLOW, gmRemixKothRulesTag(3, 1, 1));
}

static void testTagColorsAreTheColorTagValuesOfTheAsm(void)
{
    CHECK_EQ(0xED3636FF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_RED));
    CHECK_EQ(0x4E4EE9FF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_BLUE));
    CHECK_EQ(0x4EB94EFF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_GREEN));
    CHECK_EQ(0xFFDF1AFF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_YELLOW));
    CHECK_EQ(0xACACACFF, gmRemixKothRulesTagColor(GMREMIX_KOTHRULES_TAG_GRAY));
}

static void testTheContestedGrayStartsDarkAndLightensAsTheBufferRunsOut(void)
{
    CHECK_EQ(0x6E, gmRemixKothRulesContestedGray(BUFFER));
    CHECK_EQ(0x8C, gmRemixKothRulesContestedGray(15));
    CHECK_EQ(0xA8, gmRemixKothRulesContestedGray(1));
}

void remixTestKothRules(void)
{
    printf("koth rules\n");
    RUN_TEST(testAnEmptyHillHasNoKing);
    RUN_TEST(testOneFighterOnTheHillIsTheCandidate);
    RUN_TEST(testTeammatesShareTheHill);
    RUN_TEST(testTwoTeamsOnTheHillContestIt);
    RUN_TEST(testHoldingAnEnemyContestsTheHillAndLosesTheClaim);
    RUN_TEST(testAContestStartsTheBuffer);
    RUN_TEST(testTheBufferHoldsTheHillUntilItRunsOut);
    RUN_TEST(testMeterSitsAtTheBottomUnlessTheDragonKingHudApplies);
    RUN_TEST(testMeterXFollowsThePort);
    RUN_TEST(testFfaTagsFollowTheColorOrderOfTheAsmAndACpuIsGray);
    RUN_TEST(testTeamTagsUseTheTeamColorOrderAndNeverGray);
    RUN_TEST(testTagColorsAreTheColorTagValuesOfTheAsm);
    RUN_TEST(testTheContestedGrayStartsDarkAndLightensAsTheBufferRunsOut);
}
