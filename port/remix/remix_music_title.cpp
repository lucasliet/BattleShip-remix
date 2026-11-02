/*
 * remix_music_title.cpp - looks up the Remix track titles (rows generated from the Smash Remix source by
 * tools/gen_remix_music_titles.py) and hands the banner text to the overlay.
 */
#include "remix/remix_music_title.h"

#include <cstddef>
#include <cstdio>

#include "remix/remix_overlay.h"

extern "C" {
#include "port_log.h"
}

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

extern "C" void remix_music_title_show(int32_t bgm_id, float seconds) {
    const RemixMusicTitleRow *row;
    char line[256];

    if (bgm_id < 0) {
        return;
    }
    row = FindRemixMusicTitleRow(static_cast<uint32_t>(bgm_id));
    if (row == nullptr) {
        port_log("SSB64: music title: bgm %d has no title\n", static_cast<int>(bgm_id));
        return;
    }
    std::snprintf(line, sizeof(line), "%s - %s", row->track, row->game);
    port_log("SSB64: music title: bgm %d \"%s\"\n", static_cast<int>(bgm_id), line);
    remix_overlay_show_text(line, seconds);
}
