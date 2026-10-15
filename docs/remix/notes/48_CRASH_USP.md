# 0x48 CRASH: CrashUSP notes

Port file: `decomp/src/ft/ftremix/crash/ftremixcrashusp.c`, table `dFTRemixCrashUSPProcs` (6 entries, `dFTRemixCrashUSPProcsNum`). Source: `.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm` lines 1050-1304 and `Crash.asm` lines 613-651.

Not yet in `ftremixported.c`: the file-level table is exported, but `ftremixcrash.c` still has its empty `dFTRemixCrashProcs` table. Per the brief, the lead wires this in.

## Routines ported (6 of 6 in scope)

| Remix label | Port routine | Role |
|---|---|---|
| `CrashUSP.air_initial_` | `ftRemixCrashUSPAirInitial` | special entry, aerial |
| `CrashUSP.change_direction_` | `ftRemixCrashUSPChangeDirection` | interrupt |
| `CrashUSP.collision_` | `ftRemixCrashUSPProcMap` | collision |
| `CrashUSP.ground_initial_` | `ftRemixCrashUSPGroundInitial` | special entry, grounded |
| `CrashUSP.main_` | `ftRemixCrashUSPProcUpdate` | update |
| `CrashUSP.physics_` | `ftRemixCrashUSPProcPhysics` | physics |

File-local helpers (static, not in the table): `ftRemixCrashUSPBeginStatus`, `ftRemixCrashUSPLandingInitial` (`CrashUSP.landing_initial_`), `ftRemixCrashUSPAirControl` (`CrashUSP.air_control_`), `ftRemixCrashUSPAirFriction` (`CrashUSP.air_friction_`).

## 1. Vanilla patches

None are needed for CrashUSP's own routines. The USP entries reach the port through the bind:

- `Crash.asm` 645-651: `table_patch_start(ground_usp / air_usp)` writes `CrashUSP.ground_initial_` and `CrashUSP.air_initial_` into the special entry tables. `ftRemixBindSpecials` covers these through the Remix addresses in the table.
- `Crash.asm` 624-626: `add_new_action` for USPG, USPA and USPLanding. USPLanding uses vanilla main `0x800D94C4`, interrupt `0`, physics `0x800D8BB4` and collision `0x800DDEE8`. These are vanilla routines, so nothing to port.

No vanilla function needs a `#ifdef REGION_REMIX` insertion for this scope.

## 2. Shared Remix subsystems

- `PokemonAnnouncer.body_slam_announcement_`, called at the end of `CrashUSP.main_` (`CrashSpecial.asm` line 1171, `jal` with `nop` in the delay slot). Skipped with `/* REMIX-TODO(PokemonAnnouncer): ... */` in `ftRemixCrashUSPProcUpdate`. The call only runs after the jump is armed (`flags.flag0` set).

## 3. Identifications made from vanilla code

- `0x800E6F24` = `ftMainSetStatus(gobj, id, 0.0F, 1.0F, 0)`. The asm sets `a2 = 0`, `a3 = 1.0` and the fifth argument to 0.
- `0x800E0830` = `ftMainPlayAnimEventsAll`.
- `0x800DEE98` = `mpCommonSetFighterGround(fp)`, called with the player struct.
- `0x800DEEC8` = `mpCommonSetFighterAir(fp)`, called with the player struct.
- `0x800DDF44` = `mpCommonSetFighterFallOnEdgeBreak(gobj)`.
- `0x800DE80C` = `mpCommonProcFighterCliff(gobj, proc)`, identified by size and by the existing Wario port (`ftremixwario.c` line 395 calls it with an `AirToGround` proc). It has no label in `vanilla_funcs.txt`.
- `0x80160370` = `ftCaptainSpecialHiProcInterrupt` (turn subroutine).
- `0x800D8D68` = `ftPhysicsApplyGravityClampTVel(fp, gravity, tvel)`.
- `0x800D8FA8` = `ftPhysicsCheckClampAirVelXDecMax(fp, attr)`.
- `0x800D8FC8` = `ftPhysicsClampAirVelXStickRange(fp, stick_min, vel, clamp)`.
- `0x800D9074` = `ftPhysicsApplyAirVelXFriction(fp, attr)`.
- `0x800D9044` = `ftPhysicsClampAirVelXStickDefault`, which the air control replaces.
- `0x800D90E0` = `ftPhysicsApplyAirVelDrift` (vanilla). `physics_` copies 8, 4 and 6 instructions from ROM `0x548E0`, `0x54924` and `0x54948`. Decoded with `tools/remix_disasm_rom.py` and ported as C: prologue (fp, attr), the gravity call, the `bnez` decel check, then air control and friction.

## 4. Decisions that encode game behaviour

- Temp variables: `0x17C` = `flags.flag0`, `0x180` = `flags.flag1`, `0x184` = `flags.flag2`, as in the brief.
- Fast fall reset in `air_initial_`: the mask `0x07` on the byte at `0x18D` clears the first five MSB bits, which are `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall` (from `struct_offsets.txt`). This matches the Falco pilot.
- `physics_` skip: the copied `bnez v0` at `0x54930` targets vanilla `0x800D914C` (the epilogue, right after the vanilla friction `jal` at `0x54944` and its delay slot). Crash's copy keeps the same relative offset, so its target is the epilogue after the `air_friction_` call. When temp variable 2 is set and `ftPhysicsCheckClampAirVelXDecMax` returns nonzero, both `air_control_` and `air_friction_` are skipped. The port returns in that case.
- `air_friction_`: vanilla `0x800D9074` reads only `attr->air_friction` (`0x54`), so the stack attribute is an `FTAttributes` with that field set. Its other fields are zero.
- `collision_`: the `bnez v0` on kinetic state `0x14C` maps to `fp->ga != nMPKineticsGround` (the grounded branch is `nMPKineticsGround == 0`, the first entry of `MPKinetics` in `mpdef.h`).

## 5. Constants

Each float constant is written as its exact decimal value, matching the raw `lui` bits:

- `CRASH_USP_AIR_ACCEL` = `0x3DA40000` = 0.080078125F (the asm comment says 0.08, not exact)
- `CRASH_USP_AIR_ACCEL_2` = `0x3D4D0000` = 0.050048828125F (the asm comment says 0.05)
- `CRASH_USP_AIR_INITIAL_Y_SCALE` = `0x3D480000` = 0.048828125F (the asm comment says 0.05)
- The others are exact (56, 120, 1.5, 18, 48, 20, 2.0).

## 6. Uncertainties

1. Action ids `nFTRemixCrashStatusUSPG = 0xEB`, `USPA = 0xEC` and `USPLanding = 0xED` come from `string_0x0EB..ED` ("BellyFlopGround", "BellyFlopAir", "BellyFlopLanding") in `Crash.asm` lines 62-64, which match the order of `add_new_action` (after the six DSP actions at 0xE5-0xEA). Not checked against the built ROM.
2. `ftRemixCrashUSPAirFriction` uses a zero-initialised `FTAttributes`. The vanilla function must not read other fields; it does not in the decomp (`ftphysics.c` line 339).
3. `0x800DE80C` is inferred from size and from the Wario port, not from a vanilla label.
4. `ftRemixCrashUSPProcUpdate` writes `fp->jumps_used = fp->attr->jumps_max`, an implicit `s32` to `u8` narrowing, matching the asm `sb`.
5. `landing_initial_` has commented-out temp variable resets (`CrashSpecial.asm` lines 1144-1145). They are dead in the source, so the port omits them.
