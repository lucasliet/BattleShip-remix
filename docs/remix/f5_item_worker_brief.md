# F5 item worker brief: port one Smash Remix custom item to C

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths,
and use `builtin cd`, never plain `cd`.

Read `docs/remix/f5_worker_brief.md` first. Its references, translation rules,
LP64 rules and verification rules all apply here. This brief replaces its
"export one FTRemixProc table" part with the item interface below.

## Context

Remix adds items with ids starting at 0x2D (`.toolchain/pkgs/smashremix-src/src/Item.asm`,
`add_item`). Each item `.asm` in `src/items/` defines six things:

- `SPAWN_ITEM`, the stage setting routine;
- `SHOW_GFX_WHEN_SPAWNED`;
- `PICKUP_ITEM_MAIN`, the consume-type pickup;
- `PICKUP_ITEM_INIT`, the pre-pickup or hold routine;
- `DROP_ITEM`;
- `THROW_ITEM`;
- `PLAYER_COLLISION`, the touch-type routine.

It also defines an `item_info_array`, which is the decomp `ITDesc`, and an
`item_state_table`, which is an `ITStatusDesc[]`.

The port already has the shared infrastructure, so do not edit it:

- `decomp/src/it/itremix/itremix.h` declares `ITRemixItemDesc` and `itRemixGetItemDesc`.
- Vanilla dispatch already routes ids >= 0x2D to your descriptor:
  - `itManagerMakeItemSetupCommon` and `itManagerMakeItemKind` (spawn and spawn gfx);
  - `itMainSetFighterHold`, the pre-pickup routine;
  - `itMainSetFighterDrop`;
  - `itMainSetFighterThrow`;
  - `ftCommonLightGetProcDamage`, the consume routine;
  - `ftMainUpdateDamageStatItem`, the touch routine.

Item ids follow the `add_item` order:

| Id | Item |
|---|---|
| 0x38 | MrSaturn |
| 0x42 | Gem |
| 0x43 | Shuriken |
| 0x44 | Boomerang |
| 0x45 | ClanBomb |
| 0x46 | WaddleDee |
| 0x47 | WaddleDoo |
| 0x48 | Gordo |
| 0x49 | Cloud |
| 0x4A | Flashbang |
| 0x4B | Turnip |

## What to write

1. `decomp/src/it/itremix/itremix<name>.c`, wrapped in `#ifdef PORT`. It includes
   `"it/itremix/itremix.h"`, `"remix/remix_roster_data.h"` when needed, and the
   decomp headers it uses.
   - Port the `item_info_array` as a `static ITDesc` and the `item_state_table` as a
     `static ITStatusDesc[]`.
     - Each vanilla address becomes its decomp function. Look it up in
       `build/remixasm/refs/vanilla_funcs.txt`. For example, 0x801744C0 is a Maxim
       Tomato proc.
     - `0x8016E174` is `itManagerMakeItem(parent_gobj, &desc, pos, vel, flags)`.
   - **Item file.** `dw Character.<NAME>_file_<N>_ptr` is the owner fighter's file N:
     - N = 6 is `fp->data->p_file_special1`;
     - N = 7 is `fp->data->p_file_special2`;
     - N = 8 is `fp->data->p_file_special3`;
     - N = 9 is `fp->data->p_file_special4`.

     Each of these is a `void **` that can go straight into `ITDesc.p_file`. Set
     `desc.p_file` from the owner fighter before every `itManagerMakeItem` call, for
     example by copying the static desc to a stack `ITDesc`. Remix items always have a
     fighter owner when they spawn.

     Items whose file is a Remix item file loaded some other way (`File.*` with no
     character pointer) are a REMIX-TODO. Record which file in your notes.
   - **Attributes offset.** The item footer offset (`0x08`) goes into
     `o_attributes`, as in `decomp/src/it/itcommon/ittomato.c`.
   - Port every routine in the item `.asm` to C:
     - status procs return `sb32` and take the item `GObj`;
     - item state lives in `ip->item_vars` (check `decomp/src/it/itvars.h` for a
       fitting struct, or define a file-local struct cast over `ip->item_vars`, with a
       `_Static_assert` that it fits).
2. Export exactly one descriptor:

   ```c
   const ITRemixItemDesc dITRemix<Name>ItemDesc =
   {
       0x4B,                      /* Item.Turnip.id */
       itRemixTurnipMakeItem,     /* SPAWN_ITEM (vanilla ABI: parent, pos, vel, flags) */
       TRUE,                      /* SHOW_GFX_WHEN_SPAWNED */
       itRemixTurnipHoldProc,     /* PICKUP_ITEM_INIT, or NULL */
       itRemixTurnipConsumeProc,  /* PICKUP_ITEM_MAIN for consume items, or NULL */
       itRemixTurnipDroppedProc,  /* DROP_ITEM, or NULL */
       itRemixTurnipThrownProc,   /* THROW_ITEM, or NULL */
       NULL                       /* PLAYER_COLLISION, or NULL */
   };
   ```

   The `proc_make` signature is `GObj* (GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.
   Remix's `SPAWN_ITEM` often takes extra registers, such as a1 = item info array or
   a3 = an "unknown" flag. Map those onto this signature. For example,
   `flags` = the a3 boolean, the same as vanilla `itManagerMakeItemSetupCommon`.
3. A header `decomp/src/it/itremix/itremix<name>.h` that declares the spawn function
   that fighter code calls. It is usually the same `MakeItem` function. If the
   fighter's call site passes a different set of arguments, add a second, clearly
   named wrapper. Fighter call sites are wired later by another worker. Do not edit
   fighter files.

## Rules

- Edit only your own `itremix<name>.c`, `itremix<name>.h` and notes file
  `build/remixasm/notes/item_<Name>.md`. Do not edit `itremixitems.c`, which is
  generated, any vanilla file, `itremix.h` or another item.
- Record vanilla patches the item relies on in the notes, with the exact C, but do
  not apply them. Do the same for shared subsystems (GFX, Size, PokemonAnnouncer),
  marked `REMIX-TODO`.
- Never write ROM bytes into C files. Never access ITStruct, WPStruct or FTStruct by
  raw byte offset; use named fields. `build/remixasm/refs/struct_offsets.txt` lists
  ITStruct as well. Treat its offsets as approximate past pointer fields: map them
  by meaning and check the neighbouring fields.
- Verify with `scripts/remix/syntax-check.sh decomp/src/it/itremix/itremix<name>.c`
  until it prints `syntax-check OK` with no warnings from your file.
- No cmake, build, game runs or commits.

## Final report (at most 20 lines)

List:

- the files you wrote, the descriptor name and the item id;
- the routines ported versus the routines in the `.asm`;
- the item file used;
- the spawn function signature fighter code must call;
- the REMIX-TODOs;
- the path to your notes file.
