# Item.WaddleDee (id 0x46) port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/WaddleDee.asm` (1852 lines).
Port: `decomp/src/it/itremix/itremixwaddledee.c` and `itremixwaddledee.h`.
Descriptor: `dITRemixWaddleDeeItemDesc` (kind 0x46, SPAWN = `itRemixWaddleDeeStageSetting`,
SHOW_GFX FALSE, no PICKUP/DROP/THROW/COLLISION procs; the asm sets them all to 0).

## Routines

Ported (asm label -> C, all routines in the asm):

| asm | C | exported |
|---|---|---|
| stage_setting_ | itRemixWaddleDeeStageSetting, itRemixWaddleDeeSpawnMinion, itRemixWaddleDeeInitSpawn | yes (the first two) |
| minion_null_main_ | itRemixWaddleDeeMinionNullMain, itRemixWaddleDeePinToHand | yes |
| minion_throw_initial_ | itRemixWaddleDeeThrowInitial | no |
| waddle_dee_aerial_main_ | itRemixWaddleDeeWaddleDeeAerialMain | yes |
| waddle_dee_landing_main_ | itRemixWaddleDeeWaddleDeeLandingMain | yes |
| waddle_dee_collision_ | itRemixWaddleDeeWaddleDeeCollision | yes |
| waddle_dee_resting_collision_ | itRemixWaddleDeeWaddleDeeRestingCollision | yes |
| minion_detect_collision_ (OS.copy_segment 0xEE0F4 / 0xEE1CC) | itRemixWaddleDeeMinionDetectCollision | no |
| minion_hurtbox_collision_ | itRemixWaddleDeeMinionHurtboxCollision | yes |
| minion_hitbox_collision_ | itRemixWaddleDeeMinionHitboxCollision | yes |
| minion_walk_intitial | itRemixWaddleDeeMinionWalkInitial | yes (void) |
| ground_move_initial | itRemixWaddleDeeGroundMoveInitial | no |
| waddle_ground_collision | itRemixWaddleDeeWaddleGroundCollision | yes |
| waddle_ground_to_air_transition | itRemixWaddleDeeGroundToAirTransition | no |
| waddle_edge_check_ | itRemixWaddleDeeEdgeCheck | no |
| minion_walk_main | itRemixWaddleDeeMinionWalkMain (+ CheckWalkEdge, TryWalkAttack) | yes |
| check_for_targets_ | itRemixWaddleDeeCheckForTargets (+ ScanFighters, ScanItems, IsTeamProtected) | no |
| check_target_ | itRemixWaddleDeeCheckTarget (+ GetRangeX, GetRangeY) | no |
| waddle_dee_attack_initial | itRemixWaddleDeeAttackInitial | no |
| waddle_dee_jumpsquat_main | itRemixWaddleDeeJumpsquatMain (+ GroundJumpsquatStep) | no |
| jump_initial (inside jumpsquat) | itRemixWaddleDeeJumpInitial | no |
| minion_begin_main_ | itRemixWaddleDeeBeginMain | no |
| minion_landing_initial_ | itRemixWaddleDeeLandingInitial | no |
| minion_grounded_walk_initial_ | itRemixWaddleDeeGroundedWalkInitial | no |
| remove_minion_check_ | itRemixWaddleDeeRemoveMinionCheck | yes; REMIX-TODO(Dedede), body empty |
| minion_free_ | itRemixWaddleDeeMinionFree | yes; REMIX-TODO(Dedede), body empty |
| minion_blast_zone_ | itRemixWaddleDeeMinionBlastZone | yes |
| remove_bob_bomb_walk_fgm_ (vanilla hook) | not ported; see Vanilla patches | - |
| minion_grounded_walk_initial_hitboxes_ | not ported (see Uncertainties) | - |

Shared with WaddleDoo: stage setting, the state routines listed as exported, walk init, free, blast zone, remove check.
WaddleDoo's `itRemixWaddleDooAttackInitial` is called from `itRemixWaddleDeeTryWalkAttack` for non-Dee kinds.

## Item file and descriptor

- Item file: `dw Character.DEDEDE_file_6_ptr` maps to `fp->data->p_file_special1` of the owner.
  Set in `itRemixWaddleDeeSpawnMinion` before each `itManagerMakeItem`.
- Attributes: `o_attributes` = 0x40 (item footer offset).
- `item_info_array` id 0 in the asm is the Remix id 0x46 here (add_item order).
- Transform types: `{ nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 }`. The asm word 0x1B000000 is
  nGCMatrixKindTraRotRpyR (enum value 27).

## State table

`item_state_table` has 6 blocks at 0x14 + 0x20 * n, so the status ids are 0..5:
0 null (held), 1 aerial/main, 2 landing, 3 walk, 4 jump squat (attack), 5 jump.
The asm comments are off by one for these labels; the table is indexed by address.
The eight columns map to ITStatusDesc in order: update, map, hit, shield, hop (shield edge
0x801733E4 = itMainCommonProcHop), setoff (unknown, NULL), reflector (0x80173434 =
itMainCommonProcReflector), damage (hurtbox collision with hitbox).

## Struct field mapping (N64 offset -> port)

The Remix ITStruct layout is not the decomp layout, so every offset is mapped by meaning.
The N64 offsets after 0x0248 are shifted by +4 relative to the decomp struct (for example
0x02CE = is_allow_pickup/times_landed byte, 0x02C0 = lifetime, 0x02A8 = damage_gobj,
0x02D4 = attr, 0x0340 = event_id byte, 0x033E = multi halfword, 0x0354 = item_vars+4).

| N64 | port |
|---|---|
| 0x0008 owner | ip->owner_gobj |
| 0x0014 / 0x0015 team / port | ip->team / ip->player |
| 0x0024 direction | ip->lr |
| 0x002C / 0x0030 / 0x0034 velocity | ip->physics.vel_air.x/y/z |
| 0x0108 kinetic state | ip->ga |
| 0x010C hitbox enable | ip->attack_coll.attack_state |
| 0x0114 | ip->attack_coll.throw_mul |
| 0x0158 clang bit (0x8000) | ip->attack_coll.can_setoff (FALSE) |
| 0x0158 bit 0x10 | ip->attack_coll.can_rehit_shield (TRUE) |
| 0x0224 .. 0x023C hit pointers | ip->attack_coll.attack_records[0..3].victim_gobj |
| 0x0248 hurtbox enable | ip->damage_coll.hitstatus |
| 0x0298 damage | ip->damage_queue |
| 0x029C knockback angle | ip->damage_angle |
| 0x02A4 damage direction | ip->damage_lr |
| 0x02A8 attacker | ip->damage_gobj |
| 0x02C0 duration / timer | ip->lifetime |
| 0x02CE bit 0x80 pickup | ip->is_allow_pickup |
| 0x02CE bits 0x30 landed | ip->times_landed = 0 |
| 0x02CF bit 0x80 owner attack | ip->is_damage_all |
| 0x033E halfword timer / thrown flag | ip->multi |
| 0x0340 bits 0xF0 | ip->event_id = 0 |
| 0x0398 blast zone routine | ip->proc_dead |
| 0x01C4 original owner | vars.player_gobj |
| 0x01C8 max speed | vars.max_speed |
| 0x01CC rotation direction | vars.rotation_direction |
| 0x01D0 hitbox refresh timer | vars.hitbox_refresh_timer |
| 0x01D4 hitbox collision flag | vars.is_hitbox_collided |
| 0x0354 throw value (4) | vars.throw_unknown |
| FTStruct 0x0044 lr, 0x000C team, 0x000D port, 0x0012 handicap | fp->lr, fp->team, fp->player, fp->handicap |
| FTStruct 0x01C2 stick x | fp->input.pl.stick_range.x |
| FTStruct 0x0910 left hand | fp->joints[10] |
| FTStruct 0x0024 action | fp->status_id |
| DObj 0x0038 z rotation | dobj->rotate.vec.f.z |
| DObj 0x0050 anim pointer | dobj->dv |
| MObj 0x0080 image index | mobj->texture_id_curr |
| MObj 0x0090 / 0x0094 anim pointers | mobj->aobj / mobj->matanim_joint.event16 |

`ITRemixWaddleDeeVars` is a file-local overlay on `ip->item_vars` with a `_Static_assert` that
it fits. Offset 0 of `item_vars` is `bombhei.smoke_delay`, which `itBombHeiWalkInitVars` writes,
so the overlay starts with a reserved word. The N64 kept these fields in the Remix
attack_pos/attack_records area, which overlaps the decomp layout; the port stores them in
`item_vars`. The Shuriken port uses the same approach.

## REMIX-TODO

1. REMIX-TODO(Dedede): `minion_free_` and `remove_minion_check_` manage the owner's minion slots
   (N64 player 0x0ADC and 0x0AE0, held minion 0x0B20). Both bodies are empty in the port. The
   Dedede DSP port keeps one held minion in `sFTRemixDededeDSPMinion`, and it does not model the
   two-slot cap. A shared accessor from `ftremixdedededsp.c` is needed to port these routines.
2. REMIX-TODO(vanilla patch): the bob-omb walk FGM hook (see Vanilla patches). Until it is applied,
   the walk init plays the bob-omb fuse sfx for minions too.
3. REMIX-TODO(WaddleDoo, descriptor): Waddle Doo's `dITRemixWaddleDooItemDesc.proc_make` is
   `itRemixWaddleDeeStageSetting`, which spawns kind 0x46. It must spawn Doo's own ITDesc with
   `itRemixWaddleDeeSpawnMinion`.

## Vanilla patches

1. `remove_bob_bomb_walk_fgm_`: `OS.patch_start(0xF23CC, 0x8017798C)`, inside
   `itBombHeiWalkInitVars` (0x80177848, decomp `decomp/src/it/itcommon/itbombhei.c`, line 559).
   Apply under `#ifdef REGION_REMIX` in that file:

   ```c
   #ifdef REGION_REMIX
       if (ip->kind == nITKindBombHei)
       {
           func_800269C0_275C0(nSYAudioFGMBombHeiFuse);
       }
   #else
       func_800269C0_275C0(nSYAudioFGMBombHeiFuse);
   #endif
   ```
   The Remix condition is `item kind == Hazards.standard.BOBOMB`, which is `nITKindBombHei`.
   I did not apply this patch.
2. `waddle_ground_collision` jumps into the middle of `itBombHeiWalkProcMap` (0x801777D8) at
   0x801777EC, with the wall-transition argument replaced. The C port reproduces the same
   body, so no vanilla change is needed.
3. Vanilla calls used with their decomp names: itManagerMakeItem, itMainSetStatus,
   itMapSetAir, itMapSetGround, itMapCheckLRWallProcNoFloor, itMapCheckCollideAllRebound,
   itBombHeiWalkInitVars, itBombHeiCommonSetHitStatusNormal (0x80177208),
   itLinkBombCommonSetHitStatusNormal (0x80185CD4), itMainApplyGravityClampTVel,
   mpProcessUpdateMain, mpCommonRunItemCollisionDefault, mpCollisionGetFloorEdgeL/R,
   mpCollisionCheckExistLineID, ftCommonDamageGetKnockbackAngle, gmCollisionGetFighterPartsWorldPosition,
   gcAddXObjForDObjFixed (0x80008CC0), __cosf (0x80035CD0), __sinf (0x800303F0),
   func_800269C0_275C0 (FGM play), gGCCommonLinks[nGCCommonLinkIDFighter/Item], gSCManagerBattleState.

## Uncertainties (asm line numbers)

1. stage_setting_ l.163 and l.172: the asm passes the caller's a3 as the vel argument and a
   literal 0 as flags. The port passes FALSE for flags and ignores the proc_make flags.
2. stage_setting_ l.201-204: the fighter byte 0x12 (handicap) is copied to item byte 0x12. The
   Remix comment calls it an attack hitbox type. The port maps it to `ip->handicap`.
3. minion_null_main_ l.304-305 (pin): uses `s0` for the item position struct, but `s0` is never
   set in that routine. The port uses the item's own DObj.
4. minion_null_main_ l.270 (throw): passes `s0` to remove_minion_check_. The port passes the item.
5. minion_hurtbox_collision_ l.917: `sw r0, 0x010C(v1)` uses `v1` after a call, which is not
   defined. The port disables the hitbox (`attack_state = FALSE`), which is the evident intent.
6. minion_hitbox_collision_ l.1013: `ftCommonDamageGetKnockbackAngle` gets an unset `a1`. The port
   passes `ip->ga`, which is TRUE after `minion_begin_main_`.
7. minion_detect_collision_ l.893-903 (the copied fragments): the floor-miss path leaves `t5`
   stale. The port uses `mask_curr` in both paths, which is the intended value.
8. minion_detect_collision_: the stack spills of `t6`/`t7` at sp+0x24 and sp+0x20 are never
   read, so they are dropped.
9. 0x0340 "unknown bitflags" (l.1090-1091): the port treats them as `event_id` (4 bits), which the
   mask 0xFF0F clears. The field is 4 bits in the decomp struct.
10. 0x0158 bit 0x10 (l.357): the Remix comment says "unknown". The port maps it to
    `can_rehit_shield`, which is the bit's position in the decomp bitfield.
11. The `_speed_refresh` branches of the aerial, landing and jump squat routines are commented
    out in the asm (l.574-586, l.686-700, l.736-748). The port omits them, as the asm does.
12. `minion_grounded_walk_initial_hitboxes_` (l.784-790): not referenced by any Remix source
    (grep of `src/`). It jumps to 0x801863B8 with a hard-coded item info reference. Not ported.
13. Slide factor: the asm comment says 0.875 but `lui 0x3F70` is 0.9375. The port uses 0.9375.
14. waddle_dee_aerial_main_ and ground motion: `ground_move_initial` writes the item file
    pointers at `attr->data - 0x33F8 + 0x3310/0x34C0`. The port uses `itGetPData` with the same
    offsets.
15. The blast zone proc returns TRUE. `minion_free_` leaves v0 as the non-zero item struct
    pointer, so this matches the asm.
16. The asm's `item_info_array` id 0 is the Remix id 0x46 in the port (add_item order).
17. The owner-slot effects (the two-minion cap, held-minion pointer) are not modelled. See REMIX-TODO 1.

## Verification

`scripts/remix/syntax-check.sh decomp/src/it/itremix/itremixwaddledee.c` prints `syntax-check OK`
with no warnings from the file. Build and game runs were not run, as the brief requires.
