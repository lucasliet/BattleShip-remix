# Porting a Smash Remix routine to C

How to turn a routine of the Remix assembly (`.toolchain/pkgs/smashremix-src/src`) into C that runs in the port: where to read, how to resolve addresses, the MIPS
o32 and struct-offset rules, the asm idioms and their port equivalents, the item interface, the convention for patching vanilla code and how to verify. Code lives in
`decomp/src/{ft/ftremix,gr/grremix,it/itremix}/` and the `*remix*` files of the other decomp modules; the structure around it is in [`architecture.md`](architecture.md).
Verify with `scripts/remix/syntax-check.sh <file>`, the unit tests (`run_remix_mode_tests`), the smoke tests of `.claude/skills/remix-smoke-tests/` and `tools/audit_remix_patches.py` (section 11).

## 1. Where to read

| What | Where |
|---|---|
| Remix source (commit `5e04fe7fcd023cd43c71f25f89bb6e810d254d55`, fetched by `scripts/remix/assemble-remix.sh`) | `.toolchain/pkgs/smashremix-src/src/`: `<Fighter>/*.asm`, `items/`, `stages/`, `css/`, and the shared files (`OS.asm`, `Character.asm`, `Global.asm`, `Render.asm`, `Toggles.asm`, `Stages.asm`, `Item.asm`, `FGM.asm`, `GFX.asm`, `Size.asm`, `AI.asm`, `Hazards.asm`, ...) |
| Label to RAM address map of the Remix build | `build/remixasm/remix.sym`, from `scripts/remix/assemble-remix.sh` |
| Port constants for those labels | `port/remix/remix_symbols.h`: `REMIX_SYM_<Scope>__<label>`. A missing one is added to `EXTRA_SYMBOLS` in `tools/gen_remix_actions.py` and the file regenerated (never edited by hand) |
| Routines of one fighter | `build/remixasm/worklists/<id>_<NAME>.md`: every routine referenced by its action array and special tables, with address, `REMIX_SYM_*` macro and source line (written by `tools/gen_remix_actions.py`) |
| N64 offsets of struct fields | `build/remixasm/refs/struct_offsets.txt` (section 4) |
| Vanilla address to decomp function | `build/remixasm/refs/vanilla_funcs.txt` (`tools/remix_port_refs.py` writes both reference files) |
| Remix side of a topic | [`fighters.md`](fighters.md), [`stages_and_moveset.md`](stages_and_moveset.md), [`menus.md`](menus.md), [`audio.md`](audio.md), [`css_reference.md`](css_reference.md), [`modes_base.md`](modes_base.md), [`parity.md`](parity.md) (which asm file is ported where) |
| A finished example of the same shape | Fighter: `decomp/src/ft/ftremix/falco/ftremixfalco.c`. Item: `it/itremix/itremixgem.c` (fighter item), `itremixmrsaturn.c` (standard item). Stage hazard: `gr/grremix/grremixjapes.c`. Toggle: `ft/ftremix/ftremixzcancel.c`. Mode rule: `gm/gmremixtugofwarrules.c` with `tests/remix/test_tugofwar_rules.c` |

The decomp is the vanilla game; Remix changes it with `OS.patch_start(rom, ram)` blocks (code or data written at a vanilla address) and with tables indexed by character, stage or
item id. A port is therefore either a **new routine** (Remix label reached through a table: ported as a `static` function and listed in a `FTRemixProc` table, an item descriptor or a stage
handler) or a **patch** of a vanilla function (a hook under `#ifdef REGION_REMIX`, section 9).

## 2. Address resolution

| Address | Meaning | How to resolve it |
|---|---|---|
| `0x80400000` and above | Remix code or data (the 4 MB image DMAed from ROM `0x2C00000`) | `REMIX_SYM_*` in `remix_symbols.h`; data is read at run time with `remix_ram_image_read_word/half/byte` or `remix_ram_image_resolve` (never copied into C) |
| Below `0x80400000` in code | A vanilla routine | `build/remixasm/refs/vanilla_funcs.txt` (last entry at or below the address contains it) or `.toolchain/host/venv/bin/python -I tools/n64_addr_to_symbol.py 0x800E6F24 [--near 0x300]`; a routine whose decomp definition lacks the `// 0xADDR` comment is found with `grep -rn "func_<ADDR>" decomp/src` |
| Below `0x80400000` in data | A vanilla global (`0x80131300` stage file pointer, `0x80131580` player interface, ...) | Same tool: it reads the `// 0xADDR` comments and the address-bearing names (`dMNVSResultsUnused0x80138EF0`) of `decomp/src` plus `remix.sym` |
| Not found | Overlay data or code without a decomp name | Compute RAM from ROM with the segment's delta (fighter code `RAM = ROM + 0x80084800`, battle overlay `RAM = ROM + 0x800855C0`, expanded code image `RAM = ROM + 0x7D800000`) and read the instructions with `tools/remix_disasm_rom.py` (section 6); a value you cannot read is a documented deviation |

Notes that prevent wrong matches:

- Overlays share one RAM range, so one address can have several owners; pick the one whose file is loaded in the scene the asm runs in (the tool lists every candidate with its file). The address comment of a function can
  belong to another overlay: verify with what the asm does, not with the address alone.
- `lui t0, 0x800F` followed by `addiu t0, t0, 0x9C8C` builds `0x800E9C8C`: the low half is sign-extended, so a low half with the top bit set lowers the high half by one. `lui 0x8019` + `lwc1 0xC230(t0)` is `0x8018C230`.
- `OS.patch_start(rom, ram)` pairs a ROM offset with the RAM address it patches; the RAM address is the one to resolve. `Character.table_patch_start(table, id, size)` patches one row of a per-character table: the
  port reads the Remix table instead of applying the write.
- A routine reached through `jal`/`jalr` with a Remix address is a Remix routine: port it or call its port. A vanilla address is called by its decomp name (`0x800E6F24` is `ftMainSetStatus`, `0x801655C8` is
  `wpManagerMakeWeapon`, `0x8016E174` is `itManagerMakeItem`).

## 3. MIPS o32 and float rules

- Arguments: `a0..a3`, then the stack; results in `v0` (integers, pointers) or `f0` (float). In a fighter proc, `a0` is the fighter GObj: start with `FTStruct *fp = ftGetStruct(fighter_gobj);`. Weapon and item procs take the
  weapon or item GObj. A float argument travels in `f12`/`f14` only when it is the first (and second) argument; after a non-float argument it travels as raw bits in an integer register or on the stack. `lui a2, 0x3F80` as an argument
  is `1.0F`.
- Float constants are loaded as raw bits: `lui at, 0x43E6` then `mtc1 at, f4` is `460.0F`. Decode with
  `.toolchain/host/venv/bin/python -I -c "import struct;print(struct.unpack('>f', bytes.fromhex('43E60000'))[0])"`. Write the decimal when exact and short; keep the hex in a comment otherwise (`1.6015625F /* 0x3FCD0000 */`).
- Branch delay slots run before the jump takes effect; the likely forms (`beql`, `bnel`, `bc1fl`, `bc1tl`) annul the delay slot when the branch is not taken. A comparison is `c.lt.s`/`c.le.s` followed by `bc1t`/`bc1f`
  (an inverted test is common); `slt`/`sltu` set a register. `lb`/`lh` sign-extend, `lbu`/`lhu` do not: use the matching C type. `srl` is a logical shift, `sra` arithmetic.
- Follow the asm text, not what the author seems to have meant. Behaviour that looks wrong but is deterministic (an annulled delay slot that removes a case, a dead patch) is ported as it is. The exception is code that clearly
  indexes or compares with the wrong register (a value read with the team where every write uses the port): the port does what the code clearly meant, the spot gets a `DEVIATION` comment with the asm line and a
  record under `docs/bugs/` (`docs/bugs/remix_tugofwar_asm_deviations_2026-11-05.md` is the model).
- Keep the original control flow; `goto` is the most faithful form for jump-heavy routines. Do not invent behaviour.
- Branch targets that leave a patch (`j _return`, `j 0x8016F8C0` at the end of a blast zone callback) are returns: `j 0x8016F8C0` at the end of a `blast_zone_` equals returning FALSE.

## 4. Struct offsets, bitfields, LP64

The port's structs are laid out for 64-bit pointers, so `(u8 *)fp + 0x0B18` is a different field there. Never access `FTStruct`, `WPStruct`, `ITStruct` or `DObj` by raw offset: map the offset to a named field.

- `build/remixasm/refs/struct_offsets.txt` lists the N64 offset of every field of `FTStruct`, `FTAttributes`, `FTData`, `WPStruct`, `WPAttributes`, `ITStruct`, `ITAttributes`, `GObj`, `DObj`, `SObj`, `CObj`, `MObj`, `FTParts`, `EFStruct`,
  `GMColAnim`, `FTMotionDesc`, `FTStatusDesc`, `FTThrowHitDesc`, `FTAttackColl`, `FTDamageColl` (a MIPS32 big-endian layout dump of the decomp headers). Examples: `0x84(gobj)` is `gobj->user_data` (`ftGetStruct`);
  `0x017C`, `0x0180`, `0x0184` of the fighter are `motion_vars.flags.flag0/1/2` (the "temp variables 1 to 3" set by the moveset command `5C0000XX`); `0x0ADC` starts `passive_vars`, `0x0B18` starts `status_vars`.
- The dump is 4 bytes lower than the N64 offsets from about `0x248` on `ITStruct` and `0x25C` on `WPStruct`: add 4 and check the neighbouring fields (N64 `0x398` is `proc_dead`, `0x33E` is `multi`).
- Remix scratch words that share a union in the asm still share it in C: `ftRemixFighterWord(fp, n64_offset)`, `ftRemixFighterFloat` and `ftRemixFighterKirbyHat` (`ft/ftremix/ftremixfields.h`) map the offsets `0x0ADC..0x0B13`,
  `0x0B18..0x0B4B` and `0x0980`.
- Bitfields are big-endian (IDO): bit 0 is the most significant bit. For `lbu`/`andi`/`sb` on a bitfield byte, find each bit in the table and write the field assignments; mask `0x10` of the first byte is bit index 3 (the bit of
  `attack_coll.can_rehit_shield`), `0x08` is bit index 4. Cite the original byte and mask in a short comment when the mapping is not obvious.
- A pointer in a Remix struct is 4 bytes in the file data and 8 in memory. Data read from a loaded file uses the reloc helpers (`PORT_RESOLVE()` for pointers inside file data; integers and floats are already native after the
  blanket word swap of reloc files).
- Includes: `<ft/fighter.h>` first. Do not include `<stdio.h>` or `<math.h>` before decomp headers (on the Switch toolchain a platform header ahead of `<ft/fighter.h>` leaves `wint_t` undefined); log with `<port_log.h>` after
  the decomp headers. A block comment in a header must not contain `/*`.

## 5. Characters, files and per-port state

- Character ids: `Character.id.X` is `REMIX_ID_X` (`remix/remix_roster_data.h`) or the vanilla `nFTKind*` value; `fp->fkind` already is the Remix id.
- File data: `Character.<NAME>_file_N_ptr` with N = 1 is `fp->data->p_file_main`; N = 6, 7, 8, 9 is `fp->data->p_file_special1`, `special2`, `special3`, `special4`. Each is a `void **`; dereference it for the loaded file base. When Kirby
  wears a copied power the files are the copied fighter's: `ftRemixPowerData(fp)`.
- Per-port state the asm keeps in Remix memory (`li t0, table; addu t0, t0, port`) becomes a `static` array indexed by `fp->player` (0..3); clear it where the asm clears it (setup of the match).
- Remix data (`dw`, `dh`, `float32` tables) becomes `static const` C arrays when it is short and part of the routine; larger Remix tables, tables other systems share and anything that depends on the Remix build are read from the
  RAM image through `REMIX_SYM_*`. Values copied from the asm source are fine; bytes taken from a ROM never go into a versioned file.
- Statuses: Remix action `0xDC + i` is the same number in the port; name the ones of the file in a local enum taken from `scope Action` of `<Fighter>.asm` (`nFTRemixWarioStatusCorkscrew = 0xE1`). A Remix routine that
  branches for Kirby or JKirby copying the power keeps the branch, with the Kirby action ids of `src/Kirby/Kirby.asm` as local constants.
- Settings: `gmRemixToggle(label)` / `gmRemixToggleVS(label)` (`gm/gmremixtoggle.h`) for `Toggles.entry_<label>`; use the VS variant where the asm uses `Toggles.single_player_guard`. A toggle that is off must leave the vanilla
  behaviour byte for byte. Per-port panel options: `gmRemixPanel(OPTION, fp->player)` (`gm/gmremixpanel.h`, value indices are the asm's).

## 6. `OS.copy_segment`

Remix splices raw bytes of the vanilla ROM into a routine with `OS.copy_segment(rom_offset, size)`. Decode them and port the instructions as C; never write the bytes:

```
.toolchain/host/venv/bin/python -I tools/remix_disasm_rom.py <rom_offset> <size> [--vram <addr>]
```

The default `vram` is `rom_offset + 0x80084800` (the fighter code segment, the mapping of `OS.patch_start` pairs such as `0x63D2C -> 0x800E852C`); pass `--vram` for other segments. Every `jal` is annotated with its decomp name
(from `vanilla_funcs.txt`). With `--remix` the tool disassembles the assembled Remix ROM (`build/remixasm/remix.z64`) instead: ROM `0x2C00000` is RAM `0x80400000`, so routine `x` is at ROM
`0x2C00000 + (REMIX_SYM_x - 0x80400000)`. The tool needs the rabbitizer disassembler (`scripts/remix/setup-remix-tools.sh`) and `baserom.us.z64`. A spliced vanilla routine that the decomp already has (a copy of the Star or Maxim
Tomato spawn code, say) is a call to the decomp function plus the Remix changes around it.

## 7. Asm idioms and their port equivalents

| Remix asm | Port |
|---|---|
| `Global.match_info` byte `0x0001` | current stage id: `gSCManagerBattleState->gkind` |
| `Global.current_screen` | `gSCManagerSceneData.scene_curr` (the same numbers for the screens the Remix shares with the vanilla; `0x77` is the Remix modes scene) |
| `Global.get_random_int_(n)` | `syUtilsRandIntRange(n)` |
| `FGM.play_(id)` | `func_800269C0_275C0(id)` (the FGM player), or the `ftSound`/`ftParamPlay*` helper the vanilla code uses at that spot |
| `Render.register_routine(fn, group, order)`, `CREATE_OBJECT_` | A per-frame routine object: `GObj *g = gcMakeGObjSPAfter(nGCCommonKind..., NULL, link, GOBJ_PRIORITY_DEFAULT); gcAddGObjProcess(g, fn, nGCProcessKindFunc, 3);` and, for a display object, `gcAddGObjDisplay(g, gcDrawDObjTreeForGObj, 6, GOBJ_PRIORITY_DEFAULT, ~0)` (examples: `grJungleMakeTaruCann`, `grInishieMakePakkun`) |
| `Render.create_display_object_`, model parts | `gcSetupCustomDObjs` / `grModelSetupGroundDObjs` with a `DObjDesc` array from the stage file offset; animations with `gcAddAnimAll` (`grmodelsetup.c`) |
| `Render.GROUP_TABLE` | `gGCCommonLinks` (display groups: `gm/gmremixgroupdisplay*`) |
| `Render.load_file(File.X, ptr)` | `lbRelocGetExternHeapFile` / `lbRelocGetFileData` (as `mpCollisionInitGroundData` does for stage extras); Remix file ids are ordinary reloc ids |
| `lw a0, 0x1300(a0)` after `lui 0x8013` (stage file `0x80131300`) | `gMPCollisionGroundData` (`MPGroundData *`); offsets added to it are byte offsets from that pointer; pointers read out of the file are reloc tokens: `PORT_RESOLVE()` |
| RAM `0x801313F0` and following (hazard state) | `gGRCommonStruct`; hazard state that the asm keeps there goes in a `static` struct of your own file, cleared at setup (one match at a time) |
| `Size.multiplier_table` | `ftRemixSizeSetMultiplier(player, multiplier)` (`ftremixhooks.h`) and the readers in `ftremixsize*.c` |
| `Hazards.*` toggles (`Toggles.entry_hazard_mode`) | `grRemixHazardsAreOn()` / `grRemixMovementIsOn()` |
| Item spawn `jal 0x8016E174` | `itManagerMakeItem(parent, &desc, &pos, &vel, flags)`; a Remix item by kind: `itRemixMakeItem(parent, kind, &pos, &vel, flags)` |
| Weapon spawn `jal 0x801655C8` | `wpManagerMakeWeapon(parent, &wpdesc, &pos, flags)`; a stage projectile without a vanilla equivalent is a new `WPDesc` with its procs in your own file (template: `wp/wpmario/wpmarioremixfireball.c`) |
| Damage to a fighter from a hazard | `ftMainSetStatus...` / `ftParamSetDamage...` as the vanilla hazards do (`grzebes.c` acid, `grsector.c`) |
| Stage file offsets for item animation tracks | relative to `grRemixHazardsItemFileTop(stage_header)` |
| `GFX.*` / `GFXRoutine.*` ids | `lbremixgfx.c`, `gmremixcolscripts.c` (new ids are valid in the same calls as vanilla ones) |
| Moveset commands `0xD0..0xDC` | Already implemented in `ft/ftremix/ftremixcommand.c`; do not reimplement |
| `Item.*.SPAWN_ITEM`, `PokemonAnnouncer.*`, `Reflect.*`, `Costumes.*`, `CharEnvColor.*` | The subsystem modules in `architecture.md`: call their API instead of porting the helper again |

## 8. Where the C goes and how it is named

- Fighter routines: `decomp/src/ft/ftremix/<fighter>/ftremix<name>.c`, `static void ftRemix<Name><Move><What>(GObj *fighter_gobj)` with the decomp role in the name (`ProcUpdate`, `ProcInterrupt`, `ProcPhysics`, `ProcMap`,
  `SetStatus`, `InitStatusVars`). Special entry routines (`*initial_*` in the special tables) are `void (GObj *)` handlers that set the status. Weapon procs take the weapon GObj and return `sb32`
  (`decomp/src/wp/wptypes.h`); port the weapon descriptor too when the move creates a projectile.
- Each routine gets a one-line doc comment naming its Remix label (`/* WarioNSP.ground_move_ */`); other comments state rationale, a constraint or a quirk with the asm file and line, never restate the code. All code is in English.
  Wrap the file in `#ifdef PORT ... #endif`; include `"ft/ftremix/ftremix.h"`, `"remix/remix_symbols.h"` and the decomp headers (the parent's `ft/ftchar/<parent>/<parent>.h`).
- Export exactly one table per file:

```c
const FTRemixProc dFTRemix<Table>Procs[] =
{
    { REMIX_SYM_WarioNSP__ground_move_, ftRemixWarioSpecialNGroundProcUpdate },
    ...
};

const s32 dFTRemix<Table>ProcsNum = ARRAY_COUNT(dFTRemix<Table>Procs);
```

  Every routine of the fighter's worklist is in a table; helpers the worklist does not list stay `static`. After adding a file run `.toolchain/host/venv/bin/python -I tools/gen_remix_ported.py` (it regenerates `ftremixported.c`) and
  `cmake -S $PWD -B $PWD/build` (new files under `decomp/src` are globbed). Unported routines fall back to the parent's at bind time and are logged (`ftRemix: ... not ported`).
- Pure logic (numbers in, numbers out) goes into a `*rules.c` / `*rules.h` pair that uses only `int` and `float`, with a suite under `tests/remix/` (section 11).
- Functions are short and single-purpose, files are small (split by responsibility), names are explicit, early returns instead of deep nesting, TSDoc on new functions.
- When a piece depends on something the port does not have, port the surrounding logic faithfully and mark the gap `/* REMIX-TODO(<subsystem>): <what is skipped> */`; a deliberate difference is
  `/* DEVIATION(<short reason>): ... <asm file>:<line> */`. `grep -rn REMIX-TODO decomp/src` and `grep -rn DEVIATION decomp/src` list them. A hook that is half applied and changes vanilla behaviour or can crash is never left.

## 9. Patching vanilla code

A Remix patch of a vanilla function becomes a small hook in that function, guarded so that the vanilla build is unchanged.

1. Locate the function (section 2) and read all of it. Overlays make several functions share an address: confirm with the asm's behaviour.
2. Insert the Remix behaviour under `#ifdef REGION_REMIX`, with the original line(s) in `#else` when the Remix replaces them:

```c
#ifdef REGION_REMIX
    if (ftRemixCommandCheckRun(fighter_gobj, fp, ms, FALSE) != FALSE)   /* Remix load_command_ (0x800E0388) */
    {
        continue;
    }
#endif
```

3. Put logic that is more than a few lines in a file you own (`decomp/src/ft/ftremix/`, `wp/`, `it/itremix/`, `gm/`, ...), guarded by `#if defined(PORT) && defined(REGION_REMIX)`, and declare it by appending a block with a comment naming the scope to
   `decomp/src/ft/ftremix/ftremixhooks.h` (append only, never reorder), or to the header of the module. The vanilla file includes the header under `#ifdef REGION_REMIX`. The hook checks the fighter, item or stage itself (`fp->fkind == REMIX_ID_X`)
   and is a no-op for others.
4. Copies, not mutations: a Remix `ITDesc`/`WPDesc` that differs from the vanilla one is a copy; never write into the vanilla static.
5. Remix state that a patch keeps per port, per hitbox or per match lives in the module that owns it (`ftRemixCommandResetAttack`, `gmremixtagteam.c`); event listeners that must run inside the engine are registered in
   `port/port.cpp` (`RegisterRemixCommandListeners`).
6. Patches that exist only for the CPU (`cpu_*`, `recovery_logic`, `ledge_check`, `cpu_attack_weight`) belong to the AI modules (`ft/ftremix/ftremixai*.c`, hooks in `ftcomputer.c`; Remix data from the image).
7. A per-fkind table or size in the decomp that a patch extends: go through `port/fighter_registry.h` or the Remix table of the image; never index a 27-row decomp array with a synth fkind.
8. The behaviour is the Remix 2.0.1 one (section 3 lists the only exception). A gap caused by something the Remix leaves undefined (a null pointer on N64, a joint the model lacks) is a logged guard documented under `docs/bugs/`.

## 10. Items

The shared infrastructure exists; a new item only adds a file. Ids follow `Item.asm` `add_item` order from `0x2D`: Cloaking Device 0x2D, Super Mushroom, Poison Mushroom, Blue Shell, Lightning, Deku Nut, Franklin Badge, Pitfall, Golden Gun,
Dango, P-Wing, Mr. Saturn 0x38, Stopwatch 0x39, KlapTrap 0x3A, Robot Bee, Car, Bullet Bill, Scuttle Minion, Cacodemon 0x3F, Basketball 0x40, Psyduck 0x41, Gem 0x42, Shuriken, Boomerang, Clan Bomb, Waddle Dee, Waddle Doo, Gordo, Cloud,
Flashbang 0x4A, Turnip 0x4B. Each `items/<Name>.asm` defines `SPAWN_ITEM`, `SHOW_GFX_WHEN_SPAWNED`, `PICKUP_ITEM_MAIN` (consume), `PICKUP_ITEM_INIT` (pre-pickup / hold), `DROP_ITEM`, `THROW_ITEM`, `PLAYER_COLLISION` (touch), an
`item_info_array` (the decomp `ITDesc`) and an `item_state_table` (`ITStatusDesc[]`).

Write `decomp/src/it/itremix/itremix<name>.c` (`#ifdef PORT`, includes `"it/itremix/itremix.h"`) and `itremix<name>.h` declaring the spawn function that fighter or stage code calls:

- `item_info_array` is a `static ITDesc`, `item_state_table` a `static ITStatusDesc[]`; each vanilla address becomes its decomp function (`vanilla_funcs.txt`). Status procs take the item GObj and return `sb32`. State lives in `ip->item_vars`
  (a struct of `it/itvars.h` or a file-local struct cast over it, with a `_Static_assert` that it fits).
- Item file. A fighter item (`dw Character.<NAME>_file_<N>_ptr`) uses the owner's `fp->data->p_file_special<N-5>` (N = 6..9 are special1..4; each is a `void **` that goes straight into `ITDesc.p_file`), set on a stack copy of the static `ITDesc`
  before `itManagerMakeItem`. A standard item (`dw 0x8018D040`, no owner; spawned by the stage spawner, containers, taunts and the Training menu, so `parent_gobj` is usually NULL) uses `p_file = &gITManagerCommonData` (file 0xFB, patched in place by
  the Remix) and `o_attributes` = the asm `FILE_OFFSET` (the footer inside it), as `itcommon/ittomato.c` does. Hazard items read their stage file (`grRemixHazardsItemFileTop`).
- The spawn routine has the signature `GObj* (GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`. Remix `SPAWN_ITEM` routines that take extra registers (a1 = item info array, a3 = an "unknown" flag) map onto it, `flags` being the a3 boolean of
  `itManagerMakeItemSetupCommon`. Spawn code spliced from vanilla Star or Maxim Tomato routines with `OS.copy_segment` is decoded (section 6) and keeps the Remix velocity overrides (the mushroom's alternating horizontal speed).
- Export one descriptor, then run `.toolchain/host/venv/bin/python -I tools/gen_remix_items.py` (regenerates `itremixitems.c`):

```c
const ITRemixItemDesc dITRemixTurnipItemDesc =
{
    0x4B,                      /* Item.Turnip.id */
    itRemixTurnipMakeItem,     /* SPAWN_ITEM */
    TRUE,                      /* SHOW_GFX_WHEN_SPAWNED */
    itRemixTurnipHoldProc,     /* PICKUP_ITEM_INIT, or NULL */
    itRemixTurnipConsumeProc,  /* PICKUP_ITEM_MAIN (consume items), or NULL */
    itRemixTurnipDroppedProc,  /* DROP_ITEM, or NULL */
    itRemixTurnipThrownProc,   /* THROW_ITEM, or NULL */
    NULL                       /* PLAYER_COLLISION, or NULL */
};
```

  The vanilla sites (spawn, hold, drop, throw, consume through `ftCommonLightGetProcDamage` whose tail `0x80145C4C` is `itRemixConsumeTomatoTail`, touch through `ftMainUpdateDamageStatItem`) already route ids >= 0x2D to the descriptor.
- Per-player effect timers (mushrooms, Lightning, Franklin Badge, Stopwatch) are file-local arrays indexed by `fp->player`, updated by a `gcAddGObjProcess` routine and cleared on the scene-end path the asm uses; a vanilla hook the asm needs
  for per-frame work is a patch (section 9). The subsystems exist (GFX, GFXRoutine, Size, PokemonAnnouncer, Reflect, CharEnvColor, Kirby, damage types): call them.
- Debug: `SSB64_REMIX_FORCE_ITEM=<kind>` forces every stage-spawned item, `SSB64_REMIX_ITEM_LOG=1` logs `[remix-item] spawn|hold|consume|touch|drop|throw`.

## 11. Verification

After each change:

1. **Compile the file**: `scripts/remix/syntax-check.sh <file>` compiles one file with the flags of the game build (`build/compile_commands.json`); it must print `syntax-check OK: <file>` with no warning from your file.
2. **Build**: `PATH=$PWD/.toolchain/host/ninja:$PATH .toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target ssb64 -j 4` (absolute paths; a new file needs `cmake -S $PWD -B $PWD/build` first). The vanilla build must
   still compile: configure `build-us` with `-DSSB64_VERSION=us` (`docs/remix/build_and_roms.md`) and build `ssb64` there.
3. **Unit tests** for a pure rule: add `tests/remix/test_<topic>_rules.c` (suite `remixTest<Topic>Rules`, `CHECK_EQ` / `CHECK_TRUE` / `RUN_TEST` of `test_check.h`), register it in `test_main.c` and `tests/remix/CMakeLists.txt` with the
   `*rules.c` source, and run `cmake --build $PWD/build --target run_remix_mode_tests`. A test is trusted once mutating the rule makes it fail.
4. **Patch audit**: `.toolchain/host/venv/bin/python -I tools/audit_remix_patches.py [--out patches.tsv]` lists every `OS.patch_start` of the Remix with asm file and line, scope, ROM and RAM address, the decomp function it lands in (the overlay is
   chosen by checking which file's function starts fall on stack-frame prologues of the vanilla ROM at that address delta) and a status: `hooked` (the function body names a Remix symbol or `REGION_REMIX`), `ambiguous-hooked`, `unhooked`,
   `no-function` or `symbolic`. An `unhooked` row is a triage item: the port may implement the patch elsewhere (a caller, a data table, a Torch factory) or the patch may be missing. Run it before and after porting a scope;
   `tools/gen_remix_parity.py` regenerates `parity.md`.
5. **In the game**: the smoke tests run the real binary headless (`dangerouslyDisableSandbox` for the window server; never rebuild while a sweep runs): `vs.sh <fkinds> <input> <name> [frames]`, `special_matrix.sh <fkind>...` (NSP/DSP/USP),
   `stage_sweep.sh`, `hazard_sweep.sh <stage>`, `item_cpu_sweep.sh [kind]`, `stage_select_sweep.sh`, `css.sh <input> <name>`. Look at the screenshots (`sheet.py`) instead of trusting `rc`. A run passes when `rc=0`, the screenshots exist, the log has no
   abort/assert/fatal line and the effect is visible (a fighter move, an item event in `[remix-item]`, a hazard acting, a toggle changed with `SSB64_REMIX_TOGGLES="Title=value"`). A crash with a `CAMetalLayer nextDrawable` line is a macOS
   flake: repeat it before investigating. Variables, the input script format and lldb recipes: `.claude/skills/remix-smoke-tests/SKILL.md`.
6. **Reference emulator** when the Remix's own look or timing is the question: `scripts/macos/setup-ref-emu.sh` once, then `scripts/ref-emu/capture.sh baserom.remix.z64 <frames,...> build/ref-shots/<name> [input.txt]` (the input file has the
   `SSB64_INPUT_SCRIPT` format; the frame index counts presented frames, so align by content, not by index) and `.toolchain/host/venv/bin/python -I scripts/ref-emu/sheet.py build/ref-shots/<name> 4 320` for a contact sheet. Captures are copyrighted
   frames and stay in `build/`; compare with a port run of the same input script.
7. **Record**: a significant bug gets a `docs/bugs/<topic>_<YYYY-MM-DD>.md` with a row in `docs/bugs/README.md`; a deliberate difference gets its `DEVIATION` comment.

Open gaps: `docs/remix/patch_audit.md` and `docs/remix/ROADMAP.md`.
