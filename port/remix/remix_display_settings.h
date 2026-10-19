/**
 * remix_display_settings.h - Smash Remix display settings (Toggles.asm) mapped to the port renderer and Stats window.
 */

#ifndef PORT_REMIX_REMIX_DISPLAY_SETTINGS_H
#define PORT_REMIX_REMIX_DISPLAY_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Applies the Disable Anti-Aliasing and FPS Display toggles to the window. Called once per frame, so a change in the
 * Remix menu takes effect on the next frame, and the MSAA slider cannot override Disable Anti-Aliasing.
 */
void remix_display_settings_sync(void);

#ifdef __cplusplus
}
#endif

#endif /* PORT_REMIX_REMIX_DISPLAY_SETTINGS_H */
