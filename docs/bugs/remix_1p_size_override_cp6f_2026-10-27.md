# Remix 1P: forced CPU size overwritten by the CSS panel

Code: `decomp/src/ft/ftremix/ftremixsize.c`, `decomp/src/sc/sc1pmode/sc1premix1p.c`. Test: boot a Remix 1P stage with
`SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=remix1p SSB64_SPGAME_STAGE=<row>` (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

`sc1PRemix1PSetupEnemy` stored `FTREMIX_SIZE_STATE_GIANT` / `TINY` with `ftRemixSizeSetMatchState`, but the CPU of the giant
stage and the Tiny Team CPUs entered the match at normal size.

## Root cause

`ftRemixSizeInitFighter` (called from `ftManagerInitFighter`) rewrites the state from `gmRemixPanel(SIZE, player)`. In the 1P
and Bonus modes the panel value of every non-human port is 0 (`gmRemixPanelGet`, the same as `clear_settings_for_1p_` in the
asm), so the state went back to normal.

## Fix

`ftremixsize.c` keeps a forced state per port (`ftRemixSizeForceMatchState`; `FTREMIX_SIZE_STATE_UNFORCED` = -1 gives the
port back to the panel). `ftRemixSizeInitFighter` uses the forced state when there is one. `sc1PRemix1PSetupEnemy` forces
Giant on row 6 (Donkey) and Tiny on row 9 (Samus); `sc1PRemix1PRandomize` and `sc1PRemix1PSetupEnemy` on every other row
clear the forced states.

## Verification

- Row 6 (`SSB64_SPGAME_STAGE=6`): the CPU runs with state 1 and multiplier 2.25. Row 9: the three Dedede CPUs are visibly
  smaller than the Mario player.
- `SSB64_SPGAME_STAGE` is the row index of the 1P stage table; the opponents change on every run, so check the log of the
  same run.
