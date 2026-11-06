# Smash Remix menus and screens

Covers the menu side of the Remix (`Menu.asm`, `Toggles.asm`, `SRAM.asm`, `Settings.asm`, `VsRemixMenu.asm`, `SinglePlayerMenus.asm`,
`Stages.asm`, `CharacterDataScreen.asm`, `Credits.asm`, `Gallery.asm`, `VsStats.asm`, `resultsscreen.asm`, `Transitions.asm`, `Boot.asm`,
`BGM.asm` menu music, `timeouts.asm`, `VsDemo.asm`) and how the port implements it. Port code: `decomp/src/mn/**` (`mncommon`, `mnvsmode`,
`mn1pmode`, `mnmaps`, `mndata`, `mnplayers`), `decomp/src/sc/sccommon/` (credits, demo), `decomp/src/gm/gmremix*menumusic*`,
`port/remix/remix_toggles.*`, `port/gui/PortMenu.cpp`. The character select is in `docs/remix/css_reference.md`, the CSS panel in
`docs/remix/panel.md`. Asm lines refer to `.toolchain/pkgs/smashremix-src/src/`.

Tests: `tests/remix/` (`test_gallery_rules.c`, `test_vsstats_rules.c`, `test_gameend_rows_rules.c`, `test_group_display_rules.c`,
`test_stage_grid_rules.c`, `test_stage_select_rules.c`, `test_sprite_rules.c`, `test_sprite_bitmap_row.c`, ...), and the per-section recipes, which run
with `SSB64_START_SCENE=<id>` through `.claude/skills/remix-smoke-tests/`.

Audit ledger (every item closed): `docs/remix/patch_audit.md`, groups g2 (menus, Character Data, results, player tag), g8 (results screen) and g9 (gallery,
VS demo, credits, boot).

---

## 1. Settings menu and toggles

### 1.1 Remix

- **Menu framework** (`Menu.asm`): types TITLE, MAX1, BOOL, MAX3 .. MAX1023, INPUT (text entry) and INT. `Menu.info` (`:37-60`) holds the
  head pointer, position, group, colours, row height and rows per page (12); `Menu.entry` (`:66-110`) is a linked node (0x00 type, 0x04
  value, 0x08 min, 0x0C max, 0x10 function, 0x14 string table, 0x18 copy address, 0x1C next, 0x20 label, 0x24 extra, 0x28 title). It draws
  with `Render` and `Joypad` helpers and does not reuse vanilla option drawing. `Menu.export_`/`import_` (`:1569`, `:1655`) bit-pack the values
  into 32-bit words in declaration order (titles skipped, INPUT separate).
- **Toggles** (`Toggles.asm`): a root menu with groups Remix Settings, Gameplay Settings, Music Settings, Stage Settings, Pokemon Settings,
  Player Tags, Other Screens and "Load Profile". A toggle's value lives at `Toggles.<name> + 0x4`, read with `Toggles.read(name, reg)`.
  One SRAM block per group (`block_remix`, `block_gameplay`, `block_music`, `block_stages`, `block_pokemon`, `block_tags` = 20 x 20 bytes);
  `Toggles.save_`/`load_` call `Menu.export_`/`import_` and `SRAM.save_`/`load_`. `SRAM.ADDRESS` 0x05F0, `REVISION` 0x00F7; the vanilla
  save/load is restricted to the first 0x5EC bytes (`SRAM.asm:35-58`).
- **Profiles**: Community (CE 0), Tournament (TE 1), Netplay (NE 2), Japanese (JP 3), Custom (4). `write_defaults_for(profile)` writes the
  per-profile defaults, `load_profile_` (`:3089`) applies one. `normal_options` gates disabling the OPTION screen while custom toggles are
  active (`disable_options_functionality_`, `:88`).
- **Settings.asm**: one live hook at ROM 0x40898 (RAM 0x800A1B48, `scManagerInit`) jumps to `set_vs_settings_`, which writes the Tournament VS
  defaults to `Global.vs`: game_mode 3 (time and stock), time 8, stocks 3, team_attack 1, item_frequency 0. The asm comment says "should not
  be called" but the patch is live.
- **Mode Select** (`Toggles.asm:109-376`): a 5th "Settings" button (index 4) with the others rescaled (`rescale_mode_select_buttons_`);
  `mode_select_screen_change_` (`:271`) opens the super menu, clears `singleplayer_mode_flag` and the input masks. Holding L on the CSS or
  stage select opens the menu (`settings_shortcut`, `:3260`).

### 1.2 Port

- The settings menu is the ImGui menu, not a replica of `Menu.asm`: `PortMenu.cpp` `AddMenuRemix` builds Remix > profile buttons and one
  widget per toggle (checkbox, combobox or integer slider), plus Player Panel, Player Tags, Modes (Smashketball type, Tag Team queue) and
  Training pages. The OPTION screen stays vanilla; the 5th Mode Select button and the L shortcut do not exist in it. The Other Screens
  entries (Debug, View Intro, How to Play, Credits, Gallery) are the buttons of Remix > Other Screens.
- A-press rows (`port/remix/remix_toggle_actions.{h,cpp}`): the widgets have no A press, so each row runs its routine after the value
  changes. BGM Volume, SFX Volume, Punish Failed Z-Cancel, Default CPU LVL (V.S.) (`set_cpu_level`), Menu Music (`play_menu_music_`, plays
  the chosen track now) and every per-track Random Music bool (`preview_bgm_`, plays the BGM id of the entry word 0x24) are ordinary
  widgets. The title rows `toggle_all_` ("Random Music Toggles:", "Random Stage Toggles:", "Toggle:") are a "toggle all" button at the same
  place of the page, and the "Load Profile:" INT rows (`load_sub_profile_`) are one button per profile on the Music and Stage pages. Details
  and the routines' port names: [`toggles.md`](toggles.md) section 1.4.
- Deviation, Random Music preview: the asm sets `reset_menu_music` and restarts the menu music when the settings screen is left. The port
  menu also opens during a match, so the first preview remembers the BGM id playing (or none) and closing the menu (`PortMenu::UpdateElement`,
  `remix_toggle_action_menu_closed`) puts it back or stops the music; a Menu Music press after a preview keeps the track it chose.
- `port/remix/remix_toggles.{h,cpp}`: the registry is built at start-up from the `Menu.entry` nodes of the Remix RAM image (value, range,
  string table, title) and the four `Toggles.profile_defaults_*`. A value is the CVar `gRemix.Toggle.<index>`. Environment:
  `SSB64_REMIX_PROFILE=<0..3>`, `SSB64_REMIX_TOGGLES="<title>=<value>;..."`. Match code reads a toggle with `gmRemixToggle(name)`
  (`gm/gmremixtoggle.c`); `gmRemixToggleModeIsGuarded()` is true in the Bonus and 1P Game modes.
- There is no SRAM image: the vanilla save goes through `port_save.cpp`/`lbBackup`, Remix toggles and panels through CVars.
- `Settings.asm`: `scRemixTournamentApplyVSDefaults` (`decomp/src/sc/scremixtournament.c`) writes `game_rules` = time | stock, `time_limit` 8,
  `stocks` 3, `is_team_attack` TRUE and `item_appearance_rate` = none into `gSCManagerTransferBattleState` (what `Global.vs` maps to),
  called from `scManagerInit` right after the default copy (`scmanager.c`).
- `timeouts.asm` (11 patches): each patch turns the `bne` of the 18000-frame idle counter (5 minutes) into `b`, so no screen returns to the
  title by idling. Port: `mnremixtimeouts.{c,h}` (`MenuReturns`, `VSReturns`, `CSSReturns` over a core that returns FALSE under
  `REGION_REMIX`) is used by `mnmodeselect.c`, `mn1pmode.c`, `mnoption.c`, `mndata.c`, `mnmaps.c`, `mnvsmode.c`, `mnvsoptions.c` and the four CSS.
  The counters still count and re-arm on input; only the screen change is gone. In the VS CSS the `#ifdef PORT` Classic Co-op idle exit also
  stops.
- `Cheats.asm`: only the word at 0x42B3A is live; every other instruction in the file is commented out, so nothing is ported. The port keeps
  its own `gCheats.UnlockAll` (`lb/lbbackup.c`).

### 1.3 Toggle list

Columns are the defaults CE / TE / NE / JP; INT entries show the maximum value.

**Remix Settings** (`Toggles.asm:2376-2412`):

| Toggle | CE/TE/NE/JP | Toggle | CE/TE/NE/JP |
|---|---|---|---|
| Skip Results Screen (INT 2) | 0/0/1/0 | Salty Runback (INT 2) | 1/0/1/1 |
| Hold To Pause | 0/1/1/0 | Widescreen | 0/0/0/0 |
| CSS Panel Menu | 1/0/1/1 | Stereo Fix for Hit SFX | 1/1/1/1 |
| Color Overlays | 0/0/0/0 | Random Select (INT 3) | 0/0/0/0 |
| Cinematic Entry (INT 2) | 0/0/0/0 | Disable HUD (INT 2) | 0/0/0/0 |
| Flash On Z-Cancel | 0/0/0/0 | Disable Anti-Aliasing | 0/0/0/0 |
| FPS Display (INT 2) | 0/0/0/0 | Always Show Full Results | 1/1/1/1 |
| Model Display (INT 2) | 0/0/1/0 | Skip Start Cheer (INT 2) | 0/0/0/0 |
| Special Model Display (INT 3) | 0/0/0/0 | Default CPU LVL (V.S.) (INT 10) | 0/10/0/0 |
| Advanced Hurtbox Display | 0/0/0/0 | Jigglypuff Sing GFX Anims | 0/0/0/0 |
| Hold To Exit Training | 0/1/0/0 | 'L' Selects Random Character | 0/0/0/0 |
| Improved Combo Meter | 1/0/1/1 | Dpad CSS Cursor Control | 0/0/0/0 |
| Tech Chase Combo Meter | 1/0/1/1 | PK Thunder Reflect Crash Fix | 1/1/1/1 |
| Combo Meter | 1/0/1/1 | Flash Guard | 0/0/0/0 |
| 1v1 Combo Meter Swap | 0/0/0/0 | Screenshake (INT 2) | 0/0/0/0 |
| Neutral Spawns | 1/1/1/1 | BlastZone GFX (INT 2) | 0/0/0/0 |

**Gameplay Settings** (`:2413-2453`): Hitstun (INT 1) 0/0/0/0; Hitlag (INT 4) 0/0/0/1; DI (INT 2) 0/0/0/1; Japanese Sounds (INT 2) 0/0/0/1;
Japanese Stun/Sleep 0/0/0/1; Momentum Slide 0/0/0/1; Shield Stun (INT 4) 0/0/0/1; Z-Cancel (INT 4), Punish Failed Z-Cancel (INT 11), Tripping
(INT 3), Rage (INT 4), Air Dodging (INT 3), Charge Smashes (INT 2), Item Containers (INT 3), Game Speed (INT 12), Special Zoom (INT 2),
BlastZone Warp (INT 3), Single Button Mode (INT 6), Move Staling (INT 6), Stopwatch Item (INT 4) all 0 in every profile; Improved AI 1/0/1/1;
Footstool Jumping, Jab Locking, Edge C-Jumping, Perfect Shielding, Parrying, Spot Dodging, Fast Fall Aerials, Ledge Trumping, Wall Teching,
All Items R Drop (Aerial) all off in every profile.

**Music Settings** (`:2454-2546`): Play Music 1/1/1/1; Random Music 0/0/1/0; Salty Runback Preserves Song 0; Menu Music (INT) 0/0/1/0; Music
Title at Match Start 1/0/1/1; BGM Volume and SFX Volume (INT 10) 10; Random Music toggles (one per stage/menu track, 1 everywhere); one
toggle and one order entry per custom MIDI (`MIDI.MIDI_*_TOGGLE`, `*_ORDER`), default 1. "Load Profile" (INT 0-3) calls `load_sub_profile_`.

**Stage Settings** (`:2561-2615`): Stage Select Layout (INT 1; `sss.NORMAL`/`TOURNAMENT`) 0/1/0/0; Hazard Mode (INT 3) 0; Whispy Mode (INT 3)
0/0/0/Japanese; Saffron Pokemon Rate (INT 3) 0; Pokemon Announcer (INT 2) Pokemon/Off/Pokemon/Off; Dragon King HUD (INT 2) Default/Off/Default/
Default; Camera Mode (INT 3) Normal; Yoshi's Island Cloud Anims 0; Random Stage toggles (bool, CE/TE/NE/JP): Congo Jungle 1/0/1/1, Dream Land
1/1/1/1, Dream Land Beta 1 and 2 0, Duel Zone 1/0/0/1, Final Destination 1/0/0/1, How to Play 0, Hyrule Castle 1/0/1/1, Meta Crystal
1/0/1/1, Mushroom Kingdom 1/0/0/1, Peach's Castle 1/0/1/1, Planet Zebes 1/0/0/1, Saffron City 1/0/1/1, Sector Z 1/0/0/1, Yoshi's Island
1/0/0/1, Mini Yoshi's Island 1/0/1/1, plus one per custom stage (`Stages.STAGE_*_TOGGLE`).

**Pokemon Settings** (`:2624-2643`): Pokemon SFX (INT 2) 0/0/0/1, Toggle All, Onix, Snorlax, Goldeen, Meowth, Charizard, Beedrill, Blastoise,
Chansey, Starmie, Hitmonlee, Koffing, Clefairy, Mew (1 everywhere; Psyduck, Shellder, Staryu are commented out). **Player Tags**
(`:2650-2660`): 20 INPUT entries. **Other Screens** (`:2665-2680`): Debug (0x03), View Intro (0x1C), View How to Play (0x3C), View Credits
(0x38), View Gallery (`view_gallery`, `:2330`).

---

## 2. Scene ids (`Global.asm:58-122`)

Vanilla ids in use: MODE_SELECT 0x07, VS_GAME_MODE_MENU 0x09, VS_OPTIONS 0x0A, VS_CSS 0x10, _1P_CSS 0x11, STAGE_SELECT 0x15, DATA_VS_RECORD 0x19,
DATA_CHARACTERS 0x1A, CONGRATULATIONS 0x37, STAFF_ROLL 0x38, OPTION 0x39, DATA_MENU 0x3A, DATA_SOUND_TEST 0x3B, HOW_TO_PLAY 0x3C. The
Remix adds `REMIX_MODES` (0x77) for the non-1P Remix modes (All-Star, Multi-Man, HRC). The Gallery has no id of its own: while it is active
it forces CONGRATULATIONS (`Gallery.asm:1265-1280, 1448, 1475`). The Character Data screen keeps 0x1A. Scene ids of the port are the `SCKind`
enum of `decomp/src/sc/scdef.h` (for example 16 VS CSS, 18 Training CSS, 22 VS battle, 26 Character Data, 52 1P game, 56 staff roll,
59 sound test, 61 auto-demo).

---

## 3. VS Game Mode menu (`VsRemixMenu.asm`)

Remix: the VS menu has two pages. `menu_button_table` (page 1): VS Start, Rule, Time/Stock, VS Options, Remix Modes (db 0x09).
`remix_menu_button_table` (page 2): 12-Character Battle, Tag Team, King of the Hill, Smashketball, Tug of War. `mode` constants:
DEFAULT 0, TWELVE_CB 1, TAG_TEAM 2, KOTH 3, SMASHKETBALL 4, TUG_OF_WAR 5 (`:12-19`). A button row (`:~85-90`, 0x10 bytes) holds the create
routine, screen, `vs_mode_flag`, `page_flag`, 12CB flag, text X/Y floats and text texture offset. `add_button_` (`:102-150`) patches
the cursor wrap from 3 to 4 and a 5th pointer into `buttons[]`; button creation is rewritten generically (`create_menu_button_` :159,
`create_button_generic_` :196). `page_2_exit` (`:304`) returns to page 1. The settings of the VS menu are saved before the CSS
(`save_global_settings_` :388) and `set_cursor_index_` (`:810-848`) restores page 2 and the cursor on the played mode after the CSS.
The rest of the file hooks the CSS and the match (`mode_specific_setup_`, `control_match_start_`, `check_match_end_`,
`change_time_options`, `disable_mode_toggle_`, `update_announcer_on_toggle_`, `add_row_to_results_`).

Port: `mn/mnvsmode/mnremixvsmode.{c,h}` owns `gMNVSModeRemixMode` and the row table (box and text position, text offset in the MNVSMode
file; rows 0..4 are the page 2 modes, row 5 the page 1 "Remix Modes" button). `mnvsmode.c` adds the page state (`sMNVSModePage`,
`sMNVSModeRemixButtonGObjs[5]`) and the helpers `mnVSModeMakePage1Buttons`, `mnVSModeMakeRemixButtons`, `mnVSModeSetPage`,
`mnVSModeSelectRemixMode`: up/down wrap over 0..4 on both pages; A on "Remix Modes" switches to page 2 in place (the scene is not reloaded);
B on page 2 returns to page 1 with the cursor on "Remix Modes"; A or START on a page 2 button stores the mode and leaves for the VS CSS
exactly as VS Start (`mnVSModeSaveSettings()`, `scene_curr = nSCKindPlayersVS`). The text of the new buttons is a `Sprite` of the MNVSMode
file read like the vanilla texts. Mode rules are in `gm/gmremixmodecss.c` (`gmRemixModeBeforeCSS` / `LeaveCSS` / `StartMatch`).

The mode flags the asm sets (`twelve_cb_flag`, `vs_mode_flag`) are replaced by `gm/gmremixmode.c` state. As in the asm,
leaving the CSS reopens the VS menu on its Remix page with the cursor on the mode played (MN-14).

Test: `SSB64_START_SCENE=9`, input `100 108 0 - 0 -80` (four downs from VS Start reach Remix Modes), A at 220 opens page 2, four more downs
reach Tug of War, A at 380 leaves for the CSS; `gMNVSModeRemixMode` equals 1 + the button index.

---

## 4. 1P mode menu (`SinglePlayerMenus.asm`)

Remix: generic button creation (`create_menu_button_` :103, `render_menu_buttons_` :211, `fix_cursor_wrap_` :251, `set_cursor_index_`
:290, `page_2_exit` :341), `fix_a_and_b_button_crashes_` (:443), 34 hooks, stock/KO and time displays (`_ko_amount_or_feet` :460,
`load_bonus3_time` :508, `total_kos_or_feet_` :571, `draw_hrc_high_score_decimal_` :681), and the HRC/Remix headers
(`header_graphic_remix_1p` :807). Port: `mn/mn1pmode/mnremix1pmode.{c,h}` (menu button tables and pages, called from `mn1pmode.c`); the
mode logic is in `decomp/src/sc/sc1pmode/`.

---

## 5. Stage select (`Stages.asm`)

Remix: grid `NUM_ROWS` 3 x `NUM_COLUMNS` 6, `NUM_ICONS` 18, `NUM_PAGES` 5 (90 icons), `MAX_STAGE_ID` 0xD7 (`:243, 542-545`). Routines:
`update_stage_icons_`, `get_index_`, `check_valid_page_`, `get_stage_id_`, `update_text_`, `update_cursor_color_`, directional handlers
(`:2152-2299`), `random_stage_roulette` (C-Down on RANDOM), `add_stage_to_random_list_`. Layouts `sss.NORMAL` and `sss.TOURNAMENT` (the
latter a 3-page grid that turns variants and the L hazard toggle off and forces Hazard Mode to the hovered stage's value). Screen patches
(`:970-1010`): "Stage Select" image y 122 to 128 (0x80132310), no yellow name plate (0x80132354), blue bar y 134.5..140.5 (0x801321C4), no
plate shadow (0x801321D4), no vanilla name sprite (0x80132738); the name is a string at (230, 209), centred, blank over RANDOM
(`update_text_`, `:2851`). Four lines under the preview ("Hazards (L):", "Movement (L):", "Layout (C-down):", "Roulette (C-down)") and a page
strip (arrow, Z, "Page N", R, arrow) are drawn in `setup_` (`:2845-2856`).

Port (`decomp/src/mn/mnmaps/`): `mnremixstagegridrules.c` (pure grid), `mnremixstagepages.c` (pages read from the image tables
`stage_table`, `stage_table_pages`, `stage_table_normal`, `stage_table_tournament`), `mnremixstagepager.c` (page strip), `mnremixstagepanel.c`
(name and the four lines, text slot 125; the blue bar fills 134..140), `mnremixstagerandom.c` (+ generated `mnremixstagerandomtable.inc`) and
`mnremixstageroulette.c`, `mnremixstagetournament.c`, `mnremixstagevariant.c` (C-down variant cycle over `Stages.variant_table`),
`mnremixemblem.c` (series emblem on the wooden coin). Test: `SSB64_MAPS_START_GKIND=<gkind>` with `css.sh` and
`stage_select_sweep.sh`; page 3 with `inputs/css_page3_stage.txt`.

---

## 6. VS item switch extension (`Item.asm`)

Remix: `extend_item_switch_bitmask_` and `item_switch_cursor_` on the vanilla Item Switch screen (scene 0x0B) add a second page with the 13
Remix standard items (R flips the page, L toggles all items of the page); `item_switch_setup_` (`:955-975`) draws the legend. Port
(`mn/mnvsmode/mnvsitemswitch.c`, `mnremixitemswitchlegend.c`):

- Page 0 is the vanilla 15 rows, page 1 the 13 Remix items. A page flip ejects/rebuilds the grid or the custom strip GObj, clamps the
  cursor and refreshes the toggle sprites. L flips the page's first toggle and writes it to every row of the page. L_TRIG/R_TRIG are
  removed from the two input masks.
- Storage: rows 1-13 of page 1 read/write `item_toggles` bits `nITKindCommonEnd + 1 + n` for n < 12 and `gITRemixExtendedToggles` bit n - 12 for
  n == 12 (`GetItemSettings`/`SetItemSettings`); any custom item on forces the container/rate bits like the vanilla all-on rule.
  `itRemixSaveExtendedToggles` (menu exit) and `itRemixRestoreExtendedToggles` (CSS entry, `Render.asm _css`); the extended mask is zeroed
  with `item_toggles` in the competitive no-items override and in Training and reset by `lbBackupCorrectErrors`. Netplay serialises it next
  to `item_toggles`.
- Icon strip: offset 0xBD70 of the decompressed Remix MNVSItemSwitch file is a complete 68-byte `Sprite` (112x165, 3 bitmaps) drawn with
  the same `lbCommonMakeSObjForGObj` path and position as the vanilla grid. The strip exists only in the Remix variant of the files.
- Legend: the R button (file 0xC5 offset 0x0CF8, at 272.0/38.25), the orange arrow (main menu images file 0x00 offset 0xDD90, 288.0/41.0,
  `IFREMIX_SPRITE_MENU_ARROW`), the L button (0x18C8, 202.0/38.25) and ": All On/Off" (214.0/39.5, colour `0xC0C0C0FF`, scale 0.63,
  `remix_hud_text` slot 4); the sprites use the decal camera (dl link 4), the text is the overlay. Visible on both pages.
- The subtitle on page 1 shows the vanilla row names, as the Remix does not patch it.

---

## 7. VS results screen

### 7.1 Match stats (`VsStats.asm`)

Remix: after the results timer, "Press A for Match Stats" appears (`check_menu_toggle_` :675, `STATS_TIME_UNTIL_ACTIVE`); A shows page 1
(damage, combos), R page 2 (Z-Cancel, Tech, Ledge, Special Move Stats), B goes back. No setting gates it. The trackers: `calculate_tracker_percentage_`
(:168), `increment_special_tracker` (:210) at the special-move dispatches, Z-cancel (`ZCancel.asm`), wall tech (`WallTeching.asm`), passive tech and
roll, ledge grabs (`LedgeTrump.asm track_ledge_grabs_`, 0x800DE688), highest percent (`run_collect_` :1071, `collect_stats_midmatch`); drawing
`draw_port_headers_` (:542), `draw_underline_` (:288), `draw_line_` (:354), `setup_` (:917). `prevent_timed_row_flash_` (:875) hides the
normal results rows (group 0x16) while the page shows.

Port: `mn/mnvsmode/mnremixvsstats.{c,h}` (counters), `mnremixvsstatsrules.{c,h}` (pure), `mnremixvsstatspages.{c,h}` (layout: label left
aligned at X 24, one right-aligned cell per active port at X 184 + 35 * column, scale 0.8125), `mnremixvsstatsview.{c,h}` (prompt at
(160, 218), input from `gSYControllerDevices[i].button_tap` of any controller, 15-frame press buffer, FGM 163/164),
`mnremixvsstatsscene.{c,h}` (camera, dimmed background alpha 0xB0 on dl link 36 and button sprites on 37, `ifRemixSprite`).
Hooks: `ftcommonspecial{hi,n,lw}.c` and `ftcommonspecialair.c` (specials), `ftremixzcancel.c` (`ftRemixZCancelCountResult`),
`ftremixwalltech.c`, `ftcommonpassive.c` and `ftcommonpassivestand.c` (tech success), `mpcommon.c` (ledge grab, in
`mpCommonRunFighterSpecialCollisions`), `ftparam.c` `ftParamUpdateDamage` (highest percent, after the 999 clamp), three lines in
`mnvsresults.c`, reset in `scVSBattleStartBattle`. Damage per port pair is read from `gSCManagerTransferBattleState.players[]`. The
group 0x16 hide is `gmRemixGroupDisplaySet` (Render.asm `toggle_group_display_` over `gGCCommonLinks`).

Deviations: cells are centred and spaced with 6-character padding rather than aligned at fixed X; stripe parity restarts on page 2; the
special-move percent uses truncation, the others round to nearest even like `cvt.w.s`. The A, B and R buttons are extension sprites
(`ifRemixSpriteMake` in `mnremixvsstatsscene.c`); a CPU tech miss is counted by `ftremixaiteching.c` (`AI.asm` `random_teching_`).
Test: `inputs/stats_view.txt`; play a 2-4 player match with a CPU to the results, wait for the prompt (do not press START), A, R, B.

### 7.2 Extra rows, full results, winner

- **Always Show Full Results** (`TimedStock.asm show_full_results_`, 0x801376E4): `gmRemixGameEndResultsDrawKind` maps StockRoyal to
  TimeRoyal and StockTeam to TimeTeam when the toggle is on; used in `mnVSResultsMakeLabel`. The asm falls into the Time branch of `mnVSResultsInitVars`, which also sets
  the exit wait 410 instead of 370: `gmRemixGameEndResultsStockExitWait` (`gmremixgameendwaitrules.c` holds the numbers).
- **Mode rows** (`VsRemixMenu.asm add_row_to_results_`, :1007-1047): King of the Hill adds a "Times" row (`M:SS`, tens digit forced) and
  Smashketball an "FGs" row (baskets capped at 999) at y 124 (96 on a no contest at tic 100); the place line drops to 140 and appears 20
  tics later; teams use the FFA routine. Port: `gm/gmremixgameendrowsrules.c` (constants, pure) and `gmremixgameendrows.c` (`gmRemixKothGetScore` /
  `gGMRemixSmashketball.scores`), drawn in `mnVSResultsMakeKOs` (`mnvsresults.c`) with `mnVSResultsMakeNumber`, label offsets 0xE648 and 0xE9E0.
- **resultsscreen.asm**: `vs_record_fix_` skips fkinds >= 12 in the records (`mnvsresults.c`), `fix_port_index_` (`desc.player = player`),
  winner BGM `get_victory_bgm_` -> `mnRemixResultsGetWinBGM` (`mnremixwinbgm.c`: reads `Character.winner_bgm.table`, each Remix address points
  at `add_victory_bgm(bgm)`, whose last instruction loads the BGM id; a vanilla address means the parent's music), the spinning series emblem
  (`winner_logo_fix_`), "WINS!" position (`get_str_wins_lx_`), the winner name (`get_str_winner_info_`), the "&" glyph (`add_ampersand_`),
  announcer voice (`get_winner_fgm_`) and C-button victory animations (`victory_animations_`, CR 3, CL 2, CU/CD 1).
- **Header and Game N**: the mode title image (file 0x12 offset 0x2048 for 12CB, and the table of `docs/remix/css_reference.md` section 2.6) in
  `mnVSResultsMakeLabel` (`mnvsresults.c`), and "Game N" at (32, 46) for the 12CB (`gmremixtwelvecbhud.c`).
- **Transitions** (`Transitions.asm`): the restored VS-to-results transitions shrink the table to halfwords; 15 entries (Falling Shapes with file
  0x27 offsets 0x3BC0/0x3D78, Flip with file 0x2F offsets 0x0F98/0x101C, Smash Logo file 0x1262 offsets 0x3C80/0x4138, Remix Logo file 0x1366
  offsets 0xAB60/0xAF40, plus the vanilla ones). Port: `lb/lbtransition.c` `dLBTransitionDescs`, count `LB_TRANSITION_DESC_COUNT` 15 under
  `REGION_REMIX` (11 otherwise); `mnvsresults.c` picks `syUtilsRandIntRange(ARRAY_COUNT(dLBTransitionDescs))`. `SSB64_TRANSITION_ID=<0-14>`
  forces one (13 Smash Logo, 14 Remix Logo).
- **Team results** (`Teams.asm`): see `docs/remix/css_reference.md` section 4.6.

---

## 8. Character Data screen (0x1A)

Remix (`CharacterDataScreen.asm`): `set_char_order` (`:938-980`) lists 30 fighters (Mario, Luigi, Peach, Bowser, Dr. Mario, DK, Wario, Link, Young
Link, Sheik, Ganondorf, Samus, Yoshi, Kirby, Dedede, Fox, Falco, Wolf, Pikachu, Jigglypuff, Mewtwo, Captain, Ness, Marth, Goemon, Sonic, Crash,
Marina, Banjo, Conker); the scroll maximum is `new_characters + 11 = 29` (`extend_index_`). `add_char_to_data_screen` stores for each of the 18 added
fighters the biography, works and name offsets (a top-byte flag selects the Remix extended file; addends +0x30, +0x10, +0x10) and the name X/Y;
the extended files are 0x10F5 (bios) and 0x10F6 (images). Also: `extend_series_logo_offsets` (emblem from
`ResultsScreen.series_logo_offset_table[fkind]`), `special_attack_offsets` (USP/NSP/DSP name images in file 0x10F6, `:594-611`, offsets 770-835),
`special_action_table_n` and `jab_action_pointers` (`:267-300`, `set_action` lines 838-936; five fighters reuse Fox/Falcon/Link/Mario/Yoshi
tables), `extend_name_border_check` (`:635`, the tall name tag for Falcon, Dr. Mario, Ganondorf, Banjo and Young Link) and the animation end
check `<= 0.1` instead of 0 (`:996`, a fix for the "frozen" demo sequence). The records screen (0x19) is unchanged: 12 vanilla fighters.

Port: `mn/mndata/mnremixchardata.{c,h}` (30-entry order, the 18 extended entries with raw asm values and the decoder that adds the addends and
masks the offset to 24 bits, page lookup) and branches in `mncharacters.c`: `MNCHARACTERS_FIRST/LAST_PAGE` 0..29, `dMNCharactersFileIDs`
appends 0x10F5 and 0x10F6, `mnCharactersMakeRemixStory`/`Works`/`Name` read the sprites from the extended files, `mnCharactersMakeFighter`
calls `ftManagerSetupFilesAllKind`. `mn/mndata/mnremixcharacters.{c,h}` reads the Remix tables from the RAM image: the emblem
(`mnCharactersMakeRemixEmblem`: DObjDesc, MObjSub and MatAnimJoint offsets from `ResultsScreen.series_logo_offset/zoom/color_table[fkind]`; the
asm reads only the first and leaves the other two at 0, the port reads the whole triple), the special actions and jab combo
(`special_action_pointers`, `jab_action_pointers`; a pointer into the vanilla overlay selects the port's own vanilla table, `mnCharactersGetRemixMotion`),
the special-move name sprites (`special_attack_offsets`, file 0x10F6) and the tall name tag (`mnRemixCharactersHasTallNameTag`). The animation end
check is `<= 0.100098` under `REGION_REMIX`. The random motion pick skips the kinds whose status the synth fighter cannot run
(`mnCharactersCheckRemixMotionKindPlayable`). Test: `scene.sh 58` with `inputs/chardata_lap.txt` (A on Characters, then R; booting scene 26 directly gives the demo mode); left and right wrap at both ends; the Remix `.o2r` must provide files
0x10F5/0x10F6. `SSB64_REMIX_CHARDATA_MOTIONS=<k1,k2,...>` (decimal `MNCharactersMotionKind` values) replaces the random pick by that list, cycling.

---

## 9. Credits (`Credits.asm`)

- **Tables** (`:864-1160`): extended title, name, details, company and section tables. Port: `tools/gen_remix_credits.py`
  (`.toolchain/host/venv/bin/python -I tools/gen_remix_credits.py`) reads `decomp/src/credits/remix/remix_credits.txt` (20 titles, 309 names
  with details and company, 1 company; `\d` is `"` and `\s` is `'`) and writes `decomp/src/credits/remix/{staff,titles,info,companies}.credits.
  {encoded,metadata}`, `jobs.credits.inc` and `companyids.credits.inc` (84 + 309 = 393 names, 18 + 20 = 38 titles, 12 companies;
  `nSCStaffrollCompanyRemix` = 11). `scstaffroll.c` includes them under `REGION_REMIX`; the vanilla `Presents` job ends at name 84 so the
  Remix jobs are reachable; the loop end uses `ARRAY_COUNT(dSCStaffrollStaffRoleTextInfo)`.
- **Glyphs** (`use_added_characters_`, `add_new_character` :260-288): ids 0x50-0x61 are digits 0-9 (`ch + 0x20`, '4' keeps the vanilla 0x37), then
  `_ ! - / * ( ) @ ~`. The textures are not in `File.REMIX_CREDITS` (0xDF1) but appended to the vanilla credits file 0xC3 (`SCStaffroll`, 0x89C8
  bytes in the `.o2r`, a 0x58-byte header before the asm offsets); `scstaffroll.c` adds the 18 sprite-table rows (width, height, offset) at
  indices 0x50-0x61 and grows the display list table to 0x62 entries (ids 0x38-0x4F stay empty). The encoder maps the symbols to those ids.
  Details keep `|` as the line break; an apostrophe maps to the vanilla 0x45 glyph and `!` in details becomes a space as in the asm.
- **Speed assist** (`remix_credits_speed_assist_` :1310-1349, patch 0x801335C4 replaces the Start test of `scStaffrollFuncRun`): when the custom
  player tag number 2 (`string_table + 8`, the panel value 2, not the tag of port 2) is exactly `afk` (word 0x61666B00), the name id is 0x55
  and the roll speed differs from the normal one, Start is simulated once. Port: `scStaffrollIsStartPressed` (`scstaffroll.c`) with
  `sc1PRemixAfkIsSpeedAssistActive` (`sc1premixenemyafk*`); enable with `SSB64_REMIX_TAG_NAMES="2=afk"`. The fast speed on Start is 0.11
  (`:20-23`, `scstaffroll.c` 0.11F under `REGION_REMIX`, 0.05F otherwise).
- **Away mode** (`Credits.asm:1238-1246` `awayMode`): with the same `afk` tag the crosshair fires by itself at the names
  (`sc1PRemixAfkCreditsTapShoot`) and an automatic shot plays no sound and draws no highlight (`sc1PRemixAfkCreditsShotIsSilent` in
  `scStaffrollMakeHighlightGObj` and `scStaffrollCheckCursorHighlightPrompt`). Test `l5_credits_afk` against `l5_credits_noafk`: the
  info boxes of Yoshiki Suzuki and Yoshiya Taniguchi open with no input, no FGM is logged.
- Test: `SSB64_START_SCENE=56`; after "Presents" the Remix titles run from "Project Leader" to "Video Team"; names with digits and symbols
  show their glyphs.

---

## 10. Gallery (`Gallery.asm`)

Remix: one victory picture per entry (40: Mario, DK, Link, Samus, Yoshi, Kirby, Fox, Pikachu, Luigi, Captain, Ness, Jigglypuff, GND, YLink, DRM,
Falco, Wario, Bowser, Wolf, Conker, MTwo, Marth, Sonic, Sheik, Marina, Dedede, Goemon, Banjo, Crash, Peach, DSamus, Lucas, Slippy, Roy, DRL, Lanky,
Dragon King, Ebi, Piano, Boss; `id_table` :445-487) on the Congratulations scene. Pictures are the BOTTOM/TOP pairs of `File.asm` (vanilla
characters) or `SinglePlayer.custom_victory_file_table` (`(char_id - 12) * 2`, `SinglePlayer.asm:2293-2378`); the offset in the file is 0x20718.
State `Gallery.status` (`:19-35`: active, index, fade_length, music_index, idle, idle_index, idle_timer, previous_screen). Controls (`main_`
:929-1186): stick X >= +-40 of any player or D-pad left/right change the picture (fades 45 normal, 4 quick, 100 idle), A changes the
picture's song (`bgm_table`, `[count, bgm ids...]`; index 0 plays the Gallery BGM), START toggles idle mode (a shuffled `idle_table` of 40
entries every `IDLE_TIME` 150 s with random music), L/R/Z play the 18-pair drum kit (`fgm_drum_kit`), B leaves (index saved in
`Toggles.entry_view_gallery + 4`, back to `previous_screen`). Entry: a 4th "Gallery" button on the Data menu (`data_menu.add_button_` :1339,
text X 80.0, button y 183.0, cursor wrap 4 positions), or Other Screens > View Gallery.

Port: `mn/mncommon/mnremixgallery.{c,h}` (state, paging with the D-pad and the stick edge, A for the picture's songs, START for the
idle modes, L/R drums and Z for the next kit through `mnremixgalleryaudio.{c,h}`, B to the previous scene, scene reload per page) and
`mnremixgalleryrules.{c,h}` (the `id_table` as Character.id = port fkind through `REMIX_ID_TO_PORT_FKIND`, `custom_victory_file_table` and every
`set_ending_image` call, Lanky's last call wins, the song lists, the idle order, the drum kit); `mncongra.c` hands its input to the gallery,
uses the 45/4/100 fades and plays no Congratulations voice on a Gallery picture, and uses the rules for every Character.id from 12 (Boss,
polygons, Remix fighters; it also gives the Remix 1P ending its picture). Entries: the fourth Data button (`mn/mndata/mnremixdatagallery.c`,
label bitmap at 0x4EE8 of the Data menu file, B returns to Data), Remix > Other Screens > View Gallery in the port menu (first picture
chosen there, B returns to the Options screen) and `SSB64_START_GALLERY=1` (return scene `nSCKindData`, `scmanager.c`). Tests:
`l5_data_gallery` (Data, Gallery with BGM 281, B back to Data), `l5_gallery_ctl` (L 0x20, R 0x1F, Z then L 0x11F, A song 6, D-pad right
to the DK picture with the scroll sound and no announcer), `l5_screens_gallery` (the menu routine opens the Kirby picture).

---

## 11. Title, boot, music, demo

- **Title version** (`Boot.asm:155-169`): "Smash Remix v2.0.1", centred at (160, 218), colour `0x888800FF`, scale 0.9375. Port:
  `mn/mncommon/mnremixtitleversion.*`, `remix_hud_text` slot 3, put every frame by `mnTitleFuncRun`. The larger title logo rows (0x245C8, 0x3DC88)
  are in `mntitle.c`. The Remix splash (`Boot.asm:18-53`): the port never shows the N64 logo (issue #260), so every entry into the
  startup scene goes to the Congratulations scene with Character.id NONE (`scmanager.c`), which shows the splash image (file
  0x95E/0x95F) for 512 frames with the jingle (FGM 0xA0, other sounds with L held, `mn/mncommon/mnremixsplash*.c`) and then the title, as
  the asm does after the logo (also after each attract loop). Test `l5_splash`.
- **Menu music** (`BGM.asm alt_menu_music_` :492 and `_prevent_stop`): helper `decomp/src/gm/gmremixmenumusic.{c,h}` and `sys/audio.c`
  `syAudioGetBGMPlaying` (the port's `BGM.safe_id`).

| Screen | Vanilla site | Hook |
|---|---|---|
| mode select, title entry | `mnModeSelectFuncStart` | `gmRemixMenuMusicPlay(ModeSelect, scene_prev == nSCKindTitle)` |
| options, data, VS mode, 1P mode (back from sub-screens) | `mnOptionFuncStart`, `mnDataFuncStart`, `mnVSModeFuncStart`, `mn1PModeFuncStart` | `gmRemixMenuMusicPlay(ModeSelect, FALSE)` |
| VS, 1P, Training, Bonus CSS | `mnPlayersVSFuncStart`, `mnPlayers1PGameFuncStart`, `...TrainingFuncStart`, `...BonusFuncStart` | `gmRemixMenuMusicPlay(BattleSelect, FALSE)` |
| back from the four CSS (`_prevent_stop`) | the `...BackToVSMode` / `...BackTo1PMode` functions | `gmRemixMenuMusicStop()` |

  Hooks were matched by screen and by the `syAudioPlayBGM`/`syAudioStopBGMAll` call the Remix hook replaces (RAM ranges collide between overlays,
  so the decomp `// 0x` comments were not used). `gmRemixMenuMusicKeepsCurrent` is the "do not restart the playing track" rule (nothing is kept when
  no track plays or a 64 track plays; RANDOM, RANDOM CLASSICS and RANDOM ALL keep any other track). Menu Music values include the Remix menu
  tracks; RANDOM ALL (Menu Music 21, `BGM.asm:635-668`) picks from the Random Music list ignoring its toggle (`grRemixRandomMusicPickForced`),
  or one of four hidden GameBoy songs (BGM 0xA2-0xA5) when the list is empty; the list includes the custom MIDI entries (see audio.md section 5.1). Not hooked (Remix
  has no `alt_menu_music_` call there): sound test, results, Data characters/record, stage select, item switch. The Settings L shortcut
  `handle_sss_shortcut` is not ported. Test: `SSB64_START_SCENE` of each screen with Menu Music set through `SSB64_REMIX_TOGGLES`; the track must
  not restart when entering the CSS and returning.
  Easter egg `itsatrap` (`Toggles.asm play_menu_music_ _sm64_trap_check`, `BGM.asm _check_current_trap`): an A press on Menu Music counts the
  presses that resolve to FILESELECT_SM64 (value 7, or a RANDOM value that rolls 7); the fourth in a row plays `MAIN_TRAP_SM64` (260) and the
  counter becomes -1, a press that resolves to any other track clears it, and the press after the trap restarts the count at 1. While the
  counter is -1 every path that resolves value 7 (the CSS and menu screens, RANDOM) plays 260, and 260 counts as a listed menu track (kept
  on entering a screen, not stopped by `gmRemixMenuMusicStop`). Port: `gmRemixMenuMusicAudition` and `gmRemixMenuMusicTrackOf`
  (`gmremixmenumusic.c`), counter rule `gmremixmenumusicrules.c`; the counter lives in the process and starts at 0, as the asm RAM word.
  Profile-load resets (`Toggles.asm:725, 2216`) call `play_menu_music_` in the asm and would count a press; the port menu does not route
  them through `gmRemixMenuMusicAudition`, so they do not count (deviation). Test: Menu Music = FILESELECT_SM64 and four A presses in the port
  menu with `SSB64_BGM_LOG=1`: the fourth logs `[bgm] play ... track=260`, the fifth `track=259`.
- **Music title at match start** (`BGM.asm show_music_title_`): at the GO lights (`ifCommonCountdownMakeInterface`, 0x8011268C) the asm draws a
  translucent rectangle (1, 156, 320 x 28, `0x34343494`) with the track line (scale 1.0, note glyphs) and the game line (scale 0.75) in Kabel,
  destroyed at the GO! text. Port: `gm/gmremixmusictitle.c` checks the `show_music_title` and `play_music` toggles, reads
  `syAudioGetBGMPlaying(0)` and puts the rectangle (`remix_hud_text` rect slot 2) and both lines (slots 71 and 72, centred at (160, 158) and
  (160, 172)) every frame; the rows (`port/remix/remix_music_titles_data.inc`, 308 of 375 BGM ids, generated by
  `tools/gen_remix_music_titles.py` from `BGM.asm`, `Toggles.asm`, `midi.asm`) come from `remix_music_title.cpp`. Menu tracks and extra MIDIs have
  no row, as in the asm. Test: a VS match with `SSB64_REMIX_TOGGLES='Music Title at Match Start=1'` shows "Dream Land / Kirby Super Star" at
  the lights.
- **VS demo** (`VsDemo.asm`): lets Remix fighters and stages appear in the auto-demo: `enable_focused_remix_chars_` (0x80131CDC, end of
  `mnTitleSetDemoFighterKinds`), `enable_nonfocused_remix_chars_` (0x8018D928, `scAutoDemoGetFighterKind` for players >= 2),
  `enable_remix_stages_` (0x8018DA20, `scAutoDemoInitDemo`) and a name table fix (`SinglePlayer.name_texture_table`, +0x10; 0x8018DBD4 and
  0x8018DC08 in `scAutoDemoInitSObjs`). Port: `sc/sccommon/scremixvsdemo.{c,h}` (`scRemixVsDemoPickFocusFighters`: two distinct Remix fighters with
  `variant_type == REMIX_VARIANT_NA` from `kRemixRoster`, unlock masks ignored; `scRemixVsDemoPickNonFocusedFighter`; `scRemixVsDemoPickStage`: a
  random stage id in [0, 0xD7) of class BATTLE in `Stages.class_table`, never Rest Area 0x8B, `0xD7` itself never drawn as in the asm), called from
  `mntitle.c:360`, `scautodemo.c`. The Remix pick overrides `SSB64_DEMO_FKINDS`. The name plates come from
  `SinglePlayer.name_texture_table` + 0x10 for every fighter (`scRemixVsDemoGetNameTextureOffset` in `scAutoDemoInitSObjs`), and port 4
  never repeats port 3's fighter (`scRemixVsDemoPickNonFocusedFighter`). Test `l5_vsdemo` (Dedede's own plate at frame 900).
- **Sound test**: `mn/mndata/mnremixsoundtest.c` extends the music and voice lists (`MIDI.extended_music_map_table`,
  `FGM.extended_voice_map_table`; `augment_sound_test_*`); inputs `soundtest_browse.txt` / `soundtest_wrap.txt` with `SSB64_START_SCENE=59`.
  With the second player tag named `afk` (`FGM.asm afk_soundTest`) the screen presses right and A by itself after 7 minutes of a song,
  counting while `syAudioGetBGMPlaying` (the port's `BGM.safe_id`, cleared only by a stop, not when a song without a loop ends) has a
  track (`sc1premixenemyafk.c`). Test `l5_soundtest_afk` (track 33, then 34 after 25 200 frames).
- **Render groups**: `Render.asm toggle_group_display_` walks the list at `GROUP_TABLE[group]` (= `gGCCommonLinks` in the port) and writes `hide`
  to every node; port `gm/gmremixgroupdisplay.{c,h}` (`gmRemixGroupDisplaySet`, `gmRemixGroupDisplayTempSet` for `SpecialZoom.asm
  temp_toggle_group_display_`). Groups in use: 0x0D menu instructions, 0x0E HUD, 0x0F and 0x10 pause legend, 0x16 results rows. `Pause.asm
  disable_hud_` runs `ifCommonBattlePauseMakeSObjsAll` then hides groups 0xE, 0xF and 0x10 when the Disable HUD setting is above 0.
- **Button and extension sprites**: the Remix HUDs reuse the button images of the pause file 0xC5 (A 0x958 and B 0xA88 13x13 IA8 with stride 16, L
  0x18C8 and R 0xCF8 16x12, Z 0xBD8 9x15, PLUS 0x4D8 8x8 I4, L_THIN 0x1B10, "Legend" 0x19F8 42x9 I4), file 0xA06 (D-pad 0x218 16x16 RGBA16, arrows 0x688,
  0x1958, 0x1A88, 0x1BB8 13x13 IA8), the stock icons of file 0x19 (polygon icon 0x80) and file 0x14F3 (King of the Hill frame 0xAA8 64x16). Colour is
  the vanilla SObj path: `TEXTURE_INIT_` is `lbCommonMakeSObjForGObj`, `TEXTURE_RENDER_` `lbCommonDrawSObjAttr`, the colour argument becomes PRIM
  and the palette argument ENV (IA combiner `RGB = ENV + (PRIM - ENV) * I`); example, the Match Stats A button is tint `0x50A8FFFF`, palette
  `0x0010FFFF`. Port: `if/ifremixsprite.{c,h}` and `ifremixspriterules.{c,h}` (offset and format table, pure) draw a native SObj with tint and
  palette and hide/show it; no atlas. Files 0xA05/0xA06/0x14F3 are in the `.o2r` as `reloc_misc/file_2565`, `file_2566`, `file_5363`.

---

## 12. Standalone patches with a menu or setting

| Asm | Behaviour | Port |
|---|---|---|
| `AerialAttackFastFall.asm` | five hooks at 0x80129E38-0x80129E88 swap the Proc Physics of AttackAir N/F/B/Hi/Lw (`ftPhysicsApplyAirVelDrift`) for `...FastFall` (0x800D9160) when `Fast Fall Aerials` is on (Gameplay Settings, default off) | `ft/ftremix/ftremixaerialfastfall.c` via `ftMainSetStatus`: swaps the physics only for those five statuses of `dFTCommonNullStatusDescs` and reads the toggle each frame |
| `MagnifyingGlass.asm` | `apply_custom_zoom_` (0x80110E80) scales the magnified model by `Character.magnifying_glass_zoom` per character id (custom matrix per port) | `gm/gmremixmagnify.c`, wired at `ifcommon.c` |
| `SFXReplace.asm` | on Flat Zone (0x32) and Flat Zone 2 (0x62), in VS (0x16), Training (0x36) and Remix modes (0x77), the footstep (23 ids), landing (12) and hard landing (7) sounds become gaw sounds 0x602, 0x603, 0x604 (no setting); the down bounce becomes 0x604 on those stages | `ftremixsfxreplace.c`: runs before `ftRemixJapaneseMotionSoundFgm` in the PlayFGM case (hook 0x800DF864 before 0x800DF86C); `ftCommonDownBounceUpdateEffects` for the bounce |
| `Practice.asm overlay_colour_` | tints the fighter by state when `Color Overlays` is on: yellow when actionable (standing, turn, landing after the FAF), blue intangible, green invulnerable, red hitstun, cyan reflect/absorb; the Falcon turn check compares the wrong register so Falcon gets the slow FAF 5 (Remix behaviour kept) | `ftremixoverlay.c` (`ftRemixOverlayColour` at the start of `ftMainProcPhysicsMapDefault`), flags in two static arrays |
| `Practice.asm flash_on_z_cancel_` | flash on a Z-cancel landing | `ftremixzcancel.c` `ftRemixZCancelFlashOnLanding` |

Fighter-side panel options (Skeleton, Model, Size, Dpad, ...) are in `docs/remix/panel.md`.
