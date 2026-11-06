/**
 * test_1p_intro_rules.c - unit tests of the 1P title card rules (sc/sc1pmode/sc1pintrorules.c).
 */

#include "test_check.h"

#include "remix_roster_data.h"
#include "sc/sc1pmode/sc1pintrorules.h"

#define STAGE_LINK 0
#define STAGE_TEAM 1
#define STAGE_GIANT 6
#define STAGE_BOSS 10
#define NOT_REMIX_1P 0
#define REMIX_1P 1

static void testHumanPositionsTakeTheHumanPort(void)
{
    CHECK_EQ(2, sc1PIntroRulesHumanSidePort(0, 2));
    CHECK_EQ(3, sc1PIntroRulesHumanSidePort(1, 3));
    CHECK_EQ(0, sc1PIntroRulesHumanSidePort(3, 0));
}

static void testAllyPositionsTakeTheOtherOfP1AndP2(void)
{
    CHECK_EQ(1, sc1PIntroRulesHumanSidePort(2, 0));
    CHECK_EQ(1, sc1PIntroRulesHumanSidePort(4, 0));
    CHECK_EQ(0, sc1PIntroRulesHumanSidePort(5, 1));
    CHECK_EQ(0, sc1PIntroRulesHumanSidePort(2, 3));
}

static void testOpponentPortIsTheOtherOfP1AndP2(void)
{
    CHECK_EQ(1, sc1PIntroRulesOpponentPort(0));
    CHECK_EQ(0, sc1PIntroRulesOpponentPort(1));
    CHECK_EQ(0, sc1PIntroRulesOpponentPort(2));
    CHECK_EQ(0, sc1PIntroRulesOpponentPort(3));
}

static void testGiantDonkeyHumanMakesDonkeyTakeTheAlternateCostume(void)
{
    CHECK_EQ(1, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_DONKEY, REMIX_ID_GDONKEY, 0, NOT_REMIX_1P, STAGE_GIANT));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_FOX, REMIX_ID_GDONKEY, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_DONKEY, REMIX_ID_GDONKEY, 1, NOT_REMIX_1P, STAGE_LINK));
}

static void testOriginalPolygonHumanMakesPolygonsTakeTheAlternateCostume(void)
{
    CHECK_EQ(1, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NMARIO, REMIX_ID_NFOX, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(1, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NNESS, REMIX_ID_NMARIO, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NMARIO, REMIX_ID_NFOX, 2, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_MARIO, REMIX_ID_NFOX, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_GDONKEY, REMIX_ID_NFOX, 0, NOT_REMIX_1P, STAGE_LINK));
}

static void testRemixPolygonHumanMakesOriginalPolygonsTakeTheAlternateCostume(void)
{
    CHECK_EQ(1, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NLINK, REMIX_ID_NWARIO, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NWARIO, REMIX_ID_NWARIO, 0, NOT_REMIX_1P, STAGE_LINK));
}

static void testNonPolygonHumanLeavesPolygonsAlone(void)
{
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NMARIO, REMIX_ID_MARIO, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NMARIO, REMIX_ID_METAL, 0, NOT_REMIX_1P, STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_NMARIO, REMIX_ID_FALCO, 0, NOT_REMIX_1P, STAGE_LINK));
}

static void testGiantStageSameFighterTakesTheAlternateCostumeInRemix1P(void)
{
    CHECK_EQ(1, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_FALCO, REMIX_ID_FALCO, 0, REMIX_1P, STAGE_GIANT));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_FALCO, REMIX_ID_FALCO, 0, NOT_REMIX_1P, STAGE_GIANT));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_FALCO, REMIX_ID_FALCO, 0, REMIX_1P, STAGE_TEAM));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_FALCO, REMIX_ID_FALCO, 3, REMIX_1P, STAGE_GIANT));
    CHECK_EQ(0, sc1PIntroRulesOpponentCostumeIndex(REMIX_ID_FOX, REMIX_ID_FALCO, 0, REMIX_1P, STAGE_GIANT));
}

static void testDuoSlotKeepsTheVanillaAlternateCostumeOutsideTheMetalStage(void)
{
    CHECK_EQ(1, sc1PIntroRulesDuoMarioSlotCostumeIndex(NOT_REMIX_1P, STAGE_BOSS, REMIX_ID_METAL, REMIX_ID_MARIO, 0));
    CHECK_EQ(1, sc1PIntroRulesDuoMarioSlotCostumeIndex(REMIX_1P, STAGE_LINK, REMIX_ID_METAL, REMIX_ID_MARIO, 0));
    CHECK_EQ(1, sc1PIntroRulesDuoMarioSlotCostumeIndex(REMIX_1P, STAGE_BOSS, REMIX_ID_MARIO, REMIX_ID_MARIO, 2));
}

static void testMetalMarioSlotNeedsAMetalMarioHumanWithCostumeZeroToTakeTheAlternate(void)
{
    CHECK_EQ(1, sc1PIntroRulesDuoMarioSlotCostumeIndex(REMIX_1P, STAGE_BOSS, REMIX_ID_METAL, REMIX_ID_METAL, 0));
    CHECK_EQ(0, sc1PIntroRulesDuoMarioSlotCostumeIndex(REMIX_1P, STAGE_BOSS, REMIX_ID_METAL, REMIX_ID_METAL, 2));
    CHECK_EQ(0, sc1PIntroRulesDuoMarioSlotCostumeIndex(REMIX_1P, STAGE_BOSS, REMIX_ID_METAL, REMIX_ID_MARIO, 0));
}

static void testNameWidthCutOnlyOnTheTeamAndGiantCards(void)
{
    CHECK_TRUE(sc1PIntroRulesNameWidthApplies(STAGE_TEAM));
    CHECK_TRUE(sc1PIntroRulesNameWidthApplies(STAGE_GIANT));
    CHECK_EQ(0, sc1PIntroRulesNameWidthApplies(STAGE_LINK));
    CHECK_EQ(0, sc1PIntroRulesNameWidthApplies(STAGE_BOSS));
    CHECK_EQ(0, sc1PIntroRulesNameWidthApplies(4));
    CHECK_EQ(0, sc1PIntroRulesNameWidthApplies(9));
}

static void testTinyTeamMembersTakeTheirIndexModuloTheFighterKind(void)
{
    CHECK_EQ(0, sc1PIntroRulesTinyTeamCostume(0, REMIX_ID_FOX));
    CHECK_EQ(0, sc1PIntroRulesTinyTeamCostume(2, REMIX_ID_FOX));
    CHECK_EQ(1, sc1PIntroRulesTinyTeamCostume(1, REMIX_ID_DONKEY));
    CHECK_EQ(0, sc1PIntroRulesTinyTeamCostume(2, REMIX_ID_DONKEY));
    CHECK_EQ(2, sc1PIntroRulesTinyTeamCostume(2, REMIX_ID_SAMUS));
    CHECK_EQ(2, sc1PIntroRulesTinyTeamCostume(2, REMIX_ID_FALCO));
}

static void testTinyTeamOfMarioDoesNotDivideByZero(void)
{
    CHECK_EQ(0, sc1PIntroRulesTinyTeamCostume(0, REMIX_ID_MARIO));
    CHECK_EQ(0, sc1PIntroRulesTinyTeamCostume(2, REMIX_ID_MARIO));
}

void remixTest1PIntroRules(void)
{
    RUN_TEST(testHumanPositionsTakeTheHumanPort);
    RUN_TEST(testAllyPositionsTakeTheOtherOfP1AndP2);
    RUN_TEST(testOpponentPortIsTheOtherOfP1AndP2);
    RUN_TEST(testGiantDonkeyHumanMakesDonkeyTakeTheAlternateCostume);
    RUN_TEST(testOriginalPolygonHumanMakesPolygonsTakeTheAlternateCostume);
    RUN_TEST(testRemixPolygonHumanMakesOriginalPolygonsTakeTheAlternateCostume);
    RUN_TEST(testNonPolygonHumanLeavesPolygonsAlone);
    RUN_TEST(testGiantStageSameFighterTakesTheAlternateCostumeInRemix1P);
    RUN_TEST(testDuoSlotKeepsTheVanillaAlternateCostumeOutsideTheMetalStage);
    RUN_TEST(testMetalMarioSlotNeedsAMetalMarioHumanWithCostumeZeroToTakeTheAlternate);
    RUN_TEST(testNameWidthCutOnlyOnTheTeamAndGiantCards);
    RUN_TEST(testTinyTeamMembersTakeTheirIndexModuloTheFighterKind);
    RUN_TEST(testTinyTeamOfMarioDoesNotDivideByZero);
}
