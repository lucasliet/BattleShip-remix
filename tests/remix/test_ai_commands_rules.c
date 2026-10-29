/**
 * test_ai_commands_rules.c - unit tests of the Remix CPU stick and turn rules (ft/ftremix/ftremixaicommandsrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaicommandsrules.h"

#define LR_RIGHT 1
#define LR_LEFT (-1)

static void testAutofullAwayPointsAwayFromTheTarget(void)
{
    CHECK_EQ(FTREMIX_AI_STICK_FULL, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_AUTOFULL_AWAY, 10.0F, 0.0F, LR_LEFT));
    CHECK_EQ(-FTREMIX_AI_STICK_FULL, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_AUTOFULL_AWAY, -10.0F, 0.0F, LR_RIGHT));
}

static void testAutohalfAwayUsesHalfMagnitude(void)
{
    CHECK_EQ(FTREMIX_AI_STICK_HALF, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_AUTOHALF_AWAY, 10.0F, 0.0F, LR_LEFT));
    CHECK_EQ(-FTREMIX_AI_STICK_HALF, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_AUTOHALF_AWAY, -10.0F, 0.0F, LR_RIGHT));
}

static void testForwardsFollowsTheFacing(void)
{
    CHECK_EQ(FTREMIX_AI_STICK_FULL, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_FORWARDS, 0.0F, 0.0F, LR_RIGHT));
    CHECK_EQ(-FTREMIX_AI_STICK_FULL, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_FORWARDS, 0.0F, 0.0F, LR_LEFT));
}

static void testBackwardsOpposesTheFacing(void)
{
    CHECK_EQ(-FTREMIX_AI_STICK_FULL, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_BACKWARDS, 0.0F, 0.0F, LR_RIGHT));
    CHECK_EQ(FTREMIX_AI_STICK_FULL, ftRemixAiRulesStickXFromCode(FTREMIX_AI_STICK_BACKWARDS, 0.0F, 0.0F, LR_LEFT));
}

static void testVanillaStickCodesAreNotExtended(void)
{
    CHECK_EQ(0, ftRemixAiRulesStickXFromCode(0x7F, 10.0F, 0.0F, LR_RIGHT));
    CHECK_EQ(0, ftRemixAiRulesStickXFromCode(0x80, 10.0F, 0.0F, LR_RIGHT));
}

static void testTurnAndTurnRunAreTheWaitActions(void)
{
    CHECK_EQ(1, ftRemixAiRulesIsTurnAction(FTREMIX_AI_ACTION_TURN));
    CHECK_EQ(1, ftRemixAiRulesIsTurnAction(FTREMIX_AI_ACTION_TURN_RUN));
    CHECK_EQ(0, ftRemixAiRulesIsTurnAction(FTREMIX_AI_ACTION_JUMP_SQUAT));
}

void remixTestAiCommandsRules(void)
{
    printf("ai commands rules\n");
    RUN_TEST(testAutofullAwayPointsAwayFromTheTarget);
    RUN_TEST(testAutohalfAwayUsesHalfMagnitude);
    RUN_TEST(testForwardsFollowsTheFacing);
    RUN_TEST(testBackwardsOpposesTheFacing);
    RUN_TEST(testVanillaStickCodesAreNotExtended);
    RUN_TEST(testTurnAndTurnRunAreTheWaitActions);
}
