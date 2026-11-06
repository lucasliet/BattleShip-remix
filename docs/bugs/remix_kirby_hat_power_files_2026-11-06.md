# Kirby's '???' magic hat crashed when it rolled the power of a fighter not in the match

Code: `decomp/src/ft/ftremix/ftremixmagichat.c` (`ftRemixMagicHatPickNewPower`), `decomp/src/ft/ftremix/ftremixkirbyhats.c`
(`ftRemixKirbyHatSpawnInit`). Test: `lldb_count.sh` runs `r3_ft29_magichat` and `r4_ft29_magichat`
(`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

Found by the Phase 5b check of FT-29 (2026-11-06): a VS match on Final Destination, Kirby (P1) against Mario, with the CSS
panel option Kirby Hat set to `???` (`SSB64_REMIX_PANEL='KirbyHat.1=41'`) and the neutral special every 150 frames
(`nsp_tap.txt`). The second roll gave Link's power and the boomerang crashed:

```
EXC_BAD_ACCESS (address=0x10)
portFixupStructU16(base=NULL, byte_offset=16)       lbreloc_byteswap.cpp:1066
wpManagerMakeWeapon                                 wpmanager.c:130
wpLinkBoomerangMakeWeapon                           wplinkboomerang.c:594
ftKirbyCopyLinkSpecialNMakeBoomerang                ftkirbycopylinkspecialn.c:39
```

## Root cause

A copied special reads its weapon or item data from the files of the fighter that owns the power (the boomerang reads
`gFTDataLinkSpecial1`). Those files are loaded with the owner's main file (`ftManagerSetupFilesAllKind`), which a battle
scene does only for the fighters in the match. In vanilla a Kirby can only copy a fighter that is in the match, so the
files are always there. The Remix magic hat (`Kirby.asm` `kirby_magic_hat`, `_get_random_hat`) rolls any fighter of
`KirbyHats.spawn_with_table_`, and the Kirby Hat spawn option gives any power from the start, so the owner may have no
files: the weapon descriptor resolved to a NULL file.

## Fix

The magic hat roll and the Kirby Hat spawn call `ftManagerSetupFilesAllKind` for the fighter whose power Kirby takes.
The function returns at once when that fighter's main file is already loaded, and a Remix fighter also loads its parent,
whose routines the special runs. The spawn skips the call for the `???` value: its panel fighter is `REMIX_ID_NONE`
(0x1C), which has no file table (`r4_ft29_magichat` crashed in `ftManagerSetupFilesAllKind(fkind=28)` before that guard).

## Verification

`r5_ft29_magichat` (same setup: 8 rolls, the boomerang made twice) and `r4_kirbyhat_link` (Kirby Hat = Link, no Link in
the match) run to the end without a crash; see the FT-29 row of `docs/remix/patch_audit.md`.
