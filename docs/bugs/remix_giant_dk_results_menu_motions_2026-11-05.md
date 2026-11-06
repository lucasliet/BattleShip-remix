# VS results: Giant DK winner crash and the empty menu motions of Metal Mario and the polygons

Code: `decomp/src/ft/ftport.c` (`port_fighter_copy_menu_motions`, `port_fighter_share_menu_motions`),
`port/remix/remix_roster.cpp` (`patchVanillaMenuMotions`). Asm: `Character.asm:3573-3592`. Ledger: CT-24.

## Symptom

The lot 5 results sweep (`l5_rs_<fkind>`, one VS stock match per Remix winner) crashed only for Giant DK (fkind 26, the DK
D-pad up variant on the CSS): SIGSEGV with fault address 0x8 in `lbCommonAddFighterPartsFigatree`, from `ftMainSetStatus`
(status 0x10001), from `mnVSResultsInitFightersAll`. `fp->joints[nFTPartsJointTopN]->child` was NULL: the joint tree had
been built (`ftManagerMakeFighter` logged every damage collision) and was gone by the second results status.

## Root cause

The results, CSS and 1P intro poses are opening statuses (0x10000 and up), which read `fp->data->submotion`, the menu motion
array. Vanilla gives Giant DK, N Luigi and N Jigglypuff a one-row array (EggLay) and Metal Mario and the other polygons 15
rows that are empty past row 0. The results screen asks for rows 1 to 5 (Selected, Win1 to Win3, Claps), so Giant DK read
past the end of its array: a garbage `anim_desc` ejected the model parts in the first status and the second one walked a
NULL tree. N Luigi and N Jigglypuff read past their arrays too without crashing in the sweep; the 15-row fighters showed no
victory animation.

Remix fixes the arrays at build time: `copy_menu_actions` copies rows 1 to 13 of the base fighter into Metal Mario (from
Mario) and the 12 polygons (from their base), growing the arrays of N Luigi and N Jigglypuff to 14 rows, and
`origin 0x97A4C + 0x68` points Giant DK's FTData at DK's menu array. The port never applied these patches;
`docs/remix/fighter_table_extraction.md` had recorded the different menu arrays of fkinds 13 to 26 without their origin.

## Fix

`patchVanillaMenuMotions` (run by `remix_roster_load`) makes the same copies on the registered FTData. Deviation: the asm
keeps the vanilla row-count pointer (1 for N Luigi, N Jigglypuff and Giant DK). The port sizes the figatree heap from that
count (`ftManagerSetupFileSize`), so it uses the count of the grown array (Giant DK takes DK's) to keep the victory
animations inside the heap.

## Verification

`ct24_rs_*` (VS stock matches won by Giant DK, DK, Metal Mario and the polygons N Mario, N Luigi, N Kirby, N Jigglypuff, N Ness):
no crash; Giant DK plays DK's victory pose, the others play their base fighter's poses. The results sweep `l5_rs_*` over the 82
Remix winners passes.
