/**
 * test_bowserammo_rules.c - unit tests of the fire breath ammo rules (ft/ftremix/bowser/ftremixbowserammorules.c): the spawn
 * and KO refill, and the recharge of bowser_nsp_recharge (a timer in the upper halfword, the ammo in the lower one).
 */

#include "test_check.h"

#include "ft/ftremix/bowser/ftremixbowserammorules.h"

#define WORD(timer, ammo) ((((unsigned int)(timer)) << 16) | (unsigned int)(ammo))

static void testSpawnGivesFullAmmoAndAFreshTimer(void)
{
    CHECK_EQ(WORD(0, 0x14), ftRemixBowserAmmoSpawnWord());
}

static void testRefillSetsTheAmmoAndKeepsTheTimer(void)
{
    CHECK_EQ(WORD(7, 0x14), ftRemixBowserAmmoRefillWord(WORD(7, 0)));
    CHECK_EQ(WORD(7, 0x14), ftRemixBowserAmmoRefillWord(WORD(7, 0x14)));
}

static void testRechargeOnlyCountsTheTimerBeforeTheLastFrame(void)
{
    CHECK_EQ(WORD(1, 10), ftRemixBowserAmmoRechargeWord(WORD(0, 10), 0));
    CHECK_EQ(WORD(0x1D, 10), ftRemixBowserAmmoRechargeWord(WORD(0x1C, 10), 0));
}

static void testRechargeAddsTwoAmmoAndRestartsTheTimerAt30Frames(void)
{
    CHECK_EQ(WORD(0, 12), ftRemixBowserAmmoRechargeWord(WORD(0x1D, 10), 0));
}

static void testRechargeRestartsTheTimerWithoutAmmoWhileFiring(void)
{
    CHECK_EQ(WORD(0, 10), ftRemixBowserAmmoRechargeWord(WORD(0x1D, 10), 1));
}

static void testRechargeStopsAtExactlyTheMaximum(void)
{
    CHECK_EQ(WORD(0, 0x14), ftRemixBowserAmmoRechargeWord(WORD(0x1D, 0x14), 0));
}

static void testRechargePastTheMaximumKeepsGrowingByTheEqualityTest(void)
{
    CHECK_EQ(WORD(0, 0x15), ftRemixBowserAmmoRechargeWord(WORD(0x1D, 0x13), 0));
    CHECK_EQ(WORD(0, 0x17), ftRemixBowserAmmoRechargeWord(WORD(0x1D, 0x15), 0));
}

static void testEmptyAmmoFillsInTenRechargeTicks(void)
{
    unsigned int word = 0;
    int frame;

    for (frame = 0; frame < 10 * FTREMIX_BOWSER_AMMO_RECHARGE_FRAMES; frame++)
    {
        word = ftRemixBowserAmmoRechargeWord(word, 0);
    }
    CHECK_EQ(WORD(0, 0x14), word);
}

static void testTimerWrapsInsideItsHalfword(void)
{
    CHECK_EQ(WORD(0, 5), ftRemixBowserAmmoRechargeWord(WORD(0xFFFF, 5), 0));
}

void remixTestBowserAmmoRules(void)
{
    printf("bowser ammo rules\n");
    RUN_TEST(testSpawnGivesFullAmmoAndAFreshTimer);
    RUN_TEST(testRefillSetsTheAmmoAndKeepsTheTimer);
    RUN_TEST(testRechargeOnlyCountsTheTimerBeforeTheLastFrame);
    RUN_TEST(testRechargeAddsTwoAmmoAndRestartsTheTimerAt30Frames);
    RUN_TEST(testRechargeRestartsTheTimerWithoutAmmoWhileFiring);
    RUN_TEST(testRechargeStopsAtExactlyTheMaximum);
    RUN_TEST(testRechargePastTheMaximumKeepsGrowingByTheEqualityTest);
    RUN_TEST(testEmptyAmmoFillsInTenRechargeTicks);
    RUN_TEST(testTimerWrapsInsideItsHalfword);
}
