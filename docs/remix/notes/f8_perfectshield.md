# F8 toggle: Perfect Shielding (PerfectShield/PerfectShield.asm)

Toggle: title `Perfect Shielding`, entry `entry_perfect_shield` (Toggles.asm line 2435), values 0 (off, default in every
profile) and 1 (on). Read with `gmRemixToggle(perfect_shield)`: the asm uses `Toggles.read`, not `single_player_guard`,
so the toggle also applies in Bonus and 1P Game. Test: `SSB64_REMIX_TOGGLES="Perfect Shielding=1"`.

Code: `decomp/src/ft/ftremix/ftremixperfectshield.c` (new), declarations appended to `ftremixhooks.h`.

## Patches

| asm address | vanilla function (vanilla_funcs.txt) | routine | status |
|---|---|---|---|
| 0x800E64D8 (ROM 0x61CD8) | ftMainProcParams (ftmain.c) | shield_stun_action_change_skip | applied: the `else` of `shield_damage != 0` calls `ftRemixPerfectShieldHandlesGuardSetOff` first; when the fighter is in `nFTCommonStatusGuardOn` (Action.ShieldOn 0x98) it zeroes `hitlag_tics` (asm 0x40, labelled "hitstun"), `shield_damage_total`, `shield_damage` (so the local `damage` becomes 0), then runs fighter_gfx and skips ftCommonGuardSetOffSetStatus. The shield-break branch is untouched. |
| 0x800E64D8 fighter_gfx | (routine of the same file) | fighter_gfx | applied: `gmCollisionGetFighterPartsWorldPosition(joints[nFTPartsJointYRotN])` (asm 0x8F4 = joints[3]) then `efManagerCatchSwirlMakeEffect`. Not reproduced: the asm's `lw s1, 0x84(...)` before the swirl call (a "scaling" hint); `efManagerCatchSwirlMakeEffect` takes only the position, so the swirl is unscaled. |
| 0x80166C7C (ROM 0xE16BC) | wpProcessProcHitCollisions (wpprocess.c) | shield_reflect_projectiles | applied: in the shield branch, after the hop block and before `proc_shield`, `ftRemixPerfectShieldWeapon`; on success it runs `proc_reflector` (asm 0x290) and does `goto next_check`, as the asm jumps to the reflect tail then the setoff block. Projectile: victim records cleared, `owner_gobj` = shielder, `attack_coll.stale` = min(0.5 * stale, 0.5F) (REFLECT_MULTIPLIER 0x3F00 = 0.5F). |
| 0x801711BC (ROM 0xEBBFC) | itProcessProcHitCollisions (itprocess.c) | shield_reflect_items | applied, same placement and tail with `proc_reflector` (asm 0x390). Owner changes unless `kind == nITKindMBall` (Hazards POKEBALL 0x13, checked against the enum). |
| all | - | reflect_hitbox_struct | applied: `sFTRemixPerfectShieldColl` (kind = custom id 0 << 16 | CUSTOM 3, joint 4, offset (0, 60, 0), size 350 x 3, damage_resist 0x18000000) is written to the shielder's `special_coll`, as the asm does. Custom id 0 is `Reflect.custom_reflect_table[0]` (Franklin Badge routine), so the existing ftRemixReflect path runs it. |

## Offsets and assumptions

- WPStruct/ITStruct offsets come from the asm, cross-checked against `tools/remix_disasm_rom.py` on baserom: WP `attack_coll.stale`
  (0x108), `attack_records` (0x214, stride 8), `proc_shield` 0x284, `proc_hop` 0x288, `proc_setoff` 0x28C, `proc_reflector` 0x290.
  IT: `attack_coll.stale` 0x118, `attack_records` 0x224, `proc_shield` 0x384, `proc_reflector` 0x390. The C names are used, so the
  struct_offsets.txt rows (4 bytes low from 0x25C) are not needed.
- Hop path: the asm hook sits on the hop branch's false edge, so the perfect check only runs when the projectile would not hop.
- ShieldOn = `nFTCommonStatusGuardOn` is assumed (Action.asm `ShieldOn(0x098)`, first guard status in ftdef.h). Worth checking if the toggle does nothing.

## What the lead should see with the toggle on

- Short match, a shield raised on a hit frame (window: `status_total_tics < 2`): no shield stun, the fighter stays on its shield,
  the yellow catch swirl appears at the shield joint.
- Projectile (e.g. Fox laser or Mario fireball) that hits the shield in that window is reflected back, now owned by the shielder,
  and its reflect damage multiplier is halved.
- Off: vanilla behaviour, nothing changes (every entry point returns FALSE first).

## Not done

- No build or game run (per brief). Syntax check: `scripts/remix/syntax-check.sh` OK on ftremixperfectshield.c, ftmain.c,
  wpprocess.c, itprocess.c (existing warnings in ftmain.c and itprocess.c are not from these edits).
- No REMIX-TODO markers: no AI, Size, GFX or PokemonAnnouncer patch in this scope.
