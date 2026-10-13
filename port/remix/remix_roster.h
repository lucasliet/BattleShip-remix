/**
 * remix_roster.h - loads the Smash Remix fighter table from the o2r and
 * registers the synth fighters (Remix ids 0x1D..0x60) in the fighter registry.
 */

#ifndef PORT_REMIX_REMIX_ROSTER_H
#define PORT_REMIX_REMIX_ROSTER_H

/**
 * Reads resource "remix/fighters", validates it, and registers one synth
 * fighter per Remix id 0x1D..0x60. Each synth clones its vanilla parent's
 * descriptor and replaces the fields that come from the Remix data.
 *
 * Returns false (logging why) when the resource is missing or malformed.
 * On failure nothing is registered, so the vanilla registry stays intact.
 */
bool remix_roster_load();

#endif /* PORT_REMIX_REMIX_ROSTER_H */
