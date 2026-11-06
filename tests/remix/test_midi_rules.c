/**
 * test_midi_rules.c - unit tests of the Remix MIDI and FGM rules (gm/gmremixmidirules.c).
 */

#include "test_check.h"

#include "gm/gmremixmidirules.h"

#define PRIORITY_TABLE 0x80411480u
#define MASTER_VOLUME_TABLE 0x80412040u
#define SONG_ARRAY 0x804121C0u

static void testOnlyTheFirstRemixFgmIdIsSilenced(void)
{
    CHECK_EQ(1, gmRemixMidiRulesFgmIsSilenced(0x2B7));
    CHECK_EQ(0, gmRemixMidiRulesFgmIsSilenced(0x2B6));
    CHECK_EQ(0, gmRemixMidiRulesFgmIsSilenced(0x2B8));
    CHECK_EQ(0, gmRemixMidiRulesFgmIsSilenced(0));
}

static void testPointerTableEntriesAreFourBytesPerSong(void)
{
    CHECK_EQ(PRIORITY_TABLE, gmRemixMidiRulesSongEntryAddress(PRIORITY_TABLE, 0, 4));
    CHECK_EQ(PRIORITY_TABLE + 4 * 91, gmRemixMidiRulesSongEntryAddress(PRIORITY_TABLE, 91, 4));
    CHECK_EQ(PRIORITY_TABLE + 4 * 375, gmRemixMidiRulesSongEntryAddress(PRIORITY_TABLE, 375, 4));
}

static void testMasterVolumeTableEntriesAreOneBytePerSong(void)
{
    CHECK_EQ(MASTER_VOLUME_TABLE + 317, gmRemixMidiRulesSongEntryAddress(MASTER_VOLUME_TABLE, 317, 1));
}

static void testNoSongOrSongBeyondTheMusicTableHasNoEntry(void)
{
    CHECK_EQ(0, gmRemixMidiRulesSongEntryAddress(PRIORITY_TABLE, -1, 4));
    CHECK_EQ(0, gmRemixMidiRulesSongEntryAddress(PRIORITY_TABLE, GMREMIX_MIDI_COUNT, 4));
    CHECK_EQ(0, gmRemixMidiRulesSongEntryAddress(MASTER_VOLUME_TABLE, 1000, 1));
}

static void testInstrumentEntriesScaleWithTheEntrySize(void)
{
    CHECK_EQ(SONG_ARRAY + 7, gmRemixMidiRulesInstrumentEntryAddress(SONG_ARRAY, 7, 1));
    CHECK_EQ(SONG_ARRAY + 14, gmRemixMidiRulesInstrumentEntryAddress(SONG_ARRAY, 7, 2));
    CHECK_EQ(SONG_ARRAY + 70, gmRemixMidiRulesInstrumentEntryAddress(SONG_ARRAY, 70, 1));
}

static void testSongWithoutAnArrayOrInstrumentBeyondTheBankHasNoEntry(void)
{
    CHECK_EQ(0, gmRemixMidiRulesInstrumentEntryAddress(0, 7, 1));
    CHECK_EQ(0, gmRemixMidiRulesInstrumentEntryAddress(SONG_ARRAY, GMREMIX_MIDI_INSTRUMENT_COUNT, 1));
    CHECK_EQ(0, gmRemixMidiRulesInstrumentEntryAddress(SONG_ARRAY, 255, 2));
}

static void testZeroOverrideKeepsTheOriginalValue(void)
{
    CHECK_EQ(100, gmRemixMidiRulesPickOverride(0, 100));
    CHECK_EQ(200, gmRemixMidiRulesPickOverride(0, 200));
}

static void testNonZeroOverrideReplacesTheOriginalValue(void)
{
    CHECK_EQ(0x7F, gmRemixMidiRulesPickOverride(0x7F, 1));
    CHECK_EQ(1200, gmRemixMidiRulesPickOverride(1200, 200));
    CHECK_EQ(-100, gmRemixMidiRulesPickOverride(-100, 200));
}

void remixTestMidiRules(void)
{
    printf("midi rules\n");
    RUN_TEST(testOnlyTheFirstRemixFgmIdIsSilenced);
    RUN_TEST(testPointerTableEntriesAreFourBytesPerSong);
    RUN_TEST(testMasterVolumeTableEntriesAreOneBytePerSong);
    RUN_TEST(testNoSongOrSongBeyondTheMusicTableHasNoEntry);
    RUN_TEST(testInstrumentEntriesScaleWithTheEntrySize);
    RUN_TEST(testSongWithoutAnArrayOrInstrumentBeyondTheBankHasNoEntry);
    RUN_TEST(testZeroOverrideKeepsTheOriginalValue);
    RUN_TEST(testNonZeroOverrideReplacesTheOriginalValue);
}
