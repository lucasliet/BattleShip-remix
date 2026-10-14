#!/usr/bin/env python3
"""Generate the Smash Remix action-binding tables from the assembled Remix build.

Inputs:
  build/remixasm/remix.sym            bass -sym output of the Remix 2.0.1 source
                                      (scripts/remix/assemble-remix.sh)
  baserom.remix.z64                   user's Remix ROM (action arrays are read from it)
  build/remixasm/refs/vanilla_funcs.txt  tools/remix_port_refs.py

Outputs (addresses and names only; no ROM bytes):
  port/remix/remix_action_layout.h          table addresses + per-id action counts
  decomp/src/ft/ftremix/ftremixvanillaprocs.c  vanilla address -> decomp function table
  port/remix/remix_symbols.h                REMIX_SYM_* for every routine the tables reference
                                            plus every label in the CSS fighters' scopes
  build/remixasm/worklists/*.md             per-fighter list of Remix routines to port

Usage:
  .toolchain/host/venv/bin/python tools/gen_remix_actions.py
"""

import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SYM_PATH = ROOT / "build/remixasm/remix.sym"
ROM_PATH = ROOT / "baserom.remix.z64"
VANILLA_PATH = ROOT / "build/remixasm/refs/vanilla_funcs.txt"
REMIX_SRC = ROOT / ".toolchain/pkgs/smashremix-src/src"

IMAGE_RAM = 0x80400000
IMAGE_ROM = 0x02C00000
IMAGE_SIZE = 0x400000
ID_COUNT = 97
FIRST_SYNTH_ID = 0x1D
ACTION_ENTRY_SIZE = 0x14
SPECIAL_TABLES = ["ground_nsp", "ground_usp", "ground_dsp", "air_nsp", "air_usp", "air_dsp"]
EXTRA_SYMBOLS = [
    "Stages.custom_item_spawn_rate_table",
    "Stages.series_logo_table",
    "Stages.stage_file_table",
    "Stages.function_table",
    "Stages.class_table",
    "Stages.string_table",
    "Stages.default_music_track_table",
    "Stages.alternate_music_table",
    "Stages.background_table",
    "Stages.stage_hazard_table",
    "Stages.zoom_table",
    "Stages.position_table",
    "Stages.icon_offset_table",
    "Stages.variant_table",
    "Stages.stage_table_normal",
    "Stages.stage_table_tournament",
    "Stages.stage_table_pages",
    "Stages.random_table",
    "Stages.bg_type",
]
EXTRA_SYMBOL_PREFIXES = ["Hazards."]
CSS_FIGHTER_DIRS = ["Falco", "Ganondorf", "YoungLink", "DrMario", "Wario", "Dedede", "Sonic", "Sheik", "Marina",
                    "Goemon", "Crash", "Peach", "Bowser", "Wolf", "Conker", "Mewtwo", "Marth", "Banjo"]


def load_symbols() -> dict:
    symbols = {}
    for line in SYM_PATH.read_text().splitlines():
        parts = line.split(None, 1)
        if len(parts) == 2:
            symbols[parts[1].strip()] = int(parts[0], 16)
    return symbols


# Functions whose `// 0x...` comment in the decomp is wrong or truncated (the
# neighbouring functions' addresses pin the real start).
VANILLA_ADDRESS_FIXES = {
    0x8015D4C0: "ftSamusSpecialAirNStartProcMap",
    0x8015E79C: "ftYoshiSpecialAirNSetStatus",
    0x8015F2E8: "ftKirbyCopyYoshiSpecialAirNReleaseProcUpdate",
    0x8016284C: "ftKirbySpecialNTurnProcMap",
}


def load_vanilla() -> dict:
    vanilla = {}
    for line in VANILLA_PATH.read_text().splitlines():
        addr, name, _ = line.split()
        vanilla.setdefault(int(addr, 16), name)
    vanilla.update(VANILLA_ADDRESS_FIXES)
    return vanilla


def definition_return_type(lines: list, name: str, hint: int) -> str:
    """Return type of `name`; the hinted line goes stale when the decomp file is edited."""
    pattern = re.compile(rf"^(?!\s)([\w\s\*]*?)\b{re.escape(name)}\s*\(")
    if hint < len(lines):
        match = pattern.match(lines[hint])
        if match:
            return match.group(1).strip() or "void"
    for line in lines:
        match = pattern.match(line)
        if match and not line.rstrip().endswith(";"):
            return match.group(1).strip() or "void"
    raise SystemExit(f"no definition of {name} found; refresh vanilla_funcs.txt")


def vanilla_return_types() -> dict:
    """Function name -> return type, read from the definition line vanilla_funcs.txt points at."""
    types = {}
    for line in VANILLA_PATH.read_text().splitlines():
        _, name, location = line.split()
        path, _, number = location.rpartition(":")
        lines = (ROOT / path).read_text(errors="replace").splitlines()
        types[name] = definition_return_type(lines, name, int(number) - 1)
    return types


class RemixImage:
    """Big-endian reads from the Remix code image inside the ROM."""

    def __init__(self, rom: bytes):
        self.rom = rom

    def word(self, ram: int) -> int:
        if not IMAGE_RAM <= ram < IMAGE_RAM + IMAGE_SIZE:
            raise ValueError(f"0x{ram:08X} is outside the Remix image 0x{IMAGE_RAM:08X}+0x{IMAGE_SIZE:X}")
        at = ram - IMAGE_RAM + IMAGE_ROM
        return struct.unpack(">I", self.rom[at:at + 4])[0]


def c_name(label: str) -> str:
    return "REMIX_SYM_" + re.sub(r"[^A-Za-z0-9_]", "_", label.replace(".", "__"))


def action_counts(image: RemixImage, symbols: dict) -> dict:
    """remix id -> (array address, entry count) using the label after each action array."""
    by_addr = {addr: name for name, addr in symbols.items()}
    sorted_addrs = sorted(set(symbols.values()))
    table = symbols["Character.ACTION_ARRAY_TABLE"]
    counts = {}
    for rid in range(FIRST_SYNTH_ID, ID_COUNT):
        array = image.word(table + rid * 4)
        label = by_addr.get(array, "")
        if not label.endswith("_action_array"):
            continue
        following = next(a for a in sorted_addrs if a > array)
        counts[rid] = (array, (following - array) // ACTION_ENTRY_SIZE, label[len("Character."):-len("_action_array")])
    return counts


def referenced_procs(image: RemixImage, symbols: dict, counts: dict) -> dict:
    """address -> set of remix ids that reference it from action arrays or special tables."""
    refs = {}
    for rid, (array, count, _) in counts.items():
        for i in range(count):
            for k in range(4):
                addr = image.word(array + i * ACTION_ENTRY_SIZE + 4 + 4 * k)
                if addr:
                    refs.setdefault(addr, set()).add(rid)
        for table in SPECIAL_TABLES:
            addr = image.word(symbols[f"Character.{table}.table"] + rid * 4)
            if addr:
                refs.setdefault(addr, set()).add(rid)
    return refs


def write_layout(symbols: dict, counts: dict) -> None:
    lines = ["/* Generated by tools/gen_remix_actions.py from the Remix 2.0.1 build; do not edit. */",
             "#pragma once", "", "#include <stdint.h>", "",
             f"#define REMIX_ACTION_ARRAY_TABLE 0x{symbols['Character.ACTION_ARRAY_TABLE']:08X}u",
             f"#define REMIX_ACTION_ENTRY_SIZE 0x{ACTION_ENTRY_SIZE:X}u"]
    for table in SPECIAL_TABLES:
        lines.append(f"#define REMIX_SPECIAL_TABLE_{table.upper()} 0x{symbols[f'Character.{table}.table']:08X}u")
    lines += ["", f"#define REMIX_ACTION_ID_COUNT {ID_COUNT}", "",
              "/* Entries in each fighter's action array (statuses 0xDC + i); 0 = not a defined fighter. */",
              f"static const uint8_t kRemixActionCounts[REMIX_ACTION_ID_COUNT] = {{"]
    for rid in range(ID_COUNT):
        array, count, name = counts.get(rid, (0, 0, ""))
        lines.append(f"    {count},{f'  /* 0x{rid:02X} {name} @ 0x{array:08X} */' if name else ''}")
    lines.append("};")
    (ROOT / "port/remix/remix_action_layout.h").write_text("\n".join(lines) + "\n")


def write_vanilla_procs(refs: dict, vanilla: dict) -> list:
    rows = sorted((addr, vanilla[addr]) for addr in refs if addr < IMAGE_RAM and addr in vanilla)
    missing = sorted(addr for addr in refs if addr < IMAGE_RAM and addr not in vanilla)
    lines = ["/* Generated by tools/gen_remix_actions.py from the Remix 2.0.1 build; do not edit.",
             " * Vanilla routines that Remix action arrays and special tables point at. */", "",
             "#ifdef PORT", "", '#include "ftremix.h"', ""]
    return_types = vanilla_return_types()
    lines += [f"extern {return_types.get(name, 'void')} {name}(GObj *);" for _, name in rows]
    lines += ["", "/* Sorted by address for ftRemixFindProc. Non-void routines are used as procs",
              " * by Remix (the N64 ignores the return value), hence the casts. */",
              "const FTRemixProc dFTRemixVanillaProcs[] =", "{"]
    lines += [f"    {{ 0x{addr:08X}u, {name if return_types.get(name, 'void') == 'void' else f'(FTRemixProcFn){name}'} }},"
              for addr, name in rows]
    lines += ["};", "", "const s32 dFTRemixVanillaProcsNum = ARRAY_COUNT(dFTRemixVanillaProcs);", "", "#endif", ""]
    out = ROOT / "decomp/src/ft/ftremix/ftremixvanillaprocs.c"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(lines))
    return missing


def write_demo_scripts() -> int:
    """Vanilla overlay motion scripts (D_ovl1_8039XXXX) that Remix menu rows point at."""
    pattern = re.compile(r"^s32 (D_ovl1_(8039[0-9A-F]{4}))\[\]")
    names = []
    for path in sorted((ROOT / "decomp/src/sc/scsubsys").glob("*.c")):
        for line in path.read_text(errors="replace").splitlines():
            match = pattern.match(line)
            if match:
                names.append((int(match.group(2), 16), match.group(1)))
    names = sorted(set(names))
    lines = ["/* Generated by tools/gen_remix_actions.py; do not edit.",
             " * Vanilla demo/menu motion scripts by N64 address: Remix menu rows copied from a",
             " * parent store these raw overlay addresses. */", "", "#ifdef PORT", "", '#include "ftremix.h"', ""]
    lines += [f"extern s32 {name}[];" for _, name in names]
    lines += ["", "/* Sorted by address. */", "const FTRemixScript dFTRemixDemoScripts[] =", "{"]
    lines += [f"    {{ 0x{addr:08X}u, {name} }}," for addr, name in names]
    lines += ["};", "", "const s32 dFTRemixDemoScriptsNum = ARRAY_COUNT(dFTRemixDemoScripts);", "", "#endif", ""]
    (ROOT / "decomp/src/ft/ftremix/ftremixdemoscripts.c").write_text("\n".join(lines))
    return len(names)


def fighter_scopes() -> dict:
    """top-level scope name -> (fighter dir, file, line) for the CSS fighters."""
    scopes = {}
    for directory in CSS_FIGHTER_DIRS:
        for path in sorted((REMIX_SRC / directory).glob("*.asm")):
            for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
                match = re.match(r"^scope ([A-Za-z_]\w*)", line)
                if match:
                    scopes[match.group(1)] = (directory, path.relative_to(ROOT), number)
    return scopes


def label_location(label: str, scopes: dict) -> str:
    top, _, rest = label.partition(".")
    if top not in scopes:
        return "(outside the CSS fighter dirs)"
    _, path, line = scopes[top]
    leaf = rest.split(".")[0] if rest else ""
    if leaf:
        text_lines = (ROOT / path).read_text(errors="replace").splitlines()
        for number in range(line, len(text_lines) + 1):
            if re.match(rf"^\s*scope {re.escape(leaf)}\b", text_lines[number - 1]):
                return f"{path}:{number}"
    return f"{path}:{line}"


def write_symbols(symbols: dict, refs: dict, scopes: dict) -> None:
    by_addr = {}
    for name, addr in symbols.items():
        by_addr.setdefault(addr, []).append(name)
    wanted = {name for name in symbols if name.split(".")[0] in scopes}
    wanted |= {by_addr[addr][0] for addr in refs if addr >= IMAGE_RAM and addr in by_addr}
    wanted |= {name for name in EXTRA_SYMBOLS if name in symbols}
    wanted |= {name for name in symbols if name.startswith(tuple(EXTRA_SYMBOL_PREFIXES))}
    lines = ["/* Generated by tools/gen_remix_actions.py from the Remix 2.0.1 build; do not edit.",
             " * Remix RAM addresses of the routines and data the C ports reference. */", "#pragma once", ""]
    lines += [f"#define {c_name(name)} 0x{symbols[name]:08X}u" for name in sorted(wanted)]
    (ROOT / "port/remix/remix_symbols.h").write_text("\n".join(lines) + "\n")


def write_worklists(symbols: dict, refs: dict, counts: dict, scopes: dict) -> None:
    by_addr = {}
    for name, addr in symbols.items():
        by_addr.setdefault(addr, []).append(name)
    out = ROOT / "build/remixasm/worklists"
    out.mkdir(parents=True, exist_ok=True)
    for rid, (_, _, name) in counts.items():
        routines = sorted((by_addr.get(addr, [f"0x{addr:08X}"])[0], addr) for addr, ids in refs.items()
                          if rid in ids and addr >= IMAGE_RAM)
        body = [f"# 0x{rid:02X} {name}: Remix routines referenced by its action array and special tables", ""]
        body += [f"- `{label}` 0x{addr:08X} `{c_name(label)}` — {label_location(label, scopes)}"
                 for label, addr in routines]
        (out / f"{rid:02X}_{name}.md").write_text("\n".join(body) + "\n")


def main() -> None:
    for path in (SYM_PATH, ROM_PATH, VANILLA_PATH):
        if not path.exists():
            sys.exit(f"missing input {path}; see the module docstring for how to produce it")
    symbols = load_symbols()
    image = RemixImage(ROM_PATH.read_bytes())
    counts = action_counts(image, symbols)
    refs = referenced_procs(image, symbols, counts)
    scopes = fighter_scopes()
    write_layout(symbols, counts)
    missing = write_vanilla_procs(refs, load_vanilla())
    demo_scripts = write_demo_scripts()
    write_symbols(symbols, refs, scopes)
    write_worklists(symbols, refs, counts, scopes)
    remix_refs = sum(1 for addr in refs if addr >= IMAGE_RAM)
    print(f"{len(counts)} fighters, {len(refs)} referenced routines ({remix_refs} Remix), "
          f"{len(missing)} vanilla addresses without a decomp function, {demo_scripts} demo scripts")
    for addr in missing:
        print(f"  unresolved vanilla 0x{addr:08X}")


if __name__ == "__main__":
    main()
