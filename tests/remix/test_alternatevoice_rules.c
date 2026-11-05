/**
 * test_alternatevoice_rules.c - unit tests of Wario's alternate neutral special voice rule
 * (ft/ftremix/ftremixalternatevoicerules.c, Wario.asm alternate_voice_).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixalternatevoicerules.h"

#define BUTTON_A 0x8000

static void testWarioVoiceWithLHeldSwapsOnce(void)
{
    CHECK_EQ(1, ftRemixAlternateVoiceShouldSwap(FTREMIX_ALTERNATE_VOICE_WARIO_NSP, FTREMIX_ALTERNATE_VOICE_BUTTON_L, 0));
    CHECK_EQ(1, ftRemixAlternateVoiceShouldSwap(FTREMIX_ALTERNATE_VOICE_WARIO_NSP, FTREMIX_ALTERNATE_VOICE_BUTTON_L | BUTTON_A, 0));
}

static void testWarioVoiceKeepsTheOriginalOnceTheFlagIsSet(void)
{
    CHECK_EQ(0, ftRemixAlternateVoiceShouldSwap(FTREMIX_ALTERNATE_VOICE_WARIO_NSP, FTREMIX_ALTERNATE_VOICE_BUTTON_L, 1));
}

static void testWarioVoiceWithoutLIsUnchanged(void)
{
    CHECK_EQ(0, ftRemixAlternateVoiceShouldSwap(FTREMIX_ALTERNATE_VOICE_WARIO_NSP, 0, 0));
    CHECK_EQ(0, ftRemixAlternateVoiceShouldSwap(FTREMIX_ALTERNATE_VOICE_WARIO_NSP, BUTTON_A, 0));
}

static void testOtherVoicesAreUnchanged(void)
{
    CHECK_EQ(0, ftRemixAlternateVoiceShouldSwap(0x2FC, FTREMIX_ALTERNATE_VOICE_BUTTON_L, 0));
    CHECK_EQ(0, ftRemixAlternateVoiceShouldSwap(FTREMIX_ALTERNATE_VOICE_WARIO_NSP_ALTERNATE, FTREMIX_ALTERNATE_VOICE_BUTTON_L, 0));
}

void remixTestAlternateVoiceRules(void)
{
    printf("alternate voice rules\n");
    RUN_TEST(testWarioVoiceWithLHeldSwapsOnce);
    RUN_TEST(testWarioVoiceKeepsTheOriginalOnceTheFlagIsSet);
    RUN_TEST(testWarioVoiceWithoutLIsUnchanged);
    RUN_TEST(testOtherVoicesAreUnchanged);
}
