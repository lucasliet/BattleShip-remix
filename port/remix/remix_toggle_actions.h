/**
 * remix_toggle_actions.h - the A-press routines of the Remix settings (Toggles.asm), run by the port menu when a value changes.
 *
 * In the asm a setting with an A-press routine applies or previews its value when A is pressed on it: BGM Volume, SFX Volume,
 * Punish Failed Z-Cancel, Default CPU LVL (V.S.), Menu Music and the per-track Random Music toggles (a row each), plus the title
 * rows that flip a whole list (toggle_all_) and the "Load Profile:" rows (load_sub_profile_). The port menu has no A press on a
 * setting, so it runs the same routine after each change and gives the title and profile rows a button.
 */
#ifndef PORT_REMIX_REMIX_TOGGLE_ACTIONS_H
#define PORT_REMIX_REMIX_TOGGLE_ACTIONS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Runs the A-press routine of a setting after its value changed; does nothing for a setting without one.
 *
 * @param entry_addr REMIX_SYM_Toggles__entry_* address of the setting.
 */
void remix_toggle_action_apply(uint32_t entry_addr);

/**
 * Called when the port menu closes: puts back the BGM that was playing before the first Random Music preview (or stops the
 * music when none was), and does nothing when no preview ran. Deviation from the asm, which restarts the menu music through
 * reset_menu_music when the settings screen is left.
 */
void remix_toggle_action_menu_closed(void);

/**
 * Toggles.asm toggle_all_: sets every toggle after the title row to the opposite of the first one.
 *
 * @param title_addr titleAddr of a remix_toggles::ToggleAllRow.
 */
void remix_toggle_action_toggle_all(uint32_t title_addr);

/**
 * Toggles.asm load_sub_profile_: copies a profile of music_profiles (Music page) or stage_profiles (Stage page) into the toggles
 * after the title that follows the "Load Profile:" row. Does nothing for an unknown row or a profile out of its range.
 *
 * @param loader_entry_addr loaderEntryAddr of a remix_toggles::ToggleAllRow.
 * @param profile Profile number, 0 to the maximum of the loader row (its string table lists the names).
 */
void remix_toggle_action_load_sub_profile(uint32_t loader_entry_addr, int profile);

#ifdef __cplusplus
}
#endif

#endif
