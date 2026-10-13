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

#ifdef __cplusplus
}
#endif

#endif
