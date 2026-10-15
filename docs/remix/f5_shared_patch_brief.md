# F5 shared-patch worker brief: apply Remix `*shared.asm` vanilla patches in C

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute
paths and `builtin cd`.

Read `docs/remix/f5_worker_brief.md` first (references, translation rules, LP64
rules, verification rules). This brief changes one rule: **you do apply the
vanilla patches yourself**, because here the patches are the whole job.

## Context

Remix clone fighters (Dr. Mario, J Mario, Piano, J Link, Falcon clones, ...) are
synth fighters with their own fkind (= Remix character id, `REMIX_ID_*` in
`remix/remix_roster_data.h`). A `*shared.asm` file patches vanilla code
(`OS.patch_start(rom, ram)`, `Character.table_patch_start(table, id, ...)`) so
that the vanilla routines treat the clones like the parent, or differently.
Without those patches the clone plays the parent's move with the parent's
data (wrong projectile, wrong sound, wrong status ids, crashes).

Each patch's RAM address maps to a decomp function through
`build/remixasm/refs/vanilla_funcs.txt` (sorted by address; the function that
contains the address is the last entry at or below it). Port the patch as C in
that function.

## How to apply a patch

1. Read the whole `.asm` file for your scope and list its patches.
2. For each patch: locate the vanilla function, read it, and insert the Remix
   behaviour guarded so vanilla stays byte-for-byte equivalent for vanilla
   fighters:

   ```c
   #ifdef REGION_REMIX
       <remix variant>
   #else
       <original line(s)>
   #endif
   ```

3. Put non-trivial logic in a helper under `decomp/src/ft/ftremix/` (or
   `decomp/src/wp/`, `decomp/src/it/itremix/`) in a new `.c` file you own, wrapped
   in `#if defined(PORT) && defined(REGION_REMIX)`, and declare it in
   `decomp/src/ft/ftremix/ftremixhooks.h` (append a block with a comment naming
   your scope; append only, never reorder existing lines). The vanilla file
   then calls a one-line hook. Include `"ft/ftremix/ftremixhooks.h"` under
   `#ifdef REGION_REMIX` in the vanilla file.
4. Fighter id checks use `fp->fkind == REMIX_ID_X`. Kirby powers are in
   `fp->passive_vars.kirby.copy_id` (N64 0x0ADC) and the hat id byte via
   `ftRemixFighterKirbyHat(fp)` (`ft/ftremix/ftremixfields.h`).
5. Remix scratch words use `ftRemixFighterWord()` or named fields. Never raw byte
   offsets on FTStruct/WPStruct/ITStruct. `struct_offsets.txt` is 4 bytes low
   from ~0x25C on WPStruct and ~0x248 on ITStruct.
6. Remix data a patch loads (`dw` tables, float arrays, WPDesc/projectile
   structs) becomes `static const` C data. Never ROM bytes in C. A projectile
   struct pointing at `Character.<NAME>_file_N_ptr` becomes
   `fp->data->p_file_special<N-5>` (N = 6..9 -> special1..4) or `p_file_main`
   (N = 1), set on a copy of the vanilla descriptor before the spawn call.
   Remember: `ITDesc`/`WPDesc` copies, never mutate the vanilla static.
7. Patches that exist only for the CPU/AI (`cpu_*`, `recovery_logic`,
   `ledge_check`, `cpu_attack_weight`, `*_cpu_*`) are `REMIX-TODO(AI)`: skip
   them and list them in your notes.
8. Patches for subsystems that have no port yet (Size, GFX routines, PokemonAnnouncer,
   env color) are `REMIX-TODO(<subsystem>)`: apply the logic around them and list
   them in the notes.

## Rules

- Several workers edit vanilla files at the same time. Use `Edit` with small,
  unique `old_string`s and re-`Read` the file right before each edit. Never use
  `Write` on an existing vanilla file. Keep each edit minimal.
- Only touch the vanilla files your patches need, plus your own new files and
  your notes. Do not touch other fighters' `ftremix/<name>/` files.
- Syntax-check every file you touch with
  `scripts/remix/syntax-check.sh <file>` until `syntax-check OK`. Run it after
  your last edit to each file, since other workers may have edited it.
- No cmake, no builds, no game runs, no commits.
- Notes: `docs/remix/notes/shared_<scope>.md` listing each patch
  (address, vanilla function, applied / skipped and why), uncertainties with asm
  line numbers, and every `REMIX-TODO`.

## Final report (at most 20 lines)

Patches applied versus patches in the asm, files touched, helpers exported,
REMIX-TODOs, notes path.
