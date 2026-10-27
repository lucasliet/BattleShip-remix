/**
 * remix_debug.h - debug overrides for exercising Remix fighters without input.
 *
 * SSB64_DEMO_FKINDS="a,b" forces the attract-mode demo fighters (port fkinds)
 * so a synth fighter can be put in a CPU match from boot.
 *
 * SSB64_VS_FKINDS="a,b" boots straight into a VS battle with a human P1 (fkind a)
 * against a CPU P2 (fkind b), so SSB64_INPUT_SCRIPT can drive P1's specials.
 */
#ifndef PORT_REMIX_DEBUG_H
#define PORT_REMIX_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Returns the forced demo fkind for slot 0 or 1, or `fallback` when the
 * override is unset, malformed, or names an unregistered fighter.
 */
int remix_debug_demo_fkind(int slot, int fallback);

/**
 * Returns the SSB64_VS_FKINDS fkind for slot 0 or 1, or -1 when the override
 * is unset, malformed, or names an unregistered fighter.
 */
int remix_debug_vs_fkind(int slot);

/**
 * Returns the SSB64_VS_ITEMS item appearance rate (0 = most frequent) that turns
 * every item switch on in the SSB64_VS_FKINDS shortcut, or -1 when unset.
 */
int remix_debug_vs_item_rate(void);

/**
 * Returns the SSB64_REMIX_FORCE_ITEM kind that replaces every item the stage
 * spawner makes, or -1 when unset.
 */
int remix_debug_force_item_kind(void);

/**
 * Returns 1 when SSB64_REMIX_FORCE_ITEM_AT_P1 is set: the stage spawner drops the
 * item on player 1 instead of a random item point, so a pickup test is deterministic.
 */
int remix_debug_force_item_at_p1(void);

/**
 * Returns the integer value of the environment variable `name`, or `fallback` when it is unset or empty. The smoke tests
 * use it for the mode shortcuts (SSB64_VS_STOCKS, SSB64_VS_TIME, SSB64_VS_STAMINA, SSB64_KOTH_GOAL, SSB64_KOTH_LINE,
 * SSB64_SMASH_GOAL, SSB64_SMASH_BALL_X).
 */
int remix_debug_env_int(const char *name, int fallback);

/**
 * Returns the text of the environment variable `name`, or NULL when it is unset or empty (SSB64_REMIX_MODE,
 * SSB64_SMASH_TYPE). The pointer stays valid for the life of the process.
 */
const char *remix_debug_env_text(const char *name);

#ifdef __cplusplus
}
#endif

#endif
