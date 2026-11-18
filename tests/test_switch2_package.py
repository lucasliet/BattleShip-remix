#!/usr/bin/env python3
"""Unit tests for tools/verify_switch2_package.py."""

from __future__ import annotations

import struct
import tempfile
import unittest
from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
import verify_switch2_package as vsp  # noqa: E402


def _make_nro(include_oun0: bool = True, extra_payload: bytes = b"") -> bytes:
    buf = bytearray(0x200)
    mod_off = 0x80
    struct.pack_into("<I", buf, 0x04, mod_off)
    buf[0x10:0x14] = b"NRO0"
    buf[mod_off : mod_off + 4] = b"MOD0"
    at = mod_off + 0x1C
    buf[at : at + 4] = b"LNY0"
    at += 12
    if include_oun0:
        buf[at : at + 4] = b"OUN0"
        struct.pack_into("<II", buf, at + 4, 2, 1)
    buf.extend(extra_payload)
    struct.pack_into("<I", buf, 0x18, len(buf))
    buf.extend(b"ASET" + b"\x00" * 32)
    return bytes(buf)


class TestVerifySwitch2Package(unittest.TestCase):
    def test_valid_oun0_nro_passes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            nro = Path(tmp) / "BattleShip-Remix.nro"
            nro.write_bytes(_make_nro(include_oun0=True))
            self.assertEqual(vsp.verify_nro(nro), [])

    def test_switch1_without_oun0_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            nro = Path(tmp) / "BattleShip-Remix.nro"
            nro.write_bytes(_make_nro(include_oun0=False))
            errs = vsp.verify_nro(nro)
            self.assertTrue(any("OUN0" in e for e in errs), errs)

    def test_forbidden_clkrst_symbol_rejected(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            nro = Path(tmp) / "BattleShip-Remix.nro"
            nro.write_bytes(_make_nro(include_oun0=True, extra_payload=b"clkrstOpenSession"))
            errs = vsp.verify_nro(nro)
            self.assertTrue(any("clkrstOpenSession" in e for e in errs), errs)

    def test_linker_map_validation(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            mp = Path(tmp) / "BattleShip-Remix-switch2.map"
            mp.write_text(
                "/opt/devkitpro/libounce/lib/libnx.a(applet.o)\n"
                "/opt/devkitpro/portlibs/switch2/lib/libEGL.a(egl.o)\n",
                encoding="utf-8",
            )
            self.assertEqual(vsp.verify_map(mp), [])

            mp_bad = Path(tmp) / "bad.map"
            mp_bad.write_text(
                "/opt/devkitpro/libnx/lib/libnx.a(applet.o)\n"
                "/opt/devkitpro/portlibs/switch/lib/libEGL.a(egl.o)\n",
                encoding="utf-8",
            )
            errs = vsp.verify_map(mp_bad)
            self.assertGreaterEqual(len(errs), 2)


if __name__ == "__main__":
    unittest.main()
