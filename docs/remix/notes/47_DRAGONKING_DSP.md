# 47 DRAGONKING: DragonKingDSP notes

Source: `.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm`, scope `DragonKingDSP` (lines 959-1177).
Port: `decomp/src/ft/ftremix/dragonking/ftremixdragonkingdsp.c`, table `dFTRemixDragonKingDSPProcs` (5 routines, all ported).

## 1. Vanilla patches

None. The `DragonKingDSP` scope has no `OS.patch_start` blocks. The only patch in the file is `DragonKingUSP.drain_flash_fix_`, which belongs to the USP worker.

## 2. Shared Remix subsystems

None in this scope. There are no items, GFX or announcer calls.

## 3. Helpers and calls outside the fighter

Vanilla calls, resolved through `build/remixasm/refs/vanilla_funcs.txt`:

| RAM | Decomp function | Used by |
|---|---|---|
| 0x800E6F24 | `ftMainSetStatus` | all status setters |
| 0x800E0830 | `ftMainPlayAnimEventsAll` | ground_initial_, air_initial_, loop_initial_ |
| 0x800D9480 | `ftAnimEndCheckSetStatus` | aerial_main_ (line 1022) |
| 0x800D91EC | `ftPhysicsApplyAirVelFriction` | physics_ when temp variable 2 is set |
| 0x800D90E0 | `ftPhysicsApplyAirVelDrift` | physics_ otherwise |
| 0x800DDF44 | `mpCommonSetFighterFallOnEdgeBreak` | collision_, grounded branch |
| 0x800DE978 | `mpCommonProcFighterCliffWaitOrLanding` | collision_, aerial non-dive branch |
| 0x800DE80C | `mpCommonProcFighterCliff` | collision_, dive landing (line 1125) |
| 0x800DEE98 | `mpCommonSetFighterGround` | landing_initial_ (copied Falcon Kick code) |

`landing_initial_` (line 1163) copies Falcon Kick's landing 0x801600EC (`OS.copy_segment(0xDAB2C, 0x18)` and `0xDAB48, 0x20`). The disassembly shows the copy calls `mpCommonSetFighterGround`, then `ftMainSetStatus` with flags 0. The port follows that.

`loop_initial_` and `landing_initial_` are not in the worklist. They are reached only through transition pointers, so they are `static` helpers and are not in the table.

## 4. Uncertainties

1. **Stale f2 in physics_ (asm lines 1073-1076).** On the BEGIN and rise path (action != 0xE7 and temp variable 3 != MOVE), the asm stores `f2` to x velocity (`swc1 f2, 0x0048(a0)`), but `f2` is not set on that path. The value is whatever the physics call left in the FP register. The port skips the x write on that path, so `vel_air.x` keeps the value the physics routine wrote. If the physics routine leaves `f2` equal to its x result, this is identical. Verify in-game that the rise does not drift.
2. **Landing x scale (asm line 1169, `lui at, 0x3F00`).** The comment says 0.625, but the immediate `0x3F00` gives `0x3F000000` = 0.5F. The port uses 0.5F, following the code.
3. **Dead kinetic load in aerial_main_ (line 1020).** `lw t8, 0x014C(a2)` is never used. The port drops it.
4. **Kinetic state read.** `0x014C` is read as `fp->ga`, following `marina/ftremixmarinadsp.c`. Grounded is `ga == nMPKineticsGround`, which matches the asm `bnez v0, _aerial` at line 1098.
5. **Fast-fall byte (`0x018D`, `andi 0x07`).** Clears `is_absorb`, `absorb_lr`, `is_goto_attack100`, `is_fastfall`, and keeps `is_magnify_show`, `is_limit_map_bounds`, `is_invisible`. This matches the big-endian bit numbering and the Banjo DSP port. The asm comment "01111111" is wrong for mask 0x07.
6. **Flags.** `air_initial_` uses `FTSTATUS_PRESERVE_FASTFALL` (0x8, asm line 991). `loop_initial_` uses `FTSTATUS_PRESERVE_HIT` (0x1). The other setters use `FTSTATUS_PRESERVE_NONE`.
7. **Unused constant.** `INITIAL_Y_SPEED` (0x4334) is defined in the asm but never read, so it is not ported.
8. **Remix state.** None. `Item.CloakingDevice.cloaked_players` is not referenced in this scope; it appears only in USP throw and air pull.
