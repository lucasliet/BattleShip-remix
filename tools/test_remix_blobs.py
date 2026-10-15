#!/usr/bin/env python3
"""Check the Remix blobs written by Torch into an o2r against the Python reference tools.

Each BLOB resource is a 0x40-byte Torch header, a little-endian u32 payload size, then the
payload. The payload must be byte-identical to:

  audio/<bank>      tools/remix_dump_audio.py (B1_sounds1_ctl, B1_sounds1_tbl, B1_sounds2_ctl,
                    B1_sounds2_tbl, S1_music_sbk, fgm_unk, fgm_tbl, fgm_ucd)
  remix/fighters    the remix/fighters layout of docs/remix/torch_factories.md ("Contrato dos blobs"), built here from
                    the JSON of tools/remix_dump_fighters.py
  particles/<name>  the raw ROM range at offset/size of each SSB64:REMIXBLOB entry in
                    yamls/remix/particles.yml
  remix/<name>      the same, for the SSB64:REMIXBLOB entries of yamls/remix/remix.yml (ram_image)

Stdlib only. Exit status is 0 when every blob matches.

Usage: python3 tools/test_remix_blobs.py --o2r PATH [--rom PATH] [--ref-dir DIR]
"""

import argparse
import importlib.util
import re
import struct
import sys
import zipfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
TOOLS_DIR = REPO_ROOT / "tools"
DEFAULT_ROM = REPO_ROOT / "baserom.remix.z64"
DEFAULT_REF_DIR = REPO_ROOT / "build" / "remix-dump" / "audio"
REMIX_BLOB_YAMLS = [
    (REPO_ROOT / "yamls" / "remix" / "particles.yml", "particles/"),
    (REPO_ROOT / "yamls" / "remix" / "remix.yml", "remix/"),
]
REMIX_BLOB_TYPE = "SSB64:REMIXBLOB"
TOP_LEVEL_KEY = re.compile(r"^(\w+):\s*$")
YAML_FIELD = re.compile(r"^\s+(type|offset|size):\s*(\S+)\s*$")

TORCH_HEADER_SIZE = 0x40
TORCH_BLOB_TYPE = 0x4F424C42
AUDIO_BANKS = [
    "B1_sounds1_ctl", "B1_sounds1_tbl", "B1_sounds2_ctl", "B1_sounds2_tbl", "S1_music_sbk",
    "fgm_unk", "fgm_tbl", "fgm_ucd",
]
FIGHTERS_RESOURCE = "remix/fighters"
FIGHTERS_MAGIC = 0x46584D52
FIGHTERS_VERSION = 1
FIGHTERS_RECORD_SIZE = 0x80
MOTION_SIZE = 12
FILE_ID_ORDER = ["main", "mainmotion", "submotion", "model", "shieldpose", "special1", "special2", "special3", "special4"]


def load_tool(module_name):
    """Import a script from tools/ by file name."""
    spec = importlib.util.spec_from_file_location(module_name, TOOLS_DIR / f"{module_name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def torch_blob_payload(resource_name, data):
    """Return the payload of a Torch BLOB resource, checking its header."""
    if len(data) < TORCH_HEADER_SIZE + 4:
        raise ValueError(f"{resource_name}: {len(data)} bytes is too short for a Torch BLOB")
    resource_type = struct.unpack_from("<I", data, 4)[0]
    if resource_type != TORCH_BLOB_TYPE:
        raise ValueError(f"{resource_name}: resource type 0x{resource_type:08X} is not OBLB")
    declared = struct.unpack_from("<I", data, TORCH_HEADER_SIZE)[0]
    payload = data[TORCH_HEADER_SIZE + 4:]
    if declared != len(payload):
        raise ValueError(f"{resource_name}: declared size {declared} != payload {len(payload)}")
    return payload


def remix_blob_entries(yaml_path):
    """Return (name, offset, size) for each SSB64:REMIXBLOB entry, read line by line so the test needs no YAML library."""
    entries = []
    current = None
    for line in yaml_path.read_text().splitlines():
        key = TOP_LEVEL_KEY.match(line)
        if key:
            current = {"name": key.group(1)}
            entries.append(current)
            continue
        field = YAML_FIELD.match(line)
        if field and current is not None:
            current[field.group(1)] = field.group(2)
    return [(e["name"], int(e["offset"], 0), int(e["size"], 0)) for e in entries if e.get("type") == REMIX_BLOB_TYPE]


def hex_field(value):
    """Parse a '0x...' string from the JSON dump, or pass an int through."""
    return int(value, 16) if isinstance(value, str) else value


def motion_list(entries, count, label):
    """Return motion tuples for a dump array; a None array is valid only when its count is zero."""
    if entries is None:
        if count != 0:
            raise ValueError(f"{label}: dump has no entries for count {count}")
        return []
    if len(entries) != count:
        raise ValueError(f"{label}: {len(entries)} entries for count {count}")
    return [(m["anim_file_id"], hex_field(m["command_offset"]), hex_field(m["flags"])) for m in entries]


def build_fighters_reference(fighters):
    """Build the remix/fighters bytes from the fighter dump, following the plan's layout."""
    records = bytearray()
    motions = bytearray()
    for fighter in fighters:
        main_offset = len(motions)
        main = motion_list(fighter["mainmotion_entries"], fighter["mainmotion_count"], f"fkind {fighter['id']} main")
        for entry in main:
            motions += struct.pack("<3I", *entry)
        menu_offset = len(motions)
        menu = fighter["menu_entries"]
        if "count" not in menu:
            raise ValueError(f"fkind {fighter['id']}: menu status {menu.get('status')}")
        menu_entries = motion_list(menu["entries"], menu["count"], f"fkind {fighter['id']} menu")
        for entry in menu_entries:
            motions += struct.pack("<3I", *entry)
        record = struct.pack("<I", fighter["id"])
        record += struct.pack("<9I", *[fighter["file_ids"][name] for name in FILE_ID_ORDER])
        record += struct.pack("<7I", fighter["file_main_size"], fighter["attrib_offset"], fighter["file_anim_size"],
                              len(main), main_offset, len(menu_entries), menu_offset)
        records += record.ljust(FIGHTERS_RECORD_SIZE, b"\x00")
    header = struct.pack("<4I", FIGHTERS_MAGIC, FIGHTERS_VERSION, len(fighters), 0)
    return header + bytes(records) + bytes(motions)


def compare(name, actual, expected):
    """Print one comparison result and return True when the bytes are identical."""
    if actual == expected:
        print(f"OK        {name}: {len(actual)} bytes identical")
        return True
    first = next((i for i, (a, b) in enumerate(zip(actual, expected)) if a != b), min(len(actual), len(expected)))
    print(f"MISMATCH  {name}: actual {len(actual)} bytes, reference {len(expected)} bytes, first diff at 0x{first:X}")
    return False


def main(argv):
    """Compare every Remix blob in the o2r with its reference."""
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--o2r", type=Path, required=True)
    parser.add_argument("--rom", type=Path, default=DEFAULT_ROM)
    parser.add_argument("--ref-dir", type=Path, default=DEFAULT_REF_DIR)
    args = parser.parse_args(argv)

    audio_tool = load_tool("remix_dump_audio")
    fighters_tool = load_tool("remix_dump_fighters")

    audio_tool.run(args.rom, args.ref_dir)
    dump = fighters_tool.dump_rom(fighters_tool.RomImage(args.rom))
    fighters_reference = build_fighters_reference(dump["fighters"])

    results = []
    with zipfile.ZipFile(args.o2r) as archive:
        for bank in AUDIO_BANKS:
            resource = f"audio/{bank}"
            payload = torch_blob_payload(resource, archive.read(resource))
            reference = (args.ref_dir / f"{bank}.bin").read_bytes()
            results.append(compare(resource, payload, reference))
        payload = torch_blob_payload(FIGHTERS_RESOURCE, archive.read(FIGHTERS_RESOURCE))
        results.append(compare(FIGHTERS_RESOURCE, payload, fighters_reference))
        rom = args.rom.read_bytes()
        for yaml_path, resource_prefix in REMIX_BLOB_YAMLS:
            for name, offset, size in remix_blob_entries(yaml_path):
                resource = resource_prefix + name
                payload = torch_blob_payload(resource, archive.read(resource))
                results.append(compare(resource, payload, rom[offset:offset + size]))

    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
