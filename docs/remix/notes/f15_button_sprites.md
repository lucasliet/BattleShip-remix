# F15: button and extension sprites of the Remix HUDs

Research for the T8.3 HUDs (M11 Fase R, R2). Redone by a Haiku subagent on 2026-11-03 after the October run was cut while
decoding the A button out of the `.o2r`. Unblocks: the A button of Match Stats, the button glyphs of the pause and Training
legends, the CSS arrows and D-pad, the Multi-Man polygon icon and the KOTH frame (infra I2 of M11 Fase 1).

## Files (all present in the user's `.o2r`)

| File id | Path in the `.o2r` | Note |
|---|---|---|
| 0xC5 | `reloc_interface/IFCommonBattlePause` | Remix does **not** override the id (`RelocFileTable.remix.cpp:210`); vanilla pause file |
| 0xA06 | `reloc_misc/file_2566` | Remix extension file |
| 0x19 | `reloc_fighters_common/FTStocksZako` | vanilla |
| 0x14F3 | `reloc_misc/file_5363` | Remix extension file |

Archive member layout (inferred from the bytes, not yet checked against `RelocFileFactory.cpp`): data starts at
`member[80:]`, size is the u32 LE at `member[76]` (`len - 80`).

## Sprites

Fields are big-endian; the Bitmap sits at `sprite - 0x10`; the row stride is `width_img`, not `width`.

| Sprite | File @ offset | Size | Format | Texel |
|---|---|---|---|---|
| A | 0xC5 @0x958 | 13x13 | IA8 (stride 16) | 0x878 |
| B | 0xC5 @0xA88 | 13x13 | IA8 (stride 16) | 0x9A8 |
| L | 0xC5 @0x18C8 | 16x12 | IA8 | to read |
| R | 0xC5 @0xCF8 | 16x12 | IA8 | to read |
| Z | 0xC5 @0xBD8 | 9x15 | IA8 | to read |
| PLUS | 0xC5 @0x4D8 | 8x8 | I4 | to read |
| L_THIN | 0xC5 @0x1B10 | 16x12 | IA8 | to read |
| "Legend" | 0xC5 @0x19F8 | 42x9 | I4 | to read |
| D-pad | 0xA06 @0x218 | 16x16 | RGBA16 | 0x8 |
| Arrows | 0xA06 @0x688 / 0x1958 / 0x1A88 / 0x1BB8 | 13x13 | IA8 | 0x5A8 / 0x1878 / 0x19A8 / 0x1AD8 |
| Polygon stock icon | 0x19 @0x80 | 8x8 | CI4, LUT at 0x50 (unconfirmed) | 0x8 |
| KOTH frame | 0x14F3 @0xAA8 | 64x16 | IA8 | 0x698 |

## Colour

- `TEXTURE_INIT_` (0x800CCFDC) is `lbCommonMakeSObjForGObj`; `TEXTURE_RENDER_` (0x800CCF00) is `lbCommonDrawSObjAttr`. Remix
  draws these through the vanilla SObj path.
- The colour argument becomes PRIM (`sobj->sprite.red..alpha`); the palette argument becomes ENV (`sobj->envcolor`).
  Inferred: confirm the +0x28 and +0x60 offsets against `struct SObj`.
- IA combiner (`lbcommon.c:2907-2909`): `RGB = ENV + (PRIM - ENV) * I`, `A = A_tex * PRIM.a`. In IA8, I and A are 4-bit
  nibbles expanded x17 (the subagent report divided them by 255 without expanding).
- Still to check: the I4 (PLUS, "Legend") and CI4 combiners in `lbCommonPrepSObjAttr`.
- Example: the Match Stats A button uses tint 0x50A8FFFF, palette 0x0010FFFF.

## Decision: native SObj, no atlas

- Pattern to copy: `decomp/src/if/ifremixcombometer.c`: `lbRelocGetExternHeapFile` (l. 435), `lbRelocGetFileData`
  (l. 281-282), `lbCommonMakeSObjForGObj` (already does the fixup and the TMEM unswizzle),
  `gcAddGObjDisplay(..., lbCommonDrawSObjAttr, link 0x18, ...)`. Colour through `sobj->sprite.red..` and `sobj->envcolor`,
  hidden with `SP_HIDDEN`.
- Why: the asm coordinates are already SObj coordinates; the four formats, the LUT and the swizzle are already handled; the
  colour comes out the same as in Remix.
- Cost: today's callers draw every frame (`remix_hud_text_put`). With sprites each HUD needs a live GObj with a lifecycle
  (create, destroy or hide), e.g. the pause legend.

## I2 (M11 Fase 1)

- Small module (e.g. `ifremixsprite.c`): load the extension file, make an SObj with tint and palette, hide and show.
- Unit test of the offset and format table; the Match Stats A button is the first on-screen consumer.
- Before implementing: file 0xC5 is probably already loaded by the vanilla pause, so reuse it instead of a second copy
  (check `ifcommon.c`). Kabel text stays in the ImGui overlay on top of the game while the sprites go to the game DL, so
  check that no Kabel text has to sit under a sprite.
