# VS character select crashed on its fourth frame after the bonus bookend was added (2026-11-04)

## Symptom

Every run that entered the VS character select (scene 16) died on frame 3 or 4 with SIGBUS or SIGSEGV inside
`gcRunGObj`, at a program counter like `0x01000000ece40e00`. No screenshot was taken. The crash came with the bonus
bookend of the Remix CSS (`mnRemixCssBonusMakeBookend`, file 0xA06 offset 0x3B28).

## Root cause

The crashing GObj was the scene's main GObj (`mnPlayersVSFuncRun`, link 15). Its `func_run`, id and link fields were
garbage. A watchpoint on `func_run` stopped in `portFixupSpriteBitmapData`, called from `lbCommonMakeSObjForGObj` for
the bookend sprite.

The bookend bitmap in file 0xA06 has `width` 16, `width_img` 96 and `actualHeight` 96. Its texels are 16x96 RGBA16
(0xC00 bytes, from 0x2F18 to 0x3B18). The port fixup sized the texels as `width_img * actualHeight`, which is 0x4800
bytes. It byte-swapped and de-swizzled that range in place. The file is 0x6090 bytes long, and the scene heap that follows
it holds the main GObj, so the in-place swap overwrote the GObj.

On N64 the same data is harmless. `lbCommonDrawSObjBitmap` loads it with one `LoadBlock` sized by `width_img`, which only
reads extra RAM. It samples through a render tile whose line is `width` rounded up to 8 bytes
(`G_IM_SIZ_*_LINE_BYTES`), so that line is the real row stride. Vanilla sprites have `width_img` equal to that line,
which is why the port's use of `width_img` never showed a problem before.

## Fix

- `portSpriteBitmapRowTexels` (`port/bridge/sprite_bitmap_row.c`): the row stride of a sprite bitmap is `width_img`,
  or the render tile line when `width_img` is wider. `portFixupSpriteBitmapData` uses it for the byte swap size and for
  the odd-row de-swizzle.
- `portClampToContainingFile` (`port/bridge/lbreloc_byteswap.cpp`): both in-place passes stop at the end of the reloc
  file that holds the texels.

## Verification

- `tests/remix/test_sprite_bitmap_row.c`: vanilla strides are unchanged, and the bookend case (16, 96, RGBA16) gives 16.
- All nine VS CSS runs of the batch (stage select, roulette, tournament, Menu Music) went from rc 138/139 with no
  screenshot to rc 0. The bookend draws as the thin fire strip left of the grid, and its scroll (Dark Samus, Lucas,
  Peppy) works (run `b4_css_bonus`).

## Lesson

A fixup that rewrites texture data in place must size it the way the hardware samples it, not from a header field that
only sizes a load. It must also never write past the file that owns the data. Smash Remix extension files carry headers
that vanilla never exercised.
