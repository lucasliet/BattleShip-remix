# F14: Cheats.asm and MagnifyingGlass.asm

Source: `.toolchain/pkgs/smashremix-src/src/Cheats.asm` (included by `main.asm:53`) and
`.toolchain/pkgs/smashremix-src/src/MagnifyingGlass.asm` (included by `main.asm:117`).

## MagnifyingGlass.asm: ported

Files: `decomp/src/gm/gmremixmagnify.{c,h}` (under `#if defined(PORT) && defined(REGION_REMIX)`).

Public function: `Mtx *gmRemixMagnifyApplyCustomZoom(FTStruct *fp, Mtx *real_mtx)`.

Lead wiring (not yet applied, vanilla file not edited):

- File: `decomp/src/if/ifcommon.c`, function `ifCommonPlayerMagnifyUpdateViewport` (line 1633), RAM 0x80110DD4.
- Line 1708 currently: `gSPMatrix(dls[0]++, &CObjGetStruct(gGCCurrentCamera)->xobjs[0]->mtx, G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_PROJECTION);`
- Replace the matrix argument with `gmRemixMagnifyApplyCustomZoom(fp, &CObjGetStruct(gGCCurrentCamera)->xobjs[0]->mtx)`
  under `#ifdef REGION_REMIX`, and add `#include "gm/gmremixmagnify.h"`.
- Asm mapping: patch at ROM 0x8C680 / RAM 0x80110E80 replaces `sw t9, 0x4(v0)` (gSPMatrix w1) with `jal apply_custom_zoom_`.
  Its delay slot `lbu t6, 1(t2)` is the original `magnify_mode` read (line 1710), so no extra C is needed.
- Argument mapping: a0 = dls, a1 = fp (`lw t7, 0x8(a1)` = fkind, `lbu t7, 0xD(a1)` = player).
- The asm stores t9 (the matrix pointer returned by the function) back to the gSPMatrix word, which is the wiring above.

Remix data: `Character.magnifying_glass_zoom.table` at RAM 0x8049E880, one u16 per fighter kind, 97 entries
(NUM_CHARACTERS = 27 + 2 + 68, `Character.asm:15-27`).

Missing REMIX_SYM: `REMIX_SYM_Character__magnifying_glass_zoom__table` = `0x8049E880u`. Not in
`port/remix/remix_symbols.h`. The file defines it under `#ifndef` as a stop-gap. Add it to the generator (`tools/gen_remix_actions.py`
or the symbol source for `remix_symbols.h`), then drop the guard.

Unverified:

- The asm writes the 16-bit fraction at N64 Mtx byte offsets 0x20 and 0x2A (elements [0][0] and [1][1]). The port's Mtx is
  `int m[4][4]` (`decomp/include/PR/gbi.h:1188`). `gmRemixMagnifySetFrac` writes big-endian bytes into it, which is literal
  but unverified on a little-endian host. Check in game (magnified fighter with a non-zero table entry) and fix the helper if the zoom does not appear.
- Copy size is one Mtx (64 bytes), matching the asm's 0x40 loop.
- Deviation: an out-of-range fkind or player returns the real matrix (the asm would read past the table). No effect for valid data.

## Cheats.asm: no new file

Every patch is a single instruction or data word in a vanilla function. Nothing is a callable Remix routine, so a new
`gmremixcheats.{c,h}` would be empty. Each patch belongs as an inline `#ifdef REGION_REMIX` edit in the owning vanilla file.

| Asm patch (ROM / RAM) | Asm change | Owner (vanilla_funcs.txt) | Port state |
|---|---|---|---|
| 0x42B3A / 0x800A3DEA | `dw 0x007F0C90` (unlock everything) | unmapped: nearest listed start is `scManagerRunPrintGObjStatus` 0x800A3040, not a verified owner | Already exists in the port as `port_cheat_unlock_all` (`port/enhancements/UnlockCheats.cpp`, `decomp/src/lb/lbbackup.c`). Not ported as a Remix patch. |
| 0x337F8 / 0x80032BF8 | `ori t2, r0, 0x0212` replaces `lw t2, 4(t1)` (disables anti-aliasing in the f3dex2 DL builder) | unmapped: `func_80035840` region, no listed owner | Not ported; graphics setting, needs a decision by the lead. |
| 0x8B0D4 / 0x8010F8D4 | `slti at, s1, 0x7` becomes `slti at, s1, 0x1` (stock always shown as a number) | `ifCommonPlayerStockMultiProcDisplay` (0x8010F878, `decomp/src/if/ifcommon.c:1062`) | Not ported. The vanilla threshold is `if (stock_count <= 6)` at `ifcommon.c:1131`. Confirm that `s1` is `stock_count` before changing it. |
| 0x81408 / 0x80105C08 | `lb a0, 0x2A(v1)` replaced by `nop` | `grPupupuWhispyUpdateWait` (0x80105BE8, `decomp/src/gr/grcommon/grpupupu.c:282`) | Not ported. The asm comment says "I don't remember how this works". Verify before porting. |
| 0x216FC / 0x80020AFC | `sw a1, 0(t3)` replaced by `nop` (BGM id store) | `syAudioPlayBGM` (0x80020AB4, `decomp/src/sys/audio.c:1654`) | Not ported. The audible effect is covered by the `play_music` Remix toggle already in `syAudioPlayBGM`. Whether the store is skipped in the Remix ROM is unverified. |

Unverified in Cheats.asm: whether the 0x800A3DEA data patch and the BGM store nop are active at runtime in the Remix ROM.
The disassembly at that ROM offset decodes as zeros, so the owning code path is unknown.
