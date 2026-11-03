/**
 * test_twelvecb_layout.c - unit tests of the 12-Character Battle CSS layout (mn/mnplayers/mnremixtwelvecblayout.c).
 */

#include "test_check.h"

#include "mn/mnplayers/mnremixtwelvecblayout.h"

static void testEverySetHasTwelveDistinctFightersShownTwice(void)
{
    int set;

    for (set = 0; set < MNREMIX_TCB_SETS; set++)
    {
        int seen[256] = { 0 };
        int slot;

        for (slot = 0; slot < MNREMIX_TCB_SLOTS; slot++)
        {
            int fkind = mnRemixTwelveCBLayoutFkind(set, slot);

            CHECK_TRUE((fkind >= 0) && (fkind < 256));
            seen[fkind]++;
        }
        {
            int distinct = 0;
            int fkind;

            for (fkind = 0; fkind < 256; fkind++)
            {
                if (seen[fkind] != 0)
                {
                    distinct++;
                    CHECK_EQ(2, seen[fkind]);
                }
            }
            CHECK_EQ(12, distinct);
        }
    }
}

static void testDefaultSetMatchesTheAsm(void)
{
    CHECK_EQ(0x04, mnRemixTwelveCBLayoutFkind(0, 0));
    CHECK_EQ(0x00, mnRemixTwelveCBLayoutFkind(0, 1));
    CHECK_EQ(0x05, mnRemixTwelveCBLayoutFkind(0, 3));
    CHECK_EQ(0x04, mnRemixTwelveCBLayoutFkind(0, 4));
    CHECK_EQ(0x03, mnRemixTwelveCBLayoutFkind(0, 8));
    CHECK_EQ(0x0A, mnRemixTwelveCBLayoutFkind(0, 23));
    CHECK_EQ(0x3A, mnRemixTwelveCBLayoutFkind(3, 19));
}

static void testInvalidSetOrSlotHasNoFighter(void)
{
    CHECK_EQ(-1, mnRemixTwelveCBLayoutFkind(-1, 0));
    CHECK_EQ(-1, mnRemixTwelveCBLayoutFkind(MNREMIX_TCB_SETS, 0));
    CHECK_EQ(-1, mnRemixTwelveCBLayoutFkind(0, MNREMIX_TCB_SLOTS));
}

static void testCellPositionsMatchTheAsm(void)
{
    CHECK_EQ(32, (int)mnRemixTwelveCBLayoutCellX(0));
    CHECK_EQ(122, (int)mnRemixTwelveCBLayoutCellX(3));
    CHECK_EQ(168, (int)mnRemixTwelveCBLayoutCellX(4));
    CHECK_EQ(258, (int)mnRemixTwelveCBLayoutCellX(7));
    CHECK_EQ(35, (int)mnRemixTwelveCBLayoutCellY(0));
    CHECK_EQ(65, (int)mnRemixTwelveCBLayoutCellY(8));
    CHECK_EQ(95, (int)mnRemixTwelveCBLayoutCellY(23));
}

static void testSlideStartsOutsideAndNeverPassesTheTarget(void)
{
    float x = mnRemixTwelveCBLayoutStartX(0);
    int steps = 0;

    CHECK_EQ(-20, (int)x);
    while (x != mnRemixTwelveCBLayoutCellX(0))
    {
        x = mnRemixTwelveCBLayoutNextX(0, x);
        CHECK_TRUE(x <= mnRemixTwelveCBLayoutCellX(0));
        CHECK_TRUE(++steps < 100);
    }
    x = mnRemixTwelveCBLayoutStartX(7);
    CHECK_EQ(310, (int)x);
    steps = 0;
    while (x != mnRemixTwelveCBLayoutCellX(7))
    {
        x = mnRemixTwelveCBLayoutNextX(7, x);
        CHECK_TRUE(x >= mnRemixTwelveCBLayoutCellX(7));
        CHECK_TRUE(++steps < 400);
    }
    CHECK_EQ(258, (int)mnRemixTwelveCBLayoutNextX(7, 258.0F));
}

static void testHitTestKeepsEachPlayerInHisHalf(void)
{
    CHECK_EQ(0, mnRemixTwelveCBLayoutSlotAtPoint(0, 19, 25));
    CHECK_EQ(3, mnRemixTwelveCBLayoutSlotAtPoint(0, 138, 54));
    CHECK_EQ(8, mnRemixTwelveCBLayoutSlotAtPoint(0, 30, 60));
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotAtPoint(0, 140, 40));
    CHECK_EQ(4, mnRemixTwelveCBLayoutSlotAtPoint(1, 155, 25));
    CHECK_EQ(23, mnRemixTwelveCBLayoutSlotAtPoint(1, 274, 109));
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotAtPoint(1, 150, 40));
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotAtPoint(1, 280, 40));
}

static void testHitTestRejectsRowsBelowTheGrid(void)
{
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotAtPoint(0, 30, 110));
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotAtPoint(0, 30, 24));
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotAtPoint(2, 30, 40));
}

static void testSlotOfFkindFindsThePlayersOwnPortrait(void)
{
    CHECK_EQ(1, mnRemixTwelveCBLayoutSlotOfFkind(0, 0, 0x00));
    CHECK_EQ(5, mnRemixTwelveCBLayoutSlotOfFkind(0, 1, 0x00));
    CHECK_EQ(MNREMIX_TCB_NO_SLOT, mnRemixTwelveCBLayoutSlotOfFkind(0, 0, 0x2A));
    CHECK_EQ(0, mnRemixTwelveCBLayoutOwner(3) + mnRemixTwelveCBLayoutOwner(0));
    CHECK_EQ(1, mnRemixTwelveCBLayoutOwner(4));
    CHECK_EQ(-1, mnRemixTwelveCBLayoutOwner(24));
}

void remixTestTwelveCBLayout(void)
{
    printf("twelvecb layout\n");
    RUN_TEST(testEverySetHasTwelveDistinctFightersShownTwice);
    RUN_TEST(testDefaultSetMatchesTheAsm);
    RUN_TEST(testInvalidSetOrSlotHasNoFighter);
    RUN_TEST(testCellPositionsMatchTheAsm);
    RUN_TEST(testSlideStartsOutsideAndNeverPassesTheTarget);
    RUN_TEST(testHitTestKeepsEachPlayerInHisHalf);
    RUN_TEST(testHitTestRejectsRowsBelowTheGrid);
    RUN_TEST(testSlotOfFkindFindsThePlayersOwnPortrait);
}
