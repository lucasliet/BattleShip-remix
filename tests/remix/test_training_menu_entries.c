/**
 * test_training_menu_entries.c - unit tests of the Training Z menu entries (sc/sc1pmode/sc1ptrainingmenuentries.c).
 */

#include "test_check.h"

#include <string.h>

#include "remix_training.h"
#include "sc/sc1pmode/sc1ptrainingmenuentries.h"
#include "sc/sc1pmode/sc1ptrainingmenurules.h"

typedef struct ExpectedRange
{
    int32_t slot;
    int32_t min;
    int32_t max;

} ExpectedRange;

/* Ranges of the port registry (remix_training.cpp kOptions); the menu must not offer a value the registry would clamp. */
static const ExpectedRange sRegistryRanges[] =
{
    { SC1PTRAININGMENU_SLOT_COSTUME, 0, 12 },          { SC1PTRAININGMENU_SLOT_TYPE, 0, 3 },
    { SC1PTRAININGMENU_SLOT_PERCENT, 0, 999 },        { SC1PTRAININGMENU_SLOT_RESET_SETS_PERCENT, 0, 1 },
    { SC1PTRAININGMENU_SLOT_SHIELD_BREAK, 0, 2 },     { SC1PTRAININGMENU_SLOT_OOS, 0, 11 },
    { SC1PTRAININGMENU_SLOT_MUSIC, 0, 16 },           { SC1PTRAININGMENU_SLOT_STAGE_BG, 0, 1 },
    { SC1PTRAININGMENU_SLOT_TECH, 0, 6 },             { SC1PTRAININGMENU_SLOT_DI_TYPE, 0, 3 },
    { SC1PTRAININGMENU_SLOT_DI_STRENGTH, 0, 3 },      { SC1PTRAININGMENU_SLOT_DI_DIRECTION, 0, 6 },
    { SC1PTRAININGMENU_SLOT_DI_FIRST_HIT, 1, 99 },    { SC1PTRAININGMENU_SLOT_DPAD, 0, 2 },
    { SC1PTRAININGMENU_SLOT_DI_PRACTICE, 0, 1 },      { SC1PTRAININGMENU_SLOT_SPAM_PRACTICE, 0, 13 },
    { SC1PTRAININGMENU_SLOT_SPAM_INTERVAL, 1, 999 },  { SC1PTRAININGMENU_SLOT_SPAM_RANDOM, 0, 1 }
};

static void testTheMenuHasTwentyThreeEntriesOnTwoPages(void)
{
    CHECK_EQ(23, SC1PTRAININGMENU_SLOT_COUNT);
    CHECK_EQ(1, sc1PTrainingMenuRulesGetPage(SC1PTRAININGMENU_SLOT_COUNT - 1));
    CHECK_TRUE(sc1PTrainingMenuEntriesGet(-1) == NULL);
    CHECK_TRUE(sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_COUNT) == NULL);
}

static void testTheFirstPageEndsWithStageBackground(void)
{
    CHECK_EQ(11, SC1PTRAININGMENU_SLOT_STAGE_BG);
    CHECK_EQ(12, SC1PTRAININGMENU_SLOT_TECH);
    CHECK_EQ(0, strcmp("Port:", sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PORT)->title));
    CHECK_EQ(0, strcmp("Stage Background:", sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_STAGE_BG)->title));
    CHECK_EQ(0, strcmp("Preserve Menu Selections:", sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PRESERVE)->title));
}

static void testPortEntryRunsFromOneToFour(void)
{
    const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PORT);

    CHECK_EQ(1, entry->min);
    CHECK_EQ(4, entry->max);
    CHECK_EQ(SC1PTRAININGMENU_OPTION_LOCAL, entry->option);
}

static void testPerPortEntriesPickTheOptionOfThePort(void)
{
    const SC1PTrainingMenuEntry *character = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_CHARACTER);
    const SC1PTrainingMenuEntry *percent = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PERCENT);
    const SC1PTrainingMenuEntry *spawn = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_SPAWN);

    CHECK_EQ(REMIX_TRAINING_CHARACTER_P1 + 2, sc1PTrainingMenuEntriesGetOption(character, 2));
    CHECK_EQ(REMIX_TRAINING_PERCENT_P1 + 3, sc1PTrainingMenuEntriesGetOption(percent, 3));
    CHECK_EQ(REMIX_TRAINING_SPAWN_P1, sc1PTrainingMenuEntriesGetOption(spawn, 0));
}

static void testSharedEntriesIgnoreThePort(void)
{
    const SC1PTrainingMenuEntry *oos = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_OOS);

    CHECK_EQ(REMIX_TRAINING_OOS, sc1PTrainingMenuEntriesGetOption(oos, 3));
}

static void testLocalEntriesHaveNoRegistryOption(void)
{
    const SC1PTrainingMenuEntry *preserve = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PRESERVE);

    CHECK_EQ(SC1PTRAININGMENU_OPTION_LOCAL, sc1PTrainingMenuEntriesGetOption(preserve, 0));
}

static void testRangesAgreeWithThePortRegistry(void)
{
    size_t index;

    for (index = 0; index < (sizeof(sRegistryRanges) / sizeof(sRegistryRanges[0])); index++)
    {
        const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(sRegistryRanges[index].slot);

        CHECK_EQ(sRegistryRanges[index].min, entry->min);
        CHECK_EQ(sRegistryRanges[index].max, entry->max);
    }
}

static void testEveryValueOfAStringEntryHasText(void)
{
    int32_t slot;
    int32_t value;

    for (slot = 0; slot < SC1PTRAININGMENU_SLOT_COUNT; slot++)
    {
        const SC1PTrainingMenuEntry *entry = sc1PTrainingMenuEntriesGet(slot);

        if ((entry->kind != SC1PTRAININGMENU_KIND_STRINGS) && (entry->kind != SC1PTRAININGMENU_KIND_CHARACTER))
        {
            continue;
        }
        for (value = entry->min; value <= entry->max; value++)
        {
            CHECK_TRUE(sc1PTrainingMenuEntriesGetText(entry, value) != NULL);
        }
        CHECK_TRUE(sc1PTrainingMenuEntriesGetText(entry, entry->max + 1) == NULL);
        CHECK_TRUE(sc1PTrainingMenuEntriesGetText(entry, entry->min - 1) == NULL);
    }
}

static void testNumberEntriesAndTitlesHaveNoText(void)
{
    CHECK_TRUE(sc1PTrainingMenuEntriesGetText(sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PERCENT), 50) == NULL);
    CHECK_TRUE(sc1PTrainingMenuEntriesGetText(sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_PORT), 2) == NULL);
    CHECK_TRUE(sc1PTrainingMenuEntriesGetText(sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_SET_CUSTOM_SPAWN), 0) == NULL);
}

static void testSpawnEntryEndsWithCustom(void)
{
    const SC1PTrainingMenuEntry *spawn = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_SPAWN);

    CHECK_EQ(5, spawn->max);
    CHECK_EQ(0, strcmp("Custom", sc1PTrainingMenuEntriesGetText(spawn, 5)));
    CHECK_EQ(0, strcmp("Auto", sc1PTrainingMenuEntriesGetText(spawn, 0)));
}

static void testCharacterEntryStartsWithAutoThenTheRoster(void)
{
    const SC1PTrainingMenuEntry *character = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_CHARACTER);

    CHECK_EQ(0, strcmp("Auto", sc1PTrainingMenuEntriesGetText(character, 0)));
    CHECK_EQ(0, strcmp("Mario", sc1PTrainingMenuEntriesGetText(character, 1)));
    CHECK_TRUE(character->max > 90);
}

static void testBoolEntriesUseTheMenuAsmTexts(void)
{
    const SC1PTrainingMenuEntry *stage_bg = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_STAGE_BG);
    const SC1PTrainingMenuEntry *dpad = sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_DPAD);

    CHECK_EQ(0, strcmp("OFF", sc1PTrainingMenuEntriesGetText(stage_bg, 0)));
    CHECK_EQ(0, strcmp("ON", sc1PTrainingMenuEntriesGetText(stage_bg, 1)));
    CHECK_EQ(0, strcmp("ON", sc1PTrainingMenuEntriesGetText(dpad, 0)));
    CHECK_EQ(0, strcmp("Reset Only", sc1PTrainingMenuEntriesGetText(dpad, 1)));
    CHECK_EQ(0, strcmp("OFF", sc1PTrainingMenuEntriesGetText(dpad, 2)));
}

static void testOnlyPercentAndMusicWaitForA(void)
{
    int32_t slot;

    for (slot = 0; slot < SC1PTRAININGMENU_SLOT_COUNT; slot++)
    {
        int32_t waits = (sc1PTrainingMenuEntriesGet(slot)->action == SC1PTRAININGMENU_ACTION_COMMIT) ? 1 : 0;
        int32_t expected = ((slot == SC1PTRAININGMENU_SLOT_PERCENT) || (slot == SC1PTRAININGMENU_SLOT_MUSIC)) ? 1 : 0;

        CHECK_EQ(expected, waits);
    }
    CHECK_EQ(SC1PTRAININGMENU_ACTION_SET_CUSTOM_SPAWN, sc1PTrainingMenuEntriesGet(SC1PTRAININGMENU_SLOT_SET_CUSTOM_SPAWN)->action);
}

void remixTestTrainingMenuEntries(void)
{
    printf("training Z menu entries\n");
    RUN_TEST(testTheMenuHasTwentyThreeEntriesOnTwoPages);
    RUN_TEST(testTheFirstPageEndsWithStageBackground);
    RUN_TEST(testPortEntryRunsFromOneToFour);
    RUN_TEST(testPerPortEntriesPickTheOptionOfThePort);
    RUN_TEST(testSharedEntriesIgnoreThePort);
    RUN_TEST(testLocalEntriesHaveNoRegistryOption);
    RUN_TEST(testRangesAgreeWithThePortRegistry);
    RUN_TEST(testEveryValueOfAStringEntryHasText);
    RUN_TEST(testNumberEntriesAndTitlesHaveNoText);
    RUN_TEST(testSpawnEntryEndsWithCustom);
    RUN_TEST(testCharacterEntryStartsWithAutoThenTheRoster);
    RUN_TEST(testBoolEntriesUseTheMenuAsmTexts);
    RUN_TEST(testOnlyPercentAndMusicWaitForA);
}
