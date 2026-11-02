#!/usr/bin/env python3
"""Check the menu_zoom column of port/remix/remix_roster_data.h against the Remix ROM.

Character.menu_zoom.table (the CSS and results panel scale of every character id) is read from the
code image of baserom.remix.z64 and compared, id by id, with the generated roster header. The table
address comes from the symbol map written by scripts/remix/assemble-remix.sh.

Stdlib only. Exit status is 0 when all 97 ids match.

Usage: python3 tools/test_remix_menu_zoom.py [--rom PATH] [--sym PATH] [--header PATH]
"""

import argparse
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_ROM = REPO_ROOT / "baserom.remix.z64"
DEFAULT_SYM = REPO_ROOT / "build" / "remixasm" / "remix.sym"
DEFAULT_HEADER = REPO_ROOT / "port" / "remix" / "remix_roster_data.h"
TABLE_SYMBOL = "Character.menu_zoom.table"
CODE_IMAGE_ROM_BASE = 0x2C00000
CODE_IMAGE_RAM_BASE = 0x80400000
ROSTER_SIZE = 0x61
ROSTER_ROW = re.compile(r'^\s*\{0x([0-9A-F]{2}), "(\w+)",.*, ([0-9.]+)f\},')


def table_rom_offset(sym_path):
    for line in sym_path.read_text(encoding="utf-8").splitlines():
        address, _, name = line.partition(" ")
        if name == TABLE_SYMBOL:
            return CODE_IMAGE_ROM_BASE + int(address, 16) - CODE_IMAGE_RAM_BASE
    raise SystemExit(f"{sym_path}: symbol {TABLE_SYMBOL} not found")


def header_zooms(header_path):
    zooms = {}
    for line in header_path.read_text(encoding="utf-8").splitlines():
        match = ROSTER_ROW.match(line)
        if match:
            zooms[int(match.group(1), 16)] = (match.group(2), float(match.group(3)))
    return zooms


def rom_zooms(rom_path, offset):
    with rom_path.open("rb") as rom:
        rom.seek(offset)
        words = rom.read(ROSTER_SIZE * 4)
    return struct.unpack(f">{ROSTER_SIZE}f", words)


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--sym", type=Path, default=DEFAULT_SYM)
    parser.add_argument("--header", type=Path, default=DEFAULT_HEADER)
    args = parser.parse_args()

    expected = rom_zooms(args.rom, table_rom_offset(args.sym))
    actual = header_zooms(args.header)
    if sorted(actual) != list(range(ROSTER_SIZE)):
        raise SystemExit(f"{args.header}: expected {ROSTER_SIZE} roster rows with a menu_zoom, got {len(actual)}")

    mismatches = [
        f"0x{remix_id:02X} {name}: header {zoom} != ROM {expected[remix_id]:.6g}"
        for remix_id, (name, zoom) in sorted(actual.items())
        if abs(zoom - expected[remix_id]) > 1e-6
    ]
    for mismatch in mismatches:
        print(mismatch)
    print(f"{ROSTER_SIZE - len(mismatches)}/{ROSTER_SIZE} menu_zoom values match the ROM")
    return 1 if mismatches else 0


if __name__ == "__main__":
    sys.exit(main())
