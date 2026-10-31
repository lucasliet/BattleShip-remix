# f14 items_c: REMIX-TODO resolution (ROADMAP T2.11)

Scope: `decomp/src/it/itremix/` for WaddleDoo, WaddleDee, ConkerGrenade, DekuNut, GoldenGun, Gordo, Psyduck, MrSaturn.
Asm source: `.toolchain/pkgs/smashremix-src/src/items/` (`Conker/ConkerSpecial.asm` for the conker grenade).
Offsets: N64 = `struct_offsets.txt` value, plus 4 where the N64 offset is past the `damage_coll` hitstatus (0x248 keeps
hitstatus, as in Basketball). Verified on `ITStruct`: N64 0x2CE is `times_landed` (`andi 0xCF`), N64 0x33E is `multi`.

## Rows

| Original TODO | State | Decision | Asm |
|---|---|---|---|
| itremixwaddledoo.c `image` (beam init) | implemented | `DObj.mobj->texture_id_curr = 6` (jumping image) | WaddleDoo.asm `_spawn_beam_initial`, `sh at, 0x0080(v0)` |
| itremixwaddledoo.c `hitbox-flag` | implemented | `ITRemixWaddleDeeVars.is_hitbox_collided = FALSE` (N64 0x01D4 is this field in the shared minion layout) on every attack_main exit, in `itRemixWaddleDooAttackProcUpdate` | WaddleDoo.asm 226 (`_end_2`, shared by all exits) |
| itremixwaddledoo.c `image` (initial) | implemented | `texture_id_curr = 5`, `mobj->aobj = NULL`, `mobj->matanim_joint.event16 = NULL` | WaddleDoo.asm `waddle_doo_attack_initial`, 0x0080/0x0090/0x0094 |
| itremixwaddledoo.c `beam-multihit` | implemented | `attack_coll.attack_records[0].victim_gobj = NULL` on the beam's movement frames (N64 0x0214, WP attack_records) | WaddleDoo.asm 788 |
| itremixwaddledoo.c header and beam doc comments | comment | stale "stub" wording removed | none |
| itremixdekunut.c `unverified` (DObj 0x54) | implemented | `DObj.flags = DOBJ_FLAG_HIDDEN` (value 2). The asm comment "hurtbox sound" is wrong: the 0x0408 store is `attack_coll.fgm_id`, already ported | DekuNut.asm 363 |
| itremixconkergrenade.c `DObj` (0x54) | implemented | `joint->flags = DOBJ_FLAG_HIDDEN` | ConkerSpecial.asm 1668 |
| itremixgordo.c `rumble` | implemented | `itRemixGordoRumblePlayer(ip->player)`: `gSCManagerBattleState->players[p].fighter_gobj`, then `ftParamMakeRumble(fp, 6, 0)`. Skips port 4 and NULL fighters | Gordo.asm 700-735; table 0x800A50E8 is `gSCManagerBattleState` (`scmanager.c:67`) |
| itremixgordo.c `gfx` | implemented | `gcAddXObjForDObjFixed(DObjGetStruct(item_gobj), 0x2E, 0)`. `0x80008CC0` is this function (`objman.c:1197`) | Gordo.asm 110; `Dango`, `Cloud` and `Basketball` ports use the same call |
| itremixpsyduck.c `file`, itremixmrsaturn.c `file` | resolved | File pointer `0x8018D040` is `gITManagerCommonData` (`itmanager.c:121`), the same as Dango, BlueShell and PoisonMushroom. The comment now states the asm word. **Not verified:** that footer offset 0x0BB0 (Psyduck) and 0x10B0 (MrSaturn) exists in file 0xFB; that needs the ROM | Psyduck.asm 26-27; MrSaturn.asm 35-36 |
| itremixwaddledee.c header (`owner minion slots`) | **blocked, marker kept** | Dedede's minion slots (FTStruct 0x0ADC and 0x0AE0, read by `remove_minion_check_` and `minion_free_`) need a Dedede fighter accessor (`ftremixdedededsp.c`). The port does not have it | WaddleDee.asm 1764-1776; `build/remixasm/notes/item_WaddleDee.md` item 1 |
| itremixgoldengun.c `tilt` | **blocked, line untouched** | The asm reads the floats at RAM `0x8019CD00` / `0x8019CD04` (`lui 0x8019`), not `0x8018CD00` as the TODO says. The values are not in the decomp or the port, and no validated RAM-to-ROM map was found to read them | GoldenGun.asm 77-95 |

## Uncertainties

- Psyduck and MrSaturn: footer existence in file 0xFB (see row above).
- Waddle Doo beam: the asm writes 0x0214 on the movement frames only; the port does the same (after the position update).
- The `item_vars` budget for `ITRemixWaddleDooVars` is 40 bytes and is at its limit (`_Static_assert`). Adding a field there
  needs a larger union or a different home.
