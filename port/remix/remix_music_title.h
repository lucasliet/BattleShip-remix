/**
 * remix_music_title.h - the track and game name of a Remix BGM id, shown on the match start banner and the pause legend.
 */
#ifndef PORT_REMIX_MUSIC_TITLE_H
#define PORT_REMIX_MUSIC_TITLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Looks up the track and game names of a Remix BGM id (the strings of BGM.asm string_table).
 *
 * @param bgm_id Remix BGM id.
 * @param track Receives the track name (static storage); may be NULL.
 * @param game Receives the game name (static storage); may be NULL.
 * @return 1 when the id has a title, 0 when it has none (the outputs are then untouched).
 */
int remix_music_title_get(int32_t bgm_id, const char **track, const char **game);

#ifdef __cplusplus
}
#endif

#endif
