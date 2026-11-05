/**
 * test_ai_specials_rules.c - unit tests of the Remix AI charged NSP rules
 * (ft/ftremix/ftremixaispecialsrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaispecialsrules.h"

static void testDSamusChargeShotCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsDSamusCancels(0x0DF));
}

static void testDSamusKirbyAirChargeCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsDSamusCancels(0x1AD));
}

static void testDSamusKirbyDSChargingCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsDSamusCancels(0x133));
}

static void testDSamusOtherActionDoesNotCancel(void)
{
    CHECK_EQ(0, ftRemixAiSpecialsDSamusCancels(0x0E0));
}

static void testSheikNeedleChargeGroundCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsSheikCancels(0x0EB));
}

static void testSheikNeedleChargeAirCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsSheikCancels(0x0EE));
}

static void testSheikKirbyNeedleChargeCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsSheikCancels(0x167));
    CHECK_TRUE(ftRemixAiSpecialsSheikCancels(0x16A));
}

static void testSheikShootDoesNotCancel(void)
{
    CHECK_EQ(0, ftRemixAiSpecialsSheikCancels(0x0EC));
}

static void testMewtwoShadowBallChargeCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsMewtwoCancels(0x0DF));
}

static void testMewtwoShadowBallChargeAirCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsMewtwoCancels(0x0E2));
}

static void testMewtwoKirbyShadowBallChargeCancels(void)
{
    CHECK_TRUE(ftRemixAiSpecialsMewtwoCancels(0x14B));
    CHECK_TRUE(ftRemixAiSpecialsMewtwoCancels(0x14E));
}

static void testMewtwoShootDoesNotCancel(void)
{
    CHECK_EQ(0, ftRemixAiSpecialsMewtwoCancels(0x0E0));
}

static void testFoxLikeOpponentAlwaysBlocksSamusNsp(void)
{
    CHECK_TRUE(ftRemixAiSamusNspOpponentBlocks(1, 0, 0));
    CHECK_TRUE(ftRemixAiSamusNspOpponentBlocks(1, 0, 1));
}

static void testNessLikeOpponentBlocksSamusNspUnlessSheik(void)
{
    CHECK_TRUE(ftRemixAiSamusNspOpponentBlocks(0, 1, 0));
    CHECK_EQ(0, ftRemixAiSamusNspOpponentBlocks(0, 1, 1));
}

static void testOtherOpponentDoesNotBlockSamusNsp(void)
{
    CHECK_EQ(0, ftRemixAiSamusNspOpponentBlocks(0, 0, 0));
}

static void testSheikBeginAndGroundChargeKeepCharging(void)
{
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0EA, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0ED, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0EB, 0));
}

static void testSheikShootAirChargeAndJumpSquatStop(void)
{
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0EC, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0EF, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0EE, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x014, 0));
}

static void testSheikPressesBelowFullAmmo(void)
{
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_PRESS, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x00A, 5));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x00A, 6));
}

static void testSheikFallsThroughToPeppyActionChecks(void)
{
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0E1, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(1, 0x0F3, 0));
}

static void testPeppyUsesItsOwnActionIds(void)
{
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(0, 0x0E1, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(0, 0x0EE, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_KEEP, ftRemixAiSpecialsNeedleChargeOutcome(0, 0x0E2, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_STOP, ftRemixAiSpecialsNeedleChargeOutcome(0, 0x0F2, 0));
    CHECK_EQ(FTREMIX_AI_SPECIALS_NEEDLE_PRESS, ftRemixAiSpecialsNeedleChargeOutcome(0, 0x0EB, 2));
}

static void testChargeShotActiveForSheikMewtwoAndDSamus(void)
{
    CHECK_TRUE(ftRemixAiSpecialsChargeShotActive(0x3E, 0x0EA));
    CHECK_TRUE(ftRemixAiSpecialsChargeShotActive(0x3E, 0x16B));
    CHECK_EQ(0, ftRemixAiSpecialsChargeShotActive(0x3E, 0x0E9));
    CHECK_TRUE(ftRemixAiSpecialsChargeShotActive(0x39, 0x0E0));
    CHECK_TRUE(ftRemixAiSpecialsChargeShotActive(0x39, 0x14D));
    CHECK_EQ(0, ftRemixAiSpecialsChargeShotActive(0x39, 0x0E1));
    CHECK_TRUE(ftRemixAiSpecialsChargeShotActive(0x22, 0x133));
    CHECK_EQ(0, ftRemixAiSpecialsChargeShotActive(0x22, 0x0DF));
    CHECK_EQ(0, ftRemixAiSpecialsChargeShotActive(0x03, 0x133));
}

static void testSamusActionListCoversTheSamusLikeFighters(void)
{
    CHECK_TRUE(ftRemixAiSpecialsUsesSamusActionList(0x03));
    CHECK_TRUE(ftRemixAiSpecialsUsesSamusActionList(0x24));
    CHECK_TRUE(ftRemixAiSpecialsUsesSamusActionList(0x33));
    CHECK_TRUE(ftRemixAiSpecialsUsesSamusActionList(0x22));
    CHECK_TRUE(ftRemixAiSpecialsUsesSamusActionList(0x39));
    CHECK_EQ(0, ftRemixAiSpecialsUsesSamusActionList(0x3E));
}

void remixTestAiSpecialsRules(void)
{
    printf("ai specials rules\n");
    RUN_TEST(testSheikBeginAndGroundChargeKeepCharging);
    RUN_TEST(testSheikShootAirChargeAndJumpSquatStop);
    RUN_TEST(testSheikPressesBelowFullAmmo);
    RUN_TEST(testSheikFallsThroughToPeppyActionChecks);
    RUN_TEST(testPeppyUsesItsOwnActionIds);
    RUN_TEST(testChargeShotActiveForSheikMewtwoAndDSamus);
    RUN_TEST(testSamusActionListCoversTheSamusLikeFighters);
    RUN_TEST(testDSamusChargeShotCancels);
    RUN_TEST(testDSamusKirbyAirChargeCancels);
    RUN_TEST(testDSamusKirbyDSChargingCancels);
    RUN_TEST(testDSamusOtherActionDoesNotCancel);
    RUN_TEST(testSheikNeedleChargeGroundCancels);
    RUN_TEST(testSheikNeedleChargeAirCancels);
    RUN_TEST(testSheikKirbyNeedleChargeCancels);
    RUN_TEST(testSheikShootDoesNotCancel);
    RUN_TEST(testMewtwoShadowBallChargeCancels);
    RUN_TEST(testMewtwoShadowBallChargeAirCancels);
    RUN_TEST(testMewtwoKirbyShadowBallChargeCancels);
    RUN_TEST(testMewtwoShootDoesNotCancel);
    RUN_TEST(testFoxLikeOpponentAlwaysBlocksSamusNsp);
    RUN_TEST(testNessLikeOpponentBlocksSamusNspUnlessSheik);
    RUN_TEST(testOtherOpponentDoesNotBlockSamusNsp);
}
