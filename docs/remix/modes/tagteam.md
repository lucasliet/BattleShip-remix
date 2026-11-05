# Tag Team

Remix VS mode 2 (`MNREMIX_VSMODE_TAG_TEAM`, VS menu page 2; the asm code calls it "squad strike"): each port plays a queue of characters, one per stock.
Asm source: `TagTeam.asm` (in `.toolchain/pkgs/smashremix-src/src`), with `StockMode.asm`, `TimedStock.asm` and the CSS hooks of `VsRemixMenu.asm` /
`CharacterSelect.asm` / `TwelveCharBattle.asm`.
Port code: `decomp/src/gm/gmremixtagteam.c` (queues, swap, steal), `gmremixtagteamicons.c` (icon cache), `gmremixtagteamrules.c` (pure rules),
`decomp/src/if/ifremixtagteamhud.c`, `decomp/src/mn/mnplayers/mnremixtagteamcss.c`, `mnremixtagteamcssicons.c`, `mnremixtagteamcssrules.c` (CSS queue),
`port/remix/remix_modes.cpp` (port-menu queue slots). Tests: `tests/remix/test_tagteam_rules.c`, `test_stockmode_rules.c`, `test_css_vsmodes_rules.c`;
`cmake --build build --target run_remix_mode_tests`. Shared plumbing (Stock Mode, Timed Stock): [`../modes_base.md`](../modes_base.md).

## How the Remix does it

### Rules

- The queue is built on the VS CSS: every port has up to 6 slots, and the slots in use are the stock count (the asm count is 0-based: with N the queue
  has N + 1 slots, slot 0 first). When a fighter loses a stock the next character of its queue takes its place (`load_next_char_`). Team battles add
  team costumes, shades and stock stealing (`handle_stock_stealing_`: a port that takes a stock from a teammate plays the character of that stock).
- `before_css_setup_` forces `Global.vs.game_mode = 3` (Timed Stock; comments in the asm say "TIMED STOCK"), the saved stocks and teams, the title image
  `0x2738` and the Stock Mode tables. The results use the stock score path of `TimedStock.asm` `score_fix_` (Tug of War is the only special case there).

### CSS (`TagTeam.asm` `css` scope)

- `setup_` creates 6 stock icon objects per port above the panel (file `0x0019`); `slot_icon_update_routine_` hides icons at or above the stock count and
  for N/A ports, draws the cached icon of a filled slot and a default dot otherwise (Master Hand gets a smaller scale).
- `update_queue_` hooks (token select, costume change, fighter sync and the four toggles FFA / teams / team change) call `update_slot_`, which writes
  character, costume and shade into `character_queues[port][current_slot]` (Classic Sonic flag included) and caches the stock icon texture and
  palettes; `update_previous_icons_` shifts earlier slots (costume ids away from duplicates in FFA, shades up for teammates with the same fighter).
- `move_to_next_slot_on_pickup`: taking the token (not recalling it) goes to the next slot, clamped to the stock count; the pickup wait is 2 frames
  instead of 30 (`speed_up_token_pickup_`). B with a held token empties the slot and goes back one (`clear_slot_and_go_to_previous_slot_`).
  A over a portrait recalls (`check_recall_with_a_`). A stock count change clamps `current_slot` and clears the slots above it.
- `prevent_start_with_empty_slots_`: START is refused unless the last slot of the queue (index = stock count) of every non-N/A port holds a character.
- `start_match_setup_` resolves Random slots to a real character and a random legal costume (`Costumes.get_random_legal_costume_`); in team battles the
  costume is the team costume and the shade goes up by one for each earlier port with the same character in the same slot on the same team. The
  Random pick uses a recent-randoms window per port (`_recent_randoms_lookup`), it does not strictly avoid repeats.
- Announcer FGM 783 on entry (`FGM.announcer.css.TAG_TEAM`).

### Match

- `use_character_queues_on_load_`: copies slot 0 of each port into the match block (Stock Mode Last starts at the slot `global stocks - previous
  count`), clears the stolen state, sets Sonic's classic flag, sets up 4 custom N64 heaps and preloads the files of every queue character.
- `load_next_char_` (hook in `ftCommonRebirthDownSetStatus`): queue index = `initial stock count - stocks left - stolen count` plus the Last skip; a
  stolen pointer replaces the arithmetic. If the character, costume, shade or classic flag changes, the old fighter is destroyed and a new one is made
  with the old one's team, port, stock count and animation heap; the damage series icon is changed and items and projectiles owned by the old fighter
  are re-pointed to the new one (owner `0x08`, Blue Shell `0x180`, Flashbang / Waddle / Gordo `0x1C4`). A change of costume or shade of the same
  character only repaints.
- `show_all_icons_`: the HUD lists the queue in reverse, the rightmost icon is the current fighter; the icon at the current position uses the loaded
  fighter's icon or the stolen icon; the rest use the icon cache.
- N64 heap code (`load_all_files_`, `load_particle_bank_stuff_in_normal_heap`, `use_largest_anim_heap_`) exists only to fit the characters in memory.

## How the port does it

| Asm | Port |
|---|---|
| mode profile (Timed Stock, 3 stocks, teams saved) | `gmremixmodecss.c`; title `0x2738` and FGM 783 in `mnremixcssmoderules.c` |
| queue write, pickup, B, recall, stock change, start gate (`css` scope) | `mnRemixTagTeamCssWrite`, `NextSlot`, `StepBack`, `StocksChanged`, `AllowsStart`, `GetSlot` (`mnremixtagteamcss.c`; pure rules `mnremixtagteamcssrules.c`) called from `mnplayersvs.c`; pickup wait 2 frames in the token grab code |
| CSS icon rows | `mnRemixTagTeamCssIconsMake` (`mnremixtagteamcssicons.c`): icons at panel x + 5, 10 apart, y 117, the CSS derives the costume icon at draw time |
| `start_match_setup_`, `use_character_queues_on_load_` | `gmRemixTagTeamBuildQueues` (called before the fighters are made in `scVSBattleStartBattle`): slot 0 from the match data (or the first CSS pick), slots 1-5 from the CSS queue, else the port-menu CVar slot, else Random; `gmRemixTagTeamApplyStartSlot` puts the Stock Mode Last start slot in the match data and `gmRemixTagTeamSetup` puts the CSS pick back once the fighters exist |
| queue index | `gmRemixTagTeamGetSlotOfPort` with `gmRemixTagTeamRulesQueueIndex` / `...SkippedStocks` over `gmRemixStockModeGetInitialCount` |
| `load_next_char_` | `gmRemixTagTeamLoadNextFighter` (called first thing in `ftCommonRebirthDownSetStatus`, `ftcommonrebirth.c`): `gmRemixTagTeamRulesNeedsSwap`, `ftManagerDestroyFighter` then `ftManagerMakeFighter`, `gmRemixTagTeamMoveOwnership` (item and weapon `owner_gobj`), `ifRemixTagTeamRefreshHud` (damage series emblem and stock icon redraw) |
| `show_all_icons_` | `gmRemixTagTeamApplyStockIcon` (called from `ifcommon.c` stock icon setup) with `gmRemixTagTeamRulesIconSlot`; icons are resolved at match start by `gmRemixTagTeamIconsSet` (`gmremixtagteamicons.c`, only the main file of a fighter is read) |
| `handle_stock_stealing_` | `gmRemixTagTeamStealStock` (called from `ifcommon.c` stock steal) |
| heaps | `gmRemixTagTeamGetFigatreeHeap`: one animation heap per port, reused while its capacity covers the new character's `file_anim_size`, else one of `gFTManagerFigatreeHeapSize` (the largest animation, as `use_largest_anim_heap_`); at most one extra allocation per port per match |
| Stock Mode | `gmRemixStockModeBeginMatch` stores `initial_stock_count_table`; Last skip in the queue index; see `modes_base.md` |

The queue slots 1 to 5 can also be set in the port menu (Remix > Tag Team, CVars read through `remix_modes_tagteam_slot`, `Random` by default); the CSS
queue wins and a Random token of the CSS rolls like a Random slot.

### Deviations from the asm

- **N64 memory.** No custom heaps, file preload or heap slots: `ftManagerSetupFilesAllKind` loads the files of a kind once per scene. The real growth
  was the animation heap, handled as above. The swap destroys the old fighter before making the new one, as the asm does, so the new fighter takes the
  freed fighter struct, parts and heap.
- **Swap on costume or shade.** The asm repaints the fighter; the port rebuilds it (`gmRemixTagTeamRulesNeedsSwap` includes costume, shade and the
  Classic Sonic flag).
- **Match data.** `players[].fkind/costume/shade` keep the CSS pick after a swap (the asm overwrites them), so the results show the CSS pick and a
  runback starts from the same queue.
- **Remapped pointers.** Only `owner_gobj` of items and weapons is moved; the Blue Shell (`0x180`) and Flashbang / Waddle Dee / Waddle Doo / Gordo
  (`0x1C4`) player pointers are not, since the new fighter reuses the freed fighter struct slot. The `sw r0, 0x0ADC(a1)` ammo reset is not needed (new struct).
- **Stock Mode tables.** One set of Stock Mode tables for all modes; the asm copies `stockmode_table`, `stock_count_table` and
  `previous_stock_count_table` in and out per mode.
- **Skipped stocks below zero.** If the stock setting is lowered under the stored Last count the asm indexes before the queue; the port starts at slot 0.
- **Random.** The Random slot picks a uniform fighter of a 30 entry list of the Remix CSS (`gmRemixTagTeamRulesRandomFighter`), without the recent-randoms window.
- **Costumes of menu slots.** A fixed slot from the port menu uses costume 0 (team costume in team battles), a Random slot a random legal costume in free-for-all; a CSS pick keeps its costume.
- **Start gate.** The asm comment says "up to stock count"; both check the last slot only, which the port implements.

Open gaps: `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

- Shortcuts: `SSB64_REMIX_MODE=tagteam`, `SSB64_VS_STOCKS=<n>`, `SSB64_VS_FKINDS`, `SSB64_REMIX_PANEL="StockMode.1=Last"`.
- Swap and heap: `SSB64_REMIX_MODE=tagteam SSB64_VS_STOCKS=4 .claude/skills/remix-smoke-tests/scripts/vs.sh <a>,<b> .claude/skills/remix-smoke-tests/inputs/walk_right.txt tt_swap 2200`
  (P1 walks off the stage, or let the CPU kill it). Log lines: `tagteam: port N queue ... initial stocks X` at the start and, per swap,
  `tagteam: port N swap K: fkind A -> B costume C shade D, anim heap PTR cap SIZE, heap allocations M`. Pass: PTR stays the same across the swaps of a
  port (M grows only when a bigger character appears, at most once per port), no crash; the damage emblem and the stock icons change with the character.
- Stock Mode Last: play two matches in a row with a lower stock setting in the second (a CSS pass between, or the salty runback); the first fighter of
  the second match is queue slot `stocks - stored count`.
- CSS: `css.sh` with `SSB64_REMIX_MODE=tagteam`: six icons above each panel for the stock count, the slot fills on a pick, B empties it, START is refused
  with an empty last slot.
