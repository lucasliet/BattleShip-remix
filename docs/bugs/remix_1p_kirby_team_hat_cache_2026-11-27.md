# Remix 1P Kirby Team: stale hat file cache and powers without their owner's files (2026-11-27)

Issue: `lucasliet/BattleShip-remix#10` (Windows, v2.0.1-port.13): the game crashed on the Kirby Team stage of Remix 1P
when an enemy Kirby used its neutral special. The minidump stops in `Fast::Interpreter::GfxSpVertex` with a vertex
address that is not a pointer (`0x0001124f10000138`).

Code: `decomp/src/sys/taskman.c` (`syTaskmanStartTask`), `decomp/src/sc/scmanager.c` (scene 52 boot shortcut), `decomp/src/ft/ftremix/ftremixkirbyhats.c`
(`ftRemixKirbyHatsClearCache`), `decomp/src/sc/sc1pmode/sc1premix1p.c` (`sc1PRemix1PLoadKirbyPowerFiles`),
`sc1premix1prules.c` (`sc1PRemix1PKirbyPowerHasFiles`), `decomp/src/sc/sc1pmode/sc1pgame.c` (Kirby stage file setup).
Unit test: `tests/remix/test_1p_remix_rules.c`.

## Symptoms

Two separate defects, both only on the Kirby Team stage of Remix 1P:

1. The first Kirbys drew a hat with no body, then the renderer walked data that is not a display list
   (`Unhandled OP code`, SIGSEGV or SIGBUS in `Fast::Interpreter::Run`, `GfxSpVertex` on Windows). On macOS it crashed
   about 5 seconds into the stage; where it crashes depends on what the stage allocated over the old file.
2. With the first defect out of the way, a Kirby whose power makes a weapon crashed on its neutral special:

```
SIGSEGV fault_addr=0x10
portFixupStructU16
wpManagerMakeWeapon
ftRemixBanjoNSPCreateEgg
ftRemixBanjoNSPSpawnForwardEgg
```

## What the report shows

The video of the issue ends with the game frozen at 04:28 of the stage. Port 1 had just received a new Kirby (its damage
went from 33% to 0%) and the Kirby on port 2 wore the Ebisumaru hood and held a charged weapon with a star flash. The
reporter read that as Conker's slingshot. It is Ebisumaru's own copied special (`EbiNSP`, Kirby actions 417, 418, 423):
the mallet flashes when the charge is full, as the VS check `SSB64_REMIX_KIRBY_COPY_TEST=70` with B held shows, while
the Conker copy (`=56`) wears Conker's hat. Hat and power are both taken from the copy id, in the asm and in the port, so
there is no mismatched pair. That special reads no file of its owner and runs without Ebisumaru in the match, so it is
not what crashed: the renderer crash fits the first defect below, at the first draw of the Kirby that had just spawned.
The release has no PDB, so the dump could not be symbolized to confirm it.

## Root causes

1. **The hat file cache outlived the scene arena.** `ftRemixKirbyHatModelPart` loads the Remix hat files
   (`KIRBY_CUSTOM_HATS`, `_2`) on first use into the scene arena and keeps the pointers in `sFTRemixKirbyHatsFiles`. The
   cache was cleared in `scManagerRunLoop`, which runs once per top-level scene. A 1P run stays inside
   `sc1PManagerUpdateScene`: the title card (scene 14, which shows the Kirby team with its hats and so loads the file)
   and the stage (scene 52) are two tasks of the same top-level scene. The stage reused the pointer of the title card,
   whose arena `syTaskmanStartTask` had already wiped and handed to the stage. Any 1P stage after a scene that drew a
   Remix hat was exposed, not only the Kirby Team.
2. **The owners of the powers had no files.** A copied special reads its weapon and item data from the files of the
   fighter that owns the power (see `remix_kirby_hat_power_files_2026-11-06.md`). Vanilla loads file 230 on the Kirby
   stage for the seven vanilla powers; `SinglePlayerModes.asm` `remix_1p_kirby_load` (0x8018F970) also loads file
   0xC1B, which pulls in the files of every Remix power. The port had no counterpart, so the weapon descriptor resolved
   against a NULL file.

## Fix

- `syTaskmanStartTask` calls `ftRemixKirbyHatsClearCache` where it wipes the scene arena (next to
  `efManagerPortClearFileRefs`), so the cache lives exactly as long as the memory it points into. The call in
  `scManagerRunLoop` is gone.
- The Kirby stage setup of `sc1PGameFuncStart` calls `sc1PRemix1PLoadKirbyPowerFiles`, which runs
  `ftManagerSetupFilesAllKind` for the power of each of the eight Kirbys. The Magic Hat power (0x1C) is skipped: it has
  no file table and its rolls load their own files.

- The boot shortcut `SSB64_START_SCENE=52` now calls `sc1PRemix1PRandomize` and `sc1PRemixAllstarRandomize`. Only the 1P
  CSS called them, so `SSB64_REMIX_1P_MODE=remix1p` or `allstar` through the shortcut left both modes inactive and the
  vanilla stage ran (no `remix1p:` line in the log). Checks made that way since 2026-10-27 did not exercise those two
  modes: `remix_1p_size_override_cp6f_2026-10-27.md` and the Records and Rest Area recipes of
  `docs/remix/modes/singleplayer.md`. The other modes (`hrc`, `multiman`, `cruel`, `bonus3`) need no draw.

## Verification

Run: `SSB64_SPGAME_STAGE=8 SSB64_REMIX_1P_MODE=remix1p SSB64_REMIX_FIGHTER_LOG=1 scene.sh 17 inputs/css1p_pick_start.txt <name> 8000`
(through the 1P CSS, the only path that drew the Remix path when the bug was found).

| Build | Result |
|---|---|
| Before | `rc=139`, `Unhandled OP code` and a crash in `Fast::Interpreter::Run` near frame 835; one `[remix-kirbyhat] loaded file` line for two scenes |
| Cache fix only | Kirbys draw correctly; `rc=139` in `wpManagerMakeWeapon` from `ftRemixBanjoNSPCreateEgg` at battle tic 1336 |
| Both fixes | `rc=0` over 8000 frames, the hat file loaded once per scene, the Kirbys ran their copied specials (Kirby actions 223 to 415) |

With the shortcut fixed, `scene.sh 52` with `SSB64_REMIX_1P_MODE=remix1p` logs the path and runs the Kirby stage
(`SSB64_SPGAME_STAGE=8`, 5000 frames) and each of the other 13 stages (2500 frames) with `rc=0`; `allstar` logs its run
and `hrc` and a plain boot are unchanged.

The checks that had used the inactive shortcut were run again through the fixed one, all with `rc=0`:

| Check | Result |
|---|---|
| Giant stage (`SSB64_SPGAME_STAGE=6`) | one fighter with match state 1 (`lldb_count.sh`, `ret:ftRemixSizeGetMatchState`), drawn giant with its two allies |
| Tiny Team (`SSB64_SPGAME_STAGE=9`) | three fighters with match state 2 |
| All-Star Rest Area | one portrait at progress 0, two with `SSB64_ALLSTAR_PROGRESS=4`, three with `18` |
| Records | a cleared run (`SSB64_SPGAME_STAGE=13 SSB64_SPGAME_AUTOWIN=400`, `inputs/allstar_a_presses.txt`, 12000 frames) shows its score on the Remix 1P CSS, and the standard 1P CSS on the same save still shows 0 |

The record is written after the score screens: a run with no input stops on the stage clear screen and saves nothing.

The standard 1P Kirby stage (`SSB64_REMIX_1P_MODE=standard`), the VS Kirby copy of Banjo and a VS Kirby spawning with
the Banjo hat still run with `rc=0`.

## Audit hook

A port-side cache that holds a pointer into the scene arena must be reset in `syTaskmanStartTask`, not in
`scManagerRunLoop`: the 1P run, and any other manager that starts several tasks, never returns to that loop between
them. `itRemixClearActiveItems` and `gmRemixModeSceneClear` stay in `scManagerRunLoop` because they hold state, not
arena pointers.
