# F8 notes: Game Speed and Single Button Mode (Speed.asm, SingleButtonMode.asm)

Toggles (Toggles.asm, read directly by the asm, so `gmRemixToggle`, not `gmRemixToggleVS`):

- Game Speed: `entry_game_speed`, title "Game Speed", range 0 to 12. 0 = 1/1. 1 to 6 = faster (table, 2x = 5, 3x = 6).
  7 to 12 = slower (1/8, 1/4, 1/3, 1/2, 2/3, 3/4). Test: `SSB64_REMIX_TOGGLES="Game Speed=5"` (2x: two frames per game
  frame, the game looks twice as fast). Use `=7` for 1/8 (one frame in eight moves). Use `=6` for 3x.
- Single Button Mode: `entry_single_button_mode`, title "Single Button Mode", range 0 to 6. 0 = off, 1 = A, 2 = B, 3 = R,
  4 = A+C, 5 = B+C, 6 = R+C. Test: `SSB64_REMIX_TOGGLES="Single Button Mode=1"`: in a VS match only A (plus Start, Dpad,
  and C for 4 to 6) registers. Holding B or Z does nothing. Start still pauses.

## Speed.asm

| Patch | RAM | Vanilla function | Status |
|---|---|---|---|
| process_update_ (Stopwatch, not the Game Speed setting) | 0x8000A518 | gcRunGObjProcess (sys/objman.c) | Already applied by the Stopwatch worker (objman.c, REGION_REMIX block). Not touched here. |
| handle_rate_ | 0x8011427C | ifCommonBattleGoUpdateInterface (if/ifcommon.c), the final gcRunAll call | Applied: `ftRemixSpeedRunFrames()` under REGION_REMIX. |
| handle_rate_training | none (called by Training.asm:1276) | Training owner | Ported as `ftRemixSpeedTrainingRunFrames`, NOT wired. See below. |
| additional_advance (read by css/PoisonDmg.asm:84) | none | PoisonDmg owner | Getter `ftRemixSpeedIsAdditionalAdvance()` exported, NOT wired. |

Port: `decomp/src/ft/ftremix/ftremixspeed.c`. The frame count per game frame (waiting_timer shared by both variants):
speed 0 gives 1, 5 gives 2, 6 gives 3. Table speeds give 1 when active, 2 (fast) or 0 (slow) when not. The Stopwatch
backfire speed (`itRemixStopwatchGetWatchSpeed`) replaces the toggle while it is active, as in the asm. Unknown values
run 1 frame. The last gcRunAll of a game frame clears additional_advance; the extra ones set it.

Wiring still needed by other scopes:

1. Training (Training.asm:1276, sc1ptrainingmode owner): replace the call to handle_rate_training with
   `skip_advance = ftRemixSpeedTrainingRunFrames(training_speed_is_default, skip_advance);`. `training_speed_is_default` is
   TRUE when the Training speed multiplier word at RAM 0x80190B70 is 0 (the asm early-return). The decomp name of that
   word was not found, so the Training owner maps it.
2. PoisonDmg (css/PoisonDmg.asm:84): `if (ftRemixSpeedIsAdditionalAdvance()) return;` in place of the flag read.

Quirk kept on purpose: the training variant does not clear additional_advance after its extra frames (the asm never
stores FALSE on that path). So the training's own frame also reads TRUE, and PoisonDmg skips damage on it.

## SingleButtonMode.asm

| Patch | RAM | Vanilla function | Status |
|---|---|---|---|
| single_button_mode (_check_toggle to _return) | jumped to from css/DpadControl.asm:449 | css DpadControl (DpadControl owner) | Filter ported: `ftRemixSingleButtonModeFilter(held, pressed, gate)`. NOT wired. |
| handle_cpu_inputs_ A (`_a_b`) | 0xAC73C -> 0x80131CFC | not resolved | REMIX-TODO(AI). Helper `ftRemixSingleButtonCpuAOrBPress` ready. |
| handle_cpu_inputs_ B (`_a_b`) | 0xAC764 -> 0x80131D24 | not resolved | REMIX-TODO(AI). Same helper. |
| handle_cpu_inputs_ Z (`_z_l`) | 0xAC784 -> 0x80131D44 | not resolved | REMIX-TODO(AI). Helper `ftRemixSingleButtonCpuZOrLPress`. |
| handle_cpu_inputs_ L (`_z_l`) | 0xAC7A4 -> 0x80131D64 | not resolved | REMIX-TODO(AI). Same helper. |

CPU patches: `build/remixasm/refs/vanilla_funcs.txt` maps the four targets to menu overlay functions (mnTitleInitVars,
mvOpeningRoomMakeDesk, mnCharactersMakeDecals, mnDataMakeCharacters). The table is overlay-ambiguous here, so these are
not the CPU input code. The CPU input function needs to be found by the AI owner before the hooks are placed.

Gate (computed by the DpadControl owner, the asm order is top to bottom):

1. `FTREMIX_SBM_GATE_CLOSED`: no filtering. Also when the crash screen flag (RAM 0x8003CE78, dSYErrorIsScreenActive) is set,
   or the current screen is not one of TITLE_AND_1P, VS_BATTLE, BONUS, TRAINING_MODE, REMIX_MODES, or the match info is
   NULL, or pause state is 2 or more and not 3 (paused, outside Training).
2. `FTREMIX_SBM_GATE_TRAINING_MENU`: TRAINING_MODE screen and the Training toggle_menu byte is 2 or more (SSB or Custom
   menu up). Only buffered Start/B input is removed (keeps B only when it is the single button).
3. `FTREMIX_SBM_GATE_OPEN`: everything else.

The decomp has no names for `Global.current_screen`, `Global.match_info`, the Training `toggle_menu` byte or
dSYErrorIsScreenActive. The closest decomp field is `gSCManagerBattleState->game_status` (nSCBattleGameStatusPause and
nSCBattleGameStatusUnpause), which is only a guess at the same state. The lead must confirm the mapping before the gate
is written. Until then the gate cannot be computed and the filter is not called.

## Uncertainties

- handle_rate_ 0x8011427C: the asm replaces the gcRunAll call. The C hook assumes the call is the last statement of
  ifCommonBattleGoUpdateInterface (it is, at ifcommon.c end of function). Only checked by reading the source.
- Single Button Mode filter: the Joypad.asm and AI.asm reads of the toggle are other scopes and are not checked here.
- Syntax checks only. No build, no game run (per brief).
