#!/usr/bin/env python3
"""Disassembles MIPS code from the base ROM or the assembled Remix ROM.

Remix moves often splice vanilla code with OS.copy_segment(rom_offset, size);
this prints those bytes as instructions, with jal targets named from
build/remixasm/refs/vanilla_funcs.txt.

Usage:
  tools/remix_disasm_rom.py ROM_OFFSET SIZE [--vram ADDR] [--remix]

ROM_OFFSET and SIZE accept hex (0x...). --vram defaults to ROM_OFFSET +
0x80084800, the mapping of the fighter code segment (OS.patch_start pairs
such as 0x63D2C -> 0x800E852C); pass it explicitly for other segments.
--remix reads build/remixasm/remix.z64 (Remix code at ROM 0x2C00000 maps to
0x80400000) instead of the base ROM.

Run with .toolchain/host/venv/bin/python (scripts/remix/setup-remix-tools.sh
installs rabbitizer).
"""
import argparse
import pathlib
import re
import struct
import sys

import rabbitizer

ROOT = pathlib.Path(__file__).resolve().parent.parent
BASE_ROM = ROOT / "baserom.us.z64"
REMIX_ROM = ROOT / "build" / "remixasm" / "remix.z64"
VANILLA_FUNCS = ROOT / "build" / "remixasm" / "refs" / "vanilla_funcs.txt"
FIGHTER_SEGMENT_DELTA = 0x80084800
REMIX_ROM_BASE = 0x2C00000
REMIX_RAM_BASE = 0x80400000


def load_function_names() -> dict[int, str]:
    """Maps vanilla function addresses to decomp names."""
    names: dict[int, str] = {}
    if not VANILLA_FUNCS.exists():
        return names
    for line in VANILLA_FUNCS.read_text().splitlines():
        parts = line.split()
        if len(parts) >= 2 and parts[0].startswith("0x"):
            names[int(parts[0], 16)] = parts[1]
    return names


def default_vram(rom_offset: int, is_remix: bool) -> int:
    """Guesses the RAM address of `rom_offset` (see module docstring)."""
    if is_remix and rom_offset >= REMIX_ROM_BASE:
        return rom_offset - REMIX_ROM_BASE + REMIX_RAM_BASE
    return rom_offset + FIGHTER_SEGMENT_DELTA


def disassemble(rom: bytes, rom_offset: int, size: int, vram: int, names: dict[int, str]) -> list[str]:
    """Returns one formatted line per instruction."""
    lines = []
    for i in range(0, size, 4):
        word = struct.unpack(">I", rom[rom_offset + i:rom_offset + i + 4])[0]
        instr = rabbitizer.Instruction(word, vram + i)
        text = instr.disassemble()
        target = re.search(r"jal\s+(?:func_)?0x([0-9A-Fa-f]+)|jal\s+func_([0-9A-Fa-f]+)", text)
        note = ""
        if instr.isJumpWithAddress():
            address = instr.getInstrIndexAsVram()
            note = f"  ; -> {names.get(address, hex(address))}"
        elif target is None and instr.isBranch():
            note = f"  ; -> 0x{instr.getBranchVramGeneric():08X}"
        lines.append(f"0x{rom_offset + i:07X} 0x{vram + i:08X}  {word:08X}  {text}{note}")
    return lines


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("rom_offset", type=lambda v: int(v, 0))
    parser.add_argument("size", type=lambda v: int(v, 0))
    parser.add_argument("--vram", type=lambda v: int(v, 0))
    parser.add_argument("--remix", action="store_true")
    args = parser.parse_args()

    rom_path = REMIX_ROM if args.remix else BASE_ROM
    if not rom_path.exists():
        print(f"ROM not found: {rom_path}", file=sys.stderr)
        return 1
    if args.size <= 0 or args.size % 4 != 0:
        print(f"SIZE must be a positive multiple of 4, got {args.size:#x}", file=sys.stderr)
        return 1
    rom = rom_path.read_bytes()
    if args.rom_offset + args.size > len(rom):
        print(f"range {args.rom_offset:#x}+{args.size:#x} is past the ROM end {len(rom):#x}", file=sys.stderr)
        return 1
    vram = args.vram if args.vram is not None else default_vram(args.rom_offset, args.remix)
    print("\n".join(disassemble(rom, args.rom_offset, args.size, vram, load_function_names())))
    return 0


if __name__ == "__main__":
    sys.exit(main())
