# F7 hazard worker brief: port the Remix stage hazards of one group to C

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`.

Read first, in this order:

1. `docs/remix/f5_worker_brief.md` (references, translation rules, LP64 rules, `OS.copy_segment` decoding).
2. `docs/remix/f7_leftover_worker_brief.md` (rules: no cmake/build/game/git, syntax-check only, parallel-edit etiquette,
   no ROM bytes in versioned files, no `//` comments that restate code).
3. `docs/remix/stages_and_moveset.md` section 1 (how stages are tables indexed by stage id).
4. `decomp/src/gr/grremix/grremixstage.h`, `grremixstage.c`, `grremixhazards.h`, `grremixids.h`.

## How the port runs a Remix stage today

The port's `gkind` is the Remix stage id (`nGRRemixStage_*` in `grremixids.h`). `mpCollisionInitGroundData` loads the
map from `Stages.stage_file_table`, the model/collision/background come out of the map file, and
`grMainSetupMakeGround` -> `grRemixMakeGround` (grremixstage.c) runs the stage's ground maker from `Stages.function_table`:
a vanilla maker address (`grInishieMakeGround`, `grJungleMakeGround`, ...), `GRREMIX_FUNC_CLONE` (no hazards) or a
Remix hazard routine (`REMIX_SYM_Hazards__<name>` in `port/remix/remix_symbols.h`, all `Hazards.*` symbols are exported).
For an unhandled hazard routine the stage simply runs without hazards (a `[remix-stage]` line is logged), which is what
all ~170 stages do today and they render and play. Your job is to add the hazards.

Vanilla hazard code to imitate lives in `decomp/src/gr/grcommon/gr*.c` (a ground maker builds GObjs with `gcMakeGObjSPAfter`,
`gcAddGObjProcess`, `grModelSetupGroundDObjs`, `mpCollisionAddYakumonoDObj...`). Items and weapons spawned by hazards use
`it*MakeItem` / `wp*MakeWeapon`; Remix hazard items (KlapTrap, Robot Bee, Car, Bullet Bill, Scuttle Minion, Cacodemon) are
ported in `decomp/src/it/itremix/` by you when your group needs them (see `docs/remix/f5_item_worker_brief.md` for the item
interface, register through `tools/gen_remix_items.py` is done by the lead, you only export `dITRemix<Name>ItemDesc`;
ids 0x3A KlapTrap, 0x3B RobotBee, 0x3C Car, 0x3D BulletBill, 0x3E ScuttleMinion, 0x3F Cacodemon).

## Where the data comes from

- The routines: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` (your line ranges are in your prompt) and the stage file
  `.toolchain/pkgs/smashremix-src/src/stages/<name>.asm` (offsets such as `STAGE_FILE_OFFSET`, labels of the model parts).
- Model parts of the stage file are addressed as offsets from the map file base the same way the vanilla makers do it
  (`gMPCollisionGroundData` and the `map_nodes` token). Remix stage files are the reloc files `file_2161...` listed in
  `yamls/remix/reloc_misc.yml`; decode instructions with `.toolchain/host/venv/bin/python -I tools/remix_disasm_rom.py`.
- Hazard on/off: call `grRemixHazardsAreOn()` / `grRemixMovementIsOn()` (both TRUE for now) where the asm checks
  `Toggles.entry_hazard_mode`.

## Where your code goes

- New files only under `decomp/src/gr/grremix/` (`grremix<stage>.c/.h`), wrapped in `#ifdef PORT`. Your group's dispatch stub
  is a file named in your prompt (`grremixhazards_<group>.c`): replace its body so it returns TRUE after running the setup when
  `function == REMIX_SYM_Hazards__<name>`. Do not edit `grremixhazards.c`, `grremixstage.c`, `grremixids.h`.
- Hazards that need a vanilla hook (a `OS.patch_start` in a vanilla routine) are applied in that vanilla function under
  `#ifdef REGION_REMIX`, small edit, appended declarations in `decomp/src/gr/grremix/` headers or `ftremixhooks.h`.
- Do not touch `*.json`, CMake, generated files or other groups' files. New `.c` files are globbed by CMake.
- Stage-state storage: a `static` struct in your file (the port runs one match at a time), cleared at setup.
- `REMIX-TODO(<subsystem>)` for GFX/GFXRoutine/PokemonAnnouncer/CharEnvColor: apply the logic around it and list it.

## Verification and report

`scripts/remix/syntax-check.sh <file>` -> `syntax-check OK` for every file you wrote or edited, after your last edit to it.
Report (at most 20 lines): files, which `Hazards.*` routines of your range are ported and which are skipped (and why), the
stage ids they serve (`grremixids.h`), the item descs you exported, REMIX-TODOs, notes path
`build/remixasm/notes/f7_hazard_<group>.md`, and anything the lead must test (stage id to run with `SSB64_VS_STAGE=<decimal>`).

## Primer: the Remix asm idioms and their port equivalents (added after the first attempt stopped at an inventory)

The first wave of hazard workers wrote only inventories because they could not map a few idioms. Here they are. **Do not stop
at an inventory: write C, build the missing helper yourself inside `decomp/src/gr/grremix/`, and keep going. A routine you
cannot verify gets a `REMIX-TODO(<reason>)` and the surrounding logic is still ported.** The lead builds and runs the stage.

- `Global.match_info` -> `lbu 0x0001` is the current stage id: `gSCManagerBattleState->gkind`.
- RAM `0x80131300` (the "stage file", `lw a0, 0x1300(a0)` after `lui 0x8013`) is `gMPCollisionGroundData`
  (`MPGroundData*`, `decomp/src/mp/mpcollision.c`). Offsets the hazards add to it are plain byte offsets from that pointer.
  Pointers read out of file data are relocation tokens: wrap with `PORT_RESOLVE()` (see `grjungle.c`, `grinishie.c`);
  floats and ints inside the file are native after the port's blanket u32 swap.
- RAM `0x801313F0` is `gGRCommonStruct` (`GRStruct`, a union of per-stage structs). Hazard state that the asm keeps there
  goes in a `static` struct of your own file. `0x801313F4` and the like are fields of the same storage.
- `Render.register_routine(fn)` / `REGISTER_OBJECT_ROUTINE_` / `CREATE_OBJECT_`: a per-frame routine object. In C:
  `GObj *gobj = gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT);` then
  `gcAddGObjProcess(gobj, fn, nGCProcessKindFunc, 3);` (look at `grJungleMakeTaruCann` and `grInishieMakePakkun`).
  A display object adds `gcAddGObjDisplay(gobj, gcDrawDObjTreeForGObj, 6, GOBJ_PRIORITY_DEFAULT, ~0)`.
- `Render.create_display_object_`/model parts: `gcSetupCustomDObjs` / `grModelSetupGroundDObjs` with a `DObjDesc` array taken
  from the stage file offset, plus `gcAddAnimAll` for animations (see `grmodelsetup.c`).
- Remix stage items/weapons spawned by a hazard: `itManagerMakeItem(NULL, &desc, &pos, &vel, ITEM_FLAG_PARENT_DEFAULT)` and
  `wpManagerMakeWeapon(NULL, &wpdesc, &pos, flags)`; projectile ports for fighters are in `decomp/src/wp/*remix*`.
  A stage projectile that has no vanilla equivalent is a new `WPDesc` + process functions in your own file under
  `decomp/src/gr/grremix/` (use `wpmarioremixfireball.c` as the template of a Remix weapon).
- Fighter hit by a hazard: `ftMainSetStatus...`/`ftParamSetDamage...` as the vanilla hazards do (see `grzebes.c` acid,
  `grsector.c`); `Size.multiplier_table` -> `ftRemixSizeSetMultiplier(player, multiplier)` (exists in `ftremixhooks.h`).
- Sounds: `func_800269C0_275C0(fgm_id)` / `ftSoundPlay`-style helpers used by the vanilla hazards (`grep -rn "FGM" decomp/src/gr`).
- Hardcoded vanilla RAM addresses in the asm: map them with `build/remixasm/refs/vanilla_funcs.txt`
  (`awk` on the address, last entry at or below it); an address with no entry is `REMIX-TODO(vanilla addr)`.
- If the asm reads a file by id (`File.*`), load it with the same call the vanilla makers use for their extra files
  (`lbRelocGetExternHeapFile` / `lbRelocGetFileData`, see `mpCollisionInitGroundData`).
