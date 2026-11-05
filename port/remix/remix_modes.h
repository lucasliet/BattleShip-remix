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

/** Sets the Smashketball type (the toggle of the VS character select, Smashketball.asm enable_toggling_mode_): 0 basketball, nonzero soccer. */
void remix_modes_smashketball_set_type(int type);

/** Name of a Remix fighter id as the roster spells it (MARIO, CAPTAIN), or NULL when the id has no roster entry. */
const char *remix_modes_fighter_name(int fkind);

/**
 * Tag Team queue of `port` (0..3): the fighter of slot `slot` (1..5, slot 0 is the character picked in the CSS) as a Remix
 * fighter id, or -1 when the slot is Random.
 */
int remix_modes_tagteam_slot(int port, int slot);

/**
 * Sets the Tag Team queue slot `slot` (1..5) of `port` from the VS character select: the CVar of the port menu takes the
 * fighter when it is in the Remix CSS grid, else Random (the menu cannot name it).
 *
 * @return The fighter the CVar now holds as remix_modes_tagteam_slot reports it, -1 for Random.
 */
int remix_modes_tagteam_set_slot(int port, int slot, int fkind);

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
