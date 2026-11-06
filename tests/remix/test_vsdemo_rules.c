/**
 * test_vsdemo_rules.c - unit tests of the Remix VS demo pick rules (sc/sccommon/scremixvsdemorules.c). Expected values come
 * from VsDemo.asm enable_nonfocused_remix_chars_.
 */

#include "test_check.h"

#include "sc/sccommon/scremixvsdemorules.h"

static void testPortFourDrawsAgainWhenItRepeatsPortThree(void)
{
    CHECK_TRUE(scRemixVsDemoRepeatsPort3Fighter(3, 0x20, 0x20));
}

static void testPortFourKeepsADifferentFighter(void)
{
    CHECK_EQ(0, scRemixVsDemoRepeatsPort3Fighter(3, 0x20, 0x21));
}

static void testPortThreeOnlyAvoidsTheFocusedFighters(void)
{
    CHECK_EQ(0, scRemixVsDemoRepeatsPort3Fighter(2, 0x20, 0x20));
}

void remixTestVsDemoRules(void)
{
    printf("vs demo rules\n");
    RUN_TEST(testPortFourDrawsAgainWhenItRepeatsPortThree);
    RUN_TEST(testPortFourKeepsADifferentFighter);
    RUN_TEST(testPortThreeOnlyAvoidsTheFocusedFighters);
}
