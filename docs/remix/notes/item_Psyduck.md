# Item 0x41 Psyduck notes

Port: `decomp/src/it/itremix/itremixpsyduck.c`, header `decomp/src/it/itremix/itremixpsyduck.h`.
Descriptor: `dITRemixPsyduckItemDesc` (`ITRemixItemDesc`, kind 0x41 = `Item.Psyduck.id`, 0x2D + 20 in `Item.asm` `add_item` order).
Source: `.toolchain/pkgs/smashremix-src/src/items/Pokemon/Psyduck.asm` (139 lines).

## Spawn signature for fighter and Poke Ball code

`GObj* itRemixPsyduckMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

Mapping from `stage_setting_`: a0 = parent, a1 = pos, a2 = vel, a3 = flags (s0). Same ABI as
`itManagerMakeItem`, so no wrapper is needed. The asm reads the flags from a3 and passes them as the
fifth argument of `itManagerMakeItem` (`sw s0, 0x10(sp)`).

## Item file

`item_info_array` word 0x04 is `0x8018D040`, the vanilla pointer to file 0xFB. The port sets
`ITDesc.p_file = &gITManagerCommonData`, as in `itremixmrsaturn.c`. `o_attributes` is `FILE_OFFSET` 0x0BB0
(word 0x08, "hitmonlee offset").
REMIX-TODO(file): the footer at 0x0BB0 must exist in file 0xFB as patched by Remix. Not verified in this
pass (no build run, per brief).

## Descriptor fields

| Field | Value | Source |
|---|---|---|
| `ITDesc.transform_types` | `{nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}` | word 0x0C = 0x1B000000 |
| `attack_state` | `nGMAttackStateOff` | word 0x10 = 0 |
| `proc_update` | `itSawamuraCommonProcUpdate` (0x80182AE0) | word 0x14 |
| `proc_map` | `itSawamuraCommonProcMap` (0x80182B34) | word 0x18 |
| `is_show_spawn_gfx` | FALSE | `SHOW_GFX_WHEN_SPAWNED(OS.FALSE)` |
| `proc_hold`, `proc_consume`, `proc_dropped`, `proc_thrown`, `proc_touch` | NULL | `PICKUP_ITEM_INIT(0)`, `PICKUP_ITEM_MAIN(0)`, `DROP_ITEM(0)`, `THROW_ITEM(0)`, `PLAYER_COLLISION(0)` |

## Item state table

Not ported as a separate table. The asm `STATE_TABLE` has three states, and each one is identical to a
vanilla Sawamura status (`decomp/src/it/itmonster/itsawamura.c`, `dITSawamuraStatusDescs`):

| Asm state | Asm main / collision | Vanilla status index |
|---|---|---|
| 1 | 0x80182630 / 0x80182660 (`itSawamuraFallProcUpdate` / `itSawamuraFallProcMap`) | 0 Fall |
| 2 | 0x801826D0 / 0x80182714 (`itSawamuraWaitProcUpdate` / `itSawamuraWaitProcMap`) | 1 Wait |
| 3 | 0x80182764 / 0 (`itSawamuraAttackProcUpdate` / NULL) | 2 Attack |

The Psyduck asm has no setter routines of its own. Every status change is made by these vanilla procs
through `dITSawamuraStatusDescs`, so the port does not define a second table, which would be unused.
Asm state N is vanilla index N-1.

## Routines ported (asm scope, C routine)

Asm scopes: `stage_setting_` only (the `item_info_array` and `STATE_TABLE` are data). Ported: 1 of 1.

| Asm scope | Line | C routine |
|---|---|---|
| `stage_setting_` | 68 | `itRemixPsyduckMakeItem` (exported) + `itRemixPsyduckSetup` |

Setup steps, in asm order:
- `itManagerMakeItem` with `item_info_array`, pos, vel, flags.
- `ip->multi = ITMONSTER_RISE_STOP_WAIT` (asm `sh 0x16` at ITStruct +0x33E).
- `vel_air = {0, ITMONSTER_RISE_VEL_Y, 0}` (asm words +0x2C, +0x30, +0x34).
- `gcAddXObjForDObjFixed(dobj, 0x48, 0)`.
- `translate = *pos`, then `translate.y -= attr->map_coll_bottom`.
- `gcAddDObjAnimJoint(dobj, attr->data - 0x11F40 + 0x13624, 0.0F)`, written as `itGetPData`.
- `func_800269C0_275C0(0x013E)`, the appear FGM.
- `gcMoveGObjDLHead(item, 18, item->dl_link_priority)`.

## Vanilla patches

The asm has no `OS.patch_start` of its own. The Pokemon selection it depends on is in `Item.asm`, and
the port does not implement it (only recorded here, per brief):

1. `Item.asm` `whos_that_pokemon`: `OS.patch_start(0xEDCA0, 0x80173260)` followed by `jal whos_that_pokemon; nop`.
   The RAM address 0x80173260 is inside `itMainMakeMonster` (`decomp/src/it/itmain.c:760`, starts at
   0x80173228). The exact vanilla instructions there were not read in this pass. Psyduck is one of the
   entries of `pokemon_table` (`Item.asm` line 2478).
2. `Item.asm` `whos_that_pokemon_setup`: `OS.patch_start(0xE9C58, 0x8016F218)` replaces the start of
   `itManagerInitMonsterVars` (`decomp/src/it/itmanager.c:745`) with `j whos_that_pokemon_setup.clean_table`
   plus 7 nops.

No C is needed for either one inside the Psyduck port. Both belong to the Poke Ball and Pokemon
selection work.

## REMIX-TODO

- `REMIX-TODO(file)`: see Item file above (file 0xFB footer at 0x0BB0).
- No other REMIX-TODO. There is no GFX, Size, announcer or FGM variant dependency in the asm. The one FGM is
  a literal.

## Uncertainties

- Asm line 127 (`addiu a0, r0, 0x013E`): the FGM is written as a literal. It was not matched to a
  `nSYAudioVoiceMBallSawamuraAppear` enum value, which the vanilla Sawamura spawn uses.
- Asm line 111 to 125: the anim address uses `attr->data + 0x13624 - 0x11F40`. The Remix anchors are
  copied as constants, not verified against a symbol.
- `ITStruct` offsets in `build/remixasm/refs/struct_offsets.txt` (attr 0x02D0, multi 0x033A) are
  approximate. The asm uses 0x02D4 and 0x033E. The port uses the named fields `attr` and `multi`.
- `Item.asm` gives the Psyduck id through `add_item`; the id is `0x41` (0x2D + 20), as in the brief table.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
