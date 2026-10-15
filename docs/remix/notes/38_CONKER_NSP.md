# 0x38 CONKER: ConkerNSP port notes

Source: `.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm`, scope `ConkerNSP` (lines 289-988).
Output: `decomp/src/ft/ftremix/conker/ftremixconkernsp.c`, table `dFTRemixConkerNSPProcs` / `dFTRemixConkerNSPProcsNum`.
`ftremixconker.c` and `ftremixported.c` are untouched. The syntax check passes with no warnings.

## Coverage

- Worklist routines ported: 10 of 10 (`ground_begin_initial_`, `air_begin_initial_`, `ground_begin_main_`,
  `air_begin_main_`, `ground_wait_main_`, `air_wait_main_`, `end_main_`, `ground_collision_`, `air_collision_`,
  `air_collision_end_`).
- Static helpers ported (not in the table): `ground_wait_initial_`, `air_wait_initial_`, `ground_end_initial_`,
  `air_end_initial_`, `ground_to_air_`, `air_to_ground_`, `nut_stage_setting_`, `nut_main_`, `nut_destruction_`.
- Nut weapon descriptor (`nut_projectile_struct`) and properties (`nut_properties_struct`) are ported inside the file.
  The descriptor is built at run time because `p_weapon` points at the fighter's special file.

## Registration (integration task, not done here)

- `decomp/src/ft/ftremix/ftremixported.c` must add `extern const FTRemixProc dFTRemixConkerNSPProcs[];`,
  `extern const s32 dFTRemixConkerNSPProcsNum;` and a `{ dFTRemixConkerNSPProcs, &dFTRemixConkerNSPProcsNum }`
  entry in `dFTRemixPortedProcTables`. Until then, every ConkerNSP routine falls back to the parent (FOX).
- `ftremixconker.c` still exports the placeholder `dFTRemixConkerProcs` (empty), which the owner of `ftremixported.c`
  may also drop from the table list.

## Vanilla patches

- None needed. The port calls only vanilla or libultra functions:
  `ftMainSetStatus` (0x800E6F24), `ftMainPlayAnimEventsAll` (0x800E0830), `ftAnimEndCheckSetStatus` (0x800D9480),
  `mpCommonProcFighterOnEdge` (0x800DDE84), `mpCommonProcFighterLanding` (0x800DE6E4),
  `mpCommonProcFighterCliffWaitOrLanding` (0x800DE978), `mpCommonSetFighterWaitOrFall` (0x800DEE54),
  `mpCommonSetFighterAir` (0x800DEEC8), `mpCommonSetFighterGround` (0x800DEE98), `ftPhysicsClampAirVelXMax` (0x800D8EB8),
  `gmCollisionGetFighterPartsWorldPosition` (0x800EDF24), `wpManagerMakeWeapon` (0x801655C8),
  `wpMainDecLifeCheckExpire` (0x80167FE8), `wpMainApplyGravityClampTVel` (0x80168088),
  `wpMainVelSetModelPitch` (0x80167FA0), `efManagerDustExpandSmallMakeEffect` (0x800FF648),
  `func_800269C0_275C0` (the FGM player behind `FGM.play_`), `__cosf` (0x80035CD0), `__sinf` (0x800303F0).
- Weapon procs that the nut descriptor references from other fighters, declared `extern` in the file:
  `itLGunWeaponAmmoProcMap` (0x80175914, `decomp/src/it/itcommon/itlgun.c:302`),
  `wpMarioFireballProcHop` (0x801686F8, `decomp/src/wp/wpmario/wpmariofireball.c:150`),
  `wpSamusBombProcReflector` (0x801692C4, `decomp/src/wp/wpsamus/wpsamusbomb.c:197`). None of them is static.
- Projectile id: `Projectile.id.CONKER_NUT` = 0x24 (`Projectile.asm`, `bass.log` line 5677). The vanilla weapon
  system needs no change beyond the Remix registration that already exists.

## Status ids

- Conker: `NSP_Ground_Begin` 0xF6, `NSP_Ground_Wait` 0xF7, `NSP_Ground_End` 0xF8, `NSP_Air_Begin` 0xF9,
  `NSP_Air_Wait` 0xFA, `NSP_Air_End` 0xFB. Derived from `Character.asm` line 683: parent FOX size 0x208 / 0x14 = 26,
  plus the new-action index (0 to 5, the order of `Conker.asm` lines 269-274), plus 0xDC.
  Consistency check: CONKER has 26 + 6 = 32 entries (`port/remix/remix_action_layout.h`), and the +3 air/ground
  offset used by `ground_to_air_` maps 0xF6 to 0xF9.
- Kirby: `CONKER_NSP_Ground_Begin` 0x144 through `CONKER_NSP_Air_End` 0x149. Derived from `Character.asm` line 681:
  KIRBY size 0x67C / 0x14 = 83, plus the index among `add_new_action(KIRBY, ...)` calls before `Kirby.asm` line 358
  (21), plus 0xDC. The +3 offset again maps Ground_Begin to Air_Begin.
- Conker.asm `scope Action` names `Catapult*` at 0xF6 to 0xFB (lines 358-363) collide with these numbers by value.
  Those names are not used by any ConkerNSP or ConkerUSP code, so they are assumed stale.

## Translation choices

- Kirby and J Kirby branches (`ground_begin_initial_` and the other `*_initial_` routines): `fkind` equal to
  `nFTKindKirby` or `REMIX_ID_JKIRBY` picks the Kirby status id, as the `beql` pairs in asm 299-302 do.
- `FTSTATUS_PRESERVE_LOOPSFX` (`1 << 11` = 0x800) for the wait, ground-to-air and air-to-ground calls; this is the
  `ftMainSetStatus` argument 0x800 with the comment "continues FGM called with the 3C command".
- The button mask uses `B_BUTTON` (0x4000), not the `Joypad.B` value 0x0020 that the asm comments name, as in the
  DSP port (see `38_CONKER_DSP.md`, uncertainty 3).
- The `ga` branch in `nut_stage_setting_` (asm 746-841) picks ground or air initial angle. Both values are
  0.174533 in `nut_properties_struct`, so the port uses one constant.
- Power level (temp variable 3, `motion_vars.flags.flag2`): max power is `power >= 4` (asm `sltu` with MAX_POWER - 1).
  FGM id: 0x104 at power 0, 0x103 for 1 to 3, 0x102 at max; the on-hit FGM is 0x1F at max (asm 821).
- `end_main_` zeroes temp variable 1 whether or not it was set (the `sw r0` sits in the `beq` delay slot), and
  `air_collision_end_` uses the anim frame compared with 6.0 as a float (`c.lt.s`, asm 667-676).
- The nut spawn position starts at zero (asm 604 and the zeroing before it), then gets joint 17's world position
  (N64 0x092C = `FTStruct.joints[17]`, `joints` at 0x08E8), y + 60.0, z = 0.
- `wpManagerMakeWeapon` gets `WEAPON_FLAG_COLLPROJECT` (0x80000000) and parent type fighter (0).

## Uncertainties

1. Duration (asm 770): `sw t3, 0x0268(v1)` stores the 30-frame duration at N64 WPStruct 0x268. The decomp names
   0x268 as the `is_camera_follow` and `is_static_damage` bits, and names `lifetime` at 0x264, which
   `wpMainDecLifeCheckExpire` decrements. This port writes nothing: `wpManagerMakeWeapon` already clears those two
   bits, and `lifetime` is left to whatever the pool holds. If `lifetime` stays 0 or garbage, the nut may never expire
   (the decrement test is `== 0`). Verify on the N64 or in a trace before trusting nut duration.
2. Power level (asm 783, and the 0x01B4 reads in `nut_main_`): stored at `attack_coll.attack_pos[1].pos_curr.x` (N64 0x1B4 =
   0x154 + 0x60). This is the second hitbox slot, which the port assumes the nut never writes (its attack count
   is 1). Check that the hitbox code leaves `attack_pos[1]` alone for this weapon.
3. Kirby and Conker status ids (0x144 to 0x149, 0xF6 to 0xFB) come from assembler-macro arithmetic. They are not
   checked against the ROM's action array. A wrong Kirby id would only affect Kirby and J Kirby.
4. `0x80035CD0` = `__cosf` and `0x800303F0` = `__sinf` come from the Remix comments in `SpecialZoom.asm` and
   `AirDodge.asm`. They are not in `vanilla_funcs.txt`; the address-to-name link is an inference.
5. `FTStruct.joints[17]` for "part 0xD (weapon) struct" (asm 604) is the index from the offset, not from a named
   constant. Confirm it is the hand joint.
6. `nut_stage_setting_` writes palette 0.0 through `DObj.mobj` (end of `nut_stage_setting_`) without a NULL check, as the asm does.
   The port does the same.
