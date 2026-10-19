# F8 Music settings (BGM.asm, FGM.asm volume hook)

Scope: `.toolchain/pkgs/smashremix-src/src/BGM.asm` (music settings only) and the FGM volume hook in `FGM.asm`.
Files: `decomp/src/sys/audio.c` (hooks), `decomp/src/gm/gmremixmusic.c/.h` (master volume), `decomp/src/gr/grremix/grremixrandommusic.c/.h`
(Random Music list), `decomp/src/gr/grremix/grremixstage.c` (stage BGM picker now calls the random pick).
Not built, not run. Syntax-checked with `scripts/remix/syntax-check.sh`. The port C++ files (`port/enhancements/MusicSelection.cpp`,
`ShuffleMusic.cpp`, `volume.cpp`) are untouched; `syAudioPlayBGM` still calls `port_enhancement_shuffle_music` after the guard.
New `.c` files need a CMake reconfigure (the source list is a GLOB).

## Toggles

Titles and values are from `Toggles.asm` (CE / TE / NE / JP defaults). Test with `SSB64_REMIX_TOGGLES="<title>=<value>"`.

| Title (Toggles.asm) | Values | Default CE/TE/NE/JP | Read by |
|---|---|---|---|
| `Play Music` | 0 off, 1 on | 1/1/1/1 | `syAudioPlayBGM` (`gmRemixToggle(play_music)`) |
| `Random Music` | 0 off, 1 on | 0/0/1/0 | `grRemixRandomMusicPick` |
| `Congo Jungle`, `Credits`, `Data`, `Dream Land`, `Duel Zone`, `Final Destination`, `How To Play`, `Hyrule Castle`, `Meta Crystal`, `Mushroom Kingdom`, `Peach's Castle`, `Planet Zebes`, `Saffron City`, `Sector Z`, `Yoshi's Island`, `Bonus` | 0 off, 1 on | 1 in all | `grRemixRandomMusicBuildList` (per-track entries) |
| `Salty Runback Preserves Song` | 0 off, 1 on | 0/0/0/0 | `grRemixRandomMusicPickIndex` |
| `BGM Volume` | 0 to 10 | 10 | `syAudioSetBGMVolume` (`gmRemixMasterVolume`) |
| `SFX  Volume` (two spaces, as in `Toggles.asm:2465`) | 0 to 10 | 10 | `func_80020E64` (`gmRemixMasterVolume`) |

Not ported: `Menu Music` (`Toggles.menu_music`, 0 in CE/TE/JP, 1 in NE by default), `Music Title at Match Start` (`1/0/1/1`).

## Patches

| # | Asm RAM | Vanilla function (decomp file) | Applied | Port |
|---|---------|--------------------------------|---------|------|
| 1 | 0x80020B3C (`BGM.master_bgm_volume`, set_volume_ 0x80020B38) | `syAudioSetBGMVolume` (`sys/audio.c`) | yes | `vol = gmRemixMasterVolume(vol, bgm_volume)` before the 30720 clamp (asm: scale, then `sltiu` clamp) |
| 2 | 0x80020E64 (`FGM.master_fgm_volume`) | `func_80020E64` (`sys/audio.c`) | yes | `volume = gmRemixMasterVolume(volume, fgm_volume)` before `vol = ... ? AL_VOL_FULL : volume >> 8` |
| 3 | 0x80020AB4 (`BGM.play_music_`, guard) | `syAudioPlayBGM` (`sys/audio.c`) | yes, except the FD timer | off: `syAudioStopBGMAll()` (asm `stop_`), return -1 |
| 4 | 0x800FC314 (`BGM.apply_alt_or_random_music_` -> `random_music_`) | `mpCollisionInitGroundData` -> `grRemixGetStageBGM` (`gr/grremix/grremixstage.c`) | yes, random part | `grRemixGetStageBGM` = alternate pick, then `grRemixRandomMusicPick` |

Skipped:
- `BGM.play_music_` writes `FD.music_fix_.timer_start` (frame the track started). No reader in the port (no FD music fix in `decomp/src` or `port/`), so the write is skipped.
- `set_safe_id_on_play_` / `set_safe_id_on_stop` (0x80020AF8, 0x80020B30) store the current bgm id for the menu music logic. Not ported with the menu music.
- `alt_menu_music_` and its 13 call sites plus 4 `_prevent_stop` hooks (0x11DAAC..0x14B7B4, in mnvs/mnplayers/title/css functions). REMIX-TODO(MenuMusic): needs the menu state machine and the `menu.*` bgm ids; `Menu Music` is not applied.
- `show_music_title_` hooks (0x8E00C -> `ifCommonCountdownMakeInterface`, 0x8D900 -> `ifCommonAnnounceGoMakeInterface`) and the title strings. REMIX-TODO(Render): needs `Render.draw_rectangle` / `draw_string_` and the font load in `setup_`.
- `thirty_second_music_swap` (0x8E9B0), `remix_1P_ending_song` (0x1827DC), and the results audio wait (0x1569C8): not settings in this scope; left as vanilla.

## Random Music: what is and is not applied

- Applied: the 16 built-in per-track toggles, in the asm list order (`sRandomTracks`), the uniform pick, the empty-list fallback
  (keep the stage BGM), and salty preservation (replays the last pick when `GameEnd.is_salty_runback` is set).
- REMIX-TODO(custom MIDI): the custom tracks (`entry_random_music_sorted_<id>`, 328 symbols in `remix_symbols.h`) are not in the list.
  The asm adds them with the assembly-time id `Toggles.sorted_midi_<n>`, which has no symbol. A generated id table is needed.
- Deviation: `random_disabled` (asm: C-button held and an alternate song chosen) is not modelled. The port's alternate pick
  never read the C buttons, so the random pick always applies. The lead should check Random Music with C held.
- Deviation: asm stores the alternate bgm id in `current_track` too. The port keeps only the last random index, so a preserved
  pick after an alternate song can differ from the asm.
- Deviation: the pause D-pad cycle and Menu Music RANDOM ALL (both bypass the toggle in asm) are not ported.
- `GameEnd.is_salty_runback` is read from the Remix RAM image at 0x8048B174 (`build/remixasm/remix.sym`), because it is missing
  from `remix_symbols.h`. Salty Runback itself is not ported, so the flag is probably 0 and preservation will not show until it is.

## What the lead should see

- `SSB64_REMIX_TOGGLES="BGM Volume=0"`: no BGM at all. `BGM Volume=5`: quieter BGM, sound SFX unchanged. `SFX  Volume=0`: no SFX.
- `SSB64_REMIX_TOGGLES="Play Music=0"`: no stage music from the first frame of a match (the play call stops everything).
- `SSB64_REMIX_TOGGLES="Random Music=1;Congo Jungle=0"`: the stage song changes between matches on the same stage (the list has 15 tracks; Congo Jungle is removed).
  With `Random Music=1` and every per-track toggle at 0 the stage song is the stage default.
- Music titles on the VS start countdown and the menu music changes are not expected: both are not ported.
