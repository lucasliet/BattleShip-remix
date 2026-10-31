# F14 audit: SingleButtonMode.asm and SinglePlayerEnemy.asm

Source files: `.toolchain/pkgs/smashremix-src/src/SingleButtonMode.asm` and `.toolchain/pkgs/smashremix-src/src/SinglePlayerEnemy.asm` (both exist).
RAM addresses resolved through `build/remixasm/refs/vanilla_funcs.txt`. The file lists only function starts, so the
"containing function" column is the nearest preceding start and is approximate where marked (`approx`).

## SingleButtonMode.asm

| # | Patch | Effect | RAM | Port location | Constants check | Status |
|---|-------|--------|-----|---------------|-----------------|--------|
| S1 | `_check_toggle` entry, jumped from the CSS dpad hook `css_cursor_and_dpad_controls_` (RAM 0x80004000) | Gates the held/pressed filter on screen, pause and Training menu | 0x80004000 | `decomp/src/ft/ftmain.c:1469` calls `ftRemixSingleButtonModeFilter(..., FTREMIX_SBM_GATE_OPEN)` | Filter masks (A/B/R, START, dpad 0x0F00, C 0x000F, Z 0x2000) match | MISMATCH: the caller always passes GATE_OPEN. The Training menu gate is never computed. Fix: `ftremixsbmfix_training_gate.c` |
| S2 | `handle_cpu_inputs_` A press | CPU A becomes the single button; R sends A+Z | 0x80131CFC (approx, nearest start mnTitleInitVars) | `decomp/src/ft/ftcomputer.c:3482`, in `ftComputerUpdateInputs` (line 3451) | Joypad.A / B / A+Z match `ftRemixSingleButtonCpuAOrBPress` | MATCH |
| S3 | `handle_cpu_inputs_` B press | Same as S2 for B | 0x80131D24 (approx) | `ftcomputer.c:3494` | same | MATCH |
| S4 | `handle_cpu_inputs_` Z press | CPU Z removed while a mode is active | 0x80131D44 (approx) | `ftcomputer.c:3506` | Z_TRIG | MATCH |
| S5 | `handle_cpu_inputs_` L press | CPU L removed while a mode is active | 0x80131D64 (approx) | `ftcomputer.c:3518` | L_TRIG | MATCH |

Notes on S1:
- The Training menu branch (`_check_pause_training`) keeps START and, only for mode B, B. This matches `ftRemixSingleButtonFilterTrainingMenu`.
- The pause gate (state 2 closes, 0, 1 and 3 open) and the screen list are not evaluated in the port. ftmain only runs for fighters, so this is mostly harmless. Still undocumented in the port.
- The source of `Training.toggle_menu` is missing in the port (see `decomp/src/sc/sc1pmode/sc1ptrainingremix.h:53`). The fix returns OPEN for any value below 2, so calling it with 0 keeps today's behaviour.

## SinglePlayerEnemy.asm

Port equivalent: none. `grep -i enemy` in `decomp/src` finds no 1P enemy selection, `enemy_port`, `afk` tag or "Enemy Player" label. `sc1PRemix1PSetupEnemy` in `sc1premix1p.c` is the Remix 1P enemy setup, not this feature.

| # | Patch | Effect | RAM | Port location | Constants check | Status |
|---|-------|--------|-----|---------------|-----------------|--------|
| E1 | `onep_control_port` (ROM 0x10E350) | Computer 1 becomes the human enemy port (spawn 0x25, not HRC) | 0x8018FAF0 (approx, nearest start func_ovl7_8018FA54, overlay) | MISSING | HRC_ID = 6 (`SC1PREMIX_MODE_HRC`) | MISSING. Fix: `ftremixenemyfix_control.c` |
| E2a | `_afk_1p` stage clear (ROM 0x17DC68) | AFK advances the stage clear screen | 0x80134A18 (approx, sc1PIntroSetupFighterFiles) | MISSING | tag 0x61666B00 | MISSING. Fix: `ftremixenemyfix_afk.c` |
| E2b | `_afk_1p` preview (ROM 0x12DC6C) | AFK advances the preview | 0x8013492C (approx) | MISSING | 0x61666B00 | MISSING |
| E2c | `_afk_1p` continue (ROM 0x17A588) | AFK continues | 0x80133B28 (approx) | MISSING | 0x61666B00 | MISSING |
| E2d | `_afk_1p` bonus time up (ROM 0x1129E8) | AFK forces time up | 0x8018E2A8 (exact start sc1PBonusStageTimeUpProcUpdate) | MISSING | 0x61666B00 | MISSING |
| E2e | `_afk_1p` RTTF (ROM 0x86F4C) | AFK takes the complete branch | 0x8010B74C (approx, grBonus3FinishProcUpdate) | MISSING | 0x61666B00, branch 0x0E | MISSING |
| E2f | `_afk_1p._hmn_check` (called from E1) | AFK player spawn 0x21 becomes a level 10 CPU | (inside E1) | MISSING | spawn 0x21, level 10 | MISSING, in `ftremixenemyfix_afk.c` |
| E3 | `onep_control_respawn` (ROM 0x10CD88) | Team respawn of Computer 1 gets the enemy port input table (no HRC check) | 0x8018E528 (approx, scExplainFuncLights) | MISSING | stride 10 | MISSING. Fix: `ftremixenemyfix_control.c` |
| E4 | `onep_control_shield` (ROM 0x7C964) | CPU in 1P with an enemy takes its own port colour | 0x80101164 (efManagerShieldMakeEffect, range 0x80101108-0x80101180) | MISSING | stride 10 | MISSING. Fix: `ftremixenemyfix_control.c` |
| E5 | `setup_` / `enemy_routine_` / `onep_routine_`..`fourp_routine_` (Render.asm:2379) | CSS "Enemy Player" and "Press Z" labels, blink and port arrows | n/a (Render hook) | MISSING | blink period 0x50, shown < 0x29 | MISSING. Fix: `ftremixenemyfix_label.c` |
| E6 | `z_button` (ROM 0x14029C) | Z on the 1P CSS selects the enemy port; laugh FGM 0x1ED on first selection | 0x8013809C (approx, mnPlayersVSRecallPuck) | MISSING | FGM 0x1ED, Z mask 0x2000, START 0x1000, playing port at 0x80138FA8 | MISSING. Fix: `ftremixenemyfix_selection.c` |
| E7 | `b_button` (ROM 0x13EF28) | B on another port clears the enemy | 0x80136D28 (approx, inside function starting 0x80136D04 in mnplayers1pgame.c:2935) | MISSING | B 0x4000, port inputs at 0x8004522A + 10*i | MISSING. Fix: `ftremixenemyfix_selection.c` |

## Fix files (new, under `decomp/src/ft/ftremix/`, guarded by `#if defined(PORT) && defined(REGION_REMIX)`)

| File | Covers | Hook the lead must add |
|------|--------|------------------------|
| `ftremixsbmfix_training_gate.c/.h` | S1 Training menu gate | `decomp/src/ft/ftmain.c:1469`: replace `FTREMIX_SBM_GATE_OPEN` with `ftRemixSbmFixTrainingGate(<Training.toggle_menu>)` on the Training mode screen. Needs the toggle_menu source first |
| `ftremixenemyfix_selection.c/.h` | E6, E7 | CSS 1P Z and B handling. Z: call `ftRemixEnemySelectionOnZ`, play FGM 0x1ED on TRUE. B: `ftRemixEnemySelectionOnB`. Exact decomp line not located (see uncertainties) |
| `ftremixenemyfix_afk.c/.h` | E2a-E2f | Stage clear, preview, continue, bonus and RTTF button or result checks, plus the HMN-to-CPU step. Decomp lines not located |
| `ftremixenemyfix_control.c/.h` | E1, E3, E4 | Spawn of Computer 1, team respawn, shield colour. Candidate sites: spawn setup near 0x8018FAF0, respawn near 0x8018E528, `efManagerShieldMakeEffect` at `decomp/src/ef/efmanager.c:4206` |
| `ftremixenemyfix_label.c/.h` | E5 | CSS label draw, no site located |

Nothing in the existing files was edited.

## Uncertainties

1. `vanilla_funcs.txt` lists function starts only and contains overlay aliases, so the approx containing functions may be wrong. The shield (E4) is the only exact one.
2. The B loop in E7 derives `t8` from `t7`, whose source was not traced. `skip_port` is a parameter, not derived.
3. E1 sits in an overlay (ovl7 range), where the approx location is not verified.
4. The Remix `_check_toggle` entry (S1) is reached from the CSS dpad hook at RAM 0x80004000. The port applies the filter in ftmain instead, so the gate logic differs in place, not just in location.
