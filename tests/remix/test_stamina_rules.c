/**
 * test_stamina_rules.c - unit tests of the Stamina rules (gm/gmremixstaminarules.c).
 */

#include "test_check.h"

#include "gm/gmremixstaminarules.h"

#define STAMINA_DEFAULT_HP 200
#define STAMINA_DEFAULT_TOTAL_HP 199

static void testTotalHPIsTheHPMinusOne(void)
{
    CHECK_EQ(STAMINA_DEFAULT_TOTAL_HP, gmRemixStaminaRulesTotalHP(STAMINA_DEFAULT_HP));
    CHECK_EQ(0, gmRemixStaminaRulesTotalHP(1));
    CHECK_EQ(998, gmRemixStaminaRulesTotalHP(999));
}

static void testPercentEqualToTotalHPIsNotOver(void)
{
    CHECK_EQ(0, gmRemixStaminaRulesIsOverTotal(STAMINA_DEFAULT_TOTAL_HP, STAMINA_DEFAULT_TOTAL_HP));
}

static void testPercentAboveTotalHPIsOver(void)
{
    CHECK_EQ(1, gmRemixStaminaRulesIsOverTotal(STAMINA_DEFAULT_TOTAL_HP + 1, STAMINA_DEFAULT_TOTAL_HP));
    CHECK_EQ(1, gmRemixStaminaRulesIsOverTotal(STAMINA_DEFAULT_HP, STAMINA_DEFAULT_TOTAL_HP));
}

static void testPercentBelowTotalHPIsNotOver(void)
{
    CHECK_EQ(0, gmRemixStaminaRulesIsOverTotal(0, STAMINA_DEFAULT_TOTAL_HP));
    CHECK_EQ(0, gmRemixStaminaRulesIsOverTotal(150, STAMINA_DEFAULT_TOTAL_HP));
}

static void testDamageToFinishStopsThePercentAtTotalHPPlusOne(void)
{
    CHECK_EQ(50, gmRemixStaminaRulesDamageToFinish(STAMINA_DEFAULT_TOTAL_HP, 150));
    CHECK_EQ(1, gmRemixStaminaRulesDamageToFinish(STAMINA_DEFAULT_TOTAL_HP, STAMINA_DEFAULT_TOTAL_HP));
    CHECK_EQ(200, gmRemixStaminaRulesDamageToFinish(STAMINA_DEFAULT_TOTAL_HP, 0));
}

static void testHPLeftCountsDownFromTotalHPPlusOne(void)
{
    CHECK_EQ(200, gmRemixStaminaRulesHPLeft(STAMINA_DEFAULT_TOTAL_HP, 0));
    CHECK_EQ(150, gmRemixStaminaRulesHPLeft(STAMINA_DEFAULT_TOTAL_HP, 50));
}

static void testHPLeftIsNeverBelowZero(void)
{
    CHECK_EQ(0, gmRemixStaminaRulesHPLeft(STAMINA_DEFAULT_TOTAL_HP, 200));
    CHECK_EQ(0, gmRemixStaminaRulesHPLeft(STAMINA_DEFAULT_TOTAL_HP, 250));
}

void remixTestStaminaRules(void)
{
    printf("stamina rules\n");
    RUN_TEST(testTotalHPIsTheHPMinusOne);
    RUN_TEST(testPercentEqualToTotalHPIsNotOver);
    RUN_TEST(testPercentAboveTotalHPIsOver);
    RUN_TEST(testPercentBelowTotalHPIsNotOver);
    RUN_TEST(testDamageToFinishStopsThePercentAtTotalHPPlusOne);
    RUN_TEST(testHPLeftCountsDownFromTotalHPPlusOne);
    RUN_TEST(testHPLeftIsNeverBelowZero);
}
