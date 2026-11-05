#!/usr/bin/env python3
"""Check that every sprite offset the Remix CSS uses points at a Sprite header in the extracted .o2r.

The offsets of port/remix/remix_css_data.h (portraits in file 0xA05, names in 0x11, series logos in 0x14) and the CSS image
offsets of the VS character select (bookends, flash, D-pad, flags and variant icons in file 0xA06, including the `+ 0x10`
entries of CharacterSelect.asm) are read from the .o2r and each must hold a Sprite header: width, height and texel scale above zero. The reloc files carry an 0x50 byte header in the .o2r, so the Sprite sits at offset + 0x50.

Stdlib only. Exit status is 0 when every offset checks out. Nothing from the ROM is written anywhere.

Usage: python3 tools/test_remix_css_offsets.py [--o2r PATH] [--header PATH]
"""

import argparse
import re
import struct
import sys
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_O2R = REPO_ROOT / "BattleShip-Remix.o2r"
DEFAULT_HEADER = REPO_ROOT / "port" / "remix" / "remix_css_data.h"
RELOC_HEADER_SIZE = 0x50
TABLE_FILES = {
    "kRemixCssPortraitOffset": "reloc_misc/file_2565",
    "kRemixCssNameOffset": "reloc_menus/MNPlayersCommon",
    "kRemixCssLogoOffset": "reloc_fighters_common/FTEmblemSprites",
}
IMAGES_FILE = "reloc_misc/file_2566"
IMAGE_OFFSETS = {
    "bonus bookend": 0x3B28,
    "bonus bookend flash": 0x3B28 + 0x0C60,
    "random bookend": 0x53E8,
    "D-pad": 0x0218,
    "J flag": 0x0558,
    "E flag": 0x03B8,
    "polygon icon": 0x13A8,
    "Metal icon": 0x2448 + 0x10,
    "Piano icon": 0x2528 + 0x10,
    "Giga Bowser icon": 0x2608 + 0x10,
    "Super Sonic icon": 0x26E8 + 0x10,
    "Master Hand icon": 0x27F0 + 0x10,
    "Peppy icon": 0x28D8 + 0x10,
    "Slippy icon": 0x29B0 + 0x10,
    "Metal Luigi icon": 0x2A88 + 0x10,
    "Ebi icon": 0x2B60 + 0x10,
    "Dragon King icon": 0x2C38 + 0x10,
    "Dark Samus icon": 0x2D10 + 0x10,
    "Lucas icon": 0x2DE8 + 0x10,
    "Roy icon": 0x2EC0 + 0x10,
}
ENTRY = re.compile(r"(0x[0-9A-Fa-f]+),\s*/\*\s*(0x[0-9A-Fa-f]+)\s+(\w+)")


def table_entries(header_text, name):
    match = re.search(name + r"\[REMIX_CSS_NUM_CHARACTERS\] = \{(.*?)\};", header_text, re.S)
    if match is None:
        raise SystemExit(f"table {name} not found in the header")
    return [(int(value, 16), ident, fighter) for value, ident, fighter in ENTRY.findall(match.group(1))]


def sprite_problem(data, offset):
    start = offset + RELOC_HEADER_SIZE
    if start + 16 > len(data):
        return "past the end of the file"
    width, height = struct.unpack(">hh", data[start + 4:start + 8])
    scale_x, scale_y = struct.unpack(">ff", data[start + 8:start + 16])
    if width <= 0 or height <= 0 or scale_x <= 0.0 or scale_y <= 0.0:
        return f"not a Sprite header (width {width}, height {height}, scale {scale_x} {scale_y})"
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--o2r", type=Path, default=DEFAULT_O2R)
    parser.add_argument("--header", type=Path, default=DEFAULT_HEADER)
    args = parser.parse_args()
    header_text = args.header.read_text(encoding="utf-8")
    failures = 0
    checked = 0
    with zipfile.ZipFile(args.o2r) as archive:
        for table, file_name in TABLE_FILES.items():
            data = archive.read(file_name)
            for offset, ident, fighter in table_entries(header_text, table):
                if offset == 0:
                    continue
                checked += 1
                problem = sprite_problem(data, offset)
                if problem:
                    failures += 1
                    print(f"FAIL {table} {ident} {fighter} offset 0x{offset:X} in {file_name}: {problem}")
        images = archive.read(IMAGES_FILE)
        for label, offset in IMAGE_OFFSETS.items():
            checked += 1
            problem = sprite_problem(images, offset)
            if problem:
                failures += 1
                print(f"FAIL {label} offset 0x{offset:X} in {IMAGES_FILE}: {problem}")
    print(f"{checked} offsets checked, {failures} failed")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
