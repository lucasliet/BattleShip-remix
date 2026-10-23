# F12 KOTH port notes

## Step 3 (KOTH and Smashketball goal picker)

Files edited:
- `decomp/src/mn/mnplayers/mnremixgoalpicker.c` and `.h` (new): value tables, next and prev helpers, 3-digit x.
- `decomp/src/mn/mnplayers/mnplayersvs.c`: include, hook in `mnPlayersVSGetNextTimeValue` and `mnPlayersVSGetPrevTimeValue`, goal branch in `mnPlayersVSMakeTimeSetting`.

Hooks (asm -> decomp):
- `VsRemixMenu.asm:872` get_next_value (`0x8013AB00`) -> `mnPlayersVSGetNextTimeValue` (`mnplayersvs.c`), KOTH and Smashketball tables.
- `VsRemixMenu.asm:919` get_prev_value (`0x8013ABDC`) -> `mnPlayersVSGetPrevTimeValue`, same tables.
- `KingOfTheHill.asm:28` time_value_array (5..960) -> `sMNRemixGoalKothValues`.
- `Smashketball.asm:74` score_value_array (1..7, 10, 15, 20, 50, 99, 100) -> `sMNRemixGoalSmashketballValues`.
- `KingOfTheHill.asm:303` allow_3_digits_for_picker_ -> `mnPlayersVSMakeTimeSetting` goal branch with 3 digits max; x is 212 below 100 and 218 from 100 (asm `0x4354` / `0x435A`).

Behaviour:
- Gated by `gmRemixModeIs(MNREMIX_VSMODE_KOTH / SMASHKETBALL)`. Other modes take the vanilla path with no change.
- Next and prev follow the asm: the first value above (below) the current one; wrap to the first (last) value at the ends.
- Default values 30 (KOTH) and 10 (Smashketball) come from the base (`gmRemixModeBeforeCSS` loads `gmRemixModeGetGoal` into `time_limit`, read by `mnPlayersVSInitVars`).
- Value 100 (Smashketball "no target") keeps the vanilla infinity sprite path, the same as the asm.

Deviations from the asm:
- The asm picks the table by `CharacterSelect.showing_time`: the KOTH and Smashketball tables are used only when the Goal label is shown. The port has no showing_time toggle, so the goal tables are always used in these two modes. Left as `REMIX-TODO(koth-goal-label)` in `mnremixgoalpicker.c`, together with the "Goal" text label.
- The vanilla decomp picker function uses its own table `{2, 3, 5, 10, 15, 30, 60, INF}`, not the asm array `1..100` at `0x8013B93C`. The default path is unchanged.

Not done:
- "Goal" text label (REMIX-TODO(koth-goal-label)).
- Smashketball GOAL background (`use_goal_yellow_picker_`, `Smashketball.asm:1399`) is section 2 row 4 of the Smashketball note, not part of this step.

Checks:
- `scripts/remix/syntax-check.sh` OK for `mnremixgoalpicker.c`, `mnremixgoalpicker.h` and `mnplayersvs.c`. The one warning in `mnplayersvs.c` (around line 2935, `&&` / `||` parentheses) predates this step.
- Not built and not run. The lead builds and checks on screen.

On screen for the lead:
- Mode KOTH on the VS page 2 menu, open CSS: the time picker starts at 30. Left arrow goes to 25, right to 45. From 960, right wraps to 5.
- Mode Smashketball: the picker starts at 10. Right arrow goes 15, 20, 50, 99, 100 (infinity sprite), then wraps to 1. Left arrow from 10 goes to 7.
- Values 90, 120, 240, 480, 960 render three digits, and the number shifts right at 100 and above.
- Mode Tug of War, 12CB, Tag Team and the default mode: picker unchanged (vanilla values).
