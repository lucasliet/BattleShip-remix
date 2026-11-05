/**
 * test_capturecargo_rules.c - unit tests of the Remix cargo escape base (ft/ftremix/ftremixcapturecargorules.c): J DK, Marina
 * and N Marina need more inputs to escape from their shoulder hold (JDK.asm jdk_cargo_).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixcapturecargorules.h"
#include "remix/remix_roster_data.h"

static void testJDKAndMarinaUseTheHeavyBase(void)
{
    CHECK_TRUE(ftRemixCaptureCargoEscapeBase(REMIX_ID_JDK) == FTREMIX_CARGO_ESCAPE_BASE_HEAVY);
    CHECK_TRUE(ftRemixCaptureCargoEscapeBase(REMIX_ID_MARINA) == FTREMIX_CARGO_ESCAPE_BASE_HEAVY);
    CHECK_TRUE(ftRemixCaptureCargoEscapeBase(REMIX_ID_NMARINA) == FTREMIX_CARGO_ESCAPE_BASE_HEAVY);
}

static void testOtherFightersKeepTheUsBase(void)
{
    CHECK_TRUE(ftRemixCaptureCargoEscapeBase(REMIX_ID_DONKEY) == FTREMIX_CARGO_ESCAPE_BASE_DEFAULT);
    CHECK_TRUE(ftRemixCaptureCargoEscapeBase(REMIX_ID_LANKY) == FTREMIX_CARGO_ESCAPE_BASE_DEFAULT);
    CHECK_TRUE(ftRemixCaptureCargoEscapeBase(REMIX_ID_GDONKEY) == FTREMIX_CARGO_ESCAPE_BASE_DEFAULT);
}

static void testBasesMatchTheAsmFloats(void)
{
    CHECK_TRUE(FTREMIX_CARGO_ESCAPE_BASE_DEFAULT == 14.0F);
    CHECK_TRUE(FTREMIX_CARGO_ESCAPE_BASE_HEAVY == 20.0F);
}

void remixTestCaptureCargoRules(void)
{
    printf("capture cargo rules\n");
    RUN_TEST(testJDKAndMarinaUseTheHeavyBase);
    RUN_TEST(testOtherFightersKeepTheUsBase);
    RUN_TEST(testBasesMatchTheAsmFloats);
}
