# F9 announcer group C (Phase 2)

Asm: `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm` lines 1054 to 1490. Not built, not run.

| Move | Asm line | RAM (patch) | Vanilla function | Enum | Status |
|---|---|---|---|---|---|
| Slash | 1054 (`slash_announcement_`) | none (called from `Wolf/WolfSpecial.asm:906`) | `ftRemixWolfSpecialUSlashGraphics` (`ftremixwolf.c`, slash_state 0) | `Slash` | Applied (replaces REMIX-TODO) |
| Flamethrower (Fire Flower) | 1095 (`fireflower_`) | 0x80175F58 | `itFFlowerWeaponFlameMakeWeapon` (`itfflower.c`) | `Flamethrower` | Applied. Also covers Bowser NSP, which calls this function in the decomp |
| Flamethrower (Charizard) | 1110 (`charizard_`) | 0x8017FC48 | `itLizardonWeaponFlameMakeWeapon` (`itlizardon.c`) | `Flamethrower` | Applied |
| Fire Punch | 1178 (`firepunch_announcement_`) | none (called from `captainshared.asm:207`) | none in C | `FirePunch` | Skipped. The Captain Falcon punch is still asm and has no C call site |
| Counter | 1219 (`counter_announcement_`) | none (called from `Marth/MarthSpecial.asm:952, 974`) | `ftRemixMarthDSPStartAttack` (`ftremixmarth.c`) | `Counter` | Applied (replaces REMIX-TODO) |
| Rest (ground) | 1260 (`rest_g_announcement_`) | 0x801517EC | `ftPurinSpecialLwSetStatus` (`ftpurinspeciallw.c`) | `Rest` | Applied |
| Rest (air) | 1267 (`rest_a_announcement_`) | 0x80151824 | `ftPurinSpecialAirLwSetStatus` (`ftpurinspeciallw.c`) | `Rest` | Applied |
| Pound (ground) | 1315 (`pound_g_announcement_`) | 0x8015150C | `ftPurinSpecialAirNSetStatus` (`ftpurinspecialn.c`) | `Pound` | Applied. The RAM sits in the Air N function (the asm label names are swapped) |
| Pound (air) | 1322 (`pound_a_announcement_`) | 0x801514CC | `ftPurinSpecialNSetStatus` (`ftpurinspecialn.c`) | `Pound` | Applied (same note as above) |
| Seismic Toss | 1370 (`seismic_toss_announcement_`) | none (called from `jigglypuffkirbyshared.asm:297`) | kirby_fthrow_1_fix path in `ftcommonthrow.c` | `SeismicToss` | Skipped. The announcing path (JKIRBY, BOWSER, GBOWSER forward throws) is not ported. The REMIX-TODO stays |
| Hung Tough | 1430 (`hung_tough_announcement_`) | 0x80164C80 | `ftPublicPlayCommon` (`ftpublic.c`), sound 0x267 | `Tough` | Applied |

## Deviations

- The asm fallback FGM (`jal 0x800269C0` when `announcer_main_` returns FALSE) is not replicated. The public API does not expose it. The port calls only `ifRemixAnnouncerPlayCall` behind `ifRemixAnnouncerIsActive()`.
- Hung Tough: the asm also plays an item-based FGM when the call fails. Not replicated. The vanilla sound (0x267) still plays in `ftPublicPlayCommon` as before.
- Bowser flamethrower: the TODO in `ftremixbowser.c` was removed. The announcement now comes from `itFFlowerWeaponFlameMakeWeapon`, which Bowser NSP calls, so there is no second call.
- Call order: the asm patches run at the start of the vanilla function (Rest, Pound, Hung Tough) or after the first original instruction (Fire Flower). The port calls at the top of the function body.

## Left open

- Firepunch: no C call site. Needs the Captain Falcon punch to be ported first.
- Seismic Toss: needs the kirby_fthrow_1_fix forward throw port (`ftcommonthrow.c:104`).
- Build and in-game verification are the lead's job (not done by this worker).
