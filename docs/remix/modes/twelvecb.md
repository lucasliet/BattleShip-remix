# 12-Character Battle

Remix VS mode 1 (`MNREMIX_VSMODE_TWELVE_CB`, VS menu page 2): a 1 v 1 battle played over many games, where each player has 12 characters with stocks
that last across the games. Asm source: `TwelveCharBattle.asm` (in `.toolchain/pkgs/smashremix-src/src`, about 5450 lines), with its hooks in
`VsRemixMenu.asm`, `CharacterSelect.asm` (about 60 `twelve_cb_flag` sites), `Render.asm`, `GameEnd.asm`, `Pause.asm`, `StockMode.asm`, `DKMode.asm` and
`CharacterSelectDebugMenu.asm`.
Port code: `decomp/src/gm/gmremixtwelvecb.c` (state tied to the match), `gmremixtwelvecbrules.c` (pure rules), `gmremixtwelvecbhud.c` (CSS and results
text), `decomp/src/mn/mnplayers/mnremixtwelvecblayout.c`, `mnremixtwelvecbcustom.c`, `mnremixtwelvecbcustomlegend.c`, `mnremixtwelvecbstockmanual.c`
(CSS), the 12CB branches in `mnplayers/mnplayersvs.c`, `decomp/src/if/ifremixgentlemensreset.c` (pause reset), `gm/gmremixgameend.c`.
Tests: `tests/remix/test_twelvecb_rules.c`, `test_twelvecb_layout.c`, `test_twelvecb_custom.c`, `test_stockmode_rules.c`;
`cmake --build build --target run_remix_mode_tests`. Shared plumbing: [`../modes_base.md`](../modes_base.md).

## How the Remix does it

### Rules

- Two players only (ports 1 and 2; 3 and 4 are forced N/A and their panels hidden; the 2P panel is moved with 19 `change_2p_panel_position_` hooks).
  `before_css_setup_` forces free-for-all, stock rule, `Global.vs.stocks` = the saved value (default 3, 0-based, so 4 stocks) and no time.
- Each player has a pool of 12 characters x (stocks + 1) stocks (48 by default, `config.pX.stocks_remaining`). The CSS shows 24 portraits in 3 rows of 8:
  player 1 owns the four left columns and player 2 the four right ones, so every character is shown twice and each player has 12 portraits of his own.
  A game is played with the character on the portrait each token picked; each portrait keeps its own stock count in `stocks_by_portrait_id[24]`
  (one table for both players).
- Start of a game (`game_setup_`): the status becomes STARTED, `current_game` advances and a record per player is written: character, starting stocks,
  ending stocks (0xFF = none left), portrait, was-random flag. The starting stocks of a game (`set_initial_stock_count_`, shared hook with Stock Mode) are:
  the portrait's stocks for the first game of that portrait, otherwise the winner's ending stocks of the last game when any are left.
- Each KO (`update_stocks_remaining_`, only on the battle screen) takes one stock out of the player's pool and out of the portrait that lost it; when a
  pool reaches 0 the battle is COMPLETE. A portrait with no stocks is defeated: it cannot be picked again and the CSS draws it dark with the defeated
  pose (`defeated_action_override_`, DownStandU with the per-character `defeated_array`). The winner of a game keeps his portrait until it loses a
  stock (`prevent_defeated_char_select_`, `get_last_match_portrait_and_stocks_`).
- Best character (`set_best_characters_`, run at each CSS entry): per player the character with the most TKOs for, the ones with fewer TKOs against
  winning a tie. Shown as "Best Character" and "TKOs".
- Stock count changes on the CSS (`handle_stock_count_change_`) restart every portrait and both pools, up to 99, and are locked with FGM ILLEGAL after
  the battle started.
- Gentlemen's reset (`gentlemens_reset_`, pause): the combo A + B + Z + R only makes a request ("Game reset requested by P1/P2.", "If accepted, stock
  counts will be reset.", Accept / Decline with button textures) and hands the pause to the opponent; accepting takes the game back (every stock lost
  in that game returns to the pools and portraits, the record is cleared and `current_game` goes back one); START declines.
- Salty runback is disabled in this mode (`GameEnd.asm`); with the skip results toggle the end goes to the CSS, otherwise to the results screen. The
  results header shows "12-Char. Battle" and "Game N" (`update_css_header_`), the winner's crown (file 0x22) sits on the side of the player with stocks.
- Announcer: FGM 1016 on CSS entry; a COMPLETE battle announces the winner instead ("Player 1/2" FGM 503/504 and WINS 533 at frame 0x34).

### Character sets and Custom

Each player has a character set: Default (`u`), Japanese (`j`), Polygon (`pv`), Remix (`r`), and Custom (`p1`, `p2`), all with 24 slots. D-pad up
cycles the preset (or copies the opponent's set when held 60 frames), Z / R cycle the character under the token (first press at once, then every
8 frames after a 30 frame delay), L sets all portraits to the character, D-pad down randomises. Layout: portrait 32 x 32 at a 30 unit step, sliding in from
the left or right.

### Text and sprites

All text is drawn with `Render.draw_string`: "Stocks Remaining" and the two pools (player 1 `0xB00000FF`, player 2 `0x4040C0FF`), "Character Set" and the set
names, "Best Character" with name and TKOs (name squeezed to 72 wide), "Game N", the reset request and the D-pad legends (": Presets", ": Random",
": Copy", ": Set All"). Buttons, arrows, the crown, the defeated marks and the portrait variant indicators are textures from files 0x24, 0xC5, 0x22 and the
Remix files `File.CHARACTER_PORTRAITS` (0xA05) and `File.CSS_IMAGES` (0xA06).

## How the port does it

| Asm | Port |
|---|---|
| `before_css_setup_`, `force_ffa_and_stock_`, `leave_css_setup_`, `start_match_setup_` | the 12CB profile (stock rule, FFA, 3 stocks, no goal) in `gmremixmodecss.c`; `gmRemixModeLeaveCSS` resets the battle with `gmRemixTwelveCBReset` |
| `game_setup_` | `gmRemixTwelveCBBeginMatch` (`gmremixtwelvecb.c`, called before the fighters are made in `scVSBattleStartBattle`): portraits of both players, `gmRemixTwelveCBStartGame` (record), starting stocks; a COMPLETE battle starts over |
| `set_initial_stock_count_` | `gmRemixTwelveCBGetStocks`, used by `gmRemixStockModeBeginMatch` |
| `update_stocks_remaining_` | `gmRemixTwelveCBOnKo` (`ftCommonDeadUpdateScore`, after the stock was taken) -> `gmRemixTwelveCBRecordKo` (pool, portrait, COMPLETE) |
| `set_best_characters_` | `gmRemixTwelveCBCalculateBest`; shown by `gmRemixTwelveCBHudDrawCSS/DrawResults` (`gmremixtwelvecbhud.c`) through `remix_hud_text` |
| layout, sets, hit test, slide-in | `mnremixtwelvecblayout.c` (`mnRemixTwelveCBLayoutFkind/CellX/CellY/StartX/NextX`, slot at a point), portraits built by `mnPlayersVSMakeTwelveCBPortrait` in `mnplayersvs.c` |
| Custom set | `mnremixtwelvecbcustom.c` (cycle, apply all, preset, copy, randomise, repeat timer, D-pad up timer) driven by `mnPlayersVSTwelveCBApplyCustomInput`; legend in `mnremixtwelvecbcustomlegend.c` |
| Manual stock mode in the panels | `mnremixtwelvecbstockmanual.c` (blinking arrows, D-pad icon) and `gmRemixTwelveCBCanEditStocks` / `StepStocks` |
| `setup_ports_and_characters_`, `ignore_ports_3_and_4_`, `skip_rendering_panels_3_and_4_`, `change_2p_panel_position_` | ports 3-4 N/A and hidden (`mnPlayersVSTwelveCBFrameUpdate`), `mnPlayersVSGetPanelIndex` at the 19 sites |
| `prevent_defeated_char_select_` | `mnPlayersVSTwelveCBIsSlotSelectable` with `gmRemixTwelveCBGetWinnerPortrait`; defeated portraits get the dark overlay and marks (`mnPlayersVSMakeTwelveCBDefeatedMarks`) |
| `treat_selected_defeated_chars_as_unselected_` | `gmRemixTwelveCBCheckSlot` (`mnPlayersVSGetReadyPlayerCount`) |
| `handle_stock_count_change_`, `handle_stock_change_fgm_` | `gmRemixTwelveCBTrySetStocks` and the stock arrows in `mnplayersvs.c` (denied sound FGM 165 once started) |
| `handle_custom_presses_` | `mnPlayersVSTwelveCBApplyCustomInput` |
| `gentlemens_reset_` | `ifRemixGentlemensResetHandleCombo/Update` (`ifremixgentlemensreset.c`, hooks in `ifcommon.c`) and `gmRemixTwelveCBUndoCurrentGame` |
| `update_css_header_`, `update_announcer_on_entry_` | header offset `0x2048` and FGM 1016 in `mnremixcssmoderules.c`; "Game N" in the results text; a COMPLETE battle returns FALSE from `mnRemixCssModeAnnounce`, so the vanilla voice plays instead of the winner announcement |
| GameEnd salty runback off | `gmRemixGameEndSelectScene` (`gmremixgameend.c`) skips the runback when `gmRemixTwelveCBIsActive` |

`gmRemixTwelveCBPortraitOf(fkind)` is the last-resort mapping from a fighter to a portrait slot (the 12 original fighters in 0..11, any other by number in
12..23); the CSS reports the real slot through `gmRemixTwelveCBSetCssPortrait`, and the match prefers the slot the fighter holds in the layout of the
player's character set.

### Deviations from the asm

- **Records.** The asm keeps 24 game records per player, which a long battle can overrun (each game defeats one portrait of the loser, so up to 47
  games); the port keeps 48 (`GMREMIX_TWELVECB_GAMES`).
- **N64 heap.** No `custom_heap_address` reset or heap slot handling: the port loads files per kind on demand.
- **Text path.** All CSS and results text goes through `remix_hud_text` (the Remix Kabel font rebuilt from the user's .o2r); buttons, arrows and the
  crown are native sprites.

Open gaps: `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

- Shortcut: `SSB64_REMIX_MODE=12cb`. The log has `twelvecb: game N p1 fkind A stocks S pool P, p2 ...` at the start of a game and
  `twelvecb: port N KO, stocks left L, pool P, status S` per KO; `twelvecb: game taken back, ...` after an accepted reset.
- CSS and a game: `SSB64_REMIX_MODE=12cb .claude/skills/remix-smoke-tests/scripts/css.sh .claude/skills/remix-smoke-tests/inputs/css_pick_mario.txt tcb 900`
  (both ports pick Mario, START at 200, A on the stage select at 300). `SSB64_CSS_STOCKS=<n>` opens the CSS with n stocks (the value shown is n + 1).
- Direct match: `SSB64_REMIX_MODE=12cb SSB64_VS_STOCKS=2 .claude/skills/remix-smoke-tests/scripts/vs.sh 0,8 .claude/skills/remix-smoke-tests/inputs/walk_right.txt tcb_ko 2200`
  to reach a KO and the results with the pool record.
- Check: after a game the winner's portrait stays selected, a defeated portrait cannot be picked, holding A + B + Z + R + START at the end does not start a
  salty runback, the pause reset asks the opponent first.
