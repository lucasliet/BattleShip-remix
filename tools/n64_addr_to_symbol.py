#!/usr/bin/env python3
"""Find the decomp symbol that owns an N64 RAM address.

The decomp writes `// 0xADDRESS` on the line before a function or a global, and several globals also carry their address in
the name (dMNVSResultsUnused0x80138EF0). Overlays share one RAM range, so one address can have several owners; every
candidate is printed with its file so the caller can pick the overlay that is loaded at that moment. The Remix symbol map
(build/remixasm/remix.sym) is searched too, for addresses that name a Remix label.

Stdlib only.

Usage: python3 tools/n64_addr_to_symbol.py ADDRESS [ADDRESS...] [--src DIR] [--sym PATH] [--near BYTES]
"""

import argparse
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SRC = REPO_ROOT / "decomp" / "src"
DEFAULT_SYM = REPO_ROOT / "build" / "remixasm" / "remix.sym"
SOURCE_SUFFIXES = {".c", ".h"}
ADDRESS_COMMENT = re.compile(r"^\s*//\s*0x([0-9A-Fa-f]{8})\b")
NAME_ADDRESS = re.compile(r"\b([A-Za-z_]\w*?)0x([0-9A-Fa-f]{8})\b")
IDENTIFIER = re.compile(r"([A-Za-z_]\w*)\s*(?:\[|=|;|\()")


def parse_address(text):
    """Address from '0x80131580', '80131580' or decimal-looking hex; raises ValueError with the offending text."""
    cleaned = text.strip().lower()
    cleaned = cleaned[2:] if cleaned.startswith("0x") else cleaned
    try:
        return int(cleaned, 16)
    except ValueError:
        raise ValueError(f"'{text}' is not a hex address such as 0x80131580")


def declared_name(line):
    """Identifier a declaration line defines, or None when the line is not a declaration."""
    stripped = line.strip()
    if not stripped or stripped.startswith(("//", "/*", "*", "#")):
        return None
    match = IDENTIFIER.search(stripped)
    return match.group(1) if match else None


def scan_source_text(text, path):
    """Yield (address, name, path, line_number) for every address owner in one source file."""
    lines = text.splitlines()
    for index, line in enumerate(lines):
        comment = ADDRESS_COMMENT.match(line)
        if comment:
            for offset in range(index + 1, min(index + 4, len(lines))):
                name = declared_name(lines[offset])
                if name:
                    yield int(comment.group(1), 16), name, path, offset + 1
                    break
        for named in NAME_ADDRESS.finditer(line):
            yield int(named.group(2), 16), named.group(1) + "0x" + named.group(2).upper(), path, index + 1


def scan_source_tree(src_dir):
    """Every address owner of the .c and .h files under src_dir, as a list of (address, name, path, line)."""
    owners = []
    for path in sorted(Path(src_dir).rglob("*")):
        if path.suffix not in SOURCE_SUFFIXES:
            continue
        owners.extend(scan_source_text(path.read_text(errors="replace"), path))
    return owners


def scan_symbol_map(sym_path):
    """Entries of a bass symbol map ('80131fc4 Name') as (address, name, path, line)."""
    path = Path(sym_path)
    if not path.exists():
        return []
    entries = []
    for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
        fields = line.split()
        if len(fields) == 2 and re.fullmatch(r"[0-9a-fA-F]{8}", fields[0]):
            entries.append((int(fields[0], 16), fields[1], path, number))
    return entries


def lookup(owners, address, near=0):
    """Owners at exactly address, plus owners within `near` bytes below it as (owner, distance)."""
    exact = [(owner, 0) for owner in owners if owner[0] == address]
    below = [(owner, address - owner[0]) for owner in owners if 0 < address - owner[0] <= near]
    return exact + sorted(below, key=lambda pair: pair[1])


def format_result(address, results):
    if not results:
        return f"0x{address:08X}: no owner found"
    lines = [f"0x{address:08X}:"]
    for (owner_address, name, path, line), distance in results:
        place = f"{path}:{line}"
        suffix = "" if distance == 0 else f"  (+0x{distance:X} from 0x{owner_address:08X})"
        lines.append(f"  {name}  {place}{suffix}")
    return "\n".join(lines)


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    parser.add_argument("addresses", nargs="+")
    parser.add_argument("--src", default=str(DEFAULT_SRC))
    parser.add_argument("--sym", default=str(DEFAULT_SYM))
    parser.add_argument("--near", type=lambda text: int(text, 0), default=0x40,
                        help="also list owners this many bytes below the address (default 0x40)")
    args = parser.parse_args(argv)
    try:
        wanted = [parse_address(text) for text in args.addresses]
    except ValueError as error:
        print(error, file=sys.stderr)
        return 2
    owners = scan_source_tree(args.src) + scan_symbol_map(args.sym)
    for address in wanted:
        print(format_result(address, lookup(owners, address, args.near)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
