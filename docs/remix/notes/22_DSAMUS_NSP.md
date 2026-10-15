# 22_DSAMUS NSP port notes

Source: `.toolchain/pkgs/smashremix-src/src/DSamus/DsamusSpecial.asm`, scope `DSamusNSP`.
Port: `decomp/src/ft/ftremix/dsamus/ftremixdsamusnsp.c`, table `dFTRemixDSamusNSPProcs` (4 entries).

## Routines ported (4 of 4 in worklist scope)

| Remix label | Asm lines | Table entry |
|---|---|---|
| `air_charge_main` | 220-265 | `ftRemixDSamusNSPAirChargeMain` |
| `air_charge_interrupt_` | 318-367 | `ftRemixDSamusNSPAirChargeInterrupt` |
| `ground_charge_collision_` | 371-383 | `ftRemixDSamusNSPGroundChargeCollision` |
| `air_charge_collision_` | 456-469 | `ftRemixDSamusNSPAirChargeCollision` |

Static helpers (not in table, reached through function pointers): `ftRemixDSamusNSPAirChargeTransition` (`air_charge_transition_`, asm 403-451) and `ftRemixDSamusNSPGroundChargeTransition` (`ground_charge_transition_`, asm 491-537). Both are needed because the collisions pass them to vanilla `mpCommon*` procs.

## Vanilla patches

None in this scope. The `OS.patch_start` blocks in DsamusSpecial.asm belong to other scopes (`initial_variable_override`, `remove_shoot_flag_aerial_transition`, `check_aerial_charge`, `charge_level_patch_`, and so on), not to `DSamusNSP.*` routines in the worklist. The ported routines are only entered through the action table and the Remix special tables.

The Kirby hook `DSamusNSP.air_charge_kirby_main` and `kirby_ground_charge_collision_` are outside this worklist (they are Kirby routines) and are not ported here.

## Vanilla functions called

- `0x8015D394` `ftSamusSpecialNSetChargeShotPosition(FTStruct*)`
- `0x8015D300` `ftSamusSpecialNDestroyChargeShot(FTStruct*)`
- `0x8015D338` `ftSamusSpecialNProcDamage(GObj*)` (stored to `proc_damage`)
- `0x8015DAA8` `ftSamusSpecialAirNEndSetStatus(GObj*)`
- `0x80157610` `ftKirbyCopySamusSpecialAirNEndSetStatus(GObj*)`
- `0x80156E98` `ftKirbyCopySamusSpecialNProcDamage(GObj*)` (stored to `proc_damage`, Kirby/J Kirby)
- `0x800DDE84` `mpCommonProcFighterOnEdge(GObj*, proc)`
- `0x800DE6E4` `mpCommonProcFighterLanding(GObj*, proc)`
- `0x800DEE98` `mpCommonSetFighterGround(FTStruct*)`
- `0x800DEEC8` `mpCommonSetFighterAir(FTStruct*)`
- `0x800D8EB8` `ftPhysicsClampAirVelXMax(FTStruct*)`
- `0x800E6F24` `ftMainSetStatus`
- `0x800E9814` `ftParamCheckSetFighterColAnimID(GObj*, 6, 0)`
- `0x800DEE54` `mpCommonSetFighterWaitOrFall(GObj*)`
- `0x8013F9E0` `ftCommonFallSetStatus(GObj*)`

## Shared subsystems

No custom items are spawned by these routines, so there are no `REMIX-TODO` markers. The charge ball is the vanilla Samus charge shot (`wpSamusChargeShot`), and its weapon struct is written only through the `charge_size` field.

## Uncertainties

1. **Weapon field at 0x2A4.** `build/remixasm/refs/struct_offsets.txt` maps N64 0x2A4 to `weapon_vars.charge_shot.owner_gobj`. The port layout puts the charge size at `charge_size` (N64 0x2A0 by that table), and marina's DSP port uses `charge_size` for the same field. The port writes `charge_size`, following marina. Asm lines 198-202 (`sw v0, 0x02A4(v1)`).
2. **Timer and ball slots.** The asm uses 0x0B1C as the tick timer and 0x0B20 as the attached ball gobj. The port names them `specialn.charge_int` and `specialn.charge_gobj`, the Samus charge shot fields at those same slots. Asm lines 224-230 and 252.
3. **Ammo slot.** The asm uses 0x0ADC as the ammo count. The port uses `passive_vars.samus.charge_level` (same slot). Asm lines 228-236.
4. **Momentum call.** Asm line 411 is annotated `momentum capture?`. The port calls `ftPhysicsClampAirVelXMax`, the decomp name of `0x800D8EB8`. Treat that name as a best match.
5. **Empty-ammo idle.** Asm line 246 calls `0x800DEE54` (`mpCommonSetFighterWaitOrFall`), but the comment there says "set aerial idle", and asm lines 244-245 mention `0x8013E1C8` as an alternative that is commented out. The port uses the called function.
6. **Kirby action ids.** Kirby ids come from `Action.asm` scope `KIRBY` (`DSCharging = 0x133`, `DSamusNSPChargeAir = 0x1AD`), not from a ROM read. They are only used when Kirby or J Kirby copies Dark Samus.
7. **Button checks.** `A_BUTTON | B_BUTTON` on `input.pl.button_tap` (asm 0x1BE). The shield mask is `input.button_mask_z` (asm 0x1B8). The asm tests the shield bitmask against the same tapped word, and the port does the same.
8. **Transition flags.** `0x0802` is `FTSTATUS_PRESERVE_COLANIM | FTSTATUS_PRESERVE_LOOPSFX` (`mewtwo` used the raw literal).
9. **Transition helper names.** The asm labels `air_charge_transition_` and `ground_charge_transition_` are not in the worklist. They are ported as static helpers, not table entries, per the brief.
