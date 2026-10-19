# F9 announcer groups D and E (ROADMAP T2.5, phase 2)

Source: `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm` and `Stamina.asm`. Public API:
`decomp/src/if/ifremixannouncer.h`. Each C hook is `#ifdef REGION_REMIX`, and only calls `ifRemixAnnouncerIsActive()`
plus a play function. Vanilla behaviour is unchanged when the announcer is off.

Semantics: the asm `announcer_main_` returns 0 when it picked a clip, then the asm plays the FGM. The port's
`ifRemixAnnouncerPlay*` plays the FGM itself, so the hooks do not add a second `syAudioPlayFGM`.

Vanilla function and offsets come from `docs/remix/gameplay_m_z.md` (rows 158, 176-184) and `build/remixasm/refs/vanilla_funcs.txt`.
That map has stale line numbers, so the C locations below are from the current decomp source.

| Group | Asm line | RAM | Vanilla function (+offset) | C file | Call | Status |
|---|---|---|---|---|---|---|
| E name | 78 | 0x8016F240 | `itManagerMakeItemKind` (+0x8) | `src/it/itmanager.c` | `ifRemixAnnouncerPlayName(kind)` at function top | applied |
| E KO | 129 via Stamina.asm 908 | 0x8013BEB8 | `ftCommonDeadUpdateScore` (+0x154) | `src/ft/ftcommon/ftcommondead.c` | `ifRemixAnnouncerPlayKO(this_fp)` before the score update | applied, see note 1 |
| D recovery | 1491 | 0x800EA3D8 | `ftParamSetHealDamage` (+0x4) | `src/ft/ftparam.c` | `PlayCall(Recovery)` at function top | applied |
| D recovery magnet | 1540 | 0x800E33DC | `ftMainUpdateAbsorbStatWeapon` (+0xD4) | `src/ft/ftmain.c` | `PlayCall(Recovery)` before the `can_not_heal` check | applied |
| D taunt | 1592 | 0x8014E6F0 | `ftCommonAppealSetStatus` (+0x10) | `src/ft/ftcommon/ftcommonappeal.c` | `PlayCall(Taunt)` after `fp` | applied |
| D clang | 1641 | 0x800E26D8 | `ftMainSetHitInteractStats` (+0x1C) | `src/ft/ftmain.c` | `PlayCall(Clang)` if `attack_type == nGMHitTypeAttack`, after the basketball block | applied |
| D stun | 1696 | 0x801498A8 | `ftCommonFuraFuraSetStatus` (+0x4) | `src/ft/ftcommon/ftcommonfurafura.c` | `PlayCall(Stun)` at function top | applied |
| D time up | 1741 | 0x801137C4 | `ifCommonBattleEndPlaySoundQueue` (+0x28) | `src/if/ifcommon.c` | none | skipped, see note 2 |
| D team KO | 1780 | 0x8013D70C | `ftCommonSleepProcUpdate` (+0x3C) | `src/ft/ftcommon/ftcommonsleep.c` | `PlayCall(TeamKO)` at the top of the team-stock branch | applied, see note 3 |
| D air specials | 1857 | 0x80150F38 | `ftCommonSpecialAirCheckInterruptCommon` (+0x30) | `src/ft/ftcommon/ftcommonspecialair.c` | 1 in 20 roll, then `PlayCall(Attack)`, inside the B-pressed block before the hammer check | applied, see note 4 |
| D ground specials | 1930 | 0x801510C0 | `ftCommonSpecialNCheckInterruptCommon` (+0x28) | `src/ft/ftcommon/ftcommonspecialn.c` | 1 in 20 roll, then `PlayCall(Attack)`, inside the B-pressed block | applied, see note 4 |

## Notes

1. The KO hook is in `Stamina.asm` `stamina_fix_2`, a patch in `ftCommonDeadUpdateScore`. The asm calls
   `pokemon_ko_sounds_` on every call of that function, before any stock or score decision. The port call sits at the
   top of the function, after the PORT HUD guard, so it reads the same `percent_damage` and time. It does not check
   the Stamina mode, as in the asm.
2. Time up is skipped. The asm hook replaces the vanilla TIME UP sfx (0x20F) with the announcer clip 0x4AE when the
   announcer is on. The public API has no call for a raw FGM id. The core `ifRemixAnnouncerRun` already plays its own
   time-up FGM (0x4A1) when `time_remain` is 0xE09, so a second clip is not added. Needs a decision from the lead:
   either add a public call for 0x4AE, or confirm that 0x4A1 covers it and the vanilla sfx should be silenced in
   `ifCommonBattleEndPlaySoundQueue`. The vanilla sfx is still played, unchanged.
3. The team KO asm hook sits in `ftCommonSleepProcUpdate` at +0x3C. This is an estimate. Placement is at the top of
   the team-stock branch, which is where the "fallen comrade" steal logic runs. Verify in game.
4. Air and ground specials: the asm rolls `Global.get_random_int_(0x14)` (5 percent) only when the announcer is
   active, then plays the attack table with 3 entries (asm `a1 = 3`, table length 3). The roll is in the C with
   `syUtilsRandIntRange(20)`, and `&&` keeps the roll conditional on the toggle. Air placement is exact (the asm hook
   sits just before the `ftHammerCheckHoldHammer` call). Ground placement is inside the B-pressed branch. The asm hook
   has no button check of its own, so this is an estimate. Verify in game.
5. `a1` counts checked against the table lengths: recovery 1, clang 1, taunt 1, stun 2, team KO 2, attack 3. All
   match, so `ifRemixAnnouncerPlayCall` needs no count argument here.

## Left open

- Time-up decision (note 2).
- Verify placements for team KO and ground specials (notes 3 and 4) in game.
- No build or run done by this worker, per brief.
