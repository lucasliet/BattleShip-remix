# F9 announcer group A (ROADMAP T2.5, phase 2): hit, throw, combo, sleep

Source: `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm` lines 261 to 590. Public API only (`if/ifremixannouncer.h`),
no edits to the core. Build and game runs are the lead's. Not built and not committed by this worker.

## Patches

| Asm routine | Asm line | RAM (patch) | Vanilla function | Port site | Status |
|---|---|---|---|---|---|
| `sleep_announcement_` | 261 | 0x801499AC (0xC43EC) | `ftCommonFuraSleepSetStatus` (`ft/ftcommon/ftcommonfurasleep.c`) | `ifRemixAnnouncerRequestSleep()` when `ifRemixAnnouncerIsActive()`, at the top of the function | applied |
| `pokemon_combo_` | 295 | none (called from `draw_hit_count_`) | ComboMeter `draw_hit_count_`, port `ifRemixComboHitsUpdateInCombo` (`if/ifremixcombohits.c`) | `ifRemixComboHitsAnnounceTenHits`: once per combo at 10 hits, when active, `nIFRemixAnnouncerCallCombo` | applied (replaces the REMIX-TODO) |
| `hit_announcement_` | 350 | 0x801415A8 (0xBBFE8) | `ftCommonDamageGotoDamageStatus` (`ft/ftcommon/ftcommondamage.c`), `else` branch before `ftCommonDamageInitDamageVars` | `ifRemixHitAnnouncerPlayHit(fp)` | applied |
| `throw_hit_announcement_` | 486 | 0x8014B1A4 (0xC5BE4) | `ftCommonThrownReleaseThrownUpdateStats` (`ft/ftcommon/ftcommonthrown2.c`), before `ftCommonDamageInitDamageVars` | `ifRemixHitAnnouncerPlayThrow(damage, knockback_final)` | applied |
| `kirby_fthrow_1_fix` (`jigglypuffkirbyshared.asm`, 0xC4C78 / RAM 0x8014A238), JKIRBY branch only | n/a (outside group A) | 0x8014A238 | `ftCommonThrowSetStatus` (`ft/ftcommon/ftcommonthrow.c`), kirby forward throw branch | `nIFRemixAnnouncerCallSeismicToss` for `REMIX_ID_JKIRBY` when active (the call is the asm `seismic_toss_announcement_`) | applied for JKIRBY |
| `kirby_fthrow_1_fix`, BOWSER and GBOWSER branch | n/a | 0x8014A238 | same | seismic toss clip and action 0xE5 | skipped, marker kept |

## New module

`decomp/src/if/ifremixhitannouncer.c` and `.h` (guarded by `#if defined(PORT) && defined(REGION_REMIX)`). The hit and
throw share the knockback and damage buckets, so they live together instead of being copied into two fighter files.

- `ifRemixHitAnnouncerPlayHit(FTStruct *victim_fp)`: reads `damage_knockback`, `damage_queue`, `damage_element`
  (the offsets 0x7E0, 0x7F0, 0x7F8 of the asm hook).
- `ifRemixHitAnnouncerPlayThrow(s32 damage, f32 knockback)`.

## Thresholds and order (copied from the asm)

- Hit knockback: `>= 115.0` enters the knockback path. `>= 155.0` plays `nIFRemixAnnouncerCallKnockbackHigh`, otherwise
  `nIFRemixAnnouncerCallKnockbackMid`. Below 115 the damage path runs and the knockback path is not taken.
- Throw knockback: `>= 120.0` enters the knockback path. `>= 175.0` high, otherwise mid.
- Hit damage path: damage 0 returns. Fire element with damage plays `nIFRemixAnnouncerCallBurn`. Damage below 4 plays
  `nIFRemixAnnouncerCallLittleDamage`. Damage below 18 plays nothing. Otherwise `nIFRemixAnnouncerCallDamageEarly` when
  `gSCManagerBattleState->time_passed < 0x360`, else `nIFRemixAnnouncerCallDamage`.
- Throw damage path: same buckets as the hit without the fire and early checks.
- The asm `a1` counts (12, 6, 4, 1) are the table lengths, so the core picks by table length (no count argument needed).
- The asm FGM fallback (`jal 0x800269C0` with `v1`) is inside the core `ifRemixAnnouncerPlayCall`, which plays the FGM
  itself. No separate fallback is needed here.

## Left open

- `kirby_fthrow_1_fix` BOWSER and GBOWSER: seismic toss clip and action 0xE5 are not ported (the marker in
  `ftcommonthrow.c` is kept for these).
- Time unit of `0x360` (asm comment says 9 seconds) is copied as `time_passed` ticks. Core notes already list this for
  verification.
- The sleep hook runs before `ftMainSetStatus`. The asm runs it after two prologue instructions, so the order is the same.
- Not verified by a build or a game run (lead's task).
