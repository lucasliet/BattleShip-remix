/**
 * remix_toggles.h - the Smash Remix settings (Toggles.asm) as a runtime registry.
 *
 * Every toggle is a `Menu.entry` node of the Remix image (value, range, string table and title) and has four profile
 * defaults (Community, Tournament, Netplay, Japanese) in `Toggles.profile_defaults_*`. The registry is built from that
 * image; a value is stored as the CVar `gRemix.Toggle.<index>` (so the port menu edits it and it persists) and can be
 * overridden for a run with SSB64_REMIX_TOGGLES.
 */

#ifndef PORT_REMIX_REMIX_TOGGLES_H
#define PORT_REMIX_REMIX_TOGGLES_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define REMIX_TOGGLE_PROFILE_COUNT 4

/**
 * Builds the registry from the Remix image (call after remix_ram_image_load) and applies the environment overrides
 * SSB64_REMIX_PROFILE=<0..3> and SSB64_REMIX_TOGGLES="<title>=<value>;...". Returns the number of toggles, 0 on failure.
 */
int remix_toggles_init(void);

/** Current value of the toggle whose `Menu.entry` node is at `entry_addr` (a REMIX_SYM_Toggles__entry_* address); 0 when unknown. */
int remix_toggle_get(uint32_t entry_addr);

/** Sets that toggle, clamped to its range, and stores it in its CVar. */
void remix_toggle_set(uint32_t entry_addr, int value);

/** Sets every toggle to the defaults of profile 0 (CE), 1 (TE), 2 (NE) or 3 (JP). */
void remix_toggles_load_profile(int profile);

/* Toggle groups as numbered by remix_toggles::Group (the Menu.entry chains of Toggles.asm). */
#define REMIX_TOGGLE_GROUP_GAMEPLAY 1
#define REMIX_TOGGLE_GROUP_POKEMON 4

/**
 * Current value and highest value of every toggle of a group, in menu order (the order Toggles.asm packs them into its SRAM block).
 *
 * @param group REMIX_TOGGLE_GROUP_* number.
 * @param values Receives the current value of each toggle.
 * @param maxima Receives the highest value of each toggle.
 * @param capacity Size of both arrays.
 * @return Number of toggles written, 0 when the registry is not built or the group is unknown.
 */
int remix_toggle_group_snapshot(int group, int *values, int *maxima, int capacity);

#ifdef __cplusplus
}

#include <map>
#include <string>
#include <vector>

namespace remix_toggles {

enum Group { Remix, Gameplay, Music, Stage, Pokemon, GroupCount };

struct Toggle {
    uint32_t entryAddr;
    Group group;
    int index;
    std::string title;
    int min;
    int max;
    bool isBool;
    bool hasAction;
    uint32_t extra; /* Menu.entry word 0x24: the BGM id of a Random Music toggle (Toggles.asm preview_bgm_) */
    std::string cvar;
    int defaults[REMIX_TOGGLE_PROFILE_COUNT];
    std::vector<std::string> strings;
};

/**
 * A title row with the toggle_all_ A routine ("Random Music Toggles:", "Random Stage Toggles:", "Toggle:") and the toggles after it.
 * The profile loader row of the page ("Load Profile:", load_sub_profile_) is the entry just before the title.
 */
struct ToggleAllRow {
    Group group;
    uint32_t titleAddr;
    std::string title;
    std::vector<uint32_t> followers; /* entry addresses of every toggle after the title, in chain order */
    uint32_t loaderEntryAddr;        /* the "Load Profile:" entry before the title, 0 when the page has none */
};

const char *GroupName(Group group);
const std::vector<Toggle> &All();
const std::vector<ToggleAllRow> &ToggleAllRows();
/** The toggle whose Menu.entry node is at entryAddr, nullptr when unknown. */
const Toggle *Find(uint32_t entryAddr);
std::map<int32_t, const char *> ComboMap(const Toggle &toggle);

} // namespace remix_toggles
#endif

#endif /* PORT_REMIX_REMIX_TOGGLES_H */
