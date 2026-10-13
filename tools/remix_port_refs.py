#!/usr/bin/env python3
"""Generate the reference tables Remix ports translate MIPS against.

Outputs (under --out, git-ignored build dir by default):
  struct_offsets.txt  N64 byte offset -> C field path for the structs Remix
                      code touches, from clang's MIPS32 BE record layout of the
                      decomp headers (non-PORT build, so pointers are 4 bytes).
  vanilla_funcs.txt   N64 RAM address -> decomp function, from the
                      `// 0x8XXXXXXX` comment that precedes each definition.

Usage:
  .toolchain/host/venv/bin/python tools/remix_port_refs.py [--out DIR]
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
STRUCTS = ["FTStruct", "FTAttributes", "FTData", "WPStruct", "WPAttributes", "ITStruct", "ITAttributes",
           "GObj", "DObj", "SObj", "CObj", "MObj", "FTParts", "EFStruct", "GMColAnim", "FTMotionDesc",
           "FTStatusDesc", "FTThrowHitDesc", "FTAttackColl", "FTDamageColl"]
LAYOUT_LINE = re.compile(r"^\s*(\d+)(?::(\d+)-(\d+))? \| (\s*)(.*)$")
FUNC_ADDR = re.compile(r"^//\s*(0x8[0-9A-Fa-f]{7})\b")
FUNC_DEF = re.compile(r"^[A-Za-z_][\w\s\*]*?\b([A-Za-z_]\w*)\s*\([^;]*$")


def dump_layouts(work: Path) -> str:
    """Runs clang on a TU that forces every STRUCTS layout and returns the dump text."""
    tu = work / "layout_tu.c"
    body = ["#include <ft/fighter.h>", "#include <wp/weapon.h>", "#include <it/item.h>", "#include <ef/effect.h>"]
    body += [f"int sz_{name} = sizeof({name});" for name in STRUCTS]
    tu.write_text("\n".join(body) + "\n")
    cmd = ["clang", "-target", "mips-unknown-linux-gnu", "-m32", "-fsyntax-only", "-Xclang", "-fdump-record-layouts",
           f"-I{ROOT / 'decomp/include'}", f"-I{ROOT / 'decomp/src'}", "-D_LANGUAGE_C", "-DF3DEX_GBI_2", "-w", str(tu)]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        sys.exit(f"clang layout dump failed (rc={result.returncode}):\n{result.stderr[:2000]}")
    return result.stdout


def flatten_record(lines: list, start: int) -> list:
    """Returns (offset, bits, path) rows for the record whose header is lines[start]."""
    rows, stack = [], []
    for line in lines[start + 1:]:
        if "[sizeof=" in line:
            break
        match = LAYOUT_LINE.match(line)
        if not match:
            continue
        offset, bit_lo, bit_hi, indent, decl = match.groups()
        depth = len(indent) // 2
        name = decl.split()[-1]
        del stack[depth - 1:]
        stack.append(name)
        bits = f":{bit_lo}-{bit_hi}" if bit_lo else ""
        rows.append((int(offset), bits, ".".join(stack), decl))
    return rows


def write_struct_offsets(dump: str, out: Path) -> None:
    lines = dump.splitlines()
    sections = []
    for name in STRUCTS:
        header = next((i for i, l in enumerate(lines) if re.match(rf"^\s*0 \| (struct|union) {name}$", l)), None)
        if header is None:
            sections.append(f"## {name}: not found in layout dump\n")
            continue
        rows = flatten_record(lines, header)
        body = [f"0x{off:04X}{bits:<7} {path:<60} {decl}" for off, bits, path, decl in rows]
        sections.append(f"## {name}\n" + "\n".join(body) + "\n")
    out.write_text("# N64 struct offsets (clang MIPS32 BE layout of decomp headers)\n\n" + "\n".join(sections))


def collect_vanilla_funcs() -> list:
    """Scans decomp/src/**.c for `// 0x8...` comments directly above a function definition."""
    rows = []
    for path in sorted((ROOT / "decomp/src").rglob("*.c")):
        if "relocData" in path.parts:
            continue
        lines = path.read_text(errors="replace").splitlines()
        for i, line in enumerate(lines[:-1]):
            addr = FUNC_ADDR.match(line.strip())
            if not addr:
                continue
            fdef = FUNC_DEF.match(lines[i + 1])
            if fdef and not lines[i + 1].rstrip().endswith(";"):
                rows.append((int(addr.group(1), 16), fdef.group(1), f"{path.relative_to(ROOT)}:{i + 2}"))
    return sorted(set(rows))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--out", type=Path, default=ROOT / "build/remixasm/refs")
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)

    write_struct_offsets(dump_layouts(args.out), args.out / "struct_offsets.txt")
    funcs = collect_vanilla_funcs()
    (args.out / "vanilla_funcs.txt").write_text("".join(f"0x{a:08X} {n} {loc}\n" for a, n, loc in funcs))
    print(f"struct_offsets.txt: {len(STRUCTS)} structs; vanilla_funcs.txt: {len(funcs)} functions -> {args.out}")


if __name__ == "__main__":
    main()
