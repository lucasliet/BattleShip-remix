# F14 credits: REMIX-TODO left open (scstaffroll.c)

**Update 2026-11-04 (M11 Fase 4):** both items are closed. The glyph textures are in the vanilla credits file 0xC3
(`SCStaffroll`, with a 0x58 byte header before the asm offsets), not in `File.REMIX_CREDITS`, so the blocker below was a
wrong file; glyphs are ported. The speed assist is n/a (needs the "afk" player tag). See `docs/remix/menus.md`,
"Remix screens". The text below is the original analysis.

Status: both markers stay `REMIX-TODO` because neither has a verified port path yet. Nothing was changed in the behaviour of the staff roll.

## credits-glyphs (scstaffroll.c:20)

Source: `.toolchain/pkgs/smashremix-src/src/Credits.asm:260-288` (`add_new_character`), glyph ids `0x50 + n`:

| glyph | id | width x height | offset |
|---|---|---|---|
| 0 | 0x50 | 0x10 x 0x16 | 0x7958 |
| 1 | 0x51 | 0x0C x 0x16 | 0x7A18 |
| 2 | 0x52 | 0x10 x 0x16 | 0x7AE0 |
| 3 | 0x53 | 0x10 x 0x16 | 0x7BA8 |
| _ | 0x54 | 0x12 x 0x18 | 0x7C70 |
| 5 | 0x55 | 0x10 x 0x16 | 0x7E10 |
| 6 | 0x56 | 0x10 x 0x16 | 0x7ED8 |
| 7 | 0x57 | 0x10 x 0x16 | 0x7FA0 |
| 8 | 0x58 | 0x10 x 0x16 | 0x8068 |
| 9 | 0x59 | 0x10 x 0x16 | 0x8130 |
| ! | 0x5A | 0x08 x 0x16 | 0x81F8 |
| - | 0x5B | 0x0A x 0x16 | 0x82C0 |
| / | 0x5C | 0x10 x 0x16 | 0x8388 |
| * | 0x5D | 0x0B x 0x16 | 0x8470 |
| ( | 0x5E | 0x0C x 0x1F | 0x8538 |
| ) | 0x5F | 0x0C x 0x1F | 0x8640 |
| @ | 0x60 | 0x14 x 0x16 | 0x8748 |
| ~ | 0x61 | 0x0F x 0x16 | 0x88B8 |

Blocker found: the file is `File.REMIX_CREDITS` = 0xDF1 (`File.asm:1452`), which is `reloc_misc/file_3569` (43480 bytes) in `build/BattleShip-Remix.o2r`. At the offsets above, that file holds encoded name words (`ffffffdf 00000011 00000028 0000001a` at 0x7958, `0000002d 00000028 0000002b` at 0x7A18), not texture bytes. The asm offsets therefore do not match this file, or the textures live elsewhere. Resolve this first (check the offset base in the Remix build, or find the texture bytes in `remix.z64`).

Other blockers, outside this worker's scope:
- `tools/gen_remix_credits.py` `encode_scroll_char` maps digits and `_ ! - / * ( ) @ ~` to whitespace. The glyphs never appear until the encoder emits 0x50-0x61 (for example '0' + 0x20 for digits, and the table above for symbols). Names such as `CMM1215`, `Jay6T4`, `M-1`, `Likiji123` are affected.
- The port has no table for ids 0x50-0x61: `sSCStaffrollNameAndJobDisplayLists` and `dSCStaffrollNameAndJobSpriteInfo` stop at 55 or 56 entries, and the file list `dSCStaffrollFileIDs` loads only 0xC3.

## credits-speed-assist (scstaffroll.c:1475)

Source: `Credits.asm:1310-1349`, hook at RAM 0x801335C4 (`OS.patch_start(0x180CC4, 0x801335C4)`). The logic:
1. Tag check: `CharacterSelectDebugMenu.PlayerTag.string_table + 8`, dereferenced twice, must equal `0x61666B00` ("afk\0").
2. `sSCStaffrollNameID` (RAM 0x8013A8B8) must equal 0x55, the first Remix name.
3. `sSCStaffrollRollSpeed` (0x8013A8BC) must differ from the normal speed (0x8013A7B0). If so, Start is simulated (`t3 = TRUE`), so the vanilla Start branch toggles the speed back to normal.

Why it is open: the 2nd tag is a debug-menu string, set at runtime when the player types a tag. The port does not keep custom tag names (`src/if/ifremixplayertag.c:16`). In the Remix ROM image the default is empty: `remix.z64` at RAM 0x8052635C points to 0x804693E8, whose first word is 0. The port therefore never sees "afk". Logic, comparison and the Start simulation can be ported once a tag source exists. The source is a custom tag subsystem, not this file.

## Missing symbols

None missing from `port/remix/remix_symbols.h` for these two items. `Credits.remix_credits_speed_assist_` is 0x8048AE20 in `build/remixasm/remix.sym`. `REMIX_CREDITS` (0xDF1) is an asset file id, not a symbol.

## Lead must

- Confirm the offset base for the Remix credits textures (see the blocker above) before any glyph work.
- Extend `tools/gen_remix_credits.py` if the glyphs are wanted, then port the glyph tables.
- Decide whether custom player tags (the "afk" string) belong to a ROADMAP task.
