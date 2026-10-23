# F10 notes: VS results extras (transitions, stats tracking, full results)

Scope: `decomp/src/mn/mnvsmode/mnvsresults.c` (scene nSCKindVSResults), with the Remix asm of `VsStats.asm`,
`Transitions.asm`, `TimedStock.asm` (show_full_results_, 0x801376E4) and `GameEnd.asm`/`VsRemixMenu.asm`
(add_row_to_results_, 0x80136338 and neighbours). Built and run by the lead; nothing was built or run here.

## (c) Transitions (done)

- `decomp/src/lb/lbtransition.h`: the descriptor count is `LB_TRANSITION_DESC_COUNT`, 13 under REGION_REMIX and 11 otherwise.
  `dLBTransitionDescs[LB_TRANSITION_DESC_COUNT]`. The VS results pick (`mnvsresults.c` ~3540,
  `syUtilsRandIntRange(ARRAY_COUNT(dLBTransitionDescs))`) and the size scan in `lbTransitionSetupTransition` pick up the
  new entries with no other change.
- `decomp/src/lb/lbtransition.c`: two entries appended under `#ifdef REGION_REMIX` (Transitions.asm restored entries):
  - file 0x27 (reloc file 39, the `39_IFCommonObject.c` tables are the falling shapes data: DObjDesc at 0x3BC0, the
    comment in that file puts the DObjDesc array at 0x3BC0 and the terminator at 0x3D68), offsets 0x3BC0 / 0x3D78.
  - file 0x2F (reloc file 47), offsets 0x0F98 / 0x101C (same offsets as the vanilla Rotating Frame entry, as the asm says).
  - Offsets are integer literals, the same form as the generated `llLBTransition*DObjDesc` macros in `include/reloc_data.us.h`.
- NOT appended: Smash Logo (0x1262) and Remix Logo (0x1366). Those ids are Remix ids above `RELOC_FILE_COUNT` (2132) in
  `port/resource/RelocFileTable.us.h`, so `lbRelocGetExternHeapFile` cannot load them and a random pick would hit NULL.
  REMIX-TODO(results-transition-remix-logo) is in the file.
- Vanilla (no REGION_REMIX) is unchanged: 11 entries, same extern size.

## (a) VS stats tracking (tracking only, drawing not ported)

- New `decomp/src/mn/mnvsmode/mnremixvsstats.{c,h}`: per-port counts of USP/NSP/DSP (`nMNRemixVSStatsSpecial*`), reset,
  count, get count, and get percent (`part * 100 / total` of the port, same formula as `calculate_tracker_percentage_`).
- Hooks (all `#ifdef REGION_REMIX`, one counting line before each accepted dispatch, `fp->player` as the port):
  - `decomp/src/ft/ftcommon/ftcommonspecialhi.c` ground Up (asm 0xCBBF4, `ftCommonSpecialHiCheckInterruptCommon`).
  - `decomp/src/ft/ftcommon/ftcommonspecialn.c` ground Neutral (asm 0xCBB74, `ftCommonSpecialNCheckInterruptCommon`,
    only the neutral-B PASS branch; the Kirby copy branch above it is not counted).
  - `decomp/src/ft/ftcommon/ftcommonspeciallw.c` ground Down (asm 0xCBC78, `ftCommonSpecialLwCheckInterruptCommon`).
  - `decomp/src/ft/ftcommon/ftcommonspecialair.c` aerial Up (asm 0xCB9C8), aerial Down (0xCBA0C), aerial Neutral (0xCBA78),
    at the AIR_HI, AIR_LW and AIR_N dispatches. The asm's aerial hook addresses are in `docs/remix/gameplay_m_z.md` rows 827-832.
- Reset: `decomp/src/sc/sccommon/scvsbattle.c` `scVSBattleStartBattle`, first statement under REGION_REMIX.
- Not done (REMIX-TODO(results-text)): the stats page drawing (`draw_port_headers_`, `draw_underline_`, `draw_line_`,
  `run_collect_`, `setup_`). `port/remix/remix_overlay.h` (written by another worker) is `remix_overlay_show_text`, an
  ImGui toast for transient messages, not a results-page layout, so it is not used for stats text.
- Not done (REMIX-TODO(results-stats-trackers)): Z-cancel, tech and ledge trackers. Their hooks are not in scope here.
- Correction to the brief: there is no Toggles "stats" setting. `VsStats.asm` `toggle_match_stats` is the on-screen state
  of the stats page: it is shown after the results timer (STATS_TIME_UNTIL_ACTIVE) when the player presses A
  ("Press A for Match Stats"). No `Toggles.entry_*` drives it (grep of Toggles.asm: nothing). So the page is not gated by a
  settings entry; gate it by the A press when the drawing is ported.

## (b) Always Show Full Results (already in place, extra rows not ported)

- `decomp/src/gm/gmremixgameend.c` `gmRemixGameEndResultsDrawKind` maps StockRoyal to TimeRoyal and StockTeam to TimeTeam
  when `Toggles.entry_full_results` is on. It is used in `mnVSResultsMakeLabel` (`procs[...]`, guarded by
  `#if defined(PORT) && defined(REGION_REMIX)`), which is where the row procs (KOs, TKO, bar, points, place) are drawn.
  This matches TimedStock.asm show_full_results_ (0x801376E4), which skips the "mode is not Time" branch when the toggle is on.
- REMIX-TODO(results-mode-rows): GameEnd.asm add_row_to_results_ (VsRemixMenu.asm:1007, patches 0x1554D8, 0x1554E0,
  0x154878, 0x154930 and more) adds KOTH time, Smasketball score and goals rows. Nothing in `decomp/src` draws them, so
  those modes show only the vanilla rows, with or without full results.

## For the lead on screen

- Start: `SSB64_START_SCENE` with `nSCKindVSBattle` (scene order in `decomp/src/sc/scdef.h` ~156). For the transition
  check use the VS results scene (`nSCKindVSResults`, scdef.h ~158). Mario x Kirby VS, `SSB64_VS_STOCKS=1` so one KO ends
  the match, then move with the remix-smoke-tests `walk_right` input so Kirby or Mario takes the KO.
- Transition: the random pick is now 0..12 under Remix. Over several matches expect the two restored shapes (falling shapes,
  flip). The wipes must not crash; file 0x2F reuses the Rotating Frame offsets, so check that it looks like a vertical flip.
- Stats: after the results timer press A, but the page itself is not drawn yet. The counts can be checked with a
  debugger or `port_log` at the reset and at each count, for example by a special-move check with Mario's B
  (up, neutral, down) in the air and on the ground. Expect the port-count totals to match the number of specials.
- Full results: with Always Show Full Results on, a stock VS result must show the KO and TKO rows and the bar and points
  row (the time layout). With it off, the stock layout (place, bar, KOs) must be unchanged.
- Vanilla: build without REGION_REMIX should be unchanged; the new code is guarded.
