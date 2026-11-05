# Smash Remix audio: rebuilding the banks (ctl/tbl/sbk/fgm)

Covers the model that turns the Remix ROM ranges into the eight audio blobs the port loads. Code: the Torch factory
`SSB64:REMIXAUDIO` (`torch/src/factories/ssb64/RemixAudioFactory.cpp`, `RemixAudioCtl.cpp`, `docs/remix/torch_factories.md`) and its Python
reference `tools/remix_dump_audio.py` (stdlib only), which produces the same bytes. The reference writes to `build/remix-dump/audio/`
(git-ignored); nothing derived from the ROM goes to a tracked path.
Test: `.toolchain/host/venv/bin/python -I tools/test_remix_blobs.py --o2r <BattleShip-Remix.o2r>` compares every Torch blob with the reference.
Open gaps: `docs/remix/patch_audit.md`, group g9.

Reference command:

```
.toolchain/host/venv/bin/python -I tools/remix_dump_audio.py [--rom baserom.remix.z64] [--vanilla baserom.us.z64] [--out build/remix-dump/audio]
```

Files written: `B1_sounds1_ctl.bin`, `B1_sounds2_ctl.bin`, `B1_sounds1_tbl.bin`, `B1_sounds2_tbl.bin`, `S1_music_sbk.bin`, `fgm_unk.bin`,
`fgm_tbl.bin`, `fgm_ucd.bin`, `manifest.json` (layout and validation numbers). `--vanilla` is only used to compare the vanilla-relative
FGM records.

## Result

- Both banks parse clean after the reconstruction: 0 pointers out of range, 0 references into unmapped gaps, 0 sanity violations (envelope,
  keymap, book, wavetable).
- Music: 71 instruments in the header, 70 parsed (index 0 is NULL), 326 sounds, 324 ADPCM wavetables.
- SFX: 1 instrument, 1137 sounds, 1137 ADPCM wavetables, 815 samples beyond the vanilla TBL (the 815 new SFX of `docs/remix/audio.md` section 1.4).
- FGM tables: every vanilla-relative record equals the vanilla record of the same index (`fgm_tbl` 356 checked, `fgm_ucd` 535 checked, 0 mismatches).
- Sizes: `B1_sounds1_ctl` 8 097 072 bytes; `B1_sounds2_ctl` 8 070 672; `B1_sounds1_tbl` 17 183 478; `B1_sounds2_tbl` 24 680 728;
  `S1_music_sbk` 14 712 768; `fgm_unk` 2 080; `fgm_tbl` 26 525; `fgm_ucd` 38 138.

## Method

1. **RAM to ROM.** The Remix code image sits at RAM `0x80400000` = ROM `0x2C00000`, with `ROM = RAM - 0x7D800000`. The upper bound used is `0x3000000`,
   the start of `MIDI_BANK`. Checked on the bytes: the bank offset `0x3C9EA0` plus the RAM base `0x800472D0` is `0x80411170`, which is ROM
   `0x2C11170`, where the 71-instrument header is.
2. **Ctl as a runtime image.** Each ctl is DMAed to a RAM base (`INST_CTL_TABLE_PC = 0x800472D0`, `CTL_TABLE_PC = 0x8004D9F0`) and every offset is
   relative to that base. The rebuilt blob starts at that base and holds three ranges:
   - `[0, len_vanilla)`: the vanilla ROM blob (music at `0x252B560`, SFX at `0x26485F0`).
   - `[RAM 0x80400000 - base, + 0x400000)`: the code image, copied from ROM `0x2C00000..0x3000000`.
   - the rest: zeros, not backed by ROM. A reference into that range counts as unresolved. Nothing in the music bank points there; on hardware that
     range is the other ctl at runtime.
3. **TBL.** Wavetable offsets are relative to the TBL base (`INST_SAMPLE_DATA` and `RAW_SAMPLE_DATA`, read from the ROM pointer table at
   `0x3D750..0x3D768`). The rebuilt TBL is a contiguous ROM slice from that base to the last referenced byte (pass 1 computes the maximum end).
   New samples sit after the song data, at `0x3000000+`, so the slice ends at `0x3594F76` (music) and `0x3DE1AA8` (SFX). The Torch factory computes the
   reach by walking the ctl from the bank file with the same bounds rules as `BankParser` (`FindRemixSampleExtent`); only the wavetable reach counts.
4. **SBK.** Contiguous slice from `0x2504750` to the end of the last entry (`0x330C710`). The entry offsets are relative to the SBK start.
5. **FGM tables.** `fgm_unk` is a raw slice of 0x820 bytes at `0x2934370`. `fgm_tbl` (`0x2934B90`, 0x2DD0 B) and `fgm_ucd` (`0x2937960`, 0x4B20 B) are
   tables of record pointers; a pointer below the blob size is blob-relative, any other is resolved through the map RAM base (`SFX_FGM_MAP`
   `0x80073F80`, `FGM_MICROCODE_MAP` `0x80076D50`) with the `FGM.asm` difference `-0x2A00`, then RAM to ROM. The blob is rebuilt compact:
   `BE u32 count`, `count` BE u32 offsets, then the distinct records (a record ends at the next distinct start or after 0x100 bytes). The port parses
   these as `SYAudioPackage`.
6. **Validation.** `BankParser` of the reference is a Python port, with bounds checks, of `port/bridge/audio_bridge.cpp`. Instead of silently replacing
   invalid pointers with NULL, it counts each case. It also checks the ADPCM book (order and npredictors, limit 8).

## Final layout

| Blob | RAM base | ROM origin | Size (bytes) | Mapping |
|---|---|---|---|---|
| `B1_sounds1_ctl` | `0x800472D0` | vanilla `0x252B560` (len `0x6720`) + code `0x2C00000..0x3000000` | 8 097 072 (`0x7B8D30`) | vanilla at `[0, 0x6720)`, code at `[0x3B8D30, 0x7B8D30)`, zeros between |
| `B1_sounds2_ctl` | `0x8004D9F0` | vanilla `0x26485F0` (len `0xFBA0`) + code | 8 070 672 (`0x7B2610`) | vanilla at `[0, 0xFBA0)`, code at `[0x3B2610, 0x7B2610)`, SFX extension at `0x3C5810` (= ROM `0x2C13200`) |
| `B1_sounds1_tbl` | n/a | ROM `0x2531C80` to `0x3594F76` | 17 183 478 (`0x10632F6`) | wavetable base = offset relative to `0x2531C80` |
| `B1_sounds2_tbl` | n/a | ROM `0x2658190` to `0x3DE1AA8` | 24 680 728 (`0x1789918`) | wavetable base = offset relative to `0x2658190` |
| `S1_music_sbk` | n/a | ROM `0x2504750` to `0x330C710` | 14 712 768 (`0xE07FC0`) | entries `(offset, size)` relative to the SBK start |
| `fgm_unk` | n/a | ROM `0x2934370` | 2 080 | flat envelope array |
| `fgm_tbl` | n/a | ROM `0x2934B90` | 26 525 | compact FGM table (SFX) |
| `fgm_ucd` | n/a | ROM `0x2937960` | 38 138 | compact FGM table (microcode) |

In `B1_sounds1_ctl` the offsets of the new music banks (`0x3BEDA8..0x3C9E78`) fall in the code range, and the 42 vanilla instruments fall in the vanilla blob.
Each ctl is about 8 MB, mostly zero padding, because the ctl offsets are absolute from the RAM base.

## Validation numbers

Source: `manifest.json`, section `validation`.

| Metric | Music (`B1_sounds1_ctl`) | SFX (`B1_sounds2_ctl`) |
|---|---|---|
| Instruments in the header | 71 | 1 |
| Instruments parsed (not NULL) | 70 (+ percussion parsed) | 1 |
| Sounds referenced and parsed | 326 | 1137 |
| Envelopes / keymaps / books / wavetables | 215 / 326 / 324 / 324 | 1 / 816 / 1137 / 1137 |
| ADPCM loops | 138 | 26 |
| NULL pointers (offset 0) | 187 | 1112 |
| Records in the vanilla range / in the code image | sounds 119 / 207, wavetables 117 / 207 | sounds 322 / 815, wavetables 318 / 819 |
| Wavetables with a base beyond the vanilla TBL | 207 | 815 |
| Out of range (pointers) | 0 | 0 |
| Unresolved (fall in a gap) | 0 | 0 |
| Sanity violations (envelope, keymap, book, wavetable type, negative count) | 0 | 0 |
| Maximum book (order / npredictors) | 2 / 4 | 2 / 4 |
| Referenced samples with all bytes zero | 0 of 324 | 0 of 1137 |

SBK: header `S1`, 375 entries, 375 inside the limits, 0 outside. The vanilla block of sequences (entries 1..46) stays in the original ROM range.
The sample sizes match the end of the TBL (`base + len` maximum equals the blob size in both cases).

## Port consumption

`port/bridge/audio_bridge.cpp` loads the eight `audio/*` blobs from the o2r. `BankParser::fits()` bounds every read against the size of the
rebuilt ctl blob, so the relocated Remix bank (offset `0x3C9EA0`) is inside the blob and parses; a bank that points outside the blob disables audio
with an error log. See `docs/remix/audio.md` section 5.
