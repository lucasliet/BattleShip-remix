/**
 * test_envcolor_rules.c - unit tests of the env colour opacity rule (ft/ftremix/ftremixenvcolorrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixenvcolorrules.h"

#define ENVCOLOR_NONE 0xFFFFFF01u
#define ENVCOLOR_DARK 0x000000FFu
#define ENVCOLOR_CLOAKED 0xFFFFFF10u

static void testNoOverrideNeverForcesTheAlphaRenderMode(void)
{
    CHECK_EQ(0, ftRemixEnvColorRulesForcesTranslucent(0));
}

static void testOpaqueAlphaKeepsTheOwnRenderModes(void)
{
    CHECK_EQ(0, ftRemixEnvColorRulesForcesTranslucent(ENVCOLOR_DARK));
    CHECK_EQ(0, ftRemixEnvColorRulesForcesTranslucent(0x112233FFu));
}

static void testPanelOptionsNoneAndCloakedForceTheAlphaRenderMode(void)
{
    CHECK_EQ(1, ftRemixEnvColorRulesForcesTranslucent(ENVCOLOR_NONE));
    CHECK_EQ(1, ftRemixEnvColorRulesForcesTranslucent(ENVCOLOR_CLOAKED));
}

static void testAnyAlphaBelowOpaqueForcesTheAlphaRenderMode(void)
{
    CHECK_EQ(1, ftRemixEnvColorRulesForcesTranslucent(0x000000FEu));
    CHECK_EQ(1, ftRemixEnvColorRulesForcesTranslucent(0xFFFFFF00u));
}

static void testForcedRenderModeIsTheAsmAlphaWord(void)
{
    CHECK_EQ(0xC41049D8u, FTREMIX_ENVCOLOR_FORCED_RENDER_MODE);
}

void remixTestEnvColorRules(void)
{
    printf("env color rules\n");
    RUN_TEST(testNoOverrideNeverForcesTheAlphaRenderMode);
    RUN_TEST(testOpaqueAlphaKeepsTheOwnRenderModes);
    RUN_TEST(testPanelOptionsNoneAndCloakedForceTheAlphaRenderMode);
    RUN_TEST(testAnyAlphaBelowOpaqueForcesTheAlphaRenderMode);
    RUN_TEST(testForcedRenderModeIsTheAsmAlphaWord);
}
