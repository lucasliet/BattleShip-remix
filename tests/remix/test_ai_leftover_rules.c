/**
 * test_ai_leftover_rules.c - unit tests of the AI leftover rules of T8.2: the Sheik post process, the Bowser grounded up
 * special and the Banjo recovery flap (ftremixaipostsheikrules.c, ftremixaispecialsrules.c, ftremixairecoveryrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaipostsheikrules.h"
#include "ft/ftremix/ftremixaispecialsrules.h"
#include "ft/ftremix/ftremixairecoveryrules.h"

static void testSheikTeleportActionsAreTheFourUspActions(void)
{
    CHECK_EQ(1, ftRemixAiSheikIsTeleportAction(0xE4));
    CHECK_EQ(1, ftRemixAiSheikIsTeleportAction(0xE5));
    CHECK_EQ(1, ftRemixAiSheikIsTeleportAction(0xE7));
    CHECK_EQ(1, ftRemixAiSheikIsTeleportAction(0xE8));
}

static void testSheikTeleportIgnoresOtherActions(void)
{
    CHECK_EQ(0, ftRemixAiSheikIsTeleportAction(0xE6));
    CHECK_EQ(0, ftRemixAiSheikIsTeleportAction(0xE9));
    CHECK_EQ(0, ftRemixAiSheikIsTeleportAction(0xF0));
}

static void testSheikDspControlRunsOnlyOnDspBegin(void)
{
    CHECK_EQ(1, ftRemixAiSheikIsDspAction(0xF0));
    CHECK_EQ(0, ftRemixAiSheikIsDspAction(0xEF));
    CHECK_EQ(0, ftRemixAiSheikIsDspAction(0xF1));
}

static void testSheikTeleportGoesDownOnlyWithGroundBelow(void)
{
    CHECK_EQ(1, ftRemixAiSheikTeleportHasGroundBelow(0));
    CHECK_EQ(1, ftRemixAiSheikTeleportHasGroundBelow(12));
    CHECK_EQ(0, ftRemixAiSheikTeleportHasGroundBelow(-1));
}

static void testSheikDspPressesBInsideSixHundredUnitsInclusive(void)
{
    CHECK_EQ(1, ftRemixAiSheikDspIsInRange(0.0F));
    CHECK_EQ(1, ftRemixAiSheikDspIsInRange(600.0F));
    CHECK_EQ(0, ftRemixAiSheikDspIsInRange(600.5F));
}

static void testBowserUspGroundIsOnlyAction0xDE(void)
{
    CHECK_EQ(1, ftRemixAiSpecialsIsBowserUspGround(0xDE));
    CHECK_EQ(0, ftRemixAiSpecialsIsBowserUspGround(0xDD));
    CHECK_EQ(0, ftRemixAiSpecialsIsBowserUspGround(0xDF));
}

static void testBanjoFlapFrameIsFlightAtFrame21(void)
{
    CHECK_EQ(1, ftRemixAiRecoveryIsBanjoFlapFrame(0xEF, 21));
    CHECK_EQ(0, ftRemixAiRecoveryIsBanjoFlapFrame(0xEF, 22));
    CHECK_EQ(0, ftRemixAiRecoveryIsBanjoFlapFrame(0xEF, 20));
    CHECK_EQ(0, ftRemixAiRecoveryIsBanjoFlapFrame(0xEE, 21));
}

static void testBanjoPressesBOnlyWithoutGroundBelow(void)
{
    CHECK_EQ(1, ftRemixAiRecoveryBanjoNeedsPressB(-1));
    CHECK_EQ(0, ftRemixAiRecoveryBanjoNeedsPressB(0));
    CHECK_EQ(0, ftRemixAiRecoveryBanjoNeedsPressB(7));
}

void remixTestAiLeftoverRules(void)
{
    printf("ai leftover rules\n");
    RUN_TEST(testSheikTeleportActionsAreTheFourUspActions);
    RUN_TEST(testSheikTeleportIgnoresOtherActions);
    RUN_TEST(testSheikDspControlRunsOnlyOnDspBegin);
    RUN_TEST(testSheikTeleportGoesDownOnlyWithGroundBelow);
    RUN_TEST(testSheikDspPressesBInsideSixHundredUnitsInclusive);
    RUN_TEST(testBowserUspGroundIsOnlyAction0xDE);
    RUN_TEST(testBanjoFlapFrameIsFlightAtFrame21);
    RUN_TEST(testBanjoPressesBOnlyWithoutGroundBelow);
}
