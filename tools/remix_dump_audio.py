#!/usr/bin/env python3
"""Reconstruct Smash Remix audio banks from baserom.remix.z64 and validate them.

Runtime model (see docs/remix/audio.md and docs/remix/audio_extraction.md):

- A ctl bank is DMAed to a RAM base (INST_CTL_TABLE_PC / CTL_TABLE_PC). Every
  offset inside the ctl is relative to that RAM base.
- The first bytes of the ctl come from the vanilla blob in ROM. Offsets past the
  vanilla blob land in the Remix code image, which is mapped at RAM 0x80400000
  (ROM 0x2C00000) with ROM = RAM - 0x7D800000.
- RAM between the vanilla blob and the code image is not backed by ROM and is
  reconstructed as zeros. A reference into that gap is reported as unresolved.
- Wavetable base offsets are relative to the TBL base (INST_SAMPLE_DATA /
  RAW_SAMPLE_DATA). Remix samples live after the song data in ROM, so the TBL
  blob is a contiguous ROM slice that extends to the last referenced sample.
- The SBK blob is a contiguous ROM slice starting at the SBK table; entry offsets
  are relative to that start.

Usage: .toolchain/host/venv/bin/python -I tools/remix_dump_audio.py [--rom PATH] [--out DIR]
Requires only the Python standard library.
"""

import argparse
import json
import struct
import sys
from collections import Counter
from dataclasses import dataclass
from pathlib import Path

ROM_RAM_DELTA = 0x7D800000
CODE_RAM_START = 0x80400000
CODE_ROM_START = 0x2C00000
CODE_ROM_END = 0x3000000
EXPECTED_ROM_SIZE = 0x3E80000
DEFAULT_VANILLA_ROM = Path(__file__).resolve().parent.parent / "baserom.us.z64"

MUSIC_TBL_ROM = 0x2531C80
SFX_TBL_ROM = 0x2658190
SBK_ROM = 0x2504750
SBK_ENTRY_COUNT = 0x177
MUSIC_BANK_OFFSET = 0x3C9EA0

ADPCM_WAVE = 0
RAW16_WAVE = 1
MAX_BOOK_ORDER = 8
MAX_BOOK_PREDICTORS = 8

FGM_UNK_ROM = 0x2934370
FGM_UNK_SIZE = 0x820
FGM_TBL_ROM = 0x2934B90
FGM_TBL_SIZE = 0x2DD0
FGM_UCD_ROM = 0x2937960
FGM_UCD_SIZE = 0x4B20
SFX_FGM_MAP_RAM = 0x80073F80
FGM_MICROCODE_MAP_RAM = 0x80076D50
# FGM.asm: DIFFERENCE = (((midi_count + 1) * 8 + 0xF) & 0xFFF0) - 0x35C0. Remix has
# midi_count 374 or 375, and both give -0x2A00. Verified against ROM bytes (vanilla records).
FGM_MAP_DIFFERENCE = -0x2A00
FGM_RECORD_CAP = 0x100
VANILLA_FGM_TBL_ROM = 0xF57BF0
VANILLA_FGM_UCD_ROM = 0xF5A9C0
VANILLA_FGM_UCD_END = 0xF5F4E0


@dataclass(frozen=True)
class CtlSpec:
    name: str
    ram_base: int
    vanilla_rom: int
    vanilla_len: int
    tbl_rom: int
    vanilla_tbl_len: int


MUSIC_CTL = CtlSpec("B1_sounds1_ctl", 0x800472D0, 0x252B560, 0x6720, MUSIC_TBL_ROM, 0x116970)
SFX_CTL = CtlSpec("B1_sounds2_ctl", 0x8004D9F0, 0x26485F0, 0xFBA0, SFX_TBL_ROM, 0x2DC1E0)


def be32(data, off):
    return struct.unpack_from(">I", data, off)[0]


def be32s(data, off):
    return struct.unpack_from(">i", data, off)[0]


def be16s(data, off):
    return struct.unpack_from(">h", data, off)[0]


def fail(message):
    print(f"error: {message}", file=sys.stderr)
    sys.exit(1)


def check_rom(rom):
    if len(rom) != EXPECTED_ROM_SIZE:
        fail(f"ROM is {len(rom):#x} bytes, expected {EXPECTED_ROM_SIZE:#x}. Pass the Remix ROM with --rom.")
    if CODE_RAM_START - ROM_RAM_DELTA != CODE_ROM_START:
        fail("code segment constants are inconsistent")
    if be32(rom, MUSIC_CTL.vanilla_rom + 4) != MUSIC_BANK_OFFSET:
        fail(f"music ctl header at {MUSIC_CTL.vanilla_rom:#x} does not point to bank {MUSIC_BANK_OFFSET:#x}")
    if be16s(rom, SBK_ROM + 2) != SBK_ENTRY_COUNT:
        fail(f"sbk table at {SBK_ROM:#x} has {be16s(rom, SBK_ROM + 2)} entries, expected {SBK_ENTRY_COUNT}")


def build_ctl_blob(rom, spec):
    """Assemble the runtime image of a ctl bank as one contiguous blob.

    Returns (blob, mapped) where mapped lists the (start, end) byte ranges that
    are backed by ROM. Bytes outside `mapped` are zero-filled gaps.
    """
    code_off = CODE_RAM_START - spec.ram_base
    code_len = CODE_ROM_END - CODE_ROM_START
    if code_off < spec.vanilla_len:
        fail(f"{spec.name}: vanilla blob overlaps the code image")
    blob = bytearray(code_off + code_len)
    blob[0:spec.vanilla_len] = rom[spec.vanilla_rom:spec.vanilla_rom + spec.vanilla_len]
    blob[code_off:code_off + code_len] = rom[CODE_ROM_START:CODE_ROM_END]
    mapped = [(0, spec.vanilla_len), (code_off, code_off + code_len)]
    return bytes(blob), mapped


def build_rom_slice(rom, start, length):
    if start + length > len(rom):
        fail(f"slice {start:#x}+{length:#x} runs past the ROM end")
    return bytes(rom[start:start + length])


class BankParser:
    """Bounds-checked port of port/bridge/audio_bridge.cpp BankParser.

    Parses the N64 ALBankFile layout from a ctl blob without allocating native
    structs. Every non-null pointer is checked; failures are recorded in
    `stats` instead of being silently replaced by NULL.
    """

    def __init__(self, ctl, mapped, tbl_len):
        self.ctl = ctl
        self.mapped = mapped
        self.tbl_len = tbl_len
        self.cache = {}
        self.stats = Counter()
        self.sample_end = 0
        self.sample_bases = []
        self.refs = Counter()

    def is_mapped(self, off, need):
        return any(start <= off and off + need <= end for start, end in self.mapped)

    def span_ok(self, kind, off, need):
        if off + need > len(self.ctl):
            self.stats[f"out_of_range_{kind}"] += 1
            return False
        if not self.is_mapped(off, need):
            self.stats[f"unresolved_{kind}"] += 1
            return False
        return True

    def cached(self, kind, off, make):
        key = (kind, off)
        if key in self.cache:
            return self.cache[key]
        value = make()
        self.cache[key] = value
        self.refs[kind] += 1
        return value

    def parse_envelope(self, off):
        def make():
            r = off
            env = {
                "attack_time": be32s(self.ctl, r),
                "decay_time": be32s(self.ctl, r + 4),
                "release_time": be32s(self.ctl, r + 8),
                "attack_volume": self.ctl[r + 12],
                "decay_volume": self.ctl[r + 13],
            }
            if min(env["attack_time"], env["decay_time"], env["release_time"]) < 0:
                self.stats["sanity_envelope_negative_time"] += 1
            return env
        if not self.span_ok("envelope", off, 14):
            return None
        return self.cached("envelope", off, make)

    def parse_keymap(self, off):
        def make():
            r = off
            key = {
                "velocity_min": self.ctl[r], "velocity_max": self.ctl[r + 1],
                "key_min": self.ctl[r + 2], "key_max": self.ctl[r + 3],
                "key_base": self.ctl[r + 4], "detune": struct.unpack_from(">b", self.ctl, r + 5)[0],
            }
            if key["velocity_min"] > key["velocity_max"] or key["key_min"] > key["key_max"]:
                self.stats["sanity_keymap_min_above_max"] += 1
            return key
        if not self.span_ok("keymap", off, 6):
            return None
        return self.cached("keymap", off, make)

    def parse_book(self, off):
        if not self.span_ok("book_header", off, 8):
            return None
        order = be32s(self.ctl, off)
        npred = be32s(self.ctl, off + 4)
        if not (0 < order <= MAX_BOOK_ORDER and 0 < npred <= MAX_BOOK_PREDICTORS):
            self.stats["sanity_book_order_or_npred"] += 1
            return None
        need = 8 + order * npred * 16
        if not self.span_ok("book", off, need):
            return None

        def make():
            self.stats["book_max_order"] = max(self.stats["book_max_order"], order)
            self.stats["book_max_npred"] = max(self.stats["book_max_npred"], npred)
            return {"order": order, "npredictors": npred}
        return self.cached("book", off, make)

    def parse_adpcm_loop(self, off):
        if not self.span_ok("adpcm_loop", off, 44):
            return None
        return self.cached("adpcm_loop", off, lambda: {
            "start": be32(self.ctl, off), "end": be32(self.ctl, off + 4), "count": be32(self.ctl, off + 8),
        })

    def parse_raw_loop(self, off):
        if not self.span_ok("raw_loop", off, 12):
            return None
        return self.cached("raw_loop", off, lambda: {
            "start": be32(self.ctl, off), "end": be32(self.ctl, off + 4), "count": be32(self.ctl, off + 8),
        })

    def parse_wavetable(self, off):
        if not self.span_ok("wavetable", off, 20):
            return None

        def make():
            r = off
            base_off = be32(self.ctl, r)
            length = be32s(self.ctl, r + 4)
            wave_type = self.ctl[r + 8]
            if wave_type not in (ADPCM_WAVE, RAW16_WAVE) or length <= 0:
                self.stats["sanity_wavetable_type_or_len"] += 1
            self.sample_bases.append(base_off)
            if base_off + length > self.tbl_len:
                self.stats["out_of_range_sample"] += 1
            else:
                self.sample_end = max(self.sample_end, base_off + length)
            wave = {"base_offset": base_off, "length": length, "type": wave_type}
            if wave_type == ADPCM_WAVE:
                wave["loop"] = self.parse_optional(self.parse_adpcm_loop, be32(self.ctl, r + 12))
                wave["book"] = self.parse_optional(self.parse_book, be32(self.ctl, r + 16))
                self.stats["wave_adpcm"] += 1
            else:
                wave["loop"] = self.parse_optional(self.parse_raw_loop, be32(self.ctl, r + 12))
                self.stats["wave_raw"] += 1
            return wave
        return self.cached("wavetable", off, make)

    def parse_sound(self, off):
        if not self.span_ok("sound", off, 16):
            return None

        def make():
            r = off
            return {
                "envelope": self.parse_optional(self.parse_envelope, be32(self.ctl, r)),
                "keymap": self.parse_optional(self.parse_keymap, be32(self.ctl, r + 4)),
                "wavetable": self.parse_optional(self.parse_wavetable, be32(self.ctl, r + 8)),
                "pan": self.ctl[r + 12],
                "volume": self.ctl[r + 13],
            }
        return self.cached("sound", off, make)

    def parse_instrument(self, off):
        if not self.span_ok("instrument_header", off, 16):
            return None
        count = be16s(self.ctl, off + 14)
        if count < 0:
            self.stats["sanity_instrument_negative_count"] += 1
            return None
        if not self.span_ok("instrument_sounds", off, 16 + 4 * count):
            return None

        def make():
            sounds = []
            for i in range(count):
                sounds.append(self.parse_optional(self.parse_sound, be32(self.ctl, off + 16 + 4 * i)))
            return {"sound_count": count, "sounds": sounds}
        return self.cached("instrument", off, make)

    def parse_bank(self, off):
        if not self.span_ok("bank_header", off, 12):
            return None
        count = be16s(self.ctl, off)
        if count < 0:
            self.stats["sanity_bank_negative_count"] += 1
            return None
        if not self.span_ok("bank_instruments", off, 12 + 4 * count):
            return None

        def make():
            percussion = self.parse_optional(self.parse_instrument, be32(self.ctl, off + 8))
            instruments = [
                self.parse_optional(self.parse_instrument, be32(self.ctl, off + 12 + 4 * i))
                for i in range(count)
            ]
            return {"inst_count": count, "sample_rate": be32s(self.ctl, off + 4),
                    "percussion": percussion, "instruments": instruments}
        return self.cached("bank", off, make)

    def parse_bank_file(self):
        if not self.span_ok("bank_file_header", 0, 4):
            return None
        count = be16s(self.ctl, 2)
        if not self.span_ok("bank_file_table", 0, 4 + 4 * count):
            return None
        return {
            "revision": be16s(self.ctl, 0),
            "banks": [self.parse_optional(self.parse_bank, be32(self.ctl, 4 + 4 * i)) for i in range(count)],
        }

    def parse_optional(self, parse, off):
        """Follow a pointer field: offset 0 is NULL, any other offset must parse."""
        if off == 0:
            self.stats["null_pointer"] += 1
            return None
        return parse(off)


def count_instruments(bank):
    parsed = [inst for inst in bank["instruments"] if inst is not None]
    sounds = [s for inst in parsed for s in inst["sounds"] if s is not None]
    return {
        "inst_count": bank["inst_count"],
        "instruments_parsed": len(parsed),
        "sound_refs_parsed": len(sounds),
        "percussion_parsed": bank["percussion"] is not None,
    }


def count_regions(parser):
    """Split cached ctl records into vanilla-blob and code-image origins."""
    vanilla_end = parser.mapped[0][1]
    regions = Counter()
    for kind, off in parser.cache:
        regions[f"{kind}_{'vanilla' if off < vanilla_end else 'code_image'}"] += 1
    return dict(sorted(regions.items()))


def parse_ctl(name, ctl, mapped, tbl_len, vanilla_tbl_len):
    parser = BankParser(ctl, mapped, tbl_len)
    bank_file = parser.parse_bank_file()
    if bank_file is None or not bank_file["banks"] or bank_file["banks"][0] is None:
        return {"name": name, "error": "bank 0 does not parse"}, parser
    summary = {"name": name, "bank_count": len(bank_file["banks"])}
    summary.update(count_instruments(bank_file["banks"][0]))
    summary["sample_end"] = parser.sample_end
    summary["samples_beyond_vanilla_tbl"] = sum(1 for base in parser.sample_bases if base >= vanilla_tbl_len)
    summary["refs"] = dict(parser.refs)
    summary["record_regions"] = count_regions(parser)
    summary["stats"] = dict(parser.stats)
    return summary, parser


def parse_sbk(sbk):
    """Check the sequence table: 'S1' header, s16 count, 8-byte (offset, size) entries."""
    report = {"header": sbk[0:2].decode("latin-1"), "count": be16s(sbk, 2), "entries_in_bounds": 0,
              "entries_out_of_bounds": 0, "max_end": 0}
    for i in range(report["count"]):
        off = be32(sbk, 4 + 8 * i)
        size = be32(sbk, 8 + 8 * i)
        if off + size <= len(sbk):
            report["entries_in_bounds"] += 1
            report["max_end"] = max(report["max_end"], off + size)
        else:
            report["entries_out_of_bounds"] += 1
    return report


def sbk_length_needed(rom):
    """Compute the SBK slice length from the table entries read in the ROM."""
    count = be16s(rom, SBK_ROM + 2)
    end = 4 + 8 * count
    for i in range(count):
        off = be32(rom, SBK_ROM + 4 + 8 * i)
        size = be32(rom, SBK_ROM + 8 + 8 * i)
        end = max(end, off + size)
    return end


def tbl_length_needed(spec, ctl, mapped, rom):
    """Pass 1: parse the ctl with an unbounded TBL to learn the furthest sample byte."""
    tbl_limit = len(rom) - spec.tbl_rom
    summary, _ = parse_ctl("pass1", ctl, mapped, tbl_limit, spec.vanilla_tbl_len)
    return summary.get("sample_end", 0)


def fgm_record_rom(blob_rom, blob_size, map_ram, pointer, rom_len):
    """Map one FGM table pointer to a ROM offset.

    Pointers below the blob size are relative to the blob start (vanilla layout).
    Larger pointers are RAM addresses relative to the moved map, which Remix places
    in the code image (ROM = RAM - ROM_RAM_DELTA).
    """
    if pointer < blob_size:
        start = blob_rom + pointer
    else:
        start = map_ram + FGM_MAP_DIFFERENCE + pointer - ROM_RAM_DELTA
    if not 0 <= start < rom_len:
        fail(f"FGM record pointer {pointer:#x} resolves to ROM {start:#x}, outside the ROM")
    return start


def fgm_record_end(start, following, rom_len):
    """Return the end of the record starting at `start`.

    A record ends at the next distinct record start, or after FGM_RECORD_CAP bytes.
    The cap matters for the last record of a run. In baserom.us.z64, 20 records are longer
    than 0x40 (largest 0xFB), so the cap must be above that.
    """
    end = min(start + FGM_RECORD_CAP, rom_len)
    if following is not None:
        end = min(end, following)
    return end


def read_fgm_pointers(rom, blob_rom):
    """Return the raw BE u32 pointers of an FGM table, in table order."""
    count = be32(rom, blob_rom)
    return [be32(rom, blob_rom + 4 + 4 * i) for i in range(count)]


def fgm_entry_starts(rom, blob_rom, blob_size, map_ram):
    """Return the ROM offset of every record an FGM table points to, in table order."""
    return [fgm_record_rom(blob_rom, blob_size, map_ram, p, len(rom)) for p in read_fgm_pointers(rom, blob_rom)]


def build_fgm_compact_blob(rom, blob_rom, blob_size, map_ram):
    """Rebuild an FGM table as `BE u32 count`, `count` BE u32 offsets, then the records.

    Offsets are relative to the start of the returned blob, and entries that point to
    the same record share one copy of it.
    """
    starts = fgm_entry_starts(rom, blob_rom, blob_size, map_ram)
    count = len(starts)
    header_size = 4 + 4 * count
    distinct = sorted(set(starts))
    following = distinct[1:] + [None]
    offsets = {}
    records = bytearray()
    for start, next_start in zip(distinct, following):
        end = fgm_record_end(start, next_start, len(rom))
        offsets[start] = header_size + len(records)
        records += rom[start:end]
    table = bytearray(struct.pack(">I", count))
    for start in starts:
        table += struct.pack(">I", offsets[start])
    return bytes(table + records)


def check_vanilla_fgm_table(rom, vanilla, blob_rom, blob_size, vanilla_rom, vanilla_end):
    """Compare each vanilla-relative Remix FGM record with the vanilla record of the same index.

    Only pointers below blob_size are vanilla-relative, and only indices that exist in the
    vanilla table are compared. vanilla_end bounds the last vanilla record, because the
    bytes after it belong to another segment. Returns (checked count, mismatched indices).
    """
    remix_pointers = read_fgm_pointers(rom, blob_rom)
    vanilla_starts = fgm_entry_starts(vanilla, vanilla_rom, len(vanilla), 0)
    distinct = sorted(set(vanilla_starts))
    next_starts = dict(zip(distinct, distinct[1:] + [vanilla_end]))
    checked = 0
    mismatches = []
    for index, start in enumerate(vanilla_starts[:len(remix_pointers)]):
        if remix_pointers[index] >= blob_size:
            continue
        expected = vanilla[start:fgm_record_end(start, next_starts[start], len(vanilla))]
        remix_start = blob_rom + remix_pointers[index]
        actual = rom[remix_start:remix_start + len(expected)]
        checked += 1
        if actual != expected:
            mismatches.append(index)
    return checked, mismatches


def verify_vanilla_fgm(rom, vanilla_path):
    """Check the vanilla-relative FGM records against baserom.us.z64 when that file exists."""
    if not vanilla_path.exists():
        return {"skipped": f"{vanilla_path.name} not found"}
    vanilla = vanilla_path.read_bytes()
    result = {}
    tables = [
        ("fgm_tbl", FGM_TBL_ROM, FGM_TBL_SIZE, VANILLA_FGM_TBL_ROM, VANILLA_FGM_UCD_ROM),
        ("fgm_ucd", FGM_UCD_ROM, FGM_UCD_SIZE, VANILLA_FGM_UCD_ROM, VANILLA_FGM_UCD_END),
    ]
    for name, blob_rom, blob_size, vanilla_rom, vanilla_end in tables:
        checked, mismatches = check_vanilla_fgm_table(rom, vanilla, blob_rom, blob_size, vanilla_rom, vanilla_end)
        if mismatches:
            fail(f"{name}: {len(mismatches)} vanilla-relative records differ from {vanilla_path.name}, "
                 f"first index {mismatches[0]}")
        result[name] = {"checked": checked, "mismatches": 0}
    return result


def write_outputs(out_dir, files):
    out_dir.mkdir(parents=True, exist_ok=True)
    for filename, data in files.items():
        (out_dir / filename).write_bytes(data)


def run(rom_path, out_dir, vanilla_path=DEFAULT_VANILLA_ROM):
    rom = rom_path.read_bytes()
    check_rom(rom)

    music_ctl, music_mapped = build_ctl_blob(rom, MUSIC_CTL)
    sfx_ctl, sfx_mapped = build_ctl_blob(rom, SFX_CTL)

    music_tbl_len = tbl_length_needed(MUSIC_CTL, music_ctl, music_mapped, rom)
    sfx_tbl_len = tbl_length_needed(SFX_CTL, sfx_ctl, sfx_mapped, rom)
    music_tbl = build_rom_slice(rom, MUSIC_CTL.tbl_rom, music_tbl_len)
    sfx_tbl = build_rom_slice(rom, SFX_CTL.tbl_rom, sfx_tbl_len)

    sbk_len = sbk_length_needed(rom)
    sbk = build_rom_slice(rom, SBK_ROM, sbk_len)

    music_summary, _ = parse_ctl(MUSIC_CTL.name, music_ctl, music_mapped, len(music_tbl), MUSIC_CTL.vanilla_tbl_len)
    sfx_summary, _ = parse_ctl(SFX_CTL.name, sfx_ctl, sfx_mapped, len(sfx_tbl), SFX_CTL.vanilla_tbl_len)

    files = {
        f"{MUSIC_CTL.name}.bin": music_ctl,
        f"{SFX_CTL.name}.bin": sfx_ctl,
        "B1_sounds1_tbl.bin": music_tbl,
        "B1_sounds2_tbl.bin": sfx_tbl,
        "S1_music_sbk.bin": sbk,
        "fgm_unk.bin": build_rom_slice(rom, FGM_UNK_ROM, FGM_UNK_SIZE),
        "fgm_tbl.bin": build_fgm_compact_blob(rom, FGM_TBL_ROM, FGM_TBL_SIZE, SFX_FGM_MAP_RAM),
        "fgm_ucd.bin": build_fgm_compact_blob(rom, FGM_UCD_ROM, FGM_UCD_SIZE, FGM_MICROCODE_MAP_RAM),
    }
    fgm_vanilla = verify_vanilla_fgm(rom, vanilla_path)
    write_outputs(out_dir, files)

    manifest = {
        "layout": {
            "music_ctl": {"ram_base": hex(MUSIC_CTL.ram_base), "blob_len": len(music_ctl),
                          "vanilla_rom": hex(MUSIC_CTL.vanilla_rom), "vanilla_len": MUSIC_CTL.vanilla_len},
            "sfx_ctl": {"ram_base": hex(SFX_CTL.ram_base), "blob_len": len(sfx_ctl),
                        "vanilla_rom": hex(SFX_CTL.vanilla_rom), "vanilla_len": SFX_CTL.vanilla_len},
            "music_tbl": {"rom": hex(MUSIC_TBL_ROM), "blob_len": len(music_tbl)},
            "sfx_tbl": {"rom": hex(SFX_TBL_ROM), "blob_len": len(sfx_tbl)},
            "sbk": {"rom": hex(SBK_ROM), "blob_len": len(sbk)},
            "fgm_unk": {"rom": hex(FGM_UNK_ROM), "blob_len": FGM_UNK_SIZE},
            "fgm_tbl": {"rom": hex(FGM_TBL_ROM), "blob_len": len(files["fgm_tbl.bin"])},
            "fgm_ucd": {"rom": hex(FGM_UCD_ROM), "blob_len": len(files["fgm_ucd.bin"])},
        },
        "validation": {
            "music_ctl": music_summary,
            "sfx_ctl": sfx_summary,
            "sbk": parse_sbk(sbk),
            "fgm_vanilla_records": fgm_vanilla,
        },
    }
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    return manifest


def main(argv):
    repo_root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--rom", type=Path, default=repo_root / "baserom.remix.z64")
    parser.add_argument("--out", type=Path, default=repo_root / "build" / "remix-dump" / "audio")
    parser.add_argument("--vanilla", type=Path, default=DEFAULT_VANILLA_ROM)
    args = parser.parse_args(argv)
    manifest = run(args.rom, args.out, args.vanilla)
    print(json.dumps(manifest["validation"], indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
