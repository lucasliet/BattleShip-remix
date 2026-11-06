# Stamina

The Remix Stamina rule: one stock per fighter and a pool of HP instead of the damage percent. It is a value of the Rule button of the VS menu (Stamina
and Stamina Team), not a Remix Modes page entry. Asm source: `Stamina.asm` (in `.toolchain/pkgs/smashremix-src/src`), with `TimedStock.asm`,
`Poison.asm`, `SpecialZoom.asm`, `Size.asm`, `css/PoisonDmg.asm` and the menu hooks in `Stamina.asm` itself.
Port code: `decomp/src/gm/gmremixstamina*.c` (state), `decomp/src/ft/ftremix/ftremixstamina*.c` (fighter side), `decomp/src/if/ifremixstaminahud.c`
(HUD), `decomp/src/mn/mnvsmode/mnremixvsstamina.c` and the hooks in `mnvsmode.c` / `mnplayers/mnplayersvs.c` (menu and CSS).
Tests: `tests/remix/test_stamina_rules.c` (`gmremixstaminarules.c`: total HP, HP left, over-total test, damage to finish), `test_timedstock_rules.c`,
`test_stockmode_rules.c`; `cmake --build build --target run_remix_mode_tests`. Shared plumbing (Timed Stock, Stock Mode): [`../modes_base.md`](../modes_base.md).

## How the Remix does it

### Rules

- Every fighter has `TOTAL_HP` = HP - 1 (the menu value is 1 to 999, default 200 shown, 199 stored). The damage is accumulated as the normal
  percent; when the new percent is above `TOTAL_HP` the fighter is KO'd exactly once, with no stock rebirth. The match ends through the stock path when
  one fighter is left.
- Remix encodes the rule as `Global.vs.game_mode == 5` (`STAMINA_MODE`); the stock/time code treats 5 as a Timed Stock match (value 3) through the
  `stamina_fix_1..6` patches. Stamina Team is the same with `Global.vs.teams` on.
- Knockback is always computed as if the victim were at 20% (the four `stamina_knockback_*` patches; the throw patch overrides the sum for every caller).
- Results use the Timed Stock stock score (`TimedStock.asm` `score_fix_`): `((initial stocks - falls + 1) << 12) - final damage` (see `modes_base.md`).
- Consumers outside the file: `Poison.asm` (poison damage is clamped to the HP and skips a defeated fighter), `SpecialZoom.asm` (no special zoom in
  Stamina), `Size.asm` (`stamina_armor_fix_` zeroes passive armor once the HP is gone), `css/PoisonDmg.asm`, `AirDodge.asm` (the air dash array is reset
  by `action_hit_crash_prevent`), `PokemonAnnouncer.asm` (`stamina_fix_2` calls `pokemon_ko_sounds_`).
- `percent_port[4]` marks a port as already defeated (cleared by `percent_port_refresh` at match start).

### Patch groups (asm to behaviour)

| Asm scope | Behaviour |
|---|---|
| `stamina_display_`, `stamina_dark_`, `stamina_dark_fix_2` | the damage digits show HP left (`TOTAL_HP + 1 - damage`, never below 0) and darken with the ratio damage / HP (works above 300 HP). In the Remix 1P final stage the Giga Bowser HUD has its own branch with 200 / 250 / 300 HP by difficulty |
| `stamina_knockback_`, `_item_`, `_throw_`, `_projectile_` | knockback with 20% |
| `stamina_main_`, `stamina_ko_`, `stamina_percent_prevent` | the KO: KO sound, custom red flicker colour animation 0x66, the vanilla death setup; the stored percent is completed to HP + 1 on the final hit and a defeated fighter stores no more damage |
| `stamina_ko_action` | a fighter out of HP that changes status is forced to DownBounceD on the ground or Tumble in the air, except in protected statuses (dead 0-4, damage 0x25-0x38, Tornado, Barrel, DownBounceD..DownWaitU, CliffCatch..CliffEscapeSlow2, Grab..ThrownFoxB, Donkey Kong cargo/throw 0xEB-0xF5, Marina cargo 0xEB-0xFB); the Giga Bowser branch uses 200 / 250 / 300 by difficulty (below 2, below 4, else) |
| `stamina_player_control_` | the defeated fighter is taken out of control and the defeat FGM is swapped |
| `stamina_shield_prevent` | no shield after the KO |
| `stamina_ko_prevent`, `stamina_ko_prevent_front`, `stamina_fix_6`, `stamina_fix_7` | a fighter at stock -1 cannot die again (blast zone or top blast); a star KO on the last stock leaves -2 in the match data and is stored as -1 in the fighter at rebirth; the rebirth loop is cut |
| `stamina_stock`, `stamina_stock_2` | the fighter starts with stock 0 (one life), after the Stock Mode initial count is stored |
| `stamina_fix_1..5` | treat mode 5 as Timed Stock in the timer, KO, rumble and score code |
| `stamina_down_wait_down`, `_input`, `stamina_down_bounce_input`, `stamina_ledge` | out of HP the fighter cannot stand from down wait, leaves down bounce straight to down wait, and cannot grab a ledge |
| `stamina_invis_prevent`, `egg_crash_prevent`, `action_hit_crash_prevent`, `overkill_prevent` | crash guards (invisible flag skipped for some actions, dead-wait word not set during Egg Lay or a hit), and the end-of-match place counter is not decremented twice for the same fighter |
| `vs_menu`, `add_to_rule_button_` (14 hooks), `add_to_stock_time_button_`, `add_to_stock_time_value_`, `vs_css`, `stamina_scrolling_`, `stamina_hp_display_` | the Rule button grows from 4 to 6 values (Time, Stock, Stamina, Time Team, Stock Team, Stamina Team), the Time/Stock button reads "Stamina." / "H.P." with a 3-digit HP, the CSS picker scrolls the HP |

## How the port does it

### State and rule

- `gGMRemixStaminaRule` (set by the menu Rule value) and `gGMRemixStaminaHP` (1-999, default 200) are the only state; `gmRemixStaminaIsActive`
  is TRUE when the flag is set and the battle rule is `SCBATTLE_GAMERULE_TIMED_STOCK` (so the asm `game_mode == 5` fixes are implicit).
  `gmRemixStaminaGetTotalHP` is HP - 1; `gmRemixStaminaIsDefeated/SetDefeated/ResetDefeated` are `percent_port` (reset in `scVSBattleStartBattle`).
  Files: `gmremixstamina.c`, `gmremixstaminarules.c`.
- Stock: `gmRemixStockModeBeginMatch` stores the Stock Mode count, then returns 0 for the fighter in Stamina (`gmremixstockmode.c`).

### Fighter side (hooks)

| Asm | Port |
|---|---|
| `stamina_knockback_*` | `ftRemixStaminaKnockbackDamageAdd` (`ftremixstaminaknockback.c`), one point in `ftParamGetCommonKnockback` (`ftparam.c`) |
| `stamina_main_`, `stamina_ko_`, `stamina_percent_prevent` | `ftRemixStaminaStoredDamage` (`ftremixstaminako.c`), called from `ftParamUpdateDamage` (`ftparam.c`) |
| `stamina_ko_action` | `ftRemixStaminaKOStatus` (`ftremixstaminaaction.c`), last step of the status id mapping in `ftMainSetStatus` (`ftmain.c`); includes the Giga Bowser 1P branch (`sc1PRemix1PIsActive`). `ftRemixStaminaKOAnimSpeed` forces the animation speed to 1.0 whenever the KO action is forced (`lui a3, 0x3F80`), called before the mapping |
| `action_hit_crash_prevent` | `ftParamStopVoiceRunProcDamage` (`ftparam.c`) calls `ftRemixAirDodgeResetAirDash` on every hit, in every mode, and `ftRemixStaminaClearDeadWaitOnHit` (`ftremixstaminahit.c`), which clears `status_vars.common.dead.wait` of a defeated fighter hit during a VS Stamina match |
| `stamina_shield_prevent` | `ftRemixStaminaIsOutOfHP` early return in `ftcommonguard1.c` |
| `stamina_down_*`, `stamina_ledge` | `ftRemixStaminaDownBlocked`, `ftRemixStaminaLedgeBlocked` (`ftremixstaminadown.c`) in `ftcommondownwaitbounce.c` and `ftcommoncliffcatchwait.c` |
| `stamina_invis_prevent`, `egg_crash_prevent` | `ftRemixStaminaSkipsInvisible`, `ftRemixStaminaSkipsDeadWaitInit` (`ftremixstaminaguard.c`) in `ftCommonDeadInitStatusVars` |
| `stamina_ko_prevent(_front)`, `stamina_fix_6/7` | `ftRemixStaminaIsOutOfStock`, `ftRemixStaminaIsBlastRedeath`, `ftRemixStaminaRebirthSleeps` (`ftremixstaminadead.c`) in `ftcommondead.c`; a second death sends the fighter to sleep (`ftCommonSleepSetStatus`) |
| `stamina_armor_fix_` | `ftRemixStaminaArmorIsZeroed` used by `ftRemixSizePassiveArmor` (`ftremixsizebody.c`) |
| Poison, SpecialZoom | `ftRemixPoisonDamage`, `ftRemixPoisonCanDamage`, `ftRemixPoisonPanelDamage/TryApply` (`ftremixpoison.c`) and `ftRemixSpecialZoomFinalKnockback` (`gmremixspecialzoom.c`) |
| `stamina_display_`, `stamina_dark_` | `ifRemixStaminaDisplayValue` and `ifRemixStaminaDamageRatio` (`ifremixstaminahud.c`) in `ifcommon.c` (damage digits and darkening); sprites, not text |

### Menu and CSS

`mndef.h` extends `MNVSModeRule` with `nMNVSModeRuleStamina` (2) and `nMNVSModeRuleStaminaTeam` (5); `nMNVSModeRuleLast` points to the last value of the
build. `mnremixvsstamina.c` draws the rule value (Stamina image at x 176, Stamina Team as two images at 0.875 scale, x 165 and 224, y 78), the
"Stamina." label at the Stock label position (106, 114), "H.P." at (212, 120), the 3-digit HP value (x 197 / 205 / 209 by digit count) and the CSS
value (x 210 / 214); the sprites come from the VS menu file (`sMNVSModeFiles[1]`) at offsets 0x6D28 (Stamina), 0x6B28 ("Stamina.") and 0x6E28
("H.P."). `mnVSModeSaveSettings` stores the flag and `game_rules = TIMED_STOCK` for both values; the rule is restored from the flag when the menu
opens; Stamina counts as a free-for-all rule for team shading. On the CSS, `mnPlayersVSMakeStockSetting` shows the HP and the arrows call
`mnRemixVSStaminaScrollHP`; the "H.P." background uses the goal picker image path. The panel menu Initial Damage entry is limited to the HP
(`mnRemixPanelMenuRulesStaminaDamage`, `mnremixcsspanelmenuinput.c`).

HP steps: `mnRemixVSStaminaStepHP` moves 1 below 10, 10 from 10 to 990 and 1 above 990, clamped to 1..999, for both the VS menu and the CSS.

### Deviations from the asm

- **Rule encoding.** TIMED_STOCK plus a flag instead of `game_mode == 5`; the `stamina_fix_*` rewrites are implicit.
- **Shield guard.** Tests percent above HP instead of stock == -1, so a fighter out of HP cannot shield even before the KO sets the stock.
- **Damage cap.** `ftRemixStaminaStoredDamage` caps the fighter's own percent at HP + 1; the asm caps only the battle-state copy used by the HUD. The visible
  result is the same.
- **Second death.** The asm lets a re-death run the score (stock -1 to -2) and repairs it at rebirth; the port stops the re-death before the score
  (`ftcommondead.c` guards, also on the star KO death proc, which has no asm hook), so no fall or KO is counted twice. The rebirth keeps the -2 to -1 fix.
- **Status ranges.** The cliff range includes CliffEscapeSlow2 (0x063), as the asm comment says; damage statuses are 0x25-0x38 (Tumble, used by the
  KO itself, is not protected); generic heavy throws are not protected, as in the asm.
- **Armor.** Only the passive armor guard is ported; the asm also erases the passive and Yoshi armor words (`0x7E4`, `0x7E8`) on the KO.
- **Defeated fighter.** It goes to the Sleep status (`ftCommonSleepSetStatus`) instead of losing control.
- **Input type in the HP picker.** One step rule for both screens (above).

Audit ledger (every item closed): `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

- Shortcut: `SSB64_VS_STAMINA=<hp>` (VS shortcut becomes a Royal Timed Stock match with the flag and HP set; the shortcut gives each port its own team) with `.claude/skills/remix-smoke-tests/scripts/vs.sh 0,8,6 .claude/skills/remix-smoke-tests/inputs/none.txt <name> 2200`. Expect the
  HUD to show the HP left and darken, one stock per fighter, a fighter that falls over the HP leaves play once, and the match ends with results.
- Menu: open the VS menu, Rule button: values cycle Time, Stock, Stamina, Time Team, Stock Team, Stamina Team;
  the Time/Stock button shows "Stamina." and "H.P."; start and return, the rule and HP persist; the CSS picker shows the HP with 3 digits.
- Death guards: a blast zone death at stock 0 gives one death and no second score; a star KO on the last stock stays at -1 and the fighter sleeps.
- Guards: out of HP, Z does not shield, down wait does not stand, no ledge grab, giant (Size) armor does not reduce damage; KO during Egg Lay or a
  capture does not crash.
