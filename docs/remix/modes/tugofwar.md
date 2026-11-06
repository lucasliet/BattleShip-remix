# Tug of War

Remix VS mode 5 (`MNREMIX_VSMODE_TUG_OF_WAR`, VS menu page 2): a Timed Stock match where every KO also gives the KOer a stock. Asm source:
`TugOfWar.asm` (in `.toolchain/pkgs/smashremix-src/src`), plus `TimedStock.asm` (`score_fix_`), `StockMode.asm`, `css/StockMode.asm` and the CSS hooks in
`VsRemixMenu.asm` / `TwelveCharBattle.asm`.
Port code: `decomp/src/gm/gmremixtugofwar.c` (state and credit), `gmremixtugofwarrules.c` (pure choice), `decomp/src/if/ifremixbattleplace.c`, hooks in
`ft/ftcommon/ftcommondead.c`, `ftcommondamage.c`, `ftcommonshieldbreakfly.c`, `sc/sccommon/scvsbattle.c`.
Tests: `tests/remix/test_tugofwar_rules.c` (credit choice, valid ports, revive rule), `test_timedstock_rules.c`, `test_stockmode_rules.c`;
`cmake --build build --target run_remix_mode_tests`. Shared plumbing: [`../modes_base.md`](../modes_base.md).

## How the Remix does it

### Rules

- `before_css_setup_` forces `Global.vs.game_mode = 3` (Timed Stock; the asm comment says "TIMED STOCK"), `stocks = 3` and the saved `teams` flag
  (default off); `stockmode_table` is cleared. There is no timer logic in the file: the match ends by the vanilla stock rule, or by the time limit of the
  VS menu.
- On a KO the victim loses its stock by the vanilla rule. The hook then gives the **KOer** one stock: `+1` on the fighter's stock counter and on
  `players[].stock_count`. A KOer that was eliminated (its counter goes from -1 to 0; a fighter at -2, waiting for a stock steal in teams, is not) is revived with `ftCommonRebirthDownSetStatus` and the battle place counter
  is repaired (`update_sIFCommonBattlePlace_`: in a team battle nothing changes while a teammate still has stocks, otherwise the place goes back).
- Who is credited (`credit_stock_to_koer_`, `credit_stock_when_no_koer_`): the KOer when it is on another team than the victim; if the KOer is on
  the victim's team, the last opponent that hit the victim; with no KOer (self destruct, environment) the last fighter that hit the victim; if nobody
  hit the victim, nobody (a TODO in the asm). The stock the credit gives also updates `StockMode.previous_stock_count_table`, so Stock Mode Last
  follows the credited stocks.
- Last hit tracking (`update_last_hit_by_`): `last_hit_by_port[victim port]` is set on damage by a fighter of another team, and on a shield break by
  the `shield_player` when it is on another team. `setup_` (called from `Render.asm` at each match start) clears the table and `shield_player` of every
  fighter (a vanilla bug leaves it uninitialized), records the team of each port and detects 1v1: with two fighters each one's last hit is the other
  one, the damage and shield hooks do not record, and the last hit is not cleared after a credit.
- Results: `score = (stocks left + 1) << 24 + (KOs - deaths) << 12 - final damage` (`TimedStock.asm` `score_fix_`, branch `_tug_of_war`).
- Stock Mode: `StockMode.asm` `set_initial_stock_count_` uses the stored Last count as is in Tug of War (it is not lowered to the match count);
  `css/StockMode.asm` does not reset the Stock option to Default in this mode; the CSS counter may go above `stocks`.
- CSS: title image at offset `0x30F0` of the game modes file, announcer FGM 479.

## How the port does it

| Asm | Port |
|---|---|
| mode profile (Timed Stock, 3 stocks, teams saved) | `gmremixmodecss.c` profile for `MNREMIX_VSMODE_TUG_OF_WAR` |
| `setup_` | `gmRemixTugOfWarSetup` (`gmremixtugofwar.c`), called in `scVSBattleStartBattle`: `gmRemixTugOfWarInitFighters` (shield player to -1, team per port), `gmRemixTugOfWarCollectFighterPorts`, 1v1 detection and the crossed last hits |
| `update_last_hit_by_._damage` | `gmRemixTugOfWarRecordDamage(victim, attacker)`, called in `ftCommonDamageInitDamageVars` (`ftcommondamage.c`) |
| `update_last_hit_by_._shield` | `gmRemixTugOfWarRecordShieldBreak(victim)`, called before `ftParamUpdate1PGameDamageStats` in `ftCommonShieldBreakFlyCommonSetStatus` (`ftcommonshieldbreakfly.c`) |
| `credit_stock_to_koer_`, `credit_stock_when_no_koer_` | `gmRemixTugOfWarCreditKO(victim)`, called in `ftCommonDeadUpdateScore` (`ftcommondead.c`) before the victim loses its stock; the pure choice is `gmRemixTugOfWarPickCreditPort(victim_team, koer_team, koer_port, last_hit_port)` (`gmremixtugofwarrules.c`) |
| `update_sIFCommonBattlePlace_` | `gmRemixTugOfWarFixPlace` + `ifRemixBattlePlaceRestoreRevived` (`if/ifremixbattleplace.c`) |
| stock update | `gmRemixTugOfWarGiveStock`: `fp->stock_count++`, `players[].stock_count` follows, `gmRemixStockModeSetLastCount`, revive through `ftCommonRebirthDownSetStatus` when the fighter counter was -1 (`gmRemixTugOfWarShouldRevive`) |
| TimedStock Tug branch | `gmRemixTimedStockScore` calls `gmRemixTimedStockRulesTugScore` (`gmremixtimedstockrules.c`) from `mnvsresults.c` |
| Stock Mode Last in Tug | `gmRemixStockModeRulesLastCount(..., is_tug_of_war)` skips the lowering |
| CSS title and announcer | `mnremixcssmoderules.c` (offset `0x30F0`, FGM 479) |

Every hook starts with `gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR)` and returns at once in the other modes. The mode has no HUD of its own: the vanilla
stock icons show `players[].stock_count`. The port logs each credit as `tugofwar: stock credit to port N, stock_count A -> B`.

### Deliberate deviations from `TugOfWar.asm`

Three spots of the asm index or compare with the wrong register; the port does what the code clearly meant. Everything else in the credit rules
follows the asm.

- **`last_hit_by_port` read by team instead of port.** In `credit_stock_to_koer_` (`TugOfWar.asm:302`), when the KOer is on the victim's team the stock
  goes to the last opponent that hit the victim. The asm reads `last_hit_by_port` with `t2`, the victim's team (`lbu t2, 0x000C(s0)`), but every write
  indexes it by the victim's port. In free-for-all team and port are equal; in a team battle the asm reads the slot of another port. The port reads
  the victim's port (`gmRemixTugOfWarGetLastHit(victim->player)`).
- **Shield break by a teammate.** In `update_last_hit_by_._shield` (`TugOfWar.asm:~410`) the same-team test compares the attacker's team (`t0`) with
  `t1`, which at that point holds the address of `last_hit_by_port` (`li t1, last_hit_by_port`), so it never matches and a teammate that breaks the
  shield is recorded as the last hit. The damage path compares teams correctly. The port skips a shield break by a teammate
  (`gmRemixTugOfWarRecordShieldBreak`), as the comment of the hook says ("unless they're on the same team").
- **Stock cap.** The asm guards the increment with `slti 0xFF` on a signed byte, which never limits, so 127 wraps to -128. The port stops at 127
  (`GMREMIX_TUGOFWAR_STOCK_MAX`).

Audit ledger (every item closed): `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

- Shortcut: `SSB64_REMIX_MODE=tugofwar` with `.claude/skills/remix-smoke-tests/scripts/vs.sh 0,8 .claude/skills/remix-smoke-tests/inputs/none.txt tow 2200`; the log has
  `remix mode: mode=5 ...` at the start and `tugofwar: stock credit` after a KO. Two fighters is the 1v1 case; four fighters with `css.sh` and
  `inputs/css_pick_four.txt` (with `SSB64_REMIX_MODE=tugofwar`) cover free-for-all credit by last hit; add `SSB64_VS_TEAMS=1` for team credit.
- Stock Mode Last: `SSB64_REMIX_PANEL="StockMode.1=Last"` (key format `<Key>.<port>=<value>`), then a credited KO raises the stored Last count.
- A KO that eliminates the KOer's last stock then credits it must show the KOer back in play and the results place coherent.
