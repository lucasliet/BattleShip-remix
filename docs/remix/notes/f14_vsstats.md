# F14 vsstats: VS match stats (VsStats.asm)

Source: `.toolchain/pkgs/smashremix-src/src/VsStats.asm` (1136 lines), with the hooks in `ZCancel.asm`, `WallTeching.asm`,
`LedgeTrump.asm`, `AI.asm` (CPU tech miss, not ported here) and `Render.asm` (run_collect_, setup_ registration).

## REMIX-TODO lines of this scope

| Original TODO | State | Where |
|---|---|---|
| `mnremixvsstats.c` results-text (stats page not drawn) | implemented | `mnremixvsstatsview.c`, `mnremixvsstatspages.c` |
| `mnremixvsstats.c` results-stats-trackers (Z-cancel, tech, ledge) | implemented | `ftremixzcancel.c`, `ftremixwalltech.c`, `ftcommonpassive*.c`, `mpcommon.c` |
| `mpcommon.c:590` VsStats ledge grab (LedgeTrump.asm track_ledge_grabs_, 0x800DE688) | implemented | `mpcommon.c` |
| `ftremixzcancel.c:158` VsStats success and miss counters | implemented | `ftremixzcancel.c` ftRemixZCancelCountResult |
| `ftremixwalltech.c:39` tech_miss_tracker | implemented | `ftremixwalltech.c` |
| `ftremixwalltech.c:48` tech_success_tracker | implemented | `ftremixwalltech.c` |
| `ftremixaiteching.c:103` CPU tech miss (AI.asm 598) | not touched (AI file, out of scope) | still open, see below |
| new: normal results rows not hidden (Render group 0x16, 0x0D) | REMIX-TODO(VsStats) | `mnremixvsstatsview.c` |

## Files

- New: `mn/mnvsmode/mnremixvsstatsrules.{c,h}` (pure ints, the unit test target), `mnremixvsstatspages.{c,h}` (page layout),
  `mnremixvsstatsview.{c,h}` (prompt, input, page state).
- Edited: `mnremixvsstats.{c,h}` (tracker counters, highest percent), `ftremix/ftremixzcancel.c`, `ftremix/ftremixwalltech.c`,
  `ftcommon/ftcommonpassive.c`, `ftcommon/ftcommonpassivestand.c`, `ftparam.c`, `mp/mpcommon.c`, `if/ifremixcombometer.{c,h}`
  (read accessor `ifRemixComboMeterGetPort`), `mnvsmode/mnvsresults.c` (3 hook lines).

## Vanilla hooks (the lead should know these sit in vanilla files)

1. `ftcommon/ftcommonpassive.c` `ftCommonPassiveSetStatus` (0x80144660): tech success counter, under REGION_REMIX, asm
   `track_successful_tech` at 0x80144670.
2. `ftcommon/ftcommonpassivestand.c` `ftCommonPassiveStandSetStatus` (0x80144700): tech success counter, asm
   `track_successful_roll` at 0x80144714.
3. `ftparam.c` `ftParamUpdateDamage`: highest percent, after the 999 clamp, before stock_damage_all (asm run_collect_ /
   collect_stats_midmatch).
4. `mpcommon.c` `mpCommonRunFighterSpecialCollisions`: ledge grab counter for `this_fp->player` (asm 0x800DE688).

## Results screen input (what the lead must capture in game)

The view reads `gSYControllerDevices[i].button_tap`, any controller, like the asm `Joypad.check_buttons_all_ PRESSED`.

1. Start a VS match with 2 to 4 players (include a CPU), play until the results screen.
2. Wait until `results_tics + 105 >= allow_exit_wait` (the prompt appears; do not press START, it exits the results).
3. Press A: the page 1 damage and combo stats show, with `B: Back` and `R: Next Page`. Expect the 1P to 4P header only for the
   active ports.
4. Press R: page 2 (Z-Cancel, Tech, Ledge, Special Move Stats). R again wraps to page 1.
5. Press B: back to the prompt. A is ignored for 15 frames after each change (press buffer).
6. Combo rows need the Combo Meter setting on (`combo_meter` toggle). With it off, the combo section is absent, as in the asm.

Stat sources to check in a match: Z-cancel (aerial landing with the Z window), wall tech (hit a wall with Z pressed), passive
tech in place and roll, ledge grabs (Smash Remix Ledge Trumping setting may change them), specials (already counted).

## Deviations

- The overlay text is centred and proportional; the asm right-aligns cells at fixed x. Cells are spaced with 6-character
  padding, so columns line up only approximately. Labels and row order are the asm strings.
- Button icons are not drawn. The prompt reads `Press A for Match Stats`, the hints `B: Back`, `R: Next Page`.
- Stripe parity restarts on each page (the asm continues the count from page 1). Cosmetic.
- Damage per port pair is not recorded by a new hook. The port already keeps the same numbers in
  `gSCManagerTransferBattleState.players[]` (`total_damage_players`, `total_damage_given`, `total_damage_all`, updated by
  `ftParamUpdatePlayerBattleStats` and `ftParamUpdateDamage`), and the results read them the way `mnVSResultsSaveBackup` does.
  Only the highest percent needs a hook, since the port keeps no maximum.
- Percent uses round to nearest, ties to even, like `cvt.w.s` in the asm (`mnRemixVSStatsRulesPercent`). The special move
  percent in `mnremixvsstats.c` (truncation, older code) is unchanged.
- FGM ids 163 (menu.TOGGLE) and 164 (menu.SCROLL) are numeric constants with FGM.asm provenance; the decomp enum names for them
  were not found.
- Z-cancel counting: a Disabled setting counts nothing; success counts as success; fail and glide count as misses. This is
  the asm order (`_z_cancel_success` / `_z_cancel_miss`), applied to the port result. The CPU path (AI) goes through the same
  result, so CPU Z-cancels count too.

## Open items

- `ftremixaiteching.c:103` (REMIX-TODO(AI)): CPU tech miss counter (AI.asm line 598) is not counted. The asm increments
  `tech_miss_tracker` for a CPU that decides not to tech. The fix belongs in the AI file, which this scope does not touch.
  Until then, CPU misses are missing from the page.
- REMIX-TODO(VsStats): the normal results rows and menu instructions stay visible under the stats page (no display group toggle
  in the port). The page background (alpha 0xB0) covers them.

## Unit tests (lead)

`mnremixvsstatsrules.c`, pure ints. Suggested cases (the host check I ran gave these values):

- `mnRemixVSStatsRulesPercent`: (1,8)=12, (3,8)=38, (2,3)=67, (1,3)=33, (5,5)=100, (0,0)=0, (1,0)=0.
- `mnRemixVSStatsRulesHighest`: (3,7)=7, (7,3)=7.
- `mnRemixVSStatsRulesNextPage`: 0->1, 1->0.
- `mnRemixVSStatsRulesIsPromptActive`: (265,370)=TRUE, (264,370)=FALSE.
- `mnRemixVSStatsRulesTickBuffer`: 15->14, 0->0.

Counters in `mnremixvsstats.c` (need the Remix build, or a fake): `mnRemixVSStatsReset`, `mnRemixVSStatsCountTracker`,
`mnRemixVSStatsGetTracker`, `mnRemixVSStatsGetTrackerPercent`, `mnRemixVSStatsRecordPercent`, `mnRemixVSStatsGetHighestPercent`.

## Build status

`scripts/remix/syntax-check.sh` prints `syntax-check OK` for every touched file. No build, game or git was run, as the brief says.
