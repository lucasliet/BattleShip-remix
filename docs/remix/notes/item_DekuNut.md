# Item DekuNut (0x32): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/DekuNut.asm`.
Port: `decomp/src/it/itremix/itremixdekunut.c`, `decomp/src/it/itremix/itremixdekunut.h`.
Descriptor: `const ITRemixItemDesc dITRemixDekuNutItemDesc`, kind 0x32.
Spawn function for fighter call sites: `GObj* itRemixDekuNutMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.

## Item file and attributes

- Standard item (no owner file). `dw 0x8018D040` is file 0xFB, so `ITDesc.p_file = &gITManagerCommonData`.
- `o_attributes = 0x0ED0`, which is `FILE_OFFSET` (the asm item footer). It is written as a file offset, not as the `0x08` slot name.
- The Remix build patches file 0xFB in place. The port assumes the footer exists at 0x0ED0 (same assumption as the Mr. Saturn port at 0x10B0). Not verified in a build.

## Routine map

| Remix label | C | Role |
|---|---|---|
| spawn_deku_nut_ | itRemixDekuNutMakeItem, itRemixDekuNutSetup | SPAWN_ITEM and its setup |
| prepickup_ | itRemixDekuNutPrepickup | PICKUP_ITEM_INIT |
| (pickup_deku_nut) | itRemixConsumeTomatoTail (shared) | PICKUP_ITEM_MAIN, tail jump to 0x80145C4C |
| throw_initial_ | itRemixDekuNutThrowInitial | DROP_ITEM and THROW_ITEM |
| throw_physics_ | itRemixDekuNutThrownProcUpdate | thrown proc_update |
| throw_duration_ | itRemixDekuNutThrownProcMap | thrown proc_map |
| collide_transition | itRemixDekuNutCollideTransition | map callback, status 6 |
| collide_transition_player_ | itRemixDekuNutCollidePlayer | thrown hit, shield, setoff, damage |
| collide_with_wall_ | itRemixDekuNutCollideWall | status 6 procs |
| explode_transition_ | itRemixDekuNutExplodeTransition | helper |
| explode_nut_ | itRemixDekuNutExplode | helper |
| explode_subroutine_ | itRemixDekuNutSetDamageAll | helper |
| explode_initial_2 | itRemixDekuNutInitExplode | helper |
| explode_initial_ | itRemixDekuNutFlashSetStatus | status 7 entry |
| set_nut_flash_ | itRemixDekuNutSetFlash | hurtbox step, helper |
| nut_active_ | itRemixDekuNutFlashProcUpdate | status 7 proc_update |
| hurtbox_struct_ | sITRemixDekuNutFlashSteps | static table |
| item_state_table | sITRemixDekuNutStatusDescs | status table, 8 entries |

Status ids follow the asm offsets: 0 wait, 1 fall, 2 pickup, 3 thrown, 6 collide with wall, 7 flash. Statuses 4 and 5 are empty in the asm and are NULL here.

## Vanilla functions used

| RAM | Decomp name |
|---|---|
| 0x8016E174 | itManagerMakeItem |
| 0x80008CC0 | gcAddXObjForDObjFixed(joint, 0x2E, 0) |
| 0x80111EC0 | ifCommonItemArrowMakeInterface |
| 0x80172558 | itMainApplyGravityClampTVel (gravity 1.7546875F, terminal 100.0F) |
| 0x801713F4 | itVisualsUpdateSpin (asm comment says "apply movement"; the address-to-name mapping is used) |
| 0x80172EC8 | itMainSetStatus |
| 0x80173E58 | itMapCheckMapProcAll |
| 0x801733E4 | itMainCommonProcHop |
| 0x80173434 | itMainCommonProcReflector |
| 0x80177180 | itBombHeiCommonCheckMakeDustEffect(item, FALSE) |
| 0x80177208 | itBombHeiCommonSetHitStatusNormal |
| 0x80177218 | itBombHeiCommonSetHitStatusNone |
| 0x801008F4 | efManagerQuakeMakeEffect(1) |
| 0x800269C0 | func_800269C0_275C0 (FGM play, 0x407 explode, 0x408 hurtbox) |
| 0x801744C0, 0x80174524, 0x801744FC | itTomatoFallProcUpdate, itTomatoFallProcMap, itTomatoWaitProcMap |
| 0x80145C4C | tail of the consume path, via itRemixConsumeTomatoTail (shared with Gem) |

## Vanilla patches

- None applied. The asm patches nothing in vanilla code. The PICKUP_ITEM_MAIN jump to 0x80145C4C is handled by the shared dispatch via `itRemixConsumeTomatoTail`, as in Gem. Not verified in the dispatch.

## Struct field mapping (asm offset to C field)

The attack collision block starts at 0x10C (the item info array's hitbox base), so the offsets below are `0x10C + ITAttackColl field`.

| asm offset | C field | Note |
|---|---|---|
| 0x10C / 0x110 | attack_coll.attack_state / damage | damage is written from the hurtbox step |
| 0x114 | attack_coll.throw_mul | set to 1.0F in explode_initial_2 |
| 0x11C | attack_coll.element | asm name "damage type"; value 0x0A = DEKU_STUN |
| 0x138 | attack_coll.size | from the hurtbox step, as float |
| 0x13C | attack_coll.angle | decoded as 269 |
| 0x140 | attack_coll.knockback_scale | asm comment says "bkb"; the field is knockback_scale (0x34 in ITAttackColl) |
| 0x156 | attack_coll.fgm_id | 0x408 |
| 0x158 bits 0x40, 0x08, 0x04, 0x80 | can_rehit_item TRUE, can_hop FALSE, can_reflect FALSE, can_setoff FALSE | |
| 0x2C / 0x30 / 0x34 | physics.vel_air.x / .y / .z | zeroed in explode_transition_ |
| 0x2CF, bit 0x80 | is_damage_all = TRUE | inferred from the Remix comment on explode_subroutine_ (ownership). Bit position not verified |
| 0x2D3, bit 0x04 | is_unused_item_bool | same as Gem and Bob-omb spawn |
| 0x33E | multi | timer and hurtbox step index |
| 0x340, high nibble | event_id | hurtbox step index (0 to 3) |
| 0x348 | arrow_gobj | |
| 0x14 | team = ITEM_TEAM_DEFAULT (4) | |

## Decoded data

- `hurtbox_struct_` (4 entries, 8 bytes each): byte 0 is the multi value that applies the step (0x00, 0x02, 0x04, 0x06). The halfword at +4 is the size (382, 250, 150, 0).
- Angle and damage are the same for all steps. Word 0x00434040: angle is bits 31..22 signed, which is 269. Damage is bits 13..6, which is 1.

## Deviations from the asm

1. The asm's `explode_transition_` takes a flag in a1. `explode_nut_` stores it and never reads it. The port drops the parameter.
2. The asm's `explode_subroutine_` clears the owner first. The Remix comment says that was removed, so only the team and damage-all parts are ported.
3. The asm's spawn returns `a3` (the velocity pointer) when `itManagerMakeItem` fails. The port returns NULL, which matches the asm's own "no item spawned" intent.
4. `set_nut_flash_` writes `0x11C` (element) in the delay slot of the event-check branch. The port writes it inside the matched branch, which is the only case that matters after the timer check.

## REMIX-TODO markers

- **REMIX-TODO(GFX)**: `explode_nut_` screen flash (`flash_array_`, a blend colour command written to RAM 0x80131A40). Skipped. It is also skipped when Toggles.entry_flash_guard is off, because the port has no Toggles entry for it. The explosion still works without the flash.
- **REMIX-TODO(unverified)**: `explode_nut_` stores 2 at DObj offset 0x54. The port's DObj has no named field at that offset, so the store is skipped. The meaning is unknown. This is the only unnamed store.

## Uncertainties

- `knockback_scale` at 0x140: the asm writes a halfword with value 0x100. The port stores 0x100 as an int. The N64 field width may differ.
- `is_damage_all` bit at 0x2CF: inferred from the Remix comment; not checked against the struct bit layout.
- The `itVisualsUpdateSpin` mapping at 0x801713F4: the asm comment says "apply movement". The address mapping in vanilla_funcs.txt is used.
- Angle 269 and damage 1 come from the bit decode of one word. Values are the same for all four steps, so the decode may be missing a field.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
