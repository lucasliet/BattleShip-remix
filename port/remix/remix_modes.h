/**
 * remix_modes.h - options of the Smash Remix VS modes that the Remix draws inside its menus (the CSS toggle of
 * Smashketball, King of the Hill goal). The port keeps them as CVars edited in the port menu (Remix > Modes).
 */

#ifndef PORT_REMIX_REMIX_MODES_H
#define PORT_REMIX_REMIX_MODES_H

#define REMIX_MODES_CVAR_SMASHKETBALL_TYPE "gRemix.Modes.SmashketballType"

#ifdef __cplusplus
extern "C" {
#endif

/** Smashketball type: 0 basketball (Smashketball stage), 1 soccer (Soccer stage). */
int remix_modes_smashketball_type(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_REMIX_REMIX_MODES_H */
