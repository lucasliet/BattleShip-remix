/**
 * remix_modes.h - options of the Smash Remix VS modes that the Remix draws inside its menus (the CSS toggle of
 * Smashketball, King of the Hill goal). The port keeps them as CVars edited in the port menu (Remix > Modes).
 */

#ifndef PORT_REMIX_REMIX_MODES_H
#define PORT_REMIX_REMIX_MODES_H

#define REMIX_MODES_CVAR_SMASHKETBALL_TYPE "gRemix.Modes.SmashketballType"

#define REMIX_MODES_TAGTEAM_SLOTS 6

#ifdef __cplusplus
extern "C" {
#endif

/** Smashketball type: 0 basketball (Smashketball stage), 1 soccer (Soccer stage). */
int remix_modes_smashketball_type(void);

/** Name of a Remix fighter id as the roster spells it (MARIO, CAPTAIN), or NULL when the id has no roster entry. */
const char *remix_modes_fighter_name(int fkind);

/**
 * Tag Team queue of `port` (0..3): the fighter of slot `slot` (1..5, slot 0 is the character picked in the CSS) as a Remix
 * fighter id, or -1 when the slot is Random.
 */
int remix_modes_tagteam_slot(int port, int slot);

#ifdef __cplusplus
}

#include <cstdint>
#include <map>
#include <string>
#include <vector>

/** CVar names of the Tag Team queue slots 1..5 of the four ports, in port order. */
std::vector<std::string> remix_modes_tagteam_cvars();

/** Combo entries of a Tag Team slot: 0 Random, then the fighters of the Remix CSS in grid order. */
std::map<int32_t, const char *> remix_modes_tagteam_fighters();
#endif

#endif /* PORT_REMIX_REMIX_MODES_H */
