#!/usr/bin/env python3
"""Emits the Smash Remix staff roll tables under decomp/src/credits/remix/.

Input: decomp/src/credits/remix/remix_credits.txt (tab separated, see the header of that file).
Base: the vanilla encoded and metadata files in decomp/src/credits/. Remix entries are appended after
the vanilla entries, so every index of the vanilla staff roll keeps its meaning.

Run from the repository root: uv run tools/gen_remix_credits.py
"""
import sys
from dataclasses import dataclass, field
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CREDITS_DIR = REPO_ROOT / "decomp" / "src" / "credits"
REMIX_DIR = CREDITS_DIR / "remix"
SOURCE_PATH = REMIX_DIR / "remix_credits.txt"

WHITESPACE = 0xFFFFFFDF
LINE_BREAK = 0xFFFFFFC9

SCROLL_SYMBOLS = {".": 0x34, ",": 0x35, "4": 0x37}
DETAILS_SYMBOLS = {
    ":": 0x34,
    "(": 0x47,
    ")": 0x48,
    ".": 0x3F,
    "-": 0x40,
    ",": 0x41,
    "&": 0x42,
    '"': 0x43,
    "/": 0x44,
    "'": 0x45,
    "?": 0x46,
}

COMPANY_KEYS = {"remix": "nSCStaffrollCompanyRemix", "none": "nSCStaffrollCompanyNull"}

unmapped_scroll_glyphs = set()
unmapped_details_glyphs = set()


@dataclass
class Section:
    title: str
    names: list = field(default_factory=list)


@dataclass
class CreditsSource:
    company_text: str
    sections: list


def parse_source(path: Path) -> CreditsSource:
    company_text = None
    sections = []
    for number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
        if not line:
            continue
        fields = line.split("\t")
        if fields[0] == "company":
            company_text = fields[1]
        elif fields[0] == "@":
            sections.append(Section(title=fields[1]))
        else:
            if len(fields) != 3 or fields[2] not in COMPANY_KEYS:
                sys.exit(f"{path}:{number}: expected name, details, company ({sorted(COMPANY_KEYS)}), got {line!r}")
            if not sections:
                sys.exit(f"{path}:{number}: name before the first '@' title line")
            sections[-1].names.append((fields[0], fields[1], fields[2]))
    if company_text is None:
        sys.exit(f"{path}: missing 'company' line")
    return CreditsSource(company_text=company_text, sections=sections)


def encode_scroll_char(char: str) -> int:
    if "A" <= char <= "Z":
        return ord(char) - ord("A")
    if "a" <= char <= "z":
        return ord(char) - ord("a") + 26
    if char in SCROLL_SYMBOLS:
        return SCROLL_SYMBOLS[char]
    if char != " ":
        unmapped_scroll_glyphs.add(char)
    return WHITESPACE


def encode_details_char(char: str) -> int:
    if "A" <= char <= "Z":
        return ord(char) - ord("A")
    if "a" <= char <= "z":
        return ord(char) - ord("a") + 26
    if "0" <= char <= "9":
        return 0x6E - ord(char)
    if char == "|":
        return LINE_BREAK
    if char == " ":
        return WHITESPACE
    if char in DETAILS_SYMBOLS:
        return DETAILS_SYMBOLS[char]
    if char != " ":
        unmapped_details_glyphs.add(char)
    return WHITESPACE


def encode_text(text: str, encode_char) -> list:
    return [encode_char(char) for char in text]


def read_vanilla_encoded(name: str) -> list:
    text = (CREDITS_DIR / f"{name}.credits.encoded").read_text(encoding="utf-8")
    return [value.strip() for value in text.strip().split(",") if value.strip()]


def read_vanilla_metadata(name: str) -> list:
    return (CREDITS_DIR / f"{name}.credits.metadata").read_text(encoding="utf-8").splitlines()


def hex_value(value: int) -> str:
    return hex(value)


class EncodedTable:
    """Vanilla entries followed by Remix entries for one encoded credits table."""

    def __init__(self, name: str):
        self.name = name
        self.values = read_vanilla_encoded(name)
        self.metadata = read_vanilla_metadata(name)
        self.vanilla_entry_count = len(self.metadata)

    def append(self, values: list) -> int:
        start = len(self.values)
        self.values.extend(hex_value(value) for value in values)
        self.metadata.append(f"{start},{len(values)},")
        return len(self.metadata) - 1

    def write(self) -> None:
        (REMIX_DIR / f"{self.name}.credits.encoded").write_text(",".join(self.values) + ",\n", encoding="utf-8")
        (REMIX_DIR / f"{self.name}.credits.metadata").write_text("\n".join(self.metadata) + "\n", encoding="utf-8")


def build_tables(source: CreditsSource) -> dict:
    tables = {name: EncodedTable(name) for name in ("staff", "titles", "info", "companies")}
    job_lines = []
    company_id_lines = []
    total_names = tables["staff"].vanilla_entry_count
    for section in source.sections:
        if not section.names:
            sys.exit(f"title {section.title!r} has no names; the staff roll would never advance past it")
        title_index = tables["titles"].append(encode_text(section.title, encode_scroll_char))
        for name, details, company in section.names:
            tables["staff"].append(encode_text(name, encode_scroll_char))
            tables["info"].append(encode_text(details, encode_details_char))
            company_id_lines.append(COMPANY_KEYS[company])
        total_names += len(section.names)
        job_lines.append((title_index, total_names))
    tables["companies"].append(encode_text(source.company_text, encode_details_char))
    return tables, job_lines, company_id_lines


def write_job_lines(job_lines: list) -> None:
    body = [f"\t{{ -1, {title_index}, {section_end} }}," for title_index, section_end in job_lines]
    write_include("jobs.credits.inc", body)


def write_company_id_lines(company_id_lines: list) -> None:
    write_include("companyids.credits.inc", [f"\t{key}," for key in company_id_lines])


def write_include(file_name: str, body: list) -> None:
    header = "/* Generated by tools/gen_remix_credits.py from remix_credits.txt. Do not edit. */"
    (REMIX_DIR / file_name).write_text("\n".join([header] + body) + "\n", encoding="utf-8")


def main() -> None:
    REMIX_DIR.mkdir(parents=True, exist_ok=True)
    source = parse_source(SOURCE_PATH)
    tables, job_lines, company_id_lines = build_tables(source)
    for table in tables.values():
        table.write()
    write_job_lines(job_lines)
    write_company_id_lines(company_id_lines)
    print(f"sections {len(source.sections)}, names {len(company_id_lines)}")
    print(f"scroll glyphs without a vanilla sprite (REMIX-TODO credits-glyphs): {''.join(sorted(unmapped_scroll_glyphs))!r}")
    print(f"details glyphs the Remix asm also drops to a space: {''.join(sorted(unmapped_details_glyphs))!r}")


if __name__ == "__main__":
    main()
