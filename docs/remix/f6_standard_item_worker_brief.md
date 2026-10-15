# F6 standard item worker brief: port one Smash Remix standard item to C

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`.

Read these first, in this order. Everything in them applies unless this file says otherwise:

1. `docs/remix/f5_item_worker_brief.md` (item interface, file layout, rules, final report).
2. `docs/remix/f5_worker_brief.md` (references, translation rules, LP64 rules, `OS.copy_segment` decoding with
   `tools/remix_disasm_rom.py`).
3. One finished item of the same shape as yours, for structure: `decomp/src/it/itremix/itremixgem.c`
   (held and thrown item), `itremixmrsaturn.c` (standard item, pickup and effect).

## What differs for standard items

Standard items (ids 0x2D to 0x41) are not owned by a fighter. They come from the stage item spawner, container
drops, taunt spawns and the Training menu, so `parent_gobj` is usually NULL and there is no owner file.

- **Item file.** `dw 0x8018D040` in the `item_info_array` is the vanilla global that holds file 0xFB, the common item
  file (`void *gITManagerCommonData`, `decomp/src/it/itmanager.c`). Remix patched that file in place and appended the
  custom item models, so `FILE_OFFSET` is the item footer inside it. In C this is `ITDesc.p_file = &gITManagerCommonData`
  and `o_attributes = FILE_OFFSET`. Look at how `decomp/src/it/itcommon/ittomato.c` builds its `ITDesc` for the
  vanilla equivalent.
- **Spawn routine.** Most standard items use `spawn_custom_item_based_on_star_` or a similar routine that splices
  vanilla Star or Maxim Tomato spawn code with `OS.copy_segment`. Decode each fragment with
  `.toolchain/host/venv/bin/python -I tools/remix_disasm_rom.py <rom_offset> <size>` and port the instructions. The
  result is a `proc_make` with the signature `GObj* (GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`. Remix
  overrides the spawn velocity in several of these routines (for example the mushroom's alternating horizontal speed
  per spawn this frame); keep that behaviour.
- **Per-player state.** Items such as the Super Mushroom, Poison Mushroom, Lightning, Franklin Badge and Stopwatch
  keep a per-player effect timer in a registered routine object or a table indexed by player. In C, keep it in a
  file-local static array indexed by `fp->player` and a `gcAddGObjProcess`-style update (look at how
  `itremixmrsaturn.c` and the `ftremix` Sonic spring handle it) and clear it in the scene-end path the asm uses. If
  the asm hooks a vanilla routine to run its per-frame work (a `OS.patch_start` in the same file), do not apply
  the patch: record the vanilla function, the exact C to add and the reason in your notes file under `Vanilla patches`.
- **Shared subsystems that the port does not have yet** (GFX, GFXRoutine, Size, PokemonAnnouncer, Reflect, FGM
  variants): call the closest existing port function when there is an obvious one, otherwise skip the effect and mark
  it `REMIX-TODO` in code and in your notes. A missing visual effect must never make the item unusable.
- **Ids.** Use these as `kind` in your descriptor and as the first field of the `ITDesc`:

| Id | Item | Id | Item |
|---|---|---|---|
| 0x2D | CloakingDevice | 0x38 | MrSaturn (done) |
| 0x2E | SuperMushroom | 0x39 | Stopwatch |
| 0x2F | PoisonMushroom | 0x3A | KlapTrap |
| 0x30 | BlueShell | 0x3B | RobotBee |
| 0x31 | Lightning | 0x3C | Car |
| 0x32 | DekuNut | 0x3D | BulletBill |
| 0x33 | FranklinBadge | 0x3E | ScuttleMinion |
| 0x34 | Pitfall | 0x3F | Cacodemon |
| 0x35 | GoldenGun | 0x40 | Basketball |
| 0x36 | Dango | 0x41 | Psyduck |
| 0x37 | Pwing | 0x42 | Gem (done) |

## Files

- Source: `.toolchain/pkgs/smashremix-src/src/items/<Name>.asm` (Psyduck: `items/Pokemon/Psyduck.asm`). Routines in
  other Remix files that the item calls (`Hazards.asm`, `Item.asm`, `Size.asm`, `GFX.asm`, `FGM.asm`) are read-only
  references.
- Output: `decomp/src/it/itremix/itremix<lowercasename>.c`, `.h`, and the notes file
  `docs/remix/notes/item_<Name>.md`. File name is the item name in lowercase without separators
  (`itremixsupermushroom.c`, `itremixblueshell.c`).
- Export `const ITRemixItemDesc dITRemix<Name>ItemDesc` exactly as in the f5 brief. Do not edit `itremixitems.c`, the
  lead regenerates it with `tools/gen_remix_items.py`.

## Verification

`scripts/remix/syntax-check.sh decomp/src/it/itremix/itremix<name>.c` must print `syntax-check OK` with no warning
from your file. Do not run cmake, the build, the game or git. Do not edit any vanilla file.

## Final report (at most 20 lines)

The files you wrote, descriptor name, id; routines ported versus routines in the asm; `ITDesc` file and offset; the
spawn function signature; the vanilla patches you recorded (one line each, with the vanilla function and address);
the REMIX-TODOs; the notes path.
