# F11 mode research: Remix 1P modes (singleplayer), design note for the port (ROADMAP M6)

Sources (read only): `.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm` (8777 lines), `HRC.asm` (803),
`SinglePlayerMenus.asm` (1068), `Bonus.asm` (894). Decomp side checked: `decomp/src/sc/sc1pmode/` and `decomp/src/mn/mn1pmode/`.
Earlier notes read: `docs/remix/menus.md` 3.2 and 3.3, `docs/remix/notes/f10_menu_vsmodes.md`, `docs/remix/notes/f10_panel_stock_practice.md`.

Coverage: SinglePlayerModes.asm was read in part for the mode flag, the ID table, bonus2 CSS (130-266), the start of
`remix_1p_randomization` (3577-3686), the start of `allstar_randomization` (6510-6600), the Multiman random and KO-count hooks
(2186-2283), `progress_advancement`, `screen_set_end`, `heart_counter`, `advance_allstar`, `end_allstar`, `game_over_end`,
`_final_stage_spawn`. HRC.asm was read at 1-120, 200-349 and 419-508, and indexed for the rest. The rest of the file
(title card, metal bros, giant, kirby, moveset tables, rest area, Master Hand / Giga Bowser, stock/percent display) was indexed by scope
name and description comment only. Anything marked "indexed only" has not been checked line by line.

Line numbers below are `SinglePlayerModes.asm` unless a file is named. Addresses are the patch point (`OS.patch_start(ROM, RAM)`)
as written in the asm; RAM addresses are the second value.

---

## 0. Scope table: SinglePlayerModes.asm

Ranges start at the scope line and end where the next row starts. Rows marked "indexed only" were not read in full.

| Lines | Scope / block | Purpose |
|---|---|---|
| 1-19 | header comments | RAM notes for 1P init, CSS platform counts, Bonus 2 CSS header |
| 20-108 | FLAGS | `singleplayer_mode_flag` (31), `reset_flag` (38), `new_record_flag` (45), `page_flag` (52), `player_ko` (59), `allstar_progress` (64), `allstar_hearts` (71), `allstar_percent` (77), `end_game_flag` (83), `match_begin_flag` (89), `allstar_limbo` (95), `duo_spawn` (101), `metal_bros_flag` (107) |
| 110-122 | CONSTANTS | Mode IDs (114-119), `MENU_INDEX` 0x801331B8, `STAGE_FLAG` 0x800A4AE7, `PLAYER_PERCENT` 0x800A4B84, KO pointer 0x801936A0 |
| 130-266 | `bonus2_css_2_1p` | Bonus 2 CSS exit goes to 1P for Multiman, Cruel, Bonus 3, Remix 1P, All-Star, HRC; random-character and costume roll |
| 277-375 | `opponent_type`, `battle_style` | Opponent type RAM addresses; horde vs normal battle style byte |
| 375-500 | `_1p_stage`, `_team_attack`, `_item_spawn`, `_hmn_knockback` | Per-mode stage load, team attack, item rate, handicap ratio |
| 522-670 | `_opponent_knockback`, `_opponent_number_1/_2`, `_opponent_2` | CPU knockback and opponent count per mode |
| 671-733 | `_time_limit`, `_percent_fix` | Time limit routine per mode; percent display fix at 34 to 35 KOs |
| 764-842 | `_spawning_style` | CPU spawn routine and camera per mode |
| 845-863 | `spawn_human_left_remix_rttf_` | Human faces left in Remix RTTF |
| 864-888 | `_initial_screen_set` | Skips the title card for Multiman and All-Star |
| 891-988 | `_reset_button`, `_reset` | Reset button graphic and reset flag for Bonus 3, Multiman, Cruel |
| 989-1170 | `_load_timer`, `bonus3_timer` | Bonus 3 timer |
| 1172-1208 | `_end_match_screen` | Multiman and Bonus 3 end returns to Bonus 2 CSS |
| 1210-1297 | `_ko_save` | Saves KO count to SRAM for Multiman, Cruel, All-Star, Remix 1P |
| 1300-1430 | `_time_save`, `_new_record` | Saves time for Bonus 3 and HRC; new record sound flag |
| 1442-1487 | `_opponent_behavior` | Polygon-style opponent behaviour |
| 1489-1603 | `_infinite_spawn_1/2`, `_death_crash`, `_infinite_ko` | Infinite spawns and no-KO-end for Remix 1P and All-Star |
| 1605-1690 | `_preview_skip`, `_preview_song_skip` | Multiman skips the preview timer, button check and song |
| 1693-1910 | `skip_cflag_music_function`, `set_bgm`, `skip_cflag_music_function_2` | Music control for the modes |
| 1913-1957 | `load_player_stocks` | Stock count for Multiman |
| 1959-2031 | `load_1p_difficulty_1/_2`, `_opponent_cpu` | 1P difficulty and CPU level |
| 2086-2121 | `crash_prevent`, `stage_flag` | Crash over 30 KOs; C-flag mode for Multiman, B-flag for Bonus 3 |
| 2155-2184 | `icon_removal` | Hides polygon icons in 1P |
| 2186-2221 | `multiman_random` | Polygon character pick for Multiman and Cruel (see mode notes) |
| 2224-2268 | `multiman_ko_count_stop` | Stops the KO counter once the player is KO'd (bug, see section 8) |
| 2272-2283 | `player_ko_refresh` | Clears `player_ko` each match |
| 2287-2401 | `setup_` | Multiman, Cruel and HRC on-screen display (stock icon, KO counter, Dragon King HUD helpers at 2316) |
| 2404-2714 | `master_hand_`, `master_hand_music_fix_`, `giga_fix_`, `giga_stamina_check_`, `master_hand_HP_display_`, `giga_change_action_`, `giga_change_action_2_`, `no_laugh_` | Master Hand and Giga Bowser for the Remix 1P final stage (indexed only, except the scope names) |
| 2716-2990 | team settings | Fighting Polygon, Remix and Tiny team match settings (2829, 2884, 2932) |
| 2990-3214 | per-character settings | Match settings for each Remix 1P fighter, pulled by randomization (2991 onward) |
| 3215-3350 | ALLSTAR ONLY | All-Star match settings |
| 3351-3510 | Lucas, Dark Samus, Peppy, Lanky settings | Remix 1P fighter settings (indexed only) |
| 3400-3510 | `progress_icon` | Progress icon table |
| 3511-3576 | random slot tables | `match_slots`, `title_card_1_struct` and related tables that randomization overwrites |
| 3580-3941 | `remix_1p_randomization` | Remix 1P path generator: metal bros reset, All-Star branch, Kirby pool, duo select, six random slots (hook 0x1402E8 / 0x801380E8) |
| 3942-4025 | `duo_select` | Picks duo fighters for the random slots |
| 4026-4131 | `polygon_selection_match`, `_rttf` | Remix polygon selection for normal and RTTF matches |
| 4132-4295 | `polygon_file_loading`, `polygon_allocation` | Remix polygon file loading and allocation |
| 4297-4483 | `title_card_text_name`, `title_card_text_routine` | Title card name text (Marina Liteyears to Marina; team and giant text) |
| 4486-4571 | `title_card_text_position`, `title_card_text_location_routine` | Overall and team/giant text positions |
| 4574-4721 | `title_card_giant_announce`, `title_card_yoshi_team_skip`, `title_card_flag_refresh` | Giant and team announce |
| 4762-4852 | `title_card_voice_name`, `title_card_model_scale`, `title_card_1/_2` | Title card voice, scale, character load |
| 4855-4913 | `remix_1p_pointer_part1/_part2` | Stage settings pointers for Remix 1P and All-Star (hooks 0x10BE90, 0x10BEE0) |
| 4916-4951 | `remix_icon_pointer` | Remix progress icon source |
| 4952-5111 | `metal_bros_magic`, `metal_bros_bug_fix`, `metal_bros_text`, `metal_bros_stage_text` | Metal Bros boss: doubles load, title card, text |
| 5114-5171 | `remix_1p_giant` | CPU 1 is a giant in Remix 1P |
| 5173-5300 | Remix Kirby pool, Kirby order | Kirby power and hat order (5273, 5287) |
| 5303-5399 | `remix_yoshi_card`, `_shade_skip`, `_costumes` | Yoshi title card and team costume colours |
| 5400-5487 | `remix_team_shade_start`, `remix_team_shade_ko` | Same-colour CPU shading |
| 5491-5640 | `tiny_team_screen_card` | Three tiny opponents for Tiny Team |
| 5643-5732 | `doubles_card_1/_2`, `starfox_costume` | Doubles title card order; Luigi and Fox colours |
| 5735-5822 | `doubles_file_loading`, `giant_file_loading`, `giant_card` | File loading for doubles and giant stages |
| 5823-5919 | `remix_kirby_card`, `_selection`, `_skip`, `_selection_initial`, `remix_1p_kirby_load` | Kirby hats and powers use Remix characters |
| 5941-6017 | `_final_stage_spawn` | Final-stage spawn: Remix 1P keeps the standard spawn (hook 0x10C08C / 0x8018D82C) |
| 6021-6079 | `_giga_usp_prevent` | Giga Bowser up special blocked (indexed only) |
| 6080-6103 | `game_over_end` | Clears `page_flag` and the 1P settings at game over (hook 0x17A3F0 / 0x80133990) |
| 6105-6509 | moveset and menu action tables | Defeated and duo character moveset and parameter tables (indexed only) |
| 6510-6944 | `allstar_randomization` | All-Star: builds the character and stage order and clears progress state (6516) |
| 6954-7077 | `player_percent`, `clear_percent`, `clear_percent_continue` | All-Star percent carry-over |
| 7080-7148 | `rest_area_and_hrc_spawning`, `_fix` | No CPU spawn in Rest Area and HRC CPU does not move |
| 7153-7248 | `instant_spawn`, `limbo_clear`, `skip_go` | All-Star spawn and limbo handling (indexed only) |
| 7252-7408 | `icon_offsets` | Icon offsets by character ID |
| 7411-7810 | `rest_area_setup`, `rest_area_routine` | Rest Area portraits and heart spawns (indexed only) |
| 7814-7846 | `heart_persist` | Hearts kept between All-Star fights |
| 7850-7886 | `progress_advancement` | 1P stage progression for All-Star (hook 0x52530 / 0x800D6D30) |
| 7891-7913 | `screen_set_end` | Screen after an All-Star match; Practice reset restarts (hook 0x523E8 / 0x800D6BE8) |
| 7918-7945 | `heart_counter` | Counts hearts used in Rest Area (hook 0xC06E4 / 0x80145CA4) |
| 7950-7983 | `advance_allstar` | Adds 1, 2 or 3 to All-Star progress per win (hook 0x17DF18 / 0x80134CC8) |
| 7988-8026 | `pause_exit` | Pause exit for All-Star (indexed only) |
| 8029-8083 | `display_percent`, `record_percent` | All-Star percent display and record |
| 8085-8107 | `end_allstar` | Forces the final stage ID 0x0D when `end_game_flag` is set (hook 0x10F768 / 0x80190F08) |
| 8109-8177 | `prevent_erase`, `stock_prevent`, `damage_prevent` | Keeps score, stock and damage across All-Star matches |
| 8179-8214 | `costume_fix` | Costume choices reduced so vanilla characters work |
| 8216-8271 | `no_miss`, `no_miss_2`, `no_calamity` | No Miss and Calamity points not awarded |
| 8305-8450 | `_nSC1PGameStageAllstarFinal_` | All-Star final stage (indexed only) |
| 8453-8777 | enemy team sprites, stock loading, Yoshi path, final costume and character fixes | Indexed only |

Other files:

| File | Lines | Purpose |
|---|---|---|
| `HRC.asm` | 10-50 | `HRC` scope: constants (world units, camera warp, projectile warp, complete timer 45 frames), state `distance`, `warp_distance`, `complete_timer`, string pointers |
| `HRC.asm` | 52-417 | `main_`: per-frame camera follow, distance, end-of-attempt check (section 2) |
| `HRC.asm` | 419-500 | `update_projectile_and_hmn_positions_`: projectile hiding and wrap |
| `HRC.asm` | 505-580 | `initialize_signs_`: distance signs, animation tracks off |
| `HRC.asm` | 565-745 | Hook groups: pawn visible, countdown, Are You Ready, blastzones, bat despawn, volume, background |
| `SinglePlayerMenus.asm` | 9-100 | `SinglePlayerMenu` scope: `menu_button_table` (82-89), `remix_menu_button_table` (91-97) |
| `SinglePlayerMenus.asm` | 100-400 | Button creation, cursor wrap, `page_2_exit` (341), flag writes on select (417-423) |
| `SinglePlayerMenus.asm` | 440-760 | Match start: sets mode IDs for Multiman (463), Cruel (470), HRC (473), Bonus 3 (511); HRC high score (681) |
| `SinglePlayerMenus.asm` | 800-1068 | Header graphics, remix 1P header (807), original-flag and BTP/FGM hooks |
| `Bonus.asm` | 6-225 | `Bonus` scope, `NUM_BONUS_STAGES` 33 (line 7), `setup_` (11) |
| `Bonus.asm` | 227-410 | `handle_header_toggle_` (Normal 0 / Remix 1), `arrow_routine_`, `check_input_` |
| `Bonus.asm` | 411-434 | `manual_remix_bonus_start_`: Remix selection disables automatic start |
| `Bonus.asm` | 438-562 | `handle_arrow_pre_` (mode and colour tables) |
| `Bonus.asm` | 564-643 | `update_stage_and_icons_` |
| `Bonus.asm` | 645-677 | `get_bonus_stage_index_`: BTT (a1 = 0) or BTP (a1 = 1) lookup, up to 33 entries |
| `Bonus.asm` | 679-737 | Strings ("REMIX" at 679), mode, colour and level tables |
| `Bonus.asm` | 739-777 | `btt_stage_table` (33 entries, Remix fighters included) |
| `Bonus.asm` | 778-end | `btp_stage_table` (indexed only, checked for count) |

---

## 1. What the modes are and how they are selected

Selection is one byte/word, `SinglePlayerModes.singleplayer_mode_flag` (SinglePlayerModes.asm:31), with IDs at lines 114-119:

| ID | Constant | Mode |
|---|---|---|
| 0x0 | (none) | Standard 1P |
| 0x1 | `BONUS3_ID` | Bonus 3 (timed, HRC-like timer) |
| 0x2 | `MULTIMAN_ID` | Multiman |
| 0x3 | `CRUEL_ID` | Cruel Multiman |
| 0x4 | `REMIX_1P_ID` | Remix 1P |
| 0x5 | `ALLSTAR_ID` | All-Star |
| 0x6 | `HRC_ID` | Home-Run Contest |

The flag is written by the menu when a button is selected. In `SinglePlayerMenus.asm`, `menu_button_table` and `remix_menu_button_table` rows carry the value to write (byte 2 of the row) and the page flag (byte 3). Page 1 has Bonus 3 (`BONUS3_ID`) and the normal buttons. Page 2 (`page_flag` = 1, the Remix Modes page) has Remix 1P, All-Star, Multiman, Cruel Multiman and HRC (lines 91-96). Selecting a row writes `singleplayer_mode_flag` and `page_flag` (SinglePlayerMenus.asm:417-423). `SinglePlayerMenu` clears the flag on leave (`remove_flag`, hook 0x11EBE8, SinglePlayerMenus.asm:15-22).

Rule summaries, taken from the asm and the HRC code:
- Remix 1P: a randomised 1P path with fighters and stages written into match settings (see 3.2). The gameplay rules beyond that were not read in this pass (inferred: standard 1P rules).
- All-Star: one continuous run through a shuffled list of characters and stages, with hearts (Rest Area) and percent carried between fights. Winning a stage advances progress; the run ends at the final stage (ID 0x0D) or on a loss (inferred from `end_allstar` and `screen_set_end`).
- Multiman: fights a series of CPUs with a KO count shown on screen; the KO counter stops once the player is KO'd. Cruel Multiman uses the same code with `CRUEL_ID` checks in each hook.
- HRC: the player knocks a sandbag (CPU type 5) along; distance is measured from `START_X`; the attempt ends with COMPLETE or FAILURE (section 4.4).

---

## 2. Flow

Flow for the Remix 1P menu: Main menu -> 1P menu page 1 -> "Remix Modes" (page 2, `page_flag` = 1) -> mode button (sets `singleplayer_mode_flag`) -> Bonus 2 style CSS (`bonus2_css_2_1p`) -> 1P CSS -> `remix_1p_randomization` on Start -> stage/match -> results, `game_over_end` clears `page_flag` at game over.

Hooks by mode. "Decomp match" means a vanilla function with the same screen and name in `decomp/src`. Only the menu hooks the earlier notes verified are listed as matched.

| Asm file:line | Patched address (ROM / RAM) | What it does | Decomp match |
|---|---|---|---|
| SinglePlayerModes.asm:131 | 0x14CAF8 / 0x80136AC8 | Bonus 2 CSS exit sets up 1P for the new modes | not checked |
| SinglePlayerModes.asm:199 | (called from 131) | `_1p_stage_setting`: sets 1P port, character and stage; random character rolls a costume | not checked |
| SinglePlayerModes.asm:3581 | 0x1402E8 / 0x801380E8 | `remix_1p_randomization` on 1P CSS Start | not checked |
| SinglePlayerModes.asm:6516 (called from 3610) | none (direct call) | `allstar_randomization` for All-Star | n/a |
| SinglePlayerModes.asm:2187 | 0x10CB6C / 0x8018E30C | `multiman_random` polygon pick | not checked |
| SinglePlayerModes.asm:2225 | 0x10D97C / 0x8018F11C | KO counter stop | not checked |
| SinglePlayerModes.asm:2273 | 0x10C914 / 0x8018E0B4 | clears `player_ko` each match | not checked |
| SinglePlayerModes.asm:5942 | 0x10C08C / 0x8018D82C | final-stage spawn | not checked |
| SinglePlayerModes.asm:6081 | 0x17A3F0 / 0x80133990 | game over: clears `page_flag` | not checked |
| SinglePlayerModes.asm:7891 | 0x523E8 / 0x800D6BE8 | All-Star screen after match | not checked |
| SinglePlayerModes.asm:7850 | 0x52530 / 0x800D6D30 | All-Star stage progress | not checked |
| SinglePlayerModes.asm:7918 | 0xC06E4 / 0x80145CA4 | heart counter in Rest Area | not checked |
| SinglePlayerModes.asm:7950 | 0x17DF18 / 0x80134CC8 | All-Star progress advance | not checked |
| SinglePlayerModes.asm:8085 | 0x10F768 / 0x80190F08 | All-Star end (stage 0x0D) | not checked |
| SinglePlayerMenus.asm:16 | 0x11EBE8 / 0x80132AD8 | `remove_flag`: clears the mode flag on leave of menu | not checked |
| SinglePlayerMenus.asm:341 | 0x11ED70 / 0x80132C60 | `page_2_exit` | not checked |
| SinglePlayerMenus.asm:463-473 | (set values in match start) | sets Multiman / Cruel / HRC IDs before match | not checked |
| HRC.asm:565 | 0x10E454 / 0x8018FBF4 | pawn visibility for HRC | not checked |
| HRC.asm:583 | 0x8EA00 / 0x80113200 | countdown | not checked |
| HRC.asm:616 | 0x10D1BC / 0x8018E95C | "Are You Ready?" | not checked |
| HRC.asm:642 | 0x5DAF0 / 0x800E22F0 | blastzones off in HRC | not checked |
| HRC.asm:666 | 0xEE64C / 0x80173C0C | bat not despawned | not checked |
| HRC.asm:706 | 0x8EB6C / 0x8011336C | volume reduction off | not checked |
| HRC.asm:730 | 0x8000C / 0x8010480C | background animation (also Big Blue) | not checked |
| Bonus.asm:228 | 0x149388 / 0x80133358 | header toggle Normal/Remix | not checked |
| Bonus.asm:414 | 0x14CABC / 0x80136A8C | manual Remix bonus start | not checked |

The decomp has `sc/sc1pmode/sc1pgame.c` (for example `sc1PGameFuncStart`, `sc1PGameSetupStageAll`, `sc1PGameWaitStageBonus3Update`) and `mn/mn1pmode/mn1pmode.c` (`mn1PModeMake1PGame` at 0x80131E34 is the one verified match, per menus.md). The screen names do not match the hook addresses above, so no other row has a verified vanilla match. Mark those as "not matched in this pass" when used.

---

## 3. State and data

### 3.1 Globals

| Name | Where | Size | Writers |
|---|---|---|---|
| `singleplayer_mode_flag` | :31 | byte (padded to 4) | SinglePlayerMenus (select, leave), match start |
| `reset_flag` | :38 | byte | `_reset` (Bonus 3, Multiman, Cruel) |
| `new_record_flag` | :45 | byte | `_new_record` |
| `page_flag` | :52 | byte | SinglePlayerMenus (select), `game_over_end` clears it |
| `player_ko` | :59 | byte | `multiman_ko_count_stop` reads, `player_ko_refresh` clears |
| `allstar_progress` | :64 | word | `allstar_randomization` clears, `advance_allstar` adds |
| `allstar_hearts` | :71 | word | `allstar_randomization` clears, `heart_counter` increments |
| `allstar_percent` | :77 | word | `player_percent`, `clear_percent`, `record_percent` |
| `end_game_flag` | :83 | word | `allstar_randomization` clears |
| `match_begin_flag` | :89 | word | `allstar_randomization` clears |
| `allstar_limbo` | :95 | word | `allstar_randomization` clears |
| `duo_spawn` | :101 | byte | (indexed only) |
| `metal_bros_flag` | :107 | byte | `remix_1p_randomization` clears |
| `HRC.distance`, `warp_distance`, `complete_timer` | HRC.asm:30-41 | words | `HRC.main_` |
| `HRC.counter_string`, `metric_string` | HRC.asm:45-48 | pointers | HRC init |
| `bonus stage tables` | Bonus.asm:739, 778 | bytes, 33 entries each | static |

Fixed RAM used by the modes (from the asm): `STAGE_FLAG` 0x800A4AE7 (current 1P stage progress, used by All-Star and heart counter), `PLAYER_PERCENT` 0x800A4B84, `KO_AMOUNT_POINTER` 0x801936A0, `TIME_SAVE_POINTER` 0x800A4B30, game score 0x800A4AF0 (cleared in `allstar_randomization`), HRC frame counter 0x800A4B18 + 0x18.

### 3.2 Tables and pools

- `match_pool` (per character, stride 0x18): filled in the `allstar_randomization` and `remix_1p_randomization` loops. Each entry holds a used flag at +0x00 and the character ID at +0x04, and a stage at +0x04 onward.
- `allstar_character_order` and `allstar_stage_order`: the output of `allstar_randomization`. Character IDs are stored as halfwords. The stage slots are halfwords too.
- `all_star_stage_used_table`: byte per stage ID, used to avoid repeats.
- `remix_kirby_pool` (12-byte slots, power ID +0x00, hat ID +0x04, used flag +0x08), `remix_kirby_hat_pointer`, `remix_kirby_pointer`: 7 random slots plus a fixed Magic Hat slot (power 0x1C, hat 0x25).
- `MATCH_SETTINGS_PART1`: the match settings block that randomization writes into. Boss character slot is at +0xA0 (set to 1 to stop a Metal Luigi spawn).
- `match_slots`, `title_card_1_struct`: the six random slots for Remix 1P.
- Remix 1P per-character settings (2990-3214) and `progress_icon` (3400): data only.
- `btt_stage_table`, `btp_stage_table` (Bonus.asm): 33 stage IDs each, the Remix fighters included.

### 3.3 Files and sprites

The modes load file IDs as plain constants in the asm. The following were visible in the code read so far:

- `Render.load_file(0x19, ...)` in `setup_` (SinglePlayerModes.asm:2303): button images for the Multiman display. This is a vanilla file ID in the asm, not checked for Remix-only status.
- `Render.draw_texture_at_offset(0x17, 0x0B, ...)` in `setup_` (2312-2313): polygon stock icon and X glyph, file 0x17.
- `Render.draw_number(0x17, 0x0B, KO_AMOUNT_POINTER, ...)` (2314): the KO counter.
- `Render.load_font()` (2302).
- File 0xB: progress icon offset file (3558).
- The Kirby, title card and giant files are in the indexed-only sections, not listed here.

Remix-only file IDs were not separated out in this pass. The asm uses numbers only.

### 3.4 HRC constants

From HRC.asm:11-26 (world units, 1 foot tenth = 9.12 units):
- `START_X` 0xC6978000 (-19392), `DEFAULT_CAMERA_X` 0xC6A68000, `BAT_SPAWN` 0xC6B01C00.
- `CAMERA_WARP_THRESHOLD` 0x4643C000 (350 ft plus START_X), `CAMERA_WARP_DISTANCE` 0xC60E8000 (-100 ft).
- `PROJECTILE_LEFT_THRESHOLD` 0xC4900000, `PROJECTILE_WARP_WINDOW` 0x468E8000 (200 ft), `PROJECTILE_LEFT_PADDING` 0xC58E8000 (-50 ft).
- `COMPLETE_TIMER_START` 45 frames.
- Cloud constants `CLOUD_START`, `CLOUD_WARP_THRESHOLD`, `CLOUD_WARP_DISTANCE`.

---

## 4. In-match logic (per frame or per event)

### 4.1 Remix 1P (`remix_1p_randomization`, 3580)

Runs once after Start on the 1P CSS.
1. Clears `metal_bros_flag`, and sets the boss slot (`MATCH_SETTINGS_PART1` + 0xA0) count to 1 to block Metal Luigi.
2. If the mode is All-Star (ID 5), calls `allstar_randomization` and returns (3605-3613).
3. If the mode is Remix 1P (ID 4): advances the RNG 1 to 64 times based on the frame count (3619-3628).
4. Builds the Kirby pool: 7 random powers and hats from `NUM_REMIX_HATS` with a used flag, then the Magic Hat slot (3630-3660). Clears the flags afterwards.
5. Calls `duo_select` (3670).
6. Fills six random slots (`countdown` 5, then 6 slots by the comment) from `match_pool`, up to `Character.NUM_REMIX_FIGHTERS` (3673-3686 and on). Remainder of the routine indexed only.

### 4.2 All-Star (`allstar_randomization`, 6516)

Runs once after Start on the 1P CSS.
1. Advances the RNG (frame count % 64 times) (6521-6530).
2. Clears `allstar_hearts`, `allstar_progress`, `allstar_percent`, `allstar_limbo`, `end_game_flag`, the game score (0x800A4AF0) and `match_begin_flag` (6532-6545).
3. Character loop (6547-6596): picks a random character from `match_pool` (pool size `NUM_REMIX_FIGHTERS` in the comment as 0x1E = 30). Skips characters already used (flag at +0x00). For each character, picks a stage: `Global.get_random_int_(3)` (three choices), skips used stages in `all_star_stage_used_table`, marks the stage used, and saves the character ID and stage ID into the order tables.
4. Clears the used flags (`_clear_loop`, after 6598).

Loop count note: the count register starts at 0x1D (29) and the delay-slot `addiu t0,t0,-1` runs in the same step as the `bnez`. Counting that way gives 30 iterations (29 down to 0). The comment on that line says "28 character slots". This must be checked against the size of `allstar_character_order` before the port (section 8).

Stage progress per win (`advance_allstar`, 7950): the progress counter advances by 1 for a single-opponent stage (below `DOUBLE_STAGE_AMOUNT`), by 2 for a double (below `TRIPLE_STAGE_AMOUNT`), and by 3 for a triple. Progress is `allstar_progress`. `progress_advancement` (7850) sets the 1P stage to Rest (0) or to the next stage, with the Rest stage (`STAGE_FLAG` 0) tracked separately. End-of-run: `end_allstar` (8085) sets the stage ID to 0x0D when `end_game_flag` is set. `screen_set_end` (7891): a Practice reset (value 2) restarts the match screen (0x34); otherwise the screen goes to the 1P menu.

Hearts: `heart_counter` (7918) adds 1 to `allstar_hearts` each time the player uses a heart while `STAGE_FLAG` is 0 (Rest Area). `heart_persist` (7814) keeps the count between fights (indexed only). Percent: `player_percent` (6954), `clear_percent` (7002), `clear_percent_continue` (7029) carry the percent. `display_percent` (8029) and `record_percent` (8057) show and save it.

Rest Area: `rest_area_and_hrc_spawning` (7080) stops CPUs from spawning and keeps the HRC CPU still in Rest Area. `rest_area_setup` and `rest_area_routine` are indexed only.

### 4.3 Multiman and Cruel Multiman

- `multiman_random` (2187): replaces the vanilla polygon pick. For Multiman and Cruel, `t4 = random(12) + 14`, so `t4` is in 14 to 25. The meaning of that range (polygon ID or index) was not confirmed. Other modes skip it.
- `multiman_ko_count_stop` (2225): stops the KO counter when `player_ko` is 1. The branch on Cruel has a bug (section 8).
- `player_ko_refresh` (2273): clears `player_ko` at the start of each match.
- Preview: `_preview_skip` (1605) skips the preview timer and button check; `_preview_song_skip` (1654) skips the song.
- Stocks: `load_player_stocks` (1913, hook 0x520C4) loads the Multiman stock count.
- Match end: `_end_match_screen` (1172) returns to the Bonus 2 CSS for Multiman and Bonus 3, both on death and on cancel. `_ko_save` (1210) saves the KO count to SRAM for Multiman, Cruel, All-Star and Remix 1P (1219-1233).
- Reset: `_reset_button` (891) shows the reset graphic and `_reset` (930) makes the reset branch work and sets `reset_flag`, for Bonus 3, Multiman and Cruel.
- C-flag mode: `stage_flag` (2122) sets Multiman to C-flag mode and Bonus 3 to B-flag mode. The comment says this loads new characters into CPU slots, shows icons that disappear when defeated, and affects camera and spawns.
- Display: `setup_` (2287) draws the polygon icon, the X and the KO counter for Multiman and Cruel. HRC gets its own display (`_hrc`, 2307).

### 4.4 Home-Run Contest (HRC.asm)

Per frame (`HRC.main_`, 52):
1. Finds the human player struct (type 0) and the sandbag struct (type 5) by walking the player list (60-70).
2. Reads the sandbag X. `s0` = 1 when the sandbag is at or beyond `START_X` (78-83).
3. Camera: while the sandbag is before `START_X`, the camera holds `DEFAULT_CAMERA_X`. After, it pans to 1/3 of the way to the sandbag each frame (89-103). If the camera is past `CAMERA_WARP_THRESHOLD`, it is warped left by `CAMERA_WARP_DISTANCE` and `warp_distance` grows (110-120).
4. Distance (89-107): `distance` = (sandbag X - `START_X` + `warp_distance`) / `UNITS_PER_FOOT_TENTH`, truncated to an integer. The constant is commented "9.12 units/0.1 ft", so the stored unit (feet or tenths of a foot) must be confirmed against the HUD before the port (section 8).
5. Ending (206-275): the attempt has "time remaining" while frames elapsed < 0x258 (600 frames, 10 s). The asm then checks whether the sandbag has stopped, and:
   - Time remaining and sandbag off the platform before `START_X`: keep going if it is in the air.
   - Time up or sandbag grounded and stopped: check Lanky Orangstand, DK Cargo, and the human action table `action_table`. If the human is in one of those actions, let it finish.
   - Otherwise the attempt goes to `_complete` or `_failure`.
6. `_complete` (277): animates the counter to centre and grows it over the complete timer (45 frames), then calls `0x80114D58` with FGM 0x1CB ("Complete!"). `_failure` (340): counts the same timer down and calls `0x80114C80`.

Projectiles (`update_projectile_and_hmn_positions_`, 419): each projectile past `PROJECTILE_LEFT_THRESHOLD` has its own distance (0x40) and previous X (0x44). When `PX - CX` is below `PROJECTILE_LEFT_PADDING`, the projectile is kept out of view at the threshold. The projectile list ends at the human player object (loop at 492 and 496).

---

## 5. HUD and text

| Element | Mode | Source | Port note |
|---|---|---|---|
| Polygon stock icon, X, KO counter | Multiman, Cruel | `setup_` (2312-2314) | Sprite plus `Render.draw_number`; needs vanilla sprite path |
| Bonus 3 timer | Bonus 3 | `_load_timer` (989), `bonus3_timer` (1031) | Vanilla timer drawn through 1P screen |
| HRC distance counter, "FT." metric | HRC | `HRC.main_` (`counter_string`, `metric_string`) | Strings created by vanilla routines; ImGui overlay only if text is needed |
| HRC high score (decimal) | HRC | `SinglePlayerMenus.draw_hrc_high_score_decimal_` (681) | Menu side, not checked in this pass |
| Remix 1P header | Remix 1P | `header_graphic_remix_1p` (SinglePlayerMenus 807, hook 0x13C5DC) | Vanilla sprite |
| "REMIX" header label | Bonus | `Bonus.string_mode_remix` (Bonus.asm:679) | String inserted with `String.insert`; port needs the text path |
| Remix team name and giant text | Remix 1P | title card scopes (4297-4721) | Vanilla title card text path |
| All-Star progress and hearts | All-Star | `heart_counter`, `display_percent` (8029) | Indexed only |
| Reset graphic | Bonus 3, Multiman, Cruel | `_reset_button` (891) | Vanilla sprite |

---

## 6. Dependencies

- StockMode tables: `stock_prevent` (8128) and `prevent_erase` (8109) keep stocks and score across All-Star matches. Multiman and Cruel use `load_player_stocks`.
- TimedStock scoring: `_time_save` (1300) has branches for Bonus 3 (1303) and HRC (1326); `_new_record` (1398) has branches for Bonus 3 (1401) and HRC (1413). `TIME_SAVE_POINTER` 0x800A4B30 is a constant in the asm.
- Stamina: Master Hand and Giga Bowser (2404-2714) use stamina and HP routines. Indexed only.
- Neutral Spawns: `_spawning_style` (764) and `rest_area_and_hrc_spawning` (7080).
- Results screen: `_end_match_screen` (1172), `screen_set_end` (7891), `game_over_end` (6081). The results path goes back to the Bonus 2 CSS for Multiman and Bonus 3, to the 1P menu for others.
- Bonus (BTT/BTP): `Bonus.check_input_` and `get_bonus_stage_index_` (645) map a stage ID to its index in BTT (a1 = 0) or BTP (a1 = 1). Remix fighters have their own BTT and BTP entries (739-777). `manual_remix_bonus_start_` (413) stops the automatic start when Remix is selected.
- Remix 1P and All-Star share `remix_1p_randomization` (the All-Star branch is at 3605-3613), `_ko_save`, `_infinite_spawn`, `_infinite_ko`, and the stage pointers `remix_1p_pointer_part1/2`.
- Metal Bros: `metal_bros_flag` and the bug-fix scopes (4952-5111) are shared with Remix 1P.

---

## 7. Port plan

Each step is for one Haiku worker, one session. The lead validates in the running game. Steps depend on the earlier mode steps where noted. None of these steps is to be built here.

1. Port the mode ID table and `singleplayer_mode_flag` as a C enum and global, under `port/remix/` (or `decomp/src/sc/sc1pmode/` if the decomp side is preferred). Files: new header with the enum; no behaviour. Screen: the flag reads back the value written by each menu button (lead sets each mode in the 1P menu and reads the value at a breakpoint).
2. Port the Remix Modes page 2 rows: `remix_menu_button_table` (SinglePlayerMenus.asm:91-96) with its flag writes. Files: the 1P menu module (the same pattern as `mnremixvsmode.c` in F10). Screen: page 2 shows five buttons in the right place with the correct wrap.
3. Port `bonus2_css_2_1p` (SinglePlayerModes.asm:130-266) as a C function for the 1P setup and the random character roll. Depends on 1. Screen: entering Remix 1P, All-Star or HRC from Bonus 2 reaches the 1P CSS with the correct character.
4. Port `remix_1p_randomization` (3580-3941): the Kirby pool, duo select and six random slots. Depends on 3. Files: `sc/sc1pmode/` new C file, one function per part. Screen: two Remix 1P runs with different paths; the lead checks the CPU order in the match.
5. Port `allstar_randomization` (6516-6944) with the character count fixed against `allstar_character_order` (section 8). Depends on 3. Screen: All-Star run shows a new character and stage each match, with no repeats in one run.
6. Port `advance_allstar`, `progress_advancement`, `screen_set_end`, `end_allstar` (7950, 7850, 7891, 8085). Depends on 5. Screen: winning a stage moves progress by the right step; losing ends the run; the final stage is 0x0D.
7. Port `heart_counter` and `heart_persist` (7918, 7814) with the Rest Area spawn gate (7080). Depends on 6. Screen: hearts used in Rest Area carry into the next fight.
8. Port the percent carry (6954-7077, 8029-8083). Depends on 6. Screen: damage carries across All-Star fights and is shown.
9. Port `multiman_random` and `multiman_ko_count_stop` (2187, 2225) with the Cruel branch fixed (section 8). Depends on 1. Screen: Multiman KO counter stops after KO; Cruel shows the same display.
10. Port `setup_` (2287-2401) display for Multiman and Cruel: stock icon, X and KO counter. Depends on 9. Screen: counter reads 0 at start and increments per KO.
11. Port the Multiman preview skip and the stock load (1605-1690, 1913). Depends on 9. Screen: Multiman starts with no preview delay.
12. Port `_end_match_screen` and `_reset`/`_reset_button` (1172, 930, 891) for Multiman, Cruel and Bonus 3. Depends on 9. Screen: death returns to the Bonus 2 CSS; the reset button works.
13. Port HRC main logic (HRC.asm:52-417): distance, camera, ending. Files: `sc/sc1pmode/` new file for HRC, with the state struct. Depends on 1. Screen: sandbag distance counter increases in feet and ends with COMPLETE.
14. Port HRC projectiles (HRC.asm:419-500). Depends on 13. Screen: projectile is kept out of view past the threshold.
15. Port HRC hook groups (HRC.asm:565-745): visibility, countdown, Are You Ready, blastzones, bat, volume, background. Depends on 13. Screen: HRC has no blastzone death and the bat stays.
16. Port `game_over_end` (6081) and `_final_stage_spawn` (5942) for Remix 1P. Depends on 4. Screen: game over returns to the 1P menu page 1.
17. Port the Bonus BTT/BTP lookup (`Bonus.get_bonus_stage_index_`, 645) and the Remix header toggle (`handle_header_toggle_`, 227). Depends on 2. Screen: the Remix header shows and the stage index matches the BTT table.
18. Port Remix 1P team and Metal Bros settings (2716-3214, 4952-5111, 5114-5171). Depends on 4. Screen: Tiny Team spawns three tiny CPUs; Giant CPU 1 is a giant; Metal Bros has the right title card.
19. Port the Master Hand and Giga Bowser hooks (2404-2714, 6021). Depends on 16. Screen: final stage fight works for Remix 1P.

Steps 4-8 and 13-15 are the largest. Split each one by function if a Haiku worker reports it does not fit in one session.

---

## 8. Risks and unknowns

1. Cruel branch bug in `multiman_ko_count_stop` (SinglePlayerModes.asm:2236-2241). The code sets `t1` to `CRUEL_ID` (2240) but compares `t0` to `v0` (2241). `v0` at that point is whatever the vanilla caller left, so the Cruel path may take the wrong branch. Not verified in a running game. Check in the port with a Cruel run: does the KO counter stop after a KO?
2. Comment and count mismatch in `allstar_randomization` (6547, 6596). The code counts 30 iterations, the comment says 28, and the pool size comment says 30 (`0x1E`). The size of `allstar_character_order` is not checked in this pass. A short table would overflow.
3. Stage picking loop in `allstar_randomization` (6568-6580): `_assign_stage` loops until an unused stage is found, with three choices per character. If all three stages for a character are used, it never ends. The uniqueness assumption must be checked against the stage table before the port.
4. Comment inversion in `SinglePlayerMenus.asm:124-125`. The comment says "if page_flag = 0, use remix modes button table". The code uses the Remix table when `page_flag` is non-zero. The code is correct for the flag values written at lines 417-423. Fix the comment in the port.
5. Stale comment at `SinglePlayerModes.asm:2294`: it says "t1 = 1 if Bonus 2", but the code compares against the Bonus 3 ID. Do not copy this comment.
6. `HRC.asm:431` says "If sandbag warp distance = 0, continue normally", but the code does not test for zero there. The effect is probably the same, but it was not confirmed.
7. HRC ending depends on the exact check order at lines 206-275 (time, cargo hold, Lanky Orangstand, grounded, action table). Porting it wrongly changes when the attempt ends. Test each branch with a sandbag that is still moving, a stopped sandbag, and time-up with the player in an exempt action.
7b. The distance unit (section 4.4 step 4): `UNITS_PER_FOOT_TENTH` is commented as "9.12 units/0.1 ft", which would make the stored value tenths of a foot, not feet. Confirm against the HUD counter before the port.
8. Projectile list ends at the human player object (HRC.asm:492, 496). A port that uses a normal list terminator must keep that behaviour or the projectile logic misses the last projectile.
9. Hook addresses: many hooks in the decomp have no verified vanilla match (section 2). Port steps should not assume a vanilla function name without checking.
10. Indexed only: master hand, giga, rest area setup and routine, title card text, moveset tables and most of the All-Star final stage code. Their behaviour in this note is from comments and scope names only.
11. Remix-only file IDs are not separated from vanilla file IDs (section 3.3). The port needs a list from the Remix file table.
12. Not checked: the decomp side of the 1P menu hooks, and whether the Remix port already has any part of the 1P menu (menus.md 3.3 says a generic menu framework was rewritten, so existing code must be read before adding page 2 rows).
13. `SinglePlayerMenus.asm` header and BTP/FGM hooks (0x149340, 0x14073C, 0x13EF30, 0x144E24, 0x14B808) were listed but not read.
14. Text rendering: the header label "REMIX" and the HRC counter need the vanilla string path (`String.insert`) or the ImGui overlay. The port has no text path for these yet.

---

## 9. 1P leftovers (ROADMAP M11 Fase 5, 2026-11-05)

Rules in `sc1premixbonus3rules.*`, `sc1premixendingrules.*` and `sc1premixallstarrestrules.*`, tests in `tests/remix/test_1p_leftover_rules.c`
(44 checks, one mutation checked). Nothing here was run in the game; the syntax check of each file is the only verification so far.

### 9.1 Bonus 3 (`sc1premixbonus3.*`)

The mode is the Race to the Finish row (`nSC1PGameStageBonus3`) of the 1P game played alone. `stage_flag` (SinglePlayerModes.asm:2122) writes 0x0B
(the row) into the scene data, so `sc1PRemixMultimanFirstStage` returns `nSC1PGameStageBonus3` for the mode. What the asm changes on that row:

| Asm | Effect | Port |
|---|---|---|
| `_1p_stage` :375 | stage is `RACE_TO_THE_FINISH`, or `REMIX_RTTF` when `Bonus.mode` is Remix | `sc1PRemixBonus3SetupBattle` sets `gkind` |
| `opponent_type` :277, `_opponent_number_1` :562, `_opponent_behavior` :1442 | three polygons, behaviour 8 | the vanilla row already has them |
| `polygon_selection_match_rttf` :4079 | Remix course picks Remix polygons | `sc1PRemixBonus3UsesRemixPolygons` in the `Bonus3` case of `sc1PGameSetupStageAll` |
| `_team_attack` :431, `_item_spawn` :487 | team attack on, no items | `SetupBattle` |
| `load_1p_difficulty_*` :1959, `_opponent_cpu` :2002, `_opponent_knockback` :522 | Very Hard, CPU level 9, handicap 9 | `sc1PRemixBonus3EnemyLevel`, `SetupEnemy` (the Very Hard column of the table is 9 and 9 too) |
| `load_player_stocks` :1913 | stock 0 | `SetupPlayer` |
| `_time_limit` :671 | infinite time | `SetupBattle` |
| `_load_timer` :989, `bonus3_timer` :1031 | count-up timer of the Bonus screens (Dragon King HUD: y 210 and 200) | `sc1PBonusStageMakeCountUpTimer` (the body of the old `sc1PBonusStageMakeTimer` branch), called from `sc1PGameInitTimeUpMessage` |
| `_initial_screen_set` :864 | no title card | `sc1PManagerUpdateScene` skips `sc1PIntroStartScene` |
| `_end_match_screen` :1172 | every end goes back to the Bonus 2 CSS, the reset button restarts | `sc1PRemixBonus3LeaveRun` (same as Multi-Man) |
| `_time_save` :1296, `_new_record` :1398 | the finish time is the record of the fighter when it is empty or strictly faster; a beaten record plays New Record (0x1D0) instead of Complete (0x1CB); not saved when the Single Player Enemy port is on | `sc1PRemixBonus3OnComplete`, called from `grBonus3FinishProcUpdate`; fields `bonus3_time` and `bonus3_remix_time` added to `LBBackup1PRecord` |

`TIME_SAVE_POINTER` (0x800A4B30) is `time_passed` of the battle, so the saved value is in ticks. `SinglePlayer.high_score_enabled` is always TRUE in the Remix.
`SpawnStyle` (`_spawning_style` :764) and the music hooks end up in the vanilla Bonus 3 wait (`sc1PGameWaitStageBonus3Update`) and the vanilla music.

Open: `Bonus.mode` (the Normal/Remix header toggle of the Bonus 2 CSS, Bonus.asm:227) is not in the port, so the course is Normal; `sc1PRemixBonus3SetCourse`
takes the toggle once the CSS has it, and `SSB64_REMIX_BONUS3_COURSE=normal|remix` forces it in tests. The records are saved but the Bonus 2 CSS does not show them yet
(`load_bonus3_time`, SinglePlayerMenus.asm:508, belongs to the CSS).

### 9.2 Giga Bowser ending

Everything the asm changes on the final stage was already ported (R5). The end of the stage is the vanilla boss defeat: `sc1PGameBossWallpaperProcUpdate` steps
the background at 90 and 180 damage of the boss (`stock_damage_all` of `bossplayer`) and `SC1PGameBossWallpaper3ProcUpdate1` ends the fight. The Remix HP is 200 to 300, so
the 180 step is always reached by a real fight; the earlier test with HP 1 never got there. New test hook `SSB64_REMIX_1P_BOSS_PRESET_DAMAGE=<n>` (`sc1PRemix1PPresetBossDamage`) starts the boss with n percent.

### 9.3 Title cards

Not changed. Remix Team (one Marina), Mad Piano (large, cut by the 0x7070 camera) and the scrambled Metal Mario Bros texture need the running game to compare against the
emulator; the asm side (`remix_yoshi_card` :5300, `title_card_model_scale` :4800, `metal_bros_magic` :4952) is already ported.

### 9.4 All-Star

| Asm | State |
|---|---|
| `rest_area_setup` icons, :7413-7625 | `sc1PRemixAllstarRestMakeIcons` (file 0xD8B, `icon_offset_table`, ten to a row at x 30 + 9n, y 30 + 11 row). Inert until the generator exports `SinglePlayerModes.icon_offset_table` (0x804EFCF4) |
| `rest_area_setup` portraits, :7627-7690 | not made: world-space textures (`Render.draw_stage_texture_`, file 0xA05, x -200/-600/-1000, y 1000, z -20, alpha 0xA0) need a stage texture object; symbol `CharacterSelect.portrait_offset_by_character_table` is missing as well |
| `skip_go` :7218, `instant_spawn` :7172, `_time_limit` :671 | Rest Area uses the race wait (no GO, no damage meters), spawns the player without the entry and has no time limit |
| `match_begin_flag` | `sc1PRemixAllstarKeepsRunState`: a re-entry of `sc1PManagerUpdateScene` in a run that already had a Rest Area keeps stocks, score and total damage (`stock_prevent`, `prevent_erase`, `damage_prevent`). The port enters the function once per run, so there is no visible change |
| `costume_fix` :8179 | deviation: the hook sits in the Yoshi team costume loop (rom 0x10C350), a row All-Star never plays; the effect it was written for (at most 4 costumes for vanilla fighters of the final team) is `sc1PRemixAllstarPrepareFinal` |
| `pause_exit` :7988 | All-Star: leaving by A+B+R+Z already goes to the 1P menu through `is_reset`; Remix 1P: `sc1PRemix1POnPauseExit` clears the size of the player (`Size.clear_settings_for_1p_`) |
| own ending, BGM.asm:1353 | `sc1PRemixEndingGetCreditsSong` (hook in `scStaffrollFuncStart`): Remix 1P and All-Star play `Character.remix_1p_end_bgm.table[fkind]`. Inert until the generator exports that table (0x8049A250) |

Missing symbols for the generator (`tools/gen_remix_actions.py`): `SinglePlayerModes.icon_offset_table`, `Character.remix_1p_end_bgm.table`, `CharacterSelect.portrait_offset_by_character_table`.
Hooks in files outside the 1P folder: `gr/grbonus/grbonus3.c` (one call, Bonus 3 finish), `sc/sccommon/scstaffroll.c` (one call, credits song), `lb/lbtypes.h` (two record fields).
