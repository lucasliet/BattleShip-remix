# Item Gem (0x42): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Gem.asm` (used by Marina).
Port: `decomp/src/it/itremix/itremixgem.c`, `decomp/src/it/itremix/itremixgem.h`.
Descriptor: `const ITRemixItemDesc dITRemixGemItemDesc`, kind 0x42.
Spawn function for fighter call sites: `GObj* itRemixGemMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.

## Item file and attributes

- `dw Character.MARINA_file_8_ptr` is N = 8, so `desc.p_file = fp->data->p_file_special3`, set on a stack copy of the static `ITDesc` before each `itManagerMakeItem`.
- `o_attributes = 0x40`, which is `FILE_OFFSET` (the asm item footer). The `0x08` in the brief names the field's slot in `item_info_array`, not its value. `itManagerMakeItem` reads it with `lbRelocGetFileData(..., *p_file, o_attributes)`, so it must be the file offset.

## Routine map

| Remix label | C | Role |
|---|---|---|
| gem_stage_setting_ | itRemixGemMakeItem | SPAWN_ITEM |
| (inline in gem_stage_setting_) | itRemixGemApplyVariant | owned-gem stats and palette |
| hurtbox_collision_ | itRemixGemHurtboxCollision | thrown status proc_damage |
| apply_multiplier_ | itRemixGemApplyMultiplier | helper |
| falling_initial_ | itRemixGemDisableBoxes | helper |
| collide_with_player_ | itRemixGemCollideWithPlayer | thrown proc_hit, proc_shield, proc_setoff |
| set_idle_aerial_ | itRemixGemBounceBack | helper of collide_with_player_ |
| set_idle_aerial_2 | itRemixGemSetFallStatus | status change to fall |
| prepickup_ | itRemixGemPrepickup | PICKUP_ITEM_INIT |
| throw_initial_ | itRemixGemThrowInitial | THROW_ITEM |
| throw_duration_ | itRemixGemThrownProcMap | thrown proc_map |
| collide_transition | itRemixGemCollideTransition | map callback, status 4 |
| collide_with_clipping_ | itRemixGemCollideWithClipping | status 4 proc_update |
| destroy_gem_ | itRemixGemDestroy | helper |
| shatter_gfx_ | itRemixGemShatterGfx | helper |
| pickup_gem | itRemixGemPickupMain | PICKUP_ITEM_MAIN (empty, see below) |

Status table: `sITRemixGemStatusDescs[5]`, states 0 to 4. The state table in the asm lists state 4 at offset 0x80 under a stale "STATE 6" label; the port uses state 4.

## Vanilla functions used

| RAM | Decomp name |
|---|---|
| 0x8016E174 | itManagerMakeItem |
| 0x8017279C | itMainClearOwnerStats |
| 0x80008CC0 | gcAddXObjForDObjFixed(joint, 0x2E, 0) |
| 0x80111EC0 | ifCommonItemArrowMakeInterface |
| 0x80172EC8 | itMainSetStatus |
| 0x80173F78 | itMapSetAir |
| 0x80173E58 | itMapCheckMapProcAll |
| 0x80177180 | itBombHeiCommonCheckMakeDustEffect |
| 0x80177208 | itBombHeiCommonSetHitStatusNormal |
| 0x80177218 | itBombHeiCommonSetHitStatusNone |
| 0x80177530 | itBombHeiThrownProcUpdate |
| 0x801733E4 | itMainCommonProcHop |
| 0x80173434 | itMainCommonProcReflector |
| 0x8017A734 | itRShellCommonClearAnim |
| 0x80178704 | itGShellCommonClearAnim |
| 0x80101790 | efManagerShieldBreakMakeEffect(pos) (asm passes extra args, which the decomp signature ignores) |
| 0x800269C0 | func_800269C0_275C0 (FGM play) |
| 0x80018994 | syUtilsRandIntRange (inferred from red and green shell code) |
| 0x80018948 | syUtilsRandFloat (inferred from itgshell.c; the asm reads the result from f0) |
| 0x801744C0, 0x80174524, 0x801744FC | itTomatoFallProcUpdate, itTomatoFallProcMap, itTomatoWaitProcMap |
| 0x801745FC | itTomatoDroppedSetStatus (DROP_ITEM) |

## Vanilla patches and shared subsystems

- REMIX-TODO(vanilla consume tail): `pickup_gem` stores a2 and a3 to the stack, then `j 0x80145C4C`, which is the tail of the vanilla tomato and heart pickup. The port's `itRemixGemPickupMain` is empty. This requires the consume dispatch (`ftCommonLightGetProcDamage`) to continue into 0x80145C4C after calling `proc_consume`. Not verified in the dispatch.
- No vanilla patch is applied. The Gem relies on the existing `ITRemixItemDesc` dispatch for spawn, hold, drop, throw and consume.

## Struct field mapping (asm offset to C field)

| asm offset | C field | Note |
|---|---|---|
| 0x110 / 0x140 / 0x148 | attack_coll.damage / knockback_scale / knockback_base | matches refs |
| 0x10C | attack_coll.attack_state | |
| 0x156 | attack_coll.fgm_id | |
| 0x248 | damage_coll.hitstatus | 0 none, 1 normal |
| 0x268 | hit_lr | matches red shell code |
| 0x2C / 0x30 | physics.vel_air.x / .y | |
| 0x33E | multi | matches itBombHeiMakeItem (`ip->multi = 0`) |
| 0x344 in refs, 0x348 in asm | arrow_gobj | asm offset is 4 more than refs |
| 0x2D3, bit 0x04 | is_unused_item_bool | matches itBombHeiMakeItem; refs place the byte at 0x2CF |
| 0x2D4 in asm | attr | refs place attr at 0x2D0; attr->map_coll_bottom is asm 0x2E |
| 0x2CE, bit 0x80 | pickup_wait &= ~0x800 | the 12-bit field's top bit; see uncertainties |
| 0x351 | item_vars.shell.interact | |
| 0x352 | item_vars.shell.health | asm is 4 more than the refs (0x34E); see uncertainties |
| DObj 0x1C / 0x38 | translate.vec.f / rotate.vec.f.z | |
| DObj 0x80 | mobj | |
| MObj 0x80 / 0x88 | texture_id_curr (image index) / palette_id | |

## Deviations from the asm

1. Spawn failure: asm returns the velocity pointer (`or v0, a3` with a3 still the vel argument). The port returns NULL, which matches the asm's own "no item spawned" intent.
2. No-owner spawn: the asm's blue gem branch (line 142) is not ported. Without an owner the file cannot be bound, so the port returns NULL.
3. `collide_transition` (line 462): the asm reads `0x74(s0)` where s0 is stale. The port uses the item GObj, which is the evident intent.
4. `shatter_gfx_` (lines 552 to 557): the asm stores interact = 8 and then unconditionally stores stale `t1` into the same byte. The port keeps interact = 8 only.
5. `pickup_gem` tail: see the REMIX-TODO above.

## Uncertainties

- Line 297: the comment says the default multiplier is `bf80` (-1.0F), but the code loads `0x3F80` (+1.0F). The port uses +1.0F, as the code does.
- `set_idle_aerial_` (lines 353 to 363): `mul.s f18, f0, f16` uses f0 as the random float returned by 0x80018948. This matches the green shell's `syUtilsRandFloat() * ...`.
- Hit velocity multiplier (line 309 to 312): the literal at RAM 0x8018CDA8 is taken to be the same as ITRSHELL_RECOIL_MUL_X (0.7F), since the red shell reads the same address. NOT verified against baserom.us.z64. The port uses the local macro `ITREMIX_GEM_HIT_VEL_MUL_X 0.7F`.
- Health offset: the asm writes health at 0x352, which is 4 bytes past the refs value (0x34E). The port uses the named field `item_vars.shell.health`, on the assumption that the asm means the same byte. Check against the built struct.
- Pickup wait: the byte at 0x2CE has the top bit cleared. The port maps this to `pickup_wait &= ~0x800`, assuming the bitfield is MSB-first. Check against the built struct.
- Dead code not ported: `lbu t6, 0x355` and the `t8` computation in collide_with_player_ (lines 263 to 266), and `lw t1, 0x108` in collide_with_player_ (line 318). None of them feeds a store.
- `shatter_gfx_` passes a1 and a2 to `efManagerShieldBreakMakeEffect`. The decomp signature takes only `pos`, so they are not passed.
- Desc `transform_types = {nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}`, which is the asm word `0x1B000000`. Enum value 0x1B was confirmed by counting.
- `ITAttributes` FILE_OFFSET is 0x40. Confirm that Marina's special3 file has the Gem footer there.
