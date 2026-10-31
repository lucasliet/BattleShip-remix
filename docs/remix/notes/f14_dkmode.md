# F14 DK Mode notes (DKMode.asm)

## Files

- `decomp/src/ft/ftremix/ftremixdkmode.c` and `.h` (new, under `PORT && REGION_REMIX`).
- Entry point: `void ftRemixDKModeResizeHead(FTStruct *fp)`.

## Toggle

No entry of `port/remix/remix_toggles.cpp` drives this setting. `DKMode.dk_mode_enabled` (RAM 0x80517714) is written
only by the 12cb portrait easter egg (`_check_portraits`, asm lines 155-216). Values: 0 off, 1 DK Mode, 2 Lanky Mode.
`gm/gmremixtoggle.h` was not found at that path and was not searched further.

## Vanilla hook (wiring for the lead)

- RAM 0x800F296C, rom 0x6E16C, inside `ftDisplayMainProcDisplay` (`decomp/src/ft/ftdisplaymain.c:1115`).
- The asm replaces the two instructions `lui s3,0x8013; addiu s3,s3,0x12F0` (og lines) at 0x800F296C-0x800F2970 and
  returns to 0x800F2974. Before the hook, `$fp = fp` (`lw $fp, 0x84($a0)`, i.e. `ftGetStruct`).
- Insert, under `#ifdef REGION_REMIX`, immediately after line 38 (`fp = ftGetStruct(fighter_gobj);`):

```c
#ifdef REGION_REMIX
    ftRemixDKModeResizeHead(fp);
#endif
```

- Add the include `#include "ft/ftremix/ftremixdkmode.h"` to ftdisplaymain.c (or to `ftremixhooks.h`).
- The C call is an insertion only: nothing vanilla is removed. The og lines (`lui s3`/`addiu s3`, which the asm
  re-executes at `_normal`) are plain address loads and the C statements that follow them are unchanged.

## Port behaviour

- Setting 0: nothing. Setting 2: head scale 0.5F (0x3F00 << 16). Any other non-zero value: head scale 1.5F, as the asm
  (`beql t7,t6` only branches for 2).
- Arm scale 1.5F (0x3FC0 << 16). The first joint of the list gets the mode scale, every later joint the arm scale.
  In Lanky Mode the first joint is 0.5F and the rest 1.5F, as the asm does.
- Skipped fighters: REMIX_ID_BOSS (0x0C, Master Hand per the asm comment) and REMIX_ID_SANDBAG.
- Joint list: `fp->joints[nFTPartsJointCommonStart + bone]` (asm base 0x8F8 = `joints[4]`, bone index 0 = joint 4,
  so bone 1 = `joints[5]` is the head as the asm comment says). The DObj scale is `scale.vec.f.{x,y,z}` (DObj+0x40..0x48).
- Table: `Character.fighter_DK_mode.table` (RAM 0x8049E6FC) read at run time through `remix_ram_image_read_word/byte`,
  one 4-byte entry per fkind (index `fkind*4`), zero-terminated bone list, empty entry falls back to the table base.
  The table is not ported as static C data because its default entry and the Character.asm definition were not in
  the files read; the patches in DKMode.asm (lines 78-153) are data already in the Remix RAM image.

## Deviations and gaps

- NULL joint DObj is skipped; the asm would dereference it. Bone index beyond `joints[37]` is skipped (asm has no bound).
- `REMIX_SYM_DKMode__dk_mode_enabled` and `REMIX_SYM_Character__fighter_DK_mode__table` are not in
  `port/remix/remix_symbols.h`. The file uses `#ifndef` fallbacks with the remix.sym addresses. The lead should add
  them to the generated symbols and the fallbacks can then go.
- Not verified: the table content for Mario (table base) and the other entries, since they are only in the Remix ROM;
  the animation-driven scaling (the asm TODO says the hook does not follow animation scale); in-game result.
- Syntax check only (`scripts/remix/syntax-check.sh`): OK for the `.c` and `.h`. No build, no game.
