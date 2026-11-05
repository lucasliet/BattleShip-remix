# Common base of the Remix VS modes

What the mode documents ([`modes/`](modes/)) share: how the mode of a match is chosen and forced, the per-mode CSS profile, the Timed Stock rule, Stock Mode,
and the positioned HUD text. Code: `decomp/src/gm/gmremixmode.c`, `gmremixmodecss.c`, `gmremixtimedstock.c`, `gmremixstockmode.c`, `mn/mnvsmode/mnremixvsmode.c`,
`port/remix/remix_hud_text.*`, `remix_font.*`, `remix_debug.h`. Tests: `tests/remix/test_timedstock_rules.c`, `test_stockmode_rules.c`, `test_css_vsmodes_rules.c`
(`cmake --build build --target run_remix_mode_tests`). Mode documents: [koth](modes/koth.md), [smashketball](modes/smashketball.md), [stamina](modes/stamina.md),
[tugofwar](modes/tugofwar.md), [twelvecb](modes/twelvecb.md), [tagteam](modes/tagteam.md), [training](modes/training.md), [singleplayer](modes/singleplayer.md).

## Match mode

- The choice of the Remix Modes page of the VS menu is `gMNVSModeRemixMode` (`MNREMIX_VSMODE_*`: default 0, 12CB 1, Tag Team 2, KOTH 3, Smashketball 4, Tug of War 5;
  `mn/mnvsmode/mnremixvsmode.h`). Match and results code reads `gmRemixModeGet()` / `gmRemixModeIs()` (`decomp/src/gm/gmremixmode.h`); the variable is reset to default each time the VS
  menu opens. In the asm this is `VsRemixMenu.vs_mode_flag` and the `mode_setup_table`, `mode_setup_returning_table` and `mode_setup_start_table` dispatch tables.
- `SSB64_REMIX_MODE=default|12cb|tagteam|koth|smashketball|tugofwar` forces the mode for the whole run (tests without the menu). Test shortcuts of the modes (`SSB64_*`) are read through
  `remix_debug_env_int` / `remix_debug_env_text` (`port/remix/remix_debug.h`), never with `getenv` directly.
- `scVSBattleStartBattle` writes `remix mode: mode=N rules=N time_limit=N stocks=N team=N` to the log at the start of each match (`gmRemixModeLogMatch`).
- `gmRemixModeSceneClear()` (called from `scManagerRunLoop` on every scene change) clears the HUD labels and the mode sprites.

### CSS profile (`gmremixmodecss.c`)

Each mode forces its rule and settings when the VS CSS opens (`gmRemixModeBeforeCSS`, asm `before_css_setup_`), keeps its own stocks, goal and teams between visits, and gives the vanilla
settings back when the CSS returns to the VS menu (`gmRemixModeLeaveCSS`, `leave_css_setup_`); `gmRemixModeStartMatch` is `start_match_setup_` (Smashketball sets the stage here; modes with a goal get
the VS menu time limit back as the match time limit).

| Mode | Rule | Stocks | Goal | Teams | Stage select |
|---|---|---|---|---|---|
| 12CB | Stock | 3 | - | off | normal |
| Tag Team | Timed Stock | 3 | - | saved | normal |
| KOTH | Time | - | 30 (seconds) | saved | normal |
| Smashketball | Time | - | 10 (score) | on | off |
| Tug of War | Timed Stock | 3 | - | saved | normal |

The title image and the announcer of the CSS are in `mn/mnplayers/mnremixcssmoderules.c` (header offsets 0x2048, 0x2738, 0x29A8, 0x2C18 / 0x2E88, 0x30F0; FGM 1016, 783, 775, 776, 479).

## Timed Stock (`TimedStock.asm`)

- The Stock rule of the VS menu stores `game_rules = SCBATTLE_GAMERULE_TIMED_STOCK` (time | stock, value 3, `mnVSModeSaveSettings`) for free-for-all and team battles. The time limit is the one of the
  state (`time_limit`, chosen with the Time rule); `SCBATTLE_TIMELIMIT_INFINITE` (100) does not start the timer. The pure stock rules of the port (competitive ruleset, Classic Co-op) keep `game_rules == 2`.
- The HUD timer and the end by time test the TIME bit; stock tests the STOCK bit. The places that compared `== TIME` were adjusted: the single stock icon (`mnPlayersVSSetSceneData`, `netreplay.c`) and the results.
- Results: `mnVSResultsInitRankings` uses the stock places when the stock ended before the timer and the time score when the timer ran out (`gmRemixTimedStockUsesTimeScore`, asm `pick_scoring_`).
  `mnVSResultsGetPointsDirect` returns `((initial stocks - falls + 1) << 12) - final damage` (`gmRemixTimedStockScore`, asm `score_fix_`); with no stock left the score is 0. Tug of War uses
  `((stock count + 1) << 24) + ((KOs - falls) << 12) - damage`, KOTH the frames on the hill, Smashketball the winner scores (see the mode documents).
- No sudden death in Timed Stock (`gmRemixTimedStockAllowsSuddenDeath`, asm `prevent_sudden_death_`); only the Time rule has it. KOTH never has it, Smashketball has it on a tie (its own rule, in `scvsbattle.c`).
- `show_full_results_`: the full-results toggle swaps the results draw routine (`gmRemixGameEndResultsDrawKind`).
- Test: `SSB64_VS_STOCKS=3 SSB64_VS_TIME=1 .claude/skills/remix-smoke-tests/scripts/vs.sh 0,8,6 .claude/skills/remix-smoke-tests/inputs/none.txt <name> 5800` (one minute, 3 stocks: `SSB64_VS_STOCKS`
  stores rule 3 under `REGION_REMIX`, `SSB64_VS_TIME` stores the limit in minutes). The timer starts at 01:00, TIME UP appears, and on the results a fighter that took damage ranks behind.

## Stock Mode (`StockMode.asm`)

The StockMode panel option of each port (Default, Last, Manual; `gmRemixPanel(STOCK_MODE, port)`) decides the stock count of a fighter at match start. `gmRemixStockModeBeginMatch(player, match_stocks)` is
called once per port and match from `scVSBattleStartBattle` (asm `set_initial_stock_count_`): it stores `initial_stock_count_table` for every Stock Mode, and returns the count the fighter starts with.
Order: the 12-Character Battle pool (`gmRemixTwelveCBGetStocks`), the Last count, the Manual count (never above the match count), or the match count; Stamina then changes the fighter's count to 0
after the initial count is stored, as `stamina_stock` runs later.

- **Last.** `previous_stock_count_table` (`sStockModeLastCount`) is stored the first time Last is used after a CSS reset (the match count) and reused by later matches, lowered to the match count
  (`gmRemixStockModeRulesLastCount`) except in Tug of War, where the stored count is used as is. `initial_previous_stock_count_table` is written on every Last match start (the value before the match).
  `gmRemixStockModeResetLast` clears the stored counts when the CSS Stock option resets (`mnplayersvs.c`, `css/StockMode.asm` `_check_reset`); `gmRemixStockModeSaltyRunbackLast` copies the initial values back for a salty
  runback (`gmRemixGameEndStartSaltyRunback`). Tug of War writes the stored count when it credits a stock (`gmRemixStockModeSetLastCount`). Tag Team reads it to skip queue slots.
  In the asm the non-12CB path of `update_stocks_remaining_` (`TwelveCharBattle.asm`) also stores the new count at every KO, so Last follows lost stocks.
- **Manual.** The count of the panel (`STOCK_COUNT`), capped by the match count (`gmRemixStockModeRulesManualCount`).
- The CSS counter display (`CharacterSelect.asm`) only reads the stored table; it is not a separate source.
- Tests: `test_stockmode_rules.c` (Last and Manual counts).

## Positioned HUD text (`port/remix/remix_hud_text.h`)

The Remix draws scores, counters and legends with its own font, Kabel (`File.CUSTOM_FONT` 0xA15), through `Render.draw_string_`. The port rebuilds that font at run time from the user's own .o2r
(`port/remix/remix_font.cpp`; the glyph sprites come from the extracted file, the advance widths from the Remix RAM image `Render.character_offsets_custom`) and draws it in the ImGui overlay, falling back to the
ImGui font only when the font file is missing. A caller puts a label every frame with the arguments of the asm call:
`remix_hud_text_put(slot, text, x, y, 0xRRGGBBAA, scale, align)` (also `_put_fit` for `Render.apply_max_width_`, `_putf` with printf format, `remix_hud_bar_put` for the KOTH meters and
`remix_hud_rect_put` for `Render.draw_rectangle`), in N64 screen coordinates (320 x 240), `y` being the top of the 16 pixel glyph box. A label lives two frames, so a hook that stops calling it hides it;
there are 128 text slots, 16 bar slots and 48 rectangle slots, and each owner defines its slots in its own file (the slot map is in the header). Labels are hidden at every scene change.

Facts of `Render.draw_string_` that the port reproduces (Render.asm 117-121, 600-638, 1629-1740):

- Alignment: LEFT = 0, RIGHT = 1, CENTER = 2; the drawn X is `X - width` (right) or `X - width / 2` (centre). Width = the sum of (glyph width + 1) * scale, a space is `4 * scale`, and `scale` is used as is
  (the default font size does not multiply it): this is `RemixFont::Measure`.
- Each glyph is placed at `Y + (14 - glyph height) * scale`, so glyph bottoms share a baseline (`RemixFont::Draw`, `kLineBaseHeight` 14).
- Numbers (`Render.draw_number`) become strings with `String.itoa_` without sign: digits only, no "%" and no leading zeros; with `decimals = d` the string has at least d + 1 digits ("0.0", "0.5"). The value
  argument is always the address of the number.
- The `blur` argument (nine-argument macros use 1) only changes the smoothing; Remix HUD text has no shadow and is white in the cases of the modes.
- Fonts: `Render.load_font()` is Kabel (default scale 1.0); `load_font_2()` is file 0xC3 (the credits font, default 0.625). Common scales: 1.0, 0.9375, 0.9, 0.875, 0.8125, 0.75, 0.625.
- Button glyphs (A, B, R, L, Z, +, D-pad) are textures, not text: file 0xC5 (`Render.file_c5_offsets`: A 0x0958, B 0x0A88, L 0x18C8, R 0x0CF8, Z 0x0BD8, PLUS 0x04D8, L_THIN 0x1B10) and file `CSS_IMAGES` 0xA06 (D-pad 0x218).
  The spaces inside strings ("Press     for ...") reserve the hole of the sprite.

Sprites stay vanilla sprites where the mode already uses them (Stamina HP digits, stock icons, the Smashketball scoreboard, the KOTH meter frame).
