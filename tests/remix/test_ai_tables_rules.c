/**
 * test_ai_tables_rules.c - unit tests of the Remix CPU attack table rules (ft/ftremix/ftremixaitablesrules.c).
 */

#include "test_check.h"

#include "ft/ftremix/ftremixaitablesrules.h"

#define ENTRY_WORDS FTREMIX_AI_TABLES_WORDS_PER_ENTRY
#define TERMINATOR_WORD 0xFFFFFFFFu

static void testOnlyTheAllOnesWordIsATerminator(void)
{
    CHECK_TRUE(ftRemixAiTablesRulesIsTerminator(TERMINATOR_WORD));
    CHECK_EQ(0, ftRemixAiTablesRulesIsTerminator(0x13u));
    CHECK_EQ(0, ftRemixAiTablesRulesIsTerminator(0u));
}

static void testInputsFromThirtyOneAreCustom(void)
{
    CHECK_EQ(0, ftRemixAiTablesRulesIsCustomInput(0x30));
    CHECK_EQ(1, ftRemixAiTablesRulesIsCustomInput(0x31));
    CHECK_EQ(1, ftRemixAiTablesRulesIsCustomInput(0x40));
}

static void testTerminatorIsNotCustom(void)
{
    CHECK_EQ(0, ftRemixAiTablesRulesIsCustomInput(-1));
}

static void testWordsBitCastToFloats(void)
{
    CHECK_TRUE(ftRemixAiTablesRulesBitsToFloat(0x3F800000u) == 1.0F);
    CHECK_TRUE(ftRemixAiTablesRulesBitsToFloat(0xC0000000u) == -2.0F);
    CHECK_TRUE(ftRemixAiTablesRulesBitsToFloat(0x41200000u) == 10.0F);
    CHECK_TRUE(ftRemixAiTablesRulesBitsToFloat(0u) == 0.0F);
}

static void testWalkStopsAtTheSecondTerminator(void)
{
    unsigned int words[5 * ENTRY_WORDS] = {0};

    words[0 * ENTRY_WORDS] = 0x13u;
    words[1 * ENTRY_WORDS] = TERMINATOR_WORD;
    words[2 * ENTRY_WORDS] = 0x14u;
    words[3 * ENTRY_WORDS] = TERMINATOR_WORD;
    words[4 * ENTRY_WORDS] = 0x15u;
    CHECK_EQ(4, ftRemixAiTablesRulesCountEntries(words, 5));
}

static void testWalkWithOneTerminatorInsideCapacityFails(void)
{
    unsigned int words[2 * ENTRY_WORDS] = {0};

    words[0] = 0x13u;
    words[1 * ENTRY_WORDS] = TERMINATOR_WORD;
    CHECK_EQ(-1, ftRemixAiTablesRulesCountEntries(words, 2));
}

static void testLv10IsOnlyLevelTen(void)
{
    CHECK_EQ(1, ftRemixAiTablesRulesIsLv10Level(10));
    CHECK_EQ(0, ftRemixAiTablesRulesIsLv10Level(9));
    CHECK_EQ(0, ftRemixAiTablesRulesIsLv10Level(0));
}

static void testTableIndexCoversEveryCharacterOnly(void)
{
    CHECK_EQ(1, ftRemixAiTablesRulesIsTableIndex(0));
    CHECK_EQ(1, ftRemixAiTablesRulesIsTableIndex(96));
    CHECK_EQ(0, ftRemixAiTablesRulesIsTableIndex(97));
    CHECK_EQ(0, ftRemixAiTablesRulesIsTableIndex(-1));
}

void remixTestAiTablesRules(void)
{
    printf("remix AI attack tables rules\n");
    RUN_TEST(testOnlyTheAllOnesWordIsATerminator);
    RUN_TEST(testInputsFromThirtyOneAreCustom);
    RUN_TEST(testTerminatorIsNotCustom);
    RUN_TEST(testWordsBitCastToFloats);
    RUN_TEST(testWalkStopsAtTheSecondTerminator);
    RUN_TEST(testWalkWithOneTerminatorInsideCapacityFails);
    RUN_TEST(testLv10IsOnlyLevelTen);
    RUN_TEST(testTableIndexCoversEveryCharacterOnly);
}
