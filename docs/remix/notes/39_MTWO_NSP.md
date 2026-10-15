# 0x39 MTWO: NSP (Shadow Ball) port notes

Port file: `decomp/src/ft/ftremix/mewtwo/ftremixmewtwonsp.c`
Table: `dFTRemixMewtwoNSPProcs` / `dFTRemixMewtwoNSPProcsNum` (13 entries, every `MewtwoNSP.*` worklist routine).
Not ported as table entries (not in the worklist): the Kirby-only routines `MewtwoNSP.kirby_ground_begin_collision_`, `kirby_charge_main_`, `kirby_shoot_main_`, `kirby_ground_charge_collision_`, `kirby_ground_shoot_collision_`. Kirby's copies of these moves will fall back to Kirby's parent routines until someone ports them.
Exported non-static symbol: `dFTRemixMewtwoShadowBallWeaponDesc` (needed by the vanilla spawn patch below).

## 1. Vanilla patches the scope relies on (not applied; do not edit vanilla files from this port)

### 1a. Shadow Ball spawn: `wpSamusChargeShotMakeWeapon` (0x80168DDC)
- Decomp: `decomp/src/wp/wpsamus/wpsamuschargeshot.c` (`wpSamusChargeShotMakeWeapon`, line ~274).
- Remix: `.toolchain/pkgs/smashremix-src/src/samusshared.asm:81` (`get_chargeinstructions_struct_`, hook at 0x80168E14) selects `MewtwoNSP.projectile_struct` for `Character.id.MTWO`. The port uses `dFTRemixMewtwoShadowBallWeaponDesc` instead.
- Required C under `#ifdef REGION_REMIX`, inside `wpSamusChargeShotMakeWeapon` before `wpManagerMakeWeapon`:
  ```c
  extern WPDesc dFTRemixMewtwoShadowBallWeaponDesc;
  /* ... */
  if (fp->fkind == REMIX_ID_MTWO)
  {
      dFTRemixMewtwoShadowBallWeaponDesc.p_weapon = fp->data->p_file_special3;
      desc = &dFTRemixMewtwoShadowBallWeaponDesc;
  }
  ```
  Use a local `WPDesc *desc` for the spawn call. Mewtwo's launch goes through the port's `apply` path, not `wpSamusChargeShotLaunch`.
- `REMIX_ID_MTWO` is in `port/remix/remix_roster_data.h`. The file 8 mapping (`p_file_special3`) follows the brief rule N = 8.
- Open problem: the vanilla spawn writes `charge_size` (WPStruct 0x2A0) and `owner_gobj` (0x2A4) for Samus's fields. The Mewtwo overlay reads 0x2A0 as `begin_shot` and 0x2A4 as `charge_level`. See section 3.

### 1b. Charge FGM for Mewtwo: `charge_fgm_fix_` (0xE38C4 → 0x80168E84)
- Decomp: `wpSamusChargeShotMakeWeapon` again (its `ftParamPlayLoopSFX` takes the charge FGM from `dWPSamusChargeShotWeaponAttributes`).
- Remix: `MewtwoSpecial.asm:1074-1103`. For Mewtwo, the charge FGM comes from the Mewtwo table (`charge_fgm_id`, the 0x18 field of each level). The port's table `sFTRemixMewtwoShadowBallLevels` holds those values. Vanilla must read them for Mewtwo.

### 1c. Ball position: `ball_position_fix_` (0xD7D9C → 0x8015D35C)
- Decomp: `ftSamusSpecialNGetChargeShotPosition` (`decomp/src/ft/ftchar/ftsamus/ftsamusspecialn.c:35`).
- Remix: `MewtwoSpecial.asm:1184-1228`. For Mewtwo it zeroes the position, then calls `gmCollisionGetFighterPartsWorldPosition(fp->joints[part 0x20 "grab"], pos)`. The asm reads `0x0978(fp)`. The port calls the vanilla function, so this patch must exist for Mewtwo to hold the ball correctly.

### 1d. Shot velocity: `shoot_velocity_patch_` (0xD82B8 → 0x8015D878)
- Decomp: `ftSamusSpecialNEndProcUpdate` (`ftsamusspecialn.c:217`), reached by the port's `shoot_main_` through `ftSamusSpecialNEndProcUpdate`.
- Remix: `MewtwoSpecial.asm:1107-1141`. For Mewtwo the x velocity is `orig + SHOOT_X_SPEED(0x40A0 = 5.0F) * charge_level`, and the negative term is `-5.0F` instead of `-10.0F` (`lui at, 0xC0A0`).
- Must go in vanilla under `#ifdef REGION_REMIX`. Not ported here.

### 1e. Kirby copy of the shot velocity: `kirby_shoot_velocity_patch_` (0xD1E20 → 0x801573E0)
- Remix: `MewtwoSpecial.asm:1143-1180`. Kirby-only, relevant to Kirby's copy, not to this port's table.

## 2. Shared Remix subsystems

- `REMIX-TODO(GFX)` (two sites in the port, `ftRemixMewtwoShadowBallExplode` and `ftRemixMewtwoShadowBallProcHit`): Remix writes `GFX.current_gfx_id` before the effect calls. The port calls the vanilla effect functions (`efManagerSparkleWhiteMakeEffect` / `efManagerSparkleWhiteMultiExplodeMakeEffect` for 0x5C / 0x5D, and `efManagerDamageFireMakeEffect` for the dark cross 0x6C) without setting that global. The effects will look like the vanilla defaults, not the Remix ones.
  - Asm lines: 1278-1291 (explosion: 0x5C not full, 0x5D full), 1240-1243 (dark cross 0x6C).

## 3. Uncertainties (asm lines in `MewtwoSpecial.asm` unless noted)

1. **Shadow Ball slot interplay (highest risk).** The Mewtwo side reads WPStruct 0x29C (frame count, 1021-1046), 0x2A0 (begin_shot, 1000), 0x2A4 (charge level, 987, 1272) and writes 0x2AC/0x2B0. The asm has no writer for 0x2A4. The vanilla Samus spawn and release code write 0x2A0 (`charge_size`), 0x2A4 (`owner_gobj`) and 0x29C (`is_full_charge`). The port follows the Mewtwo side. Its charge-loop write of `charge_level` (see 6) is my interpretation. Verify in game: the ball may launch early, never launch, or read a garbage level.
2. **Kirby action ids 0x14A-0x14F.** Derived from `build/remixasm/remix.z64`. The `KIRBY_action_array` (0x804A1BA0) entries at ROM 0x2CA2438..0x2CA249C hold the six MewtwoNSP routines in order. Ground begin is entry 110 (`0xDC + 110 = 0x14A`), with a 0x14 stride. The Mewtwo array (0x804BE6F0) and the NMewtwo array (0x804DA1D0) give the same RAM-to-ROM delta, which I used to place the Kirby array. Verify against the Remix build (`Character.add_new_action` at `Character.asm:622-700`, `constant Action.{name}(...)`).
3. **Charge loop body (`charge_main_`, asm 443-469).** The 15 copied instructions of 0x8015D5AC are not readable from the source. The port uses the decomp semantics of `ftSamusSpecialNLoopProcUpdate`. The MAX branch uses gfx 0x58 (`ftParamCheckSetFighterColAnimID`), `ftSamusSpecialNDestroyChargeShot`, then `mpCommonSetFighterWaitOrFall` (0x800DEE54) as the asm names it.
4. **Charge loop non-max branch.** The decomp writes `charge_size` here. Mewtwo reads that slot as `begin_shot`, so the port writes `charge_level` (0x2A4) instead. This is an interpretation, see item 1.
5. **Shoot status flags.** `ground_shoot_initial_` (709-752), `air_shoot_initial_` (755-797), and the `ground_shoot_transition_` / `air_shoot_transition_` bodies (881-968) pass arg 5 from a register or stack slot the source does not set in the body (`sw t6` at 728, `sw t7` at 898/944, none at 773/759). The port uses `FTSTATUS_PRESERVE_COLANIM` (the decomp Samus value).
6. **Air shoot copy.** `air_shoot_initial_` copies 15 lines of 0x8015DAA8 (`ground→air` switch with clamp). The port mirrors that with `mpCommonSetFighterAir` and `ftPhysicsClampAirVelXMax` when `fp->ga == nMPKineticsGround`. Same for `air_shoot_transition_` (copy of 0x8015DA04, 8 lines).
7. **Ground charge interrupt (507-517).** The copied 14 lines of 0x8015D640 contain the B/A check and the escape/Z handling. The port mirrors the decomp Samus interrupt with B/A going to `ground_shoot_initial`. The asm return targets (0x8015D6F4 / 0x8015D68C) are not reproduced.
8. **Air charge interrupt (519-560)** is custom. A/B → `air_shoot_initial`. Shield (`input.button_mask_z`, 0x1B8) with `button_tap` → destroy + `ftCommonFallSetStatus`. The asm checks the shield mask against `buttons_pressed` (0x1BE), the port uses `button_tap`. Same field per the struct offsets.
9. **Sine function.** `0x800303F0` (asm 1031) is assumed to be `__sinf` (`decomp/src/libultra/gu/sinf.c`, `decomp/src/sys/utils.h`). Not in `vanilla_funcs.txt`. Verify the address.
10. **Dead loads.** `ground_begin_transition_` line 295 (`lw t8, 0x08E8(s0)`) is overwritten at 298 and has no effect. Not ported.
11. **Stray line.** `MewtwoSpecial.asm:502` is a bare `80157114` with no instruction. Ignored.
12. **Bobbing.** Uses the decomp `DObj.translate.vec.f.y` (asm offset 0x20 of bone 0) and `scale.vec.f.x/y` (0x40/0x44). Verified against Samus's `DObjGetStruct` uses, not against a DObj layout dump.
13. **Parent.** The brief names YOSHI as the parent. The NSP code is Samus-derived (Samus Charge Shot, `ftsamusspecialn.c`), so the port includes the Samus headers, not the Yoshi ones.
14. **Entry set.** The worklist lists 13 `MewtwoNSP.*` routines. All are in the table, including `ground_begin_initial_` and `air_begin_initial_`, which are special entries (`ftRemixMewtwoSpecialNGroundBeginInitial`, `ftRemixMewtwoSpecialAirNBeginInitial`).
