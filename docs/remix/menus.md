# Smash Remix: menu and UI side (CSS, Remix menus, new screens, title)

Source: `.toolchain/pkgs/smashremix-src/src/` (MIPS asm, bass/armips-style macros). Line numbers below are from that tree.
Decomp side: `decomp/src/mn/**`. Address matches come from `// 0xADDR` comments in the decomp.

**Caveat on address matching.** Remix hooks are RAM addresses inside overlays, and the same RAM range is reused by different overlays at different times. The decomp `// 0x` comments are per overlay. Many matches below are therefore wrong overlay, for example `0x80133E78` (VS game-mode button code) resolves to a staff-roll function. Rows are marked **ok** when the decomp function matches the screen the hook belongs to, and **collision** otherwise. Treat collisions as unresolved until checked against the overlay table.

---

## 1. Character Select (CSS) expansion

### 1.1 Grid and slots
- Constants: `NUM_ROWS(3)`, `NUM_COLUMNS(10)`, `NUM_PORTRAITS = 30`, `BOOKEND_BONUS_PORTRAIT = 30`, `BOOKEND_RANDOM_PORTRAIT = 31` (`CharacterSelect.asm:3570-3574`). `START_X(29)`, `START_Y(34)`, `PORTRAIT_WIDTH(24)` px, `PORTRAIT_SCALE` 0.8125 (`:3575-3590`).
- `NUM_SLOTS(30)` (`:3599`). `scope layout` defines 30 slots, 3 rows of 10 (`:3600-3635`):
  - row 1: MARINA, DRM, LUIGI, MARIO, DONKEY, LINK, SAMUS, CAPTAIN, GND, SONIC
  - row 2: DEDEDE, YLINK, NESS, YOSHI, KIRBY, FOX, PIKACHU, JIGGLYPUFF, FALCO, SHEIK
  - row 3: GOEMON, CRASH, WARIO, PEACH, BOWSER, WOLF, CONKER, MTWO, MARTH, BANJO
- Bonus bookend: `bonus_chars` (10 entries, `:23-35`, `NUM_BONUS_CHARS(10)`): DSAMUS, LUCAS, PEPPY, SLIPPY, ROY, DRL, LANKY, DRAGONKING, EBI, PIANO. Per-port scroll index per mode (`bonus_char_index_vs/1p/training/bonus`, `:44-61`).
- Slide-in: `portrait_x_position` (10 columns + 2 bookends, `:~3890`), `portrait_velocity` (12 floats, `:~3905-3918`).
- Layout tables built with `while` loops over `NUM_SLOTS`:
  - `id_table` (portrait index -> `Character.id`), `:~3921-3927`
  - `portrait_offset_table` (portrait index -> file offset of portrait texture), `:~3936-3942`
  - `portrait_id_table` (`Character.id` -> portrait index, with `NMARIO`/`NDONKEY` special cases), `:~3995-4020`
  - `name_texture_table`, `series_logo_offset_table`, `selection_action_table`, `white_circle_size_table`, `fgm_table` are indexed by `Character.id` and padded to `Character.NUM_CHARACTERS * 4` (`:2998, 3033, 3069, 3433, 3565`).
- Character count: `Character.NUM_CHARACTERS = 27 + 2 + 68 = 97` (`Character.asm:15-27`). Only 30 + 10 bonus + random are on the grid. The rest load through `load_additional_characters_` and token placement.

### 1.2 Cursor and token behaviour
- Cursor/token position, not hand position, selects the character: `get_character_id_` (`:449-480`), one hook per mode (VS, Training, 1P, BTT/BTP). Reads char id from the per-mode CSS player struct (`CSS_PLAYER_STRUCT` 0x8013BA88, `_TRAINING` 0x80138558, `_1P` 0x80138EC0, `_BONUS` 0x80137620; `:16-19`).
- Column and row from index: `index = row * NUM_COLUMNS + column` (`:547-554`), and `portrait_id % NUM_COLUMNS`, `/ NUM_COLUMNS` (`:710-791`), with 12CB using `TwelveCharBattle.NUM_COLUMNS` on its own page.
- Token auto-position and boundaries: `token_autoposition_` (`:646-760`), `fix_right_boundary_check_`, `fix_bottom_boundary_check_` (`:947-980`). Extra y-range for the bottom panel menu at `:990`.
- Scaled-token and variant overlap fixes at `:765-1080`.

### 1.3 Random select
- `get_random_char_id_` (`:1128-1140`): VS hook at ROM 0x136AD8 replaces the vanilla random call (`jal 0x80018A30 / a0=0xC`, commented out) with `get_random_char_id_`. Training and 1P/BTT have the same pattern (`:1138`).
- Random pool: `NUM_SLOTS` plus bonus chars (`:1155-1165`), so Remix characters are eligible.
- Anti-repeat: `recent_randoms_table` holds `RRL_NUM_SLOTS(6)` recent picks per port, 8 bytes per port with the write index at +7 (`:1320-1342`, `:1395`). `_recent_randoms_lookup` rerolls on a repeat. Skipped when the pick is for the placeholder '?' (`placeholder_port`, `:1328-1330`).
- Random Select toggle (`Toggles.asm` `entry_variant_random`, string table `string_table_random_variants`, options: default / bonus / variants / vanilla) decides the pool.
- Hooks: VS `0x80138858` maps to `mnPlayersVSRandFighterKind` (`mnplayersvs.c:3900`), **ok**. Training `0x80137AB0` and others are **collision**.

### 1.4 Costumes
- `NUM_COSTUMES` is declared per character in `costumes/*.asm` (for example Mario 5, Peach 7, Banjo 7, NMario 6, Pikachu 4+2, Jigglypuff 5+1). Mewtwo has `NUM_COSTUMES(6)` at `costumes/Mewtwo.asm:12`.
- `get_default_costume_` hooks at `:1945-1985` (ROM 0x678F4, 0x6790C, 0x67920) patch the c-button and team costume lookups to read `Character.default_costume.table`.
- `check_variant_input_` (`:2365-2390`): D-pad/Z input during the CSS selects variants.

### 1.5 Mapping fighter id to slot, and load path
- Slot -> `Character.id`: `id_table`. `Character.id` -> portrait index: `portrait_id_table`. Name texture: `name_texture_table[Character.id]`.
- Extra character loading: `load_additional_characters_` (`:1985-2005`), `dynamically_load_character_` (`:2130`), heap slots `dynamic_css.heap_slot_0..` (`:4081-4290`).
- Model load for CSS uses an alternate req list: `load_character_model_` (`:77-113`) and `alt_req_table` / `alt_malloc_table` (`:195-310`), with `.req` files in `src/Req/` (82 files, for example `NMARIO_MODEL.req`, `SAMUS_MODEL.req`).
- VS character-select hook sites: `0x8013B1C0` -> `mnPlayersVSFuncStart` (`mnplayersvs.c:5439`), **ok**. `0x8013B1D8` same.

### 1.6 CSS panel menu (per-port debug panel)
- Gated by the "CSS Panel Menu" toggle (`Toggles.asm:2381`).
- Items are defined in `src/css/*.asm` (each has `define LABEL("...")`), included through `CharacterSelectDebugMenu.asm:2657-2710`:
  Initial Damage, Dpad ctrl, Dpad map, Handicap, Delay, Input HUD, Kirby Hat, Knockback, Model, Player Tag, Poison Dmg, Practice, Shield, Size, Skeleton, Start With, Stock, Taunt Btn., Taunt Itm., Visibility.

---

## 2. Remix menu system (Menu.asm, Settings.asm, Toggles.asm, SRAM.asm)

### 2.1 Generic menu framework (`Menu.asm`)
- Types (`Menu.asm:14-28`): TITLE, MAX1, BOOL, MAX3 ... MAX1023, INPUT (text entry), INT (size chosen on save).
- `Menu.info` macro (`:37-60`): head pointer, ulx/uly, room, group, width, cursor/label/value colors, scale, row height, max rows per page (default 12), blur, dpad flag. Default macro uses `Render.FONTSIZE_DEFAULT`, row height 0x0E.
- `Menu.entry` macro (`:66-110`): linked list node. Fields: 0x00 type, 0x04 current value, 0x08 min, 0x0C max, 0x10 a_function, 0x14 string table, 0x18 copy address, 0x1C next, 0x20 label object, 0x24 extra, 0x28 title text.
- Drawing: `Menu.draw_` (`:530`) calls `draw_entry_` (`:327`), `Render.draw_string_` (`:377`, `:428`), `Render.draw_rectangle_` (`:676`, `:693`), `Render.apply_max_width_`. Cursor: `update_cursor_` (`:722`).
- Input: `Menu.update_` (`:772`) uses `Joypad.check_stick_` and `Joypad.check_buttons_all_`, sounds via `FGM.play_`. Keyboard char input for INPUT entries (`:117-270`, `:951-1080`).
- So the Remix menus **do not** reuse vanilla option drawing. They use their own generic menu, built on Render and Joypad. Vanilla is only reused for the VS/1P mode-select buttons (see section 3).
- SRAM export/import: `Menu.export_` (`:1569`) and `Menu.import_` (`:1655`). Values are bit-packed into 32-bit words in declaration order. Titles are skipped. INPUT entries are handled separately. Words start at block+0x10.

### 2.2 Toggles: structure and storage (`Toggles.asm`)
- Root "super menu" (`Menu.info(head_super_menu, 30, 50, 3, 0, 32, ...)`, `:986`), with groups (`:2367-2374`):
  Remix Settings, Gameplay Settings, Music Settings, Stage Settings, Pokemon Settings, Player Tags, Other Screens, and "Load Profile" at the top.
- Each toggle is one `Menu.entry`. Its value lives at `Toggles.<name> + 0x4`, read with the macro `Toggles.read(name, reg)` (`:3235-3237`).
- Storage: one SRAM block per group, sized by `block_size` bit packing (`update_block_size`, `:1004-1014`): block_remix (`:2852`), block_gameplay (`:2854`), block_music (`:2856`), block_stages (`:2858`), block_pokemon (`:2860`), block_tags = `MAX_TAGS(20) * 20` bytes (`:2862`).
- Save/load: `Toggles.save_` (`:846`) calls `Menu.export_` then `SRAM.save_` for each block. `Toggles.load_` (`:890`) is the reverse.
- SRAM base and revision (`SRAM.asm`): `SRAM.ADDRESS = 0x05F0`, `REVISION = 0x00F7` (bump on any toggle, stage or MIDI change or order change). Save/load patched to use only the first 0x5EC bytes (`SRAM.asm:35-47`). Header check `check_saved_` (`:103`).
- Profiles: `profile.CE(0)`, `TE(1)`, `NE(2)`, `JP(3)`, `CUSTOM(4)` (`Toggles.asm:1103-1108`). Names: Community, Tournament, Netplay, Japanese, Custom, plus a string "Semi-Competitive" (`:1112-1118`). Defaults for each toggle are per profile, written by `write_defaults_for(profile)` (`:1095`). `load_profile_` (`:3089`), `get_current_profile_` (`:3156`).
- `normal_options` flag (`:166`) gates disabling the OPTION screen when custom toggles are active (`disable_options_functionality_`, `:88`).

### 2.3 Settings.asm
- Only one hook: at ROM 0x40898 (RAM `0x800A1B48`, `Settings.asm:26`) it jumps to `set_vs_settings_`, which writes Tournament defaults into `Global.vs.*`: game_mode=3, time=8, stocks=3, team_attack=1, item_frequency=0 (`:9-13`, `:29-50`). The comment says "should not be called", but the patch is live. Verify before relying on it.

### 2.4 Full toggle list
Default values are listed as CE / TE / NE / JP (`OS.TRUE`=1). INT ranges are max values. Source line in `Toggles.asm`.

**Remix Settings** (`:2376-2412`, 32 entries)

| Toggle | Type (max) | CE | TE | NE | JP |
|---|---|---|---|---|---|
| Skip Results Screen | INT (2) | 0 | 0 | 1 | 0 |
| Hold To Pause | bool | 0 | 1 | 1 | 0 |
| CSS Panel Menu | bool | 1 | 0 | 1 | 1 |
| Color Overlays | bool | 0 | 0 | 0 | 0 |
| Cinematic Entry | INT (2) | 0 | 0 | 0 | 0 |
| Flash On Z-Cancel | bool | 0 | 0 | 0 | 0 |
| FPS Display *BETA | INT (2) | 0 | 0 | 0 | 0 |
| Model Display | INT (2) | 0 | 0 | 1 | 0 |
| Special Model Display | INT (3) | 0 | 0 | 0 | 0 |
| Advanced Hurtbox Display | bool | 0 | 0 | 0 | 0 |
| Hold To Exit Training | bool | 0 | 1 | 0 | 0 |
| Improved Combo Meter | bool | 1 | 0 | 1 | 1 |
| Tech Chase Combo Meter | bool | 1 | 0 | 1 | 1 |
| Combo Meter | bool | 1 | 0 | 1 | 1 |
| 1v1 Combo Meter Swap | bool | 0 | 0 | 0 | 0 |
| Neutral Spawns | bool | 1 | 1 | 1 | 1 |
| Salty Runback | INT (2) | 1 | 0 | 1 | 1 |
| Widescreen | bool | 0 | 0 | 0 | 0 |
| Stereo Fix for Hit SFX | bool | 1 | 1 | 1 | 1 |
| Random Select | INT (3) | 0 | 0 | 0 | 0 |
| Disable HUD | INT (2) | 0 | 0 | 0 | 0 |
| Disable Anti-Aliasing | bool | 0 | 0 | 0 | 0 |
| Always Show Full Results | bool | 1 | 1 | 1 | 1 |
| Skip Start Cheer | INT (2) | 0 | 0 | 0 | 0 |
| Default CPU LVL (V.S.) | INT (10) | 0 | 10 | 0 | 0 |
| Jigglypuff Sing GFX Anims | bool | 0 | 0 | 0 | 0 |
| 'L' Selects Random Character | bool | 0 | 0 | 0 | 0 |
| Dpad CSS Cursor Control | bool | 0 | 0 | 0 | 0 |
| PK Thunder Reflect Crash Fix | bool | 1 | 1 | 1 | 1 |
| Flash Guard | bool | 0 | 0 | 0 | 0 |
| Screenshake | INT (2) | 0 | 0 | 0 | 0 |
| BlastZone GFX | INT (2) | 0 | 0 | 0 | 0 |

**Gameplay Settings** (`:2413-2453`, 31 entries)

| Toggle | Type (max) | CE | TE | NE | JP |
|---|---|---|---|---|---|
| Hitstun | INT (1) | 0 | 0 | 0 | 0 |
| Hitlag | INT (4) | 0 | 0 | 0 | 1 |
| DI | INT (2) | 0 | 0 | 0 | 1 |
| Japanese Sounds | INT (2) | 0 | 0 | 0 | 1 |
| Japanese Stun/Sleep | bool | 0 | 0 | 0 | 1 |
| Momentum Slide | bool | 0 | 0 | 0 | 1 |
| Shield Stun | INT (4) | 0 | 0 | 0 | 1 |
| Z-Cancel | INT (4) | 0 | 0 | 0 | 0 |
| Punish Failed Z-Cancel | INT (11) | 0 | 0 | 0 | 0 |
| Improved AI | bool | 1 | 0 | 1 | 1 |
| Tripping | INT (3) | 0 | 0 | 0 | 0 |
| Rage | INT (4) | 0 | 0 | 0 | 0 |
| Footstool Jumping | bool | 0 | 0 | 0 | 0 |
| Air Dodging | INT (3) | 0 | 0 | 0 | 0 |
| Jab Locking | bool | 0 | 0 | 0 | 0 |
| Edge C-Jumping | bool | 0 | 0 | 0 | 0 |
| Perfect Shielding | bool | 0 | 0 | 0 | 0 |
| Parrying (BETA) | bool | 0 | 0 | 0 | 0 |
| Spot Dodging | bool | 0 | 0 | 0 | 0 |
| Fast Fall Aerials | bool | 0 | 0 | 0 | 0 |
| Ledge Trumping | bool | 0 | 0 | 0 | 0 |
| Wall Teching | bool | 0 | 0 | 0 | 0 |
| Charge Smashes | INT (2) | 0 | 0 | 0 | 0 |
| Item Containers | INT (3) | 0 | 0 | 0 | 0 |
| Game Speed | INT (12) | 0 | 0 | 0 | 0 |
| Special Zoom (BETA) | INT (2) | 0 | 0 | 0 | 0 |
| BlastZone Warp (BETA) | INT (3) | 0 | 0 | 0 | 0 |
| Single Button Mode | INT (6) | 0 | 0 | 0 | 0 |
| All Items R Drop (Aerial) | bool | 0 | 0 | 0 | 0 |
| Move Staling | INT (6) | 0 | 0 | 0 | 0 |
| Stopwatch Item | INT (4) | 0 | 0 | 0 | 0 |

**Music Settings** (`:2454-2546`)
- Play Music: 1/1/1/1. Random Music: 0/0/1/0. Salty Runback Preserves Song: 0/0/0/0. Menu Music INT (`menu_music.MAX_VALUE`): 0/0/1/0. Music Title at Match Start: 1/0/1/1. BGM Volume INT (10): 10 in all. SFX Volume INT (10): 10 in all.
- Random Music Toggles (bools, default 1/1/1/1 for every track): Bonus, Congo Jungle, Credits, Data, Dream Land, Duel Zone, Final Destination, How To Play, Hyrule Castle, Meta Crystal, Mushroom Kingdom, Peach's Castle, Planet Zebes, Saffron City, Sector Z, Yoshi's Island (`:2468-2490`).
- Custom MIDI tracks are generated from `MIDI.MIDI_*_TOGGLE`, sorted by `MIDI.MIDI_*_ORDER` (`:2495-2545`). Default 1 in all profiles.
- "Load Profile" is an INT (0-3) with defaults 0/0/0/0 and calls `load_sub_profile_`.

**Stage Settings** (`:2561-2615`)
- Load Profile INT (2): 0/1/2/0.
- Stage Select Layout INT (1): 0 normal / 1 tournament (`string_table_sss_layout`, `:1480-1481`). Defaults 0/1/0/0.
- Hazard Mode INT (3): 0 in all. Whispy Mode INT (3): 0/0/0/JAPANESE. Saffron Pokemon Rate INT (3): 0.
- Pokemon Announcer INT (2): POKEMON / OFF / POKEMON / OFF. Dragon King HUD INT (2): DEFAULT / OFF / DEFAULT / DEFAULT. Camera Mode INT (3): NORMAL in all. Yoshi's Island Cloud Anims bool: 0.
- Random Stage Toggles (bools, CE/TE/NE/JP):
  - Congo Jungle 1/0/1/1
  - Dream Land 1/1/1/1
  - Dream Land Beta 1 0/0/0/0
  - Dream Land Beta 2 0/0/0/0
  - Duel Zone 1/0/0/1
  - Final Destination 1/0/0/1
  - How to Play 0/0/0/0
  - Hyrule Castle 1/0/1/1
  - Meta Crystal 1/0/1/1
  - Mushroom Kingdom 1/0/0/1
  - Peach's Castle 1/0/1/1
  - Planet Zebes 1/0/0/1
  - Saffron City 1/0/1/1
  - Sector Z 1/0/0/1
  - Yoshi's Island 1/0/0/1
  - Mini Yoshi's Island 1/0/1/1
  - plus custom stages (`Stages.STAGE_*_TOGGLE`, `:2597-2612`)

**Pokemon Settings** (`:2624-2643`)
- Pokemon SFX INT (2): 0/0/0/1. Toggle All.
- Onix, Snorlax, Goldeen, Meowth, Charizard, Beedrill, Blastoise, Chansey, Starmie, Hitmonlee, Koffing, Clefairy, Mew: bools 1/1/1/1. Psyduck, Shellder, Staryu are commented out (`:2639-2641`).

**Player Tags** (`:2650-2660`): 20 INPUT entries, `toggle_edit_mode_`.

**Other Screens** (`:2665-2680`): Debug (view_screen_ 0x03), View Intro (0x1C), View How to Play (0x3C), View Credits (view_credits_ 0x38), View Gallery (`view_gallery`, `:2330`).

### 2.5 Mode-select and VS-menu integration (Toggles hooks)
- `add_mode_select_remix_button_` (`:109`) and `set_settings_button_display_` (`:202`) add a 5th "Settings" button (index 4) on the Mode Select screen. Hooks at ROM `0x11D942`, `0x11D962`, `0x11D866`, `0x11D984`, `0x11D89C` (RAM `0x801329B2`, `0x801329D2`, `0x801328D6`, `0x801329F4`, `0x8013290C`, `Toggles.asm:111-229`). Uses `sMNModeSelectOption` at RAM 0x80132C88 (`mnmodeselect.c:116`) to find the selected button.
- `rescale_mode_select_buttons_` (`:221`) shrinks the buttons and text to fit.
- `mode_select_screen_change_` (`:271`, hook ROM `0x11D668` / RAM `0x801326D8`, `:272`) opens the super menu (`show_super_menu_`), clears the single-player flag and shortcut state, and clears input masks so How to Play does not read as Title. The decomp symbol at that address is `mnTitleProcUpdate` (`mntitle.c:771`), which is a **collision**. The RAM variable use points to the mode-select overlay (`mnmodeselect.c`).
- `settings_shortcut` (`:3260`): holding L on CSS or SSS opens the settings menu, with the return screen stored in `shortcut_stored_screens` (`:3260` area, `:~3240`).
- Per-frame run: `run_` (`:563`) calls `Menu.update_` every frame and handles B/back to the super menu.

### 2.6 Drawing routines reused from vanilla
- Remix menus: own generic routines (section 2.1). Vanilla is reused only through `Render.*` helpers and the mode-select button code (section 3).

---

## 3. New screens, modes and vanilla hooks

### 3.1 Scene (screen) IDs (`Global.asm:58-122`)
- Vanilla used: MODE_SELECT 0x07, VS_GAME_MODE_MENU 0x09, VS_OPTIONS 0x0A, VS_CSS 0x10, _1P_CSS 0x11, STAGE_SELECT 0x15, DATA_VS_RECORD 0x19, DATA_CHARACTERS 0x1A, OPTION 0x39, DATA_MENU 0x3A, DATA_SOUND_TEST 0x3B, HOW_TO_PLAY 0x3C, CONGRATULATIONS 0x37, STAFF_ROLL 0x38.
- **Remix-added**: `REMIX_MODES(0x77)` (`Global.asm:122`): "Remix Modes (other than Remix 1P): All-star, Multiman, HRC". Used by the VS page-2 button (`VsRemixMenu.asm` `remix_menu_button_table`, `db 0x09` screen; page flag `page_flag`).
- The **Gallery** does not have its own ID. It reuses CONGRATULATIONS (0x37): `Gallery.asm:1265-1280` forces that ID while the gallery is active, and `:1448`, `:1475` do the same. Gallery state: `status` struct (`Gallery.asm:19-35`), entry from the Other Screens menu.
- Character Data (`CharacterDataScreen.asm`) keeps 0x1A but now handles all 97 characters: `character_id_to_data_screen_id_table`, `extend_*` tables (`:380-660`), `allow_infinite_characters_` (`:662`), `extend_index_` (`:982`).

### 3.2 VS Game Mode menu: 5 + 4 buttons (`VsRemixMenu.asm`)
- `mode` constants (`:12-19`): DEFAULT 0, TWELVE_CB 1, TAG_TEAM 2, KOTH 3, SMASHKETBALL 4, TUG_OF_WAR 5.
- Globals saved before CSS (`:28-60`): game mode, time, stocks, teams, stage select, stage, stockmode table, stock count table.
- `menu_button_table` (page 1, `:~88-96`): VS Start, Rule, Time Stock, VS Options, Remix Modes (db 0x09). `PAGE_1_MAX(4)`.
- `remix_menu_button_table` (page 2): 12CB, Tag Team, King of the Hill, Smashketball, Tug of War. `PAGE_2_MAX(4)`.
- Button table row format is documented in the comment at `:~85-90` (0x10 bytes: create routine, screen, vs_mode_flag, page_flag, 12cb flag, text X/Y floats, text texture offset).
- `add_button_` (`:102-150`): patches the cursor wrap limits from 3 to 4 (`:104-120`), and adds a 5th pointer to `GObj** buttons[]` on the stack.
- Button creation rewritten generically: `create_menu_button_` (`:159`), `create_button_generic_` (`:196`), replacing vanilla `0x80132154..0x80132238` and `0x80132EBC..0x80132FD8` (`:~147`, `:193`).
- Vanilla hook matches: `0x80132154` -> `mnVSModeMakeVSStartButton` (`mnvsmode.c:318`), **ok**. `0x80132EBC` -> `mnVSModeMakeVSOptionsButton` (`mnvsmode.c:736`), **ok**.
- Other VS-menu hooks (`:104-1047`, about 32 in total) have mixed matches. For example `0x80133E78` (`add_button_`) and `0x80133FE0` map to unrelated staff-roll functions (**collision**). Hooks such as `0x80136338` (`ftComputerCheckFindItem`) are also collisions. Only the rows marked above are verified.
- Page 2 exit: `page_2_exit` (`:304`) returns to page 1 from Remix Modes (sets `page_flag`, returns to the VS menu, exits the function early at `0x801345B4`).
- `save_global_settings_` (`:388`) stores current VS settings before CSS.
- `control_match_start_` (`:684`), `check_match_end_` (`:721`), `set_cursor_index_` (`:809`), `change_time_options` (`:861`), `get/prev_value` (`:872-968`), `disable_mode_toggle_` (`:968`), `update_announcer_on_toggle_` (`:991`), `add_row_to_results_` (`:1007`).

### 3.3 1P mode menu (`SinglePlayerMenus.asm`)
- Rewritten generic button creation: `create_menu_button_` (`:103`), `render_menu_buttons_` (`:211`), `fix_cursor_wrap_` (`:251`), `set_cursor_index_` (`:290`), `page_2_exit` (`:341`), `fix_a_and_b_button_crashes_` (`:443`, hooks `0x11EB70`, `0x11ED7C`, which the fix reconnects to an exit). 34 hooks, several to vanilla mn1pmode (`mn1PModeMake1PGame` 0x80131E34, `mn1PModeInitVars` 0x80132910, match **ok**).
- Stock/KO and time displays: `_ko_amount_or_feet` (`:460`), `load_bonus3_time` (`:508`), `total_kos_or_feet_` (`:571`), `draw_hrc_high_score_decimal_` (`:681`), HRC/Remix headers `header_graphic_remix_1p` (`:807`).

### 3.4 VS stats (`VsStats.asm`)
- Enables match stats on the results screen. Toggle gate `check_menu_toggle_` (`:675`). Stat tracking: `calculate_tracker_percentage_` (`:168`), `increment_special_tracker` (`:210`). Drawing: `draw_port_headers_` (`:542`), `draw_underline_` (`:288`), `draw_line_` (`:354`). Setup `setup_` (`:917`), per-frame `run_collect_` (`:1071`). Hooks on special-move interrupt routines (`ftCommonSpecial*CheckInterruptCommon`, `:216-246`) count moves, **ok** by name (decomp `ft/ftcommon/...`).

### 3.5 Credits (`Credits.asm`)
- Extended title, name, company and detail tables (`use_extended_*`, `:864-1160`). Added characters credited via `use_added_characters_` (`:1164`). Speed assist: `remix_credits_speed_assist_` (`:1310`). Credits screen entry: `view_credits_` (`Toggles.asm:2142`). Vanilla credit tables: `0x80136B10` (title info), `0x8013685C` (title strings), `:899, :928`.

### 3.6 Transitions (`Transitions.asm`)
- Restores unused VS-to-results transitions. Shrinks the original transition table to halfwords and moves pointers and strings to expansion RAM. Hooks in `lbTransitionMakeTransition` (decomp `lb/lbtransition.c:244`, RAM 0x800D4320..0x800D43AC, `:83-107`) and `lbTransitionSetupTransition` (`lbtransition.c:275`, `:65-77`), **ok**.

### 3.7 Japan (`Japan.asm`)
- Gameplay only (JP profile): hitlag, guard, brake, Fura Fura, motion events, items, weapons (`:30-532`). No menu screens. Settings sources are in Toggles (JP profile values above).

### 3.8 Stage select (`Stages.asm`, 68 hooks)
- Grid: `NUM_ROWS(3)`, `NUM_COLUMNS(6)`, `NUM_ICONS = 18`, `NUM_PAGES(5)` (`:542-545`), so 90 icons across 5 pages. `MAX_STAGE_ID(0xD7)` (`:243`).
- Routines: `update_stage_icons_` (`:1275`), `get_index_` (`:1384`), `check_valid_page_` (`:1410`), `get_stage_id_` (`:1448`), `update_text_` (`:1546`), `update_cursor_color_` (`:1811`), directional handlers `right_`/`left_`/`up_`/`down_` (`:2152-2299`), `random_stage_roulette` (`:2349`, activated by C-Down on RANDOM), `add_stage_to_random_list_` (`:2567`).
- Layout: `sss.NORMAL` / `sss.TOURNAMENT` (Toggles `:1483-1485`).
- Hooks for these are in the stage-select overlay. Most are **unverified** (the address match is outside the decomp's stage-select file).

### 3.9 Random (`Random/Random.asm`)
- Move-data only (Sheik-related actions and moveset includes, `:5-64`). It is not menu code.

### 3.10 Boot and Settings
- `Boot.asm` (10 hooks): intro and screen adjustments, some **collisions**.
- `Settings.asm` patch: see section 2.3.

---

## 4. Title screen

- Remix does not add a separate title screen. The Title/mode-select flow is changed in these places:
  1. Mode Select gets a 5th button (Settings) and shrinks the others (`Toggles.asm:109-270`).
  2. `mode_select_screen_change_` opens the Settings super menu, clears the single-player flag, and clears the input masks (`:271-340`).
  3. Holding L on CSS or SSS opens Settings (`settings_shortcut`, `:3260`).
  4. Credits and SinglePlayerMenus hook `mnTitleFuncUpdate` (`Credits.asm:1171`, `0x80134098`), `mnTitleShowFire` (`Gallery.asm:1486`, `SinglePlayerMenus.asm:444`), `mnTitleLogoFireProcDisplay` (`SinglePlayerMenus.asm:578`), `mnTitleMakeActors` (`SinglePlayerMenus.asm:687`). Decomp `mntitle.c` names match these, but the overlay-collision caveat applies.
- The Remix "Remix Modes" entry sits on VS page 2 (section 3.2), and on the 1P menu through `SinglePlayerMenus`.

---

## 5. Feasibility: data tables vs new C logic

**Can be data tables (port as constants or generated tables):**
- Toggle defaults per profile, option ranges, string tables, titles, group membership (section 2.4). Remix has an `entry` macro, so the port can mirror it as a static table of `{title, type, defaults[4], min, max, next}`.
- CSS slot layout (`layout.slot_1..30`), bonus character list, `portrait_id` and name texture tables, velocity and x-position tables.
- VS menu button table (5 + 4 entries: position, screen, mode flag, page flag, text texture).
- Stage grid constants (3 x 6, 5 pages) and stage toggle defaults.
- Costume counts per character (`NUM_COSTUMES`).
- SRAM block sizes (computed from entry count and max, as in `update_block_size`).

**Needs new C logic (or fits into existing decomp C with changes):**
- Generic menu framework (`Menu.info`/`Menu.entry`, draw, input, export/import bit packing). This is a sizeable module. The port's existing mn code uses different menu structs, so this would need a thin layer.
- Toggle persistence: packing values into SRAM blocks with revision checks. Port likely needs its own save file layer; the bit-packing order must match the Remix revision if save compatibility matters.
- CSS: the 30-slot grid with 10 bonus, random bookend, and the extra character loader (`load_additional_characters_`, heap slots). Token auto-positioning and boundary checks need changes in `mnplayersvs.c` (`mnPlayersVSPuckAdjustPortraitEdge` at 4269, `mnPlayersVSGetPortrait` at 377, `mnPlayersVSMakePortrait` at 468, `mnPlayersVSRandFighterKind` at 3900).
- Random anti-repeat window (6 per port) and placeholder handling.
- VS page 2 (Remix Modes) and the 5th mode-select button: needs changes to `mnvsmode.c` (button creation at 318 and 736, page logic) and `mnmodeselect.c` (option count and selection).
- Gallery as a new state machine hosted on the Congratulations screen, plus the Data screen extension to all characters.
- VS stats tracker (per-special counters, results drawing).
- Credits extension (added titles and names).
- Transitions table restoration.

**Decomp functions to hook or replace in the port (best candidates, by the verified matches):**
- `mnPlayersVSRandFighterKind` (`decomp/src/mn/mnplayers/mnplayersvs.c:3900`)
- `mnPlayersVSGetPortrait` / `mnPlayersVSMakePortrait` (`mnplayersvs.c:377`, `:468`)
- `mnVSModeMakeVSStartButton` / `mnVSModeMakeVSOptionsButton` (`decomp/src/mn/mnvsmode/mnvsmode.c:318`, `:736`)
- `mnPlayersVSFuncStart` (`mnplayersvs.c:5439`, CSS load)
- `lbTransitionMakeTransition` / `lbTransitionSetupTransition` (`decomp/src/lb/lbtransition.c:244`, `:275`)
- `mnTitleProcUpdate` (`decomp/src/mn/mncommon/mntitle.c:771`), but confirm the overlay before using it for mode select. The mode-select state is in `decomp/src/mn/mncommon/mnmodeselect.c`.

**Not verified:** Training and 1P CSS hook addresses, stage-select hooks, most VS-menu hooks, and the CSS panel `set_panel_display_` path. These need an overlay table (ROM offset -> overlay -> RAM) to resolve.

---

## Quick index of key files
- `src/CharacterSelect.asm`: CSS grid, slots, random, costumes, hooks (about 7200 lines)
- `src/css/*.asm`, `src/CharacterSelectDebugMenu.asm`: CSS panel items
- `src/Menu.asm`: generic menu framework, SRAM export/import
- `src/Toggles.asm`: all toggles, profiles, mode-select settings button, SRAM blocks
- `src/Settings.asm`: VS tournament defaults hook
- `src/SRAM.asm`: SRAM layout and revision
- `src/VsRemixMenu.asm`: VS Game Mode buttons, Remix Modes page
- `src/SinglePlayerMenus.asm`: 1P menu rewrite
- `src/Stages.asm`: stage select grid, random roulette
- `src/Gallery.asm`, `src/CharacterDataScreen.asm`, `src/VsStats.asm`, `src/Credits.asm`, `src/Transitions.asm`, `src/Japan.asm`, `src/Random/Random.asm`, `src/Boot.asm`
- `src/Global.asm:58-122`: screen IDs
