/**
 * test_envmap_rules.c - unit tests of the screen rule of SinglePlayer.asm env_mapping_fix_ (sys/sysremixenvmaprules.c).
 */

#include "test_check.h"

#include "sys/sysremixenvmaprules.h"

static void testVsPreviewResultsRoomAndContinueSetTheLookAt(void)
{
    CHECK_TRUE(sysRemixEnvMapIsLookAtScreen(0x0E));
    CHECK_TRUE(sysRemixEnvMapIsLookAtScreen(0x18));
    CHECK_TRUE(sysRemixEnvMapIsLookAtScreen(0x30));
    CHECK_TRUE(sysRemixEnvMapIsLookAtScreen(0x31));
}

static void testEveryCssScreenSetsTheLookAt(void)
{
    int screen_id;

    for (screen_id = 0x10; screen_id <= 0x14; screen_id++)
    {
        CHECK_TRUE(sysRemixEnvMapIsLookAtScreen(screen_id));
    }
}

static void testOtherScreensKeepTheirCamera(void)
{
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(0x0F));
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(0x15));
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(0x16));
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(0x17));
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(0x36));
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(0x01));
    CHECK_EQ(0, sysRemixEnvMapIsLookAtScreen(-1));
}

void remixTestEnvMapRules(void)
{
    printf("env mapping rules\n");
    RUN_TEST(testVsPreviewResultsRoomAndContinueSetTheLookAt);
    RUN_TEST(testEveryCssScreenSetsTheLookAt);
    RUN_TEST(testOtherScreensKeepTheirCamera);
}
