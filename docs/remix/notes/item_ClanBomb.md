# Item 0x45 ClanBomb notes

Port: `decomp/src/it/itremix/itremixclanbomb.c`, header `decomp/src/it/itremix/itremixclanbomb.h`.
Descriptor: `dITRemixClanBombItemDesc` (`ITRemixItemDesc`, kind 0x45).
Source: `.toolchain/pkgs/smashremix-src/src/items/ClanBomb.asm` (lines 1-487).
Item file: `Character.MARINA_file_8_ptr`, which is `fp->data->p_file_special3`, set on the copied
`ITDesc` before every `itManagerMakeItem` call. Attributes offset is `FILE_OFFSET` 0x130.

Spawn signature for fighter code: `GObj* itRemixClanBombMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.
The owner fighter is `parent_gobj`. The fighter call site is wired by another worker.

## Routines ported (asm line, C routine)

| Asm routine | Line | C routine |
|---|---|---|
| `clanbomb_stage_setting_` (spawn) | 83 | `itRemixClanBombMakeItem` (exported) |
| `prepickup_` (hold) | 218 | `itRemixClanBombPrepickupProc` |
| `hold_clanbomb_` | 179 | `itRemixClanBombHoldProcUpdate` |
| `throw_initial_` (thrown) | 237 | `itRemixClanBombThrownProc` |
| `throw_duration_` | 253 | `itRemixClanBombThrownDurationProcMap` |
| `collide_transition` | 271 | `itRemixClanBombCollideTransition` |
| `collide_with_player_` | 197 | `itRemixClanBombCollidePlayerProcHit` |
| `explode_transition_player_` | 288 | `itRemixClanBombExplodeTransitionPlayer` (static, not in any table) |
| `collide_with_clipping_` | 301 | `itRemixClanBombCollideClippingProcUpdate` |
| `explode_transition_` | 325 | `itRemixClanBombExplodeTransition` |
| `check_explode` | 345 | `itRemixClanBombCheckExplode` |
| `explode_initial_` | 393 | `itRemixClanBombExplodeInitial` |
| `explode_` | 409 | `itRemixClanBombExplodeProcUpdate` |
| `animate_clanbomb_` | 450 | `itRemixClanBombAnimate` (+ helpers `itRemixClanBombGetMObj`, `itRemixClanBombNextPalette`) |
| `pickup_` (consume) | 440 | not ported, see descriptor note |

Routines: 15 of 15 in the asm, with 1 of them (`pickup_`) reduced to `proc_consume = NULL`.
Helper routines are static and not exported. Nothing in this file needs a worklist table.

## Descriptor and tables

- `ITDesc` item_info_array (asm 27-38): kind 0x45, o_attributes 0x130, transform types
  `{nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}` (= word 0x1B000000, matrix kind 27),
  attack state `nGMAttackStateOff` (= 0), proc_update `itTomatoFallProcUpdate` (0x801744C0),
  proc_map `itTomatoFallProcMap` (0x80174524), all other procs NULL.
- `ITStatusDesc` item_state_table (asm 42-79), 8 procs per state, state 0 to 5. The asm comments
  give wrong offsets (0x30, 0x3C), but each state is 8 words, so the layout is by state.
- Descriptor: `proc_make` = spawn, `is_show_spawn_gfx` = TRUE, `proc_hold` = `prepickup_`,
  `proc_consume` = NULL, `proc_dropped` = `itTomatoDroppedSetStatus` (0x801745FC, Maxim Tomato drop),
  `proc_thrown` = `throw_initial_`, `proc_touch` = NULL (PLAYER_COLLISION(0)).

## Why pickup_ is NULL (proc_consume)

`pickup_` only saves a2/a3 to the caller's stack and jumps to 0x80145C4C. That address is the
common tail after the item switch in `ftCommonLightGetProcDamage` (`decomp/src/ft/ftcommon/ftcommonget.c`).
After the switch in the C code is the function end, and the Remix default branch `break`s. So the
dispatch already does everything `pickup_` does, and no code runs in the consume proc. Setting it
to NULL is the same behaviour as an empty consume proc.

## Vanilla patches

None. ClanBomb.asm has no `OS.patch_start` and no `j` or `jal` hooks into vanilla code. It calls vanilla
functions only. No C needs inserting into vanilla files.

Vanilla routines called by name (all in decomp, already declared):
- `itManagerMakeItem` (0x8016E174), `itMainClearOwnerStats` (0x8017279C, void in decomp; the asm
  reuses the DObj in v0 as the value of `gcAddXObjForDObjFixed`'s first argument);
- `gcAddXObjForDObjFixed` (0x80008CC0, `decomp/src/sys/objman.c:1197`), with `dobj, 0x2E, 0`;
- `ifCommonItemArrowMakeInterface` (0x80111EC0);
- `itBombHeiCommonSetHitStatusNone` (0x80177218), `itBombHeiCommonSetHitStatusNormal` (0x80177208);
- `itBombHeiCommonCheckMakeDustEffect` (0x80177180, override 0), `itBombHeiExplodeInitVars` (0x80177BAC);
- `itBombHeiCommonUpdateAttackEvent` (0x80177A24), `itBombHeiThrownProcUpdate` (0x80177530);
- `itMainCommonProcHop` (0x801733E4), `itMainCommonProcReflector` (0x80173434);
- `itMapCheckMapProcAll` (0x80173E58), `itMainRefreshAttackColl` (0x8017275C), `itMainSetStatus` (0x80172EC8);
- `efManagerSparkleWhiteMultiExplodeMakeEffect` (0x801005C8), `efManagerQuakeMakeEffect` (0x801008F4);
- `itTomatoWaitProcMap` (0x801744FC), `itTomatoFallProcUpdate`, `itTomatoFallProcMap`, `itTomatoDroppedSetStatus`;
- `func_800269C0_275C0` (FGM player; asm `FGM.play(0x0001)`).

## REMIX-TODO markers

None. The FGM call goes through `func_800269C0_275C0`, the same function `FGM.play_` wraps (see
`decomp/src/it/itcommon/itbombhei.c:6`), and the FGM ID is 0x0001. Nothing else is skipped.

## Uncertainties and deviations (asm line numbers)

1. **Arrow pointer offset (asm 164-168).** The asm stores `ifCommonItemArrowMakeInterface(ip)` at
   ITStruct 0x0348. The decomp field `arrow_gobj` is at 0x0344 (`struct_offsets.txt`), and 0x0348 is
   `arrow_timer`. The port uses `ip->arrow_gobj`, following the meaning (every other item stores this
   result there, e.g. `itmball.c:516`). Verify on the real build that 0x348 is not a Remix layout change.
2. **Flag byte 0x02D3 (asm 161-163).** The asm ORs 0x04 into byte 0x02D3, labelled "original code here",
   which is the Bob-omb spawn code. The decomp Bob-omb spawn sets `ip->is_unused_item_bool = TRUE`
   (`itbombhei.c`), and the byte falls inside the `attr` pointer field in the decomp layout. The port uses
   `is_unused_item_bool` by meaning.
3. **Explode scale (asm 361-368, literal at RAM 0x8019CD30).** The float constant was not decoded from
   the ROM. The port uses 1.4F, the same value as `ITBOMBHEI_EXPLODE_SCALE` in the vanilla Bob-omb explode
   (`decomp/src/it/itcommon/itbombhei.c`, `itBombHeiCommonSetExplode`). Confirm the literal at 0x8019CD30 is 1.4F.
4. **Spawn returns on failure (asm 97, 173-175).** When `itManagerMakeItem` returns NULL, the asm
   returns the incoming a3 value, not NULL. The port returns NULL, since a boolean or flags value is not
   a valid GObj pointer. Callers that check for NULL then behave correctly.
5. **Translate save and restore (asm 104-112, 124-133).** The asm copies the DObj translate vector around
   `gcAddXObjForDObjFixed` and restores it. The port does the same. The vanilla bomb under REGION_US does
   this too.
6. **`frame_draw_last` as the animation timer (asm 464-466, 473-474).** The asm reads GObj byte 0x0E,
   labelled "async timer". The decomp names this byte `frame_draw_last` (`objtypes.h`, used by
   `objdisplay.c`). The port reads the same byte, with mask 0x0F and bit 0x20, as the asm does. The palette
   cycle depends on this byte, so verify it on the real build.
7. **Palette write (asm 469-480).** The asm writes only the high halfword of the float palette (`sh`).
   3.0F, 4.0F and 2.0F have a zero low halfword, so the port writes whole floats. The asm's first
   `lli at, 0x4080` is overwritten by `lli at, 0x4000` before the branch, but the delay slot still runs,
   so the 0x4080 result is reached only through the taken branch. The port keeps that behaviour.
8. **Null MObj (asm 454-461).** When the DObj and its child have no MObj, the asm dereferences NULL. The
   port returns without changing anything.
9. **Dead loads.** The spawn's `lw t7` of the owner's FTStruct and `explode_transition_`'s `a2` loads are
   unused by the asm and are not ported.
10. **Unused parameter dropped.** `explode_transition_` passes `a1 & 0xFF` into `check_explode`, which
    stores it and never reads it. The port drops the parameter.
11. **Attack events come from the Bob-omb data.** `explode_` calls `itBombHeiCommonUpdateAttackEvent`,
    which reads the Bob-omb's `llITCommonDataBombHeiAttackEvents` through `dITBombHeiItemDesc`. The asm
    does the same, so the port keeps it.
12. **`explode_transition_player_`** is not referenced by any state table in the asm. It is ported as a
    static routine and is not in any table.
