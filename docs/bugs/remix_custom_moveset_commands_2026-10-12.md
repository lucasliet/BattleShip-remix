# Remix custom moveset commands 0xD0-0xFF were dropped

Code: `decomp/src/ft/ftremix/ftremixcommand.{c,h}` (the commands and their per-player state),
`port/remix/remix_command_hooks.cpp` (event listeners, registered next to the focus listener in `port/port.cpp`), call sites in
`decomp/src/ft/ftmain.c`, `ftparam.c`, `ftphysics.c`. Test: `vs.sh 29,33 <input-script> <name>` (Falco vs Wario) with a
script that presses B on the ground (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

With Falco's Phantasm routines ported (`decomp/src/ft/ftremix/falco/`), the aerial Phantasm dashed correctly, but the grounded
one played its pose and afterimage without moving. `motion_vars.flags.flag2` stayed at 1 for the whole action: the moveset
script never reached its `5C000002` ("begin movement") command.

## Root cause

Smash Remix extends the moveset microcode with custom commands at opcode bytes 0xD0-0xFF (`src/command.asm`). It hooks the
three motion script runners:

- `load_command_` at 0x800E0388, inside `ftMainUpdateMotionEventsAll`;
- `load_command_2_` at 0x800E0548, inside `ftMainUpdateMotionEventsForward`;
- `load_command_2_` at 0x800E072C, inside `ftMainUpdateMotionEventsForwardEffect`.

The port's runners only knew the vanilla opcodes (up to 0x33). For a Remix command:

- `ftMainParseMotionEvent` hits no case and never advances the script, so the runner spins until the PORT watchdog kills the
  script after 256 iterations;
- `ForwardEffect` advances one word, which misaligns 8-byte commands.

Phantasm's ground script starts with `D0013F00` (frame speed 0.5), so the script died before its `5C000002`. Most Remix
movesets use these commands.

## Fix

`ftremixcommand.c` (REGION_REMIX) implements commands 0xD0-0xDC with Remix's two dispatch tables. The fast-forward table skips
y velocity, fast fall and direction switch, and skips random sfx, env color and taunt voice as 8-byte commands. Unassigned
slots advance 4 bytes, as in Remix. Each runner calls `ftRemixCommandCheckRun` before decoding the opcode.

The commands keep per-player state, which Remix also patches into vanilla code:

| Remix patch | Port site |
|---|---|
| `change_action_` 0x800E6F2C (reset multiplier and env color) | `ftMainSetStatus` -> `ftRemixCommandResetStatus` |
| `end_hitbox_` 0x800E852C | `ftParamClearAttackCollAll` -> `ftRemixCommandResetAttackAll` |
| `create_hitbox_` 0x800DF1F0 | `ftMainParseMotionEvent` (make attack) and the `FighterHitboxSlotResetEvent` listener |
| `apply_direction_` 0x800E446C | `ftMainProcessHitCollisionStatsMain` and the `FighterDamageDirectionApplyEvent` listener |
| `override_env_color_` | `FighterEnvColorQueryEvent` listener |
| `apply_fgm_` 0x800E2CA8 | `ftMainPlayHitSFX` -> `ftRemixCommandPlayHitFGM` |
| `apply_ground_{x,z}_` / `apply_air_{x,y,z}_` (0xD3 multiplier) | `ftPhysicsApplyGroundVelTransN`, `ftPhysicsGetAirVelTransN` |
| `texture_form_fix_` 0x800E962C (Metal Mario) | `ftParamSetTexturePartID` |

## Verification

A scripted VS run (Falco vs Wario, B pressed on the ground) shows the grounded Phantasm crossing the platform and hitting
Wario; the aerial Phantasm still works and the log has no watchdog lines.

Open gaps: `docs/remix/patch_audit.md`, group g5 (the training-mode action frame counter reset of `command.asm` is not driven
from `ftRemixCommandResetStatus`).
