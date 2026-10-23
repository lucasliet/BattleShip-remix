/**
 * remix_overlay.h - short text messages drawn over the game (ImGui toast of the engine overlay).
 */
#ifndef PORT_REMIX_OVERLAY_H
#define PORT_REMIX_OVERLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shows text over the game for the given time. libultraship's GameOverlay draws it every frame inside the ImGui
 * pass. Does nothing before the window and its GUI exist.
 *
 * @param text Text to show, copied by the overlay.
 * @param seconds Time on screen before the text fades out.
 */
void remix_overlay_show_text(const char *text, float seconds);

#ifdef __cplusplus
}
#endif

#endif
