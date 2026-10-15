# 0x22 DSAMUS: DSamusDSP (Dark Samus down special)

Port: `decomp/src/ft/ftremix/dsamus/ftremixdsamusdsp.c`, table `dFTRemixDSamusDSPProcs` / `dFTRemixDSamusDSPProcsNum`.
Source: `.toolchain/pkgs/smashremix-src/src/DSamus/DsamusSpecial.asm`, lines 635-958 (scope DSamusDSP).

Routines in the worklist: 3 of 3 ported (`air_physics_`, `air_collision_`, `ground_collision_`).

## Action ids (verified in remix.z64)

- `DSPAir` = 0xE8 (ROM array index 12 of `Character.DSAMUS_action_array`, RAM 0x804AA460, entry asm2 `air_physics_`, asm4 `air_collision_`).
- `DSPLanding` = 0xE9 (index 13, main `0x800D94C4`, asm3 `0x800D8BB4`, asm4 `0x800DDEE8`).
- Derived from Character.asm `add_new_action` (`Action.x = action_array_size.SAMUS/0x14 + n + 0xDC`, n = NSPChargeAir 0, DSPAir 1, DSPLanding 2).
- Uncertainty: `DsamusSpecial`/`DSamus.asm` `action_string_table` labels 0xE8 "BoostBallAir" and 0xE9 "BoostBallLanding". Those are debug names only. The ids above match the ROM entries, so they are kept.

## Routine mapping

| Remix label | Port | Role | Notes |
|---|---|---|---|
| `DSamusDSP.air_physics_` (asm 646) | `ftRemixDSamusDSPAirProcPhysics` (table) | ProcPhysics | Original air physics is `0x8015E050` = `ftSamusSpecialAirLwProcPhysics`. |
| `DSamusDSP.air_collision_` (asm 689) | `ftRemixDSamusDSPAirProcMap` (table) | ProcMap | Non-landing path is `0x800DE99C` = `mpCommonProcFighterCliffFloorCeil`. Landing path is `0x800DE6E4` = `mpCommonProcFighterLanding`. |
| `DSamusDSP.ground_collision_` (asm 740) | `ftRemixDSamusDSPGroundProcMap` (table) | ProcMap | Jump check is `0x8013F474` = `ftCommonKneeBendGetInputTypeCommon`. Floor check is `0x800DDDDC` = `mpCommonProcFighterOnFloor`. |
| `DSamusDSP.landing_initial_` (asm 722) | `ftRemixDSamusDSPLandingInitial` (static) | status entry | Not in the worklist. Used as the landing transition target. |
| `DSamusDSP.ground_to_air_` (asm 774) | `ftRemixDSamusDSPGroundToAir` (static) | status entry | Not in the worklist. Used as the ground-floor transition target. |
| `DSamusDSP.ground_to_air_jump_` (asm 793) | `ftRemixDSamusDSPGroundToAirJump` (static) | status entry | Not in the worklist. Called from `ground_collision_`. |

Shared static helper: `ftRemixDSamusDSPSwitchToAir` (the common prefix of ground_to_air_ and ground_to_air_jump_: `0x8015E1DC` = `ftSamusSpecialLwSwitchStatusAir`, reset temp variable 2, scale vel_air.x by 0.625).

Remix temp variables (from struct_offsets.txt): temp variable 2 = `motion_vars.flags.flag1` (N64 0x0180), temp variable 3 = `motion_vars.flags.flag2` (N64 0x0184). The port uses those C fields directly. No raw offsets.

No `REMIX-TODO` markers: the DSP spawns no Remix custom item or GFX (`0x800FF3F4` is the vanilla `efManagerDustHeavyDoubleMakeEffect`, ported as a direct call).

## Vanilla patches (for the vanilla decomp; not edited by this port)

All patches are `OS.patch_start` blocks in `DsamusSpecial.asm`. Each needs the C below under `#ifdef REGION_REMIX` in the named vanilla function. The C only calls vanilla functions, so no prototype needs to be made non-static. The `DSAMUS_*` names below stand for literal values; add them to the vanilla file as local `#define`s or inline the numbers. `Character.id.DSAMUS` is `REMIX_ID_DSAMUS` (0x22, `port/remix/remix_roster_data.h`).

1. **0x8015E020**, `ftSamusSpecialLwProcPhysics` (`decomp/src/ft/ftchar/ftsamus/ftsamusspeciallw.c:53`). Dark Samus gets ground x vel `0.625F` (0x3F20) and ground accel `10.0F` (0x4120) in the `ftPhysicsSetGroundVelStickRange` call of the `flag3` branch. Patch `ground_x_physics_patch_`.
   ```c
   #ifdef REGION_REMIX
           if (fp->fkind == REMIX_ID_DSAMUS) ftPhysicsSetGroundVelStickRange(fp, 0.625F, 10.0F);
           else
   #endif
           ftPhysicsSetGroundVelStickRange(fp, attr->walk_speed_mul * FTSAMUS_BOMB_DRIFT, attr->traction);
   ```
2. **0x8015E0C0**, `ftSamusSpecialAirLwProcPhysics` (`ftsamusspeciallw.c:67`). Dark Samus gets air accel `0.1875F` (0x3E40) in the `ftPhysicsClampAirVelXStickRange` call. Patch `air_x_physics_patch_`.
   ```c
   #ifdef REGION_REMIX
           fp->fkind == REMIX_ID_DSAMUS ? 0.1875F : attr->air_accel * FTSAMUS_BOMB_DRIFT,
   #else
           attr->air_accel * FTSAMUS_BOMB_DRIFT,
   #endif
   ```
   Put that expression in place of the accel argument. Only the accel argument changes.
3. **0x8015E1AC**, `ftSamusSpecialLwTransferStatusAir` (`ftsamusspeciallw.c`, function spanning 0x8015E170-0x8015E1DC). Dark Samus gets initial y vel `8.0F` (0x4100) instead of `FTSAMUS_BOMB_VEL_Y_BASE` (40.0F). Patch `ground_y_velocity_patch_`.
   ```c
   fp->physics.vel_air.y = (fp->fkind == REMIX_ID_DSAMUS) ? 8.0F : FTSAMUS_BOMB_VEL_Y_BASE;
   ```
   Under `#ifdef REGION_REMIX`, use this form. The vanilla form is kept in `#else`.
4. **0x8015E158**, `ftSamusSpecialAirLwSwitchStatusGround` (`ftsamusspeciallw.c:94`). For all fighters, the `ftMainSetStatus` flags argument becomes `0x2003` (was `FTSTATUS_PRESERVE_NONE`). Patch `air_to_ground_patch_`. The vanilla `sw zero, 0x10(sp)` after the call is harmless.
   ```c
   #ifdef REGION_REMIX
       ftMainSetStatus(fighter_gobj, nFTSamusStatusSpecialLw, fighter_gobj->anim_frame, 1.0F, 0x2003);
   #else
       ftMainSetStatus(fighter_gobj, nFTSamusStatusSpecialLw, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
   #endif
   ```
   `0x2003` is bits 0x2000, 0x2 (COLANIM) and 0x1 (HIT). The 0x2000 bit name is not checked in `ftdef.h`.
5. **0x8015E200**, `ftSamusSpecialLwSwitchStatusAir` (`ftsamusspeciallw.c:118`). Same change as patch 4 (`ftMainSetStatus` flags to `0x2003`). Patch `ground_to_air_patch_`.
   ```c
   #ifdef REGION_REMIX
       ftMainSetStatus(fighter_gobj, nFTSamusStatusSpecialAirLw, fighter_gobj->anim_frame, 1.0F, 0x2003);
   #else
       ftMainSetStatus(fighter_gobj, nFTSamusStatusSpecialAirLw, fighter_gobj->anim_frame, 1.0F, FTSTATUS_PRESERVE_NONE);
   #endif
   ```
6. **0x8015E218**, `ftSamusSpecialLwInitStatusVars` (`ftsamusspeciallw.c:125`). The `j` replaces the `jr ra`. The original `sw r0, 0x17C(a0)` (flag0 reset) stays in the delay slot. Patch `init_variables_patch_` also resets temp variables 2 and 3.
   ```c
   void ftSamusSpecialLwInitStatusVars(FTStruct *fp)
   {
       fp->motion_vars.flags.flag0 = FALSE;
   #ifdef REGION_REMIX
       fp->motion_vars.flags.flag1 = FALSE;
       fp->motion_vars.flags.flag2 = FALSE;
   #endif
   }
   ```
7. **0x8015E284**, `ftSamusSpecialAirLwSetStatus` (`ftsamusspeciallw.c:145`). The status id passed to `ftMainSetStatus` is `DSPAir` (0xE8) for Dark Samus, else 0xE6 (`Action.SAMUS.BombAir` = `nFTSamusStatusSpecialAirLw`). Patch `air_action_patch_`. The disassembly at 0xD8CC4 confirmed `addiu a1, zero, 0xE6`, so the patch swaps that one line.
   ```c
   #ifdef REGION_REMIX
       ftMainSetStatus(fighter_gobj, (fp->fkind == REMIX_ID_DSAMUS) ? 0xE8 : nFTSamusStatusSpecialAirLw, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
   #else
       ftMainSetStatus(fighter_gobj, nFTSamusStatusSpecialAirLw, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE);
   #endif
   ```
   The patch reads `fkind` from `s0 + 0x8`, and `s0` is assumed to be `fp` (not confirmed in this pass).

The Remix `DSamusDSP` patch block also includes `ground_y_velocity_patch_` (item 3) and `init_variables_patch_` (item 6), both covered above.

## Uncertainties

- Ground-to-air calls `ftSamusSpecialLwSwitchStatusAir` (0x8015E1DC) while the asm comment says "transition to DSPA". Ported as the call. Asm lines 776 and 795.
- `air_physics_`: the branch-likely at asm line 655 (`bnel`) skips the `sw r0` delay slot when temp variable 2 == 1. So temp variable 2 stays 1 in that branch. Ported faithfully (no reset).
- `ground_collision_`: the jump check (`0x8013F474`) runs only when temp variable 3 is set. Ported as `flag2 != 0 && kneebend != 0`. Asm lines 745-753.
- `landing_initial_` copies Falcon Kick's landing start (0xDAB2C 0x18 and 0xDAB48 0x20). Disassembly confirmed: `mpCommonSetFighterGround`, then `ftMainSetStatus(gobj, status, 0.0F, 1.0F, 0)`. The 0.625F scale is on `vel_ground.x` (N64 0x60). Ported with `DSPLanding` = 0xE9.
- The Remix ROM was not rebuilt and the game was not run. Only the syntax check and ROM-array lookups were used.
