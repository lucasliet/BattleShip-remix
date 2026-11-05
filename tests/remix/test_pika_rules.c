/**
 * test_pika_rules.c - unit tests of the Pikachu clone patch rules (ft/ftremix/ftremixpikarules.c): the file selection of
 * pikashared.asm and the J Pikachu Thunder Jolt duration of JPika.asm.
 */

#include "test_check.h"

#include "ft/ftremix/ftremixpikarules.h"
#include "remix/remix_roster_data.h"

#define VANILLA_JOLT_LIFETIME 100

static void testEPikaAndJPikaThunderReadTheirOwnFile(void)
{
    CHECK_TRUE(ftRemixPikaRulesThunderUsesOwnFile(REMIX_ID_EPIKA));
    CHECK_TRUE(ftRemixPikaRulesThunderUsesOwnFile(REMIX_ID_JPIKA));
    CHECK_TRUE(!ftRemixPikaRulesThunderUsesOwnFile(REMIX_ID_PIKACHU));
    CHECK_TRUE(!ftRemixPikaRulesThunderUsesOwnFile(REMIX_ID_NPIKACHU));
}

static void testPikachuClonesThunderJoltReadTheirOwnFile(void)
{
    CHECK_TRUE(ftRemixPikaRulesThunderJoltUsesOwnFile(REMIX_ID_JPIKA));
    CHECK_TRUE(ftRemixPikaRulesThunderJoltUsesOwnFile(REMIX_ID_EPIKA));
    CHECK_TRUE(!ftRemixPikaRulesThunderJoltUsesOwnFile(REMIX_ID_PIKACHU));
    CHECK_TRUE(!ftRemixPikaRulesThunderJoltUsesOwnFile(REMIX_ID_NPIKACHU));
}

static void testJPikaThunderJoltLastsOneHundredAndTwentyFrames(void)
{
    CHECK_EQ(120, ftRemixPikaRulesThunderJoltLifetime(REMIX_ID_JPIKA, VANILLA_JOLT_LIFETIME));
}

static void testOtherPikachuThunderJoltKeepsTheVanillaLifetime(void)
{
    CHECK_EQ(VANILLA_JOLT_LIFETIME, ftRemixPikaRulesThunderJoltLifetime(REMIX_ID_PIKACHU, VANILLA_JOLT_LIFETIME));
    CHECK_EQ(VANILLA_JOLT_LIFETIME, ftRemixPikaRulesThunderJoltLifetime(REMIX_ID_EPIKA, VANILLA_JOLT_LIFETIME));
}

void remixTestPikaRules(void)
{
    printf("pika rules\n");
    RUN_TEST(testEPikaAndJPikaThunderReadTheirOwnFile);
    RUN_TEST(testPikachuClonesThunderJoltReadTheirOwnFile);
    RUN_TEST(testJPikaThunderJoltLastsOneHundredAndTwentyFrames);
    RUN_TEST(testOtherPikachuThunderJoltKeepsTheVanillaLifetime);
}
