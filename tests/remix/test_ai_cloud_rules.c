/**
 * test_ai_cloud_rules.c - unit tests of the Remix AI Magic Cloud rules (ft/ftremix/ftremixaicloudrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaicloudrules.h"

static void testGoemonRideIsUspAction(void)
{
    CHECK_TRUE(ftRemixAiCloudIsGoemonUspAction(0xDF));
}

static void testGoemonJumpIsUspAction(void)
{
    CHECK_TRUE(ftRemixAiCloudIsGoemonUspAction(0xE1));
}

static void testGoemonIdleIsNotUspAction(void)
{
    CHECK_EQ(0, ftRemixAiCloudIsGoemonUspAction(0xE3));
}

static void testEbisumaruRideIsUspAction(void)
{
    CHECK_TRUE(ftRemixAiCloudIsEbisumaruUspAction(0xDF));
}

static void testEbisumaruAttackIsNotUspAction(void)
{
    CHECK_EQ(0, ftRemixAiCloudIsEbisumaruUspAction(0xE0));
}

static void testGoemonStickUpCoversRideAndAttack(void)
{
    CHECK_TRUE(ftRemixAiCloudIsGoemonStickUpAction(0xDF));
    CHECK_TRUE(ftRemixAiCloudIsGoemonStickUpAction(0xE0));
}

static void testGoemonStickUpSkipsJump(void)
{
    CHECK_EQ(0, ftRemixAiCloudIsGoemonStickUpAction(0xE1));
}

static void testHighByteReplaceKeepsLowByte(void)
{
    CHECK_EQ(0x80FF, ftRemixAiCloudReplaceButtonHighByte(0x12FF, 0x80));
}

static void testHighByteZeroClearsHighButtons(void)
{
    CHECK_EQ(0x0034, ftRemixAiCloudReplaceButtonHighByte(0xF034, 0));
}

void remixTestAiCloudRules(void)
{
    printf("ai cloud rules\n");
    RUN_TEST(testGoemonRideIsUspAction);
    RUN_TEST(testGoemonJumpIsUspAction);
    RUN_TEST(testGoemonIdleIsNotUspAction);
    RUN_TEST(testEbisumaruRideIsUspAction);
    RUN_TEST(testEbisumaruAttackIsNotUspAction);
    RUN_TEST(testGoemonStickUpCoversRideAndAttack);
    RUN_TEST(testGoemonStickUpSkipsJump);
    RUN_TEST(testHighByteReplaceKeepsLowByte);
    RUN_TEST(testHighByteZeroClearsHighButtons);
}
