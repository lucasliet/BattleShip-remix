/**
 * test_yoshishared_rules.c - unit tests of the Remix Yoshi clone rules (ft/ftremix/ftremixyoshisharedrules.c: yoshishared.asm
 * shield, grab and throw fixes, JYoshi.asm egg timer and mash, yoshi_egg rows), of the double jump rules
 * (ftremixjumpaerialrules.c: yoshi_dj_fix_1, joshi_armor, Character.ness_jump) and of bowser.asm thrown_mario_bros_fix_
 * (ftremixthrownrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixjumpaerialrules.h"
#include "ft/ftremix/ftremixthrownrules.h"
#include "ft/ftremix/ftremixyoshisharedrules.h"
#include "remix/remix_roster_data.h"

#define VANILLA_ESCAPE_WAIT 250
#define VANILLA_BREAKOUT_INPUTS 750
#define VANILLA_KNOCKBACK_RESIST 140.0F

static void testEggShieldIsForYoshiAndJYoshiOnly(void)
{
    CHECK_TRUE(ftRemixYoshiSharedRulesHasEggShield(REMIX_ID_YOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesHasEggShield(REMIX_ID_JYOSHI));
    CHECK_EQ(0, ftRemixYoshiSharedRulesHasEggShield(REMIX_ID_NYOSHI));
    CHECK_EQ(0, ftRemixYoshiSharedRulesHasEggShield(REMIX_ID_PIANO));
}

static void testGrabSwallowsForEveryYoshi(void)
{
    CHECK_TRUE(ftRemixYoshiSharedRulesIsGrabSwallower(REMIX_ID_YOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsGrabSwallower(REMIX_ID_NYOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsGrabSwallower(REMIX_ID_JYOSHI));
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsGrabSwallower(REMIX_ID_PIANO));
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsGrabSwallower(REMIX_ID_KIRBY));
}

static void testThrowSwallowsForEveryYoshiAndMadPiano(void)
{
    CHECK_TRUE(ftRemixYoshiSharedRulesIsThrowSwallower(REMIX_ID_YOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsThrowSwallower(REMIX_ID_NYOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsThrowSwallower(REMIX_ID_JYOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsThrowSwallower(REMIX_ID_PIANO));
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsThrowSwallower(REMIX_ID_BOWSER));
}

static void testJapaneseLayerIsJYoshiOrAKirbyThatCopiedHim(void)
{
    CHECK_TRUE(ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_JYOSHI, REMIX_ID_KIRBY));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_KIRBY, REMIX_ID_JYOSHI));
    CHECK_TRUE(ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_JKIRBY, REMIX_ID_JYOSHI));
}

static void testOtherLayersAreNotJapanese(void)
{
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_YOSHI, REMIX_ID_JYOSHI));
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_KIRBY, REMIX_ID_YOSHI));
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_KIRBY, REMIX_ID_KIRBY));
    CHECK_EQ(0, ftRemixYoshiSharedRulesIsJapaneseEggLayer(REMIX_ID_NYOSHI, REMIX_ID_JYOSHI));
}

static void testJapaneseEggBreaksSoonerAndNeedsFewerInputs(void)
{
    CHECK_EQ(210, ftRemixYoshiSharedRulesEggEscapeWaitMax(1, VANILLA_ESCAPE_WAIT));
    CHECK_EQ(630, ftRemixYoshiSharedRulesEggBreakoutInputsMin(1, VANILLA_BREAKOUT_INPUTS));
}

static void testOtherEggsKeepTheVanillaValues(void)
{
    CHECK_EQ(VANILLA_ESCAPE_WAIT, ftRemixYoshiSharedRulesEggEscapeWaitMax(0, VANILLA_ESCAPE_WAIT));
    CHECK_EQ(VANILLA_BREAKOUT_INPUTS, ftRemixYoshiSharedRulesEggBreakoutInputsMin(0, VANILLA_BREAKOUT_INPUTS));
}

static void testDonkeySizedEggRowsOfTheBowsersDededesAndPiano(void)
{
    const int fkinds[] = { REMIX_ID_BOWSER, REMIX_ID_NBOWSER, REMIX_ID_DEDEDE, REMIX_ID_NDEDEDE, REMIX_ID_PIANO };
    float row[FTREMIX_YOSHI_EGG_ROW_FLOATS];
    int index;

    for (index = 0; index < (int)(sizeof(fkinds) / sizeof(fkinds[0])); index++)
    {
        CHECK_TRUE(ftRemixYoshiSharedRulesGetEggRow(fkinds[index], row));
        CHECK_EQ(35, (int)((row[0] * 10.0F) + 0.5F));
        CHECK_EQ(0, (int)row[1]);
        CHECK_EQ(230, (int)row[2]);
        CHECK_EQ(0, (int)row[3]);
        CHECK_EQ(245, (int)row[4]);
        CHECK_EQ(245, (int)row[5]);
        CHECK_EQ(245, (int)row[6]);
    }
}

static void testGigaBowserEggRowIsLarger(void)
{
    float row[FTREMIX_YOSHI_EGG_ROW_FLOATS];

    CHECK_TRUE(ftRemixYoshiSharedRulesGetEggRow(REMIX_ID_GBOWSER, row));
    CHECK_EQ(57, (int)((row[0] * 10.0F) + 0.5F));
    CHECK_EQ(400, (int)row[2]);
    CHECK_EQ(350, (int)row[4]);
    CHECK_EQ(350, (int)row[5]);
    CHECK_EQ(350, (int)row[6]);
}

static void testOtherFightersKeepTheParentEggRow(void)
{
    float row[FTREMIX_YOSHI_EGG_ROW_FLOATS];

    CHECK_EQ(0, ftRemixYoshiSharedRulesGetEggRow(REMIX_ID_YOSHI, row));
    CHECK_EQ(0, ftRemixYoshiSharedRulesGetEggRow(REMIX_ID_JYOSHI, row));
    CHECK_EQ(0, ftRemixYoshiSharedRulesGetEggRow(REMIX_ID_MTWO, row));
}

static void testYoshiDoubleJumpIsForEveryYoshi(void)
{
    CHECK_TRUE(ftRemixJumpAerialRulesUsesYoshiPhysics(REMIX_ID_YOSHI));
    CHECK_TRUE(ftRemixJumpAerialRulesUsesYoshiPhysics(REMIX_ID_NYOSHI));
    CHECK_TRUE(ftRemixJumpAerialRulesUsesYoshiPhysics(REMIX_ID_JYOSHI));
    CHECK_EQ(0, ftRemixJumpAerialRulesUsesYoshiPhysics(REMIX_ID_NESS));
}

static void testOnlyJYoshiHasTheJapaneseDoubleJumpArmor(void)
{
    CHECK_EQ(110, (int)ftRemixJumpAerialRulesYoshiKnockbackResist(REMIX_ID_JYOSHI, VANILLA_KNOCKBACK_RESIST));
    CHECK_EQ(140, (int)ftRemixJumpAerialRulesYoshiKnockbackResist(REMIX_ID_YOSHI, VANILLA_KNOCKBACK_RESIST));
    CHECK_EQ(140, (int)ftRemixJumpAerialRulesYoshiKnockbackResist(REMIX_ID_NYOSHI, VANILLA_KNOCKBACK_RESIST));
}

static void testNessJumpTableCoversTheNineFighters(void)
{
    const int fkinds[] = { REMIX_ID_NESS, REMIX_ID_NNESS, REMIX_ID_JNESS, REMIX_ID_LUCAS, REMIX_ID_NLUCAS,
                           REMIX_ID_MTWO, REMIX_ID_NMTWO, REMIX_ID_PEACH, REMIX_ID_NPEACH };
    int index;

    for (index = 0; index < (int)(sizeof(fkinds) / sizeof(fkinds[0])); index++)
    {
        CHECK_TRUE(ftRemixJumpAerialRulesUsesNessPhysics(fkinds[index]));
    }
}

static void testNessJumpTableLeavesOtherFightersAlone(void)
{
    CHECK_EQ(0, ftRemixJumpAerialRulesUsesNessPhysics(REMIX_ID_YOSHI));
    CHECK_EQ(0, ftRemixJumpAerialRulesUsesNessPhysics(REMIX_ID_KIRBY));
    CHECK_EQ(0, ftRemixJumpAerialRulesUsesNessPhysics(REMIX_ID_JYOSHI));
}

static void testBowsersKeepTheThrownStatus(void)
{
    CHECK_TRUE(ftRemixThrownRulesKeepsStatusAtAnimEnd(REMIX_ID_BOWSER));
    CHECK_TRUE(ftRemixThrownRulesKeepsStatusAtAnimEnd(REMIX_ID_GBOWSER));
    CHECK_TRUE(ftRemixThrownRulesKeepsStatusAtAnimEnd(REMIX_ID_NBOWSER));
}

static void testOtherFightersSwitchStatusWhenTheThrowEnds(void)
{
    CHECK_EQ(0, ftRemixThrownRulesKeepsStatusAtAnimEnd(REMIX_ID_DEDEDE));
    CHECK_EQ(0, ftRemixThrownRulesKeepsStatusAtAnimEnd(REMIX_ID_YOSHI));
}

void remixTestYoshiSharedRules(void)
{
    printf("yoshi shared rules\n");
    RUN_TEST(testEggShieldIsForYoshiAndJYoshiOnly);
    RUN_TEST(testGrabSwallowsForEveryYoshi);
    RUN_TEST(testThrowSwallowsForEveryYoshiAndMadPiano);
    RUN_TEST(testJapaneseLayerIsJYoshiOrAKirbyThatCopiedHim);
    RUN_TEST(testOtherLayersAreNotJapanese);
    RUN_TEST(testJapaneseEggBreaksSoonerAndNeedsFewerInputs);
    RUN_TEST(testOtherEggsKeepTheVanillaValues);
    RUN_TEST(testDonkeySizedEggRowsOfTheBowsersDededesAndPiano);
    RUN_TEST(testGigaBowserEggRowIsLarger);
    RUN_TEST(testOtherFightersKeepTheParentEggRow);
    RUN_TEST(testYoshiDoubleJumpIsForEveryYoshi);
    RUN_TEST(testOnlyJYoshiHasTheJapaneseDoubleJumpArmor);
    RUN_TEST(testNessJumpTableCoversTheNineFighters);
    RUN_TEST(testNessJumpTableLeavesOtherFightersAlone);
    RUN_TEST(testBowsersKeepTheThrownStatus);
    RUN_TEST(testOtherFightersSwitchStatusWhenTheThrowEnds);
}
