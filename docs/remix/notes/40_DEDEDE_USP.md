# 0x40 DEDEDE: DededeUSP notes (Super Dedede Jump)

Port file: `decomp/src/ft/ftremix/dedede/ftremixdededeusp.c`
Table: `dFTRemixDededeUSPProcs` / `dFTRemixDededeUSPProcsNum` (12 entries, every routine of `build/remixasm/worklists/40_DEDEDE.md` whose label starts with `DededeUSP.`).
Source: `.toolchain/pkgs/smashremix-src/src/Dedede/DededeSpecial.asm`, `scope DededeUSP` (lines 7 to 580).

## 1. Vanilla patches

No `OS.copy_segment` or `OS.patch_start` in the USP scope. Nothing patches vanilla code, so no vanilla file needs a change.

The scope depends on two table patches in `Dedede.asm`, which the Remix bind applies, not C code:

- `Dedede.asm` lines 589 to 594: `air_usp` and `ground_usp` special entries are `dw DededeUSP.begin_initial_`. Served by `dFTRemixDededeUSPProcs` (`begin_initial_`).
- `Dedede.asm` lines 507 to 510: `Character.edit_action` for `USP_BEGIN`, `USP_MOVE`, `USP_LAND` and `USP_CANCEL`, and lines 555 and 579 for `USP_CEILING_BONK`. These name `begin_main_`, `begin_physics_`, `begin_collision_`, `move_cancel_`, `move_physics_`, `move_collision_`, `landing_main_`, `cancel_main_`, `cancel_collision_`, `ceiling_bonk_main_` and `ceiling_bonk_collision_`, all in the table.

Vanilla functions called (for cross-check):

| RAM | Decomp name | File |
|-----|-------------|------|
| 0x800E6F24 | ftMainSetStatus | decomp/src/ft/ftmain.c:4580 |
| 0x800E0830 | ftMainPlayAnimEventsAll | decomp/src/ft/ftmain.c:1047 |
| 0x800D9480 | ftAnimEndCheckSetStatus | decomp/src/ft/ftanimend.c:4 |
| 0x800D8D68 | ftPhysicsApplyGravityClampTVel | decomp/src/ft/ftphysics.c:231 |
| 0x800D8FA8 | ftPhysicsCheckClampAirVelXDecMax | decomp/src/ft/ftphysics.c:309 |
| 0x800DDDDC | mpCommonProcFighterOnFloor | decomp/src/mp/mpcommon.c:233 |
| 0x800DE87C | mpCommonCheckFighterCeilHeavyCliff | decomp/src/mp/mpcommon.c:660 |
| 0x800DE8E4 | mpCommonSetFighterWaitOrLanding | decomp/src/mp/mpcommon.c:676 |
| 0x800DEE98 | mpCommonSetFighterGround | decomp/src/mp/mpcommon.c:885 |
| 0x80144C24 | ftCommonCliffCatchSetStatus | decomp/src/ft/ftcommon/ftcommoncliffcatchwait.c:39 |
| 0x801438F0 | ftCommonFallSpecialSetStatus | decomp/src/ft/ftcommon/ftcommonfallspecial.c:72 |
| 0x800EDF24 | gmCollisionGetFighterPartsWorldPosition | decomp/src/gm/gmcollision.c:515 |
| 0x8016C954 | wpYoshiStarMakeStars | decomp/src/wp/wpyoshi/wpyoshistar.c:202 |
| 0x800269C0 | func_800269C0_275C0 (FGM play) | decomp/src/sc/sc1pmode/sc1pintro.c (extern use) |
| 0x8013E1C8 | ftCommonWaitSetStatus | decomp/src/ft/ftcommon/ftcommonwait.c:16 |

## 2. Shared Remix subsystems

- **Star projectile, no WPDesc.** `landing_main_` spawns the stars by calling the vanilla Yoshi star routine, `wpYoshiStarMakeStars(gobj, &pos)` at 0x8016C954, not a Remix weapon. The port calls it directly, so no `WPDesc` or `wpManagerMakeWeapon` is needed.
- **No custom Remix items, GFX or announcer calls** in this scope. No `REMIX-TODO` markers were needed.

## 3. Helpers and internal transitions

These are not in the worklist. They are `static` in the port and are not in the table:

- `begin_set_aerial` is passed as a function pointer to `mpCommonProcFighterOnFloor`. Port: `ftRemixDededeUSPBeginSetAerial`.
- `move_initial_` is passed as the anim-end transition in `begin_main_`. Port: `ftRemixDededeUSPMoveInitial`.
- `cancel_initial_` is passed as the anim-end transition in `ceiling_bonk_main_`. Port: `ftRemixDededeUSPCancelInitial`.
- `ceiling_subroutine_` is called from `ceiling_bonk_collision_`. Port: `ftRemixDededeUSPCeilingSubroutine`.
- `apply_lateral_movement_` and `apply_vertical_movement_` are split into `ftRemixDededeUSPApplyLateralMovement` (takes FTStruct) and `ftRemixDededeUSPApplyVerticalMovement` (takes GObj, as the asm does).

These internal routines are passed as C function pointers to vanilla code, so the port uses the C names directly. The Remix addresses are not needed in the table.

## 4. Uncertainties and deviations

1. **Dead instructions dropped.** `begin_main_` loads `kinetic state` into t8 and never uses it (asm line 60). `begin_initial_` has `lui t0, 0x3F00` with no use (line 47). `ceiling_bonk_collision_` has `lhu t8` after an unconditional branch (line 540, unreachable). `apply_lateral_movement_` has `lw at, 0x0020(sp)` (line 136) with no use. `move_initial_` loads `at = 1` only to store it as the flags argument (line 220), which the port writes as `FTSTATUS_PRESERVE_HIT` (= 1).
2. **Gravity phase constant.** `GRAVITY_PEAK` is `0x3E4C0000`, which is exactly 0.19921875. The Remix comment says 0.2. The port uses the exact value, with the hex in a comment.
3. **Stick x slot 0x0B20.** `begin_physics_`, `apply_lateral_movement_` and the launch step store the stick x as a float at 0x0B20. The port writes it to `status_vars.common.dead.pos.y`, which is the same offset. No routine in this scope reads it back. Another Remix routine may read it, so the write is kept.
4. **`cancel_main_` reads `anim_frame`.** The asm reads `0x0078(a0)` where `a0` is the GObj, not the FTStruct. The port uses `fighter_gobj->anim_frame`, which is the GObj field at 0x78 (`decomp/src/sys/objtypes.h`). If the Remix proc convention ever gives a different a0, this needs checking.
5. **Idle address.** `landing_main_` loads the return-to-idle routine with `lui a1, 0x8014` and `addiu a1, a1, 0xE1C8`. The `addiu` immediate is negative, so the address is 0x8013E1C8 (`ftCommonWaitSetStatus`). `DededeSpecial.asm` line 2925 uses `jal 0x8013E1C8` (transition to idle), which confirms it.
6. **`ftCommonFallSpecialSetStatus` mapping.** `cancel_main_` passes `(gobj, 1.0F, FALSE, TRUE, FALSE, 1.0F, FALSE)`. The mapping comes from the stack slots: a1 = drift 1.0, a2 = unknown 0, a3 = is_fall_accelerate 1, stack 0x10 = is_goto_landing 0, stack 0x14 = landing lag 1.0 (the landing fsm), stack 0x18 = is_allow_interrupt 0. This matches the Conker port's call.
7. **Clipping masks.** `0x3000` (cliff bits), `0x0800` (Surface.GROUND), `0x0400` (Surface.CEILING) and `0x4000` on `mask_curr` (ceiling flag in `ceiling_bonk_collision_`) are taken from the asm and written as named constants. The meaning of `0x4000` on `mask_curr` is not confirmed. It is the only bit the asm checks there.
8. **Ground and cliff helper.** `begin_collision_`, `cancel_collision_` and `move_collision_` share `ftRemixDededeUSPSetGroundOrCliff`. The asm does the same ground then cliff steps in each. `move_collision_` checks the ceiling only when there is no cliff bit, as the asm does.
9. **`button_press_buffer`** (`DededeUSP`, 0x8056C5BC, 4 bytes) is data that no routine in this scope reads. It is not ported.
10. **B button read.** The asm reads the high byte at 0x1BE with `lbu` and masks 0x40. That is `button_tap & 0x4000`, which is `B_BUTTON` (`CONT_B`). The port tests `button_tap & B_BUTTON`.
11. **Extern FGM declaration.** `func_800269C0_275C0` is declared locally as `extern alSoundEffect *func_800269C0_275C0(u16 sfx_id);`, copied from the Conker port, because the decomp has no header for it in this file's include set.
12. **Role names.** The functions are named by routine label (`begin`, `move`, `cancel`, `landing`, `ceiling_bonk`) rather than by ProcUpdate/Interrupt roles. The table does not depend on those names.

## 5. Not in this file (for the integrator)

- `ftremixported.c` must include `dFTRemixDededeUSPProcs`. This file does not touch it.
- The file is picked up by the CMake `GLOB_RECURSE` over `decomp/src/ft/*.c`, so no build edit is needed. Re-run cmake after adding it.

## 6. Verification

`scripts/remix/syntax-check.sh decomp/src/ft/ftremix/dedede/ftremixdededeusp.c` printed `syntax-check OK` with no warnings. The build, cmake and the game were not run, as the brief requires.
