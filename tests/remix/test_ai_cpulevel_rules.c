/**
 * test_ai_cpulevel_rules.c - unit tests of the VS CSS CPU level rules (mn/mnplayers/mnremixcpulevelrules.c).
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixcpulevelrules.h"

static void testTheRightArrowRaisesLevelsBelowTen(void)
{
    CHECK_EQ(1, mnRemixCpuLevelCanRaise(9));
    CHECK_EQ(1, mnRemixCpuLevelCanRaise(1));
}

static void testTheRightArrowStopsAtLevelTen(void)
{
    CHECK_EQ(0, mnRemixCpuLevelCanRaise(10));
}

static void testOnlyLevelTenNeedsTensDigit(void)
{
    CHECK_EQ(0, mnRemixCpuLevelNeedsTens(9));
    CHECK_EQ(1, mnRemixCpuLevelNeedsTens(10));
}

static void testUnitsDigitOfLevelTenIsZero(void)
{
    CHECK_EQ(0, mnRemixCpuLevelUnitsDigit(10));
}

static void testUnitsDigitOfOneDigitLevelsIsTheLevel(void)
{
    CHECK_EQ(1, mnRemixCpuLevelUnitsDigit(1));
    CHECK_EQ(9, mnRemixCpuLevelUnitsDigit(9));
}

void remixTestAiCpuLevelRules(void)
{
    testTheRightArrowRaisesLevelsBelowTen();
    testTheRightArrowStopsAtLevelTen();
    testOnlyLevelTenNeedsTensDigit();
    testUnitsDigitOfLevelTenIsZero();
    testUnitsDigitOfOneDigitLevelsIsTheLevel();
}
