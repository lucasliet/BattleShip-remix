# 0x38 CONKER: ConkerUSP notes

Port file: `decomp/src/ft/ftremix/conker/ftremixconkerusp.c`
Table: `dFTRemixConkerUSPProcs` / `dFTRemixConkerUSPProcsNum` (7 entries, all of the worklist's ConkerUSP routines except `descent_air_physics_`, see below).
Source: `.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm`, the `scope ConkerUSP` block that ends before `scope ConkerNSP` at line 287.

## 1. Vanilla patches

None. Nothing in this scope patches or hooks a vanilla function. The `OS.copy_segment` blocks are Fox code copied from `original.z64`, and they are fully expressed in the port, so no vanilla file needs a change.

Vanilla functions called (for the reviewer's cross-check):

| RAM | Decomp name | File |
|-----|-------------|------|
| 0x800E6F24 | ftMainSetStatus | decomp/src/ft/ftmain.c:4580 |
| 0x800E0830 | ftMainPlayAnimEventsAll | decomp/src/ft/ftmain.c:1047 |
| 0x800D8D68 | ftPhysicsApplyGravityClampTVel | decomp/src/ft/ftphysics.c:231 |
| 0x800D8E78 | ftPhysicsClampAirVelX | decomp/src/ft/ftphysics.c:270 |
| 0x800D8FC8 | ftPhysicsClampAirVelXStickRange | decomp/src/ft/ftphysics.c:315 |
| 0x800DEEC8 | mpCommonSetFighterAir | decomp/src/mp/mpcommon.c:897 |
| 0x801438F0 | ftCommonFallSpecialSetStatus | decomp/src/ft/ftcommon/ftcommonfallspecial.c:72 |

## 2. Shared Remix subsystems

None in scope. No `REMIX-TODO` markers were needed.

## 3. Copied Fox code (decoded by hand)

`OS.copy_segment` inserts ROM bytes from the vanilla Fox code. The port reproduces what those bytes do:

- `ConkerUSP_part2_` (ROM 0xD6328 len 0x14 and 0xD6340 len 0x74, asm lines 32 to 37 and 63 to 68). The Fox `li a1, 0xE7` at ROM 0xD633C is not copied. The Remix code replaces it with `addiu a1, 0xE4` (air) or `0xE3` (ground). Body: `ftMainSetStatus(gobj, action, 0.0F, 1.0F, 0)`, `ftMainPlayAnimEventsAll(gobj)`, `ftPhysicsClampAirVelX(fp, 38.0F)`, then `status var word 0 = 9`, `jumps_used = attr->jumps_max`, and the stat flag clear (see uncertainty 3).
- `main_air` (ROM 0xD7190 len 0x04 and 0xD7194 len 0x1C, asm lines 77 and 80). These are the prologue, the `anim_frame` load, the `c.le.s` against 0 and `li a3,1`. The branch at asm line 81 returns when `anim_frame > 0`.
- `descent_air_physics_` (ROM 0xD6788 len 0x60, asm line 278). Not ported, see uncertainty 1.

## 4. Uncertainties

1. **`descent_air_physics_` is not ported.** It is not in `build/remixasm/worklists/38_CONKER.md`, so no action table or special table references it. It is at asm line 277 to 285, and it is a plain copy of Fox code with `lw a1, 0x0018(sp)`. Add it if a later worklist or table references it.
2. **Status var field.** `sw t9, 0x0B18(s0)` (asm line 4 of ConkerUSP_part2_, value 9) is written to `fp->status_vars.common.dead.wait` (the first s32 of the union). The field name was chosen by offset only, because the decomp has no Conker status vars. The meaning of value 9 is unknown.
3. **Dead branch omitted.** `ConkerUSP_part2_` takes the caller's `a1` as a flag, saves it at 0x34(sp), and clears stat flag `ga` (byte 0x28E mask 0x08) only when the flag is 0. Both callers (`air_initial_`, `ground_initial_`) set `a1 = 1`, so that path is unreachable and the port omits it.
4. **Decay constant.** The Remix comment at asm line 107 says "0.94", but the instruction loads `lui t0, 0x3F70`, which is `0x3F700000 = 0.9375F`. The port uses 0.9375F and keeps the hex in a comment.
5. **Player struct source.** `main_air` reads the player struct from `a2`, which is saved to 0x28(sp) right after the first copy (asm lines 77 to 78) and reloaded later with `lw t8, 0x28(sp)`. The port uses `ftGetStruct(fighter_gobj)` instead. This assumes `a2 == gobj->user_data` for every caller. Not verified against the Remix caller code (outside this scope).
6. **Inexact float literals.** `0x3D8F5D00` (about 0.0700016) and `0x3D3751EC` (about 0.0447559) are written as their exact decimal values with the hex in a comment. The others (0.35F, 0.04F, 0.9375F, 30.0F, 38.0F, 50.0F, 80.0F, 32.0F, 0.25F) are the nearest float to the decimal value.
7. **Role names.** `ground_y_velocity_` is named `ProcInterrupt` and `main_air` and `descent_main_air` are named `ProcUpdate`, by reading their role in the action. The Remix engine call site was not checked, so this may need renaming.
8. **Dead `addiu` values.** `main_air` at asm lines 87 and 88 (`addiu a1` 0x3A, then 0xE6) loads `a1 = 0x3A` and then `a1 = 0xE6`. The first value is overwritten and is ignored in the port.

## 5. Verification

`scripts/remix/syntax-check.sh decomp/src/ft/ftremix/conker/ftremixconkerusp.c` printed `syntax-check OK` with no warnings. The build, cmake and the game were not run, as the brief requires.
