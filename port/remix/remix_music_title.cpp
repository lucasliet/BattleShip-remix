/*
 * remix_music_title.cpp - looks up the Remix track titles (rows generated from the Smash Remix source by
 * tools/gen_remix_music_titles.py) and serves them to the match banner and the pause legend.
 */
#include "remix/remix_music_title.h"

#include <cstddef>

namespace {

struct RemixMusicTitleRow {
    uint32_t bgm_id;
    const char *track;
    const char *game;
};

const RemixMusicTitleRow kRemixMusicTitleRows[] = {
#include "remix_music_titles_data.inc"
};

/**
 * Finds the title row of a Remix BGM id.
 *
 * @param bgm_id Remix BGM id.
 * @return The row, or NULL when the id has no title.
 */
const RemixMusicTitleRow *FindRemixMusicTitleRow(uint32_t bgm_id) {
    for (const RemixMusicTitleRow &row : kRemixMusicTitleRows) {
        if (row.bgm_id == bgm_id) {
            return &row;
        }
    }
    return nullptr;
}

} // namespace

extern "C" int remix_music_title_get(int32_t bgm_id, const char **track, const char **game) {
    const RemixMusicTitleRow *row;

    if (bgm_id < 0) {
        return 0;
    }
    row = FindRemixMusicTitleRow(static_cast<uint32_t>(bgm_id));
    if (row == nullptr) {
        return 0;
    }
    if (track != nullptr) {
        *track = row->track;
    }
    if (game != nullptr) {
        *game = row->game;
    }
    return 1;
}
