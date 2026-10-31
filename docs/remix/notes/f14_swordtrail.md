# F14 SwordTrail (SwordTrail.asm)

Source: `.toolchain/pkgs/smashremix-src/src/SwordTrail.asm` (304 lines). Port files (new, guarded by `PORT && REGION_REMIX`):
`decomp/src/ft/ftremix/ftremixswordtrail.{c,h}`. No existing file was edited. Syntax check: `syntax-check OK`.

## Model

- Trail id is `fp->afterimage.is_itemswing` (player struct 0x0A9C). Ids 0 and 1 are vanilla; custom ids are `2 + index` in
  the table (`dFTRemixSwordTrails`, 21 entries, asm order).
- Moveset command: the asm prints `0xCC<4*id>0000`. Decoded as the vanilla SetAfterImage bitfield (opcode 6 bits, is_itemswing 8 bits,
  drawstatus 18 bits, big-endian), `(word >> 18) & 0xFF` is the id. The vanilla motion event already sets is_itemswing, so no new
  command handler is needed. Assumption: opcode of `0xCC` equals `nFTMotionEventSetAfterImage`; not checked against the enum value.
- Per entry: `fkind` (Remix id, or `FTREMIX_SWORD_TRAIL_ANY_FKIND` = asm 0xFFFF), `model_part` (joint = model_part + 4,
  `nFTPartsJointCommonStart`), `axis` (row of the bone matrix used as direction), two RGBA colours, start and end distances.
- Colours: asm RGBA32 `0xRRGGBBAA` is stored as bytes r, g, b, a. The macro `FTREMIX_SWORD_TRAIL_RGBA` does the same split.

## Patches and hooks (lead wires these)

| RAM (ROM) | Decomp location | Hook |
|---|---|---|
| 0x800E6740 (0x61F40) initial_setup_ | `ftmain.c:4515`, `switch (fp->afterimage.is_itemswing)` in `ftMainProcParams` (header line 4262). Inferred: the patched code is the record block before `func_ovl2_800EDBA4(fp->joints[11])` (line 4526). | Add `default:` to the switch (below). |
| 0x800E6768 sword_trail_fix_ | `ftmain.c:4519` Link check, already done by `ftRemixLinkSharedHasSwordTrail` | No change |
| 0x800E67F4 axis_setup_ | `ftmain.c:4531-4533` vec loads | Covered by `ftRemixSwordTrailRecord` (row = axis) |
| 0x800F253C (0x6DD3C) second_check_ | `ftdisplaymain.c:996`, `switch` in `ftDisplayMainDrawAll` | Add `default:` (below) |
| 0x800F1060 (0x6C860) draw_trail_ | `ftdisplaymain.c:435`, `switch` in `ftDisplayMainDrawAfterImage` | Add `default:` (below) |

### 1. ftmain.c, in the `switch (fp->afterimage.is_itemswing)` at line 4515, after `case TRUE:`

```c
#ifdef REGION_REMIX
        default:
            ftRemixSwordTrailRecord(fp);
            break;
#endif
```
Add `#include "ft/ftremix/ftremixswordtrail.h"` to `ftmain.c`. The enclosing condition (`hitlag_tics == 0 && drawstatus != -1`) is unchanged.

### 2. ftdisplaymain.c, `ftDisplayMainDrawAll`, switch at line 996

```c
#ifdef REGION_REMIX
        default:
            if (ftRemixSwordTrailIsCustomId(fp->afterimage.is_itemswing))
            {
                ftDisplayMainDrawAfterImage(fp);
            }
            break;
#endif
```

### 3. ftdisplaymain.c, `ftDisplayMainDrawAfterImage`, switch at line 435

Declare `FTRemixSwordTrailDraw remix_trail_draw;` with the other locals, then add:

```c
#ifdef REGION_REMIX
    default:
        ftRemixSwordTrailGetDraw(fp->afterimage.is_itemswing, &remix_trail_draw);
        var_f20 = remix_trail_draw.start_dist;
        var_f22 = remix_trail_draw.end_dist;
        color1 = &remix_trail_draw.colour_1;
        color2 = &remix_trail_draw.colour_2;
        add_alpha = 0;
        base_alpha = 0xFF;
        rotate = F_CLC_DTOR32(30.0F);
        vtx_dl = dFTDisplayMainAfterImageVertexDL;
        tri_dl = dFTDisplayMainAfterImageTriangleDL;
        break;
#endif
```
Add `#include "ft/ftremix/ftremixswordtrail.h"` to `ftdisplaymain.c`. The asm writes the display lists 0x8012C490 and 0x8012C4B0 and base alpha 0xFF, add alpha 0. Those are the same values as the vanilla FALSE case. The asm does not set `rotate`; 30 degrees is an assumption.

## Public API (`ftremixswordtrail.h`)

- `sb32 ftRemixSwordTrailIsCustomId(s32 trail_id)`: id is in the table.
- `sb32 ftRemixSwordTrailGetDraw(s32 trail_id, FTRemixSwordTrailDraw *draw)`: fills distances and colours; FALSE for non-custom ids.
- `void ftRemixSwordTrailRecord(FTStruct *fp)`: character check (fkind or ANY), joint lookup, `func_ovl2_800EDBA4`, ring store, advance.

## Subsystems

- No port toggle exists for sword trails (grep of `port/` and `gm/` finds nothing). The asm has no toggle either; it is always on.
- The trail effect is the vanilla afterimage (ring of 3 `FTAfterImage` entries drawn as triangles). No new GFX or effect API is needed.
- Nothing else is missing. The only Link-only check left in the vanilla path is `ftRemixLinkSharedHasSwordTrail`, which stays as is.

## Uncertainties

1. Whether the asm skips the `modelpart_status[...] == 0` check for custom trails. The implementation skips it (the patch jumps past the Link check). Verify in the N64 disassembly at 0x800E6740-0x800E6780.
2. `rotate` in the custom draw path (asm does not write it; 30 degrees is assumed).
3. Axis row equivalence: `vec = mtx_translate[axis]`, inferred from the asm's `v1 + 0x50 + axis*0x10` offsets, with Z = row 2 as in vanilla.
4. Kirby trails use fkind ANY. Kirby's moveset must set the id itself; not checked.
5. Whether the `0xCC` opcode equals `nFTMotionEventSetAfterImage` (see Model).
