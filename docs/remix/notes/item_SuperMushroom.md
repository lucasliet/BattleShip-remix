# Item 0x2E SuperMushroom notes

Port: `decomp/src/it/itremix/itremixsupermushroom.c`, header `decomp/src/it/itremix/itremixsupermushroom.h`.
Descriptor: `dITRemixSuperMushroomItemDesc` (`ITRemixItemDesc`, kind 0x2E = `Item.SuperMushroom.id`).
Source: `.toolchain/pkgs/smashremix-src/src/items/SuperMushroom.asm` (739 lines). The same file also holds
the Poison Mushroom logic (`Item.PoisonMushroom.id` 0x2F): the spawn routine indexes the info table by id, and
`handle_active_mushroom_` branches on it.

## Spawn signature for fighter code

`GObj* itRemixSuperMushroomMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

Register mapping: a0 = parent, a1 = pos, a2 = vel (ignored: the asm builds its own velocity on the stack, `addiu a3, sp, 0x34`),
a3 = flags. Fighter call sites are wired by another worker.

## Item file

`item_info_array` word 0x04 is `0x8018D040` (file 0xFB, the common item file), so the port uses
`&gITManagerCommonData`, as MrSaturn does. Attributes: `FILE_OFFSET` 0xD90 (word 0x08) goes into `o_attributes`.
REMIX-TODO(file): the footer at 0xD90 sits in file 0xFB, which is shared with vanilla item data. Check that the port's
file 0xFB contains the Remix data at 0xD90 at runtime.

## Descriptor fields

| Field | Value | Source |
|---|---|---|
| `ITDesc.transform_types` | `{nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}` | word 0x0C = 0x1B000000 |
| `attack_state` | `nGMAttackStateOff` | word 0x10 = 0 |
| `proc_update` | `itRemixSuperMushroomMovementProcUpdate` | word 0x14 `movement_routine_no_bounce_` |
| `proc_map` | `itRemixSuperMushroomGroundProcMap` | word 0x18 `ground_collision_no_bounce_` |
| `proc_hit` | `itStarCommonProcHit` (0x80174A0C) | word 0x1C |
| `is_show_spawn_gfx` | TRUE | `SHOW_GFX_WHEN_SPAWNED(OS.TRUE)` |
| `proc_hold` | NULL | `PICKUP_ITEM_INIT(0)` |
| `proc_consume` | NULL | `PICKUP_ITEM_MAIN(0)` |
| `proc_dropped` | NULL | `DROP_ITEM(0)` |
| `proc_thrown` | NULL | `THROW_ITEM(0)` |
| `proc_touch` | `itRemixSuperMushroomProcTouch` | `PLAYER_COLLISION(collide_mushroom_)` |

No `item_state_table` exists in the asm, so there is no `ITStatusDesc` array.

## Routines ported (asm scope, C routine)

Scopes in the asm: 14 (plus the data tables). Routines ported: 11. `clear_active_mushrooms_` is ported as an exported
function. The data tables are ported as `static const` arrays.

| Asm scope | Line | C routine |
|---|---|---|
| `spawn_custom_item_based_on_star_` | 47 | `itRemixSuperMushroomMakeItem` + `itRemixSuperMushroomGetSpawnVel` + `itRemixSuperMushroomSetupItem` |
| `ground_collision_no_bounce_` | 112 | `itRemixSuperMushroomGroundProcMap` |
| `handle_ground_to_air_transition_` | 149 | `itRemixSuperMushroomGroundToAirNoop` (empty) |
| `movement_routine_no_bounce_` | 157 | `itRemixSuperMushroomMovementProcUpdate` |
| `collide_mushroom_` | 177 | `itRemixSuperMushroomProcTouch` |
| `handle_active_mushroom_` | 238 | `itRemixSuperMushroomProcUpdate` (+ `StartRoutine`, `CheckMatchSize`, `Advance`, `UpdateSize`, `ApplyTransition`, `Revert`) |
| `clear_active_mushrooms_` | 646 | `itRemixSuperMushroomClearStates` (exported) |
| `transition_frames_*`, `transition_sizes_*` | 655-739 | `sITRemixSuperMushroom*Frames`, `*Sizes`, `*PoisonSizes` |

## Vanilla calls and external symbols

| RAM | Port call | Note |
|---|---|---|
| 0x8016E174 | `itManagerMakeItem` | spawn |
| 0x80008CC0 | `gcAddXObjForDObjFixed(dobj, 0x2E, 0)` | render routine 0x2E, same as Star |
| 0x801735A0 | `itMapCheckLRWallProcNoFloor` | grounded branch of ground collision |
| 0x801737B8 | `itMapTestAllCollisionFlag(item, MAP_FLAG_FLOOR)` | not grounded branch |
| 0x801737EC | `itMapCheckCollideAllRebound(item, CEIL\|RWALL\|LWALL, 1.0F, NULL)` | |
| 0x80172508 | `itMainSetSpinVelLR` | |
| 0x80174940 / 0x80174958 | `itMainApplyGravityClampTVel` (gravity skipped while grounded) | |
| 0x800269C0 | `func_800269C0_275C0` | FGM play |
| `Render.register_routine` (ID 0x0400) | `gcMakeGObjSPAfter` + `gcAddGObjProcess`, same as `itremixcloakingdevice.c` | per-effect routine |
| `Render.DESTROY_OBJECT_` | `gcEjectGObj` via `itRemixSuperMushroomFreeRoutine` | |
| `Global.rumble_` (id 7, 60 frames) | `ftParamMakeRumble(fp, 7, 60)` | CPU check is inside `ftParamMakeRumble` |

## Patches the item relies on

### Vanilla patches

1. **Scene-change hook.** `Item.asm` line 1562 calls `SuperMushroom.clear_active_mushrooms_` from
   `clear_active_custom_items_` (called on CSS, VS, Training and VS Results). The port exposes
   `itRemixSuperMushroomClearStates`. No caller exists in the port yet, and `itRemixCloakingDeviceClearStates` has none
   either, so the lead must add one scene-change call for all Remix item clears. Vanilla function: the scene-change
   code that the port uses for `clear_active_custom_items_`. Not applied here. Exact C:

   ```c
   #ifdef REGION_REMIX
   itRemixSuperMushroomClearStates();
   #endif
   ```

   Declaration: `void itRemixSuperMushroomClearStates(void);` (in `itremixsupermushroom.h`).

2. **Touch continuation.** `collide_mushroom_` ends with `j 0x800E3A5C` (`a1 = 0x258`, `a0 = 0x4C(sp)`). The port's
   dispatcher `ftMainUpdateDamageStatItem` (`decomp/src/ft/ftmain.c`, `#ifdef REGION_REMIX` branch, ~line 2727) calls
   `proc_touch` and then continues. Whether that continuation matches 0x800E3A5C is not verified. No vanilla change is
   written here.

### Shared subsystems

- **Poison Mushroom (0x2F) shares this effect state.** `player_shrooms` and the routine logic are used by both mushrooms.
  The port keeps them as file-local statics here, so the Poison port cannot reach them. The lead should move the
  effect pool and `StartRoutine` into a shared module (for example `itremixmushroomeffect.c`) before porting Poison.
- **Hazards.asm `shrink_timer_`** (lines 4741 and 5176) reads `Item.SuperMushroom.player_shrooms`. The shrink ray
  checks for an active mushroom through that table. The port keeps the table private, so the Hazards port must use
  `itRemixSuperMushroomGetActive(port)` (not yet written) or the shared module above.

## REMIX-TODO markers

- **REMIX-TODO(Size)**: `Size.multiplier_table` and `Size.match_state_table` are not ported. The state machine and
  timers run, and the target is kept in `routine->size_multiplier`, but it is never written to the fighter. The match
  state is always NORMAL, so the tiny/giant branches are unreachable until Size is ported. The collision and revert
  paths that depend on it are otherwise faithful.
- **REMIX-TODO(Global.flash)**: `Global.flash_(player object, flash id 5, 0)` (the quick bright flash) is skipped in
  `itRemixSuperMushroomFlash`. The sfx still plays.
- **REMIX-TODO(struct)**: `ori t5, t4, 0x4` on ITStruct byte 0x2D3 (`lbu` / `ori` / `sb`) in the copied Star tail
  (ROM 0xEF51C to 0xEF52C, asm lines 98-100). The struct table does not name that bit, so it is not written. Star's
  `is_unused_item_bool` is also not set. No item function reads it.
- **REMIX-TODO(rumble)**: the asm then writes 0x17 to the rumble info `rumble_id` (`sb t0, 0(v0)`) so an action change
  does not cut the rumble. `ftParamMakeRumble` returns void, so this is skipped and the rumble may be cut early.
- **REMIX-TODO(file)**: see Item file above.

## Uncertainties (asm line numbers)

1. **Grounded flag location** (asm 116-133, 273-274, 286-287). The asm stores the flag at GObj 0x40, which is the
   item gobj scratch field. The port keeps it in `item_vars` (offset 0). This overlaps the shell fields that a shell
   item would use, which is safe because this item has no shell routines.
2. **Movement grounded test** (asm 164-168). `lh a2, 0x92(a0)` is read as `coll_data.mask_stat & 0x0800`
   (MAP_FLAG_FLOOR). The gravity skip follows `j 0x80174940` for not grounded and `j 0x80174958` for grounded.
   The Star body after the gravity call is assumed to be `ip->multi--`, refresh, and visuals spin. This matches
   `itStarCommonProcUpdate` in `itstar.c`.
3. **Spawn velocity** (asm 60-80). The x sign is `-speed` when `pos.x >= cam.at.x`, then flipped for even spawn counts.
   y is `|x|`, so it equals the speed. z is 0. The count is post-increment, matching `andi t1, t1, 1`.
4. **Transition frame indexing** (asm 595-608). The frame index wraps at 12 and is stored only when a step is taken,
   as the asm does with `bnel`.
5. **Death check** (asm 422-424). The asm reads `lhu 0x26` of the player and treats values below 4 as dead.
   The port uses `fp->status_id <= nFTCommonStatusDeadUpFall`, which is the same range (DeadDown..DeadUpFall = 0..3).
6. **Mismatch cancel** (asm 340-381). The port treats "giant + super" and "tiny + poison" as the cancel case, only
   while in transition 1. That is what the asm branches compute.
7. **Routine GObj per touch.** Each pickup creates a routine GObj from a 16-slot pool. Pool exhaustion makes the
   pickup a no-op, which is the safe default.
8. **Crash sound** (asm 219-224). Uses `REMIX_ID_CRASH` (0x48) from `remix/remix_roster_data.h`, the Remix id the asm
   compares with `Character.id.CRASH`.

## Hooks applied 2026-10-14 (this session)

- State unification with Poison Mushroom LANDED: itremixsupermushroom.c is now the single owner of the shared asm state (player_shrooms/spawning_shrooms as the routine pool + spawn count); GetSpawnVel, SetupItem, MovementProcUpdate, GroundProcMap and ProcTouch are exported via itremixsupermushroom.h and used by both descs.
- REMIX-TODO(struct) closed: ITStruct 0x2D3 bit 0x4 written as is_unused_item_bool = TRUE in SetupItem.
- REMIX-TODO(Global.flash) closed: _flash adds the quick bright colanim via ftParamCheckSetFighterColAnimID(gobj, 5, 0).
- Vanilla patch for touch continuation (ftmain.c 0x800E3A5C fall-through): still pending — the dispatcher at ftMainSearchHitItem already routes custom kinds to desc->proc_touch, equivalence to the asm jump not verified.
