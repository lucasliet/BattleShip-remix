# Remix audio, milestone M3 (2026-10-16)

| Task | Result | Code |
|---|---|---|
| T3.1 BGM | stage music, victory music per fighter, results scene | `mnremixwinbgm.c`, `mnvsresults.c`, `grremixstage.c` |
| T3.2 SFX and voices | new FGM ids (0x2B7..0x608) play, none out of the table | `audio_bridge.cpp` blobs, `n_env.c` log, `lbcommon.c` pan |
| T3.3 Sound test | 375 songs, 1094 voices | `mnsoundtest.c`, `mnremixsoundtest.c` |

## Findings worth keeping

- The Torch `REMIXAUDIO` blobs already work: `audio/fgm_tbl` (1288 entries) and `audio/fgm_ucd` (1545) are self-contained (every
  offset lies inside the blob, the moved and new blocks are appended after the grown pointer table), so no runtime image lookup
  is needed. Do not add one.
- **Two FGM play functions exist in `n_env.c`**: `func_800269C0_275C0` is the one the game calls (fighter motion events, items,
  menus); `func_80026A10_27610` is a sibling. A trace placed in the wrong one reads as "SFX are mute"; use lldb
  (`breakpoint set -n func_800269C0_275C0`) before concluding that nothing plays.
- Remix keeps music ids 0-based in `Stages.alternate_music_table` and 1-based in `default_music_track_table`
  (`grRemixGetStageBGM` subtracts 1); the winner music table holds code addresses (`add_victory_bgm`), whose `ori a1` carries the id.
- The sound test selector shows `id + 1`; Remix did not widen the digit objects (vanilla has three), the port makes four.
- A VS shortcut with no `SSB64_VS_STOCKS` runs as a demo game type; use `SSB64_VS_STOCKS=<n>` when the run must reach the
  results scene. The results scene itself renders white in that shortcut, with vanilla fighters too (checked with `res_0`), so its
  visuals belong to ROADMAP T5.3.

## Harness

`SSB64_AUDIO_DUMP_PATH`, `SSB64_AUDIO_DUMP_SECONDS`, `SSB64_AUDIO_FGM_LOG=1`, `SSB64_VS_STOCKS`; `vs.sh` writes `<run>/audio.wav`;
`scripts/wavstat.py` prints RMS per second. Sound test scene: `SSB64_START_SCENE=59` with `inputs/soundtest_*.txt`.

## Open

BGM volume hook, FGM volume hook, Japanese voices, `entry_stereo_fix`, menu music choice: toggles (M4). `SFXReplace.asm`
(footsteps and landing) and the Pokemon announcer (T2.5) are still to do.
