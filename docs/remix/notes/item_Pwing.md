# Item 0x37 Pwing notes

Port: `decomp/src/it/itremix/itremixpwing.c`, header `decomp/src/it/itremix/itremixpwing.h`.
Descriptor: `dITRemixPwingItemDesc` (`ITRemixItemDesc`, kind 0x37 = `Item.Pwing.id`, 0x2D + 10 in `Item.asm` `add_item` order).
Source: `.toolchain/pkgs/smashremix-src/src/items/Pwing.asm` (about 450 lines).

## Spawn signature for fighter code

`GObj* itRemixPwingMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

The asm `SPAWN_ITEM` is `Item.spawn_custom_item_based_on_tomato_`. It copies the vanilla tomato spawn
and replaces only `a1` with this item's info array, so the port calls `itManagerMakeItem` with its own
`ITDesc` and then runs the tomato post-spawn steps (`gcAddXObjForDObjFixed`, `is_unused_item_bool`, arrow).
No velocity override. Fighter call sites are wired by another worker.

## Item file

`item_info_array` word 0x04 is `0x8018D040` (file 0xFB, `gITManagerCommonData`), word 0x08 is `FILE_OFFSET` 0x1060.
The port sets `ITDesc.p_file = &gITManagerCommonData` and `o_attributes = 0x1060`, the same as MrSaturn.
REMIX-TODO(file): offset 0x1060 is inside the Remix-patched file 0xFB. Not verified at runtime (no build run).

## Descriptor fields

| Field | Value | Source |
|---|---|---|
| `ITDesc.transform_types` | `{nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}` | word 0x0C = 0x1C000000, same as Maxim Tomato's `dITTomatoItemDesc` |
| `attack_state` | `nGMAttackStateOff` | word 0x10 = 0 |
| `proc_update` | `itTomatoFallProcUpdate` (0x801744C0) | word 0x14 |
| `proc_map` | `itTomatoFallProcMap` (0x80174524) | word 0x18 |
| `is_show_spawn_gfx` | TRUE | `SHOW_GFX_WHEN_SPAWNED(OS.TRUE)` |
| `proc_hold` | NULL | `PICKUP_ITEM_INIT(0)` |
| `proc_consume` | `itRemixPwingPickup` | `PICKUP_ITEM_MAIN(pickup_pwing)` |
| `proc_dropped` | `itTomatoDroppedSetStatus` (0x801745FC) | `DROP_ITEM(0x801745FC)` |
| `proc_thrown` | NULL | `THROW_ITEM(0)` |
| `proc_touch` | NULL | `PLAYER_COLLISION(0)` |

## Item state table (not ported as a table)

`item_state_table` at 0x34 + 0x20 * n (ITStatusDesc has 8 words):

| State | update | map |
|---|---|---|
| 0 | NULL | 0x801744FC (`itTomatoWaitProcMap`) |
| 1 | 0x801744C0 (`itTomatoFallProcUpdate`) | 0x80174524 (`itTomatoFallProcMap`) |
| 2 | 0x801744C0 | 0x801745CC (`itTomatoDroppedProcMap`) |
| 3 | all NULL | all NULL |

Not ported as a C table. `ITDesc` and `ITRemixItemDesc` have no status-table field, and no routine in
this item calls `itMainSetStatus`. The item's status procs are the vanilla tomato ones listed above.
The table is only used by the Remix dispatch, which the port does not have for this item.

## Routines ported (asm scope, C routine)

Asm scopes: `pickup_pwing` (line 62), `handle_active_pwing_` (line 110, with the white circle block inside it),
`clear_active_pwings_` (line 429). The spawn routine is `Item.spawn_custom_item_based_on_tomato_`.
Ported: `pickup_pwing` -> `itRemixPwingPickup`, `handle_active_pwing_` -> `itRemixPwingProcUpdate` plus helpers,
`clear_active_pwings_` -> `itRemixPwingClearActive`, the white circle block -> `itRemixPwingWhiteCircleGfx`,
the spawn routine -> `itRemixPwingMakeItem`. The `_check_*` labels of `handle_active_pwing_` are split into
`itRemixPwingCanJump`, `itRemixPwingCheckBonk`, `itRemixPwingIsBonkCooldown`, `itRemixPwingEvaluateJumping`,
`itRemixPwingEvaluateCooldown`, `itRemixPwingEvaluateRemaining` and `itRemixPwingEvaluateJump`.

## Per-player state

`players_with_pwing` (asm 442) and `pwing_jump_flag` become `sITRemixPwingPlayers[MAXCONTROLLERS]`, indexed by
`fp->player`. Each slot holds the owner GObj, the process handle, the timer and the jump state (0 none, 1 jumping, 2 bonked).

## Vanilla patches

- The asm applies no `OS.patch_start` of its own. The pickup enters through the generic `item_pickup_check_`
  in `Item.asm` (line 395), which jumps to `pickup_pwing`. The port reaches the vanilla tail through
  `itRemixConsumeTomatoTail` (0x80145C4C), so no vanilla change is needed for that.
- REQUIRED WIRING: `Item.clear_active_custom_items_` calls `Pwing.clear_active_pwings_` (`Item.asm` line 1571).
  That scope is called from `Render.asm` at lines 2224, 2305 and 2345. The port must call `itRemixPwingClearActive()`
  from the same scene-end and intro paths, next to the existing custom-item clear. Its vanilla address is not in
  `vanilla_funcs.txt`, so the lead must add the call.

## REMIX-TODO

- REMIX-TODO(GFXRoutine): the pickup sets the Pwing glow gfx routine 0x7A as the player's override
  (`GFXRoutine.port_override`, asm `pickup_pwing` lines 62 to 108). The port has no override table, so the glow is not shown.
- REMIX-TODO(GFXRoutine): on destroy the asm removes the override and calls `ftParamResetStatUpdateColAnim`
  (0x800E98D4) when the override is the Pwing one. Not ported for the same reason.

## Deviations

- Per-frame work runs as a `gcAddGObjProcess` on the owner's fighter GObj (`nGCProcessKindFunc`, priority 4),
  not as `Render.register_routine`. The asm's group and order are not reproduced. The process ends with
  `gcEndGObjProcess` when the Pwing ends.
- The duplicate-pickup check uses the slot's process and the owner GObj identity, not the non-zero
  `players_with_pwing` pointer. A stale slot whose owner GObj changed is overwritten.
- Action ids are the Remix values from `Action.asm` (KIRBY Jump3..6 = 0xE0..0xE3, JIGGLY Jump4 = 0xE1),
  `Dedede.asm` (JUMP_4 = 0xE1) and `Ebi.asm` (USP = 0xDF). The asm compares them without checking the fighter,
  so the port does the same. Fighter ids are `REMIX_ID_*` from `remix/remix_roster_data.h`.

## Uncertainties

- `_check_multi_jump` (asm 235 to 252): the asm loads `Action.KIRBY.Jump6` into `at` after the Jump5 compare but never
  compares it. The port does not compare it either.
- `_check_bonk` (asm 188 to 207): Marina's `jumps_used = 6` is written only on the bonk branch. The port matches
  that, and the bonk sets the state even when the state was already bonked.
- `_pwing_sfx` (asm 306 to 320): on jump frame 0 the sfx plays and no jump is granted (the asm branches to
  `subtract_timer`). The port matches this in `itRemixPwingEvaluateCooldown`.
- `_destroy_pwing` (asm 335 to 377): the asm writes `jumps_used` from `jumps_max` without the Marina override
  (base 2 or 4). The port uses `itRemixPwingGetJumpsBase` there.
- The white circle size uses the DObj `scale.vec.f.x` of `joints[0]` (the asm's `0x8E8` topjoint).
  The offset `0x0040` was read as `scale.vec.x`. Not checked at runtime.
- Process priority 4 has no basis in the asm. It is only chosen so that the update runs after the fighter update.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
