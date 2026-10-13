# F5 worker brief: port Smash Remix fighter routines to C

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip` (always use absolute
paths; use `builtin cd`, never plain `cd`).

## Context

This is a PC port of the SSB64 decompilation. The Smash Remix 2.0.1 fighters
are added as "synth" fighters whose fighter kind equals the Remix character id.
Each Remix fighter has an action array (status ids 0xDC + i) that names its
status routines by N64 RAM address, plus special-move entry tables. At runtime,
`decomp/src/ft/ftremix/ftremixbind.c` maps every address to a C function:

- vanilla addresses map to decomp functions (generated table);
- Remix addresses map to the hand-ported C routines that you write, exported
  as `FTRemixProc { u32 addr; FTRemixProcFn proc; }` tables.

Any Remix routine with no port falls back to the parent fighter's routine,
which plays the wrong move. Your job is to port the Remix MIPS assembly of
your assigned scope to faithful C so the fighter plays its real Remix moveset.

## Read first

1. The pilot: `decomp/src/ft/ftremix/falco/ftremixfalco.c`, a port of
   `.toolchain/pkgs/smashremix-src/src/Falco/Phantasm.asm`. Copy its structure.
2. `decomp/src/ft/ftremix/ftremix.h` for the table types.
3. Your worklist: `build/remixasm/worklists/<id>_<NAME>.md`, which lists
   every routine in scope with its address, its `REMIX_SYM_*` macro and its
   source line.

## References

- Remix source: `.toolchain/pkgs/smashremix-src/src/<Fighter>/*.asm`, plus
  shared files in `.toolchain/pkgs/smashremix-src/src/*.asm` (`OS.asm`,
  `Character.asm`, `FGM.asm`, `GFX.asm`, `Global.asm`, `Kirby/Kirby.asm`...).
- Remix symbols: `port/remix/remix_symbols.h`, where
  `REMIX_SYM_<Scope>__<label>` is the routine address. The full map is in
  `build/remixasm/remix.sym`.
- Struct offsets: `build/remixasm/refs/struct_offsets.txt`, which maps the
  N64 offset of every field of FTStruct, FTAttributes, FTData, WPStruct,
  ITStruct, GObj, DObj, EFStruct, FTAttackColl and others to the C field path.
  - `0x84(gobj)` is `gobj->user_data` (the FTStruct, `ftGetStruct`).
  - `0x17C`, `0x180` and `0x184` are `motion_vars.flags.flag0/1/2` (Remix
    "temp variable 1/2/3", set by moveset command `5C0000XX`).
  - Bitfields are big-endian (bit 0 is the MSB). For `lbu`/`andi`/`sb` on
    bitfield bytes, find each bit's field name in the table and write the C
    field assignments, with a short comment naming the original byte and mask.
- Vanilla functions: `build/remixasm/refs/vanilla_funcs.txt` maps an address
  to its decomp function and file. For `jal 0x800E6F24`, look it up and call
  `ftMainSetStatus`. Some functions have no `// 0x` comment; for those, grep
  `decomp/src` for `func_<ADDR>` (for example `func_800269C0_275C0`, the FGM
  player that `FGM.play_` wraps).
- Fighter headers: `decomp/src/ft/ftchar/<parent>/` for the parent's status
  enums, constants and routines.
- Disassembler for `OS.copy_segment(rom_offset, size)` fragments, which
  splice raw vanilla code bytes into Remix routines. Decode them, then port
  the decoded instructions:
  `.toolchain/host/venv/bin/python -I tools/remix_disasm_rom.py <rom_offset> <size> [--vram <addr>]`.
  The default vram assumes the fighter code segment (rom + 0x80084800). Each
  `jal` is annotated with its decomp name. Add `--remix` to disassemble the
  assembled Remix ROM instead; there, ROM 0x2C00000 is RAM 0x80400000, so
  `0x2C00000 + (REMIX_SYM_x - 0x80400000)` disassembles routine x as built.
  Never write ROM bytes into C files; port the instructions as C.
- Moveset commands `0xD0`–`0xDC` (frame speed, armour, hitbox direction,
  translation multiplier, y velocity, fast fall, random sfx, kinetics,
  hitbox fgm, env color, switch direction, goto moveset file, taunt voice)
  are already implemented in `decomp/src/ft/ftremix/ftremixcommand.c`. Do
  not reimplement them.

## Translation rules

- Status routines: `static void ftRemix<Name><Move><What>(GObj *fighter_gobj)`,
  using decomp naming for the role (`ProcUpdate`, `ProcInterrupt`,
  `ProcPhysics`, `ProcMap`, `SetStatus`, `InitStatusVars`). In Remix procs,
  a0 is the fighter GObj, and a2 or another register often holds the player
  struct: start with `FTStruct *fp = ftGetStruct(fighter_gobj);`.
- Special entry routines (`*initial_*` in the special tables) are
  `void (GObj *)` handlers that set the status.
- Weapon procs referenced by a Remix weapon descriptor take the weapon GObj
  and return `sb32` (see `decomp/src/wp/wptypes.h` and the vanilla `wp*`
  files for signatures). Port the weapon descriptor and procs too when your
  moves create projectiles (`jal 0x801655C8` is `wpManagerMakeWeapon`).
- Status ids: Remix action `0xDC + i` is the same number in the port. Name
  them in a file-local enum taken from `scope Action` in `<Fighter>.asm`,
  for example `nFTRemixWarioStatusCorkscrew = 0xE1`.
- MIPS o32 calls: when the first argument is not a float, float arguments
  travel as raw bits in a2/a3/stack. `lui a2, 0x3F80` is `1.0F`, and
  `lui at, 0x43E6; mtc1 at, f4` is `460.0F`. Decode with
  `.toolchain/host/venv/bin/python -c "import struct;print(struct.unpack('>f',bytes.fromhex('43E60000'))[0])"`.
  When the decimal form is not exact or short, keep the hex in a comment.
- Remix per-player state kept in Remix memory (for example
  `li t0, table; addu t0, t0, port`) becomes a `static` array indexed by
  `fp->player` in your file.
- Remix data (`dw`, `float32`, `dh` tables) becomes `static const` C arrays.
- File data: `Character.<NAME>_file_N_ptr` with N = 1 is
  `fp->data->p_file_main`, and N = 6, 7, 8 or 9 is
  `fp->data->p_file_special1/2/3/4`. Each is a `void **`; dereference it to
  get the loaded file base.
- Character id checks (`Character.id.X`) compare `fp->fkind` with
  `REMIX_ID_X` from `remix/remix_roster_data.h`, or the vanilla
  `nFTKind*` value.
- Kirby branches: Remix routines often branch for Kirby or JKirby copying the
  power. Port those branches faithfully, with the Kirby action ids from
  `src/Kirby/Kirby.asm` as file-local enum constants.
- Keep the original control flow; `goto` is fine and is often the most
  faithful form. Do not invent behaviour.
- Comments: give each ported routine a one-line doc comment naming its Remix
  label (`/* WarioNSP.ground_move_ */`). Add other comments only for
  rationale or constraints, never to restate the code. All code is in English.
- Wrap the whole file in `#ifdef PORT ... #endif`. Include
  `"ft/ftremix/ftremix.h"`, `"remix/remix_symbols.h"` and the decomp headers
  you need, such as `<ft/fighter.h>`, `<wp/weapon.h>` and the parent's
  `ft/ftchar/<parent>/<parent>.h`.
- Export exactly one table per file:

  ```c
  const FTRemixProc dFTRemix<Table>Procs[] =
  {
      { REMIX_SYM_WarioNSP__ground_move_, ftRemixWarioSpecialNGroundProcUpdate },
      ...
  };

  const s32 dFTRemix<Table>ProcsNum = ARRAY_COUNT(dFTRemix<Table>Procs);
  ```

  Every routine of your scope that appears in the worklist must be in the
  table. Helpers that the worklist does not list stay `static` and stay out
  of the table.

## Things you cannot do in your file: record them in your notes file

Your notes file is `build/remixasm/notes/<id>_<NAME>[_<MOVE>].md`. Record:

1. **Vanilla patches.** These are `OS.patch_start(rom, ram)` blocks or `j`/`jal`
   hooks into vanilla code that your scope relies on. For each one, give the
   RAM address, the decomp function and its file (from vanilla_funcs.txt),
   what the patch does, and the exact C to insert in that vanilla function
   under `#ifdef REGION_REMIX`. When the inserted C must call one of your
   routines, make that routine non-static, name it, and give its prototype.
   Do not edit the vanilla file yourself.
2. **Shared Remix subsystems.** These include custom items (`Item.*.SPAWN_ITEM`),
   custom GFX (`GFX.*`, `GFXRoutine.*`), `PokemonAnnouncer.*`, `Size.*` and
   other helpers outside your fighter's directory. If a helper is small and
   self-contained, port it as a `static` function in your file. Otherwise
   write the surrounding logic faithfully and call nothing in its place, with
   a `/* REMIX-TODO(<subsystem>): <what is skipped> */` marker. Then list it
   in your notes with the source location.
3. **Uncertainties.** List every place where you were unsure of the
   semantics, with the asm line numbers.

## Rules

- Edit only your assigned C file(s) and your notes file. Do not touch any
  other file. In particular, do not edit `ftremixported.c`, other fighters'
  files, vanilla decomp files or the generators.
- Do not run cmake, make, ninja, the full build or the game. Other workers
  are running in parallel.
- Verification is mandatory. Run
  `/Users/lucas/Projetos/Pessoal/BattleShip/scripts/remix/syntax-check.sh <your file>`
  (repository-relative or absolute path) until it prints `syntax-check OK`
  with no warnings from your file.
- Do not create commits.

## Final report (at most 25 lines)

- File(s) written, table name(s), and the number of routines ported versus
  the number in your scope.
- Approximations and `REMIX-TODO` markers, one line each.
- The path to your notes file.
