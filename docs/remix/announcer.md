# Pokemon Stadium announcer and other announcer voices

Covers `PokemonAnnouncer.asm` (the commentator of Pokemon Stadium, Pokemon Stadium 2 and Gym Leader Castle, or of every stage) and the announcer voice lines the Remix adds elsewhere. Code: `decomp/src/if/ifremixannouncer.{c,h}` (pick modes, per-frame process), `ifremixannouncertables.c` and `ifremixannouncerrules.{c,h}` (the call tables, their shared-array layout, the STANDARD pick and the sleep and once-per-match rules; pure, unit tests in `tests/remix/test_announcer_rules.c`), `decomp/src/if/ifremixhitannouncer.{c,h}` (hit and throw buckets) and one `#ifdef REGION_REMIX` call at each trigger site listed in section 3. Setting: `Pokemon Announcer` (`announcer_mode`, `docs/remix/toggles.md`).
Test: `SSB64_REMIX_TOGGLES="Pokemon Announcer=1" SSB64_AUDIO_FGM_LOG=1 .claude/skills/remix-smoke-tests/scripts/vs.sh 0,8 .claude/skills/remix-smoke-tests/inputs/none.txt announcer 3000`; the log prints `[fgm] play id=<n>` for every sound, so the announcer clips show as ids 0x45B-0x4C1, 0x4AE, 0x5CF-0x5D3 and 0x608 (ids of section 2). `=1` makes the announcer speak on every stage; for the Stadium default put `SSB64_VS_STAGE=45` (Pokemon Stadium, 0x2D) or `44` (Gym Leader Castle, 0x2C) or `164` (Pokemon Stadium 2, 0xA4) in the environment. There are no unit tests; check with the FGM log.
Audit ledger (every item closed): `docs/remix/patch_audit.md`, groups g3, g5, g6 and g9.

## 1. Activation

`Pokemon Announcer` has three values: 0 STADIUM, 1 ALL STAGES, 2 OFF (defaults CE 0, TE 2, NE 0, JP 2). `toggle_announcer_` (asm) returns "active" for ALL STAGES, and for STADIUM only when the stage is Pokemon Stadium (0x2D), Gym Leader Castle (0x2C) or Pokemon Stadium 2 (0xA4). Port: `ifRemixAnnouncerIsActive` reads `gmRemixToggle(announcer_mode)` and compares `gSCManagerBattleState->gkind`. Every trigger site calls it first, so with the announcer off the vanilla path is unchanged.

`ifRemixAnnouncerSetup` (asm `announcer_setup_`, called from `Stages.asm:626`) runs once at battle start from `scVSBattleStartScene` (`sc/sccommon/scvsbattle.c:299`) and the 1P battle start (`sc/sc1pmode/sc1pgame.c:2649`). It does nothing when the announcer is not active; otherwise it clears the global and general timers and the sleep request, resets every entry's timer and `used` flag, and adds a GObj (`nGCCommonLinkIDInterface`) whose process is `ifRemixAnnouncerRun`.

## 2. Engine

**Call tables.** The asm declares 40 call tables (94 entries, 16 bytes each: id, FGM id, cooldown in frames, running timer, used flag, special flag). The port keeps them as `IFRemixAnnouncement` arrays in `IFRemixAnnouncerCall` order (`nIFRemixAnnouncerCallPokemon` ... `Taunt`). Cooldowns count down one per frame while the announcer runs.

The asm tables lie one after the other in memory and `announcer_main_` draws over the `a1` entries that follow the base address, so some draws run into the next table and share its timers and `used` flags. The port models this with `IFRemixAnnouncerTable` (`count` entries owned, which reset and count down once, and `pick_count` entries drawn over) on shared arrays: Knockback high (owns 12, draws 18, the 6 mid calls follow), Knockback mid, Burrow (owns 1, draws 3, the 2 burrow again calls follow), Burrow Again, Gym Leader (owns 1, draws 2, the first opening call follows) and Opening. Burn owns 2 entries but draws over 1 (`hit_announcement_ _fire` passes `a1 = 1`), so "Its got a Nasty Burn" (0x49F) never plays.

| Call | FGM ids | Cooldown (frames) | Flag |
|---|---|---|---|
| Pokemon (13 names, Onix to Mew) | 0x45C-0x468 | 900 | |
| Quick KO / Regular KO / Long KO | 0x46D-0x46E / 0x469-0x46C / 0x476-0x477 | once per match / 1200 / 600 | |
| General (8) / Close match | 0x499-0x49E, 0x4BE, 0x4BF / 0x4A0 | 14400 / once per match | |
| Sleep (3) / Sing | 0x47C, 0x47D, 0x47F / 0x47E | 1800 | |
| Combo | 0x480 | 900 | 1 |
| Knockback high (12) / mid (6) | 0x482-0x48D / 0x48E-0x493 | 900 | |
| Damage (4) / Damage early / Little damage / Burn (2) | 0x495-0x498 / 0x494 / 0x4A9 / 0x478, 0x49F | 900 / long / 1800 / 7200 | 1 except Damage |
| Thunder, Quick, Charge, Disable, Teleport, Fire Punch | 0x4A2, 0x4AB, 0x4A6, 0x4A4, 0x4A3, 0x4A5 | 1800 | 1 |
| Rest (2), Pound, Tough, Counter, Seismic Toss | 0x4AC-0x4AD, 0x4AA, 0x4A7, 0x4A8, 0x4AF | 1800 (Counter 1200) | 1 |
| Recovery, Clang, Stun (2), Taunt | 0x4B1, 0x4B2, 0x4B3-0x4B4, 0x608 | 1800 | 1 |
| Team KO (2), Slash, Flamethrower | 0x4B9-0x4BA, 0x4B8, 0x4B5 | long (0x8708) / 1800 | 1 |
| Gym Leader, Opening (3) | 0x4B6, 0x4B7, 0x4C0, 0x4C1 | long (once per match) | 1 |
| Attack (3, 5% roll) | 0x4BB-0x4BD | 900 | |
| Burrow / Burrow Again (2) / Dig / Body Slam | 0x5D1 / 0x5D3, 0x5D2 / 0x5D0 / 0x5CF | 1800 | 1 |
| Game set (replaces vanilla 0x1E8) | 0x45B THERE GOES THE BATTLE | | |
| Time running out (timer 0xE09) / Time up (replaces 0x20F) | 0x4A1 / 0x4AE | | |

**Pick modes (`announcer_main_`).** Nothing plays while the global timer (0x78 = 120 frames, set by every successful clip so the announcer does not talk over himself) is non-zero.
- NAME (`ifRemixAnnouncerPlayName`): the Pokemon clip for item kind `kind - 0x20`, silent while that clip cools down.
- KO (`ifRemixAnnouncerPlayKO`): under 0x4B0 (1200) elapsed frames a quick KO (falls back to a regular KO once the clip was used), above 150% damage a long KO (regular KO while it cools down), otherwise a regular KO. The regular KO is one uniform draw of 4: the asm retry branch falls through, so its last draw always wins.
- GENERAL (per-frame): the close-match clip once per match when the stock rule is on and every port is down to its last stock, otherwise a free general clip.
- STANDARD (`ifRemixAnnouncerPlayCall`, `ifRemixAnnouncerRulesPickStandard`): up to `pick_count` random draws; the first entry with no cooldown plays; if all are cooling down the last draw plays unless its special flag is set. Clang is also gated by its `used` flag: it plays at most once per match (`clang_announcement_` tests the byte before calling `announcer_main_`).

**Per-frame process (`ifRemixAnnouncerRun`, asm `announcer_constant_`).** Counts the global timer down, plays a pending sleep clip (only on frames where the global timer was running before the countdown, and that frame ends there), counts every entry timer down, plays 0x4A1 when `time_remain` is 0xE09 (about one minute left), plays the opening clip at 30 elapsed frames (the Gym Leader Castle clip on that stage), and runs the general timer: a general clip is tried after 0x960 (2400) frames with a 1 in 2400 roll or at 0x12C0 (4800) frames, and a played clip restarts the timer. The sleep clip comes from `ifRemixAnnouncerRequestSleep`, set when a fighter falls asleep.

**Replaced sounds.** `ifRemixAnnouncerGameSetFgm` swaps the GAME SET voice (`nSYAudioVoiceAnnounceGameSet`) for 0x45B and `ifRemixAnnouncerGameSetDelay` lengthens the end delay from 90 to 128 frames (`if/ifcommon.c:3831`, `:3833`). `ifRemixAnnouncerTimeUpFgm` turns the TIME UP voice 0x20F of the battle end queue into 0x4AE (`if/ifcommon.c:3041`).

## 3. Triggers

Asm line = `PokemonAnnouncer.asm` label; every site is a short `if (ifRemixAnnouncerIsActive()) ifRemixAnnouncerPlayCall(...)` unless noted. Decomp paths are relative to `decomp/src`.

| Call | Trigger in the asm | Port site |
|---|---|---|
| Pokemon (name) | `pokemon_name_` 78: a Pokemon item is made | `itManagerMakeItemKind` (`it/itmanager.c:874`, `ifRemixAnnouncerPlayName`) |
| KO | `pokemon_ko_sounds_` 129, called from `Stamina.asm:908` in `ftCommonDeadUpdateScore` | `ft/ftcommon/ftcommondead.c:73` (before the score update, no Stamina check, as in the asm) |
| Sleep | `sleep_announcement_` 261 | `ftCommonFuraSleepSetStatus` (`ft/ftcommon/ftcommonfurasleep.c:42`, request) |
| Combo | `pokemon_combo_` 295, from the combo meter at 10 hits | `ifRemixComboHitsAnnounceTenHits` (`if/ifremixcombohits.c:29`); the meter also plays FGM 466 INCREDIBLE at 20 hits |
| Knockback, damage, burn | `hit_announcement_` 350: knockback `>= 115` enters the knockback path (`>= 155` high, else mid); below it damage 0 is silent, fire with damage is Burn, damage under 4 is Little, under 18 silent, else Damage Early before 0x360 (864) elapsed frames and Damage afterwards | `ifRemixHitAnnouncerPlayHit` (`ft/ftcommon/ftcommondamage.c:745`, reads `damage_knockback`, `damage_queue`, `damage_element`) |
| Same for throws | `throw_hit_announcement_` 486: thresholds 120 and 175, no fire or early buckets | `ifRemixHitAnnouncerPlayThrow` (`ft/ftcommon/ftcommonthrown2.c:228`) |
| Thunder | `thunder_g_/a_announcement_` 590, Pikachu down special | `ft/ftchar/ftpikachu/ftpikachuspeciallw.c:114`, `:128` |
| Quick | `quick_g_/a_announcement_` 645, Pikachu up special | `ftpikachuspecialhi.c:111`, `:126` |
| Charge | `charging_announcement_` 697, Samus charge shot | `wp/wpsamus/wpsamuschargeshot.c:287` |
| Sing | `sing_announcement_` 748 | `efManagerPurinSingMakeEffect` (`ef/efmanager.c:4914`) |
| Disable / Teleport | 799 / 843, Mewtwo down special; Mewtwo and Super Sonic up special | `ft/ftremix/mewtwo/ftremixmewtwodsp.c:48`, `:63`; `ftremixmewtwousp.c:210`; `ft/ftremix/ssonic/ftremixssonicusp.c:228` |
| Burrow / Dig / Body Slam | 888 / 963 / 1008, Crash | `ft/ftremix/crash/ftremixcrashdsp.c:316`, `:404`; `ftremixcrashusp.c:118` |
| Slash | 1054, Wolf up special | `ft/ftremix/wolf/ftremixwolf.c:505` |
| Flamethrower | 1148 `fireflower_`, 1163 `charizard_` (Bowser's breath calls the Fire Flower function) | `it/itcommon/itfflower.c:333`; `it/itmonster/itlizardon.c:438` |
| Counter | 1219, Marth down special | `ft/ftremix/marth/ftremixmarth.c:569` |
| Rest / Pound | 1259 / 1314, Jigglypuff | `ft/ftchar/ftpurin/ftpurinspeciallw.c:52`, `:65`; `ftpurinspecialn.c:123`, `:137` |
| Seismic Toss | 1370, forward throw of Kirby, Jigglypuff Kirby, Bowser and Giga Bowser (`jigglypuffkirbyshared.asm:297`) | `ft/ftcommon/ftcommonthrow.c:123`, `:138` (NKirby is skipped, as the asm hook leaves it to the vanilla check) |
| Tough | 1430, hit sound 0x267 | `ftPublicPlayCommon` (`ft/ftpublic.c:178`) |
| Recovery | 1490 heal, 1539 absorb (Psi Magnet) | `ftParamSetHealDamage` (`ft/ftparam.c:1879`); `ft/ftmain.c:2673` |
| Taunt | 1591 | `ft/ftcommon/ftcommonappeal.c:31` |
| Clang | 1640, contact type 3 | `ftMainSetHitInteractStats` (`ft/ftmain.c:2273`) |
| Stun | 1695 | `ft/ftcommon/ftcommonfurafura.c:42` |
| Team KO | 1779, team stock branch | `ft/ftcommon/ftcommonsleep.c:70` |
| Attack | 1856 / 1929, a special move press with a 1 in 20 roll | `ft/ftcommon/ftcommonspecialair.c:205`; `ftcommonspecialn.c:150` |
| Game set, time up | 18 / 1740 | section 2 |

## 4. Other announcer voices

| Feature | Port |
|---|---|
| Pokemon SFX (`pokemon_voices`): Japanese Pokemon voices on the item sites | `it/itremix/itremixpokemonvoices.c`; setting in `docs/remix/toggles.md` section 6 |
| COMBO INCREDIBLE (FGM 466) | `if/ifremixcombohits.c:84` |
| Character-select announcer names and mode lines (`FGM.announcer.names`, `.css`, `.team`, `.results`, `.singleplayer`, `.fight`) | data of the roster and of the CSS and mode screens, not part of this module |

## 5. Deviations

- The asm plays the announcer clip through `announcer_main_`'s FGM return and, when the call fails, falls back to a vanilla FGM held in `v1` at several sites (Pikachu, Samus, Jigglypuff, Hung Tough). The port plays the clip inside `ifRemixAnnouncerPlayEntry` and has no fallback sound; a failed pick is silent.
- Standard and general picks make one attempt per drawn entry (`pick_count`); the asm general loop retries until a free entry exists, which can hang when all are cooling down, so the port stops after `pick_count` attempts and stays silent.
- Burrow: the asm always takes the repeat branch (`a2 = 1<<port` is never 0), so every burrow draws over 3 entries from `burrow_call` (the burrow call and the two burrow again calls that follow it) and the history byte (`dig_match_history`, cleared by `refresh_burrow_binary_`) is dead. The port does the same through the shared `sBurrowCalls` array, with no history byte (`ifremixannouncertables.c`, called from `ftremixcrashdsp.c:316`). A burrow therefore plays "It Burrowed Underground", "The Underground attack continues" or "It Burrowed Underground Again" with equal odds, and stays silent when the three draws all hit a cooling clip (special flag set).
- Knockback high: as in the asm, `announcer_main_` draws among 0x12 (18) entries, the 12 high calls and the 6 mid calls that follow them in memory (same in throws), so a high hit can play a mid clip and the shared timers cool both calls (`sKnockbackCalls`).
- Clang: as in the asm, it plays at most once per match (`used` flag, `IFRemixAnnouncerTable.plays_once`, `ifRemixAnnouncerRulesIsSpent`); the 1800-frame cooldown only matters before the first clip.
- Sleep: as in the asm, the pending request is read only on frames where the global timer was non-zero before it counted down (`ifRemixAnnouncerRulesChecksSleep`), and `announcer_main_` refuses while the timer is non-zero, so the sleep clip can only play on the frame the global timer reaches 0 after another clip; a request made while the timer is 0 stays pending until some other clip starts and its 120 frames run out. Calls made during the sleep frame end the frame there, as in the asm.
- Burn: the asm draws over 1 entry, so only "It Suffered a Burn" (0x478) plays and 0x49F is unreachable; the port keeps both entries and draws over 1.
- Close match: the asm tests `Global.vs.game_mode < 2`; the port tests the stock bit (0x2) of `game_rules`. Elapsed and remaining times map to `time_passed` and `time_remain` of `SCBattleState`.
- The Gym Leader Castle opening draws `random(2)` over a one-entry table in the asm (index 1 reads the first opening call, 0x4B7); the port does the same through the shared `sGymOpeningCalls` array, so the Gym Leader Castle plays 0x4B6 or 0x4B7 with equal odds.
- `opening_comment` (random FGM 0x479-0x47B on the "2" of the countdown, `Hazards.asm:6271`) is played from the countdown (`if/ifremixcountdown.c`); the Captain Falcon `firepunch_announcement_` (`captainshared.asm:207`) plays from `efManagerCaptainFalconPunchMakeEffect` for every punch power except Ganondorf, Wolf and Mewtwo (`ftremixdkcaptainshared.c`, SH-8).
- `IFREMIX_ANNOUNCER_TIME_UP_FGM` is defined twice in `if/ifremixannouncer.c` (0x4A1 near the top for the one-minute call, 0x04AE later for the time-up swap); each use sees the value defined before it.
