# F10 panel option: Poison Dmg (CSS option 11, PoisonDmg)

Source (read only): `.toolchain/pkgs/smashremix-src/src/css/PoisonDmg.asm` (apply_damage, state_table) and `src/Poison.asm`
(poison_hook at 0x800E1820, tail 0x800E1850, heal path 0x800E1828, `_poison_check` after apply_damage).
Vanilla function hooked: `ftMainProcUpdateInterrupt` (`decomp/src/ft/ftmain.c`, call of `ftRemixPoisonUpdate` in the
`damage_heal == 0` branch, already present). No new public declarations, so `ftremixhooks.h` is unchanged.

## Edited

`decomp/src/ft/ftremix/ftremixpoison.c`
- `ftRemixPoisonPanelTickDue`: match tic test (`time_passed`, 0 = not started). Game speed toggle 0: low 7 bits == 0.
  Speed 9 or 11 (1/3, 2/3): low bits < 3. Other speeds: low bits == 0. Faster-speed extra advance never ticks.
- `ftRemixPoisonPanelIsBlocked`: Super Star (`star_invincible_tics`) or respawn (`status_id < nFTCommonStatusWait`).
- `ftRemixPoisonPanelAmount`: 1 Low = 1, 2 Medium = 2, 3 High = 4.
- `ftRemixPoisonPanelHeal`: sets `damage_heal = 2`, so the vanilla heal block heals 2 HP.
- `ftRemixPoisonPanelDamage`: adds the percent, caps at 999, then `ftRemixPoisonSyncStock`.
- `ftRemixPoisonPanelTryApply`: reads `gmRemixPanel(POISON_DAMAGE, fp->player)`. Returns TRUE when it healed or hurt, which
  skips the hazard poison for that frame (the asm jumps to the heal or tail path).
- `ftRemixPoisonUpdate`: calls `ftRemixPoisonPanelTryApply` first, then the existing hazard path unchanged.
- Removed the `REMIX-TODO(toggles)` and `FTREMIX_POISON_CSS_DAMAGE_DEFAULT`.

Default value 0 returns FALSE before any side effect, so the hazard path is byte-for-byte the previous behaviour.

## Deviations from the asm and the brief

- Heal is not blocked by the cloak: in `PoisonDmg.asm` the Heal branch (`_anti_venom`) is taken before the Cloaking
  Device check, which only guards the damage types. The brief said cloaked fighters are skipped; the port follows the asm.
  Lead to confirm.
- Heal lands on the next frame, not the same frame: the asm jumps into the healing portion immediately, the port sets
  `damage_heal` and the vanilla heal block applies it on the following frame (the heal block runs before the poison call).
- Timer: the asm reads the match elapsed tic (`match_info + 0x18`), the port uses `gSCManagerBattleState->time_passed`
  ("Current match tic"). The hazard path keeps its own 0x801313F8 timer.
- Game speed: reads `gmRemixToggle(game_speed)` directly like the asm (Toggles entry_game_speed), not the stopwatch-aware
  `ftRemixSpeedCurrent`.

## Left

- `REMIX-TODO(Stamina)`: the stamina clamp at `Stamina.TOTAL_HP` and the skip of defeated stocks. `Stamina.VS_MODE` has no
  port. The hazard path keeps its own TODO.

## Verification

- `scripts/remix/syntax-check.sh decomp/src/ft/ftremix/ftremixpoison.c`: syntax-check OK.
- Not built or run (lead). A warning in `port/remix/remix_player_panel.h` line 2 (`/*` inside a block comment) comes from
  another worker's file, not this one.
