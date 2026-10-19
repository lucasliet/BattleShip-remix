# F8 menu music: Menu Music and Music Title at Match Start

Source: `.toolchain/pkgs/smashremix-src/src/BGM.asm` (`alt_menu_music_` 572-800, `handle_sss_shortcut` 876-930,
`show_music_title_` 981-1130), `Toggles.asm` (`play_menu_music_` 664-795, menu_music strings 1414-1446, entries 2462-2463).

## Toggles

| Title (`SSB64_REMIX_TOGGLES` name) | Asm | Range | Defaults CE/TE/NE/JP |
|---|---|---|---|
| `Menu Music` | `entry_menu_music` | 0 to 22 (`menu_music.MAX_VALUE` = 22 = OFF) | 0 / 0 / 1 / 0 |
| `Music Title at Match Start` | `entry_show_music_title` | bool, 0 or 1 | 1 / 0 / 1 / 1 |

Values of `Menu Music`: 0 DEFAULT, 1 64, 2 MELEE, 3 MELEE MENU 2, 4 BRAWL, 5 GOLDENEYE 007, 6 MARIO TENNIS,
7 SUPER MARIO 64, 8 BLAST CORPS, 9 DIDDY KONG RACING, 10 MARIO KART 64, 11 SNOWBOARD KIDS, 12 MARIO PARTY,
13 MARIO ARTIST: TALENT STUDIO, 14 PERFECT DARK, 15 CRASH TEAM RACING, 16 KIRBY 64, 17 BOMBERMAN 64, 18 SONIC R,
19 RANDOM, 20 RANDOM CLASSICS, 21 RANDOM ALL, 22 OFF.

To test: `SSB64_REMIX_TOGGLES="Menu Music=<value>"` (Music Title: `"Music Title at Match Start=1"`).

## Ported

- `gmRemixMenuMusicPlay(default_bgm, from_title_screen)` in `decomp/src/gm/gmremixmenumusic.c` (+ `.h`).
  - Value 22 (OFF): `syAudioStopBGMAll()`.
  - Value 0 (DEFAULT): the vanilla BGM. The title screen chance roll (85 percent default, 5 percent each MELEE,
    MENU 2, BRAWL) runs only when `from_title_screen` is TRUE.
  - Value 1 (64): the vanilla BGM.
  - Values 2 to 16: the Remix BGM ids of `menu.MAIN_*` (MELEE 71, BRAWL 108, MENU2 224, GOLDENEYE 231, MARIOTENNIS 248,
    FILESELECT_SM64 259, BLASTCORPS 262, DKR 280, MK64 283, SBK 296, MARIOARTIST 307, MARIOPARTY 309, PERFECTDARK 312,
    CTR 319, K64 338).
  - Values 19 (RANDOM, uniform 1 to 18) and 20 (RANDOM CLASSICS, uniform 5 to 18): the picked value goes through the
    same table. A pick of 1 (64) keeps the vanilla BGM.
  - Values 17 and 18 (BOMBERMAN, SONIC R): `REMIX-TODO(MIDI ids)`, the vanilla BGM. Their ids are assembly-time
    values (`insert_extra_midi(STAGESELBM64)`, `insert_extra_midi(SONIC_R)`, MIDI.asm 681-683), with no symbol in
    `remix_symbols.h`.
  - Value 21 (RANDOM ALL): `REMIX-TODO(random list)`, the vanilla BGM. The asm picks from `random_music_` without the
    Random Music gate, and `grRemixRandomMusicPick` returns early when Random Music is off.
- Random picks use `syUtilsRandIntRange`, the port's existing random source.

## Patch applied

| # | Asm RAM | Vanilla function (decomp file) | Applied | Port |
|---|---------|--------------------------------|---------|------|
| 1 | 0x8013B2F8 (`alt_menu_music_` hook "VS CSS", `addiu a1, CHARACTER_SELECT`) | `mnPlayersVSFuncStart` (`decomp/src/mn/mnplayers/mnplayersvs.c`, `if (scene_prev != nSCKindMaps)` block) | yes | `syAudioPlayBGM(0, nSYAudioBGMBattleSelect)` becomes `gmRemixMenuMusicPlay(nSYAudioBGMBattleSelect, FALSE)` under `#ifdef REGION_REMIX` |

The VS CSS call site is the only one matched with confidence: `mnPlayersVSFuncStart` starts at 0x8013B0C8 in
`build/remixasm/refs/vanilla_funcs.txt`, and the play call is in its body. With Menu Music 0 the call is the same as
vanilla (`FALSE` skips the title roll), so toggle-off behaviour is unchanged.

## Skipped / REMIX-TODO

- The other 12 `alt_menu_music_` hooks (RAM 0x80132B1C title, 0x80133008, 0x801335A8, 0x80132EA8, 0x80134740 VS
  results/adjust/sound test/VS main menu, 0x80138534 1P CSS, 0x80137ED4 Training CSS, 0x80136ED8, 0x80138120,
  0x801357F0, 0x80135784, and the other 1P/Bonus CSS sites). The RAM addresses do not map one-to-one to the decomp call
  sites in `vanilla_funcs.txt` (the table lists function starts only, and several hooks sit in functions the map
  attributes to a different name). Applying them by guess could patch the wrong screen. Needs the build map
  (`build/*.map`, or `build/remixasm/remix.sym` for the Remix side) to confirm each call site. Until then, Menu Music
  applies only on the VS CSS, and the title and main menu keep the vanilla BGM.
- Title screen chance roll: the helper supports it (`from_title_screen`), but no call site passes TRUE yet (see above).
- `_prevent_stop` hooks (0x1363A0, 0x13EEDC, 0x144DD0, 0x14B7B4): the asm keeps the menu track when going back from a CSS
  (no `BGM.stop_` for a track other than OFF). The port stops the BGM on the way back, as vanilla does. Behaviour gap.
- `BGM.safe_id` ("do not restart the track already playing"): the port has no current-BGM tracker for menus, so every
  start restarts the track. `REMIX-TODO(safe_id)`.
- `handle_sss_shortcut` (876-930): the SSS shortcut refresh uses `BGM.safe_id` and the title font. Not ported.
- `show_music_title_` (981-1130): `REMIX-TODO(Render)`. The popup draws a rectangle (`Render.draw_rectangle`), the track
  name with `Render.draw_string_`, and destroys the objects on `GO!`. The port has no music title object, and the
  name strings come from the Remix RAM image (`Toggles.menu_music` / BGM names), not copied into the source. Hooks
  0x8E00C (`ifCommonCountdownMakeInterface`, `if/ifcommon.c`) and 0x8D900 (`ifCommonAnnounceGoMakeInterface`) are the
  two call sites. `Music Title at Match Start` has no effect yet.

## What to see with Menu Music on (lead)

- `Menu Music=22` (OFF): on the VS character select, the BGM stops. The vanilla menu BGM still plays on the other screens.
- `Menu Music=2` (MELEE): VS character select plays the MELEE track (BGM 71). Other menus unchanged.
- `Menu Music=19` (RANDOM): VS character select plays a different track each time. A pick of 1 plays the vanilla BGM,
  which is the 64 track, and picks of 17 or 18 also play the vanilla BGM.
- `Menu Music=0` and `Menu Music=1`: VS character select plays the vanilla BGM, as with the toggle off.
- `Music Title at Match Start`: no visible change (REMIX-TODO(Render)).
