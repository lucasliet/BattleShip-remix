# Item Pitfall (0x34): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Pitfall.asm`.
Port: `decomp/src/it/itremix/itremixpitfall.c`, `decomp/src/it/itremix/itremixpitfall.h`.
Descriptor: `const ITRemixItemDesc dITRemixPitfallItemDesc`, kind 0x34.
Spawn function: `GObj* itRemixPitfallMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.

## Item file and attributes

- `dw 0x8018D040` in `item_info_array` is the common item file. The port sets `p_file = &gITManagerCommonData`, with no owner needed.
- `o_attributes = 0xF70`, which is `FILE_OFFSET`.

## Routine map

| Remix label | C | Role |
|---|---|---|
| stage_setting_ | itRemixPitfallMakeItem | SPAWN_ITEM |
| (inline in stage_setting_) | itRemixPitfallApplySpawnHitbox | bury damage, angle, knockback, damage, hurtbox FGM |
| prepickup_ | itRemixPitfallPrepickup | PICKUP_ITEM_INIT |
| throw_initial_ | itRemixPitfallThrowInitial | THROW_ITEM and DROP_ITEM |
| pickup_item | itRemixConsumeTomatoTail | PICKUP_ITEM_MAIN, jumps to 0x80145C4C |
| collision_ | itRemixPitfallThrownProcMap | thrown proc_map |
| begin_buried_ | itRemixPitfallBeginBuried | helper of collision_ |
| stick_to_surface | itRemixPitfallStickToSurface | helper of begin_buried_ |
| buried_main_ | itRemixPitfallBuriedMain | buried proc_update |
| buried_collision_ | itRemixPitfallBuriedCollision | buried proc_map |
| destroy_ | itRemixPitfallDestroy | hit, shield, hop, set-off and damage procs |

Status table: `sITRemixPitfallStatusDescs[5]`, states 0 to 4 (wait, fall, pickup, thrown, buried).
State 0 map is `itTomatoWaitProcMap` (0x801744FC). The asm comment calls it "buried", but the vanilla name is the tomato wait map.

## Not ported (unreferenced in the asm)

- `hurtbox_collision_`, `apply_multiplier_` and `falling_initial_`: not in any state table and no `jal` to them. `drop_item_` is commented out.
- `detect_collision_`: see Vanilla patches below.

## Vanilla functions used

| RAM | Decomp name |
|---|---|
| 0x8016E174 | itManagerMakeItem |
| 0x801744C0 | itTomatoFallProcUpdate |
| 0x80174524 | itTomatoFallProcMap |
| 0x801744FC | itTomatoWaitProcMap |
| 0x80177218 | itBombHeiCommonSetHitStatusNone |
| 0x80177208 | itBombHeiCommonSetHitStatusNormal |
| 0x80172EC8 | itMainSetStatus |
| 0x80177530 | itBombHeiThrownProcUpdate |
| 0x80173434 | itMainCommonProcReflector |
| 0x801737B8 | itMapTestAllCollisionFlag |
| 0x801737EC | itMapCheckCollideAllRebound |
| 0x80173F54 | itMapSetGround |
| 0x80176708 | itMSBombAttachedUpdateSurface |
| 0x80185CD4 | itLinkBombCommonSetHitStatusNormal |
| 0x80008CC0 | gcAddXObjForDObjFixed |
| 0x80111EC0 | ifCommonItemArrowMakeInterface |
| 0x800FC67C | mpCollisionCheckExistLineID |
| 0x800269C0 | func_800269C0_275C0 (FGM play) |

## Field mapping

- Hitbox: `Item.STRUCT.HITBOX.TYPE` (0x11C) is `attack_coll.element`; `ANGLE` (0x13C) is `attack_coll.angle`; `KNOCKBACK1/2/3` (0x140, 0x144, 0x148) are `knockback_scale`, `knockback_weight`, `knockback_base`; `DAMAGE` (0x110) is `attack_coll.damage`; hurtbox FGM (0x156) is `attack_coll.fgm_id`.
- Owner timer: asm 0x350 is `ITRemixPitfallVars.owner_timer`, a file-local struct cast over `ip->item_vars`.
- Owner: asm 0x008 is `owner_gobj`.
- Floor flag: asm 0x092 is `coll_data.mask_stat`.
- Hurtbox state: asm 0x248 is `damage_coll.hitstatus`.
- Display list: asm DObj 0x50 is `dl`. The 0x3F0 skip is `ITREMIX_PITFALL_DISPLAY_LIST_OFFSET`.
- Buried angle: asm 0xBFC90FF9 is `rotate.vec.f.x` = -1.5707999467849731F.
- Bounce multiplier: asm `lui 0x3F00` is 0x3F000000 = 0.5F.

## Vanilla patches

- `detect_collision_` (asm line 341) copies the vanilla `itMapTestAllCollisionFlag` prologue from 0xEE0F4 (0x88 bytes) and epilogue from 0xEE1CC (0x2C bytes) with the ground-line calls removed. `collision_` passes it in `a1` and then overwrites `a1` with the mask before `jal 0x801737B8`, so this file has no call site for it. The port calls vanilla `itMapTestAllCollisionFlag`, not the modified copy.
- For the Remix behaviour to be preserved, some other Remix file must hook 0x801737B8 to `detect_collision_`. That hook is not in `Pitfall.asm`. The port does not apply it. The C to add, if the hook is confirmed, is a `#ifdef REGION_REMIX` branch in `itMapTestAllCollisionFlag` (`decomp/src/it/itmap.c:153`) that runs the ground-free variant. Check the hook in `Item.asm` before writing it.

## Uncertainties

- Arrow: the asm stores the arrow GObj at ITStruct 0x348. The decomp `arrow_gobj` is at 0x344 (`arrow_timer` is at 0x348). The port uses `arrow_gobj`, as `itremixgem.c` does. Check the offset in the Remix build.
- Buried collision: the asm reads halfword 0x2D0 (the low half of `attr`) with the comment "get a flag?". The port uses `attach_line_id` (0x2CC).
- `collision_` mask: the asm passes 0x0C21 to the collision test and the rebound call, and tests 0x0800 in `mask_stat` for ground. The port keeps those values.

## REMIX-TODO

- `REMIX-TODO(hazard)`: `begin_buried_` destroys the pitfall on a hazardous floor when hazards are enabled. The Surface clipping helper, `Toggles.entry_hazard_mode` and the SAFFRON_CITY exception are not ported, so the pitfall always buries itself.
- `REMIX-TODO(rumble)`: `stick_to_surface` calls `ftParamMakeRumble` when the planting player's match_info entry at offset 0x78 is set. Skipped: the asm passes the item struct, and the match_info field is not named.
- `REMIX-TODO(bitfield)`: `begin_buried_` clears 2 bits at byte 0x2CE. `stick_to_surface` sets bit 0x40 at byte 0x2CF. Neither byte has named ITStruct fields in `ittypes.h`.
- `REMIX-TODO(clang)`: `throw_initial_` and `begin_buried_` clear the clang bit at halfword 0x158. No named field.
- `REMIX-TODO(unknown-halfword)`: `stage_setting_` zeroes the halfword at 0x33E. No named field. `ITStruct.multi` is at 0x33A.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
