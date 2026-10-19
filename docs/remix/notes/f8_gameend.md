# F8 notes: GameEnd (end of a VS match) and Always Show Full Results

Scope: `.toolchain/pkgs/smashremix-src/src/GameEnd.asm` (162 lines), `TimedStock.asm` lines 210-237 (`show_full_results_`).

## Asm addresses

| Patch | ROM | RAM (hook) | Remix label |
|-------|-----|------------|-------------|
| `GameEnd.update_screen_` | 0x0010B204 | 0x8018E314 | `GameEnd.update_screen_` at 0x8048AFC4 (remix.sym) |
| `TimedStock.show_full_results_` | 0x156884 | 0x801376E4 | not in remix.sym by name |
| `GameEnd.is_salty_runback` (dw) | n/a | n/a | not in remix.sym; the old literal 0x8048B174 was unverified and is removed |

## Vanilla functions hooked

- `update_screen_` runs where the VS battle sets the next scene. The C equivalent is the tail of `scVSBattleStartScene`
  (`decomp/src/sc/sccommon/scvsbattle.c`, after `scene_curr = nSCKindVSResults`).
  `build/remixasm/refs/vanilla_funcs.txt` maps 0x8018E314 to `sc1PTrainingModeMakeDamageDisplay`
  (`sc/sc1pmode/sc1ptrainingmode.c:968`). That is an overlay collision (training overlay code shares the RAM range), so it is not used.
- `show_full_results_` decides the KO/TKO layout of the results screen. The C equivalent is the draw-proc index in
  `mnVSResultsMakeLabel` (`decomp/src/mn/mnvsmode/mnvsresults.c`, `procs[sMNVSResultsKind]`).
  `vanilla_funcs.txt` maps 0x801376E4 to `mnPlayersVSCheckCursorPuckGrab` (overlay collision), so it is not used.

## Patches

| Patch | Status | Where |
|-------|--------|-------|
| update_screen_: salty runback restart (VS battle, not results) | applied | `gmRemixGameEndSelectScene` |
| update_screen_: skip results (0 off, 1 on, 2 on reset via `is_reset`) | applied | `gmRemixGameEndSelectScene` |
| update_screen_: R or L held shows results, cancels runback | applied | `gmRemixGameEndSelectScene` |
| update_screen_: Great Bay FGM 0x566 on runback | applied | `gmRemixGameEndStartSaltyRunback` |
| update_screen_: `is_salty_runback` flag | applied as C static | `gmRemixGameEndIsSaltyRunback` (read by grremixstage) |
| update_screen_: 12 Character Battle check | REMIX-TODO(TwelveCharBattle) | no port reader for `TwelveCharBattle.twelve_cb_flag` |
| update_screen_: pause D-pad song cycle timer reset | skipped | Pause state of the Remix pause menu, no port |
| update_screen_: StockMode previous stock counts reset | REMIX-TODO(StockMode) | no port state |
| update_screen_: custom heap reset | REMIX-TODO(heap) | Remix custom heap, no port equivalent |
| show_full_results_: full layout for stock kinds | applied | `gmRemixGameEndResultsDrawKind` (stock royal to time royal, stock team to time team) |
| Preserve Song (BGM.asm 208, 446) | already applied | `gr/grremix/grremixrandommusic.c`, not part of GameEnd.asm |

The vanilla Skip Results enhancement (`port_enhancement_skip_results_screen`, CVar `gEnhancements.SkipResultsScreen`) is
replaced by the Remix toggle only in `REGION_REMIX` builds. The non-Remix path is unchanged.

## Toggles

| Title (Toggles.asm) | Entry | Values |
|---------------------|-------|--------|
| Skip Results Screen | `entry_skip_results_screen` | 0 OFF, 1 ON, 2 ON RESET |
| Salty Runback | `entry_salty_runback` | 0 OFF, 1 default mask (A B Z R START), 2 alt mask (A B Z R D-Pad Left) |
| Salty Runback Preserves Song | `entry_preserve_salty_song` | bool |
| Always Show Full Results | `entry_full_results` | bool |

Test one toggle at a time, for example `SSB64_REMIX_TOGGLES="Skip Results Screen=1"`,
`"Salty Runback=1"`, `"Always Show Full Results=1"`.

## What to see in a short match

- Skip Results Screen = 1: when the last stock is lost, the game goes to character select without results.
  Skip Results Screen = 2: it skips only after a reset (A+B+R+Z); a normal end shows the results.
- Salty Runback = 1: hold Start (A B Z R Start) when the match ends. The next match starts at once with the same stage
  and no results. On Great Bay a song plays (FGM 0x566).
- Salty Runback = 2: same, with D-Pad Left instead of Start.
- Holding R or L at the end always shows the results, even with the toggles on.
- Always Show Full Results = 1: stock matches show the KO and TKO layout of the time mode (KOs and TKOs on the results screen).
  Check that the labels of the layout are right, since only the draw procedure changes.
- Salty Runback Preserves Song = 1 with Random Music on: the runback replays the same song.
- Repeat the salty runback 5 or more times: the skipped custom heap and stock count resets could show as a memory growth or
  a stock count error.

## Uncertainties

- `Joypad.check_buttons_` for the held type is taken as `button_hold` of each port (`gSYControllerDevices`). Assumed, not verified in the port.
- `Joypad.DL` is taken as `L_JPAD` (`CONT_LEFT`).
- The 12 Character Battle mode has no port reader. Confirm that it cannot be reached in the port, or port the flag before enabling the runback.
- The full results layout reuses the time draw procedures. The asm only changed the branch, so time-only labels may show.
- `remix_symbols.h` has no `GameEnd.is_salty_runback` symbol. The port now uses the C variable, so no symbol is needed.
