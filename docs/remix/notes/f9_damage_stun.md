# F9 notes: Damage.asm stun, deku stun and flashbang actions (Worker S)

Source: `.toolchain/pkgs/smashremix-src/src/Damage.asm`. Port: `decomp/src/ft/ftremix/ftremixdamagestun.c` (new).
Declarations: block appended to `decomp/src/ft/ftremix/ftremixhooks.h`. Hooks: `decomp/src/ft/ftcommon/ftcommondamage.c`.
Bury side (`ftremixbury.c`, `ftremixbury.h`) is Worker B's; `ftremixdamagestun.c` includes `ft/ftremix/ftremixbury.h`.

## Ported asm routines

| Asm (Damage.asm) | RAM | Vanilla function (vanilla_funcs.txt) | Status | Notes |
|---|---|---|---|---|
| `stun_damage_action_` (line 198), STUN branch | 0x8014158C | `ftCommonDamageGotoDamageStatus` (ftcommondamage.c), +0x2C | applied | Hook before the sleep check, `#ifdef REGION_REMIX`. `ftRemixDamageGotoStun` |
| `stun_damage_action_`, DEKU_STUN branch | 0x8014158C | same | applied | `ftRemixDamageDekuStunSetStatus` |
| `stun_damage_action_`, FLASHBANG branch | 0x8014158C | same | applied | `ftRemixDamageFlashbangSetStatus` |
| `stun_damage_action_`, BURY branch | 0x8014158C | same | applied | Calls `ftRemixDamageBuryOrPlungeInitial(gobj, !buried)` (Worker B) |
| `stun_initial_modified_` (line 302) | none | `ftCommonFuraFuraSetStatus` (ftcommonfurafura.c, 0x801498A4) | applied as a copy | `ftRemixDamageStunSetStatus`, see deviation 3 |
| `deku_stun_initial_modified_` (line 336) | none | `ftCommonFuraFuraSetStatus` (0x801498A4) | applied as a copy | `ftRemixDamageDekuStunSetStatus` |
| `flashbang_initial_modified_` (line 418 area, 0x3F80 anim speed) | none | `ftCommonFuraFuraSetStatus` (0x801498A4) | applied as a copy | `ftRemixDamageFlashbangSetStatus` |
| `stun_main_aerial_` (line 539) | none | `ftCommonFuraFuraProcUpdate` (0x80149810) | applied | `ftRemixDamageStunMainAerial`, uses the aerial fall `ftCommonFallSetStatus` (0x8013F9E0) |
| `stun_electric_action_` (line 911) | 0x801412E8 | `ftCommonDamageInitDamageVars` (ftcommondamage.c), electric status check | applied, location inferred | `ftRemixDamageUsesElectricStatus` replaces the element test at ftcommondamage.c:615, see deviation 6 |
| Constants `FLASHBANG_*`, `initial_buried_time` etc. | none | none | applied where used | Flashbang times in the new file, bury constants in Worker B's file |

Helpers used from vanilla: `ftMainSetStatus` (0x800E6F24), `ftCommonCaptureTrappedInitBreakoutVars` (0x8014E3EC),
`ftCommonCaptureTrappedUpdateBreakoutVars` (0x8014E400), `ftParamCheckSetFighterColAnimID` (0x800E9814),
`ftParamCheckSetSkeletonColAnimID`, `ftRemixFuraFuraBreakoutWait` (ftremixjstunsleep.c).

Verified at compile time: `nFTCommonStatusFuraFura == 0xA4` (the asm `Action.Stun`). This is a `_Static_assert`
in a scratch copy only, not in the versioned file.

## Not ported here

- `bury_or_plunge_initial_`, `bury_initial_`, `bury_player_`, `plunge_*`, `buried_main_`, `prevent_wind_push_while_buried`:
  Worker B (`ftremixbury.c`).
- Nothing else in the brief's list is left out.

## Deviations and uncertainties

1. Stun initial: the asm jumps into the vanilla `ftCommonFuraFuraSetStatus` after replacing the flags argument of
   `ftMainSetStatus` with 0 (`FTSTATUS_PRESERVE_NONE`). The port copies the body (8 lines) with flags NONE instead of
   changing the vanilla function. The Japanese stun toggle (`ftRemixFuraFuraBreakoutWait`) is applied, as in the vanilla body.
2. Deku and flashbang do not apply the Japanese toggle, because the asm does not call the vanilla body for them.
3. Breakout waits: stun = 400 - percent (not below 0) + `FTCOMMON_FURAFURA_BREAKOUT_WAIT_MIN`. Deku = same, plus 0x5A.
   Flashbang = grounded 150 or aerial 0, plus percent, plus 90 (no clamp). These are literal from the asm.
4. Deku and flashbang on a fighter already in the stun action: the asm skips the action change and does not fall back to
   the vanilla path, so the hit does nothing. The port does the same (returns TRUE from the hook).
5. Stun on a fighter already in the stun action, or hit from the back, or in the air: the asm falls back to the vanilla
   branch. The port returns FALSE and the vanilla `ftCommonDamageInitDamageVars` runs with STUN as element.
6. Electric status: the asm `stun_electric_action_` takes the electric branch only for ELECTRIC and FLASHBANG. For STUN,
   DEKU_STUN and BURY it takes the vanilla branch. The header comment of the asm says "acts like electric damage" for STUN,
   so the code does not match its own comment. The port follows the code, not the comment. Lead: check this in a game test if
   a STUN hit on a stunned fighter should look electric. The electric branch location (ftcommondamage.c:615) is inferred
   from the electric status check, the asm offset +0x404 was not confirmed by disassembly.
7. Assumed offsets (asm to C field): 0x14C = `fp->ga` (aerial is `nMPKineticsAir`), 0x9D4 = `fp->proc_update`,
   0x26C = `fp->breakout_wait`, 0x34 = `fp->shield_health`, 0x2C = `fp->percent_damage`, 0x48/0x4C =
   `fp->physics.vel_air.x/y`, 0x7FC = `fp->damage_lr`, 0x44 = `fp->lr`, 0x24 = `fp->status_id`.
8. The stun_main_aerial end test uses the breakout wait after the mash and before the x3 scaling (asm `bgtz v0`), not the
   scaled value that ProcUpdate tests.
9. The hook in `ftCommonDamageGotoDamageStatus` sits after the cliff catch wait and before the sleep check. The asm patch
   point is the sleep check, so this order is the same. Types 7..0xC never equal sleep.
10. Bury comparison (asm line 211, `li a1, buried_main_` compared to `fp->proc_update`): the port passes
    `!ftRemixDamageIsBuried(fp)` as `is_start`. `ftRemixDamageIsBuried` is a required symbol from Worker B
    (ftremixbury.h). It must compare `fp->proc_update` with the buried main routine. Worker B must add it to
    `ftremixbury.h`, or the build fails. It is not in the brief, so it is flagged here.

## Lead: build and test

- `ftremixbury.h` did not exist when this file was written. The syntax check of `ftremixdamagestun.c` passed against a
  scratch stub header with `ftRemixDamageBuryOrPlungeInitial` and `ftRemixDamageIsBuried`. The versioned file still
  needs Worker B's header.
- `ftcommondamage.c`: `scripts/remix/syntax-check.sh` reports `syntax-check OK` (one warning, already in the file).
- Flashbang item id: `Item.asm` `add_item` order gives 0x4A (base 0x2D, standard 13 items, stage 7, Psyduck 1, character
  items Gem..Flashbang at index 9 of the character list). The asm prints `Added Item 0x4A: Flashbang` at build time: check
  the build log. Use `SSB64_REMIX_FORCE_ITEM=Flashbang` or the id 0x4A per the item-kind variable in the build.
- Tests to run: STUN hit on a grounded fighter facing the hit (expect the Fura Fura stun), the same hit from the back
  (expect the normal damage), a DEKU_STUN hit (mash out of the stun), a FLASHBANG hit in the air (expect the aerial
  ShieldBreakFall and the mash timer).
