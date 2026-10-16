# F7 Size phase (b): movement physics

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm` lines 563-842. Each patch is the same arithmetic as the asm,
`x = x * (1 + coefficient * (size - 1))`, through `ftRemixSizeAttrMultiplier` and the `FTREMIX_SIZE_COEF_*` constants.
Every helper returns the vanilla value while the multiplier is 1.0F.

## Files

- New: `decomp/src/ft/ftremix/ftremixsizemove.c` (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`).
- Appended block: `decomp/src/ft/ftremix/ftremixhooks.h` (Size phase b movement block, after the weight block).
- Vanilla edits (each under `#ifdef REGION_REMIX`, vanilla line kept in `#else`):
  `decomp/src/ft/ftphysics.c`, `decomp/src/ft/ftcommon/ftcommondash.c`, `ftcommonrun.c`, `ftcommonkneebend.c`,
  `ftcommonjump.c`, `ftcommonjumpaerial.c`.

## Patches

| Asm scope | RAM (asm) | Vanilla function (file) | Status |
|-----------|-----------|-------------------------|--------|
| adjust_player_walking_speed_ | 0x800D8A74 | ftPhysicsSetGroundVelAbsStickRange (ftphysics.c) | applied: walk speed and brake force (friction arg) |
| adjust_player_initial_dash_speed_ | 0x8013ED40 | ftCommonDashSetStatus (ftcommon/ftcommondash.c) | applied: dash_speed |
| adjust_player_dash_deceleration_ | 0x800D897C | ftPhysicsSetGroundVelFriction (ftphysics.c) | applied: friction arg (every caller, as the asm patch does) |
| adjust_player_running_speed_ | 0x8013EF14 | ftCommonRunSetStatus (ftcommon/ftcommonrun.c) | applied: run_speed |
| adjust_player_jump_squat_ | 0x8013F308 | ftCommonKneeBendProcUpdate (ftcommon/ftcommonkneebend.c) | applied: jump squat length, +1 frame when size multiplier > 1.0F |
| adjust_jumping_height_multiplier_1 | 0x8013F94C | ftCommonJumpSetStatus (ftcommon/ftcommonjump.c) | applied: jump_height_mul and jump_height_base |
| adjust_jumping_height_multiplier_2 | 0x8013FE64 | ftCommonJumpAerialSetStatus (ftcommon/ftcommonjumpaerial.c, first aerial formula) | applied: jump_height_mul, jump_height_base, jumpaerial_height (second jump coefficient). Pwing 0.8 skipped, see U1 |
| adjust_jumping_height_multiplier_kirby | 0x80140014 | ftCommonJumpAerialMultiSetStatus (ftcommon/ftcommonjumpaerial.c, jumps_used == 1 branch) | applied: same formula as _2 without the Pwing factor (the asm has none). Not gated by fkind, as the asm is not |

## Uncertainties and REMIX-TODO

- U1. `REMIX-TODO(Pwing)`: `adjust_jumping_height_multiplier_2` (Size.asm 0x8013FE64 hook) multiplies the second
  jump height by 0.8 while `Item.Pwing.pwing_jump_flag` is set for the player (Size.asm ~345-355 of the `_2` scope,
  `_check_pwing`). The port has `itremixpwing.h` but no accessor for that flag, so the 0.8 factor is not applied.
- U2. The Kirby hook (0x80140014) was mapped to the `jumps_used == 1` branch of
  `ftCommonJumpAerialMultiSetStatus` (`vel_air.y = ((stick*mul)+base)*jumpaerial_height`), since its
  `cvt.s.w` and `lwc1 0x48` match that line. The table branch (`dFTKirbyJumpAerialFVelocities`) is untouched.
  Verify once `baserom.us.z64` is available.
- U3. The vanilla function mapping comes from `build/remixasm/refs/vanilla_funcs.txt`, whose line numbers are stale (see
  f7_size_a.md U2). Function names were matched by address range; the jump squat address 0x8013F308 falls in
  `ftCommonKneeBendProcUpdate` because the jump squat is the knee bend status.
- U4. `ftPhysicsSetGroundVelFriction` and `ftPhysicsSetGroundVelAbsStickRange` are patched for every caller, as the asm
  patch is at the function entry. Non-walk callers of `AbsStickRange` (if any) get the walk and brake scale too.

## Not verified

No build, no game run (per brief). `scripts/remix/syntax-check.sh` returned `syntax-check OK` for every touched file.
With multiplier 1.0F every patch is a no-op except the float expression order in the jump height lines, which is
unchanged (`x * 1.0F` is exact).
