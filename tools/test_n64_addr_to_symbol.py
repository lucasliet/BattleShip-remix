#!/usr/bin/env python3
"""Unit tests of tools/n64_addr_to_symbol.py on synthetic sources (no repository files needed).

Usage: python3 tools/test_n64_addr_to_symbol.py
"""

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import n64_addr_to_symbol as tool

SAMPLE = """
// 0x80131580
IFPlayerCommon gIFCommonPlayerInterface;

// 0x80138B70
void mnVSResultsFuncStart(void)
{
}

s32 *unused_array[] = { dMNVSResultsUnused0x80138EF0 };
"""


class ParseAddressTest(unittest.TestCase):
    def test_accepts_prefixed_and_bare_hex(self):
        self.assertEqual(0x80131580, tool.parse_address("0x80131580"))
        self.assertEqual(0x80131580, tool.parse_address("80131580"))

    def test_rejects_text_with_the_offending_value(self):
        with self.assertRaises(ValueError) as caught:
            tool.parse_address("zz")
        self.assertIn("'zz'", str(caught.exception))


class ScanTest(unittest.TestCase):
    def owners(self):
        return list(tool.scan_source_text(SAMPLE, Path("sample.c")))

    def test_comment_owner_is_the_next_declaration(self):
        names = {owner[0]: owner[1] for owner in self.owners()}
        self.assertEqual("gIFCommonPlayerInterface", names[0x80131580])
        self.assertEqual("mnVSResultsFuncStart", names[0x80138B70])

    def test_address_inside_a_name_is_an_owner(self):
        names = {owner[0]: owner[1] for owner in self.owners()}
        self.assertEqual("dMNVSResultsUnused0x80138EF0", names[0x80138EF0])

    def test_blank_and_comment_lines_do_not_name_a_declaration(self):
        self.assertIsNone(tool.declared_name(""))
        self.assertIsNone(tool.declared_name("// text"))
        self.assertIsNone(tool.declared_name("#define X 1"))


class LookupTest(unittest.TestCase):
    def test_exact_match_has_zero_distance(self):
        owners = [(0x80131580, "gIFCommonPlayerInterface", Path("a.c"), 2)]
        self.assertEqual([(owners[0], 0)], tool.lookup(owners, 0x80131580))

    def test_near_lists_owners_below_within_range(self):
        owners = [(0x80131580, "gIFCommonPlayerInterface", Path("a.c"), 2)]
        result = tool.lookup(owners, 0x80131588, near=0x10)
        self.assertEqual(8, result[0][1])
        self.assertEqual([], tool.lookup(owners, 0x80131588, near=0))

    def test_unknown_address_is_reported_as_not_found(self):
        self.assertIn("no owner found", tool.format_result(0x80000000, []))


if __name__ == "__main__":
    unittest.main()
