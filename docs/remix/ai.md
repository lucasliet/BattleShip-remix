# CPU AI

How the Smash Remix 2.0.1 CPU works (read from `.toolchain/pkgs/smashremix-src/src`: `AI.asm`, `Character.asm`, the per-fighter
`<Fighter>/AI/Attacks.asm`, the `*shared.asm` files, `Reflect.asm`, `KingOfTheHill.asm`, `Smashketball.asm`) and how the port
reproduces it. Code: `decomp/src/ft/ftcomputer.c` (the vanilla CPU, with `#if defined(PORT) && defined(REGION_REMIX)` hook
blocks), `decomp/src/ft/ftremix/ftremixai<topic>.{c,h}` (hook bodies; `ftremixai<topic>rules.{c,h}` hold pure int/float rules; topics: tables,
commands, prevent, longrange, weight, prediction, size, movement, recovery, recoverylogic, drift, shared, stage, cloud, lv10, level,
roll, combat, specials, teching, di, postprocess, postsheik, reflect, modes),
`decomp/src/mn/mnplayers/mnremixcpulevel*.{c,h}` (CSS level 10), `decomp/src/gm/gmremixspawnsetup.c` (default level).
Tests: rule tests `tests/remix/test_ai_*.c` (`cmake --build build --target run_remix_mode_tests`); in the game use
`.claude/skills/remix-smoke-tests/scripts/vs.sh <fkinds> <input> <name>` (P1 human, the other ports CPU) with
`SSB64_VS_CPU_LEVEL=<n>` (level of every CPU), `SSB64_REMIX_TOGGLES="Improved AI=1"` and `SSB64_VS_STAGE`. Log lines to grep:
`ftRemix AI:` (attack lists), `Remix AI input` (scripts), `cpu di`, `ftRemixAiPreventCandidate`, `ftRemixAiLongRangeSwitchKind`, `ftRemixAiRecoveryLogic`.

## 1. Pipeline and conventions

Vanilla frame (`ftComputerProcessAll`, 0x8013A834): when the script wait is over (`input_wait == 0`) the CPU runs
`ftComputerProcessTrait` (0x8013A63C, may override the behaviour), `ftComputerProcessBehavior` (0x8013A4AC, picks the main
procedure), `ftComputerProcessObjective` (0x8013A38C: calls the procedure, usually `ftComputerProcDefault` 0x80137778, then runs
the objective: Stand, Walk, Attack, Evade, Recover, ...). `ftComputerUpdateInputs` (0x80131C68) then interprets the current
command script byte by byte into `input.cp` (stick, buttons). A decision is a command id: `ftComputerSetCommandWaitShort`
(0x80132564), `...Immediate` (0x80132758) and `...WaitLong` (0x80132778) load the script of that id and a wait of
`(9 - level)`-scaled frames. Attacks come from per-fighter lists of `FTComputerAttack` (input id, hit start frame, four f32
detect bounds) filtered by `ftComputerCheckDetectTarget` (0x80132EC8).

Remix patches this code with jumps (`OS.patch_start(rom, ram)`), extends tables, and adds scripts. `AI.asm` is organised as: set-up
and bug fixes (21-1460), script engine and extended commands (1467-2490), per-fighter routine tables and constants (2491-3260),
`MANAGE_CPU` map (3264), `level_ten` (3547-6820, the level 10 work), CSS level 10 (6823-6900), `clamp_level_9_*` (7105-7337).

Port conventions:
- A Remix id is the port `fkind` (`port/remix/remix_roster_data.h`, `REMIX_ID_*`); vanilla fighters are `fkind < nFTKindEnumCount`
  (27), Remix fighters are 0x1D to 0x60. Ids 0x1B and 0x1C (random, none) have no table.
- Remix data is read at run time from the Remix RAM image (`port/remix/remix_ram_image.h`): words are host order (floats are
  bit-cast), byte streams use the big-endian path. Symbols are `REMIX_SYM_*` in the generated `port/remix/remix_symbols.h`; a new
  one is added to `EXTRA_SYMBOLS` in `tools/gen_remix_actions.py` and the header is regenerated.
- N64 field offsets of the asm never map 1:1 to the port `FTStruct`. Map them by field name (status id, level, floor line, hitlag,
  `input.cp`, ...); fields without a name go through `ftRemixFighterWord/Float` (`ft/ftremix/ftremixfields.h`).
- Hook shape: a `ftcomputer.c` block calls one `ftRemixAi<Topic>` function; the decision is in `ftRemixAi<Topic>Rules*` so
  `tests/remix` compiles it standalone. Without `REGION_REMIX` the vanilla path is untouched.

## 2. Levels, toggles and gates

`fp->level` is 1 to 9 in the vanilla game and 10 in Remix (the lv10 CPU). Level 10 is reachable through the VS CSS arrow
(`mnRemixVSCanRaiseArrowValue`, two-digit "10" sprites, saturates at 10 and 1, no wrap; the human handicap keeps the vanilla limit 9)
and through the `Default CPU LVL (V.S.)` toggle (0 to 10; `force_default_cpu_lvl_setup`: on the first VS set-up of a boot the
four slot levels become the toggle value, 3 when it is 0 — `gmRemixDefaultCPULevelSetup`, called from `mnplayersvs.c`).
`SSB64_VS_CPU_LEVEL` sets the level of every CPU in the VS shortcut. The port-only enhancement `CpuLevel9`
(`port/enhancements/CpuLevel9.cpp`) forces level 9 and overrides a 10.

Reads of `level` that subtract it from 9 or 18 would go negative at 10, so nine patches clamp the level to 9 in those reads
(`clamp_level_9_1..9`): the wait formulas of the three `SetCommand*` functions, `func_ovl3_8013837C` (follow wait),
`func_ovl3_80138AA8` (`MAX - level` product), `ftComputerGetObjectiveStatus` (CliffWait `(MAX - level) * 15`), the walk-stop
compare in `func_ovl3_8013877C` (`18 - 2*level`, floored at 0) and the damage-detect size of `ftComputerCheckDetectTarget`
(level 10 uses 0.875 instead of 1.0, so lv10 underestimates opponent size). Port: macros `FTCOMPUTER_LEVEL_CLAMPED`,
`FTCOMPUTER_WALK_STOP_WAIT`, `FTCOMPUTER_COLL_SIZE_BASE` in `ftcomputer.c` (vanilla expansion when compiled out) backed by
`ftremixailevel*.c`. The unpatched `* 25` downwait line keeps reading the raw level, as in the asm.

Gates used by the patches:

| Gate | Asm | Port |
|---|---|---|
| level 10 (`level >= 10`) | `slti 10` on byte 0x13 | `fp->level` compared against 10 (`ftRemixAiIsLevelTen`, `FTREMIX_AI_*_LEVEL_*`) |
| Improved AI toggle | `Toggles.entry_improved_ai` | `gmRemixToggle(improved_ai)`; `SSB64_REMIX_TOGGLES="Improved AI=1"` |
| Remix 1P (`singleplayer_mode_flag == 4`) | forces the "automatic improved AI" | `sc1PRemix1PIsActive()` |
| classic mode (vanilla 1P / RTTF, `GAMEMODE.CLASSIC`) | disables the toggle | `gSCManagerBattleState->game_type == nSCBattleGameType1PGame` (`ftRemixAiRecoveryIsClassicMode`) |
| Single Button Mode | `Toggles.entry_single_button_mode` | `gmRemixToggle(single_button_mode)` |
| excluded screens (How to Play 0x3C, DK vs Samus intro 0x2D) and Training (0x36) | teching and Z-cancel | `gSCManagerSceneData.scene_curr` (`nSCKindExplain`, `nSCKindOpeningJungle`, `nSCKind1PTrainingMode`) |

## 3. Data tables (Character.asm)

All tables have 97 entries (`NUM_CHARACTERS`), indexed by `Character.id` (= port `fkind`). Remix fighters that have no own entry copy
their parent's (`ai_behaviour`, `ai_aerial_chase`, `ai_long_range`).

| Table | RAM | Content | Port |
|---|---|---|---|
| `ai_behaviour` | 0x8049A320 | pointer to the fighter's attack list (two groups, see section 5) | read by `ftRemixAiGetAttackList` (`ftremixaitables.c`) for Remix fighters; vanilla fighters use the registry list |
| `lv10_ai_behaviour` | 0x8049D9C4 | attack list for level 10; filled for the 12 vanilla fighters by `VanillaLv10Attacks.asm`, 0 elsewhere | read by `ftRemixAiGetAttackList` for any fkind with an entry |
| `ai_attack_prevent` | `REMIX_SYM_Character__ai_attack_prevent__table` | per-fighter routine (`PREVENT_ATTACK.ROUTINE.*`), registered in each `AI/Attacks.asm` | read by `ftRemixAiPreventCandidate` (`ftremixaiprevent.c`): a vanilla routine address is matched by value, a Remix one by the entry of the fighter that registers it (section 5) |
| `ai_long_range` | 0x8049A7D0 | far-range routine: NSP_SHOOT 0x80138D24, PHYSICAL_SHOOT 0x80138CD4, NONE 0x80138ECC | read by `ftRemixAiLongRangeSwitchKind` |
| `nsp_shoot_custom_move` | 0x8049E198 | command id that replaces the far-range NSP command (0 = none) | read by `ftRemixAiLongRangeCommand` |
| `ai_aerial_chase` | 0x8049A640 | per fighter NORMAL 0x8013389C or CHASE 0x80133A14 (= skip the candidate): whether an NSP candidate is dropped against a Ness/Fox target | `ftRemixAiReflectSkipsNspCandidate` (`ftremixaireflect.c`): read for the own fighter (the copied power for a Kirby) in `ftComputerCheckDetectTarget`; polygon variants are always NORMAL |
| `recovery_logic` | 0x8049D538 | per-fighter recovery routine, called after the walk step | `ftRemixAiRecoveryLogicRunFighter` (`ftremixairecoverylogic.c`) matches the fighter's table entry with the entry of the fighter that registers it and runs the C port (`ftremixairecoverylogic*rules.c`); the registry slot `ai_recovery_routine` is gone |
| `cpu_post_process` | 0x8049D6BC | per-fighter routine at the end of the frame's decisions | `ftRemixAiPostProcessFighter` switch on `fkind` (Sheik) |
| `cpu_attack_weight` | not in `remix_symbols.h` | per-fighter routine that rescales a candidate's chance | dispatch exists in the accumulate loop (`port_fighter_ai_attack_weight_routine`); no code fills that slot |
| `close_quarter_combat` | not in `remix_symbols.h` | per-fighter branch of `stop_roll_spam_`; every entry is `_continue`, the per-fighter patches (AI.asm 6311-6359) are commented out | none needed |
| `AI.command_table` | 0x805192D0 | 0xFF words: ids 0 to 0x30 are the vanilla scripts, 0x31+ point to the routines below | read by `ftRemixAiGetInputScript` |

Who registers what: `recovery_logic` — Banjo, Crash, Dedede, Falco, Marina, Marth, Roy, Peach, Sheik, Sonic, Wario, Captain/JFalcon/
DragonKing, Fox/JFox, the Link, Mario (Luigi, Dr. Mario/Luigi), Ness (JNess, Lucas), Pikachu, Puff and Yoshi families.
`cpu_post_process` — Bowser, GBowser, Dedede, DragonKing, Falco, Ganondorf, Marina, Marth, Roy, Mewtwo, Sheik, Wario, Wolf, Captain/
JFalcon (`cpu_post_process_falcon`), DK/JDK, Fox/JFox, Link/JLink/ELink/YLink, Pikachu family, Puff family, Kirby/JKirby (`_kirby`),
Yoshi/JYoshi. `cpu_attack_weight` — Crash, Dedede, DragonKing, Ganondorf, Marina, Piano, Captain/JFalcon, Puff family, Kirby/JKirby
(`_kirby`), Yoshi/JYoshi. `nsp_shoot_custom_move` — Dedede, Peach and Young Link (the last two use the DSP routine).

## 4. Input scripts and extended commands

A script is a byte stream ending in `END` (0xFF). Vanilla commands set the stick, buttons and a wait; the Remix macros
(`STICK_X`, `PRESS_A`, `UNPRESS_B`, `RESET_TIMER`, `FAST_FALL`, `CUSTOM(n)`, ...) emit the same bytes. Remix adds:

- **Routines**: `add_cpu_input_routine(label)` gives ids from 0x31 in source order. `AI.asm` defines 0x31 to 0x4C (ids in hex):
  SHORT_HOP_TOWARDS 31, SHORT_HOP_AWAY 32, SHORT_HOP_IN_PLACE 33, FULL_C_JUMP_TOWARDS 34, SHORT_HOP_NAIR 35, SHORT_HOP_DAIR 36,
  PIVOT_AWAY 37, NESS_DJC_NAIR 38, NESS_DJC_DAIR 39, MULTI_SHINE 3A, MULTI_SHINE_TURNAROUND 3B, SHIELD_DROP 3C, LUCAS_BAT_BACKWARDS 3D,
  LUCAS_BAT_FORWARDS 3E, PUFF_SHORT_HOP_DAIR 3F, CLIFF_LET_GO 40, SMASH_DSP 41, NSP_TOWARDS 42, FAIR 43, BAIR 44, DASH_GRAB 45,
  DASH_USMASH 46, DASH_ATTACK 47, USP_TOWARDS 48, POINT_STICK_TO_TARGET 49, JUMPCANCEL_UPSMASH 4A, JUMPCANCEL_USP 4B, NULL 4C (an
  empty script that protects an input from being overwritten). The id comments in the asm source are stale; the source order is
  the truth. Fighter files add their own after 0x4C, in include order (the build log `build/remixasm/bass.log` prints each id): WARIO_NSP_TOWARDS 4D, WARIO_NSP_TOWARDS_JUMP 4E, DSAMUS_DSP_FOLLOW 4F, BOWSER_SHORTHOP_FIREBREATH 50, SONIC_CHARGED_DSP 51, MARINA_DOUBLE_DSP 52, GOEMON_HELD_NSP 53, BANJO_BACKWARDS_NSP 54, CRASH_DASH_NSP 55, ROY_DSP_HELD_10F 56, ROY_DSP_HELD_20F 57; training scripts (1917-2010) and the
  twenty-four DI scripts (`DI_SMASH_*`, `DI_SLIDE_*`, 2074-2144) are not in the table.
- **Stick X codes** (`extend_stick_x_commands`, 0x80131DA4): 0x81 AUTOFULL_AWAY (+-80 away from the target X), 0x82 AUTOHALF_AWAY (+-40),
  0x83 FORWARDS and 0x84 BACKWARDS (+-80 along the facing).
- **`CUSTOM(n)`** = command byte 0xFE then an index byte (`CUSTOM_COMMANDS`): 0 JUMPSQUAT_WAIT (wait while in JumpSquat), 1 PRESS_C
  (button bit 0x0001), 2 UNPRESS_C (clear the four C bits), 3 STICK_TO_TARGET (stick = vector to the target scaled to 80), 4
  TURNAROUND_WAIT (wait while Turn/TurnRun). A waiting command sets a one-frame wait and re-reads itself next frame.

Port (`ftremixaicommands.c`, `ftremixaicommandsrules.c`): `ftComputerGetInputScript(id)` returns the host copy of a Remix script
(up to 0x80 bytes, copied byte by byte from the image on first use, one log line per id) or the vanilla script; the three
`SetCommand*` functions use it. Comparisons of `p_command` with a table entry (post process, dash attack check) compare pointers
against `ftComputerGetInputScript(id)`. Ids past the table, or a routine without an END byte within 0x80 bytes, behave as `END`. The
`switch` of `ftComputerUpdateInputs` gets cases 0x81 to 0x84 (`ftRemixAiStickXFromCode`) and 0xFE (`ftRemixAiRunCustomCommand`, a
C switch in place of the asm jump table). STICK_TO_TARGET rounds with `lrintf` (the asm `cvt.w.s` rounds to nearest too) and reads
the top joint position.

## 5. Targets and attack selection

**Attack lists.** An entry is 7 words (input id, hit start frame, hit end frame (0 for `add_attack_behaviour`), min X, max X, min Y,
max Y). A list is a grounded group, `END_ATTACKS` (`0xFFFFFFFF` + 6 zeros), an aerial group, `END_ATTACKS`. Input ids below 0x31 are
vanilla commands; ids from 0x31 are routines (section 4). `custom_attacks_table` (AI.asm 3547, `level_ten`) runs in
`ftComputerCheckDetectTarget` where the vanilla code loads the list pointer: at level 10, if `lv10_ai_behaviour[id]` is nonzero it
replaces the pointer. Port: `ftRemixAiGetAttackList(fkind, level)` returns the lv10 list for level 10 and any fkind with an entry,
else the `ai_behaviour` list for Remix fighters (`fkind >= nFTKindEnumCount`), else NULL and `ftComputerCheckDetectTarget` uses the
registry list. Each list is built once per fkind (walk to the second terminator, at most 128 entries, one `ftRemix AI:` log line
with kept and dropped counts) and cached; a custom id is kept only when `ftRemixAiIsCustomInputSupported` finds a terminated
routine in the image. `VanillaLv10Attacks.asm` covers Mario, Link, DK, Fox, Captain, Pikachu, Samus, Jigglypuff, Luigi, Yoshi, Ness
and Kirby.

**Picker.** `ftComputerCheckDetectTarget` loops the candidates of the current group: predicts the target position at the hit frame,
tests the detect box (scaled by the Super Mushroom factor, see below) and collects the matches with a chance each; the chances
are accumulated (a repeated input is penalised by the `*_button_*_count` adjustments), a random point picks the move, the veto runs,
and the command is issued with `ftComputerSetCommandWaitShort`.

| Patch (AI.asm) | Vanilla site | Behaviour | Port |
|---|---|---|---|
| `size_multiplier_check_` (56) | `ftComputerCheckDetectTarget` 0x80133428 | a non-giant fighter whose `Size.multiplier_table[port]` is not 1.0 takes the giant-DK detect-range multiply path with that factor | `ftRemixAiSizeDetectScale` |
| `check_invulnerable_` (3636) | evade loop 0x80132C84 | lv10: a respawning enemy (`invincible_tics != 0`) counts like a Starman for the evade distance test | `ftRemixAiEvadeCountsEnemyInvulnerable` |
| `fix_movement_prediction` (5183) | `ftComputerCheckDetectTarget` | lv10: while Dash/Run/RunBrake only grab, up smash, grounded USP/NSP and DASH_ATTACK are candidates; predicted X uses the CPU velocity with ground friction, target X/Y with its velocity and gravity (falling Y while airborne) | `ftremixaiprediction.c` |
| `weight_attack_options` (5487) | gather loop 0x80133784 | lv10 only, replaces the chance of each candidate by stages: own landing before the move (0), combo (opponent hitstun left after a tech landing; chance x1000, a roll of `span` must be 0 when span >= 2), not-combo (one float always drawn; a move with start frame >= 15 survives only below 1/8, 20: 1/32, 30: 1/64), slow-move scaling by `frame`/`frame^2`/`1/frame^2` per opponent state and percent (>90, >150), grab against shield or shield stun (x1000) | `ftRemixAiWeightAttackOption` (level gate `ftRemixAiWeightAttackOptionsApplies`) |
| `skip_repeated_input_check` / `_skip` (5944, 5980) | same function | lv10 skips the repeated-input range adjustment and never reports "four in a row" (the counter resets) | `ftRemixAiSkipRepeatedInputCheck`, `ftRemixAiInputRepeatLimitHit` |
| `no_attack_when_all_odds_eq_zero` (6016) | same function exit | when every chance is 0 no attack is picked (return FALSE) | the port returns FALSE on that fall-through for every level |
| `extend_attack_check_range(_2)` (5136, 5158) | `ftComputerFollowObjectiveAttack` 0x801392C8 | the "target is close enough to attack" distance base is 2000 at lv10 (vanilla 1200) | `ftRemixAiPredictionAttackViewBase` |
| `PREVENT_ATTACK` routines (2973-3260) | candidate loop of `ftComputerCheckDetectTarget` (jump at 0x801334D0) | per candidate attack: set `is_attempt_cliffcatch` (the ledge probe may drop it) or skip the candidate | `ftRemixAiPreventCandidate` (verdict ALLOW, CLIFF_CHECK or SKIP), replacing the vanilla `switch (fkind)` |
| `AERIAL_CHASE.extend_polygon_check_` (2503) and `Reflect.AI.extend_reflect_absorb_character_check_` (0x80133850) | skip-NSP-against-Ness/Fox switch | the polygon exclusion also covers Remix polygons; the Ness/Fox test becomes the `fighter_reflect` table (Wolf, Falco, JFox, JNess, Lucas, NLucas, Marina, Dedede, Piano, Slippy) so CPUs do not shoot at reflectors/absorbers | `ftRemixAiReflectSkipsNspCandidate`: target in `Character.fighter_reflect` and own `ai_aerial_chase` entry CHASE drops the AutoX B candidate (level >= 5), see section 10 |
| `ai_ball_attack_range` (Smashketball 1211, 0x80133144) | picker, before the list pointer is used | with the Smashketball mode on, the ball replaces the target: hurtbox 200 x 200, ball position, velocity, gravity and fall speed | `ftRemixAiModesGetBallTarget` (`ftremixaimodes.c`) |

PREVENT_ATTACK routines (by `fkind`, as registered): FALCO_NSP (Falco: NSP vetoed airborne, then the FOX_USP rule), FOX_USP (Fox,
JFox, Slippy: USP vetoed at lv10), BOWSER_USP_DSP (Bowser: DSP and USP), GBOWSER (DSP), USP (Wolf, Dedede: USP), SONIC_DSP (Sonic,
DSamus: DSP; DSamus registers it last, after `YOSHI_FALCON`), WARIO (DSP and the literal ids 0x42/0x43 of the asm, which are NSP_TOWARDS and FAIR; the port vetoes Wario's own NSP routines 0x4D/0x4E instead), MARINA_NSP (NSP), CONKER_GRENADE
(DSP or SMASH_DSP vetoed while a grenade is out, FTStruct word 0x0ADC), CRASH (DSP/USP vetoed over a soft platform), and the vanilla
routines MARIO (0x801334EC: no self-destruct USP; Banjo, Dr. Mario/Luigi, Marth, Peach, Piano, Roy), YOSHI_FALCON (0x80133510: DSP;
Ganondorf, DragonKing, and DSamus, whose later SONIC_DSP registration (`DSamus.asm`) wins) and KIRBY (0x80133500: DSP and USP, the
vanilla Kirby entry). `PUFF_DSP` is only installed by commented-out
code (AI.asm 6452-6465) and never runs. The veto contract differs slightly from the asm (`t2 = 1` then a jump to the commit address
or SKIP_ATTACK): the port returns FALSE from the whole check.

**Far range** (`func_ovl3_80138AA8`, 0x80138AA8). For fighters outside the vanilla ids (0x00-0x0D and 0x1A-0x1D keep the vanilla
`switch`), the `ai_long_range` routine selects the case: NSP_SHOOT runs the Mario body, PHYSICAL_SHOOT the Link body (boomerang
within 1500, then NSP), NONE nothing. The `extend_polygon_check_` patch (2543) sends polygons to NONE. At the end of the function
`custom_move` (2568) replaces the command by `nsp_shoot_custom_move[fkind]` when nonzero (default AutoX B, or the tilt-AutoX variant
when delayed). Port: `ftRemixAiLongRangeSwitchKind`, `ftRemixAiLongRangeSetCommand`.

**Targets.** `ftComputerCheckFindTarget` (0x8013295C) chooses the opponent; `ftComputerCheckTargetItemOrTwister` (0x80134B4C) retargets
items and the Hyrule tornado. Remix hooks there: `detect_offstage_opponents` (4869; lv10 skips the on-stage group so off-stage
opponents are targets) and `clamp_target_pos_to_stage` (4903; lv10 clamps the stored target to the stage bounds with a 300 X padding
and a Y floor), Smashketball `ai_ball_behavior_` (1153, 0x80132B44: target = ball position +-400 by team, distance recomputed) and
KOTH `ai_hill_behavior_` (1712, 0x80134B84 at the start of `ftComputerCheckTargetItemOrTwister`: in KOTH the target is the hill
indicator position minus 500 Y, `target_line_id` is the hill line, items and the tornado are ignored, TRUE is returned; if the indicator's clipping is off or it is hidden, `target_line_id` is set to -1 and
the vanilla logic runs). Port: `ftRemixAiMovementTargetsOffstage`, `ftRemixAiMovementClampTarget`, `ftRemixAiModesSeekBall` (`ai_ball_behavior_`, end of `ftComputerCheckFindTarget`; the ball position comes from `itRemixBasketballGetPosition`, the home team from `gmRemixSmashketballGetHomeTeam`) and `ftRemixAiModesSeekHill` (`ai_hill_behavior_`, before the item loop of `ftComputerCheckTargetItemOrTwister`; the marker position from `gmRemixKothModelGetMarkerPosition`).

## 6. Movement, recovery and specials

| Patch (AI.asm) | Vanilla site | Behaviour | Port |
|---|---|---|---|
| `recovery_fix_` (133) | `ftComputerFollowObjectiveWalk` 0x8013557C | the "up-B already used" limit is skipped at lv10 / Remix 1P, and with Improved AI outside classic mode | `ftRemixAiRecoveryAllowsUpBRepeat` (Remix 1P, level, classic mode and toggle) |
| `fix_target_pos_x` (182) | Walk 0x8013537C | Improved AI: the walk target X is clamped to the map bounds minus 200 (bounds are the ground data left/right halfwords) | `ftRemixAiRecoveryClampWalkTargetX` (`gMPCollisionGroundData->map_bound_left/right`) |
| `fix_remix_recovery` (240) | `ftComputerFollowObjectiveRecover` 0x80137F4C | Pikachu family: vanilla block in classic mode or with Improved AI off, Remix recovery otherwise; Sheik (action 0xEE), DSamus (0xE7), Peppy (0xF2), Mewtwo (0xE2) press Z while charging NSP and skip walk and recovery logic; Wolf USP actions (0xE3, 0xE4, 0xE6, 0xE8) aim at X 0, Y 4000; Banjo Flight (0xEF) on frame 21 presses B and, with no floor line below, sends PRESS B and skips the recovery logic | `ftRemixAiRecoveryVanillaPikachuApplies`, `ftRemixAiRecoveryWolfTarget`, `ftRemixAiRecoverySpecialCase` |
| `custom_recovery_logic` (384) | after the walk, 0x80137FBC | calls `recovery_logic[fkind]` | `ftRemixAiRecoveryLogicRunFighter`, called in `ftComputerFollowObjectiveRecover` after `ftComputerFollowObjectiveWalk` (section 8b) |
| `_improve_recovery_drift` (4337) | Walk 0x80135400, just before the `joint Y < target Y` compare | lv10 and (special fall, or all jumps used, or Recover objective with no floor line below): refresh `cliff_left_pos` / `cliff_right_pos` from the floor lines (`_find_closest_ledge`: left edges for direction 1, right edges for -1, edges 200 inside the blastzone, no ground 50 below, a ledge above counts 1.5 times its Y distance, grabbable = floor edge flag and nobody hangs there), pick the side from X + 200 * facing, then aim at the ledge: outside it (300), at its grab box when facing a grabbable ledge, inside it when above, special cases for the up special | `ftRemixAiRecoveryDrift` (`ftremixairecoverydrift.c`, rules in `ftremixairecoverydriftrules.c`), called at `block_52` of `ftComputerFollowObjectiveWalk` |
| `fix_multi_doublejump_usage` (5448) | Walk jump status list | lv10 also accepts the aerial-jump procedure (`ftCommonJumpAerialProcUpdate`) as jump-ready so the CPU does not wait in Fall | `ftRemixAiCanJumpWhileFalling` |
| `prevent_fastfall` (4799) | Walk "fast fall?" block 0x80135704 | lv10: no fast fall with all jumps used or an off-stage target; otherwise only when `floor_dist < -2000` | `ftRemixAiMovementFastfallPermitted` |
| `shorthop_followup_attack` (4987) | Walk grounded branch 0x80135B18 | lv10: target above by <=300 point stick, <=600 SHORT_HOP_TOWARDS, <=900 FULL_C_JUMP_TOWARDS, else vanilla | `ftRemixAiMovementShorthopInput` |
| `jump_in_neutral` (5065) | Walk 0x80135ABC | lv10 rolls 0..199 per frame in that state: 0-2 SHORT_HOP_TOWARDS, 3 SHORT_HOP_IN_PLACE, 4-5 FULL_C_JUMP_TOWARDS (objective becomes Attack) | `ftRemixAiMovementNeutralInput` |
| `shield_drop_` (6366) | Walk platform-drop branch 0x80135B3C | lv10 in Wait sends SHIELD_DROP, others STICK_DOWN | `ftRemixAiDownCommand` |
| `_fix_weird_air_drift_right/left` (4007, 4033) | `ftComputerCheckSetTargetEdgeRight/Left` 0x80134000 / 0x80134368 | lv10 with upward Y speed does not retarget to itself +-500 X | `ftRemixAiSkipsUpwardDriftTarget` |
| `usp_check_` (1342) | `ftComputerUpdateInputs` air branch 0x80132140 | the Fox USP branch covers Fox, JFox, Peppy, Falco; Goemon / Ebisumaru in a Magic Cloud action skip the `dist_y < 0` stick zeroing | `ftRemixAiUspIsFoxFamily`, `ftRemixAiUspIsCloudAction` |
| `recovery_stick_y_fix` (1402) | after each command, 0x8013253C | Goemon / Ebisumaru Magic Cloud: stick up; Goemon presses Z, Ebisumaru A above a floor line (Cloud Escape clears Z) | `ftRemixAiCloudRecoveryInput` |
| `evade_fix_` (94) | `ftComputerCheckEvadeDistance` 0x80132C00 | with Improved AI (or Remix 1P) the CPU does not evade while it is Starman (>= 112 frames left) or respawn-invincible (>= 20 frames) | `ftRemixAiCombatEvadeIsSuppressed` (toggle and timers). The asm's Remix 1P compare uses `v1` after loading 4 into `t6`; at the only call site (0x801369EC) `v1` is -1, so the term never fires and is not ported |
| `metal_luigi_ai` (MLuigi.asm 104) | `ftComputerCheckFindTarget` 0x80132AA4 | Metal Luigi, like Metal Mario, only targets grounded opponents | `ftRemixAiIsMetalKind` |
| `remix_ignore_projectiles_` (1299) | `ftComputerProcDefault` 0x801377E4 | MLuigi and GBowser (and anyone with the Franklin Badge) skip the projectile reaction | `ftRemixAiCombatProjectileIgnored` |
| `charge_less_often` (871) | `ftComputerCheckTryChargeSpecialN` 0x80136A2C | lv10 starts a charge only on a 1/60 roll (rolled only at lv10) | `ftRemixAiCombatChargeIsSkipped` |
| `improve_remix_specials` (924) | same, 0x80136A54 | charge decisions for Remix ids: Samus-like (Mewtwo, DSamus, ESamus, JSamus) share the Samus charge, JDK the DK one, Bowser/GBowser zero the stick X during a grounded USP (action 0xDE), Sheik/Peppy keep charging in their charge states, stop when shooting or in jumpsquat, and press B below 6 needles/shots | cases in `ftComputerCheckTryChargeSpecialN`, `ftRemixAiSpecialsBowserUspStickReset`, `ftRemixAiSpecialsNeedleCharge` (Sheik falls through into the Peppy action checks, as the asm) |
| `improve_remix_charged_NSP_1` (1031) | `ftComputerCheckTryCancelSpecialN` 0x80136C0C | DSamus, Sheik, Mewtwo (and Kirby copies) press Z while a charge can be cancelled; JDK is added to the DK case | `ftRemixAiSpecialsCancelChargeWithZ` |
| `improve_remix_charged_NSP_2` (1130) | `func_ovl3_80138AA8` 0x80138C24 | a Samus-like NSP is not fired at Fox, Slippy, Wolf, Marina, Falco, Piano, JFox, nor at Ness, Lucas, JNess unless the shooter is Sheik | `ftRemixAiSamusNspIsBlocked` |
| `improve_remix_charged_NSP_3` (1180) | same, 0x80138C44 | own-action exit for Sheik, Mewtwo, DSamus (and the Kirby copies of their actions) and the Samus action list for JSamus, ESamus, DSamus and Mewtwo (stop the long-range walk while charging). The Kirby hat half compares the copy word with 0x1E, 0x1B and 0x13, stale hat ids that match no copyable fighter, so Kirby keeps the Samus hat case only | `ftRemixAiSpecialsFarRangeIsCharging`, `ftRemixAiSpecialsFarRangeUsesSamusActions` in `func_ovl3_80138AA8` |
| `extend_cliff_attack_` (3663) | `ftComputerGetObjectiveStatus` CliffWait 0x80136E04 | lv10: `get_random_int_(2) >> 1` is always 0, so the getup is always CLIFF_LET_GO | `ftRemixAiCliffGetupCommand` (still draws the RNG once) |
| `taunt_replace_` (3690) | same, taunt case 0x80137754 | lv10 never taunts | `ftRemixAiTauntIsSuppressed` |
| `force_objective_attack` (3892) | `ftComputerProcessTrait` 0x8013A718 | lv10 forces behaviour 0 (no RNG call) | `ftRemixAiTraitRoll` |
| `stop_roll_spam_` (6040) | `func_ovl3_80138EE4` roll L/R, 0x80139294 | for lv10 CPUs (and GBowser at any level): in actions after TurnRun reset the stick; against an opponent in an advantage state (damage, down, stun, sleep) roll 16: 0 SHORT_HOP_IN_PLACE, 1 SHORT_HOP_TOWARDS, else skip; otherwise 0 the vanilla roll, 1 SHORT_HOP_IN_PLACE, 2 SHORT_HOP_TOWARDS, 3 SHORT_HOP_AWAY, 4-5 PIVOT_AWAY, else skip | `ftRemixAiStopRollSpam` |
| `planet_zebes_acid_._ai_fix` (Hazards.asm 165-177, 234-254) | `ftComputerGetObjectiveStatus` acid check, 0x801374E8 | the CPU flees the rising acid on every stage whose `Stages.function_table` entry is the Planet Zebes maker (0x03, 0x5C, 0x74, 0x79, 0xA5), and only with the hazards on (stage 3 alone, hazards or not, before) | `ftRemixAiZebesAcidApplies` (`ftremixaistage.c`); the two vanilla checks in `ftComputerCheckSetTargetEdgeLeft/Right` stay vanilla, as in the asm |

## 7. Teching, Z-cancel and DI

- **Teching** (`random_teching_`, 443; sites `ftCommonDamageAirCommonProcMap` 0x80140980 and `ftCommonDamageFallProcMap` 0x801435F4,
  which replace the vanilla tech chain and the bounce). Sandbag never techs. Single Button Mode A, B, A+C, B+C blocks the tech for
  every player (R and R+C allow). A CPU takes the improved path at lv10, in Remix 1P, in Training, or with Improved AI outside
  classic mode (not on the excluded screens), otherwise the vanilla chain. The roll is 0..99: <30 forward, <60 backward, <90 in
  place, else a failed tech (counted in `VsStats.tech_miss_tracker`); off screen only the in-place band (<90) applies. In Training
  the menu entry (1 backward, 2 forward, 3 in place, 4 roll left, 5 roll right, 6 none) replaces the roll unless it is 0.
  Port: `ftRemixAiTechIsHandled` (`ftremixaiteching.c`); "off screen" is `is_magnify_show`; the miss counter is
  `mnRemixVSStatsCountTracker`.
- **Z-cancel** (`z_cancel_`, 667): on the improved path, in a CPU, outside classic mode and the excluded screens, with the SBM gate
  open, the CPU Z-cancel succeeds on a 0..99 roll below 95 (the asm leaves the boolean in `at`). Port: `ftRemixAiShouldZCancel`,
  consumed by `ftRemixZCancelResult` (a "Disabled" Z-cancel option still returns success for the CPU, as the asm does).
- **DI** (`cpu_di_`, 6534; hook before the objective switch at 0x8013A3C8). When the CPU is in hitlag in a damage action (DamageHi1 to
  DamageFlyRoll) and the hit is past the first hit (< 2 hits and status frame < 10 skips; in Training `combo_count_foe >= Start on Hit`),
  it replaces the objective by a DI script (`DI_SMASH_{HIGH,MEDIUM,LOW}_{LEFT,RIGHT,UP,DOWN}`, `DI_SLIDE_*`: smash = tap and release the
  stick for 1, 3 or 5 frames; slide = the tap repeated while the stick sweeps the other axis). Training uses the DI menu entries
  (type none/random/smash/slide, strength random/high/medium/low, direction random/L/R/U/D/away/toward; random picks are stored per
  port and combo). Lv10 uses an advanced index 0..8 stored for the combo (0 none, 1-4 smash, 5-8 slide, one per direction, high
  strength). Levels 4 and up with the advanced AI (Remix 1P, or Improved AI outside classic mode) roll the same index with low
  strength (levels 4-6) or medium (7-9). The per-port parameters reset when the hit count is 0. Port: `ftRemixAiCpuDiSetObjective`
  (`ftremixaidi.c`, rules in `ftremixaidirules.c`, scripts rebuilt by `ftRemixAiDiBuildScript`); Training values come from
  `remix/remix_training.h`. The Training spam mode shares the same hook point (`ftRemixTrainingSpamSetObjective`).

## 8. Post process and per-fighter hooks

`cpu_post_process` (AI.asm 3733, patch at 0x8013A884, called after `ftComputerProcessObjective` inside the `input_wait == 0` block)
runs in this order: (1) Improved AI: a dead CPU (status Sleep) mashes START every 16 frames to respawn; (2) a DASH_ATTACK script that
cannot start (status not Wait or Run) becomes NULL; (3) lv10: a running/dashing CPU whose script is the up smash or USP input
becomes JUMPCANCEL_UPSMASH / JUMPCANCEL_USP; (4) lv10: a script that is USP becomes USP_TOWARDS; (5) `cpu_post_process[fkind]`. Port:
`ftRemixAiPostProcess` (`ftremixaipostprocess.c`), the fighter switch holds Sheik (`ftremixaipostsheik.c`): on the USP actions
(0xE4, 0xE5, 0xE7, 0xE8) with a floor line below, NULL command and stick (0, -80); on DSP begin (0xF0) with a target within 600 units, B is
pressed, else NULL.

Hooks in shared fighter files follow one pattern: they extend a vanilla `fkind` test to the clones, or add a Remix-specific behaviour.

| Patch | Vanilla site | Behaviour |
|---|---|---|
| `linkshared.cpu_pull_out_bombs_` (740) | `func_ovl3_80138AA8` 0x80138A04 | ELink, JLink, YLink, Peach pull bombs like Link; Marina when her charge is 2+. Port: cases in `func_ovl3_80138AA8` |
| `dkshared.giant_punch_fix_1` (268), `cpu_fix_2` (288) | 0x80136C30, 0x80138F78 | JDK joins the Giant Punch checks (charge, cancel, evade); Marina and N Marina never evade while in Cargo (0xEB to 0xEF). Port: JDK cased in `ftComputerCheckTryChargeSpecialN`, `...CancelSpecialN` and `func_ovl3_80138EE4`; `ftRemixAiEvadeIsMarinaCargo` |
| `nessshared.cpu_usp_usage_fix_` (795), `cpu_usp_control_fix_` (822), `cpu_pk_thunder_attack_` (1133), `improved_pk_thunder_` (1160), `save_cpu_recovery_location_` (1245) | 0x80134730, 0x80137160, 0x80138A0C, 0x801371FC, 0x80153CA8 | JNess and Lucas use the Ness USP range, PK Thunder steering and idle PK Thunder attack (cases in `func_ovl3_801346D4`, `ftComputerGetObjectiveStatus`, `func_ovl3_8013877C`). Level 10, Remix 1P or Improved AI outside classic mode: while the hold status is at most 0x3B tics old, the aim offset is `512 * atan2(ledge Y - own Y + 500, abs(ledge X - own X))` instead of 100 (`ftRemixAiPkThunderTargetY`). The ledge is the saved per-port `cliff_left_pos` (own X < 0) or `cliff_right_pos`, written by `ftRemixAiPkThunderSaveLedge` at the start of `ftNessSpecialHiMakePKThunder` |
| `jigglypuffkirbyshared.kirby_cpu_inhale` 1-3 (513-570) | 0x80136C1C, 0x80138F08, 0x801370F0 | the copied-power selector is `copy_id` for Kirby and J Kirby (`ftRemixAiCopiedPowerKind`; Dedede has no copy word, the asm reads a minion pointer that matches nothing). In the inhale block of `ftComputerGetObjectiveStatus`, Kirby and J Kirby run the vanilla block unless the hat id (unsigned) is 0x20 or more; with that Dedede hat, and Dedede himself, the CPU presses B (routine 0x09) in NSP_IDLE_GROUND / NSP_FALL (Dedede 0xEC, 0xF4; Kirby hat 0x170, 0x178) |
| `jigglypuffkirbyshared.multi_jump_recovery_fix_` (813), `remix_skip_offensive_usp_air_` (860) | 0x80135198, 0x80133F70 | the Puff/Kirby walk branch (`ftRemixAiWalkUsesMultiJumpBranch`) covers Kirby, Purin, JPuff, EPuff, Marina and Dedede; a Dedede in USP_MOVE (0xE5) leaves the walk objective. The asm also lists J Kirby (compared through `t6`, which holds the top joint pointer there), N Marina and N Dedede (compared through a register already overwritten): those never match and are not ported. JPuff and EPuff join the Purin USP skip (the vanilla 90 percent roll stays) |
| `yoshishared.yoshi_cpu_fix_1` (278) | 0x8013558C | `ftRemixAiRecoveryMayUseUsp`: J Yoshi does not use the up special to recover; Marina uses it only when `jumps_used == 6` |

### 8b. Character.recovery_logic routines

`custom_recovery_logic` calls the fighter's `recovery_logic` entry after the walk step of the Recover objective. Port: `ftRemixAiRecoveryLogicRunFighter` reads the entry from the image, finds the registering fighter with the same entry (Mario, Pikachu, Jigglypuff, Yoshi, Ness, Captain, Link, Fox, Banjo, Wario, Marth, Crash, Dedede, Falco, Sheik, Marina, Sonic, Peach; clones copy the entry), fills `FTRemixAiRecoveryLogicState` and applies the `FTRemixAiRecoveryLogicResult` (target, stick, buttons, then `ftComputerSetCommandImmediate`; a result with `roll_modulus` applies only when a draw in [0, modulus) is 0). All routines read the nearest of `cliff_left_pos` / `cliff_right_pos` by X distance (left on a tie). Level 10 only: Mario family, Pikachu, Jigglypuff, Yoshi and Ness, Captain, Link, Fox; the others run at any level.

| Family | Behaviour |
|---|---|
| Mario, Luigi, Dr. Mario (+J) | tornado in the air: mash B on odd screen frames, point the stick at the ledge until frame 50; 2000 above the ledge: NSP towards; tornado not yet used and farther than 2560 in X with the ledge above or level: DSP |
| Pikachu | first aerial zip: aim 25 degrees above the line to the target (within 2000 point at it); second zip and end: point the stick at the target |
| Jigglypuff, JPuff, EPuff | Pound in the air: hold up and toward the ledge; farther than 1000 and 2000 above the ledge, or in a mid jump (JumpAerialF, Jump2 to Jump5): NSP towards |
| Yoshi, JYoshi, Ness, JNess, Lucas | after the double jump while rising: DSP when the grab box (attributes 0xB0 used for X and Y by the asm) misses the ledge |
| Captain, JFalcon, Dragon King | Falcon Punch in the air: hold up and toward the ledge; right after the double jump at max air speed, farther than 4000 and 100 above the ledge: NSP towards |
| Link, JLink, ELink | no item, farther than 2000 and 2000 above the ledge: DSP towards the ledge |
| Fox, JFox | not in Fire Fox and falling: within 4000 (2000 with no jumps) of the ledge, a 1 in 20 roll presses DSP |
| Banjo | not in FAIR or a jump command, facing the expected way, falling, farther than 1800 and not too far above the ledge: FTILT with stick Y 0 (any level) |
| Wario, Marth and Roy, Crash, Dedede, Falco, Sheik, Marina, Sonic, Peach | see `ftremixairecoverylogicfighterrules.c` (any level) |

## 9. Reflect, absorb and the game modes

`Reflect.asm` (AI scopes 54-480, with the `fighter_reflect` byte table) teaches the CPU to reflect or absorb. Seven patches, all in the
hazard check of `ftComputerProcDefault` and the attack picker: `extend_reflect_absorb_character_check_` (0x80133850, do not shoot at
an opponent that can reflect/absorb), `set_projectile_detection_radius_` and `set_item_detection_radius_` (0x80135C40, 0x80135ED4;
Marina's slow absorb uses radius 25 instead of 15), `extend_projectile_reflect_initial_` / `extend_item_reflect_initial_`
(0x80135E90, 0x80136134; set the CPU reflect flag for JFox, Falco, Slippy, Wolf (reflect) and JNess, Lucas, NLucas, Piano, Marina
(absorb); Sheik needles and Conker nuts are not absorbed), `maintain_reflect_input_` (0x8013782C) and `apply_reflect_input_`
(0x80138058; press the reflector or absorb special, keep holding it while its absorb actions 0xE5-0xED (Piano) or the Marina absorb range
run).

Port (`ftremixaireflect.c`, rules in `ftremixaireflectrules.c`, table reads from the RAM image): `ftRemixAiReflectDetectionFrames` (projectile window 15, Marina 25), `ftRemixAiReflectProjectileRaisesFlag` (Fox, N Fox and Ness always; JFox, Falco, Slippy, Wolf; JNess, Lucas, NLucas, Piano and N Ness unless the projectile is a Sheik needle 0x23 or Conker nut 0x24; Marina unless she holds an item or her charge is 4), `ftRemixAiReflectItemRaisesFlag` (Fox, N Fox and Ness, then the `fighter_reflect` table, Marina as above), `ftRemixAiReflectMaintainInput` (ProcDefault: release B while in the Fox or Ness down special scope, Piano actions 0xE5-0xED, Marina 0xFE-0x109) and `ftRemixAiReflectApplyInput` (CounterAttack: press the down special when outside that scope). `set_item_detection_radius_` (0x80135ED4) is a no-op: it loads 9.0 (the `/ 9.0F` chance divisor, not a radius) and compares the Marina id with `s7`, which is not the character id there.

Mode targets are in section 5 (`ai_hill_behavior_`, `ai_ball_behavior_`, `ai_ball_attack_range`). Training-mode CPU behaviour (menu tech, DI, spam,
shield break) uses the tech and DI hooks of section 7.

## 10. Deviations from the asm

| Area | Deviation | Reason |
|---|---|---|
| TURNAROUND_WAIT | waits on Turn or TurnRun | the asm's first `beql` annuls the TurnRun load, so TurnRun is never detected; the comment states the intent |
| Polygon test (`extend_polygon_check_`) | uses the roster's polygon variants (0x0E-0x19 and 0x4D-0x60) | the asm range `NUM_CHARACTERS - NUM_POLYGONS` starts at 0x41 and would also cover Goemon, Peppy, Banjo, Roy, Lanky, ...; the `< FALCO` side includes GDonkey, random, none |
| `clamp_target_pos_to_stage` | pulls a target back only when it lies beyond an edge +-300; reads `gMPCollisionBounds.current` | the asm's `bc1fl` clamps targets that are inside and reads `0x28/0x2C(s3)` (current right/left); the port follows the intent stated by the asm comments |
| `fix_movement_prediction` | the predicted X keeps the vanilla `* lr` | detect boxes are flipped by facing; the asm omits the factor |
| `prevent_fastfall` | with no target the `floor_dist` test still applies | the asm fast falls at once |
| classic mode | `nSCBattleGameType1PGame` | stands for `GAMEMODE.CLASSIC` of the vanilla 1P/RTTF |
| Single Button Mode in tech | blocks CPUs only | the port follows the asm comment "don't let CPU tech"; the asm block runs before the human/CPU split and blocks every player |
| `weight_attack_options` | the combo read at N64 0x40 is `hitlag_tics` | by `FTStruct` layout; the asm comment calls it the hitstun timer |
| `_custom_weight_table` order | applied once in the accumulate loop, after the 0.25 repeat factor | equal for linear routines |
| `extend_cliff_attack_` | keeps the always-CLIFF_LET_GO result of `rand(2) >> 1` | the asm's literal behaviour |
| `ftRemixAiPreventCandidate` | the Remix routines are identified by matching table entries with their owners, not by symbol | the routines are Remix code addresses without symbols; they are ported as C |
| WARIO PREVENT | vetoes ids 0x4D/0x4E (Wario's NSP_TOWARDS routines) | the asm literals 0x42/0x43 are the generic NSP_TOWARDS and FAIR scripts while its comments say Wario NSP towards; the port follows the comment |
| Crash PREVENT | soft platform = `coll_data.floor_flags & MAP_VERTEX_COLL_PASS` | N64 surface halfword 0x00F6 bit 0x4000 |
| `recovery_logic` hold-up routines | Jigglypuff Pound, Falcon Punch and Wario Body Slam hold toward the nearest ledge | the asm compares `f14 < f0` before either is computed (stale registers); the nearest ledge side is the evident intent |
| `_find_closest_ledge` | with no candidate the ledge is (0, 0) and not grabbable | the asm leaves `is_grabbable` uninitialized |
| `_improve_recovery_drift` flag byte | tests and clears all eight flags of byte 0x49 of the computer struct | the asm reads `lb` / `sb r0` of the whole byte, not just `is_attempt_specialhi_recovery` |
| `CpuLevel9` enhancement | overrides level 10 when on | port-only convenience setting |
| `extend_reflect_absorb_character_check_` | the port drops the AutoX B candidate when the target is in `fighter_reflect` and the own `ai_aerial_chase` entry is CHASE (the vanilla meaning with the table in place of Ness/Fox) | read literally, the asm jumps to 0x801338A0 (the continue path, identical to NORMAL) for a reflector and runs the own-fighter table for everyone else, which would invert the vanilla rule for the seven fighters whose entry is CHASE; the labels (`_dont_shoot`) and comments state the vanilla meaning. `ftRemixAiReflectRulesSkipsNspCandidate` is the one place to flip |
| `apply_reflect_input_` | NLucas is not handled | the asm compares LUCAS twice where the second is labelled NLUCAS |
| `ai_ball_behavior_` | the side offset uses the Smashketball home team | the asm compares the fighter team with `ITStruct` word 0x68, which is `vel_push.y` of the collision data (the comment says home team); that is 0 in practice, so the asm treats team 0 as the home team |
| `ai_hill_behavior_` | the target Y is the marker Y minus 500 | the asm loads the Y of both joints from the X offset (0x1C twice), giving Y = X - 500; the port follows the commented intent |

Open gaps: `docs/remix/patch_audit.md`, groups g7 (CPU hooks of AI.asm and fighters), g5 (Reflect) and g8 (modes).
