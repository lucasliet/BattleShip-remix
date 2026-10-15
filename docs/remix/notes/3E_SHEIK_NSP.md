# 3E SHEIK NSP (Needles) port notes

Port file: `decomp/src/ft/ftremix/sheik/ftremixsheiknsp.c`
Table: `dFTRemixSheikNSPProcs` / `dFTRemixSheikNSPProcsNum` (14 entries, the full worklist scope).
Source: `.toolchain/pkgs/smashremix-src/src/Sheik/SheikSpecial.asm` lines 732-1949 (SheikNSP scope).
Syntax check: `syntax-check OK`, no warnings.

## 1. Vanilla patches and hooks

None. SheikSpecial.asm has no `OS.patch_start` in the SheikNSP scope. The action
table and on-hit wiring live in Sheik.asm (lines 433-437, 463, 489-492), which
the bind handles. Vanilla addresses in these routines (0x800E6F24, 0x800DEE98,
0x8015D338 and similar) are plain calls or handler pointers, mapped to decomp
functions below.

Vanilla handler mapping used by the port:

| Remix address | Decomp function | Port use |
|---|---|---|
| 0x800E6F24 | `ftMainSetStatus` | status changes |
| 0x800E0830 | `ftMainPlayAnimEventsAll` | `begin_initial_` |
| 0x8015D338 | `ftSamusSpecialNProcDamage` | `proc_damage` (Sheik) |
| 0x80156E98 | `ftKirbyCopySamusSpecialNProcDamage` | `proc_damage` (Kirby, JKirby) |
| 0x8015DB4C | `ftSamusSpecialNStartInitStatusVars` | `begin_initial_` (Sheik) |
| 0x801576B4 | `ftKirbyCopySamusSpecialNInitStatusVars` | `begin_initial_` (Kirby) |
| 0x800DEE98 / 0x800DEEC8 | `mpCommonSetFighterGround` / `mpCommonSetFighterAir` | transitions |
| 0x800D8EB8 | `ftPhysicsClampAirVelXMax` | aerial transitions |
| 0x800DDE84 / 0x800DE6E4 | `mpCommonProcFighterOnEdge` / `mpCommonProcFighterLanding` | collisions |
| 0x8013E1C8 / 0x8013F9E0 | `ftCommonWaitSetStatus` / `ftCommonFallSetStatus` | cancel |
| 0x801492F8 / 0x80149294 | `ftCommonEscapeGetStatus` / `ftCommonEscapeSetStatus` | escape |
| 0x800DEE54 | `mpCommonSetFighterWaitOrFall` | idle |
| 0x800E9814 | `ftParamCheckSetFighterColAnimID` | charge spark |
| 0x800EDF24 | `gmCollisionGetFighterPartsWorldPosition` | hand position |
| 0x801655C8 | `wpManagerMakeWeapon` | needle |
| 0x80167FA0 / 0x80167FE8 / 0x80168088 | `wpMainVelSetModelPitch` / `wpMainDecLifeCheckExpire` / `wpMainApplyGravityClampTVel` | needle |
| 0x800FF648 | `efManagerDustExpandSmallMakeEffect` | needle smoke |
| 0x80035CD0 / 0x800303F0 | `__cosf` / `__sinf` | needle velocity (comments in asm say cos, sin) |
| 0x800269C0 | `func_800269C0_275C0` | FGM 0x0410 |
| 0x80175914 / 0x8016DD2C / 0x801692C4 | `itLGunWeaponAmmoProcMap` / `wpBossBulletProcHop` / `wpSamusBombProcReflector` | WPDesc procs |

## 2. Shared subsystems

- **REMIX-TODO(GFXRoutine.SHEIK_CHARGE)**, in `ftRemixSheikNSPChargeProcUpdate`.
  `charge_main_` lines 1221-1225 write the charge level to `[GObj(FTStruct 0x0B20)->user_data + 0x2A4]`
  when the spark GFX exists. The GFX routine is not ported, so the write is skipped.
- Not in this worklist, not ported: `SheikNSP.kirby_ground_begin_collision_`,
  `kirby_ground_charge_collision_`, `kirby_ground_shoot_collision_` (Kirby action
  table, Kirby.asm lines 392-397). `air_shoot_collision_`, `air_shoot_interrupt_`,
  `ground_shoot_interrupt_` and `ground_shoot_transition_` are also not in the
  worklist. `ground_shoot_transition_` and `air_shoot_interrupt_` are referenced
  only from those, so no in-scope routine calls them.
- Remix Kirby action ids (0x166-0x16B) are only used as values here. The Kirby
  table entries are another worker's scope.

## 3. Constants and derived values

- Kirby action ids: `(Character.action_array_size.KIRBY 0x67C / 0x14) + index + 0xDC`
  with index = count of earlier `add_new_action(KIRBY, ...)` calls (Kirby.asm
  line 392 is index 55). Result: GroundBegin 0x166, GroundCharge 0x167, GroundShoot 0x168,
  AirBegin 0x169, AirCharge 0x16A, AirShoot 0x16B. Cross-checked against
  `remix.sym` strings: `Action.KIRBY.string_0x166` reads "NeedleStormStartGround",
  `0x16A` reads "NeedleStormChargeAir".
- Needle projectile kind 0x23: `build/remixasm/bass.log` line 5676 ("Added Projectile: SHEIK_NEEDLE - ID: 0x23").
- Charge GFX routine 0x6D: `bass.log` line 5946 ("ID is 0x6D").
- FTSTATUS flags: 0x2 = `FTSTATUS_PRESERVE_COLANIM`; 0x802 = COLANIM | LOOPSFX (0x800).
- Weapon flags 0x80000000 = `WEAPON_FLAG_COLLPROJECT`.
- Float constants: 225 and 250 are exact. -0.785398F and 0.785398F match bits 0xBF490FD8 and 0x3F490FD8.
- `fp->joints[15]` is the Remix 0x0924 hand DObj (0x924 - 0x8E8 = 0x3C, index 15). The asm comment says "part 0xC".
- WPDesc layout matches the Remix descriptor: flags, kind, p_weapon, o_attributes, transform_types {0x12,0x47,0x00} (word 0x12470000), then the procs at 0x14..0x30.
- `p_weapon` is set per spawn to `fp->data->p_file_special1` (Character.SHEIK_file_6_ptr).

## 4. Uncertainties and deviations

1. **Escape itemthrow tics (asm 1264-1272).** `ftCommonEscapeSetStatus` gets a2 = the
   fighter GObj pointer as its third argument. The port passes 0. A pointer has no tic
   meaning on LP64, but this changes the escape item-throw buffer compared with Remix.
   Decide whether to keep 0 or mirror the Remix value.
2. **Aerial needle rotation (asm 1783-1784).** The branch tests v0, which holds whatever
   `__sinf` left there. The port uses `fp->ga == nMPKineticsAir`, the intended "aerial" case.
   The palette store (`mobj->palette_id`) follows the same branch, so the grounded case
   stores `(f32)lr` (asm 1770, f10 from cvt.s.w). The aerial case stores 0.0.
3. **Dead store dropped (asm 1787-1790).** The aerial branch stores a float from RAM
   0x8019CA84 into DObj rotate.y. `wpMainVelSetModelPitch` (called at 1798) overwrites
   rotate.y right after, so the store is dead. The ROM bytes at that address are not a
   meaningful constant (0x25CEB720), so it is not ported.
4. **Needle lifetime offset (asm 1745).** Remix writes the duration at WPStruct 0x268.
   The decomp `WPStruct.lifetime` is at 0x264 in `struct_offsets.txt`, which is a
   clang layout. The port uses `wp->lifetime`, which is what `wpMainDecLifeCheckExpire`
   reads. Verify against the IDO layout.
5. **Flag0 clear on the weapon (asm 1631-1634).** When the needle path runs and the
   animation has also ended, `sw r0, 0x17C(v0)` uses v0 = the returned weapon GObj, not
   the fighter, so Remix clears 0x17C on the needle GObj. The port clears the fighter's
   flag0 instead. The fighter flag0 was already cleared at line 1600, so the port only
   drops a write to the needle GObj.
6. **Unused copy paths.** `ground_begin_transition_` loads 0x8E8 (asm 1093) and then
   overwrites t8; that load is dead and not ported. `ground_charge_interrupt_` line
   1254 (`sw a1,0x1C(sp)`) is unreachable and not ported. The charge-continue store (1221-1225) is the REMIX-TODO above.
7. **Props fields dropped.** Min speed (0x08), bounce (0x10), rotation speed (0x14)
   and the projectile data pointer (0x24, only loaded into t0 and never used) are not used
   in the Needles code, so they are not in the port struct.
8. **Copied Samus code.** The port reimplements the copied fragments as C:
   - `ground_charge_interrupt_` (0xD8080, 7 instructions): the B/A/shield reads.
   - `ground_shoot_initial_` (0xD84A0, 5): flag/status setup with arg4 = 2.
   - `air_shoot_initial_` (0xD84E8, 15): grounded-to-air step, then the status setup.
   - `ground_shoot_transition_` (0xD83F0) and `air_shoot_transition_` (0xD8444): the
     air transition copy calls `mpCommonSetFighterAir` only, no clamp.
   Fragments were disassembled with `tools/remix_disasm_rom.py` and checked against the
   instruction stream.
9. **Kirby branch confirmation.** Each `beql`/`lli` pair that picks the Kirby action
   skips the Sheik default (for example asm 744-751). The port uses a single
   `ftRemixSheikNSPIsKirby` predicate: `fkind == nFTKindKirby || fkind == REMIX_ID_JKIRBY`.
10. **`begin_main_` ground test.** Remix branches on `ga == 0` (grounded) and `needle`
    on `ga == 1` (aerial). The port uses `nMPKineticsGround` and `nMPKineticsAir`, which
    match those values.
