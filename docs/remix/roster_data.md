# Roster data generated from the Remix source

Covers `port/remix/remix_roster_data.h` (97 entries, ids 0x00..0x60, `kRemixRoster`), the generated table of per-fighter facts the port needs at
compile time. Generator: `tools/gen_remix_roster.py` (stdlib only). Consumers: `port/remix/remix_roster.cpp` (`docs/remix/roster_loader.md`) and the
`REMIX_ID_*` constants used across `decomp/src`.
Regenerate with:

    .toolchain/host/venv/bin/python -I tools/gen_remix_roster.py [path/to/smashremix-src]

Default source: `.toolchain/pkgs/smashremix-src`. It reads `src/Character.asm` (the `scope id` table, `variant_type` / `sound_type`, the
`define_character` calls), `src/costumes/*.asm` (`NUM_COSTUMES`, `NUM_EXTRA_COSTUMES`, `register_extra_costumes_for_char`) and the
`menu_zoom` patches (`table_patch_start(menu_zoom, ...)`).
Test: `tools/test_remix_menu_zoom.py` checks the `menu_zoom` column against the Remix ROM.

## Decisions

- `parent_remix_id` is the real parent of the `define_character` (ids 0x00..0x0B). Vanilla and 0x1C stay at 0xFF.
- `variant_type` uses the value that `define_character` writes into the table (`table_patch_start`), not the static table. That is why 0x1B comes out
  as SPECIAL (the static table says UNUSED).
- `has_define_character == false` marks slots without a `define_character` (vanilla and 0x1C). For those, `add_actions`, `file_*`,
  `attrib_offset`, `jab3` and `inhale_copy` are zero/NULL/false placeholders.
- Ids follow the macro order: RANDOM = 0x1B, 0x1C is the Mario clone (skipped), then 0x1D onward. Every `// 0xNN - NAME` comment is checked against
  the computed id (a mismatch aborts).
- The port fkind of a Remix id is the id itself: `REMIX_ID_TO_PORT_FKIND(id) = id`.

## Known limits

- 24 entries have `num_costumes == 0` (no `costumes/*.asm` registers them directly): BOSS, METAL, RANDOM, NONE, FALCO, GND, DRM, DSAMUS, LUCAS,
  GBOWSER, PIANO, MARTH, SONIC, SANDBAG, SSONIC, DEDEDE, PEPPY, MLUIGI, EBI, DRAGONKING, CRASH, ROY, DRL, LANKY. The port does not use the column for
  costume logic: it reads `Costumes.select_.num_costumes` and the extra-costume tables from the Remix RAM image (`docs/remix/notes/i7_part_image.md`).
- Comment labels that disagree in the source (warning only, the id matches): SSONIC (`SUPER SONIC`), MLUIGI (`METAL`), EBI (`EBISUMARU`).
- `file_*` are the argument text: `File.X` or a hex literal (for example `0x0D9`), with no RAM value resolved.
- `kRemixRoster` is `static const` in a header: every translation unit that includes it has its own copy (`-Wunused-const-variable` warning in
  units that do not use the table).
