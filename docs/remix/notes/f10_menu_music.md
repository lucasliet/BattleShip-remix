# F10 menu music hooks (ROADMAP T5.3)

Source: `.toolchain/pkgs/smashremix-src/src/BGM.asm`, `alt_menu_music_` (`:492`) and its callers, plus `_prevent_stop`.
Helper: `decomp/src/gm/gmremixmenumusic.c` (owned by this task). Port id reads: `syAudioGetBGMPlaying` (new, `sys/audio.c`,
guarded by `PORT && REGION_REMIX`), the port equivalent of `BGM.safe_id`.

## Hook map (Remix RAM / ROM address, screen, vanilla site)

| Remix hook | Screen | Vanilla function (site) | Hook |
|---|---|---|---|
| `0x80132B1C` / `0x11DAAC` | mode select, title entry | `mnModeSelectFuncStart` (`mnmodeselect.c:1003`) | `gmRemixMenuMusicPlay(ModeSelect, scene_prev == nSCKindTitle)` |
| `0x801335A8` / `0x120D58` | options, back from screen adjust | `mnOptionFuncStart` (`mnoption.c:1160`) | `gmRemixMenuMusicPlay(ModeSelect, FALSE)` |
| `0x80132EA8` / `0x1222F8` | data screen, back from sound test | `mnDataFuncStart` (`mndata.c:884`) | `gmRemixMenuMusicPlay(ModeSelect, FALSE)` |
| `0x80134740` / `0x1250F0` | VS mode, back from VS CSS | `mnVSModeFuncStart` (`mnvsmode.c:1814`) | `gmRemixMenuMusicPlay(ModeSelect, FALSE)` |
| `0x80133008` / `0x11F118` | 1P mode, back from 1P/training/bonus CSS | `mn1PModeFuncStart` (`mn1pmode.c:956`) | `gmRemixMenuMusicPlay(ModeSelect, FALSE)` |
| `0x80138534` / `0x140734` | 1P CSS | `mnPlayers1PGameFuncStart` (`mnplayers1pgame.c:3862`) | `gmRemixMenuMusicPlay(BattleSelect, FALSE)` |
| `0x80137ED4` / `0x1474B4` | training CSS | `mnPlayers1PTrainingFuncStart` (`mnplayers1ptraining.c:3570`) | `gmRemixMenuMusicPlay(BattleSelect, FALSE)` |
| `0x80136ED8` / `0x14CF08` | bonus CSS | `mnPlayers1PBonusFuncStart` (`mnplayers1pbonus.c:3213`) | `gmRemixMenuMusicPlay(BattleSelect, FALSE)` |
| `0x8013B2F8` / `0x139578` | VS CSS | `mnPlayersVSFuncStart` (`mnplayersvs.c:5777`) | already hooked before this task, unchanged |
| `0x80138120` / `0x1363A0` | back from VS CSS (`_prevent_stop`) | `mnPlayersVSBackToVSMode` (`mnplayersvs.c:3850`, the second `syAudioStopBGMAll`) | `gmRemixMenuMusicStop()` |
| `0x80136CDC` / `0x13EEDC` | back from 1P CSS (`_prevent_stop`) | `mnPlayers1PGameBackTo1PMode` (`mnplayers1pgame.c:2924`) | `gmRemixMenuMusicStop()` |
| `0x801357F0` / `0x144DD0` | back from training CSS (`_prevent_stop`) | `mnPlayers1PTrainingBackTo1PMode` (`mnplayers1ptraining.c:2325`) | `gmRemixMenuMusicStop()` |
| `0x80135784` / `0x14B7B4` | back from bonus CSS (`_prevent_stop`) | `mnPlayers1PBonusBackTo1PMode` (`mnplayers1pbonus.c:2324`) | `gmRemixMenuMusicStop()` |

The ROM addresses in the decomp `// 0x` comments were not used: `build/remixasm/refs/vanilla_funcs.txt` lists collisions for
these RAM ranges (for example `0x80132EA8` is `mnVSResultsMakeWallpaper` there). Matches were made by screen, by the
`syAudioPlayBGM` / `syAudioStopBGMAll` call the Remix hook replaces, and by the `scene_prev` condition around that call.

Not hooked, because Remix has no `alt_menu_music_` call there: the sound test entry, the results screens, data
characters and VS record (`nSYAudioBGMData`), the stage select and item switch. `handle_sss_shortcut` (BGM.asm, `scope handle_sss_shortcut`) is
not ported.

## What changed in the helper

- `gmRemixMenuMusicKeepsCurrent`: the "do not restart the track already playing" rule of BGM.asm `_check_current_track` and
  `_finish`. Nothing is kept when no track plays or when a 64 track (ModeSelect or BattleSelect) plays, since BGM.asm
  restarts those. RANDOM, RANDOM CLASSICS and RANDOM ALL keep any other track. DEFAULT keeps a listed menu track. A track
  value keeps its own track only.
- `gmRemixMenuMusicStop`: `_prevent_stop`. Keeps the track when the value is RANDOM / RANDOM CLASSICS / RANDOM ALL or a
  listed menu track plays (BGM.asm list of MELEE to K64, the same 15 ids as `sMenuMusicTrackBGM`).
- `gmRemixMenuMusicChoose` no longer handles OFF (the callers do). RANDOM ALL still plays `default_bgm` (not ported).
- `gmRemixMenuMusicPlay` with OFF calls `syAudioStopBGMAll` (BGM.asm `_menu_music_off`).

## Judgement calls

- Title roll: the mode select hook passes `scene_prev == nSCKindTitle` as `from_title_screen`. BGM.asm comments say the chance
  roll applies "if coming from Title screen", but the hook is a call site, not an origin. Check on screen that entering the
  main menu from Explain or AutoDemo does not roll the 85/5/5/5 chance (it should play the 64 track).
- `mnPlayersVSBackToVSMode` hooks only the general stop (`:3850`). The Classic Co-op branch (`:3841`) keeps the vanilla stop,
  since that path goes to the 1P Mode menu, not VS.

## Known gaps

- The port stores the BGM id after `port_enhancement_shuffle_music`, so with the music shuffler on, `safe_id` compares against
  the shuffled id and a kept track can restart. Not changed here.
- RANDOM ALL is not ported (plays the 64 track). BOMBERMAN 64 and SONIC R values (17, 18) play the 64 track (MIDI ids).
- The title hook was not verified against the title overlay. It is matched to the mode select call only.

## For the lead on screen

Scene ids to start with (`SSB64_START_SCENE`): `nSCKindModeSelect` (main menu and title roll), `nSCKindOption` (screen
adjust return), `nSCKindData` (sound test return), `nSCKindPlayersVS` (VS CSS and VS back), `nSCKind1PMode` (1P menu, back
from CSS), `nSCKind1PGame` / `nSCKind1PTraining` / `nSCKind1PBonus` (CSS). Set Menu Music to 64, MELEE, RANDOM, RANDOM ALL, OFF
and check the track and whether it restarts when entering CSS and returning. The mode select entry from the title is the
main check of the title roll.

## Files touched

- `decomp/src/gm/gmremixmenumusic.c`, `decomp/src/gm/gmremixmenumusic.h`
- `decomp/src/sys/audio.c`, `decomp/src/sys/audio.h` (accessor only)
- `decomp/src/mn/mncommon/mnmodeselect.c`, `decomp/src/mn/mnoption/mnoption.c`, `decomp/src/mn/mndata/mndata.c`,
  `decomp/src/mn/mnvsmode/mnvsmode.c`, `decomp/src/mn/mn1pmode/mn1pmode.c`
- `decomp/src/mn/mnplayers/mnplayers1pgame.c`, `mnplayers1ptraining.c`, `mnplayers1pbonus.c`, `mnplayersvs.c`
- Syntax check: OK for every file above. `mnvsmode.c` failed once while another worker's edit was mid-way, then passed.
