#!/usr/bin/env python3
"""Dump fighter (FTData) tables from the Smash Remix and vanilla ROMs as JSON.

Stdlib only. Reads big-endian z64 images and writes JSON under --out
(default build/remix-dump/, git-ignored). ROM-derived data must never be
written anywhere tracked.

Outputs (in --out):
  remix_fighters.json   every fighter in the Remix struct table
  vanilla_fighters.json every fighter in the vanilla struct table
  validation.json       vanilla values vs decomp/src/ft/ftdata.c, and Remix vs vanilla diffs

Usage: python3 tools/remix_dump_fighters.py [--out DIR] [--remix ROM] [--vanilla ROM]
"""

import argparse
import json
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_OUT = REPO_ROOT / "build" / "remix-dump"
DEFAULT_REMIX = REPO_ROOT / "baserom.remix.z64"
DEFAULT_VANILLA = REPO_ROOT / "baserom.us.z64"
DECOMP_FTDATA = REPO_ROOT / "decomp" / "src" / "ft" / "ftdata.c"
RELOC_HEADER = REPO_ROOT / "include" / "reloc_data.us.h"

STRUCT_TABLE_ROM = 0x92610
STRUCT_TABLE_RAM = 0x80116E10
ACTION_TABLE_HOOK_ROM = 0x62BD0
MAX_FIGHTERS = 0x100
VANILLA_FIGHTER_COUNT = 27

FT_DATA_SIZE = 0x78
MOTION_ENTRY_SIZE = 0x0C
ACTION_ENTRY_SIZE = 0x14
VANILLA_DATA_BASE = 0x80084800
OVERLAY_WINDOW = (0x80390000, 0x80400000)
OVERLAY_BASE = 0x80288A20
EXPANDED_BASE = 0x80400000
EXPANDED_ROM_BASE = 0x2C00000

FILE_ID_OFFSETS = [0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20]
FIELD_OFFSETS = {
    "file_main_size": 0x24,
    "p_file_main": 0x28,
    "o_attributes": 0x60,
    "mainmotion_ptr": 0x64,
    "submotion_ptr": 0x68,
    "mainmotion_count": 0x6C,
    "submotion_count_ptr": 0x70,
    "file_anim_size": 0x74,
}


class RomImage:
    """Big-endian ROM image with RAM-to-ROM translation for fighter data."""

    def __init__(self, path):
        self.path = Path(path)
        self.data = self.path.read_bytes()

    def word(self, rom_offset):
        """Return the big-endian u32 at rom_offset; raise if outside the image."""
        if rom_offset < 0 or rom_offset + 4 > len(self.data):
            raise ValueError(f"{self.path.name}: ROM offset 0x{rom_offset:X} outside image")
        return struct.unpack_from(">I", self.data, rom_offset)[0]

    def ram_to_rom(self, ram):
        """Map a RAM pointer to a ROM offset, or None when it is not in a known window."""
        if VANILLA_DATA_BASE <= ram < OVERLAY_WINDOW[0]:
            return ram - VANILLA_DATA_BASE
        if OVERLAY_WINDOW[0] <= ram < OVERLAY_WINDOW[1]:
            return ram - OVERLAY_BASE
        if ram >= EXPANDED_BASE:
            return ram - EXPANDED_BASE + EXPANDED_ROM_BASE
        return None

    def overlay_ram_to_rom(self, ram):
        """Map a RAM pointer that lives in the overlay window (menu and submotion arrays)."""
        if OVERLAY_WINDOW[0] <= ram < OVERLAY_WINDOW[1]:
            return ram - OVERLAY_BASE
        return self.ram_to_rom(ram)


def signed16(value):
    """Interpret a 16-bit immediate as a signed integer."""
    return value - 0x10000 if value & 0x8000 else value


def read_action_table_ram(rom):
    """Decode the RAM address of ACTION_ARRAY_TABLE from the move_action_array_table_ hook.

    The hook is `lui t9, HI` then `lw t9, LO(t9)`. A signed LO means HI must be
    adjusted, so the address is HI + signed(LO). Decoding the ROM avoids trusting
    which of the two Remix or vanilla layouts is in use.
    """
    lui = rom.word(ACTION_TABLE_HOOK_ROM)
    lw = rom.word(ACTION_TABLE_HOOK_ROM + 0x10)
    if lui >> 26 != 0x0F or lw >> 26 != 0x23:
        raise ValueError(f"{rom.path.name}: hook at 0x{ACTION_TABLE_HOOK_ROM:X} is not lui/lw")
    return ((lui & 0xFFFF) << 16) + signed16(lw & 0xFFFF)


def read_fighter_table_entries(rom):
    """Read the struct pointer table until its NULL terminator."""
    entries = []
    for index in range(MAX_FIGHTERS):
        pointer = rom.word(STRUCT_TABLE_ROM + index * 4)
        if pointer == 0:
            return entries
        entries.append(pointer)
    raise ValueError(f"{rom.path.name}: no NULL terminator in struct table within {MAX_FIGHTERS} entries")


def read_ft_struct(rom, struct_ram):
    """Parse the 0x78-byte FTData struct at struct_ram (decomp struct FTData layout)."""
    struct_rom = rom.ram_to_rom(struct_ram)
    if struct_rom is None:
        raise ValueError(f"struct RAM 0x{struct_ram:08X} is not in a known window")
    words = [rom.word(struct_rom + off) for off in range(0, FT_DATA_SIZE, 4)]
    return {
        "struct_ram": f"0x{struct_ram:08X}",
        "struct_rom": f"0x{struct_rom:07X}",
        "file_ids": {
            "main": words[0x00 // 4],
            "mainmotion": words[0x04 // 4],
            "submotion": words[0x08 // 4],
            "model": words[0x0C // 4],
            "shieldpose": words[0x10 // 4],
            "special1": words[0x14 // 4],
            "special2": words[0x18 // 4],
            "special3": words[0x1C // 4],
            "special4": words[0x20 // 4],
        },
        "file_main_size": words[FIELD_OFFSETS["file_main_size"] // 4],
        "attrib_offset": words[FIELD_OFFSETS["o_attributes"] // 4],
        "file_anim_size": words[FIELD_OFFSETS["file_anim_size"] // 4],
        "mainmotion_ptr": words[FIELD_OFFSETS["mainmotion_ptr"] // 4],
        "submotion_ptr": words[FIELD_OFFSETS["submotion_ptr"] // 4],
        "mainmotion_count": words[FIELD_OFFSETS["mainmotion_count"] // 4],
        "submotion_count_ptr": words[FIELD_OFFSETS["submotion_count_ptr"] // 4],
        "raw_words": [f"0x{w:08X}" for w in words],
    }


def read_motion_entries(rom, array_ram, count):
    """Read `count` FTMotionDesc entries (anim_file_id, command offset, flags) at array_ram."""
    array_rom = rom.overlay_ram_to_rom(array_ram)
    if array_rom is None:
        return None
    entries = []
    for index in range(count):
        base = array_rom + index * MOTION_ENTRY_SIZE
        entries.append({
            "anim_file_id": rom.word(base),
            "command_offset": f"0x{rom.word(base + 4):08X}",
            "flags": f"0x{rom.word(base + 8):08X}",
        })
    return entries


def read_action_entries(rom, array_ram, count):
    """Read `count` 0x14-byte action entries; keep raw words, the layout is not decoded here."""
    array_rom = rom.ram_to_rom(array_ram)
    if array_rom is None:
        return None
    entries = []
    for index in range(count):
        base = array_rom + index * ACTION_ENTRY_SIZE
        words = [rom.word(base + off) for off in range(0, ACTION_ENTRY_SIZE, 4)]
        entries.append({
            "param_staling_word": f"0x{words[0] >> 16:04X}",
            "unknown_halfword": f"0x{words[0] & 0xFFFF:04X}",
            "asm": [f"0x{w:08X}" for w in words[1:]],
        })
    return entries


def inferred_action_counts(action_pointers):
    """Infer each distinct action array's length from the gap to the next array in the same window.

    Arrays in the same window are assumed packed back to back, which holds for the
    vanilla layout. The last array in a window has no successor and gets None.
    """
    distinct = sorted(set(p for p in action_pointers if p != 0))
    counts = {}
    for index, pointer in enumerate(distinct):
        next_pointer = distinct[index + 1] if index + 1 < len(distinct) else None
        same_window = next_pointer is not None and (next_pointer >= EXPANDED_BASE) == (pointer >= EXPANDED_BASE)
        counts[pointer] = (next_pointer - pointer) // ACTION_ENTRY_SIZE if same_window else None
    return counts


def dump_fighter(rom, index, struct_ram, action_table_ram, action_counts):
    """Build the JSON record for one fighter table index."""
    record = {"id": index, **read_ft_struct(rom, struct_ram)}
    record["mainmotion_entries"] = read_motion_entries(rom, record["mainmotion_ptr"], record["mainmotion_count"])
    record["menu_entries"] = read_menu(rom, record)
    record["action_array"] = read_action_array(rom, index, action_table_ram, action_counts)
    return record


def read_menu(rom, record):
    """Read the menu/submotion array; its count comes from the pointer at struct offset 0x70."""
    count_rom = rom.overlay_ram_to_rom(record["submotion_count_ptr"])
    if count_rom is None:
        return {"status": "unmapped_count_pointer"}
    count = rom.word(count_rom)
    if count > MAX_FIGHTERS:
        return {"status": "implausible_count", "count": count}
    span = record["submotion_count_ptr"] - record["submotion_ptr"]
    menu = {
        "ram": f"0x{record['submotion_ptr']:08X}",
        "count": count,
        "consistent_with_count_pointer": span == count * MOTION_ENTRY_SIZE,
        "entries": read_motion_entries(rom, record["submotion_ptr"], count),
    }
    return menu


def read_action_array(rom, index, action_table_ram, action_counts):
    """Read the fighter's action array, found through ACTION_ARRAY_TABLE[index]."""
    table_rom = rom.ram_to_rom(action_table_ram)
    pointer = rom.word(table_rom + index * 4)
    if pointer == 0:
        return {"ram": "0x00000000", "count": 0, "count_method": "none", "entries": []}
    count = action_counts.get(pointer)
    array = {"ram": f"0x{pointer:08X}", "count": count, "count_method": "inferred_gap_unverified"}
    if count is None:
        array["entries"] = None
        return array
    array["entries"] = read_action_entries(rom, pointer, count)
    return array


def dump_rom(rom):
    """Return the full fighter table dump for one ROM image."""
    pointers = read_fighter_table_entries(rom)
    action_table_ram = read_action_table_ram(rom)
    action_table_rom = rom.ram_to_rom(action_table_ram)
    table_pointers = [rom.word(action_table_rom + i * 4) for i in range(len(pointers))]
    action_counts = inferred_action_counts(table_pointers)
    fighters = [dump_fighter(rom, i, p, action_table_ram, action_counts) for i, p in enumerate(pointers)]
    return {
        "source": rom.path.name,
        "struct_table": {"rom": f"0x{STRUCT_TABLE_ROM:X}", "ram": f"0x{STRUCT_TABLE_RAM:08X}", "count": len(pointers)},
        "action_table": {"ram": f"0x{action_table_ram:08X}", "rom": f"0x{action_table_rom:07X}"},
        "fighters": fighters,
    }


def parse_reloc_defines(path):
    """Map reloc_data symbol names to their integer values from the generated header."""
    text = path.read_text(encoding="utf-8", errors="replace")
    defines = {}
    for name, value in re.findall(r"#define\s+(\w+)\s+\(\(intptr_t\)(0x[0-9a-fA-F]+)\)", text):
        defines[name] = int(value, 16)
    return defines


def evaluate_token(token, defines):
    """Return the integer value of a decomp initializer token, or None if it is not a literal or known symbol."""
    if re.fullmatch(r"0x[0-9a-fA-F]+|\d+", token):
        return int(token, 0)
    return defines.get(token)


def strip_comments(text):
    """Remove block and line comments from C source text."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    return "\n".join(line.split("//")[0] for line in text.split("\n"))


def port_branch_lines(body):
    """Keep only lines active when PORT is defined, dropping #ifndef PORT branches."""
    kept = []
    stack = []
    for line in body.split("\n"):
        stripped = line.strip()
        if stripped == "#ifdef PORT":
            stack.append(True)
        elif stripped == "#ifndef PORT":
            stack.append(False)
        elif stripped == "#if defined(REGION_US)":
            stack.append(True)
        elif stripped.startswith("#if"):
            raise ValueError(f"unsupported conditional in initializer: {stripped!r}")
        elif stripped == "#else":
            stack[-1] = not stack[-1]
        elif stripped == "#endif":
            stack.pop()
        elif not stripped.startswith("#") and all(stack):
            kept.append(line)
    return kept


def brace_body(text, open_index):
    """Return the text between the brace at open_index and its matching close brace."""
    depth = 0
    for position in range(open_index, len(text)):
        if text[position] == "{":
            depth += 1
        elif text[position] == "}":
            depth -= 1
            if depth == 0:
                return text[open_index + 1:position]
    raise ValueError(f"unbalanced braces starting at offset {open_index}")


def initializer_tokens(text, opening_pattern):
    """Split the PORT-branch initializer that follows opening_pattern into comma-separated tokens."""
    match = re.search(opening_pattern, text)
    if match is None:
        return None
    body = brace_body(text, match.end() - 1)
    joined = "\n".join(port_branch_lines(strip_comments(body)))
    return [token.strip() for token in joined.split(",") if token.strip()]


def load_decomp_sources(root):
    """Return the text of every decomp C file, keyed by path."""
    return {path: path.read_text(encoding="utf-8", errors="replace") for path in (root / "decomp" / "src").rglob("*.c")}


def decomp_fighter_symbols(ftdata_text):
    """Return the decomp FTData symbol for each fkind index, from dFTManagerDataFiles."""
    match = re.search(r"FTData\s*\*\s*dFTManagerDataFiles\s*\[[^\]]*\]\s*=\s*\{", ftdata_text)
    body = brace_body(ftdata_text, match.end() - 1)
    names = re.findall(r"&(\w+)", strip_comments(body))
    return names


def find_motion_array(sources, name):
    """Return the PORT-branch token list for the FTMotionDesc array called name, or None."""
    pattern = r"FTMotionDesc\s+%s\s*\[\s*\]\s*=\s*\{" % re.escape(name)
    for text in sources.values():
        tokens = initializer_tokens(text, pattern)
        if tokens is not None:
            return tokens
    return None


def motion_tokens_to_entries(tokens, defines):
    """Group decomp motion tokens into (anim_file_id, command_offset, flags) triples, None where unresolved."""
    entries = []
    for start in range(0, len(tokens) - 2, 3):
        anim, offset, flags = tokens[start:start + 3]
        entries.append({
            "anim_file_id": evaluate_token(anim, defines),
            "command_offset": evaluate_token(offset, defines),
            "flags": evaluate_token(flags, defines),
        })
    return entries


def compare_motion_entries(rom_entries, decomp_entries):
    """Compare anim id and flags of ROM and decomp motion entries; offsets are skipped (symbolic in decomp)."""
    mismatches = []
    for index, (rom_entry, decomp_entry) in enumerate(zip(rom_entries, decomp_entries)):
        for field in ("anim_file_id", "flags"):
            rom_value = rom_entry[field] if field == "anim_file_id" else int(rom_entry[field], 16)
            if decomp_entry[field] is not None and decomp_entry[field] != rom_value:
                mismatches.append({"index": index, "field": field, "rom": rom_value, "decomp": decomp_entry[field]})
    return mismatches


def validate_vanilla_against_decomp(vanilla, defines, sources, ftdata_text):
    """Compare vanilla ROM FTData for fkinds 0..26 with the decomp initializers."""
    symbols = decomp_fighter_symbols(ftdata_text)
    results = []
    for index in range(VANILLA_FIGHTER_COUNT):
        symbol = symbols[index]
        tokens = initializer_tokens(ftdata_text, r"FTData\s+%s\s*=\s*\{" % re.escape(symbol))
        fighter = vanilla["fighters"][index]
        results.append(compare_fighter_fields(index, symbol, tokens, fighter, defines, sources))
    return results


def compare_fighter_fields(index, symbol, tokens, fighter, defines, sources):
    """Compare one fighter's scalar FTData fields and its motion arrays with decomp tokens."""
    checks = []
    expected = {
        **{f"file_{name}": evaluate_token(tokens[i], defines) for i, name in enumerate(
            ["main", "mainmotion", "submotion", "model", "shieldpose", "special1", "special2", "special3", "special4"])},
        "file_main_size": evaluate_token(tokens[9], defines),
        "attrib_offset": evaluate_token(tokens[24], defines),
        "mainmotion_count": evaluate_token(tokens[27], defines),
        "file_anim_size": evaluate_token(tokens[29], defines),
    }
    actual = {
        **{f"file_{name}": value for name, value in fighter["file_ids"].items()},
        "file_main_size": fighter["file_main_size"],
        "attrib_offset": fighter["attrib_offset"],
        "mainmotion_count": fighter["mainmotion_count"],
        "file_anim_size": fighter["file_anim_size"],
    }
    for field, decomp_value in expected.items():
        checks.append({"field": field, "decomp": decomp_value, "rom": actual[field], "match": decomp_value == actual[field]})
    motion = {"mainmotion": motion_check(tokens[25], fighter["mainmotion_entries"], defines, sources),
              "menu": motion_check(tokens[26], (fighter["menu_entries"] or {}).get("entries") or [], defines, sources)}
    return {"fkind": index, "decomp_symbol": symbol, "field_checks": checks, "motion_checks": motion}


def motion_check(pointer_token, rom_entries, defines, sources):
    """Compare a motion array named inside a pointer token (e.g. &dFTMarioMotionDescs) with the ROM entries."""
    name_match = re.search(r"&(\w+)", pointer_token)
    if name_match is None:
        return {"status": "no_array_name"}
    tokens = find_motion_array(sources, name_match.group(1))
    if tokens is None:
        return {"status": "decomp_array_not_found", "name": name_match.group(1)}
    decomp_entries = motion_tokens_to_entries(tokens, defines)
    return {
        "name": name_match.group(1),
        "decomp_count": len(decomp_entries),
        "rom_count": len(rom_entries),
        "count_match": len(decomp_entries) == len(rom_entries),
        "mismatches": compare_motion_entries(rom_entries, decomp_entries),
    }


def diff_remix_against_vanilla(remix, vanilla):
    """List the fields that differ between Remix and vanilla for each fkind present in both tables."""
    diffs = []
    for index in range(VANILLA_FIGHTER_COUNT):
        remix_fighter = remix["fighters"][index]
        vanilla_fighter = vanilla["fighters"][index]
        changed = []
        for field in ("file_ids", "file_main_size", "attrib_offset", "file_anim_size", "mainmotion_count"):
            if remix_fighter[field] != vanilla_fighter[field]:
                changed.append(field)
        for field in ("mainmotion_ptr", "submotion_ptr", "submotion_count_ptr"):
            if remix_fighter[field] != vanilla_fighter[field]:
                changed.append(field + " (pointer)")
        if remix_fighter["mainmotion_entries"] != vanilla_fighter["mainmotion_entries"]:
            changed.append("mainmotion_entries")
        if remix_fighter["menu_entries"] != vanilla_fighter["menu_entries"]:
            changed.append("menu_entries")
        if remix_fighter["action_array"]["entries"] != vanilla_fighter["action_array"]["entries"]:
            changed.append("action_array")
        diffs.append({"fkind": index, "changed_fields": changed, "identical": not changed})
    return diffs


def build_validation(remix, vanilla, defines, sources, ftdata_text):
    """Assemble the validation payload: decomp comparison plus Remix-vs-vanilla diffs."""
    decomp = validate_vanilla_against_decomp(vanilla, defines, sources, ftdata_text)
    return {
        "decomp_source": str(DECOMP_FTDATA.relative_to(REPO_ROOT)),
        "defines_source": str(RELOC_HEADER.relative_to(REPO_ROOT)),
        "vanilla_vs_decomp": decomp,
        "remix_vs_vanilla": diff_remix_against_vanilla(remix, vanilla),
    }


def write_json(path, payload):
    """Write payload as indented JSON, creating the parent directory."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=1) + "\n", encoding="utf-8")


def main(argv):
    """Parse arguments, dump both ROMs and write the JSON outputs."""
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--remix", type=Path, default=DEFAULT_REMIX)
    parser.add_argument("--vanilla", type=Path, default=DEFAULT_VANILLA)
    args = parser.parse_args(argv)
    remix = dump_rom(RomImage(args.remix))
    vanilla = dump_rom(RomImage(args.vanilla))
    write_json(args.out / "remix_fighters.json", remix)
    write_json(args.out / "vanilla_fighters.json", vanilla)
    validation = build_validation(
        remix,
        vanilla,
        parse_reloc_defines(RELOC_HEADER),
        load_decomp_sources(REPO_ROOT),
        DECOMP_FTDATA.read_text(encoding="utf-8", errors="replace"),
    )
    write_json(args.out / "validation.json", validation)
    print(f"remix: {len(remix['fighters'])} fighters -> {args.out / 'remix_fighters.json'}")
    print(f"vanilla: {len(vanilla['fighters'])} fighters -> {args.out / 'vanilla_fighters.json'}")
    print(f"validation -> {args.out / 'validation.json'}")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
