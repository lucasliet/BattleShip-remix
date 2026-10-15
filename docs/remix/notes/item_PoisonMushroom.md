# Item PoisonMushroom (0x2F): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/PoisonMushroom.asm`, which reuses `SuperMushroom.asm` (spawn, movement, ground, collision and the effect state machine).
Port: `decomp/src/it/itremix/itremixpoisonmushroom.c`, `decomp/src/it/itremix/itremixpoisonmushroom.h`.
Descriptor: `const ITRemixItemDesc dITRemixPoisonMushroomItemDesc`, kind 0x2F.
Spawn function for fighter call sites: `GObj* itRemixPoisonMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.

## Item file and attributes

- Standard item (no owner). `desc.p_file = &gITManagerCommonData` (file 0xFB, the asm `dw 0x8018D040`), set in the static `ITDesc`.
- `o_attributes = 0xDE0`, the asm `FILE_OFFSET`.

## Dependency on SuperMushroom (not shared state)

The asm routines are SuperMushroom's, and they share globals with it:
- `spawning_shrooms` (spawn count, reset by movement_routine_no_bounce_),
- `player_shrooms[4]` (the active effect per player),
- the different-kind branch of `collide_mushroom_` / `handle_active_mushroom_` (a pickup of the other mushroom ends the active one).

This port keeps its own copies in `itremixpoisonmushroom.c`, so a Super Mushroom effect and a Poison Mushroom effect do not interact. The lead should either point both items at one shared state, or accept this split. The SuperMushroom worker must not rely on this file's state.

## Routine map

| Remix label | C | Role |
|---|---|---|
| spawn_custom_item_based_on_star_ | itRemixPoisonMushroomMakeItem | SPAWN_ITEM |
| (inline) | itRemixPoisonMushroomSetSpawnVel | spawn speed and direction |
| movement_routine_no_bounce_ | itRemixPoisonMushroomProcUpdate | ITDesc proc_update |
| ground_collision_no_bounce_ | itRemixPoisonMushroomProcMap | ITDesc proc_map |
| handle_ground_to_air_transition_ | itRemixPoisonMushroomGroundToAir | empty callback |
| collide_mushroom_ | itRemixPoisonMushroomCollide | PLAYER_COLLISION |
| (inline in collide_mushroom_) | itRemixPoisonMushroomStartEffect | same-item branch only |
| handle_active_mushroom_ | itRemixPoisonMushroomProcEffect, itRemixPoisonMushroomTick, itRemixPoisonMushroomUpdateState, itRemixPoisonMushroomTransition | per-frame effect (poison branch) |
| (inline) | itRemixPoisonMushroomStartFeedback | first-frame sound and flash |
| (inline) | itRemixPoisonMushroomRumble | Global.rumble_ |
| clear_active_mushrooms_ | not ported | no caller in this port (see REMIX-TODO) |

Item info: `ITDesc` kind 0x2F, proc_update = movement, proc_map = ground, proc_hit = `itStarCommonProcHit` (asm 0x1C = 0x80174A0C). Hold, consume, drop, throw are NULL.

## Vanilla functions used

| RAM | Decomp name |
|---|---|
| 0x8016E174 | itManagerMakeItem |
| 0x80174930 (gravity part) | itMainApplyGravityClampTVel (ITSTAR_GRAVITY, ITSTAR_TVEL) |
| 0x80174A0C | itStarCommonProcHit |
| 0x801735A0 | itMapCheckLRWallProcNoFloor |
| 0x801737B8 | itMapTestAllCollisionFlag (MAP_FLAG_FLOOR) |
| 0x801737EC | itMapCheckCollideAllRebound (CEIL, RWALL, LWALL, mod 1.0F, NULL) |
| 0x80172508 | itMainSetSpinVelLR |
| 0x80008CC0 | gcAddXObjForDObjFixed(joint, 0x2E, 0) |
| 0x800269C0 | func_800269C0_275C0 (FGM play) |
| 0x800E9814 | ftParamCheckSetFighterColAnimID(fighter_gobj, 5, 0) (asm `Global.flash_`) |
| 0x80115530 | gmRumbleSetPlayerRumbleParams(port, 7, 60) (asm `Global.rumble_`) |
| 0x800E3A5C | not called: continuation of the touch dispatch in `ftMainUpdateDamageStatItem` |

## Struct field mapping (asm offset to C field)

| asm offset | C field | Note |
|---|---|---|
| 0x92 (halfword, bit 0x800) | coll_data.mask_stat & MAP_FLAG_FLOOR | refs say mask_stat; itmap.c uses it the same way |
| 0x40 of item GObj | ITRemixPoisonMushroomVars.is_grounded | deviation: kept in item_vars |
| 0x10C | attack_coll.attack_state | set to nGMAttackStateOff on pickup |
| 0x154 | attack_coll.interact_mask | GMHITCOLLISION_FLAG_FIGHTER |
| 0x33E | multi | set to 0x20 (asm), vanilla Star uses 16 |
| 0x2D3, bit 0x04 | is_unused_item_bool | |
| 0x30 | physics.vel_air.y | zeroed on floor contact (no bounce) |
| 0x84 of GObj | user_data (ITStruct) | `itGetStruct` |
| FTStruct 0x4 / 0x8 / 0xD / 0x20..0x23 / 0x24 | fighter_gobj / fkind / player / pkind (low byte = player type) / status_id | |

## Routine detail and deviations

- Spawn velocity: `speed = 16 + 2 * count` (count after this spawn's increment). `x = pos.x < cobj.at.x ? +speed : -speed`, then flipped when count is even. `y = speed`, `z = 0`. The `vel` argument is not used (the asm overrides it).
- Translate of the DObj is saved and restored around `gcAddXObjForDObjFixed` (the asm copies it), as in `itremixgem.c`.
- Effect is one `GObjProcess` per activation, added to the fighter GObj with priority 6 (unverified against the Remix Render order 0x8000). It is ended on timeout, on death (fighter status below 4) and on the same-item refresh.
- The asm creates the routine object during the touch and runs its first frame later. Here the first frame (flash and sound, then the timer) runs in the process's first tick. The one-frame difference is accepted.
- The state machine is the poison branch only: shrink (48 frames), maintain until timer 543, grow (57 frames) from timer 544. The effect ends at timer 600.
- Dead code omitted: the asm sound at the first transition frame is skipped because the pickup flash already set its flag.
- Dead code omitted: giant and tiny branches (Size match state). See REMIX-TODO(Size).

## Vanilla patches

None. The asm uses no `OS.patch_start` in PoisonMushroom.asm or SuperMushroom.asm. The item relies only on the existing `ITRemixItemDesc` dispatch and `ftMainUpdateDamageStatItem` touch path.

## REMIX-TODO

- REMIX-TODO(Size): the asm writes `Size.multiplier_table[port]` every frame (shrink keyframes, 0.5 maintain) and reads `Size.match_state_table` for giant and tiny branches. Not ported: the effect state machine runs, but the item does not change the fighter's size. The keyframe tables `transition_frames_shrink`, `transition_sizes_shrink_poison` (and the grow ones for the Super port) are in the asm, lines 657 to 738 of SuperMushroom.asm, and are needed for the size port.
- REMIX-TODO(rumble): the asm rewrites the rumble struct's `rumble_id` to 0x17 so an action change does not cancel the rumble. `gmRumbleSetPlayerRumbleParams` returns nothing, so this is not ported.
- REMIX-TODO(clear): `clear_active_mushrooms_` is not called. Stale effects are discarded when the fighter GObj changes (pointer comparison, no dereference). Pointer reuse of a freed fighter GObj is not handled.

## Uncertainties

- Priority 6 of the effect process (see above).
- `mask_stat` versus `mask_curr` for the floor flag (refs say 0x92 = `mask_stat`, which is what `itmap.c` reads).
- `gcEndGObjProcess` is called from the process's own tick on timeout (mirrors `Render.DESTROY_OBJECT_` on itself).

## Hooks applied 2026-10-14 (this session)

- REWRITE: itremixpoisonmushroom.c is now a thin wrapper matching PoisonMushroom.asm — own ITDesc (FILE_OFFSET 0xDE0) + MakeItem; every routine points at the shared SuperMushroom implementation. The divergent parallel effect machine (StartEffect/EndEffect/ProcEffect/...) is deleted.
- The sweep SIGBUS (gcAddGObjProcess <- itRemixPoisonMushroomStartEffect) lived in the deleted path; re-run the forced-item sweep to confirm.
- Poison.clear_poison_ (poisoned_players status table, src/Poison.asm): still no port — different subsystem, not the mushroom item.
