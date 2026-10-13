# Remix custom moveset commands 0xD0–0xFF were dropped (2026-10-12)

## Symptom

After Falco's Phantasm routines were ported (`decomp/src/ft/ftremix/falco/`),
the aerial Phantasm dashed correctly, but the grounded one played its pose and
afterimage without moving. A debug log showed `motion_vars.flags.flag2` stuck
at 1 for the whole action. The moveset script never reached its `5C000002`
("begin movement") command.

## Root cause

Smash Remix extends the moveset microcode with custom commands at opcode bytes
0xD0–0xFF (`src/command.asm`). It hooks the three motion script runners:

- `load_command_` at 0x800E0388, inside `ftMainUpdateMotionEventsAll`;
- `load_command_2_` at 0x800E0548, inside `ftMainUpdateMotionEventsForward`;
- `load_command_2_` at 0x800E072C, inside `ftMainUpdateMotionEventsForwardEffect`.

The port's runners only know the vanilla opcodes (up to 0x33). For a Remix
command (opcode `0x34+`):

- `ftMainParseMotionEvent` hits no case and never advances the script, so the
  runner spins until the PORT watchdog kills the script after 256 iterations;
- `ForwardEffect` advances one word. That misaligns 8-byte commands.

Phantasm's ground script starts with `D0013F00` (frame speed 0.5), so the
script died before its `5C000002`. Most Remix movesets use these commands.

## Fix

`decomp/src/ft/ftremix/ftremixcommand.c` (REGION_REMIX) implements commands
0xD0–0xDC with Remix's two dispatch tables. The fast-forward table skips y
velocity, fast fall and direction switch, and skips random sfx, env color and
taunt voice as 8-byte commands. Unassigned slots advance 4 bytes, as in Remix.
Each runner calls `ftRemixCommandCheckRun` before decoding the opcode.

The commands keep per-player state, which Remix also patches into vanilla code:

| Remix patch | Port site |
|---|---|
| `change_action_` 0x800E6F2C (reset multiplier and env color) | `ftMainSetStatus` |
| `end_hitbox_` 0x800E852C | `ftParamClearAttackCollAll` |
| `create_hitbox_` 0x800DF1F0 | existing `FighterHitboxSlotResetEvent` listener |
| `apply_direction_` 0x800E446C | existing `FighterDamageDirectionApplyEvent` listener |
| `override_env_color_` | existing `FighterEnvColorQueryEvent` listener |
| `apply_fgm_` 0x800E2CA8 | `ftMainPlayHitSFX` |
| `apply_ground_{x,z}_` / `apply_air_{x,y,z}_` (0xD3 multiplier) | `ftPhysicsApplyGroundVelTransN`, `ftPhysicsGetAirVelTransN` |
| `texture_form_fix_` 0x800E962C (Metal Mario) | `ftParamSetTexturePartID` |

The listeners live in `port/remix/remix_command_hooks.cpp` and are registered
next to the focus listener in `port/port.cpp`.

Not ported:

- Remix's size items (`Size.multiplier_table`, always 1.0 without them);
- the training-mode action counter reset;
- `Item.respawn_with_item_` in `change_action_`.

## Verification

The scripted VS run (`build/agent-shots/falco3`, Falco vs Wario, B at frame
780) shows the grounded Phantasm crossing the platform and hitting Wario for
7%. The aerial Phantasm still works, and the log has no watchdog lines.
