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

void remixTestAiSpecialsRules(void)
{
    printf("ai specials rules\n");
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
