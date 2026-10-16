# F7 notes: Damage.asm and Poison.asm vanilla patches

Source: `.toolchain/pkgs/smashremix-src/src/Damage.asm`, `Poison.asm`. Helpers: `decomp/src/ft/ftremix/ftremixpoison.c`,
`decomp/src/ft/ftremix/ftremixdamage.c`. Declarations: appended block in `decomp/src/ft/ftremix/ftremixhooks.h`.

## Poison.asm

| Asm address | Patch | Vanilla function (vanilla_funcs.txt) | Status |
|---|---|---|---|
| 0x800E1808 `star_fix` | branch to the heal check | ftMainProcUpdateInterrupt (ftmain.c) | Applied implicitly: both paths of the vanilla `star_invincible_tics == ITSTAR_WARN_BEGIN_FRAME` branch reach the heal block, so the hook sits at its `else` |
| 0x800E1820 `poison_hook` | poison when `damage_heal == 0` | ftMainProcUpdateInterrupt (ftmain.c, heal block) | Applied: `#ifdef REGION_REMIX ... else ftRemixPoisonUpdate(fighter_gobj)` after the `damage_heal` block |
| 0x800E1850 (poison tail) | stock sync and colanim 9 reset | same | Applied inside `ftRemixPoisonUpdate` (`ftRemixPoisonSyncStock`) |
| css/PoisonDmg.asm `apply_damage` | CSS "Poison Damage" option | none (CSS debug menu, no port) | Skipped, toggle off: `FTREMIX_POISON_CSS_DAMAGE_DEFAULT FALSE`. Remix default state_table is 0 for all ports, so the asm returns at once |
| `Poison.clear_poison_` | durations cleared between matches | none | Ported as `ftRemixPoisonClear`, not wired. Item.asm:1577 calls it |

Spawned Fear consumer (`decomp/src/gr/grremix/grremixspawnedfear.c`, Hazards.asm acid, lines 8515-8580):
- `ftRemixPoisonSet(port, 60, 3, 0)` each frame while a grounded fighter stands on the acid below 666%.
- The timer is the halfword at 0x801313F8 (`sh t1, 0(at)`, once per frame in the acid routine). It moved to
  `ftRemixPoisonAdvanceTimer` and the acid's `acid_timer` field was removed.
- Colour animation: the poison tick sets `gmRemixColScriptsSetOverride(port, 0x79)` and
  `ftParamCheckSetFighterColAnimID(0x79, 0)` when no override is active (asm `_continue`).

## Damage.asm

| Asm address | Patch | Vanilla function | Status |
|---|---|---|---|
| 0x800E3FC4 `extend_on_hit_gfx_` | GFX on hit by damage type | ftMainProcessHitCollisionStatsMain, fighter case (ftmain.c switch on `ft_attack_coll->element`) | Applied to the fighter hit switch only: SHADOW (7) uses `ftRemixDamageMakeShadowEffect` (GFX 0x6C, then the fire effect); STUN (8) and FLASHBANG (0xC) use the electric effect. LASER, DEKU, BURY keep the normal branch. Weapon and item hits are not routed (not sure the asm patch covers them, see uncertainties) |
| 0x80140BCC `extend_on_hit_routines_` | on-hit colour animation by damage type | ftCommonDamageCheckElementSetColAnim (ftcommondamage.c) | Applied: ids 7..0xC return `ftRemixDamageSetColAnim` (SHADOW 0x59 + level, STUN/FLASHBANG electric, LASER 0x6B, DEKU 0x6C, BURY common) |
| 0x8014158C `stun_damage_action_` | Stun action for STUN, bury and flashbang | ftCommonDamageGotoDamageStatus+0x2C | Skipped: needs the stun, bury, plunge and flashbang main routines (about 450 asm lines), not ported |
| 0x801412E8 `stun_electric_action_` | electric damage action when already stunned | ftCommonDamageInitDamageVars+0x404 | Skipped, same reason |
| 0x800E86B4 `prevent_wind_push_while_buried` | no Whispy push while buried | ftParamSetVelPush (ftparam.c) | Skipped: needs `buried_main_` identity, same reason |
| jump table data (`add_damage_type`) | table entries 7..0xC | none | Same ids as above. The vanilla tables are not touched |

Consequence: with this patch, types 7..0xC get their GFX and colour animation but not the stun, bury or flashbang action
changes. Those types behave like a normal hit in the action sense.

## Toggles

- `FTREMIX_POISON_CSS_DAMAGE_DEFAULT FALSE`: the CSS "Poison Damage" option is off. The Remix default is 0 (off).

## REMIX-TODOs

- `REMIX-TODO(toggles)`: CSS PoisonDmg option not ported (ftremixpoison.c).
- `REMIX-TODO(Stamina)`: stamina clamp and defeated-stock skip of the poison damage (ftremixpoison.c, `ftRemixPoisonDamage`).
- `REMIX-TODO(GFX)`: acid colanim write when Hazards.asm 0x0A28 is 0 applies on the next poison tick instead (grremixspawnedfear.c).
- `REMIX-TODO(Damage)`: stun, bury and flashbang action patches (ftremixdamage.c header).

## Wiring the lead must do

- Call `ftRemixPoisonClear()` where the Remix code calls `Poison.clear_poison_` (Item.asm:1577).
- The `REMIX-TODO(GFX)` in grremixspawnedfear.c `grRemixSpawnedFearPoison` says the port's colanim is applied on the poison tick.

## Uncertainties

- Hit GFX (Damage.asm:40, `extend_on_hit_gfx_`): the patch is at vanilla 0x800E3FC4, `ftMainProcessHitCollisionStatsMain` + 0x108.
  The fighter case is the first switch that comes that far into the function. The weapon and item switches are later in the
  same function and are not routed, so new damage types from weapons or items keep the vanilla default.
- `star_fix` (Poison.asm:17): decoded with `tools/remix_disasm_rom.py` at 0x5D008. The branch is
  `bnel v0, 0x78`, where v0 is `star_invincible_tics`. Both paths reach the damage_heal block, so the Remix hook is the `else`.
- `damage` sign: Poison.asm says "must be negative", but Hazards.asm writes `ACID_DAMAGE = 3` and the tick adds it to the
  percent, so the poison raises the percent. The port does the same (`percent_damage += damage`).
- 0x801313F8 is a shared word: Hazards.asm also reads and writes it (casino bumper, item pointer). The port keeps the poison
  timer separate; behaviour differs only where Remix shares the word.
- The asm's KO path (`_ko`, Poison.asm:200) is unreachable (only reachable through commented-out code), so it is not ported.
