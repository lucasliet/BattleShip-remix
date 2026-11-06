/**
 * remix_toggle_actions.cpp - see remix_toggle_actions.h.
 */

#include "remix_toggle_actions.h"

#include "remix_toggles.h"

extern "C" {
#include "remix/remix_ram_image.h"
#include "remix/remix_symbols.h"

extern void syAudioSetBGMVolume(int sngplayer, unsigned int volume);
extern int syAudioGetBGMPlaying(int sngplayer);
extern int syAudioPlayFGM(unsigned int fgm);
extern int syAudioPlayBGM(int sngplayer, unsigned int bgm);
extern void syAudioStopBGMAll(void);
extern void func_80020E64(unsigned int volume);

/* decomp/src/gm/gmremixtoggleactionrules.h, which the port layer cannot include */
extern int gmRemixToggleActionGetPunishFgm(int punish_mode, int menu_music);
extern int gmRemixToggleActionGetToggleAllValue(int first_value);
extern int gmRemixToggleActionGetPreviewRemembered(int remembered_bgm, int playing_bgm);
extern int gmRemixToggleActionGetPreviewRestoreStep(int remembered_bgm);

/* decomp/src/gm/gmremixspawnsetup.h and gmremixmenumusic.h */
extern void gmRemixDefaultCPULevelRefresh(void);
extern void gmRemixMenuMusicAudition(void);
}

namespace {

/* gmremixtoggleactionrules.h GMREMIX_VOLUME_ACTION_LEVEL and GMREMIX_VOLUME_ACTION_TEST_FGM */
constexpr unsigned int kVolumeActionLevel = 0x7800;
constexpr unsigned int kVolumeActionTestFgm = 0x020;

/* gmremixtoggleactionrules.h GMREMIX_PREVIEW_IDLE and GMREMIX_PREVIEW_RESTORE_* */
constexpr int kPreviewIdle = -2;
constexpr int kPreviewRestorePlay = 1;
constexpr int kPreviewRestoreStop = 2;

/* BGM id playing before the first Random Music preview since the port menu opened; kPreviewIdle when none was previewed. */
int sPreviewRememberedBgm = kPreviewIdle;

/* Toggles.asm update_bgm_volume: sets the song volume again; the setting scales it. Only while a song plays, as the BGM player
 * does not exist before the first one. */
void ApplyBgmVolume() {
    if (syAudioGetBGMPlaying(0) < 0) {
        return;
    }
    syAudioSetBGMVolume(0, kVolumeActionLevel);
}

/* Toggles.asm update_fgm_volume: sets the sound volume and plays a test sound. */
void ApplyFgmVolume() {
    func_80020E64(kVolumeActionLevel);
    syAudioPlayFGM(kVolumeActionTestFgm);
}

/* Toggles.asm punish_fgm_: plays the sound of the chosen punishment. */
void PlayPunishFgm() {
    const int fgm = gmRemixToggleActionGetPunishFgm(remix_toggle_get(REMIX_SYM_Toggles__entry_punish_on_failed_z_cancel),
                                                    remix_toggle_get(REMIX_SYM_Toggles__entry_menu_music));
    if (fgm != 0) {
        syAudioPlayFGM(static_cast<unsigned int>(fgm));
    }
}

/* Toggles.asm play_menu_music_: plays the chosen menu track now. The track the user just picked is the music to keep, so a
 * pending preview restore is dropped. */
void AuditionMenuMusic() {
    gmRemixMenuMusicAudition();
    sPreviewRememberedBgm = kPreviewIdle;
}

/* The bool rows of the Music page with an A routine are the per-track Random Music toggles (entry_bool_with_a, preview_bgm_). */
bool IsRandomMusicRow(uint32_t entryAddr) {
    const remix_toggles::Toggle *toggle = remix_toggles::Find(entryAddr);
    return (toggle != nullptr) && toggle->hasAction && toggle->isBool && (toggle->group == remix_toggles::Music);
}

/* Toggles.asm preview_bgm_: plays the BGM id of the Random Music row (Menu.entry word 0x24). The asm sets reset_menu_music so
 * the menu music comes back when the settings screen is left; the port remembers the track playing before the first preview
 * and remix_toggle_action_menu_closed puts it back. */
void PreviewBgm(uint32_t entryAddr) {
    const remix_toggles::Toggle *toggle = remix_toggles::Find(entryAddr);
    if (toggle == nullptr) {
        return;
    }
    sPreviewRememberedBgm = gmRemixToggleActionGetPreviewRemembered(sPreviewRememberedBgm, syAudioGetBGMPlaying(0));
    syAudioPlayBGM(0, toggle->extra);
}

const remix_toggles::ToggleAllRow *FindToggleAllRow(uint32_t titleAddr) {
    for (const auto &row : remix_toggles::ToggleAllRows()) {
        if (row.titleAddr == titleAddr) {
            return &row;
        }
    }
    return nullptr;
}

const remix_toggles::ToggleAllRow *FindLoaderRow(uint32_t loaderEntryAddr) {
    for (const auto &row : remix_toggles::ToggleAllRows()) {
        if ((row.loaderEntryAddr != 0) && (row.loaderEntryAddr == loaderEntryAddr)) {
            return &row;
        }
    }
    return nullptr;
}

/* Toggles.asm load_sub_profile_: music_profiles on the Music page, stage_profiles on the other. */
uint32_t ProfileTableOf(remix_toggles::Group group) {
    return (group == remix_toggles::Music) ? REMIX_SYM_Toggles__music_profiles : REMIX_SYM_Toggles__stage_profiles;
}

} // namespace

extern "C" void remix_toggle_action_apply(uint32_t entry_addr) {
    switch (entry_addr) {
        case REMIX_SYM_Toggles__entry_bgm_volume:
            ApplyBgmVolume();
            break;
        case REMIX_SYM_Toggles__entry_fgm_volume:
            ApplyFgmVolume();
            break;
        case REMIX_SYM_Toggles__entry_punish_on_failed_z_cancel:
            PlayPunishFgm();
            break;
        case REMIX_SYM_Toggles__entry_default_cpu_level:
            gmRemixDefaultCPULevelRefresh();
            break;
        case REMIX_SYM_Toggles__entry_menu_music:
            AuditionMenuMusic();
            break;
        default:
            if (IsRandomMusicRow(entry_addr)) {
                PreviewBgm(entry_addr);
            }
            break;
    }
}

extern "C" void remix_toggle_action_menu_closed(void) {
    const int step = gmRemixToggleActionGetPreviewRestoreStep(sPreviewRememberedBgm);
    const int rememberedBgm = sPreviewRememberedBgm;
    sPreviewRememberedBgm = kPreviewIdle;
    if (step == kPreviewRestorePlay) {
        syAudioPlayBGM(0, static_cast<unsigned int>(rememberedBgm));
    } else if (step == kPreviewRestoreStop) {
        syAudioStopBGMAll();
    }
}

extern "C" void remix_toggle_action_toggle_all(uint32_t title_addr) {
    const remix_toggles::ToggleAllRow *row = FindToggleAllRow(title_addr);
    if ((row == nullptr) || row->followers.empty()) {
        return;
    }
    const int value = gmRemixToggleActionGetToggleAllValue(remix_toggle_get(row->followers.front()));
    for (const uint32_t follower : row->followers) {
        remix_toggle_set(follower, value);
    }
}

extern "C" void remix_toggle_action_load_sub_profile(uint32_t loader_entry_addr, int profile) {
    const remix_toggles::ToggleAllRow *row = FindLoaderRow(loader_entry_addr);
    const remix_toggles::Toggle *loader = remix_toggles::Find(loader_entry_addr);
    if ((row == nullptr) || (loader == nullptr) || (profile < loader->min) || (profile > loader->max)) {
        return;
    }
    const uint32_t slot = ProfileTableOf(row->group) + static_cast<uint32_t>(profile) * 4;
    const uint32_t defaults = remix_ram_image_read_word(slot);
    if (defaults == 0) {
        return;
    }
    for (size_t i = 0; i < row->followers.size(); i++) {
        const uint32_t value = remix_ram_image_read_word(defaults + static_cast<uint32_t>(i) * 4);
        remix_toggle_set(row->followers[i], static_cast<int>(value));
    }
}
