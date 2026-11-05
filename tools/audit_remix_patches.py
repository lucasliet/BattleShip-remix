#!/usr/bin/env python3
"""List every OS.patch_start of the Smash Remix source with the decomp function it lands in and whether the port hooks it.

A patch writes code or data at a vanilla ROM offset and RAM address. Overlays share RAM, so one address can belong to several
decomp functions; the candidate kept is the file whose function starts, moved by the ROM-to-RAM delta of the patch, land on
stack-frame prologues in the vanilla ROM (the ROM is only read, nothing derived from it is written). A candidate is "hooked" when its function body in the port
names a Remix symbol (REGION_REMIX, or an identifier containing "Remix"/"remix"). "unhooked" rows are the ones to triage: the
port may implement the patch elsewhere (a caller, a data table, a Torch factory), or the patch may be missing.

Stdlib only. Output: a TSV on stdout (or --out), one row per patch, and a summary on stderr.

Usage: .toolchain/host/venv/bin/python -I tools/audit_remix_patches.py [--asm DIR] [--src DIR] [--out FILE]
"""

import argparse
import bisect
import re
import sys
from collections import Counter
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_ASM = REPO_ROOT / ".toolchain" / "pkgs" / "smashremix-src" / "src"
DEFAULT_SRC = REPO_ROOT / "decomp" / "src"
DEFAULT_ROM = REPO_ROOT / "baserom.us.z64"
PATCH = re.compile(r"^\s*OS\.patch_start\(\s*([^,]+?)\s*,\s*(.+?)\s*\)\s*(?://.*)?$")
SCOPE = re.compile(r"^\s*scope\s+([A-Za-z_]\w*)\s*:")
HEX_EXPR = re.compile(r"^[0-9A-Fa-fx\s\+\-]+$")
ADDRESS_COMMENT = re.compile(r"^\s*//\s*0x([0-9A-Fa-f]{8})\b")
FUNC_NAME = re.compile(r"\b([A-Za-z_]\w*)\s*\(")
REMIX_TOKEN = re.compile(r"REGION_REMIX|[A-Za-z_]\w*[Rr]emix\w*|\bREMIX_\w+")
LAST_FUNCTION_SPAN = 0x400


def eval_hex(text):
    """Value of '0xA97C4 + 0x10' style expressions, or None for symbolic ones (Remix tables, macros)."""
    if not HEX_EXPR.match(text):
        return None
    try:
        return eval(text, {"__builtins__": {}}, {})
    except (SyntaxError, NameError, TypeError):
        return None


def scan_patches(asm_root):
    """Yield dicts with file, line, scope, rom, ram and the raw arguments of every patch."""
    for path in sorted(asm_root.rglob("*.asm")):
        scopes = []
        for number, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
            scope = SCOPE.match(line)
            if scope:
                scopes.append(scope.group(1))
            match = PATCH.match(line)
            if not match:
                continue
            yield {
                "file": str(path.relative_to(asm_root)),
                "line": number,
                "scope": scopes[-1] if scopes else "",
                "rom": eval_hex(match.group(1)),
                "ram": eval_hex(match.group(2)),
                "raw": f"{match.group(1)}, {match.group(2)}",
            }


def function_spans(lines):
    """(start address, name, first line, last line) of every `// 0x...` function of one file, sorted by address."""
    marks = []
    for index, line in enumerate(lines):
        comment = ADDRESS_COMMENT.match(line)
        if not comment:
            continue
        for offset in range(index + 1, min(index + 4, len(lines))):
            text = lines[offset]
            name = FUNC_NAME.search(text)
            if name and not text.rstrip().endswith(";") and "=" not in text:
                marks.append((int(comment.group(1), 16), name.group(1), offset))
                break
    spans = []
    for position, (address, name, first) in enumerate(marks):
        last = marks[position + 1][2] - 1 if position + 1 < len(marks) else len(lines) - 1
        spans.append((address, name, first, last))
    return sorted(spans)


def load_functions(src_root):
    """Per decomp file: the sorted function starts and (start, end, name, line, hooked) rows."""
    index = {}
    for path in sorted(src_root.rglob("*.c")):
        lines = path.read_text(errors="replace").splitlines()
        spans = function_spans(lines)
        if not spans:
            continue
        rows = []
        for position, (address, name, first, last) in enumerate(spans):
            end = spans[position + 1][0] if position + 1 < len(spans) else address + LAST_FUNCTION_SPAN
            body = "\n".join(lines[first:last + 1])
            rows.append((address, end, name, first + 1, bool(REMIX_TOKEN.search(body))))
        index[str(path.relative_to(src_root))] = ([row[0] for row in rows], rows)
    return index


def candidates_for(ram, functions):
    """Functions whose address range holds `ram`, at most one per file (one per overlay that maps that address)."""
    found = []
    for path, (starts, rows) in functions.items():
        position = bisect.bisect_right(starts, ram) - 1
        if position < 0:
            continue
        address, end, name, line, hooked = rows[position]
        if address <= ram < end:
            found.append((name, path, line, hooked))
    return found


def prologue_score(path, functions, delta, rom):
    """Fraction of a file's function starts that land on a stack-frame prologue (addiu sp, sp, -n) in the vanilla ROM at
    this ROM-to-RAM delta. The file of the overlay that is really mapped there scores near 1, the others near 0."""
    starts = functions[path][0]
    hits = 0
    for address in starts:
        offset = address - delta
        if 0 <= offset <= len(rom) - 4:
            word = int.from_bytes(rom[offset:offset + 4], "big")
            hits += (word >> 16) == 0x27BD and (word & 0x8000) != 0
    return hits / len(starts)


def pick_overlay(patches, functions, rom):
    """Keep, per patch, the candidate files whose functions line up with the vanilla ROM at the patch delta."""
    for patch in patches:
        if len(patch["candidates"]) < 2 or patch["rom"] is None:
            continue
        delta = patch["ram"] - patch["rom"]
        scores = {c[1]: prologue_score(c[1], functions, delta, rom) for c in patch["candidates"]}
        best = max(scores.values())
        if best >= 0.5:
            patch["candidates"] = [c for c in patch["candidates"] if scores[c[1]] == best]


def classify(patch):
    """'symbolic', 'no-function', 'hooked', 'ambiguous-hooked' or 'unhooked'."""
    if patch["ram"] is None:
        return "symbolic"
    if not patch["candidates"]:
        return "no-function"
    flags = {hooked for _, _, _, hooked in patch["candidates"]}
    if flags == {True}:
        return "hooked"
    if True in flags:
        return "ambiguous-hooked"
    return "unhooked"


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--asm", type=Path, default=DEFAULT_ASM)
    parser.add_argument("--src", type=Path, default=DEFAULT_SRC)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM, help="vanilla US ROM (.z64, big-endian)")
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    if not args.asm.is_dir():
        sys.exit(f"Remix source not found at {args.asm} (expected .toolchain/pkgs/smashremix-src/src)")

    functions = load_functions(args.src)
    patches = list(scan_patches(args.asm))
    for patch in patches:
        patch["candidates"] = candidates_for(patch["ram"], functions) if patch["ram"] is not None else []
    if not args.rom.is_file():
        sys.exit(f"vanilla ROM not found at {args.rom} (expected the US 1.0 .z64 the Remix patches)")
    pick_overlay(patches, functions, args.rom.read_bytes())

    rows = ["status\tasm\tline\tscope\trom\tram\tfunction\tdecomp_file\tdecomp_line"]
    counts = Counter()
    for patch in patches:
        status = classify(patch)
        counts[status] += 1
        rom = f"0x{patch['rom']:X}" if patch["rom"] is not None else patch["raw"]
        ram = f"0x{patch['ram']:08X}" if patch["ram"] is not None else ""
        names = ",".join(c[0] for c in patch["candidates"])
        paths = ",".join(c[1] for c in patch["candidates"])
        lines = ",".join(str(c[2]) for c in patch["candidates"])
        rows.append(f"{status}\t{patch['file']}\t{patch['line']}\t{patch['scope']}\t{rom}\t{ram}\t{names}\t{paths}\t{lines}")
    text = "\n".join(rows) + "\n"
    if args.out:
        args.out.write_text(text)
    else:
        sys.stdout.write(text)
    summary = ", ".join(f"{key} {value}" for key, value in sorted(counts.items()))
    print(f"{len(patches)} patches: {summary}", file=sys.stderr)


if __name__ == "__main__":
    main()
