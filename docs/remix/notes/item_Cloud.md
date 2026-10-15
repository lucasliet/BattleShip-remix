# Item.Cloud (0x49) port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Cloud.asm`
Port: `decomp/src/it/itremix/itremixcloud.c`, `decomp/src/it/itremix/itremixcloud.h`
Descriptor: `const ITRemixItemDesc dITRemixCloudItemDesc` (kind 0x49)
Spawn signature: `GObj* itRemixCloudMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`. Goemon's USP call site must pass the parent (Goemon GObj), the position and the velocity. The `flags` argument is not forwarded (see uncertainty 1).
Item file: Goemon file 7, `fp->data->p_file_special2`. The item footer offset is 0x40, which goes into `ITDesc.o_attributes`.

## Routines

| Asm | C | Status |
|---|---|---|
| `stage_setting_` (L49-113) | `itRemixCloudMakeItem` + `itRemixCloudSetupItem` | ported |
| `cloud_null_main_` (L117-230) | `itRemixCloudNullProcUpdate` + helpers | ported |
| `cloud_aerial_main_` (L235-278) | `itRemixCloudAerialProcUpdate` | ported |
| `cloud_blast_zone_` (L282-285) | `itRemixCloudBlastZoneProc` (no-op) | ported |
| `enable_flash` patch (L287-315) | not applied, see Vanilla patches | recorded |

Item state is stored in `ITStruct.item_vars` through `ITRemixCloudVars`, with a `_Static_assert` that it fits.

## Vanilla patches

### enable_flash (Cloud.asm L287-315)

- RAM 0x8016F5B4 (ROM 0xE9FF4) is inside the item main processing code. `build/remixasm/refs/vanilla_funcs.txt` puts `itProcessProcItemMain` at 0x8016F534 (`decomp/src/it/itprocess.c:93`). The exact line of the patch has not been confirmed.
- The patch skips the `item_gobj->flags` clear for Cloud (0x49) and the Bowser bomb (0x11A). The patch returns to the original code at 0x8016F64C.
- Not applied. Suggested C, to be confirmed against the disassembly of 0x8016F5B4 before use:

```c
#ifdef REGION_REMIX
    else if (!itRemixKeepsFlashFlags(ip->kind)) item_gobj->flags = GOBJ_FLAG_NONE;
#else
    else item_gobj->flags = GOBJ_FLAG_NONE;
#endif
```

`itRemixKeepsFlashFlags` should return TRUE for 0x49 and 0x11A. It is not written.

## Shared subsystems (REMIX-TODO)

- `REMIX-TODO(BlastWall)`: the asm stores `cloud_blast_zone_` at item special slot 0x398 (`blast_zone_proc` here). Blast-wall destruction code calls it. That caller is not ported, so the proc is never called.
- `REMIX-TODO(transform_types)`: `item_info_array` 0x0C is `0x1B000000` or `0x1C000000`, "possible argument". Its meaning is unknown. The port uses `{ nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0 }` as a placeholder, copied from Tomato.
- `FGM.play`: the port calls `func_800269C0_275C0` directly. This is the same pattern as `ftremixgoemonusp.c`.

## Uncertainties (asm line numbers)

1. L57: `itManagerMakeItem` gets 0 as its fifth argument. The port passes 0 and ignores the dispatcher's `flags`.
2. L74-75: the asm calls `0x80008CC0`, which is `gcAddXObjForDObjFixed(joint, 0x2E, 0)` in `decomp/src/sys/objman.c:1197`. This matches `ittomato.c`.
3. L50-54 and L64-67: the asm loads item struct fields that are not used later (`t0`, `a0` reloads). They are dropped.
4. L100-103: the direction read sets `t1` and `a1`, and nothing uses either. It is dropped.
5. L85-88: the asm copies the player's attack hitbox byte (player 0x12) to item 0x12, which has no named ITStruct field. It is dropped.
6. L194-195: `lw a0, 0x0074(s0)` reads `s0`, which `cloud_null_main_` never sets. The intent, from the comment, is the item's DObj z rotation. The port zeroes the item's own DObj rotate.z. Check this one first.
7. L186-190: the `bnez t0` branch before `_continue` has the same effect on both paths, so the port has no branch.
8. L149-150: the default death timer is 1. Jump and escape use `TOTAL_TIMER` (30).
9. L261 and `Cloud.asm` L11: `DESPAWN_FGM` is `FGM.CLOUD_FADE`, which is 0x113 in `FGM.asm` L1927. The "was 0x501" comment is stale. The port uses 0x113.
10. L205-215: the owner frame timer is the N64 0x0B18 field, read as `fp->status_vars.common.dead.wait`. That is the same field `GOEMON_USP_FRAME_TIMER` names in `decomp/src/ft/ftremix/goemon/ftremixgoemonusp.c`.
11. L51: `item_gobj->flags` is written with a whole-word store (0 or the flash bit), as the asm does. This is the same field vanilla uses as a bitfield (`GOBJ_FLAG_*`). Every write here replaces the whole word.
12. Remix item ITStruct slots 0x1C4, 0x1CC, 0x1D0 and 0x1D4 have no reader in this item. They are kept in `ITRemixCloudVars` for any reader outside it.
13. The vanilla ITStruct has no named field at N64 0x2C0 or 0x33E. Those slots are mapped by meaning to `life_timer` and `death_timer`.
14. The port sets the descriptor's `proc_update` to the null-state update. The asm stores it in the state table only, and the port assumes the Remix dispatcher starts the item in state 0.

## Step 2 wave 2026-10-14 (partial — kimi quota window)

- LANDED (W3): enable_flash — itRemixKeepsFlashFlags(kind) helper added (cloud.c defines, waddledoo.c consumes). NOTE: the vanilla-side consumption in itprocess.c (itProcessProcItemMain flash-flag preservation) was NOT applied (shared-file worker died) — helper currently inert; wire it in the follow-up wave.
