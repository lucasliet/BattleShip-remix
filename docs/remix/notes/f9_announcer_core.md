# F9 announcer core (ROADMAP T2.5, phase 1)

Source: `.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm`. Port: `decomp/src/if/ifremixannouncer.c` and
`decomp/src/if/ifremixannouncer.h`, guarded by `#if defined(PORT) && defined(REGION_REMIX)`. Syntax check: OK.

## Ported routines (asm line of each label)

| Asm routine | Line | C function |
|---|---|---|
| `toggle_announcer_` | 2003 | `ifRemixAnnouncerIsActive` (toggle `announcer_mode`, stage check on `gSCManagerSceneData.gkind`) |
| `announcer_main_` NAME | 2054 | `ifRemixAnnouncerPlayName` / `ifRemixAnnouncerPickName` |
| `announcer_main_` KO | 2054 | `ifRemixAnnouncerPlayKO` / `ifRemixAnnouncerPickKO` (+ quick, long, regular picks) |
| `announcer_main_` GENERAL | 2054 | `ifRemixAnnouncerPickGeneral` (run by the per-frame general timer) |
| `announcer_main_` STANDARD | 2054 | `ifRemixAnnouncerPlayCall` / `ifRemixAnnouncerPickStandard` |
| `announcer_setup_` | 2248 | `ifRemixAnnouncerSetup` (resets, creates the per-frame GObj) |
| `announcer_constant_` | 2287 | `ifRemixAnnouncerRun` (countdown, time-up, opening, sleep, general) |
| call tables | 2453-2642 | static `IFRemixAnnouncement` arrays, `sIFRemixAnnouncerTables` in `IFRemixAnnouncerCall` order |
| `there_goes_` | 18 (RAM 0x80114D30) | `ifRemixAnnouncerGameSetFgm` |
| `delay_` | 42 (RAM 0x80114D38) | `ifRemixAnnouncerGameSetDelay` |

Not ported here: `pokemon_name_` (asm 78, RAM 0x8016F240) and `pokemon_ko_sounds_` (asm 129). They are phase-2
group E and call `ifRemixAnnouncerPlayName` and `ifRemixAnnouncerPlayKO`.

## Public API (ifremixannouncer.h)

- `IFRemixAnnouncerCall` enum: 40 values, `nIFRemixAnnouncerCallPokemon` ... `nIFRemixAnnouncerCallTaunt`, then
  `nIFRemixAnnouncerCallCount`.
- `sb32 ifRemixAnnouncerIsActive(void)`
- `void ifRemixAnnouncerSetup(void)`: no-op when not active (asm gates `announcer_setup_` on `toggle_announcer_`).
- `sb32 ifRemixAnnouncerPlayCall(IFRemixAnnouncerCall call)`
- `sb32 ifRemixAnnouncerPlayName(s32 item_kind)`
- `sb32 ifRemixAnnouncerPlayKO(FTStruct *victim_fp)`
- `void ifRemixAnnouncerRequestSleep(void)`
- `u16 ifRemixAnnouncerGameSetFgm(u16 vanilla_fgm)`
- `s32 ifRemixAnnouncerGameSetDelay(s32 vanilla_delay)`

## Wiring left to the lead

1. Setup call site: `ifRemixAnnouncerSetup()` at the same place as `ifRemixComboMeterSetup()`
   (`decomp/src/sc/sccommon/scvsbattle.c:285`, `decomp/src/sc/sc1pmode/sc1pgame.c:2384`). The asm calls it from
   the stage load in `Stages.asm:626`, so the VS and 1P battle scenes are the closest match. Confirm before wiring.
2. Game set: `decomp/src/if/ifcommon.c` lines 3645 and 3647 pass `nSYAudioVoiceAnnounceGameSet` and `90` to
   `ifCommonBattleSetInterface`. Wrap them as `ifRemixAnnouncerGameSetFgm(nSYAudioVoiceAnnounceGameSet)` and
   `ifRemixAnnouncerGameSetDelay(90)`, under `#ifdef REGION_REMIX`. The `sfx_id` is an FGM id, so the helper fits.
   Not wired by this worker.

## Deviations and assumptions to verify

- Regular KO: the asm retry branch falls through, so the last of four draws is always used. The port draws once,
  uniformly. The cooldown is not checked for regular KO, as in the asm.
- Standard and general picks: one attempt per table entry, as the asm standard path does. The general pick is bounded
  to the table length. The asm loops until a free entry is found, which can hang when all are cooling down.
- Gym Leader Castle opening: the asm draws `random(2)` over `gym_leader_call`, which has one entry, so index 1 reads
  the first entry of `opening_calls` (0x4B7). The port uses the table length (1). Decide whether to keep the asm
  behaviour.
- Close match stock check: the asm tests `Global.vs.game_mode < 2`. The port tests the stock flag (0x2) in
  `gSCManagerBattleState->game_rules`. Verify the mapping.
- Time: `Global.vs.elapsed` maps to `time_passed` and `Global.vs.timer` to `time_remain` (both `SCBattleState`).
  The asm values (0x1E opening, 0x4B0 quick KO, 0xE09 time-up) are used as tick counts. Verify the units.
- The per-frame GObj is linked with `nGCCommonLinkIDInterface`, by analogy with `ifcommon.c`. The asm uses Render group 1.
  Pause behaviour is not checked.
- The global timer is 0x78 (120 ticks) although the asm comment says 3 seconds. The port keeps the value.
- Phase-2 call sites must pass the asm `a1` count when it differs from the table length. Only the gym branch differs.
- Tables are the 40 call tables of the asm (94 entries). The `.c` file is long because of them.

## Left open

Phase 2 groups A to E, the `pokemon_name_` and `pokemon_ko_sounds_` hooks, and the game-set wiring above.
