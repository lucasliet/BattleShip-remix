# Item 0x33 FranklinBadge notes

Port: `decomp/src/it/itremix/itremixfranklinbadge.c`, header `decomp/src/it/itremix/itremixfranklinbadge.h`.
Descriptor: `dITRemixFranklinBadgeItemDesc` (`ITRemixItemDesc`, kind 0x33 = `Item.FranklinBadge.id`, 0x2D + 6 in `Item.asm` `add_item` order).
Source: `.toolchain/pkgs/smashremix-src/src/items/FranklinBadge.asm` (440 lines).

## Spawn signature for fighter code

`GObj* itRemixFranklinBadgeMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

SPAWN_ITEM is `Item.spawn_custom_item_based_on_tomato_` (`Item.asm` line 374). It copies the vanilla
Maxim Tomato spawn (ROM 0xEF064 and 0xEF08C) and only replaces a1 with the item info array. The port
mirrors the Tomato spawn body with the Franklin item info. Vanilla ABI, so no wrapper is needed.

## Item file

`item_info_array` word 0x04 is `0x8018D040` (file 0xFB, `gITManagerCommonData`). The port sets
`ITDesc.p_file = &gITManagerCommonData`, the same as the Maxim Tomato.
`FILE_OFFSET` 0xF20 goes into `o_attributes` as a raw offset.
REMIX-TODO(file): the footer at 0xF20 is inside file 0xFB, shared with vanilla item data. The Remix build
must include the badge data in that file. Not verified in this pass (no build run, per brief).

## Descriptor fields

| Field | Value | Source |
|---|---|---|
| `kind` | 0x33 | `Item.FranklinBadge.id` |
| `proc_make` | `itRemixFranklinBadgeMakeItem` | `SPAWN_ITEM` |
| `is_show_spawn_gfx` | TRUE | `SHOW_GFX_WHEN_SPAWNED(OS.TRUE)` |
| `proc_hold` | NULL | `PICKUP_ITEM_INIT(0)` |
| `proc_consume` | `itRemixFranklinBadgePickupConsume` | `PICKUP_ITEM_MAIN(pickup_franklin_badge)` |
| `proc_dropped` | `itTomatoDroppedSetStatus` (0x801745FC) | `DROP_ITEM` |
| `proc_thrown` | NULL | `THROW_ITEM(0)` |
| `proc_touch` | NULL | `PLAYER_COLLISION(0)` |

`item_info_array`: transform word 0x0C = `0x1C000000`, kept as a raw value (`ITREMIX_FRANKLIN_TRANSFORM_KIND`).
Its neighbour in the port, Maxim Tomato, uses `nGCMatrixKindTraRotRpyR` (0x1B). The name of 0x1C was not found
in this pass. Word 0x10 = 0 (`nGMAttackStateOff`). Word 0x14 = 0x801744C0 (`itTomatoFallProcUpdate`). Word 0x18 = 0x80174524 (`itTomatoFallProcMap`).

## Item state table: no port table

The asm `item_state_table` label is at 0x30. Its ITStatusDesc entries start at 0x34 with stride 0x20:
state 0 = {0, `itTomatoWaitProcMap`}, state 1 = {`itTomatoFallProcUpdate`, `itTomatoFallProcMap`},
state 2 = {`itTomatoFallProcUpdate`, `itTomatoDroppedProcMap`}. This is the same table as
`dITTomatoStatusDescs` in `ittomato.c`. The port does not declare a second copy. DROP_ITEM is the Tomato
routine, which uses that table. The word at 0x30 is 0 and is not part of a state.

## Routines ported (asm scope, C routine)

Asm scopes in `FranklinBadge.asm`: 8 (`pickup_franklin_badge`, `handle_active_franklin_badge_`, `reflect_initial_`,
`reflect_gfx_`, `keep_on_action_change_`, `clear_active_franklin_badges_`, `grant_projectile_immunity_`,
`grant_item_immunity_`), plus the spawn routine in `Item.asm`.

| Asm | C | Status |
|---|---|---|
| `pickup_franklin_badge` | `itRemixFranklinBadgePickupConsume`, `itRemixFranklinBadgeActivate` | ported |
| `handle_active_franklin_badge_` | `itRemixFranklinBadgeProcUpdate`, `itRemixFranklinBadgeEnd`, `itRemixFranklinBadgeRegisterProcess` | ported (process on fighter GObj) |
| `reflect_initial_` | `itRemixFranklinBadgeReflectInitial` | ported; not yet wired (see Reflect TODO) |
| `reflect_gfx_` | none | REMIX-TODO(GFX) |
| `keep_on_action_change_` | `itRemixFranklinBadgeKeepsReflect` (condition only) | hook not applied, see Vanilla patches |
| `clear_active_franklin_badges_` | `itRemixFranklinBadgeClearAll` | ported; caller is `Item.asm` `clear_active_custom_items_` (line 1568), not in this file |
| `grant_projectile_immunity_` | `itRemixFranklinBadgeIsActive` (condition only) | hook not applied, see Vanilla patches |
| `grant_item_immunity_` | none | hook not applied, see Vanilla patches |
| `players_with_franklin_badge` | `sITRemixFranklinBadgeStates[MAXCONTROLLERS]` | ported (file-local) |
| `reflect_hitbox_struct` | `sITRemixFranklinBadgeReflectColl` | ported |
| `spawn_custom_item_based_on_tomato_` (Item.asm) | `itRemixFranklinBadgeMakeItem` | ported |

Ported state for each player: `is_active`, `timer` (frames) and `process_fighter_gobj`. The process is added
once per fighter GObj. Each frame it does the asm countdown: a dead player (status id below 0x0A) ends the
badge at once, a player still holding the badge hitbox keeps `is_reflect`, and the timer decrements.
On expiry, or on death, the badge ends. The reflect flag clears unless the player is dead.

## Vanilla patches (recorded, not applied)

1. `keep_on_action_change_`: RAM 0x800E70EC (ROM 0x628EC), inside `ftMainSetStatus` (decomp/src/ft/ftmain.c:4580, function start 0x800E6F24).
   The asm keeps the reflect bit on a status change when the badge is active and the hitbox is the badge's.
   Exact C to add where the vanilla code clears `is_reflect` on a status change, under `#ifdef REGION_REMIX`:
   ```c
   if (itRemixFranklinBadgeKeepsReflect(fp) == FALSE)
   {
       /* vanilla line: clear the reflect flag */
   }
   ```
   The prototype `sb32 itRemixFranklinBadgeKeepsReflect(FTStruct *fp);` is in `itremixfranklinbadge.h`.
   The exact vanilla line was not mapped in this pass (the asm operates on t4, t5, t6, t7).
2. `grant_projectile_immunity_`: RAM 0x800E54F4 (ROM 0x60CF4), inside `ftMainSearchHitWeapon`
   (decomp/src/ft/ftmain.c:3374, function start 0x800E4ED4). The asm's logic is: immune if projectile damage is disabled
   OR the badge is active. Otherwise the hurtbox check runs. Exact C to add:
   ```c
   if (projectile_damage_enabled == FALSE || itRemixFranklinBadgeIsActive(fp))
   {
       /* skip the hurtbox check, as the asm jumps to 0x800E558C */
   }
   ```
   `projectile_damage_enabled` and `fp` are the vanilla locals; the names are not verified.
3. `grant_item_immunity_`: RAM 0x800E5B48 (ROM 0x61348), inside `ftMainSearchHitItem`
   (decomp/src/ft/ftmain.c:3571, function start 0x800E55DC). The asm logic in order:
   - item damage disabled: immune;
   - item is SuperMushroom (0x2E), PoisonMushroom (0x2F) or Star: no immunity;
   - no badge: no immunity;
   - badge and item is PIRANHA_PLANT: no immunity;
   - badge and item < ELECTRODE: immune;
   - badge and ELECTRODE <= item <= MEW: no immunity;
   - badge and item > MEW: immune.
   Exact C: the Hazards constants (`Hazards.stage.PIRANHA_PLANT`, `ELECTRODE`, `Hazards.pokemon.MEW`) are not
   resolved to C values in this pass. The hook must be written once those values are known.
   The asm also jumps to 0x800E5BE0 (immune) and 0x800E5B58 (collision check).

## Shared subsystems and REMIX-TODOs

- REMIX-TODO(GFX): the badge gfx routine override (`GFXRoutine.port_override.override_table`, `GFXRoutine.FRANKLIN`,
  player struct 0x0A28, which the asm labels "todo: typo") is not set on pickup. The restore of the default gfx routine
  (vanilla 0x800E98D4, `ftParamResetStatUpdateColAnim`) is skipped on expiry. The badge is still usable without it.
- REMIX-TODO(GFX): `reflect_gfx_` spark effect (`GFX` id 5 via vanilla 0x800CE870, then part coordinates from
  0x800EDF24) is not ported. `reflect_initial_` skips it.
- REMIX-TODO(Reflect): the custom reflect routine that calls `reflect_initial_` (`Reflect.custom_reflect_table`, type 4)
  is not ported. `itRemixFranklinBadgeReflectInitial` is exported but not called.
- REMIX-TODO(Reflect): the Ness shield-break and absorb states (flag 0x0880 and 0x0C80 in the 16-bit word at 0x18C) are not
  ported. The port's `is_reflect` is the 0x0400 bit only. The asm keeps the timer running through absorb.

## Uncertainties

- Process priority `ITREMIX_FRANKLIN_PROCESS_PRIORITY` (1) is chosen, not stated in the asm.
- The `fp->special_coll` check in `itRemixFranklinBadgeKeepsReflect` assumes that Fox and Ness replace the hitbox pointer
  the way the asm reads 0x0850. The asm comment says so, but it was not traced.
- Reflect hitbox `joint_id` takes the asm word "?" (4). The asm's size comment says 512, but the bits 0x43AF0000 are 350.0F.
  The port uses 350.0F.
- Transform value 0x1C (`item_info_array` word 0x0C) is kept as a raw value.
- `itRemixFranklinBadgeMakeItem` keeps the translate save and restore from the vanilla US spawn path. The non-US spawn sets `*pos`. The region is not confirmed for this port.

## Hooks applied 2026-10-14 (this session)

- keep_on_action_change_ (0x800E70EC) applied in ftMainSetStatus: the is_reflect clear is guarded by itRemixFranklinBadgeKeepsReflect(fp); is_absorb/is_shield clears stay unguarded as in the asm. The vanilla line was found: the 0xFFFB byte-0x18C mask = fp->is_reflect (struct_offsets.txt corroborated by remix-ROM disasm of the trampoline neighborhood).
- grant_projectile_immunity_ (0x800E54F4) applied in ftMainSearchHitWeapon: badge-active fighters continue past the weapon damage-collision loop.
- grant_item_immunity_ (0x800E5B48) applied in ftMainSearchHitItem via new helper itRemixFranklinBadgeIsItemImmune(fp, ip) (itremixfranklinbadge.c): mushrooms/star never absorbed; badge absorbs everything except Piranha Plant (nITKindPakkun) and the pokemon range Marumine..Mew (Hazards constants resolved: ELECTRODE is Hazards.stage, not pokemon).
- Hooks B/C sit at the gFTMainIsDamageDetect continue guards of the wp_attack_coll/it_attack_coll damage loops (immune = continue, as the asm jumps to the loop tail).
