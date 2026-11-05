# F20: Tag Team, Stock Mode Last, Tug of War and Stamina in-match leftovers (M11 Fase 5)

Asm: `TagTeam.asm`, `StockMode.asm`, `TimedStock.asm`, `TugOfWar.asm`, `Stamina.asm` (`.toolchain/pkgs/smashremix-src/src/`).
Not run, not built: the lead validates. HUD text slots 14..18 are reserved in `port/remix/remix_hud_text.h` (nothing draws there yet).

## Heap with repeated switching (finding and fix)

The port has no N64 heap slots (`dynamic_css.heap_slot_*`, `alt_heap_pointer`, `always_load`, `load_all_files_`, `use_largest_anim_heap_`
only exist to fit the characters in 4 MB; `ftManagerSetupFilesAllKind` loads the files of a kind once per scene, so the files are bounded by
the number of distinct queue characters). The real growth was the animation (figatree) heap: every swap called
`ftManagerAllocFigatreeHeapKind` (a bump allocation in `syTaskmanMalloc`, never freed) and abandoned the old fighter's heap. Now:

- `gmRemixTagTeamGetFigatreeHeap` (gmremixtagteam.c) keeps one heap per port. A swap reuses it while its capacity covers the new
  character's `file_anim_size` (the asm passes `fp->anim_heap` on, `load_next_char_` 0x0094(sp)); otherwise it allocates one of
  `gFTManagerFigatreeHeapSize` (the biggest animation, as `use_largest_anim_heap_` does) and keeps that. At most one extra allocation per port per match.
- The swap now destroys the old fighter before making the new one, as the asm does (the new fighter takes the freed FTStruct, parts and heap).

## asm -> port

| asm | port |
|---|---|
| TagTeam `start_match_setup_` (random chars, random legal costume, team costume, shade) | `gmRemixTagTeamBuildQueues` (new, called from `scVSBattleStartBattle` before the fighters are made). New: random costume for Random slots in FFA (`mnRemixCostumeGetRandomLegal`), shade per slot in team battles (`gmRemixTagTeamRulesShade`, tested) |
| TagTeam `use_character_queues_on_load_` (first fighter from the queue, Stock Mode Last skip `global stocks - previous count`) | `gmRemixTagTeamApplyStartSlot`: for Last with a stored count, the match data (`players[].fkind/costume/shade`) gets the start slot before the loop that makes the fighters; `gmRemixTagTeamSetup` puts the CSS pick back after |
| TagTeam `load_next_char_` queue index (`initial - left - stolen`, plus `global - initial` for Last) | `gmRemixTagTeamGetSlotOfPort` with `gmRemixTagTeamRulesQueueIndex` / `...SkippedStocks` (tested). Uses the initial stock count of `StockMode.initial_stock_count_table`, so Manual counts now start at slot 0 (the old `stocks - left` did not) |
| `load_next_char_` swap when costume or shade differ (`_update_costume`) | `gmRemixTagTeamRulesNeedsSwap` now includes costume and shade: the fighter is rebuilt instead of repainted |
| `load_next_char_` damage series icon + `_redraw_icons` | `ifRemixTagTeamRefreshHud` (new `if/ifremixtagteamhud.c`): emblem of the damage HUD and a forced redraw of the stock icons |
| `load_next_char_` item/projectile owner remap (0x08) | `gmRemixTagTeamMoveOwnership` (already there) |
| StockMode `set_initial_stock_count_` -> `initial_stock_count_table` | `gmRemixStockModeBeginMatch` (replaces `gmRemixStockModeCount`; stores the initial count, Stamina only changes the fighter's count to 0 after storing, as `stamina_stock` runs later); `gmremixstockmoderules.c` (tested) |
| TimedStock `score_fix_` reads `initial_stock_count_table` | `gmRemixTimedStockScore` reads `gmRemixStockModeGetInitialCount` (before, it called the Last logic again at results time, which overwrote `initial_previous_stock_count_table` for the salty runback) |
| TugOfWar `credit_stock_to_koer_` writes `previous_stock_count_table` | `gmRemixTugOfWarGiveStock` calls `gmRemixStockModeSetLastCount` (the Last count now follows the credited stocks in Tug of War) |
| TugOfWar `update_sIFCommonBattlePlace_` | `ifRemixBattlePlaceRestoreRevived` (new `if/ifremixbattleplace.c`), the `extern sIFCommonBattlePlace` left `gm/` |
| TugOfWar `setup_` | `gmRemixTugOfWarInitFighters` + `gmRemixTugOfWarCollectFighterPorts` (the old function collected and wrote) |
| Stamina `stamina_fix_7` | `ftRemixStaminaRebirthSleeps` no longer writes `players[].stock_count = -1`: the asm fixes only the fighter struct. `stamina_ko_prevent(_front)` read the match data stock: `ftRemixStaminaIsOutOfStock` now reads `players[].stock_count` |

REMIX-TODO(stamina) list of T6.D5: the extra `players[].stock_count = -1` write (closed, removed), the `last_hit` index and the teammate
shield test (documented in `docs/bugs/remix_tugofwar_asm_deviations_2026-11-05.md`), `gmRemixTugOfWarSetupFighters` (split),
`sIFCommonBattlePlace` (accessor). `ftRemixStaminaStoredDamage` / `ftRemixStaminaRebirthSleeps` keep their names: their doc comments say what they store.

## Deviations (not ported, with the reason)

- Per-mode storage of the Stock Mode tables (`TagTeam.stockmode_table`, `previous_stock_count_table`, copied in and out by `before_css_setup_` /
  `leave_css_setup_`): the port has one set for all modes. CSS/menu scope.
- Skipped stocks below zero (stock setting lowered under the stored count): the asm indexes before the queue, the port starts at slot 0.
- `players[].fkind/costume/shade` keep the CSS pick after a swap (the asm overwrites the match struct); the results screen shows the CSS pick,
  and a runback starts from the same queue. Consumers of the last character would need a results hook.
- Blue Shell (0x180), Flashbang, Waddle Dee/Doo, Gordo (0x1C4) player pointers are not remapped on a swap (only `owner_gobj`); the new
  fighter reuses the freed FTStruct slot, so FTStruct pointers stay valid. `sw r0, 0x0ADC(a1)` (ammo field) not needed: the fighter struct is new.
- Explicit (menu) queue slots take costume 0, the asm takes the CSS choice (CSS worker).
- Classic Sonic in the queue (`is_classic`): Classic Sonic worker.
- Tug of War: last-hit index and teammate shield break (see the bugs doc), stock cap 127.

## Unit tests

`tests/remix/test_tagteam_rules.c` (Last skip, queue index with initial/stolen/skipped, shade, swap on costume/shade),
`tests/remix/test_stockmode_rules.c` (new, Last and Manual counts). Compiled alone with `cc`: 42 checks, 0 failed.

## In-game recipe for the lead

Tag Team (heap check): `SSB64_REMIX_MODE=tagteam SSB64_VS_STOCKS=4 SSB64_VS_FKINDS=<a>,<b>` and P1 walking off the stage
(`.claude/skills/remix-smoke-tests/inputs/walk_right.txt`, repeat lines or let the CPU kill P1). Log lines (`port_log`):
`tagteam: port N queue ... initial stocks X` at start and, per swap,
`tagteam: port N swap K: fkind A -> B costume C shade D, anim heap PTR cap SIZE, heap allocations M`. Pass: PTR stays the same across
swaps of a port (M grows only when a bigger character appears, at most once per port), no crash. Check the damage emblem and the stock
icons change with the character.
Stock Mode Last: add `SSB64_REMIX_PANEL="StockMode.1=Last"` (key format `<Key>.<port>=<value>`), play two matches in a row with a lower
`SSB64_VS_STOCKS` in the second (needs a CSS pass between, or the salty runback): the first fighter of the second match is the queue slot
`stocks - stored count`.
Tug of War: `SSB64_REMIX_MODE=tugofwar`; with Last, a credited KO raises the Last count (log `tugofwar: stock credit`).
Stamina: `SSB64_VS_STAMINA=100`; a star KO on the only stock must still end that fighter (sleep) and the damage HUD of a dead fighter
stays as the asm leaves it (match stock -2 after a star KO).
