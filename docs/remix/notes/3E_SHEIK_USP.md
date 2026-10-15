# 0x3E SHEIK USP (Vanish) port notes

Port: `decomp/src/ft/ftremix/sheik/ftremixsheikusp.c`, table `dFTRemixSheikUSPProcs`
(13 entries, `dFTRemixSheikUSPProcsNum`). Source: `.toolchain/pkgs/smashremix-src/src/Sheik/SheikSpecial.asm`
lines 15-727 (scope SheikUSP), actions from `Sheik.asm:114-119` (USPG_BEGIN 0xE4 .. USPA_END 0xE9).

The table is not yet referenced from `ftremixported.c`. `ftremixported.c` already names
`dFTRemixSheikProcs`, and `ftremixsheik.c` (NSP/DSP owner) holds that table, so the lead must merge the
two tables (or add a `dFTRemixSheikUSPProcs` entry to `dFTRemixPortedProcTables`).

## 1. Vanilla patches

None. `SheikUSP` contains no `OS.patch_start` blocks and no `j`/`jal` hooks into vanilla code.
The special tables that point at `SheikUSP.*_initial_` (Sheik.asm:471-479) are Remix table patches
handled by `ftRemixBindSpecials`, not vanilla code changes.

## 2. Shared Remix subsystems

- `CharEnvColor.moveset_table[port] = 0xFFFFFF00` (SheikSpecial.asm:286-291, 410-415, 452-457). Skipped with
  `REMIX-TODO(CharEnvColor)` in `ftRemixSheikUSPHideIntangible`. `is_invisible` (0x018D bit 0) is honoured by the port instead.
  Same choice as Mewtwo USP.
- `OS.copy_segment(0xD8798, 0x68)` and `OS.copy_segment(0xD8808, 0x1C)` (asm 606, 610) splice Samus' up special
  collision (0x8015DD58). Both fragments were decoded with `tools/remix_disasm_rom.py` and ported as C
  (`ftRemixSheikUSPEndProcMap`), not written as ROM bytes. No REMIX-TODO is needed, because the copied code is
  fully reproduced.
- Status vars 0x0B18 (vanish timer) and 0x0B20 (movement angle) have no typed field in `FTStatusVars`, so they are
  kept in per-port static arrays, the same as Mewtwo USP.

## 3. Uncertainties

- asm 633-635, `apply_movement_`: movement applies only when the timer is below 6
  (`slti t1, timer, 6`). Ported as an early return in `ftRemixSheikUSPApplyMovement`. The first call from
  `move_initial_` happens with timer = 12 (asm 280, delay slot of the jal), so it moves nothing.
- asm 701-718, `end_physics_`: arguments to the vanilla helpers were mapped as follows.
  - `0x800D8FC8` = `ftPhysicsClampAirVelXStickRange(fp, 8, attr->air_accel, attr->air_speed_max_x)`.
    The stick minimum `8` is passed in `a1` (asm 701, "original line"). The name
    `SHEIK_USP_END_DRIFT_STICK_MIN` is a guess at intent.
  - `0x800D8E50` = `ftPhysicsApplyGravityDefault(fp, attr)`.
  - `0x800D8FA8` = `ftPhysicsCheckClampAirVelXDecMax(fp, attr)`. Friction is skipped when it returns TRUE.
  - `0x800D9074` = `ftPhysicsApplyAirVelXFriction(fp, attr)`.
  - The header comment (asm 684) says "based on 0x800D91EC". The code calls 0x800D9074 (asm 717), and the port follows the code.
- asm 606-614, `end_collision_`: the copied Samus routine's `a1` (0x8015DD20, loaded at asm-copy offset 0x24)
  is the pass callback. The port uses `ftSamusSpecialHiProcPass`, as Mewtwo does.
  The copied fragment also contains a dead `sw v1, 0x1C(sp)` at 0x8015CFE4. It is unreachable and harmless.
- asm 550-555, `air_end_main_`: the fall special is called with `is_fall_accelerate = 1` (`a3`),
  `is_goto_landing = 0` (stack 0x10), `landing_lag = 0.5` (stack 0x14), `is_allow_interrupt = TRUE` (stack 0x18).
  Mewtwo passes a different interrupt flag and lag, so do not copy its values.
- asm 587-598, `end_invisibility_`: `is_invisible = (temp variable 3 != 0)`, matching Mewtwo.
- `air_begin_physics_` (asm, between `air_begin_collision_` and `ground_begin_transition_`) is not in the worklist
  and not referenced by any `edit_action`: USPA_BEGIN physics is vanilla `0x800D90E0`. It is not ported.
- asm 184-304, `move_initial_`: decides between Ground_Move and Air_Move from the stick, then zeroes the three
  velocities, then changes action. The order is preserved. The kinetic state used for the platform
  pass-through (asm 203, saved to 0x28(sp)) is read after the turnaround call. `ftParamSetStickLR` does not
  change the kinetic state, so the value is the same as reading it first.
- `LANDING_FSM` (0x3EBD3000) is exactly 0.3695068359375F, not 0.37. The asm comment says 0.37 and "30 frames".
  The exact binary value is used.
