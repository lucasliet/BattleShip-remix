# Item Stopwatch (Remix id 0x39) port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Stopwatch.asm` (1233 lines, read in full).
Output: `decomp/src/it/itremix/itremixstopwatch.c`, `itremixstopwatch.h`, descriptor `dITRemixStopwatchItemDesc`.
Verification: `scripts/remix/syntax-check.sh decomp/src/it/itremix/itremixstopwatch.c` prints `syntax-check OK` with no warnings. Not built, not run in game.

## Item interface

- Kind: `ITREMIX_STOPWATCH_KIND 0x39`. Descriptor fields: spawn `itRemixStopwatchMakeItem`, `SHOW_GFX_WHEN_SPAWNED` TRUE, PICKUP_ITEM_INIT NULL, PICKUP_ITEM_MAIN `itRemixStopwatchPickupProc` (consume type), DROP_ITEM `itTomatoDroppedSetStatus` (asm DROP_ITEM 0x801745FC), THROW_ITEM NULL, PLAYER_COLLISION NULL.
- Spawn signature: `GObj* itRemixStopwatchMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`. Same as `itTomatoMakeItem` but with the Stopwatch `ITDesc`, which is the asm's `spawn_custom_item_based_on_tomato_`.
- `ITDesc`: `kind 0x39`, `p_file = &gITManagerCommonData` (file 0xFB, `dw 0x8018D040`), `o_attributes = 0x1120` (`FILE_OFFSET`). Proc update and map are `itTomatoFallProcUpdate` (0x801744C0) and `itTomatoFallProcMap` (0x80174524). No owner file is used: standard item, `parent_gobj` may be NULL.
- Status table (asm `item_state_table` 0x30 to 0x9C) names only tomato routines: wait map `itTomatoWaitProcMap` (0x801744FC), fall update and map, and `itTomatoDroppedProcMap` (0x801745CC). The Stopwatch item does not carry its own table, because the tomato statuses own those transitions.

## Routines: ported versus asm

| Asm routine | Lines | C | State |
|---|---|---|---|
| `pickup_stopwatch` | 66-201 | `itRemixStopwatchPickupProc` | ported |
| `clock_out_` | 203-401 | `itRemixStopwatchClockOut`, `...ClockOutPlayer`, `...ApplySpeedUp`, `...ApplySlowdown` | ported |
| `handle_active_stopwatch_` | 403-659 | `itRemixStopwatchHandleActive`, `...TickPlayer`, `...TickEngine`, `...Expire`, `...SpawnEndCircle`, `...Destroy` | ported (GFX and BG parts skipped, see below) |
| `check_function_` | 661-948 | `itRemixStopwatchCheckFunction` and helpers | ported; exported for Speed.asm |
| `clear_active_stopwatch_` | 1069-1098 | `itRemixStopwatchClearActive` | ported; exported for Item.asm |
| `spawn_custom_item_based_on_tomato_` (item info part) | Item.asm 374-389 | `itRemixStopwatchMakeItem` | ported |
| `physics_map` (3 hitlag patches) | 958-1010 | none | vanilla patch, not applied (see below) |
| `platform_despawn_shadow_crash_fix` | 1192-1233 | none | vanilla patch, not applied (see below) |
| data tables and flags | 1100-1187 | `sITRemixStopwatch*` tables and statics | ported |
| commented `proc_update` hitlag hooks | 1012-1065 | none | commented out in the asm, not ported |

## Vanilla patches (not applied, C recorded)

All are under `#ifdef REGION_REMIX` in the named vanilla function. The C is the minimum to add.

1. `physics_map` hitlag branches, all inside `ftMainProcPhysicsMap` (`decomp/src/ft/ftmain.c`, starts at 0x800E2048):
   - RAM 0x800E2088 (asm hitlag_check_1, lines 979-988). Replace the hitlag load with
     `hitlag = itRemixStopwatchIsFunctionAbbr() ? 1 : fp->hitlag_tics;`
   - RAM 0x800E240C (asm hitlag_check_2, lines 990-999). The asm calls `0x800EB528` (`ftParamsUpdateFighterPartsTransformAll`, vanilla_funcs.txt line 722) with `fp->joints`, then sets `v0 = itRemixStopwatchIsFunctionAbbr() ? 1 : fp->hitlag_tics;`.
   - RAM 0x800E2420 (asm hitlag_check_3, lines 1001-1009). The asm calls `0x800E0654` with the stack argument at 0x70(sp), then `v0 = itRemixStopwatchIsFunctionAbbr() ? 1 : fp->hitlag_tics;`.
   - Prototype needed: `sb32 itRemixStopwatchIsFunctionAbbr(void);` (in `itremixstopwatch.h`).
   - Reason: a frame frozen by the slow-down skips the hitlag decrement so the player stays frozen.
2. `platform_despawn_shadow_crash_fix` at RAM 0x8013B014 (asm lines 1192-1233). Reason: a platform despawning under a player while the Stopwatch is active crashes in the shadow code. The C would be: when the clipping status of the platform whose ID is at 0x00EC of the fighter struct is 3 or more, call `ftCommonFallSetStatus(fp->fighter_gobj)` and skip the vanilla branch. The vanilla function containing 0x8013B014 is not listed in `build/remixasm/refs/vanilla_funcs.txt` (nearest entry is `mnPlayersVSInitSlot` at 0x8013AFC0), so the exact host function and site are UNVERIFIED.
3. Speed.asm hook (Remix shared file, not a vanilla patch): `jal Item.Stopwatch.check_function_` at Speed.asm line 26, reading `watch_speed_flag` at lines 65, 104, 167 and 206. C contract:
   - call `itRemixStopwatchCheckFunction(gobj, update_proc)` before each fighter or projectile update;
   - `ITREMIX_STOPWATCH_CHECK_SKIP` (1): skip the update; `ITREMIX_STOPWATCH_CHECK_RUN_TWICE` (2): run the update twice; 0: run once;
   - read game speed with `itRemixStopwatchGetWatchSpeed()`.
4. Item.asm line 1574 (`jal Stopwatch.clear_active_stopwatch_`): call `itRemixStopwatchClearActive();` at the same place in the scene-end path.

## REMIX-TODO markers (in code)

- `REMIX-TODO(Toggles)`: `Toggles.entry_stopwatch_behaviour` (asm lines 74-82, 224-231) has no port. DEFAULT is always used, so the backfire roll is 10% and the speed-up roll is 20%.
- `REMIX-TODO(GFX)`: the `GFXRoutine.port_override` table entries and the `STOPWATCH_SLOWED` / `STOPWATCH_FASTED` gfx routines (asm 318-325, 360-367). Restoring the default gfx on expire (asm 477-489, `0x800E98D4` = `ftParamResetStatUpdateColAnim`) is also skipped.
- `REMIX-TODO(Render)`: the backfire BG rectangle is never drawn, coloured, faded, made transparent or freed (asm 97-128, 570-611, 649-652, 1083-1084).

## Uncertainties (asm line numbers)

- Lines 74-82, the pickup roll: delay-slot effects were traced. Result: DEFAULT gives `roll < 10`; BACKFIRE ALWAYS (3) gives backfire; BACKFIRE NEVER (4), SLOW ALWAYS (1) and FAST ALWAYS (2) give no backfire. The inline comments on lines 78-82 ("a0 = 1", "a0 = 2") disagree with the register values; the C follows the traced behaviour.
- Lines 900-901 and 913-916, the `0x18C` byte: compared as a whole byte in the asm. The C rebuilds it from the bitfields `is_attack_active` through `reflect_lr` (`itRemixStopwatchHitboxFlagsByte`). That is exact only if no other bit in that byte changes.
- Lines 636-644, the destroy path: the asm clears 8 words starting at `watch_routine_`, so the last four bytes (remaining time and speed type of player 3) are not cleared. The C clears all four entries. The change is harmless because destroy only runs when every remaining time is zero.
- Lines 764 and 847: the projectile speed-limit flag is written at halfword 2 (the "fast" slot) and read at halfword 0 (the "slow" slot). Slot 0 is never written. The C keeps both slots as in the asm.
- Lines 172 and 419-420: the handler stores the owner (0x40) and item (0x44). Only commented-out code reads them, so the C does not store them.
- Line 966 comment names `ftMainUpdateMotionEventsDefaultEffect` for RAM 0x800E240C, but the call target 0x800EB528 is `ftParamsUpdateFighterPartsTransformAll` in vanilla_funcs.txt. The notes use the call target.
- Line 167, `Render.register_routine`: assumed to be a GObj with a per-frame run, created by `gcMakeGObjSPAfter` with `nGCCommonKindEffect` and `nGCCommonLinkIDEffect` (the pattern in `itbox.c`). Destroy uses `gcEjectGObj`, which is assumed to be valid when called from the object's own run function (same as `itMainDestroyItem` usage; not verified).
- Lines 751-773 (projectile path) and lines 790-808 (player path): the Ness, Jeff, Lucas PK Thunder and Crash exemptions apply to player procs only. Projectiles skip them, as the branch at line 794 does; the C follows that.
- Action ids used (Revive1/2/ReviveWait 0x7 to 0x9, PK Thunder 0xE4/0xE5/0xE8/0xE9, Crash DigginItBegin 0xE5) come from `Action.asm` lines 12-14 and 2020-2025 and the asm comments.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Hooks applied 2026-10-14 (this session)

- Vanilla patch 1 (physics_map hitlag x3): applied in ftMainProcPhysicsMap — the three 'fp->hitlag_tics == 0' gates now read '(itRemixStopwatchIsFunctionAbbr() ? 1 : fp->hitlag_tics) == 0' under #ifdef REGION_REMIX (#else arm byte-identical vanilla).
- Speed.asm process_update_: applied in sys/objman.c gcRunGObjProcess nGCProcessKindFunc case — fighter/weapon GObjs (gobj->id nGCCommonKindFighter/nGCCommonKindWeapon) consult itRemixStopwatchCheckFunction first: SKIP skips the run, RUN_TWICE runs exec.func a second time (sGCRunStatus guard). Fighter double-runs happen inside CheckFunction (itRemixStopwatchRunTwice), matching the asm's _run_fast.
- Vanilla patch 2 (platform_despawn_shadow_crash_fix): still NOT applied (host function unverified in vanilla_funcs.txt).

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
