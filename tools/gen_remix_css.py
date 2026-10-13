#!/usr/bin/env python3
"""Generate port/remix/remix_css_data.h from the public Smash Remix Character Select source.

Reads CharacterSelect.asm, Character.asm, FGM.asm and File.asm from a Remix checkout.
Symbolic constants are resolved from their asm declarations, and Remix fighter ids come
from tools/gen_remix_roster.py so both generators agree on them. Stdlib only:

    python3 tools/gen_remix_css.py [path/to/smashremix-src]
"""

import re
import struct
import sys
from collections import namedtuple
from pathlib import Path

import gen_remix_roster as roster_source

REPO_ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REMIX_SRC = REPO_ROOT / ".toolchain" / "pkgs" / "smashremix-src"
OUTPUT_HEADER = REPO_ROOT / "port" / "remix" / "remix_css_data.h"

SOURCE_FILES = ("Character.asm", "FGM.asm", "File.asm", "CharacterSelect.asm")
CSS_SCOPE = ("CharacterSelect",)
ROOT_SCOPE = ()
MAX_SYMBOL_DEPTH = 32
NO_PORTRAIT_OVERRIDE = -1
ADD_TO_CSS_ARG_COUNT = 8
VANILLA_ENTRY_COUNT = roster_source.CLONE_ID + 1

LAYOUT_CONSTANTS = (
    "NUM_ROWS",
    "NUM_COLUMNS",
    "NUM_SLOTS",
    "NUM_BONUS_CHARS",
    "START_X",
    "START_Y",
    "START_VISUAL",
    "PORTRAIT_WIDTH",
    "PORTRAIT_HEIGHT",
    "BOOKEND_WIDTH",
    "BOOKEND_BONUS_PORTRAIT",
    "BOOKEND_RANDOM_PORTRAIT",
)

VANILLA_TABLE_LABELS = {
    "announce_fgm": "fgm_table",
    "circle_size": "white_circle_size_table",
    "selection_action": "selection_action_table",
    "logo_offset": "series_logo_offset_table",
    "name_offset": "name_texture_table",
    "portrait_offset": "portrait_offset_by_character_table",
}

EXPECTED_MARIO_PORTRAIT_OFFSET = 0x1078
EXPECTED_BONUS_BOOKEND_OFFSET = 0x3B28
EXPECTED_RANDOM_BOOKEND_OFFSET = 0x53E8
EXPECTED_CRASH_PORTRAIT_OFFSET = 0x2AE58 + 0x10
EXPECTED_PORTRAITS_FILE_ID = 0xA05
EXPECTED_IMAGES_FILE_ID = 0xA06
EXPECTED_PORTRAIT_SCALE = 0.8125

DECLARATION_PATTERN = re.compile(r"\s*(constant|define)\s+(\w+)\((.*)\)\s*$")
PLACEHOLDER_PATTERN = re.compile(r"[{}]")
MACRO_PATTERN = re.compile(r"\s*macro\s+\w+\(.*\)\s*\{")
MACRO_MARKER = "<macro>"
SCOPE_PATTERN = re.compile(r"\s*scope\s+(\w+)\s*:?\s*\{")
TOKEN_PATTERN = re.compile(r"0[xX][0-9A-Fa-f]+|\d+|[A-Za-z_][\w.]*|[-+*()]")
TABLE_ENTRY_PATTERN = re.compile(r"(dh|dw|db|float32)\s+(.+)")
CONSTANT_LINE_PREFIX = "constant "
ADD_TO_CSS_PATTERN = re.compile(r"add_to_css\((.*)\)")

Symbol = namedtuple("Symbol", ["kind", "raw", "scope"])


class CssParseError(Exception):
    """Raised when a Remix source construct does not match the expected shape."""


def strip_comment(line):
    return line.split("//", 1)[0]


def inside_macro(scope_stack):
    return MACRO_MARKER in scope_stack


def update_scope_stack(line, scope_stack):
    """Track brace nesting; named scopes are kept so symbols get dotted paths."""
    scope_match = SCOPE_PATTERN.match(line)
    macro_match = MACRO_PATTERN.match(line)
    opens = line.count("{")
    if scope_match:
        scope_stack.append(scope_match.group(1))
        opens -= 1
    elif macro_match:
        scope_stack.append(MACRO_MARKER)
        opens -= 1
    scope_stack.extend([None] * opens)
    for _ in range(line.count("}")):
        if scope_stack:
            scope_stack.pop()


def tokenize(expression):
    tokens = []
    position = 0
    while position < len(expression):
        if expression[position].isspace():
            position += 1
            continue
        match = TOKEN_PATTERN.match(expression, position)
        if match is None:
            raise CssParseError(f"unsupported syntax in expression {expression!r} at column {position}")
        tokens.append(match.group(0))
        position = match.end()
    return tokens


class ExpressionParser:
    """Evaluates integer expressions: literals, names, parentheses, unary minus, + - and *."""

    def __init__(self, expression, resolve):
        self.expression = expression
        self.tokens = tokenize(expression)
        self.position = 0
        self.resolve = resolve

    def parse(self):
        value = self.parse_sum()
        if self.position != len(self.tokens):
            raise CssParseError(f"unexpected token {self.tokens[self.position]!r} in {self.expression!r}")
        return value

    def peek(self):
        return self.tokens[self.position] if self.position < len(self.tokens) else None

    def take(self):
        token = self.peek()
        if token is None:
            raise CssParseError(f"expression ends early: {self.expression!r}")
        self.position += 1
        return token

    def parse_sum(self):
        total = self.parse_product()
        while self.peek() in ("+", "-"):
            operator = self.take()
            operand = self.parse_product()
            total = total + operand if operator == "+" else total - operand
        return total

    def parse_product(self):
        total = self.parse_unary()
        while self.peek() == "*":
            self.take()
            total *= self.parse_unary()
        return total

    def parse_unary(self):
        if self.peek() == "-":
            self.take()
            return -self.parse_unary()
        return self.parse_atom()

    def parse_atom(self):
        token = self.take()
        if token == "(":
            value = self.parse_sum()
            if self.take() != ")":
                raise CssParseError(f"missing ')' in {self.expression!r}")
            return value
        if token[0].isdigit():
            return int(token, 0)
        return self.resolve(token)


class AsmSymbols:
    """Constants and defines from the asm sources, keyed by their dotted path."""

    def __init__(self):
        self.symbols = {}

    def add_source(self, text):
        scope_stack = []
        for raw_line in text.splitlines():
            line = strip_comment(raw_line)
            match = DECLARATION_PATTERN.match(line)
            if match and not inside_macro(scope_stack) and not PLACEHOLDER_PATTERN.search(match.group(3)):
                kind, name, raw = match.groups()
                scope = tuple(part for part in scope_stack if part)
                self._store(kind, scope + (name,), raw, scope)
            update_scope_stack(line, scope_stack)

    def add_literal(self, path, value):
        self._store("constant", tuple(path.split(".")), str(value), ROOT_SCOPE)

    def _store(self, kind, parts, raw, scope):
        path = ".".join(parts)
        if path in self.symbols:
            raise CssParseError(f"duplicate declaration of {path!r}")
        self.symbols[path] = Symbol(kind, raw.strip(), scope)

    def lookup(self, name, scope):
        for depth in range(len(scope), -1, -1):
            path = ".".join(scope[:depth] + (name,))
            if path in self.symbols:
                return self.symbols[path]
        raise CssParseError(f"unresolved symbol {name!r} (searched from scope {'.'.join(scope) or 'root'})")

    def evaluate(self, expression, scope, depth=0):
        if depth > MAX_SYMBOL_DEPTH:
            raise CssParseError(f"symbol chain too deep while evaluating {expression!r}")
        return ExpressionParser(expression, lambda name: self._resolve_value(name, scope, depth)).parse()

    def _resolve_value(self, name, scope, depth):
        symbol = self.lookup(name, scope)
        if symbol.kind != "constant":
            raise CssParseError(f"{name!r} is a define, expected a numeric constant")
        return self.evaluate(symbol.raw, symbol.scope, depth + 1)

    def define_name(self, name, scope):
        symbol = self.lookup(name, scope)
        if symbol.kind != "define":
            raise CssParseError(f"{name!r} is a constant, expected a define naming a character")
        return symbol.raw

    def table_values(self, entries, scope):
        values = []
        for kind, expression in entries:
            if kind == "float32":
                values.append(float(expression))
            else:
                values.append(self.evaluate(expression, scope))
        return values


def read_table_entries(text, label):
    """Return (kind, expression) data lines after `label:`, stopping at the first non-data line."""
    lines = [strip_comment(line).strip() for line in text.splitlines()]
    headers = [index for index, line in enumerate(lines) if line == f"{label}:"]
    if len(headers) != 1:
        raise CssParseError(f"expected exactly one '{label}:' label, found {len(headers)}")
    entries = []
    for line in lines[headers[0] + 1 :]:
        if not line or line.startswith(CONSTANT_LINE_PREFIX):
            continue
        match = TABLE_ENTRY_PATTERN.fullmatch(line)
        if match is None:
            break
        entries.append((match.group(1), match.group(2).strip()))
    return entries


def add_to_css_calls(css_text):
    """Return the argument lists of every `add_to_css(...)` call, in source order."""
    calls = []
    for raw_line in css_text.splitlines():
        match = ADD_TO_CSS_PATTERN.fullmatch(strip_comment(raw_line).strip())
        if match is None:
            continue
        args = [argument.strip() for argument in match.group(1).split(",")]
        if len(args) != ADD_TO_CSS_ARG_COUNT:
            raise CssParseError(
                f"add_to_css has {len(args)} args, expected {ADD_TO_CSS_ARG_COUNT}: {raw_line.strip()!r}"
            )
        calls.append(args)
    return calls


def add_define_character_ids(symbols, character_text):
    """Register the ids of fighters added by define_character, the same way the roster generator does."""
    calls = roster_source.collect_define_calls(character_text)
    added_ids = roster_source.assign_added_ids(calls)
    for (args, comment, line_number), remix_id in zip(calls, added_ids):
        name = args[roster_source.ARG_NAME]
        roster_source.check_source_comment(comment, remix_id, name, line_number)
        symbols.add_literal(f"Character.id.{name}", remix_id)


def character_ids_by_name(symbols):
    prefix = "Character.id."
    return {
        path[len(prefix):]: int(symbol.raw, 0)
        for path, symbol in symbols.symbols.items()
        if path.startswith(prefix)
    }


def names_by_id(ids_by_name):
    names = {}
    for name, remix_id in ids_by_name.items():
        names.setdefault(remix_id, name)
    return names


def id_of(ids_by_name, name):
    if name not in ids_by_name:
        raise CssParseError(f"unknown character name {name!r}")
    return ids_by_name[name]


def slot_ids(symbols, slot_count, ids_by_name):
    slot_names = [symbols.define_name(f"layout.slot_{number}", CSS_SCOPE) for number in range(1, slot_count + 1)]
    return [id_of(ids_by_name, name) for name in slot_names]


def vanilla_arrays(symbols, css_text, num_characters):
    arrays = {}
    for field, label in VANILLA_TABLE_LABELS.items():
        values = symbols.table_values(read_table_entries(css_text, label), CSS_SCOPE)
        if len(values) != VANILLA_ENTRY_COUNT:
            raise CssParseError(
                f"{label} has {len(values)} entries, expected {VANILLA_ENTRY_COUNT} (ids 0x00..0x{roster_source.CLONE_ID:02X})"
            )
        filler = 0.0 if field == "circle_size" else 0
        arrays[field] = values + [filler] * (num_characters - len(values))
    return arrays


def portrait_id_array(num_characters, slots, ids_by_name, bonus_portrait, random_portrait):
    """Mirror the portrait_id_table loop in CharacterSelect.asm, before the add_to_css overrides."""
    boss_id = ids_by_name["BOSS"]
    polygon_base = ids_by_name["NMARIO"]
    portrait_ids = [bonus_portrait] * num_characters
    for index, slot_id in enumerate(slots):
        portrait_ids[slot_id] = index
        if slot_id == ids_by_name["MARIO"]:
            portrait_ids[ids_by_name["METAL"]] = index
        if slot_id == ids_by_name["DONKEY"]:
            portrait_ids[ids_by_name["GDONKEY"]] = index
        if slot_id < boss_id:
            portrait_ids[slot_id + polygon_base] = index
        if slot_id == ids_by_name["LINK"]:
            portrait_ids[boss_id] = index  # Upstream writes BOSS for the LINK slot (the "Masterhand" block); kept as-is.
    portrait_ids[ids_by_name["PLACEHOLDER"]] = random_portrait
    portrait_ids[ids_by_name["NONE"]] = 0
    return portrait_ids


def apply_add_to_css(symbols, arrays, css_text, num_characters):
    """Overlay the per-character values written by the add_to_css calls for the added ids."""
    calls = add_to_css_calls(css_text)
    written_ids = set()
    for args in calls:
        char_id = symbols.evaluate(args[0], CSS_SCOPE)
        if char_id >= num_characters or char_id in written_ids:
            raise CssParseError(f"add_to_css id {char_id} is out of range or repeated")
        written_ids.add(char_id)
        arrays["announce_fgm"][char_id] = symbols.evaluate(args[1], CSS_SCOPE)
        arrays["circle_size"][char_id] = float(args[2])
        arrays["selection_action"][char_id] = symbols.evaluate(args[3], CSS_SCOPE)
        arrays["logo_offset"][char_id] = symbols.evaluate("series_logo.offset." + args[4], CSS_SCOPE)
        arrays["name_offset"][char_id] = symbols.evaluate(args[5], CSS_SCOPE)
        arrays["portrait_offset"][char_id] = symbols.evaluate(args[6], CSS_SCOPE)
        override = symbols.evaluate(args[7], CSS_SCOPE)
        if override != NO_PORTRAIT_OVERRIDE:
            arrays["portrait_id"][char_id] = override
    if len(calls) != num_characters - VANILLA_ENTRY_COUNT:
        raise CssParseError(
            f"found {len(calls)} add_to_css calls, expected {num_characters - VANILLA_ENTRY_COUNT}"
        )


def slide_arrays(symbols, css_text, layout):
    """Return slide-in x positions (computed as the asm evaluate loop does) and velocities."""
    start = layout["START_VISUAL"] + layout["START_X"]
    columns = layout["NUM_COLUMNS"]
    x_positions = [float(start + layout["PORTRAIT_WIDTH"] * column) for column in range(columns)]
    x_positions.append(float(start - layout["BOOKEND_WIDTH"]))
    x_positions.append(float(start + layout["PORTRAIT_WIDTH"] * columns))
    velocities = symbols.table_values(read_table_entries(css_text, "portrait_velocity"), CSS_SCOPE)
    if len(velocities) != columns + 2:
        raise CssParseError(f"portrait_velocity has {len(velocities)} entries, expected {columns + 2}")
    return x_positions, velocities


def bonus_char_ids(symbols, css_text, expected_count):
    values = symbols.table_values(read_table_entries(css_text, "bonus_chars"), CSS_SCOPE)
    if len(values) != expected_count:
        raise CssParseError(f"bonus_chars has {len(values)} entries, expected {expected_count}")
    return values


def decode_high_half_float(bits):
    return struct.unpack(">f", struct.pack(">I", bits << 16))[0]


def assemble_css_data(remix_src):
    texts = {name: roster_source.read_text(remix_src / "src" / name) for name in SOURCE_FILES}
    symbols = AsmSymbols()
    for name in SOURCE_FILES:
        symbols.add_source(texts[name])
    add_define_character_ids(symbols, texts["Character.asm"])
    css_text = texts["CharacterSelect.asm"]
    ids_by_name = character_ids_by_name(symbols)
    layout = {name: symbols.evaluate(name, CSS_SCOPE) for name in LAYOUT_CONSTANTS}
    num_characters = symbols.evaluate("Character.NUM_CHARACTERS", ROOT_SCOPE)
    slots = slot_ids(symbols, layout["NUM_SLOTS"], ids_by_name)
    arrays = vanilla_arrays(symbols, css_text, num_characters)
    arrays["portrait_id"] = portrait_id_array(
        num_characters, slots, ids_by_name, layout["BOOKEND_BONUS_PORTRAIT"], layout["BOOKEND_RANDOM_PORTRAIT"]
    )
    apply_add_to_css(symbols, arrays, css_text, num_characters)
    slide_x, slide_velocity = slide_arrays(symbols, css_text, layout)
    return {
        "layout": layout,
        "num_characters": num_characters,
        "file_ids": {
            "portraits": symbols.evaluate("File.CHARACTER_PORTRAITS", ROOT_SCOPE),
            "images": symbols.evaluate("File.CSS_IMAGES", ROOT_SCOPE),
        },
        "bonus_bookend_offset": symbols.evaluate("portrait_offsets.BONUS_BOOKEND", CSS_SCOPE),
        "random_bookend_offset": symbols.evaluate("portrait_offsets.RANDOM_BOOKEND", CSS_SCOPE),
        "portrait_scale_bits": symbols.evaluate("PORTRAIT_SCALE", CSS_SCOPE),
        "bonus_ids": bonus_char_ids(symbols, css_text, layout["NUM_BONUS_CHARS"]),
        "slot_ids": slots,
        "arrays": arrays,
        "slide_x": slide_x,
        "slide_velocity": slide_velocity,
        "ids_by_name": ids_by_name,
        "names_by_id": names_by_id(ids_by_name),
    }


def expect_value(label, actual, expected):
    if actual != expected:
        raise CssParseError(f"{label}: got 0x{actual:X}, expected 0x{expected:X}")


def check_css_data(data):
    """Fail loudly if the extracted CSS data contradicts the values the port depends on."""
    names = data["names_by_id"]
    if data["num_characters"] != roster_source.ROSTER_SIZE:
        raise CssParseError(
            f"NUM_CHARACTERS is {data['num_characters']}, roster expects {roster_source.ROSTER_SIZE}"
        )
    slots = data["slot_ids"]
    repeated = sorted({slot for slot in slots if slots.count(slot) > 1})
    if repeated:
        listed = ", ".join(f"0x{slot:02X} {names[slot]}" for slot in repeated)
        raise CssParseError(f"slot ids repeat in the CSS layout: {listed}")
    portrait_offsets = data["arrays"]["portrait_offset"]
    name_offsets = data["arrays"]["name_offset"]
    for char_id in slots + data["bonus_ids"]:
        if portrait_offsets[char_id] == 0 or name_offsets[char_id] == 0:
            raise CssParseError(
                f"id 0x{char_id:02X} {names[char_id]} has a zero portrait or name offset"
            )
    ids = data["ids_by_name"]
    expect_value("MARIO portrait offset", portrait_offsets[ids["MARIO"]], EXPECTED_MARIO_PORTRAIT_OFFSET)
    expect_value("CRASH portrait offset", portrait_offsets[ids["CRASH"]], EXPECTED_CRASH_PORTRAIT_OFFSET)
    expect_value("BONUS_BOOKEND offset", data["bonus_bookend_offset"], EXPECTED_BONUS_BOOKEND_OFFSET)
    expect_value("RANDOM_BOOKEND offset", data["random_bookend_offset"], EXPECTED_RANDOM_BOOKEND_OFFSET)
    expect_value("CHARACTER_PORTRAITS file id", data["file_ids"]["portraits"], EXPECTED_PORTRAITS_FILE_ID)
    expect_value("CSS_IMAGES file id", data["file_ids"]["images"], EXPECTED_IMAGES_FILE_ID)
    scale = decode_high_half_float(data["portrait_scale_bits"])
    if scale != EXPECTED_PORTRAIT_SCALE:
        raise CssParseError(f"PORTRAIT_SCALE decodes to {scale}, expected {EXPECTED_PORTRAIT_SCALE}")


def c_float(value):
    return f"{float(value)!r}f"


def c_hex(value):
    return f"0x{value:X}"


def render_define(name, value):
    return f"#define {name} {value}"


def render_array(c_type, name, length_macro, values, format_value, labels):
    rows = [f"static const {c_type} {name}[{length_macro}] = {{"]
    rows += [f"    {format_value(value)},  /* {label} */" for value, label in zip(values, labels)]
    rows.append("};")
    return rows


HEADER_PROLOGUE = """/*
 * Generated by tools/gen_remix_css.py from the public Smash Remix source
 * (src/CharacterSelect.asm, Character.asm, FGM.asm, File.asm). Do not edit by hand; re-run the script.
 *
 * Per-character arrays are indexed by Remix character id (see remix_roster_data.h).
 * Portrait ids are CSS portrait indices: 0..29 for slots, 30 bonus bookend, 31 random bookend.
 * Offsets are byte offsets into the reloc file named by the matching *_FILE_ID macro.
 */
#ifndef REMIX_CSS_DATA_H
#define REMIX_CSS_DATA_H

#include <stddef.h>
#include <stdint.h>
"""

PER_CHARACTER_ARRAYS = (
    ("kRemixCssPortraitOffset", "uint32_t", "portrait_offset", c_hex),
    ("kRemixCssNameOffset", "uint32_t", "name_offset", c_hex),
    ("kRemixCssLogoOffset", "uint32_t", "logo_offset", c_hex),
    ("kRemixCssAnnounceFgm", "uint16_t", "announce_fgm", c_hex),
    ("kRemixCssCircleSize", "float", "circle_size", c_float),
    ("kRemixCssSelectionAction", "uint32_t", "selection_action", c_hex),
    ("kRemixCssPortraitId", "uint8_t", "portrait_id", c_hex),
)


def render_defines(data):
    layout = data["layout"]
    defines = [
        ("REMIX_CSS_NUM_CHARACTERS", str(data["num_characters"])),
        ("REMIX_CSS_NUM_SLIDE_PORTRAITS", str(len(data["slide_x"]))),
    ]
    defines += [(f"REMIX_CSS_{name}", str(layout[name])) for name in LAYOUT_CONSTANTS]
    defines += [
        ("REMIX_CSS_PORTRAIT_SCALE", c_float(decode_high_half_float(data["portrait_scale_bits"]))),
        ("REMIX_CSS_PORTRAITS_FILE_ID", c_hex(data["file_ids"]["portraits"])),
        ("REMIX_CSS_IMAGES_FILE_ID", c_hex(data["file_ids"]["images"])),
        ("REMIX_CSS_BONUS_BOOKEND_OFFSET", c_hex(data["bonus_bookend_offset"])),
        ("REMIX_CSS_RANDOM_BOOKEND_OFFSET", c_hex(data["random_bookend_offset"])),
    ]
    return [render_define(name, value) for name, value in defines]


def render_slide_and_slot_arrays(data):
    names = data["names_by_id"]
    slot_labels = [f"slot {number} {names[slot]}" for number, slot in enumerate(data["slot_ids"], start=1)]
    bonus_labels = [f"bonus {number} {names[bonus]}" for number, bonus in enumerate(data["bonus_ids"], start=1)]
    slide_labels = [f"column {column}" for column in range(1, data["layout"]["NUM_COLUMNS"] + 1)]
    slide_labels += ["left bookend", "right bookend"]
    return [
        *render_array("uint8_t", "kRemixCssSlotIds", "REMIX_CSS_NUM_SLOTS", data["slot_ids"], c_hex, slot_labels),
        *render_array("uint8_t", "kRemixCssBonusIds", "REMIX_CSS_NUM_BONUS_CHARS", data["bonus_ids"], c_hex, bonus_labels),
        *render_array("float", "kRemixCssPortraitX", "REMIX_CSS_NUM_SLIDE_PORTRAITS", data["slide_x"], c_float, slide_labels),
        *render_array(
            "float", "kRemixCssPortraitVelocity", "REMIX_CSS_NUM_SLIDE_PORTRAITS", data["slide_velocity"], c_float, slide_labels
        ),
    ]


def render_per_character_arrays(data):
    names = data["names_by_id"]
    labels = [f"0x{char_id:02X} {names[char_id]}" for char_id in range(data["num_characters"])]
    rows = []
    for c_name, c_type, field, format_value in PER_CHARACTER_ARRAYS:
        rows += render_array(c_type, c_name, "REMIX_CSS_NUM_CHARACTERS", data["arrays"][field], format_value, labels)
    return rows


def render_header(data):
    sections = [
        HEADER_PROLOGUE.rstrip("\n"),
        "\n".join(render_defines(data)),
        "\n".join(render_slide_and_slot_arrays(data)),
        "\n".join(render_per_character_arrays(data)),
        "#endif /* REMIX_CSS_DATA_H */",
        "",
    ]
    return "\n\n".join(sections)


def main(argv):
    remix_src = Path(argv[1]) if len(argv) > 1 else DEFAULT_REMIX_SRC
    data = assemble_css_data(remix_src)
    check_css_data(data)
    OUTPUT_HEADER.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_HEADER.write_text(render_header(data), encoding="utf-8")
    print(f"wrote {OUTPUT_HEADER} with {len(data['slot_ids'])} slots, {len(data['bonus_ids'])} bonus ids, "
          f"{data['num_characters']} characters")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main(sys.argv))
    except (CssParseError, roster_source.RosterParseError, OSError) as error:
        print(f"gen_remix_css: {error}", file=sys.stderr)
        sys.exit(1)
