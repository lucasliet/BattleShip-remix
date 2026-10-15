# Item Dango (0x36): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Dango.asm`, which is a Maxim Tomato clone with its own item info and a damage-restore pickup.
Port: `decomp/src/it/itremix/itremixdango.c`, `decomp/src/it/itremix/itremixdango.h`.
Descriptor: `const ITRemixItemDesc dITRemixDangoItemDesc`, kind 0x36.
Spawn function for fighter call sites: `GObj* itRemixDangoMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.

## Item file and attributes

- Standard item (no owner). `desc.p_file = &gITManagerCommonData` (file 0xFB, the asm `dw 0x8018D040`), set in the static `ITDesc`.
- `o_attributes = 0x1010`, the asm `FILE_OFFSET`.

## Routine map

| Remix label | C | Role |
|---|---|---|
| spawn_custom_item_based_on_tomato_ | itRemixDangoMakeItem | SPAWN_ITEM |
| pickup_main_ | itRemixDangoConsume | PICKUP_ITEM_MAIN |
| (vanilla) | itTomatoFallProcUpdate, itTomatoFallProcMap | ITDesc proc_update, proc_map (0x801744C0, 0x80174524) |
| (vanilla) | itTomatoDroppedSetStatus | DROP_ITEM (0x801745FC) |

PICKUP_ITEM_INIT, THROW_ITEM and PLAYER_COLLISION are 0 in the asm, so NULL here.

## Vanilla functions used

| RAM | Decomp name |
|---|---|
| 0x8016E174 | itManagerMakeItem(parent, &desc, pos, vel, flags) (asm passes the caller's vel in a3) |
| 0x80008CC0 | gcAddXObjForDObjFixed(joint, 0x2E, 0) |
| 0x80111EC0 | ifCommonItemArrowMakeInterface(ip), stored at ip->arrow_gobj |
| 0x80145C40 | ftParamSetHealDamage(fp, 10) (asm `pickup_main_` sets a1 = 10 and jumps here) |
| 0x80145C4C | itRemixConsumeTomatoTail (destroy item and count the 1P bonus) |

## Spawn

The asm spawn is `itTomatoMakeItem` (0x80174624) with the Dango info array. The decoded fragment (`0xEF08C`) keeps the DObj translate across `gcAddXObjForDObjFixed` and sets `is_unused_item_bool` (asm 0x2D3, bit 0x04) and `arrow_gobj` (asm 0x348). This port follows the same order. It takes the REGION_US translate path, as `itremixgem.c` does.

## Vanilla patches

None. Dango.asm has `PICKUP_ITEM_MAIN(pickup_main_)` and `DROP_ITEM(0x801745FC)` only. The shared `item_pickup_check_` hook in `Item.asm` is not part of this item.

## Dependencies on vanilla tomato

The four ITDesc callbacks and the status table are the vanilla Maxim Tomato ones (`dITTomatoStatusDescs`: 0x801744FC, 0x801744C0, 0x80174524, 0x801745CC). The Dango info array points at the same addresses, so the item reuses them through `itTomato*`.

## REMIX-TODO

None.

## Uncertainties

- The asm's pickup jumps to 0x80145C40 with a1 = 10, and the port treats that as `ftParamSetHealDamage(fp, 10)`. The address is not in the decomp sources; the value 10 is from `DAMAGE_RESTORE` in the asm. The Maxim Tomato heals `ITTOMATO_DAMAGE_HEAL` (100), which is different, as the asm says.
- The consume tail also runs the 1P tomato bonus counter, which is the registry's semantics for every Remix consume item (`itRemixConsumeTomatoTail`).

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
