# Smash Remix: extracting the fighter table from the ROM

Covers where the fighter (`FTData`) table sits in `baserom.remix.z64`, the RAM to ROM windows, and how it was validated against the decomp. Code:
`tools/remix_dump_fighters.py` (stdlib only; the Torch factory `SSB64:REMIXFIGHTERS` produces the same table, `docs/remix/torch_factories.md`).
The script writes JSON to `build/remix-dump/` (git-ignored); no ROM-derived data is versioned.
Test: `.toolchain/host/venv/bin/python -I tools/test_remix_blobs.py --o2r <BattleShip-Remix.o2r>` compares the Torch blob with a reference built from this JSON.
Open gaps: `docs/remix/patch_audit.md`, group g10.

Command:

```
.toolchain/host/venv/bin/python -I tools/remix_dump_fighters.py [--out DIR] [--remix ROM] [--vanilla ROM]
```

Outputs: `remix_fighters.json` (97 fighters), `vanilla_fighters.json` (27), `validation.json`.

## 1. Where the table is

Checked on the ROM bytes, not on documentation.

- **FTData struct table: ROM `0x92610` / RAM `0x80116E10`, in place.** The table was not relocated: the 27 vanilla entries are followed by 70 Remix
  entries, terminated by NULL at index 97. Entry 27 (RANDOM) points to `0x8049F470` (expanded segment) and entry 28 (NONE) points to Mario's struct
  (`0x80117810`), as `docs/remix/fighters.md` 1.1 says.
- **Confirmed RAM to ROM mapping:**
  - vanilla data `0x80084800`..: `ROM = RAM - 0x80084800` (confirmed by the struct table and the param arrays).
  - expanded segment `0x80400000`+: `ROM = RAM - 0x7D800000`.
  - overlay window `0x80390000`..`0x80400000` (menu/submotion arrays): `ROM = RAM - 0x80288A20`. It was found because the pattern of the first entry
    of `dFTMarioSubMotionDescs` appears at ROM `0x108480`, and the same relation holds for all 27 vanilla fighters with a consistent count.
- **ACTION_ARRAY_TABLE was moved by the Remix.** `docs/remix/fighters.md` 2.1 lists vanilla ROM `0xA6F40`; in the Remix the table is at RAM `0x804A05D0`
  (ROM `0x2CA05D0`) with 97 entries (`REMIX_ACTION_ARRAY_TABLE` in `port/remix/remix_action_layout.h`). The first 27 slots of `0xA6F40` are intact,
  the others are overwritten by Remix data. The address was decoded from the `move_action_array_table_` hook (ROM `0x62BD0`: `lui t9` + `lw` with a
  signed offset).
- **`FTData` layout (0x78 bytes):** confirmed against `decomp/src/ft/fttypes.h`. The 9 file ids are at `0x00..0x20`, `file_main_size` at `0x24`,
  `o_attributes` at `0x60`, `mainmotion` at `0x64` (count at `0x6C`), `submotion` at `0x68` with a pointer to the count at `0x70`, `file_anim_size` at
  `0x74`. The pointers `0x28..0x48` are runtime values and were not compared.
- **Motion entries:** 12 bytes, `(anim_file_id u32, command_offset u32, flags u32)`. The decomp's submotion array (0x68/0x70) is the Remix's 0xF entry
  menu array (checked: `ptr70 - ptr68 == count * 0xC` for 97 - 3 fighters).
- **Registered fighters:** 97 in the Remix, 27 in the vanilla ROM. All 97 were read successfully.

## 2. Validation against the decomp

Source: `decomp/src/ft/ftdata.c`, `#ifdef PORT` branch, with symbols resolved through `include/reloc_data.us.h`. The fkind order comes from the
`dFTManagerDataFiles` array.

Result for fkinds 0..26 (vanilla ROM against decomp):

- **Scalar fields: 0 differences** over 27 fighters (9 file ids, `file_main_size`, `o_attributes`, mainmotion count, `file_anim_size`).
- **Motion arrays: 0 differences.** 5825 mainmotion entries and 386 menu/submotion entries compared (anim id and flags; offsets are symbolic in the
  decomp and were not compared). The counts match everywhere.

`o_attributes` values for MMario, NMario, NFox and NKirby match the expectation:

| Fighter (fkind) | Vanilla | Remix |
|---|---|---|
| MMario (13) | 0x2A8 | 0x2BC |
| NMario (14) | 0x298 | 0x2B0 |
| NFox (15) | 0x2A4 | 0x2BC |
| NKirby (22) | 0x2C0 | 0x2F8 |

All 13 polygon/new fighters from 13 to 25 change `o_attributes` (vanilla to Remix): NDK (16) 0x298 -> 0x2B0, NSamus (17) 0x3BC -> 0x3D4, NLuigi (18) 0x298 -> 0x2B8,
NLink (19) 0x2D8 -> 0x2F0, NYoshi (20) 0x2B8 -> 0x2D0, NCaptain (21) 0x29C -> 0x2B4, NPikachu (23) 0x2A8 -> 0x2C0, NPurin (24) 0x2A0 -> 0x2E8, NNess (25) 0x2F0 -> 0x308.
Mario (0), Fox (1), DK (2), Samus (3), Luigi (4), Yoshi (6), Captain (7), Pikachu (9), Purin (10), Ness (11), Boss (12) and GDonkey (26, 0x3C8) do not change.
The port patches the vanilla rows 13 to 25 at load time (`patchVanillaAttributeOffsets` in `port/remix/remix_roster.cpp`).

### Remix against vanilla, per fkind (0..26)

| fkind | Fighter | Different fields |
|---|---|---|
| 0 | MARIO | only the param array pointer (`0x64`) changed; the entries are identical (array moved) |
| 5 | LINK | `action_array` (caveat 1) |
| 8 | KIRBY | mainmotion count (`0x6C`), pointer and entries of mainmotion, `action_array` (155 action extension, expected) |
| 13-25 | MMario..NNess | `o_attributes` (`0x60`) and the content of the menu array |
| 18, 24 | NLuigi, NPurin | also the submotion/menu pointer; the count pointer (`0x70`) still points at the vanilla value |
| 26 | GDONKEY | submotion/menu pointer and menu content |

Identical to vanilla: 1 FOX, 2 DONKEY, 3 SAMUS, 4 LUIGI, 6 YOSHI, 7 CAPTAIN, 9 PIKACHU, 10 PURIN, 11 NESS, 12 BOSS. Of the 27 vanilla fighters, 17 differ.

## 3. Caveats

1. **Action array counts are not stored in the struct** and the decomp has no such tables. The script infers a size from the gap between distinct pointers
   (`count_method: inferred_gap_unverified`), which fails: Link (5) has 17 entries in vanilla and 100 in the Remix from the same pointer `0x8012A5AC`.
   Treat the JSON `entries` as unreliable. The port gets per-id action counts from `port/remix/remix_action_layout.h`, generated by
   `tools/gen_remix_actions.py` from the assembled Remix build.
2. RANDOM (27) and NONE (28) have pointer 0 in the Remix action table; the script records `count: 0`.
3. For 13..26 the content of the Remix menu array differs from vanilla, where most entries are zero (`anim 0`, `cmd 0x80000000`). Origin
   (2026-11-05): `Character.asm:3573-3592` copies menu actions 1 to 13 of the base fighter into Metal Mario and the polygons
   (`copy_menu_actions`, which builds a 14-row array for N Luigi and N Jigglypuff) and points Giant DK at DK's array. The port applies
   the same copies at roster load (`patchVanillaMenuMotions`, ledger CT-24).
4. NLuigi (18), NPurin (24) and GDonkey (26): the submotion array pointer changed but the count pointer is still the vanilla one
   (`consistent_with_count_pointer: false` in the JSON).
5. Not compared with the decomp: motion offsets (symbolic in the decomp), runtime pointers, and the 70 Remix-only fighters (no decomp reference). Remix
   fighter names are not in the JSON, only ids; names are in `src/Character.asm` (`define_character`, `docs/remix/fighters.md` 1.2).
