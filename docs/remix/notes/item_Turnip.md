# Item.Turnip (0x4B) port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Turnip.asm`
Port: `decomp/src/it/itremix/itremixturnip.c`, `decomp/src/it/itremix/itremixturnip.h`
Descriptor: `const ITRemixItemDesc dITRemixTurnipItemDesc` (kind 0x4B)
Spawn signature: `GObj* itRemixTurnipMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`. The asm's a1/a2/a3 map to pos/vel/flags, as in vanilla `itManagerMakeItem`.
Item file: Peach file 7, `fp->data->p_file_special2`. The `item_info_array` is copied per spawn and `p_file` is set from the owner before `itManagerMakeItem`. The footer offset 0x40 goes into `ITDesc.o_attributes`.
Face setter: `void itRemixTurnipSetFace(GObj *item_gobj, ITRemixTurnipFace face)`, which sets `attack_coll.damage` and `mobj->texture_id_curr`. The enum is `nITRemixTurnipFaceNormal/Wink/Dot/Stitch`.

## Routines

| Asm | C | Status |
|---|---|---|
| `stage_setting_` (L104-236) | `itRemixTurnipMakeItem` + `itRemixTurnipSetFace` + `itRemixTurnipRollFace` | ported |
| `throw_initial_` (L239-254) | `itRemixTurnipThrownProc` (descriptor THROW_ITEM) | ported |
| `reflected_initial_` (L257-273) | `itRemixTurnipReflectedSetStatus` | ported |
| `falling_initial_` (L277-298) | `itRemixTurnipDroppedProc` (descriptor DROP_ITEM) | ported |
| `thrown_main_` (L302-332) | `itRemixTurnipThrownProcUpdate` | ported |
| `throw_collide_` (L336-360) | `itRemixTurnipThrownCollide` (hit, shield and set-off slots) | ported |
| `throw_collide_hitbox_` (L364-388) | `itRemixTurnipThrownCollideHitbox` (damage slot) | ported |
| `throw_collision_` (L393-411) | `itRemixTurnipThrownProcMap` (map slot) | ported |
| `detect_collision_` (L415-428) | not ported | not referenced by the Turnip (see uncertainty 1) |
| `reflect_` (L432-445) | `itRemixTurnipReflectorProc` (reflector slot) | ported |
| `prepickup_` (L449-464) | `itRemixTurnipHoldProc` (descriptor PICKUP_ITEM_INIT) | ported |
| `pickup_` (L468-475) | `itRemixTurnipConsumeProc` (descriptor PICKUP_ITEM_MAIN, no-op) | ported, see Vanilla patches |

`item_state_table` is `sITRemixTurnipStatusDescs`. Statuses 0 and 1 use the tomato wait and fall procs from `itcommon/ittomato.c`, and status 4 uses the tomato fall proc. The item descriptor's `proc_update` and `proc_map` are the tomato fall procs (`0x801744C0`, `0x80174524`), as in the asm's `item_info_array`. The hop slot is `itMainCommonProcHop` (`0x801733E4`).

Item vars live in `ITStruct.item_vars` through `ITRemixTurnipVars`, with a `_Static_assert` that it fits. The `throw_timer` field is N64 0x1CC, which the asm uses as both rotation direction (set to 0 at spawn) and THROW_TIMER.

## Vanilla patches

### pickup_ continues in ftCommonLightGetProcDamage (Turnip.asm L468-475)

- `pickup_` ends with `j 0x80145C4C`. `build/remixasm/refs/vanilla_funcs.txt` puts 0x80145C4C inside `ftCommonLightGetProcDamage` (`decomp/src/ft/ftcommon/ftcommonget.c:81`, entry 0x80145BE4).
- The port does not patch that function. It assumes the existing dispatcher calls the descriptor's `proc_consume` and then continues with its own tail, which is where the asm jumps. The consume proc is therefore an empty function.
- No C patch is needed if the dispatcher already routes ids >= 0x2D to `itRemixGetItemDesc` at this site. The infrastructure is described in `itremix.h`. Not verified in this pass.

## Shared subsystems (REMIX-TODO)

- `REMIX-TODO(ITStruct)`, N64 0x398: `stage_setting_` (L168) clears the blast-wall routine slot. The field has no ITStruct name, so the port does not write it. The blast-wall caller is not ported.
- `REMIX-TODO(ITStruct)`, N64 0x33E: `stage_setting_` (L142, L169) clears the halfword "flag used for bomb". The field has no ITStruct name, so the port does not write it. Only Bob-omb code reads it.
- `REMIX-TODO(ITStruct)`, N64 0x2D3: `_continue` (L220-222) sets bit 0x04. The N64 table places the `attr` pointer at 0x2D0, so the byte does not map to a named field, and the port does not write it.
- `REMIX-TODO(ITStruct)`, N64 0x2CE mask 0x02: `falling_initial_` (L284-286) clears the "don't randomly despawn" bit. In the decomp it is part of `pickup_wait`, and the port's timer write (L291) replaces the whole value, so the result is the same. Marked in code.

## Vanilla calls used

| Asm address | Decomp function | Used by |
|---|---|---|
| 0x8016E174 | `itManagerMakeItem` | stage_setting_ |
| 0x8017279C | `itMainClearOwnerStats` | stage_setting_ |
| 0x80008CC0 | `gcAddXObjForDObjFixed(joint, 0x2E, 0)` | stage_setting_ |
| 0x80111EC0 | `ifCommonItemArrowMakeInterface` | stage_setting_ |
| Global.get_random_int_ | `syUtilsRandIntRange(58)` | face roll |
| 0x801713F4 | `itVisualsUpdateSpin` | thrown_main_ |
| 0x80172558 | `itMainApplyGravityClampTVel` | thrown_main_ |
| 0x80172EC8 | `itMainSetStatus` | all status changes |
| 0x801737B8 | `itMapTestAllCollisionFlag` | throw_collision_ |
| 0x80173434 | `itMainCommonProcReflector` | reflect_ |
| 0x801733E4 | `itMainCommonProcHop` | status 3 hop slot |
| 0x80177218 | `itBombHeiCommonSetHitStatusNone` | prepickup_ |

## Peach and Marina call sites

- Peach `PeachSpecial.asm` L523-524 spawns with `Item.Turnip.SPAWN_ITEM`, and the face is random inside the spawn. The fighter call site therefore needs only `itRemixTurnipMakeItem`.
- Peach `PeachSpecial.asm` L456-461 checks `attack_coll.damage` (0x110) against 30 (stitch) for the BINGO line.
- Marina `MarinaSpecial.asm` L2473-2482 reads 0x110 and maps 30, 12 and 6 to charge levels 4, 3 and 2. The port keeps those values.
- `itRemixTurnipSetFace` is exported for the case where a caller needs a fixed face. No current call site uses it.

## Uncertainties (asm line numbers)

1. `detect_collision_` (L415-428) copies vanilla bytes from ROM 0xEE0F4 and 0xEE1CC and is not called by the Turnip. The boomerang probably uses it. It is not ported.
2. L37: the transform word 0x1B000000 is mapped to `nGCMatrixKindTraRotRpyR`, copied from tomato. The numeric enum value was not checked against 0x1B.
3. L291: `sh GROUND_TIMER, 0x02D2`. The N64 offset 0x02D2 overlaps the `attr` pointer in the table, so the timer is mapped by meaning to `pickup_wait`, the counter that `itprocess.c:121` decrements.
4. L308 and L324: the asm comments say "apply rotation" and "apply movement", but both call 0x801713F4, which vanilla names `itVisualsUpdateSpin`. The port calls it twice in the same order as the asm.
5. L181-217: the face roll. 0-51 normal (damage 2, image 6). 52 stitch (damage 30, image 4). 53 dot (damage 12, image 5). 54-57 wink (damage 6, image 3). The code follows the asm's branches; the slti/beql sequence was re-checked against the comments.
6. L226: `swc1 f4, 0x0038(t6)` writes 0.0 to the DObj at 0x38, mapped to `rotate.vec.f.z` by `struct_offsets.txt`. The write is placed before the arrow call, as in the asm.
7. L131-161: the asm saves DObj 0x1C (`translate.vec.f`) before `itMainClearOwnerStats` and `gcAddXObjForDObjFixed`, then restores it. The port does the same.
8. L125-127 and L213-216: the image index is written to `DObj.mobj` (0x80) and then to `MObj.texture_id_curr` (0x80). The first write (5) is always overwritten by the face write, so it is kept only for fidelity.
9. L40-41 and L89-90: the item's `0x14` and `0x18` procs are the tomato fall procs, and the port uses them directly.
10. 0x1D0 and 0x1D4 (asm L164-167) are kept in `ITRemixTurnipVars` with no reader, as in the Cloud port. `ftremixyounglink.c` says N64 0x1D0/0x1D4 is an FGM handle slot, which conflicts with this. Check the two ports when either is wired up.
11. L320: the gravity constant 0x3FE0999A is 1.7546875476837158F, which is not exact in decimal. The hex is kept in the define comment.
12. `pickup_` (L468-475) passes its consumed item as `a2`. The port takes `item_gobj` as the second argument of `proc_consume`, which matches the descriptor typedef.
