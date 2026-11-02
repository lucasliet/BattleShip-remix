#!/usr/bin/env python3
"""Generate port/remix/remix_roster_data.h from the public Smash Remix source.

Reads src/Character.asm (ids, variant/sound tables, define_character calls) and
src/costumes/*.asm (NUM_COSTUMES, NUM_EXTRA_COSTUMES, register_extra_costumes_for_char)
from a Remix checkout. Stdlib only. Run with system python3:

    python3 tools/gen_remix_roster.py [path/to/smashremix-src]
"""

import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REMIX_SRC = REPO_ROOT / ".toolchain" / "pkgs" / "smashremix-src"
OUTPUT_HEADER = REPO_ROOT / "port" / "remix" / "remix_roster_data.h"

ROSTER_SIZE = 0x61
VANILLA_SLOT_COUNT = 0x1B
RANDOM_ID = 0x1B
CLONE_ID = 0x1C
FIRST_ADDED_ID = 0x1D
NO_PARENT = 0xFF
MAX_VANILLA_PARENT_ID = 0x0B
ID_ENUM_PREFIX = "REMIX_ID_"
STATIC_VARIANT_PAD = "NA"
STATIC_SOUND_PAD = "U"
POLYGON_VARIANT = "POLYGON"

# Vanilla menu zoom table (ROM 0x108370, ids 0x00..0x0B). Character.asm moves it with move_table_12 and the
# Remix ROM stores the same floats (checked against Character.menu_zoom.table in baserom.remix.z64).
VANILLA_MENU_ZOOM = (1.25, 1.15, 1.0, 1.03, 1.21, 1.33, 1.05, 1.07, 1.22, 1.2, 1.26, 1.3)
MENU_ZOOM_PATCH_PATTERN = re.compile(
    r"table_patch_start\(menu_zoom,\s*Character\.id\.(\w+),\s*0x4\)\s*float32\s+([0-9.]+)"
)
CHARACTER_SELECT_FILE = "CharacterSelect.asm"

DEFINE_ARG_COUNT = 21
ARG_NAME = 0
ARG_PARENT = 1
ARG_FILE_MAIN = 2
ARG_FILE_MOVESET = 3
ARG_FILE_CHAR = 5
ARG_ATTRIB_OFFSET = 11
ARG_ADD_ACTIONS = 12
ARG_JAB_3 = 13
ARG_INHALE_COPY = 14
ARG_SOUND_TYPE = 19
ARG_VARIANT_TYPE = 20

NUMBER_PATTERN = r"0x[0-9A-Fa-f]+|\d+"


class RosterParseError(Exception):
    """Raised when a Remix source construct does not match the expected shape."""


def read_text(path):
    return path.read_text(encoding="utf-8")


def scope_body(text, header):
    """Return the text inside the brace block that follows the unique `header` line."""
    if text.count(header) != 1:
        raise RosterParseError(f"expected exactly one '{header}', found {text.count(header)}")
    open_index = text.index("{", text.index(header))
    depth = 0
    for index in range(open_index, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[open_index + 1 : index]
    raise RosterParseError(f"unclosed block after '{header}'")


def parse_int(token):
    try:
        return int(token, 0)
    except ValueError as error:
        raise RosterParseError(f"not an integer literal: {token!r}") from error


def strip_comment(line):
    return line.split("//", 1)[0]


def parse_id_scope(text):
    """Return ({name: id}, {id: first listed name}) from `scope id`."""
    body = scope_body(text, "scope id {")
    ids_by_name = {}
    primary_name_by_id = {}
    for match in re.finditer(r"constant (\w+)\((" + NUMBER_PATTERN + r")\)", body):
        name, value = match.group(1), parse_int(match.group(2))
        ids_by_name[name] = value
        primary_name_by_id.setdefault(value, name)
    return ids_by_name, primary_name_by_id


def parse_static_table(body, pad_name):
    """Expand the `fill N, NAME` / `db NAME` lines after `table:` into one member name per id."""
    names = []
    in_table = False
    for raw_line in body.splitlines():
        line = strip_comment(raw_line).strip()
        if line.startswith("table:"):
            in_table = True
            continue
        if not in_table or not line:
            continue
        fill_match = re.fullmatch(r"fill\s+(" + NUMBER_PATTERN + r"),\s*(\w+)", line)
        db_match = re.fullmatch(r"db\s+(\w+)", line)
        if fill_match:
            names.extend([fill_match.group(2)] * parse_int(fill_match.group(1)))
        elif db_match:
            names.append(db_match.group(1))
    return names, pad_name


def parse_variant_table(text):
    return parse_static_table(scope_body(text, "scope variant_type {"), STATIC_VARIANT_PAD)


def parse_sound_table(text):
    return parse_static_table(scope_body(text, "scope sound_type {"), STATIC_SOUND_PAD)


def static_name_for(table, remix_id):
    names, pad_name = table
    return names[remix_id] if remix_id < len(names) else pad_name


def split_define_args(inner_text):
    return [argument.strip() for argument in inner_text.split(",")]


def find_id_comment(lines, call_index):
    """Return (id, name) from the nearest `// 0xNN - NAME` comment above a define call, or None.

    Polygon fighters are written under a bare `// NAME` heading with no id, so None is valid there.
    """
    for index in range(call_index - 1, max(call_index - 6, -1), -1):
        match = re.search(r"//\s*0x([0-9A-Fa-f]+)\s*-\s*(\w+)", lines[index])
        if match:
            return int(match.group(1), 16), match.group(2)
    return None


def collect_define_calls(text):
    """Return (args, source_comment) for each full define_character call, in source order."""
    lines = text.splitlines()
    calls = []
    for line_index, line in enumerate(lines):
        match = re.match(r"^\s*define_character\((.*)\)\s*$", line)
        if not match:
            continue
        args = split_define_args(match.group(1))
        if len(args) == 1:
            continue
        if len(args) != DEFINE_ARG_COUNT:
            raise RosterParseError(
                f"line {line_index + 1}: define_character has {len(args)} args, expected {DEFINE_ARG_COUNT}"
            )
        calls.append((args, find_id_comment(lines, line_index), line_index + 1))
    return calls


def enum_member(argument, table_name):
    """Turn `variant_type.POLYGON` style arguments into the bare member name."""
    head, _, member = argument.partition(".")
    if head != table_name or not member:
        raise RosterParseError(f"expected '{table_name}.<MEMBER>', got {argument!r}")
    return member


def evaluate_sum(expression):
    """Evaluate a literal sum such as `5 + 1` used by NUM_COSTUMES; anything else is an error."""
    total = 0
    for term in expression.split("+"):
        term = term.strip()
        if not re.fullmatch(NUMBER_PATTERN, term):
            raise RosterParseError(f"unsupported constant expression: {expression!r}")
        total += parse_int(term)
    return total


def parse_costume_files(costumes_dir, ids_by_name):
    """Return {remix_id: (file_name, num_costumes, num_extra_costumes)} from costume registrations."""
    registered = {}
    for path in sorted(costumes_dir.glob("*.asm")):
        text = read_text(path)
        costume_match = re.search(r"constant NUM_COSTUMES\(([^)]*)\)", text)
        extra_match = re.search(r"constant NUM_EXTRA_COSTUMES\((" + NUMBER_PATTERN + r")\)", text)
        if not costume_match or not extra_match:
            continue
        num_costumes = evaluate_sum(costume_match.group(1))
        num_extra = parse_int(extra_match.group(1))
        for char_match in re.finditer(r"register_extra_costumes_for_char\(Character\.id\.(\w+)\)", text):
            char_name = char_match.group(1)
            if char_name not in ids_by_name:
                raise RosterParseError(f"{path.name}: unknown character id {char_name!r}")
            char_id = ids_by_name[char_name]
            if char_id in registered:
                raise RosterParseError(f"id 0x{char_id:02X} registered by two costume files")
            registered[char_id] = (path.name, num_costumes, num_extra)
    return registered


def assign_added_ids(calls):
    """Ids follow the macro order: RANDOM takes 0x1B, the 0x1C clone is skipped, then 0x1D upward."""
    return [RANDOM_ID] + list(range(FIRST_ADDED_ID, FIRST_ADDED_ID + len(calls) - 1))


def check_source_comment(comment, expected_id, name, line_number):
    if comment is None:
        return
    comment_id, comment_name = comment
    if comment_id != expected_id:
        raise RosterParseError(
            f"line {line_number}: source comment says id 0x{comment_id:02X}, "
            f"generator computed 0x{expected_id:02X} for {name}"
        )
    if comment_name != name:
        print(
            f"warning: line {line_number}: source comment label {comment_name!r} differs from define name {name!r}",
            file=sys.stderr,
        )


def build_defined_entry(args, expected_id, ids_by_name, costumes_by_id):
    """Build the roster record for one define_character call at `expected_id`."""
    name = args[ARG_NAME]
    parent = args[ARG_PARENT]
    if parent not in ids_by_name or ids_by_name[parent] > MAX_VANILLA_PARENT_ID:
        raise RosterParseError(f"{name}: unsupported parent {parent!r}")
    file_name, num_costumes, num_extra = costumes_by_id.get(expected_id, (None, 0, 0))
    return {
        "remix_id": expected_id,
        "name": name,
        "parent_remix_id": ids_by_name[parent],
        "has_define_character": True,
        "add_actions": parse_int(args[ARG_ADD_ACTIONS]),
        "num_costumes": num_costumes,
        "num_extra_costumes": num_extra,
        "costume_file": file_name,
        "variant_type": enum_member(args[ARG_VARIANT_TYPE], "variant_type"),
        "sound_type": enum_member(args[ARG_SOUND_TYPE], "sound_type"),
        "is_polygon": enum_member(args[ARG_VARIANT_TYPE], "variant_type") == POLYGON_VARIANT,
        "jab3": args[ARG_JAB_3] == "OS.TRUE",
        "inhale_copy": args[ARG_INHALE_COPY] == "OS.TRUE",
        "file_main": args[ARG_FILE_MAIN],
        "file_moveset": args[ARG_FILE_MOVESET],
        "file_char": args[ARG_FILE_CHAR],
        "attrib_offset": parse_int(args[ARG_ATTRIB_OFFSET]),
    }


def build_static_entry(remix_id, name, variant_name, sound_name, costumes_by_id):
    """Build a record for a slot with no define_character call (vanilla or the 0x1C clone)."""
    file_name, num_costumes, num_extra = costumes_by_id.get(remix_id, (None, 0, 0))
    return {
        "remix_id": remix_id,
        "name": name,
        "parent_remix_id": NO_PARENT,
        "has_define_character": False,
        "add_actions": 0,
        "num_costumes": num_costumes,
        "num_extra_costumes": num_extra,
        "costume_file": file_name,
        "variant_type": variant_name,
        "sound_type": sound_name,
        "is_polygon": variant_name == POLYGON_VARIANT,
        "jab3": False,
        "inhale_copy": False,
        "file_main": None,
        "file_moveset": None,
        "file_char": None,
        "attrib_offset": 0,
    }


def assemble_roster(remix_src):
    """Return the list of ROSTER_SIZE records, ordered by remix_id."""
    character_text = read_text(remix_src / "src" / "Character.asm")
    vanilla_ids_by_name, primary_name_by_id = parse_id_scope(character_text)
    variant_table = parse_variant_table(character_text)
    sound_table = parse_sound_table(character_text)
    calls = collect_define_calls(character_text)
    added_ids = assign_added_ids(calls)
    ids_by_name = dict(vanilla_ids_by_name)
    ids_by_name.update({args[ARG_NAME]: remix_id for (args, _, _), remix_id in zip(calls, added_ids)})
    costumes_by_id = parse_costume_files(remix_src / "src" / "costumes", ids_by_name)

    entries = {}
    for remix_id in range(VANILLA_SLOT_COUNT):
        name = primary_name_by_id[remix_id]
        entries[remix_id] = build_static_entry(
            remix_id, name, static_name_for(variant_table, remix_id), static_name_for(sound_table, remix_id),
            costumes_by_id,
        )

    for (args, comment, line_number), remix_id in zip(calls, added_ids):
        check_source_comment(comment, remix_id, args[ARG_NAME], line_number)
        entries[remix_id] = build_defined_entry(args, remix_id, ids_by_name, costumes_by_id)

    entries[CLONE_ID] = build_static_entry(
        CLONE_ID, primary_name_by_id[CLONE_ID], static_name_for(variant_table, CLONE_ID),
        static_name_for(sound_table, CLONE_ID), costumes_by_id,
    )
    roster = [entries[remix_id] for remix_id in range(ROSTER_SIZE)]
    apply_menu_zoom(roster, remix_src, ids_by_name)
    return roster


def menu_zoom_patches(remix_src, ids_by_name):
    """Return {id: zoom} from every `table_patch_start(menu_zoom, ...)` in the Remix source.

    CharacterSelect.asm is read first: Sheik and NSheik are patched there and again in their own files,
    and the ROM keeps the character file's value.
    """
    source_files = sorted((remix_src / "src").rglob("*.asm"), key=lambda path: path.name != CHARACTER_SELECT_FILE)
    zoom_by_id = {}
    for path in source_files:
        for match in MENU_ZOOM_PATCH_PATTERN.finditer(read_text(path)):
            if match.group(1) not in ids_by_name:
                raise RosterParseError(f"{path.name}: menu_zoom patch for unknown character {match.group(1)!r}")
            zoom_by_id[ids_by_name[match.group(1)]] = float(match.group(2))
    return zoom_by_id


def base_menu_zoom(entry, ids_by_name):
    """Zoom an id holds before the patches, mirroring move_table_12 and the parent copy in define_character."""
    remix_id = entry["remix_id"]
    if remix_id <= MAX_VANILLA_PARENT_ID:
        return VANILLA_MENU_ZOOM[remix_id]
    if remix_id in (ids_by_name["BOSS"], ids_by_name["METAL"]):
        return VANILLA_MENU_ZOOM[ids_by_name["MARIO"]]
    if remix_id == ids_by_name["GDONKEY"]:
        return VANILLA_MENU_ZOOM[ids_by_name["DONKEY"]]
    polygon_base = ids_by_name["NMARIO"]
    if polygon_base <= remix_id < polygon_base + len(VANILLA_MENU_ZOOM):
        return VANILLA_MENU_ZOOM[remix_id - polygon_base]
    if entry["has_define_character"]:
        return VANILLA_MENU_ZOOM[entry["parent_remix_id"]]
    return 0.0


def apply_menu_zoom(entries, remix_src, ids_by_name):
    """Set entry["menu_zoom"] on every record."""
    patches = menu_zoom_patches(remix_src, ids_by_name)
    for entry in entries:
        entry["menu_zoom"] = patches.get(entry["remix_id"], base_menu_zoom(entry, ids_by_name))


def check_roster(roster):
    """Fail loudly if the assembled roster is not the 97-slot table the port expects."""
    if [entry["remix_id"] for entry in roster] != list(range(ROSTER_SIZE)):
        raise RosterParseError("roster ids are not the contiguous range 0x00..0x60")
    added = [entry for entry in roster if entry["remix_id"] >= FIRST_ADDED_ID]
    if len(added) != ROSTER_SIZE - FIRST_ADDED_ID:
        raise RosterParseError(f"expected {ROSTER_SIZE - FIRST_ADDED_ID} added fighters, got {len(added)}")


def c_string(value):
    return "NULL" if value is None else f'"{value}"'


def c_bool(value):
    return "true" if value else "false"


def render_id_enum(roster):
    lines = ["enum RemixFighterId {"]
    for entry in roster:
        lines.append(f"    {ID_ENUM_PREFIX}{entry['name']} = 0x{entry['remix_id']:02X},")
    lines.append("};")
    return "\n".join(lines)


def render_entry(entry):
    fields = [
        f"0x{entry['remix_id']:02X}",
        c_string(entry["name"]),
        f"0x{entry['parent_remix_id']:02X}",
        c_bool(entry["has_define_character"]),
        f"0x{entry['add_actions']:02X}",
        str(entry["num_costumes"]),
        str(entry["num_extra_costumes"]),
        f"REMIX_VARIANT_{entry['variant_type']}",
        f"REMIX_SOUND_{entry['sound_type']}",
        c_bool(entry["is_polygon"]),
        c_bool(entry["jab3"]),
        c_bool(entry["inhale_copy"]),
        c_string(entry["file_main"]),
        c_string(entry["file_moveset"]),
        c_string(entry["file_char"]),
        f"0x{entry['attrib_offset']:03X}",
        f"{entry['menu_zoom']!r}f",
    ]
    costume_note = entry["costume_file"] or "no costume file"
    return f"    {{{', '.join(fields)}}},  /* {entry['name']}: {costume_note} */"


HEADER_PROLOGUE = """/*
 * Generated by tools/gen_remix_roster.py from the public Smash Remix source
 * (src/Character.asm and the costume .asm files). Do not edit by hand; re-run the script.
 *
 * Entries with has_define_character == false are vanilla or non-fighter slots:
 * their add_actions, file_*, attrib_offset and jab3/inhale_copy are not defined by
 * the Remix source and are zero/NULL/false placeholders. menu_zoom is the CSS/results panel
 * scale of the fighter (Character.menu_zoom.table), patches included.
 */
#ifndef REMIX_ROSTER_DATA_H
#define REMIX_ROSTER_DATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum RemixVariantType {
    REMIX_VARIANT_NA = 0x00,
    REMIX_VARIANT_POLYGON = 0x01,
    REMIX_VARIANT_J = 0x02,
    REMIX_VARIANT_E = 0x03,
    REMIX_VARIANT_SPECIAL = 0x04,
    REMIX_VARIANT_UNUSED = 0xFF,
};

enum RemixSoundType {
    REMIX_SOUND_U = 0x00,
    REMIX_SOUND_J = 0x01,
};

/* Port fkind for a Remix fighter id: identity. Ids 0x1B (RANDOM) and 0x1C stay
 * unregistered so 28 (nFTKindNull) never names a fighter. */
#define REMIX_ID_TO_PORT_FKIND(id) (id)

typedef struct RemixRosterEntry {
    uint8_t remix_id;
    const char *name;
    uint8_t parent_remix_id;
    bool has_define_character;
    uint8_t add_actions;
    uint8_t num_costumes;
    uint8_t num_extra_costumes;
    enum RemixVariantType variant_type;
    enum RemixSoundType sound_type;
    bool is_polygon;
    bool jab3;
    bool inhale_copy;
    const char *file_main;
    const char *file_moveset;
    const char *file_char;
    uint16_t attrib_offset;
    float menu_zoom;
} RemixRosterEntry;
"""


def render_header(roster):
    rows = "\n".join(render_entry(entry) for entry in roster)
    return "\n".join([
        HEADER_PROLOGUE,
        render_id_enum(roster),
        "",
        f"static const RemixRosterEntry kRemixRoster[{ROSTER_SIZE}] = {{",
        rows,
        "};",
        "",
        "#endif /* REMIX_ROSTER_DATA_H */",
        "",
    ])


def main(argv):
    remix_src = Path(argv[1]) if len(argv) > 1 else DEFAULT_REMIX_SRC
    roster = assemble_roster(remix_src)
    check_roster(roster)
    OUTPUT_HEADER.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_HEADER.write_text(render_header(roster), encoding="utf-8")
    print(f"wrote {OUTPUT_HEADER} with {len(roster)} entries")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except (RosterParseError, OSError) as error:
        print(f"gen_remix_roster: {error}", file=sys.stderr)
        sys.exit(1)
