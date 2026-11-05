# Remix audio: how the port plays Remix music and sound effects

Covers stage and victory music, the new FGM (sound effect) ids, the sound test, and what to check when "nothing plays". Code:
`decomp/src/mn/mnvsmode/mnremixwinbgm.c`, `mnvsresults.c` (victory music), `decomp/src/gr/grremix/grremixstage.c` (stage BGM),
`port/bridge/audio_bridge.cpp` and `yamls/remix/audio.yml` (`SSB64:REMIXAUDIO` blobs), `decomp/src/libultra/n_audio/n_env.c`
(FGM play), `decomp/src/mn/mndata/mnsoundtest.c`, `mnremixsoundtest.c` (sound test, 375 songs and the extra voices). Test:
`.claude/skills/remix-smoke-tests/` with the variables listed under "Harness". Ported menu music, announcer and SFX replace
live in `gmremixmenumusic.c`, `ifremixannouncer.c` and `ftremixsfxreplace.c`.

## Findings worth keeping

- The Torch `REMIXAUDIO` blobs `fgm_tbl` and `fgm_ucd` are self-contained: every offset lies inside the blob, and the moved
  and new blocks are appended after the grown pointer table. No runtime image lookup is needed for FGM data; do not add one.
- **Two FGM play functions exist in `n_env.c`.** `func_800269C0_275C0` is the one the game calls (fighter motion events, items,
  menus); `func_80026A10_27610` is a sibling. A trace placed in the wrong one reads as "SFX are mute". Break on
  `func_800269C0_275C0` in lldb before concluding that nothing plays.
- Remix keeps music ids 0-based in `Stages.alternate_music_table` and 1-based in `default_music_track_table`
  (`grRemixGetStageDefaultTrack` subtracts 1). The winner music table holds code addresses (`add_victory_bgm`), whose
  `ori a1` carries the id.
- The sound test selector shows `id + 1`. Remix did not widen the number objects (vanilla has three digits); the port uses
  four (`MNSOUNDTEST_NUMBER_DIGITS` in `mnsoundtest.c`).
- A VS shortcut without `SSB64_VS_STOCKS` runs as a demo game type; use `SSB64_VS_STOCKS=<n>` when the run must reach the
  results scene.

## Limits of audio checks

A WAV dump proves that sound is produced and that the requested FGM ids are right, not that it sounds right (instruments,
tempo, crackle). Clipping counted on four dumps stayed at a few samples at full scale out of 1.5 million, all in the opening
burst of a match.

## Harness

`SSB64_AUDIO_DUMP_PATH`, `SSB64_AUDIO_DUMP_SECONDS`, `SSB64_AUDIO_FGM_LOG=1` (logs `[fgm] play id=<n>`; ids >= 695 are Remix
sounds), `SSB64_VS_STOCKS`. `vs.sh` writes `<run>/audio.wav`;
`.toolchain/host/venv/bin/python -I .claude/skills/remix-smoke-tests/scripts/wavstat.py <run>` prints RMS per second. The
sound test scene is `SSB64_START_SCENE=59` with `inputs/soundtest_browse.txt` or `soundtest_wrap.txt`.

Open gaps: `docs/remix/patch_audit.md`, group g9.
