# F14 notes: results rows of King of the Hill and Smashketball

Scope: `REMIX-TODO(results-mode-rows)` in `decomp/src/gm/gmremixgameend.c`, asm `VsRemixMenu.asm` `add_row_to_results_` (line 1007 onward).

## Files

- `decomp/src/gm/gmremixgameendrowsrules.{c,h}`: pure ints. Seconds, minutes, seconds digit, goals cap (999), place y push (+16), tens digit rule, asm constants (y 124 / no contest 96, place delay 20 tics, no contest 100 tics, label texture offsets 0xE648 "Times" and 0xE9E0 "FGs").
- `decomp/src/gm/gmremixgameendrows.{c,h}`: reads the live state. `gmRemixGameEndRowsHasRow()` is KOTH or Smashketball. `gmRemixGameEndRowsGetValue(port)` is `gmRemixKothGetScore(port)` (frames) for KOTH and `gGMRemixSmashketball.scores[port]` (baskets) for Smashketball.
- `decomp/src/gm/gmremixgameend.c`: TODO comment reworded. No code change.

## Asm facts

- KOTH row: `KingOfTheHill.scores` (frames) per port, drawn as `M:SS` (seconds = frames/60, minutes = seconds/60, seconds digit = seconds%60). Label "Times". Tens digit forced on (`_show_seconds_tens_digit_`). Only drawn when the port is present (`gMNResultsIsPresent`).
- Smashketball row: `Smashketball.scores` (baskets, not `winner_scores`) per port, capped at 999 (`_draw_goals`, `slti 0x3E8`). Label "FGs".
- New row at y 124 (`_delay_place`, `lli a0, 124`), the place line moves from 124 to 140 (`_move_place`).
- Place line delayed by 20 tics after the new row (`addiu at, at, 20`).
- No contest: new row at y 96 on tic 100 (`_no_contest_new_row`).
- Teams: KOTH and Smashketball use the FFA results routine (`_change_teams_routine`, sets `0x96EC` to the `0x96E4` routine).

## Hooks not wired (shared file, not edited)

The rows are drawn by `decomp/src/mn/mnvsmode/mnvsresults.c`, which this scope does not own. The hook has to call the existing `mnVSResultsMakeKOs(s32 y)` (mnvsresults.c:2045) with a second row:

1. In `mnVSResultsMakeKOs`, when `gmRemixGameEndRowsHasRow()` is TRUE, add a row at y 124 with the label offset (`GMREMIX_GAMEEND_ROWS_KOTH_LABEL_OFFSET` or `..._SMASHKETBALL_...`) and the values from `gmRemixGameEndRowsGetValue`, drawn through `mnVSResultsMakeNumber` (mnvsresults.c:1726, `(GObj*, f32 x, f32 y, s32 number, s32 color_id)`), columns from `mnVSResultsGetColumnX` (mnvsresults.c:1940).
2. Place line: `mnVSResultsMakePlaceRow(124)` at mnvsresults.c:2375 and :2427 becomes `mnVSResultsMakePlaceRow(gmRemixGameEndRowsPlaceY(124, gmRemixGameEndRowsHasRow()))`, and the row is created 20 tics after the place line's own trigger.
3. Team routine switch (`_change_teams_routine`), see the mode routine table in mnvsresults.c around line 2533.

## Uncertainties

- The exact tic on which the vanilla place counter fires before the 20-tic delay is not verified. It is loaded from the vanilla code in the function that contains `0x80136338` and has to be checked in the game.
- The KOTH time value is shown as minutes and seconds, but the exact x offsets (minutes at x-20, colon at x+11) come from the asm `f20` arithmetic and are not ported here, since they belong to the drawing hook.
- No game run, no build, no git. The new files were only checked with `scripts/remix/syntax-check.sh`.

## Hook-facing functions

- `sb32 gmRemixGameEndRowsHasRow(void)`
- `s32 gmRemixGameEndRowsGetValue(s32 port)`
- `int gmRemixGameEndRowsSeconds(int)`, `gmRemixGameEndRowsSecondsDigits(int)`, `gmRemixGameEndRowsMinutes(int)`, `gmRemixGameEndRowsClampCount(int)`, `gmRemixGameEndRowsPlaceY(int, int)`, `gmRemixGameEndRowsForcesTensDigit(int)`
