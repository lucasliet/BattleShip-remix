# Item 0x31 Lightning notes

Port: `decomp/src/it/itremix/itremixlightning.c`, header `decomp/src/it/itremix/itremixlightning.h`.
Descriptor: `dITRemixLightningItemDesc` (`ITRemixItemDesc`, kind 0x31 = `Item.Lightning.id`, 0x2D + 4 in `Item.asm` `add_item` order).
Source: `.toolchain/pkgs/smashremix-src/src/items/Lightning.asm` (441 lines).

## Spawn signature for fighter code

`GObj* itRemixLightningMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

Same ABI as `itManagerMakeItem`. Fighter call sites are wired by another worker.

## Item file

`item_info_array` word 0x04 is `0x8018D040` ("hard-coded pointer to file 0xFB"), which is `gITManagerCommonData`.
The port sets `ITDesc.p_file = &gITManagerCommonData` and `o_attributes = 0xE80` (`FILE_OFFSET`).
No owner-fighter `p_file_special*` is used. Same as the Maxim Tomato item.
REMIX-TODO(file): the footer at 0xE80 is inside file 0xFB, shared with vanilla item data. The Remix build
must contain the Lightning footer there. Not verified in this pass (no build run, per brief).

## Descriptor fields

| Field | Value | Source |
|---|---|---|
| `kind` | 0x31 | `Item.Lightning.id` |
| `proc_make` | `itRemixLightningMakeItem` | `SPAWN_ITEM` = `spawn_custom_item_based_on_tomato_` |
| `is_show_spawn_gfx` | FALSE | `SHOW_GFX_WHEN_SPAWNED(OS.FALSE)` |
| `proc_hold` | NULL | `PICKUP_ITEM_INIT(0)` |
| `proc_consume` | `itRemixLightningConsume` | `PICKUP_ITEM_MAIN(pickup_lightning_)` |
| `proc_dropped` | `itTomatoDroppedSetStatus` (0x801745FC) | `DROP_ITEM` |
| `proc_thrown` | NULL | `THROW_ITEM(0)` |
| `proc_touch` | NULL | `PLAYER_COLLISION(0)` |

## Spawn routine

`spawn_custom_item_based_on_tomato_` is the Maxim Tomato spawn with the item info array swapped. The two
`OS.copy_segment` fragments (ROM 0xEF064 0x1C and 0xEF08C 0xA4) were decoded with
`tools/remix_disasm_rom.py`. They call `itManagerMakeItem`, save and restore the DObj translate around
`gcAddXObjForDObjFixed(joint, 0x2E, 0)`, set `is_unused_item_bool` (0x2D3 bit 0x4) and set
`arrow_gobj = ifCommonItemArrowMakeInterface(ip)`. That is `itTomatoMakeItem` in `ittomato.c`, so the port
mirrors it with the Lightning `ITDesc`. The decoder's vram base gives 0x80173864, not the port's
`itTomatoMakeItem` address 0x80174624. The instructions match, so the offset mismatch is the tool's default base.

Status routines come from the Tomato set (`itTomatoFallProcUpdate`, `itTomatoFallProcMap`, Tomato
`SetStatus` functions). The asm's `item_state_table` is the Tomato table, so no Lightning status table is needed.

## Routines ported versus the asm

| Asm routine | Port | Notes |
|---|---|---|
| `spawn_custom_item_based_on_tomato_` | `itRemixLightningMakeItem` | Mirrors `itTomatoMakeItem` |
| `pickup_lightning_` | `itRemixLightningConsume` + `itRemixLightningStartRoutine` | Camera flash skipped (REMIX-TODO(GFX)) |
| `aerial_main_` | `itRemixLightningRoutineProcUpdate` | Per-pickup routine GObj; timer 25 zap, 20 sfx, 0 end |
| `lightning_zap_players_` | `itRemixLightningZapPlayers` + `itRemixLightningCanZap` + `itRemixLightningZapPlayer` | Franklin Badge and Size effects skipped |
| `shrink_timer_` | `itRemixLightningShrinkProcUpdate` + `itRemixLightningShrinkTick` | Super Mushroom clear and Size reset skipped |
| `clear_active_lightning_routine_` | `itRemixLightningClearActive` | Also resets the routine pool, see Vanilla patches |
| `shrink_timer_table` | `sITRemixLightningShrinkTimers[MAXCONTROLLERS]` | Indexed by `fp->player` |
| `lightning_routine_` | `sITRemixLightningShrinkGObj` | Pointer for the one shrink routine |

Not ported: the camera flash in `pickup_lightning_` and `aerial_main_`, and the `0xFF` timer branch in
`aerial_main_` (unreachable, the `0` end check runs first).

## Per-player state

- Per-frame lightning state lives in `sITRemixLightningRoutines[ITREMIX_LIGHTNING_ROUTINE_MAX]`. Each routine has
  a GObj from `gcMakeGObjSPAfter(nGCCommonKindInterface, ...)` with `user_data.p` pointing at its slot.
  The Remix code uses `Render.register_routine`, which has no port equivalent. The GObj link is a deviation
  to check at review.
- Pool size is 8. A full pool consumes the item with no zap. The Remix code has no cap.
- Shrink timers are `sITRemixLightningShrinkTimers[port]`, indexed by `fp->player`, as the asm indexes by port byte.

## Vanilla patches

- The asm itself has no `OS.patch_start`. Its pickup return jumps to 0x80145C4C, which the port
  reaches through `itRemixConsumeTomatoTail`. No patch needed.
- `Item.asm` `clear_active_custom_items_` (line 1565) calls `Lightning.clear_active_lightning_routine_`
  on CSS, VS, Training and VS Results load. The port equivalent is `itRemixLightningClearActive()`.
  The call site is not applied. The clean-up path must call it, in the same place as the other
  `clear_active_*` calls for Remix items:
  ```c
  itRemixLightningClearActive();
  ```
  The call goes under `#ifdef REGION_REMIX` in the item clean-up function that the port uses for Remix items.
  The port call site was not located in this pass.

## REMIX-TODOs

- `REMIX-TODO(GFX)`: camera flash in `pickup_lightning_` (camera 0x80131460, mode 2, colour white, skipped when
  `Toggles.entry_flash_guard` is set). Also the per-frame dark blue or white camera colour
  (timer bit 4) and the reset to mode 4 at routine end.
- `REMIX-TODO(Size)`: `Size.multiplier_table[port]` set to 0.5F in zap, and the reset at shrink timer end
  (from `Size.match_state_table`).
- `REMIX-TODO(SuperMushroom)`: destroy an active Super Mushroom on zap, clear shrink timers of ports with a
  Super Mushroom in `shrink_timer_`.
- `REMIX-TODO(FranklinBadge)`: Franklin Badge reflect check (`FranklinBadge.players_with_franklin_badge`) is
  skipped. Every eligible player is zapped.
- `REMIX-TODO(file)`: see Item file above.

## Uncertainties

- `aerial_main_` lines 174-176: the `addiu t2, 0x0004` in the branch delay slot runs only when the branch is taken,
  and the next `addiu t2, 0x0002` runs only on the non-end path. The port uses the same split by checking
  timer == 0 first, and the end path does not set the camera mode (REMIX-TODO(GFX)).
- `lightning_zap_players_` team check uses `gSCManagerVSBattleState.is_team_battle` and `is_team_attack`
  for the asm's `Global.match_info` bytes 0x02 and 0x09. Not checked against the in-game match struct.
  The VS battle state is the closest port source.
- The asm resets all four shrink slots to 0x1E0 before zapping, then sets zapped players to 600.
  Ported as-is. Non-zapped ports with 0x1E0 still count down, which only matters once Size is ported.
- Team, action and invincibility checks follow asm order: owner, team, revive actions (Remix ids 7, 8, 9),
  `invincible_tics`, `star_invincible_tics`.
- Revive action ids are the Remix `Action.asm` values, compared numerically. They are not mapped to
  `nFTCommonStatusRebirth*`, because the vanilla numbering (`nFTCommonStatusEntryNull` onwards) does not
  match the Remix one.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
