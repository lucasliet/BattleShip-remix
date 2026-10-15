# Item.Shuriken (id 0x43) notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Shuriken.asm`
Port: `decomp/src/it/itremix/itremixshuriken.c`, `decomp/src/it/itremix/itremixshuriken.h`
Descriptor: `const ITRemixItemDesc dITRemixShurikenItemDesc` (kind 0x43)
Item file: Marina file 8 (`Character.MARINA_file_8_ptr`), so `fp->data->p_file_special3`. Attributes offset 0x90 (FILE_OFFSET) goes into `o_attributes`.

## Routines

Ported (14 of 15 routines in the `.asm`):

| Asm label | C | Slot |
|---|---|---|
| stage_setting_ | itRemixShurikenMakeItem | SPAWN_ITEM |
| throw_initial_ | itRemixShurikenThrowSetStatus (static) | THROW_ITEM |
| reflected_initial_ | itRemixShurikenReflectedSetStatus | called by reflect_ |
| slow_initial_ | itRemixShurikenSlowSetStatus | called by thrown and hit routines |
| falling_initial_ | itRemixShurikenFallSetStatus | called by slow_main_ |
| thrown_main_ | itRemixShurikenThrownProcUpdate | state 3 main |
| slow_main_ | itRemixShurikenSlowProcUpdate | state 4 main |
| throw_collide_ | itRemixShurikenThrownProcHit | state 3 hurtbox, shield, clang |
| slow_collide_ | itRemixShurikenSlowProcHit | state 4 hurtbox, shield, clang |
| throw_collision_ | itRemixShurikenThrownProcMap | state 3 collision |
| slow_collision_ | itRemixShurikenSlowProcMap | state 4 collision |
| detect_collision_ | itRemixShurikenDetectCollision | callback of slow_collision_ (approximation, see below) |
| reflect_ | itRemixShurikenProcReflector | state 3 and 4 reflector |
| prepickup_ | itRemixShurikenPrepickupProc | PICKUP_ITEM_INIT |

Not ported as a routine: `pickup_shuriken` (PICKUP_ITEM_MAIN). It is a REMIX-TODO, see below.

Vanilla routines reused by the descriptor, the status tables and the spawn:

- `itTomatoFallProcUpdate`, `itTomatoFallProcMap`, `itTomatoWaitProcMap`, `itTomatoDroppedProcMap`, `itTomatoDroppedSetStatus` (ittomato.c, the addresses 0x801744C0, 0x80174524, 0x801744FC, 0x801745CC, 0x801745FC from the asm's state tables and DROP_ITEM).
- `itMainCommonProcHop` (0x801733E4), `itMainCommonProcReflector` (0x80173434), `itMainSetStatus` (0x80172EC8), `itMainClearOwnerStats` (0x8017279C), `itMainClearAttackRecord`.
- `itMapTestAllCollisionFlag` (0x801737B8), `mpProcessUpdateMain` (0x800DA034), `itVisualsUpdateSpin` (0x801713F4).
- `itBombHeiCommonSetHitStatusNone` (0x80177218), `ifCommonItemArrowMakeInterface` (0x80111EC0), `gcAddXObjForDObjFixed` (0x80008CC0, render index 0x2E), `efManagerDustLightMakeEffect` (0x800FF048), `func_800269C0_275C0` (0x800269C0, FGM play).

## REMIX-TODO

1. **REMIX-TODO(fighter pickup tail): `pickup_shuriken` is not ported. `proc_consume` is NULL.** The asm sets `a2`/`a3` in the stack and jumps to 0x80145C4C. That address is inside the vanilla light-get routine, after `item_pickup_check_` (0x80145C14). Decoded so far (`tools/remix_disasm_rom.py 0xC1400 0x80 --vram 0x80145C00`, and 0x80145C7C onward):
   - `a1 = 0x7A` or `0x7B`, chosen by `lb 0x1C2(fp) * fp->0x44`, which is the FGM choice;
   - for fighter actions `0x02, 0x10, 0x1A`, `a1 += 0x7C` and the status is set with `ftCommonItemThrowSetStatus` (0x80146690);
   - `fp->proc_damage` (offset 0x9EC) is set to `ftCommonHeavyGetProcDamage` (0x80145D28).
   The rest of the routine was not decoded. Effect in the port: picking up a Shuriken does nothing beyond the fighter's own pickup. Fill this in once the tail is decoded. The vanilla patch belongs in `ftcommonget.c`, under `#ifdef REGION_REMIX`, and must not be applied by the item worker.

## Vanilla patches the item relies on

None applied. The Remix item table patches are handled by the shared dispatch in `itmain.c`, `itmanager.c` and `ftcommonget.c`, which already route ids >= 0x2D to `dITRemixShurikenItemDesc`. The only patch the item needs is the pickup tail above (item 1).

## Mapping decisions (asm offset to C field)

The brief asks for field names. The table below lists the offset that the asm uses and the field it was mapped to by meaning.

| Asm access | C field | Note |
|---|---|---|
| `0x08(item)` owner | `ip->owner_gobj` | |
| `0x2C/0x30(item)` velocity | `ip->physics.vel_air.x/y` | |
| `0x84(item)` | `itGetStruct(item)` | |
| `0x110`, `0x13C`, `0x140`, `0x148` | `attack_coll.damage`, `angle`, `knockback_scale`, `knockback_base` | 2, 150, 50, 70 |
| `0x156` (halfword) | `attack_coll.fgm_id` | 0x0106 |
| `0x10C` (hitbox off) | `attack_coll.attack_state = nGMAttackStateOff` | |
| `0x248` (hurtbox off) | `damage_coll.hitstatus = 0` | 0 = none, per the ITDamageColl comment |
| `0x1CC` (timer) | `ITRemixShurikenVars.status_timer` in `ip->item_vars` | Remix scratch slot in ITStruct; moved into `item_vars` as the brief asks. Size checked with `_Static_assert`. |
| `0x1D0` (hitbox refresh) | `ITRemixShurikenVars.hit_refresh_timer` in `ip->item_vars` | Same move |
| `0x224..0x23C` (4 hit pointers) | `itMainClearAttackRecord(ip)` | Asm lines 382-385 |
| `0x33E` (halfword, spawn) | `ip->multi = 0` | Same as `itBombHeiMakeItem`, which does `ip->multi = 0` |
| `0x2D3` (OR 4, spawn) | `ip->is_unused_item_bool = TRUE` | Same as `itBombHeiMakeItem` |
| `0x348` (arrow) | `ip->arrow_gobj` | |
| `0x2CF` bit 0x20 (thrown) | `ip->is_thrown = FALSE` | See uncertainty 1 |
| DObj `0x38` (rotate z) | `dobj->rotate.vec.f.z = 0.0F` | |

## Uncertainties

1. **Asm line 298-300**: `lbu/andi 0xDF/sb` at 0x2CF clears the bonus-damage bit on the first thrown frame. The port maps it to `ip->is_thrown`. The meaning matches the ITStruct comment ("Apply magnitude and throw multiplier to damage output"). However, `build/remixasm/refs/struct_offsets.txt` places `is_thrown` at 0x2CB bit 2, not 0x2CF, so the byte does not match. Check the ITStruct layout against the N64 struct before trusting the mapping.
2. **Asm line 302, 305**: `beqz` on the velocity integer bits. The port compares the floats with `== 0.0F`, which treats -0.0 as zero. The asm does not. The difference is minor but it is not bit-exact.
3. **Asm line 418-419 (`throw_collide_`)**: the hitbox refresh timer is stored through `v1`, which this routine never sets. The port stores it through `ip->item_vars`, which is the evident intent. The N64 behaviour depends on whatever `v1` held, so this is a deliberate deviation. `slow_collide_` (line 443-444) uses `v0`, which is correct.
4. **Asm line 487 (`slow_collision_`)**: `a2 = s0`, commented as "item object". The port passes `item_gobj`. Confirm that `s0` is the item at this call site.
5. **`detect_collision_` (asm lines 507-520)**: the 0x88-byte copy from 0x801737B8 is the wall, ceiling and floor part of `itMapProcAllCheckCollisionFlag`. The 0x2C-byte copy from 0x80173890 is the return test `(mask_stat & coll_flags) != 0`. The port rebuilds that: walls and ceiling as in vanilla, floor check plus `mpProcessSetLandingFloor`, then the return test. Vanilla's floor-edge adjust and map-collide sfx (`itMapProcAllCheckCollisionFlag` lines 133-138) are not in the Remix copy, so they are left out. The offsets in the copy were checked by disassembly only up to the wall/ceiling part, so this function is an approximation.
6. **Spawn, `0x8017279C` and `0x80008CC0`**: the asm stores the return of `0x8017279C` as a pointer, but `itMainClearOwnerStats` returns void. The port treats it as the bombhei path does, which is a call with `item_gobj`. This matches `itBombHeiMakeItem` (itbombhei.c:747-749).
7. **Smoke effect `0x800FF048`** (asm lines 329-333): arguments taken as `(pos, lr = 1, f_index = 1.0F)`. The 0x3F80 in `a2` is `1.0F`. The lr value is inferred from `lli a1, 1`.
8. **Descriptor transform types**: the asm word at 0x0C is `0x1B000000`. The port uses `{ nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 }`, as the Maxim Tomato descriptor does. The asm word has the byte layout of `DObjTransformTypes`, and the port assumes `nGCMatrixKindTraRotRpyR == 0x1B`. Not verified against the enum.
9. **Asm comments that are stale**: the `FILE_OFFSET` comment says "Gem item file", but Shuriken uses Marina file 8. The `prepickup_` comment says it "won't run" because the drop routine is based on the tomato. The port keeps the routine because the dispatch in `itmain.c` calls `proc_hold` for Remix ids.
10. **Sfx and FGM ids**: `func_800269C0_275C0(0x0107)` for the stab hit and `0x0106` for the hit FGM are raw ids. No named enum in `gmsound.h` was found for them.
11. **Throw angle**: the asm checks `Action.ItemThrowDash` (0x069), which the port maps to `nFTCommonStatusLightThrowDash` (ftdef.h, status 105). The low two action bits decide the angle as derived from asm lines 205-214: `(action & 3) == 0` gives 270, `(action & 2) != 0` gives 150, otherwise 90.

## Verification

- `scripts/remix/syntax-check.sh decomp/src/it/itremix/itremixshuriken.c` prints `syntax-check OK` with no warnings.
- Not run: cmake, the full build, the game, `tools/gen_remix_items.py`, commits. `itremixitems.c` was not edited, as the brief asks.
