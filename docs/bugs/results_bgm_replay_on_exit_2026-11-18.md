# VS results BGM could replay during the screen exit and leak onto the CSS (2026-11-18)

Found by the independent re-audit of the patch ledger against the Remix 2.0.1 asm (2026-11-18), scope
`prevent_results_bgm_on_css_` (`BGM.asm` 1407-1417), which the ledger had closed as `NA (medium confidence)` with a
mechanism that did not hold.

## Symptoms

On the N64, exiting the VS results screen with Menu Music set to a Random value could keep the results music playing on
the character select: the screen's audio thread restarts the results BGM right as the screen tears down, and the RANDOM
menu music rule keeps any track that is not a menu/CSS/Data theme. The Remix fixes it by adding `screen_interrupt` to
the thread's replay condition so the replay is skipped while the screen exits.

## Root cause

- The port's `mnVSResultsAudioThreadUpdate` (`mn/mnvsmode/mnvsresults.c`) was vanilla: on the exit frame the results
  exit path (`mnVSResultsFuncRun`) runs `syAudioStopBGMAll()` and `syTaskmanSetLoadScene()`, but the latter only sets a
  flag (`sys/taskman.c` 971) — the frame finishes, the audio thread wakes once more before the scene ejects it and, if
  it observes `AL_STOPPED`, replays `nSYAudioBGMResults`. The CSS then keeps it (`gmRemixMenuMusicRulesRandomStep`
  returns KEEP for any non-menu track).
- The ledger's `NA` reason ("the scene change ejects it, there is no screen_interrupt race to guard") is wrong about the
  mechanism: ejection happens only after the exit frame completes, one thread wake later — the guard's window does
  exist in the taskman semantics.
- Empirically, however, the race does not reproduce on the PC: the bridge's BGM stop does not surface `AL_STOPPED` to
  the thread within the exit frame, so even the unfixed binary never replays. Verified with an A/B smoke run
  (`bgmgap_before` / `bgmgap_after`, 1-stock match into the results screen, START exit, `SSB64_BGM_LOG=1`): both logs
  show exactly one results-track play and the CSS starting its own screen track; no replay in either.

## Fix

The asm guard is ported anyway, as defence against audio backends whose stop lands synchronously (the other platforms'
bridges differ), and because it is the reference behavior: pure rule
`gmRemixMenuMusicRulesResultsBgmShouldReplay` (`gm/gmremixmenumusicrules.c`) — replay only when the player stopped and
the screen is not exiting — applied in `mnVSResultsAudioThreadUpdate` under `PORT && REGION_REMIX`, with the exit
signal read from the results screen's own exit path (`scene_curr` already moved off `nSCKindVSResults` before the
`syAudioStopBGMAll` call, the same ordering the asm relies on). No `screen_interrupt` global exists in the port and
none is needed.

## Verification

- Unit test `testResultsBgmReplaysOnlyWhileTheScreenRuns` in `tests/remix/test_menu_music_rules.c`; `run_remix_mode_tests`
  passes (8520 checks, 0 failed).
- `ssb64` builds clean.
- Smoke A/B above: identical, correct BGM sequences before and after (stage track → stop → fanfare → results track →
  stop at exit → CSS screen track; no results replay, no results track kept on the CSS).
