/**
 * test_blueshell_rules.c - unit tests of the Blue Shell first-place rule (it/itremix/itremixblueshellrules.c).
 */

#include "test_check.h"

#include "it/itremix/itremixblueshellrules.h"

static void testOnlyTheStaminaRulesByteMeansLowerWins(void)
{
    CHECK_EQ(1, itRemixBlueShellRulesIsStamina(ITREMIX_BLUESHELL_RULES_GAME_RULES_STAMINA));
    CHECK_EQ(0, itRemixBlueShellRulesIsStamina(1));
    CHECK_EQ(0, itRemixBlueShellRulesIsStamina(0));
}

static void testHighestValueWinsOutsideStamina(void)
{
    const int32_t values[4] = { 2, 5, 3, 1 };
    const int32_t percents[4] = { 0, 0, 0, 0 };
    const int32_t present[4] = { 1, 1, 1, 1 };

    CHECK_EQ(1, itRemixBlueShellRulesFirstPort(0, values, percents, present));
}

static void testLowestValueWinsInStamina(void)
{
    const int32_t values[4] = { 2, 5, 3, 1 };
    const int32_t percents[4] = { 0, 0, 0, 0 };
    const int32_t present[4] = { 1, 1, 1, 1 };

    CHECK_EQ(3, itRemixBlueShellRulesFirstPort(1, values, percents, present));
}

static void testPortWithoutFighterIsSkippedButPortZeroAlwaysStarts(void)
{
    const int32_t values[4] = { 0, 9, 1, 1 };
    const int32_t percents[4] = { 0, 0, 0, 0 };
    const int32_t present[4] = { 0, 0, 1, 0 };

    CHECK_EQ(2, itRemixBlueShellRulesFirstPort(0, values, percents, present));
    {
        const int32_t none[4] = { 0, 0, 0, 0 };

        CHECK_EQ(0, itRemixBlueShellRulesFirstPort(0, values, percents, none));
    }
}

static void testTieOfValuesGoesToTheLowerPercent(void)
{
    const int32_t values[4] = { 3, 3, 3, 0 };
    const int32_t percents[4] = { 50, 80, 20, 0 };
    const int32_t present[4] = { 1, 1, 1, 1 };

    CHECK_EQ(2, itRemixBlueShellRulesFirstPort(0, values, percents, present));
}

void remixTestBlueShellRules(void)
{
    printf("blue shell rules\n");
    RUN_TEST(testOnlyTheStaminaRulesByteMeansLowerWins);
    RUN_TEST(testHighestValueWinsOutsideStamina);
    RUN_TEST(testLowestValueWinsInStamina);
    RUN_TEST(testPortWithoutFighterIsSkippedButPortZeroAlwaysStarts);
    RUN_TEST(testTieOfValuesGoesToTheLowerPercent);
}
