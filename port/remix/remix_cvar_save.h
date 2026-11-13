/**
 * remix_cvar_save.h - stores a Remix CVar changed by the game and writes the port settings file soon after.
 *
 * The port menu widgets ask the GUI to save the CVars on the next frame; the game-side setters (stage select hazard
 * mode, Training menu, CSS panel, Remix modes, profile loaders) go through this so a change survives a process that is
 * killed instead of closed (the PS5 home button, an Android task swipe).
 */

#ifndef PORT_REMIX_REMIX_CVAR_SAVE_H
#define PORT_REMIX_REMIX_CVAR_SAVE_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Sets the integer CVar `name` to `value`. When the stored value changes, asks the GUI to write the CVars on the next
 * frame (several changes in one frame give one write; no write without a GUI).
 */
void remix_cvar_set_saved(const char *name, int value);

#ifdef __cplusplus
}
#endif

#endif /* PORT_REMIX_REMIX_CVAR_SAVE_H */
