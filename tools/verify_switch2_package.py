#!/usr/bin/env python3
"""
Verify the Nintendo Switch 2 (libounce + OunceVK) NRO, ELF and linker map that
scripts/switch/build-switch2.sh produces.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

NRO_NRO0_MAGIC = b"NRO0"
MOD0_MAGIC = b"MOD0"
OUN0_MAGIC = b"OUN0"
ASET_MAGIC = b"ASET"
ELF_MAGIC = b"\x7fELF"
EM_AARCH64 = 183

FORBIDDEN_MAP_SUBSTRINGS = (
    "/libnx/",
    "/portlibs/switch/lib/libEGL.a",
    "/portlibs/switch/lib/libglapi.a",
    "/portlibs/switch/lib/libdrm_nouveau.a",
    "libdeko3d",
    "libuam",
)

REQUIRED_MAP_SUBSTRINGS = (
    "/libounce/lib/",
    "/portlibs/switch2/lib/libEGL.a",
)

FORBIDDEN_BINARY_SYMBOLS = (
    b"nvMapCreate",
    b"nvGpuInit",
    b"nvHostOpen",
    b"dkDeviceCreate",
    b"uam_compile_dksh",
    b"clkrstOpenSession",
    b"pcvSetClockRate",
)


def verify_nro(nro_path: Path) -> list[str]:
    errors: list[str] = []
    if not nro_path.is_file():
        return [f"NRO file not found: {nro_path}"]

    data = nro_path.read_bytes()
    if len(data) < 0x80:
        return [f"NRO file too small ({len(data)} bytes): {nro_path}"]

    if data[0x10:0x14] != NRO_NRO0_MAGIC:
        return [f"{nro_path.name}: not a valid NRO0 binary (magic={data[0x10:0x14]!r})"]

    page = data[:0x1000]
    mod = struct.unpack_from("<I", page, 4)[0]
    if mod + 0x1C > len(page) or page[mod : mod + 4] != MOD0_MAGIC:
        errors.append(f"{nro_path.name}: missing MOD0 startup header at offset 0x{mod:x}")
    else:
        found_oun0 = False
        at = mod + 0x1C
        for _ in range(8):
            if at + 12 > len(page):
                break
            magic = page[at : at + 4]
            if magic == OUN0_MAGIC:
                platform, version = struct.unpack_from("<II", page, at + 4)
                if platform != 2 or version != 1:
                    errors.append(
                        f"{nro_path.name}: unexpected OUN0 metadata platform={platform} version={version}"
                    )
                found_oun0 = True
                break
            if magic[:3] != b"LNY":
                break
            at += 12
        if not found_oun0:
            errors.append(
                f"{nro_path.name}: missing Switch 2 'OUN0' startup tag in MOD0 header"
            )

    nro_size = struct.unpack_from("<I", data, 0x18)[0]
    if nro_size == 0 or nro_size > len(data):
        errors.append(
            f"{nro_path.name}: invalid NRO header size {nro_size} (file size {len(data)})"
        )
    elif nro_size < len(data):
        asset_hdr = data[nro_size : nro_size + 4]
        if asset_hdr != ASET_MAGIC:
            errors.append(
                f"{nro_path.name}: trailing asset header magic is {asset_hdr!r}, expected 'ASET'"
            )

    for sym in FORBIDDEN_BINARY_SYMBOLS:
        if sym in data:
            errors.append(
                f"{nro_path.name}: contains forbidden Switch 1 symbol {sym.decode('ascii', errors='replace')}"
            )

    return errors


def verify_elf(elf_path: Path) -> list[str]:
    errors: list[str] = []
    if not elf_path.is_file():
        return [f"ELF file not found: {elf_path}"]

    data = elf_path.read_bytes()
    if len(data) < 64 or data[:4] != ELF_MAGIC:
        return [f"{elf_path.name}: not a valid ELF binary"]

    e_machine = struct.unpack_from("<H", data, 0x12)[0]
    if e_machine != EM_AARCH64:
        errors.append(
            f"{elf_path.name}: ELF e_machine is {e_machine}, expected {EM_AARCH64} (EM_AARCH64)"
        )

    for sym in FORBIDDEN_BINARY_SYMBOLS:
        if sym in data:
            errors.append(
                f"{elf_path.name}: contains forbidden Switch 1 symbol {sym.decode('ascii', errors='replace')}"
            )

    return errors


def verify_map(map_path: Path) -> list[str]:
    errors: list[str] = []
    if not map_path.is_file():
        return [f"Linker map file not found: {map_path}"]

    text = map_path.read_text(encoding="utf-8", errors="replace").replace("\\", "/")
    for req in REQUIRED_MAP_SUBSTRINGS:
        if req not in text:
            errors.append(f"{map_path.name}: missing required Switch 2 archive reference '{req}'")

    for bad in FORBIDDEN_MAP_SUBSTRINGS:
        if bad in text:
            sample = next((line.strip() for line in text.splitlines() if bad in line), "")
            errors.append(
                f"{map_path.name}: contains forbidden Switch 1 archive reference '{bad}' ({sample})"
            )

    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--nro", type=Path, required=True, help="Path to Switch 2 .nro")
    parser.add_argument("--elf", type=Path, required=False, help="Path to Switch 2 .elf")
    parser.add_argument("--map", type=Path, required=False, help="Path to linker .map file")
    args = parser.parse_args(argv)

    all_errors: list[str] = []
    all_errors.extend(verify_nro(args.nro))
    if args.elf is not None:
        all_errors.extend(verify_elf(args.elf))
    if args.map is not None:
        all_errors.extend(verify_map(args.map))

    if all_errors:
        for err in all_errors:
            print(f"[FAIL] {err}", file=sys.stderr)
        return 1

    print(
        f"[OK] Verified Switch 2 OUN0 package: {args.nro} ({args.nro.stat().st_size:,} bytes)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
