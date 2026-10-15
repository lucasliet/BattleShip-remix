# 42 PEPPY NSP notes

Port: `decomp/src/ft/ftremix/peppy/ftremixpeppynsp.c`, table `dFTRemixPeppyNSPProcs` (15 entries) and `dFTRemixPeppyNSPProcsNum`. Source: `.toolchain/pkgs/smashremix-src/src/Peppy/PeppySpecial.asm`, scope `PeppyNSP`.

Routines ported: 15 of 15 in the worklist.

## 1. Vanilla patches

None. The scope has no `OS.patch_start` or `j`/`jal` hooks into vanilla code. Two `OS.copy_segment` fragments splice vanilla Samus code, and they are ported as C (see section 3):

- `ground_shoot_initial_`: `copy_segment(0xD84A0, 0x14)` (vanilla 0x8015CCA0, Samus 0x8015DA60 frame).
- `air_shoot_initial_`: `copy_segment(0xD84E8, 0x3C)` (vanilla 0x8015CCE8, Samus 0x8015DAA8 frame, includes the grounded to aerial conversion).
- `air_shoot_transition_`: `copy_segment(0xD8444, 0x20)` (vanilla 0x8015CC44, includes `mpCommonSetFighterAir`).

## 2. Shared Remix subsystems

None. The move has no custom item, so no `REMIX-TODO(Item.*)` marker is needed. The laser's projectile callbacks reuse the vanilla Fox blaster (`wpFoxBlasterProcUpdate`, `wpFoxBlasterProcReflector`, from `wp/wpfox/wpfoxblaster.h`).

The blast particle (`create_orange_blast_gfx_`, asm line 1309) is ported as a file-local helper. It calls `lbParticleMakeCommon(gEFManagerParticleBankID, 0x8D)`, the same call the asm makes with bank pointer 0x801313C4.

## 3. Ported vanilla fragments

The copy_segment bytes were decoded with `tools/remix_disasm_rom.py`:

- Ground shoot, 5 instructions: frame `addiu sp,-0x28`, `sw ra`, `lw t6,0x84(a0)` (fp), `addiu t7,r0,2`, `sw t7,0x10(sp)`. Port: `ftRemixPeppyNSPShootInitial` with flags `FTSTATUS_PRESERVE_COLANIM` (2).
- Air shoot, 15 instructions: when `ga == 0` it calls `mpCommonSetFighterAir` (0x800DEEC8) and `ftPhysicsClampAirVelXMax` (0x800D8EB8), then sets flags 2. Port: `ftRemixPeppyNSPAirShootInitial`.
- Air shoot transition, 8 instructions: calls `mpCommonSetFighterAir` unconditionally, flags 2. Port: `ftRemixPeppyNSPAirShootTransition`.

## 4. Kirby action ids

The Kirby ids are `Kirby.Action.PEPPY_NSP_*`, assigned by `Character.add_new_action_params` and not written as literals in the source. They were read from the built ROM (`build/remixasm/remix.z64`): entry = `KIRBY_action_array (0x804A1BA0)` + `(id - 0xDC) * 0x14`, with the routine pointer at entry + 4. The check on `0xE7` and `0x127` returned the expected routines.

| Routine | Kirby id |
|---|---|
| ground begin (begin_main_) | 0x195 |
| ground charge (charge_main_) | 0x196 |
| ground shoot (shoot_main_) | 0x197 |
| air begin | 0x198 |
| air charge | 0x199 |
| air shoot | 0x19A |

## 5. Uncertainties (asm line numbers in PeppySpecial.asm)

1. **Escape item-throw tics, lines 561-569.** `ground_charge_interrupt_` passes the fighter GObj pointer (register a2 holds the gobj, loaded from 0x20(sp)) as the third argument of `ftCommonEscapeSetStatus`. The port passes `0`, as vanilla Samus does (`ftsamusspecialn.c`). If the Remix value must be kept, the port would have to store a pointer in an `s32`, which is not portable.
2. **Shoot end flags, line 974.** `shoot_main_` `_change_action` calls `0x800E6F24` with no store to 0x10(sp), so argument 4 is a stale stack word. The port passes `FTSTATUS_PRESERVE_NONE`.
3. **Stale `v0` in the shoot idle path, lines 921-928 and 988-989.** After `laser_stage_setting_` returns, `v0` holds the new weapon GObj (or 0), and the idle path writes `sw r0, 0x17C(v0)`. The fighter flag0 was already cleared at line 893, so the port writes `fp->motion_vars.flags.flag0 = 0` on the fighter. The asm store may land on the weapon or on address 0 instead.
4. **Spark charge field, line 514.** `charge_main_` stores the charge level at WPStruct `0x2A4`. The port uses `weapon_vars.charge_shot.charge_size`, the field the vanilla Samus charge uses (`ftsamusspecialn.c` line 158). `build/remixasm/refs/struct_offsets.txt` labels `0x2A4` as `weapon_vars.charge_shot.owner_gobj`. The two disagree and charge_size was chosen because it matches the charge semantics.
5. **Laser transform types, line 1396.** The projectile word `0x1C000000` is written to `transform_types.tk1 = 0x1C`, raw, as `decomp/src/ft/ftremix/sheik/ftremixsheiknsp.c` does with raw N64 bytes. The Fox blaster uses the named `nGCMatrixKindTraRotRpyRSca`, which has a different value in the decomp enum (index 15). The raw value follows the Remix data.
6. **Laser vertical velocity.** The asm never sets `vel_air.y` for the laser. Its value comes from `wpManagerMakeWeapon` initialisation (line 1262 reads it for the angle). The port does not set it either.
7. **Laser spawn offsets, lines 898-923.** Peppy: x 128.0 (0x43000000), y 52.0 (0x42500000). Kirby and J Kirby: x 160.0 (0x43200000), y -32.5 (0xC2020000). The gun joint is `fp->joints[17]` (N64 0x92C = 0x8E8 + 0x44). The z component is zeroed before and after `gmCollisionGetFighterPartsWorldPosition`.
8. **Begin charge at maximum, lines 99-107.** When the charge is already 5, `is_release` becomes 1. The asm also rewrites charge 5, which the port keeps.
9. **Air charge button masks, lines 605-607.** The air charge uses the Joypad constants A 0x8000 and B 0x4000 (`Joypad.asm`). The ground charge and both begins use the fighter masks 0x1B4 and 0x1B6. The port keeps these two different sources as in the asm.

## 6. Not ported (outside the worklist)

- `ground_shoot_interrupt_` and `air_shoot_interrupt_` (asm lines 1000-1063 and 1034-1063) are not in the 15-routine worklist and are not referenced from the Peppy action array in this scope. They are not ported.
- `kirby_ground_begin_collision_`, `kirby_ground_charge_collision_` and `kirby_ground_shoot_collision_` (asm lines 284, 650, 1080; `0x80573ED8`, `0x805742D4`, `0x805747B4`) are Kirby-table routines and are not in this worklist. They call the same vanilla common collisions with `ftRemixPeppyNSP` transitions, but those transitions are static here. If the Kirby worker needs them, make the transitions non-static and declare them in a header.
- `air_collision_`, `air_to_ground_`, `laser_stage_setting_` details, `blaster_projectile_collision_check_` and the other helpers are covered by the port, not by table entries.
