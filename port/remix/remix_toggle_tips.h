/**
 * remix_toggle_tips.h - the player-facing tips shown on the Remix settings pages of the port menu.
 */
#ifndef PORT_REMIX_REMIX_TOGGLE_TIPS_H
#define PORT_REMIX_REMIX_TOGGLE_TIPS_H

#ifdef __cplusplus

#include "remix_toggles.h"

/**
 * Tip of a Remix setting, written for players who do not know the Remix. The fixed settings of every page have their own text;
 * the per-track Random Music, per-stage Random Stage and per-Pokemon toggles share one generic text per page.
 *
 * @param toggle Setting of the Remix Settings, Gameplay, Music, Stage or Pokemon page.
 * @return Static tip text, nullptr when the setting has no tip.
 */
const char *remix_toggle_tip(const remix_toggles::Toggle &toggle);

#endif

#endif /* PORT_REMIX_REMIX_TOGGLE_TIPS_H */
