/**
 * remix_music_title.h - the track and game name of a Remix BGM id, shown as a message when a match starts.
 */
#ifndef PORT_REMIX_MUSIC_TITLE_H
#define PORT_REMIX_MUSIC_TITLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shows "track - game" for a Remix BGM id through remix_overlay_show_text. Does nothing when the id has no title in
 * the Remix source (menu tracks and the extra MIDIs), or when bgm_id is negative (no track playing).
 *
 * @param bgm_id Remix BGM id, the value of syAudioGetBGMPlaying.
 * @param seconds Time on screen.
 */
void remix_music_title_show(int32_t bgm_id, float seconds);

#ifdef __cplusplus
}
#endif

#endif
