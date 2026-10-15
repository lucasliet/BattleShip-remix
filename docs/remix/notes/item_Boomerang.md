# Item Boomerang (id 0x44), port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Boomerang.asm`
Port: `decomp/src/it/itremix/itremixboomerang.c`, `decomp/src/it/itremix/itremixboomerang.h`
Descriptor: `const ITRemixItemDesc dITRemixBoomerangItemDesc` (kind 0x44)
Spawn: `GObj* itRemixBoomerangMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`
Owner file: `fp->data->p_file_special3` (ASM `Character.MARINA_file_8_ptr`, file N = 8)
Item footer offset: `0xE0` (ASM `FILE_OFFSET`), stored in `ITDesc.o_attributes`

## Routines

All 25 routines in the asm are ported:
stage_setting_, throw_initial_, turn_initial_, return_initial_, falling_initial_,
thrown_main_, turn_main_, return_main_, falling_main_, throw_collision_,
turn_collision_, return_collision_, detect_collision_, throw_collide_,
turn_collide_, return_collide_, return_collide_clipping_, hurtbox_collision_,
shield_collision_, reflect_, apply_multiplier_, apply_rotation_,
handle_item_catch_, prepickup_, pickup_boomerang.

The item_state_table is ported as `sITRemixBoomerangStatusDescs[7]`. The asm
entries 0x801744FC, 0x80174524, 0x801744C0, 0x801745CC, 0x801745FC and 0x801733E4
are the decomp tomato procs (`itTomatoWaitProcMap`, `itTomatoFallProcMap`,
`itTomatoFallProcUpdate`, `itTomatoDroppedProcMap`, `itTomatoDroppedSetStatus`)
and `itMainCommonProcHop`. The asm entries in the state table are dw-ordered by
status id 0..6. Their comments (`0x20 - main` for status 6) are wrong, but the
ordering is clear.

Helpers that the asm inlines are kept `static`: ZeroMinorAxis (first frame of
thrown_main_), RunWallsAndCeil (part of detect_collision_), IsInCatchRange,
IsCatchableAction, TryCatchOwner (the `_check_catch` blocks), ConsumeTomatoTail
(0x80145C4C), AxisVel, GetVars, SetBaseAttack.

## Item state

`ip->item_vars` holds `ITRemixBoomerangVars { timer, target_axis, target_vel, accel }`.
The asm keeps these at ITStruct N64 offsets 0x1CC (timer), 0x1D0 (target axis),
0x1D4 (target velocity) and 0x1D8 (acceleration). No named ITStruct field exists for
them, so they live in `item_vars`, with a `_Static_assert` that they fit.

## Vanilla dependencies

1. **Pickup hook, Item.asm `item_pickup_check_`** (patch at ROM 0xC0654, RAM 0x80145C14,
   inside `ftCommonLightGetProcDamage`, `decomp/src/ft/ftcommon/ftcommonget.c`).
   The Remix item-id check jumps to `pickup_boomerang` with a0 = fighter struct, a2 = item
   gobj and a3 = fighter struct. The port handles this in the `REGION_REMIX` default case
   of `ftCommonLightGetProcDamage`, which calls `desc->proc_consume(fp, item_gobj)`. No
   vanilla edit is needed.

2. **Tail of the tomato case, RAM 0x80145C4C** (disassembled from the base ROM at
   ROM 0xC068C with `--vram 0x80145C14`). After the heal, the vanilla code runs
   `itMainDestroyItem(item)` and then the 1P tomato bonus block:
   ```c
   if ((gSCManagerBattleState->game_type == nSCBattleGameType1PGame) && (fp->player == gSCManagerSceneData.player) && (gSC1PGameBonusTomatoCount < U8_MAX))
   {
       gSC1PGameBonusTomatoCount++;
   }
   ```
   `pickup_boomerang` skips the heal and jumps here. The port reproduces the tail in
   `itRemixBoomerangConsumeTomatoTail`. This is a faithful reproduction, but it means a
   Boomerang pickup also increments the tomato bonus counter. That may be a Remix bug.

3. **Collision callback, `OS.copy_segment(0xEE0F4, 0x88)` and `(0xEE1CC, 0x2C)`**. These copy
   the body of vanilla `itMapProcAllCheckCollisionFlag` (decomp `itmap.c:108`, RAM
   0x801736B4). The copied bytes are visible at RAM 0x801737B8. The decomp label for that
   address is `itMapTestAllCollisionFlag`, which only wraps `mpProcessUpdateMain`. The label
   and the bytes disagree. The asm omits the ground block in the middle, at ROM
   0xEE190..0xEE1C8: `mpProcessRunFloorEdgeAdjust` when `mask_stat & 0x800`, the landing
   FGM 0x2E, the spin and rotation reset, and `is_coll_end = 1`. The port implements the
   callback without that block (`itRemixBoomerangProcCollision`). No vanilla edit is needed.

4. **Vanilla calls used as-is** (decomp name in brackets):
   - 0x8016E174 `itManagerMakeItem`
   - 0x8017279C `itMainClearOwnerStats`
   - 0x80008CC0 `gcAddXObjForDObjFixed(joint, 0x2E, 0)`
   - 0x80111EC0 `ifCommonItemArrowMakeInterface`
   - 0x80177208 `itBombHeiCommonSetHitStatusNormal`
   - 0x80177218 `itBombHeiCommonSetHitStatusNone`
   - 0x80172EC8 `itMainSetStatus`
   - 0x80172CA4 `itMainSetFighterHold`
   - 0x801728D4 `itMainDestroyItem`
   - 0x80173434 `itMainCommonProcReflector`
   - 0x801733E4 `itMainCommonProcHop`
   - 0x800DA034 `mpProcessUpdateMain`
   - 0x800DD59C `mpProcessSetLandingFloor`
   - 0x8013FB2C `ftCommonJumpAerialProcInterrupt`

5. **ITStruct fields mapped by meaning** (asm N64 offset to field):
   - 0x0008 `owner_gobj`, 0x0084 `user_data`, 0x0074 DObj
   - 0x0110 `attack_coll.damage`, 0x013C `angle`, 0x0140 `knockback_scale`, 0x0148 `knockback_base`
   - 0x0156 `attack_coll.fgm_id`, 0x010C `attack_coll.attack_state`, 0x0248 `damage_coll.hitstatus`
   - 0x033E (set to 0, asm line 144) → `ip->multi = 0`, matching the decomp Bob-omb spawn (`itBombHeiMakeItem`)
   - 0x0344 `spin_step` (asm rotation), 0x0348 `arrow_gobj`
   - 0x02CF bit 0x20 → `is_thrown` (asm lines 247-248, 269-270); see uncertainty 2
   - 0x02D3 bit 0x04 → `is_unused_item_bool` (asm line 180-182); see uncertainty 2
   - 0x0038 `coll_data`, 0x002C/0x0030 `physics.vel_air.x/.y`, 0x0038 DObj `rotate.vec.f.z`
   - Fighter: 0x0024 `status_id`, 0x0840 `catch_gobj`, 0x084C `item_gobj`, 0x09DC `proc_interrupt`, 0x000D `player`

## REMIX-TODO

- `REMIX-TODO(Size)`: `handle_item_catch_` (asm 759-767) multiplies the catch range by
  `Size.multiplier_table[fp->player]`. Size is not ported. The port uses `1.0F`, the
  unscaled value, as a local in `itRemixBoomerangHandleCatch`. This is a default, not a
  substitute call.

## Uncertainties

1. **s0 in the collision procs** (asm 468-471, 496-499, 524-527). `throw_collision_`,
   `turn_collision_` and `return_collision_` pass `s0` as the item object without setting
   it. The port passes `item_gobj`, which assumes the caller keeps the item in s0.
2. **Bit mapping.** The ITStruct bitfield offsets in the asm are 4 bytes higher than the
   decomp table in this region (asm 0x02CF ↔ table 0x02CB, asm 0x02D3 ↔ table 0x02CF).
   The bits were mapped by meaning. `is_thrown` = "apply magnitude and throw multiplier to
   damage output" (asm comment "enable bitflag for bonus damage"). `is_unused_item_bool`
   is what the Bob-omb and tomato spawns set.
3. **Zero tests.** `thrown_main_` (asm 299-310) and the first-frame check compare raw
   words against zero. The port compares floats with `== 0.0F`, which differs only for -0.0.
4. **Initial proc.** The Remix `item_info_array` has five words, with no proc fields
   (asm 29-35). The port uses the air-wait procs (`itTomatoFallProcUpdate` and
   `itTomatoFallProcMap`) for the descriptor, the same as the Bob-omb spawn. Check against
   the real Remix ITDesc layout if the Boomerang must run before it is thrown.
5. **Stack adjust** (asm 848-849). The `addiu sp, -0x30` and `addiu sp, 0x30` around the
   `0x80172CA4` call (marked "unsafe") has no C equivalent and is not reproduced.
6. **Owner check.** The asm compares the owner's first word (`0x0000`) with 0x3E8,
   `nGCCommonKindFighter`. The port uses `owner_gobj->id`, which is the same word.
