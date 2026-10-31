# F14: CSS goal label and human handicap 10 to 40

## Status

| TODO | File | State |
|------|------|-------|
| koth-goal-label | decomp/src/mn/mnplayers/mnremixgoalpicker.c:6 | Still open. Not ported. |
| AI (human handicap 40) | decomp/src/mn/mnplayers/mnremixcpulevel.c:17 | Still open. Rules added, panel not switched. |

## Done

- `mnremixcpulevelrules.{c,h}`: added `MNREMIX_HANDICAP_MAX` (40) and `mnRemixHandicapCanRaise`,
  `mnRemixHandicapNeedsIcon`, `mnRemixHandicapIconIndex`. Existing functions and their tests are unchanged.
  Source: AI.asm VS_CSS `handle_right_arrow_press_` (`lli t0, 0x28`), `render_10_._handicap` (`slt at, t6, 10`),
  handicap indicator table index = value - 10.

## Not done, and why

1. Human limit in `mnremixcpulevel.c` stays at 9. Raising it to 40 without a render hook makes
   `mnPlayersVSMakeHandicapLevelValue` read `offsets[value]` out of range.
   Hook needed in `decomp/src/mn/mnplayers/mnplayersvs.c` near line 3351 (`mnRemixVSIsCPULevelTens` branch):
   for a human value >= 10 call an icon renderer (AI.asm `_draw_icon`, CSS images file, handicap.table)
   instead of the digit. The icon renderer does not exist in the port yet.
   Then `mnRemixVSCanRaiseArrowValue` and `mnRemixVSIsArrowRightLimit` can switch to `mnRemixHandicapCanRaise`
   and `value == MNREMIX_HANDICAP_MAX`.
   Gameplay side: Handicap.asm `use_handicap_` and the CPU handicap consumers must accept 10 to 40 (not checked here).

2. Goal label and showing_time toggle (CharacterSelect.asm lines 64 and 6357-6400). The toggle is an input branch
   in the CSS time picker, in shared `mnplayersvs.c`; the port has no hook for it. The asm has no plain "Goal" string
   (grep for `goal` case-insensitive finds only KingOfTheHill.asm save/restore code), so the label is probably a
   sprite. Its position and source file are still unknown. Remaining work: find the label asset, then add
   `remix_hud_text_put` with a slot in `mnremixgoalpicker.c` and the toggle hook in `mnplayersvs.c`.

## Verification

- `scripts/remix/syntax-check.sh` on `mnremixcpulevelrules.c` and `mnremixcpulevel.c`: OK.
- No build, game or tests run. Unit tests for the new rules are not added yet (lead to add in
  `tests/remix/test_ai_cpulevel_rules.c`): 40 can not be raised, 9 can, 10 needs icon, icon index of 10 is 0.
