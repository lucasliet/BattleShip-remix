# F10 menu notes: Remix extended credits (menus.md 3.5)

Source: `.toolchain/pkgs/smashremix-src/src/Credits.asm` (1349 lines). Scene: `nSCKindStaffroll` (index 56 in the `SCKind` enum, `decomp/src/sc/scdef.h`); start with `SSB64_START_SCENE` on that id.

## What was ported

Remix entries are appended after the vanilla entries, so every vanilla index keeps its meaning. Everything is gated by `REGION_REMIX`; vanilla builds use the vanilla includes.

- **Text source**: `decomp/src/credits/remix/remix_credits.txt`. Extracted once from Credits.asm (20 titles, 309 names with details and company, 1 company). `\d` in the asm is read as `"` and `\s` as `'`. Lines are `@<TAB>title`, `name<TAB>details<TAB>remix|none`, and `company<TAB>text`. Details keep `|` as the line break.
- **Generator**: `tools/gen_remix_credits.py`. Run it from the repo root with `uv run tools/gen_remix_credits.py`. If uv's default cache is blocked, set `UV_CACHE_DIR` to a writable path first. It writes these, which are versioned:
  - `decomp/src/credits/remix/{staff,titles,info,companies}.credits.{encoded,metadata}`: vanilla base plus Remix entries.
  - `decomp/src/credits/remix/jobs.credits.inc`: 20 Remix job entries `{ -1, title, section_end }`.
  - `decomp/src/credits/remix/companyids.credits.inc`: 309 company ids.
- **Counts after generation**: 84 + 309 = 393 names, 18 + 20 = 38 titles, 12 companies (`nSCStaffrollCompanyRemix` = 11).

## C edits (`decomp/src/sc/sccommon/scstaffroll.c`, `decomp/src/sc/scdef.h`)

- Eight data includes (`dSCStaffrollNameCharacters`, `NameTextInfo`, `JobCharacters`, `JobTextInfo`, `StaffRoleCharacters`, `StaffRoleTextInfo`, `CompanyCharacters`, `CompanyTextInfo`) pick the `credits/remix/` files under `REGION_REMIX`.
- `dSCStaffrollCompanyIDs`: trailing comma added and the Remix include appended.
- `dSCStaffrollJobDescriptions`: the vanilla `Presents` entry ends at name 84 under REGION_REMIX (it was `-1`, which never advances past it, so the Remix jobs were unreachable). The Remix job include follows.
- Fast roll speed (`sSCStaffrollRollSpeed` on Start, asm `0x187EB4`): 0.11F under REGION_REMIX, 0.05F otherwise.
- `scdef.h`: `nSCStaffrollCompanyRemix` added under `REGION_REMIX`.
- The loop end uses `ARRAY_COUNT(dSCStaffrollStaffRoleTextInfo)`, so it picks up the 393 names with no change.

## Asm hooks and what they map to

| Asm | Behaviour | Port status |
|---|---|---|
| `Credits.asm:864-1160` extended title, name, details, company tables | Remix tables appended | Ported (data) |
| `Credits.asm:1164` `use_added_characters_` | Remix scroll glyphs 0x50-0x61 with textures from the Remix credits file. Despite the name, this is glyph data, not fighter credits. Fighter credits are the names. | **REMIX-TODO(credits-glyphs)**. The encoder maps digits, `_ ! - / * ( ) @ ~` to spaces in names and titles. |
| `Credits.asm:1310` `remix_credits_speed_assist_` | Slows the roll at the first Remix name when the 2nd player tag is `"afk"` | **REMIX-TODO(credits-speed-assist)**. It needs the Remix player-tag string table, which the port lacks. |
| `Credits.asm:1164` `use_added_characters_` | Glyph textures (above) | Same as above |
| `Credits.asm:835` `load_files_` | Loads the Remix credits file | Not needed: the port compiles the tables in |

## Judgement calls to check on screen

- Details apostrophe. The asm tests `'\s'` in the details branch and plain `'` falls through to a space. I mapped `'` to 0x45 (the vanilla para glyph), because the Remix credits are meant to show apostrophes ("Let's", "Fray's"). If the lead sees spaces where apostrophes should be, this is the cause.
- Details `!` is dropped to a space, which matches the asm (one entry: "Battle! Champion").
- Scroll names and titles with digits or symbols are approximations (see REMIX-TODO above). Names affected include `smb123w64gb`, `M-1`, `The_Smashfather`, `Jo~o *Johnnyz* Buaes`.

## What to look at on screen

1. The credits run: after "Presents", the Remix titles should appear from "Project Leader" onward, and the last Remix title is "Video Team" and its names end the roll.
2. Names with `_`, digits or `-` show gaps where the glyphs are.
3. Apostrophes in details (see above).
4. Start held to fast-roll: it runs at 0.11, not 0.05. Remix's auto-slow at the first Remix name does not happen (REMIX-TODO).
5. Vanilla credits (non-Remix build) should be unchanged. Only the `#ifdef` branches differ.

## Repo notes

- `decomp/` is a submodule. The C edits and the `credits/remix/` files are in the decomp working tree and need committing there.
- `scstaffroll.c` was modified on disk by another worker during this task. My edits applied on top cleanly; the other changes were not reviewed.
- Syntax check: `scripts/remix/syntax-check.sh` passes for `scstaffroll.c` and `scdef.h` (the pre-existing warnings in `scstaffroll.c` are not mine).
