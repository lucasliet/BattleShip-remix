# F9 worker brief: PokemonAnnouncer (ROADMAP T2.5)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Absolute paths and `builtin cd` always. Remix source (read-only):
`.toolchain/pkgs/smashremix-src/src/PokemonAnnouncer.asm` (2642 lines). Read `docs/remix/f7_leftover_worker_brief.md` first
(rules, translation rules, final report). Build and game runs are the lead's; you only write code and notes.

## What the announcer is

The Pokemon Stadium announcer plays FGM voice clips (ids from 0x45B up, Remix sounds) for KOs, hits, specials, combos, sleep,
the opening and the end of the match. The toggle `Announcer` (`gmRemixToggle(announcer_mode)`: 0 = Pokemon stages only,
1 = all stages, 2 = off) picks where it speaks. When it is off the game plays the vanilla sound or none, with no change.

`toggle_announcer_` (asm line 2003) returns v0 = 0 when the announcer is ACTIVE for the current stage (mode 1; mode 0 on
Pokemon Stadium, Gym Leader Castle or Pokemon Stadium 2) and 1 when it is not. Stage id: `gSCManagerBattleState->gkind`
(Remix stage ids in `decomp/src/gr/grremix/grremixids.h`).

## Phase 1 (one worker): the core, `decomp/src/if/ifremixannouncer.c` + `.h`

Port to C, all under `#if defined(PORT) && defined(REGION_REMIX)`:

1. The call tables (asm lines 2453 to 2642, `add_announcement(id, fgm, duration, flag)`) as `static` arrays of
   `IFRemixAnnouncement { u16 id; u16 fgm_id; u32 duration; u32 timer; u16 used; u16 flag; }`, one array per asm label
   (`pokemon_calls`, `quick_ko_calls`, `regular_ko_calls`, `long_ko_calls`, `general_calls`, `close_match_call`,
   `sleep_calls`, `combo_calls`, `knockback_*`, `damage_calls`, `thunder_call`, ... every label). FGM ids and durations are
   code constants of the open-source asm; copy them as `#define`s or table literals.
2. `announcer_main_` (line 2054) with its four modes NAME / KO / GENERAL / STANDARD, the global 3-second timer
   (`announcer_timer_global` = 0x78 ticks) and per-call timers, as one function per mode (short functions, no gotos).
   Random picks use `syUtilsRandIntRange(n)`.
3. `announcer_setup_` (line 2248, reset of the timers, `sleep_flag`, used flags) and `announcer_constant_` (line 2287, the
   per-frame countdown, the 1-minute-left call, the opening call at 30 ticks, the random general call every 45 s to 1:30,
   the sleep call). The per-frame part runs from a GObj process created at match start: copy the pattern of
   `decomp/src/if/ifremixcombometer.c` or the nearest Render-routine port (`grep -rn gcAddGObjProcess decomp/src/if/ifremix*`)
   and name the setup call site in the notes (the lead wires it where `ifRemixComboMeterSetup` is called).
4. `there_goes_` and `delay_` (lines 18 to 76): "There goes the battle" replaces the vanilla game-set clip. Name the vanilla
   function and give a helper `ifRemixAnnouncerGameSetFgm(u16 vanilla_fgm)` / `ifRemixAnnouncerGameSetDelay(s32 vanilla)`;
   the lead wires it, or wire it yourself under `#ifdef REGION_REMIX` if the call site is obvious in
   `decomp/src/if/ifcommon.c`.
5. The public API the phase-2 workers will call (declare it in `ifremixannouncer.h`, document each line):

```c
sb32 ifRemixAnnouncerIsActive(void);                       /* toggle_announcer_ == 0 */
/* Plays the clip of the call when the global timer and the call timer allow it. Returns TRUE when a clip played. */
sb32 ifRemixAnnouncerPlayCall(IFRemixAnnouncerCall call);  /* STANDARD mode over the table of the enum value */
sb32 ifRemixAnnouncerPlayName(s32 item_kind);              /* NAME mode, pokemon item */
sb32 ifRemixAnnouncerPlayKO(FTStruct *victim_fp);          /* KO mode */
void ifRemixAnnouncerRequestSleep(void);                   /* sets sleep_flag */
```

   `IFRemixAnnouncerCall` is an enum with one value per table (`nIFRemixAnnouncerCallThunder`, `...Quick`, ...). Phase 2
   hooks call only these functions.

Deliverables: the two files, a notes file `docs/remix/notes/f9_announcer_core.md` (asm address of each ported routine,
what is left, the call site for the setup), and in your final report the exact list of enum values and functions.

## Phase 2 (workers after the core exists, one site group each)

Each worker replaces the `REMIX-TODO(PokemonAnnouncer)` lines (`grep -rn "REMIX-TODO(PokemonAnnouncer)" decomp/src`) and the
asm sites of its group with `if (ifRemixAnnouncerIsActive()) { ifRemixAnnouncerPlayCall(...); }`, keeping the vanilla
behaviour identical when the announcer is off. Groups (asm lines): A hit/throw/combo/sleep (295 to 590), B thunder/quick/
charging/sing/disable/teleport/burrow/dig/body slam (590 to 1054), C slash/flamethrower/charizard/firepunch/counter/rest/
pound/seismic/hung tough (1054 to 1490), D recovery/taunt/clang/stun/time up/team KO/air and ground specials (1490 to 2002),
E KO (`pokemon_ko_sounds_`, 129) and the Pokemon name (line 78, item `itManagerMakeItemKind`/`itRemixPokemonMakeMonster`).
