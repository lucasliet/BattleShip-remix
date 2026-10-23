# F11 notes: Training mode extensions (Training.asm, ROADMAP T6.3)

Scope: `.toolchain/pkgs/smashremix-src/src/Training.asm` (3938 lines), with the Remix hooks it depends on
(`Speed.asm` handle_rate_training, `Item.asm` training item menu and HUD, `Toggles.asm` training entries, `AI.asm` CPU
reads). Decomp side: `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` (vanilla training scene, 2130 lines) and
`decomp/src/mn/mnplayers/mnplayers1ptraining.c` (the 1P/training character select, 3635 lines). The second file is the
CSS, not the in-match menu, so it is only relevant to the CSS steps.

Read only. No code was changed. Addresses are the Remix ROM address (first arg of `OS.patch_start`) and the RAM address
(second arg). Line numbers refer to the files named in each heading.

## 0. Scope table of Training.asm

| Scope | Lines | Purpose |
|---|---|---|
| header, constants, `struct` (port_1..4, table) | 1-137 | `toggle_menu` 0x80190979, `stage` 0x80190969, per-port struct (ID, type, costume, percent, spawn_id, spawn_pos, spawn_dir, clipping_id) |
| `load_character_` | 141-395 | Loads char, type, costume and CPU tag color from the training struct into `Global.vs` P_OFFSET/P_DIFF; Classic Sonic costume flag; on SSS load, fills the struct from the match info |
| `init_character_` | 399-463 | On respawn in training: custom spawn direction, ledge clipping, reset percent when "Reset Sets Percent" is on |
| `set_action_` | 467-502 | Starts a custom-spawned fighter on a ledge in the CliffWait action |
| `obey_cp_state_` | 506-544 | Calls the vanilla CPU routine (0x80190220) for each CPU port |
| `disable_during_pause_` | 548-575 | Disables movement of non-human ports while paused |
| `enable_during_unpause_` | 579-622 | Re-enables movement on unpause; keeps the B bit set so a held B is not replayed |
| `check_enter_menu_` | 626-669 | Lets any human port press Start to open the pause menu; records owner port |
| `check_leave_menu_` | 673-715 | Any port can close the menu (Start or B) and returns control to the owner |
| `check_disabled_item_spawn_` | 719-747 | A disabled port spawning an item while paused falls back to the owner port |
| `hold_to_exit_` | 751-809 | Exit requires holding A for 33 frames (32 + 1 for the fill); `held_frames` counter |
| `load_from_sss_` | 813-860 | Training loaded from stage select: clears reset counter, shield status, control object; sets spawn ids 0..3 |
| `load_from_reset_` | 865-945 | Training loaded from reset or exit: reset counter +1, revokes temporary owner, clears freeze, fixes a disabled human port |
| `disable_hud_` | 950-988 | "Disable HUD = ALL" (Toggles.entry_disable_hud) hides the game HUD; menu still works |
| `allow_reset`, `skip_advance` | 1033-1037 | Flags shared with the frame advance and Speed code |
| `advance_frame_` | 1039-1333 | D-pad shortcuts: D-pad up toggles freeze, D-pad right advances one frame, D-pad down cycles the model display, D-pad left quick reset; calls `run_` and `Speed.handle_rate_training` |
| `run_after_object_routines_` | 1339-1351 | Runs `update_actions_` after the object routines, so it also runs in freeze |
| `reset_percent_` | 1357-1372 | Subtracts the current percent from a fighter (through `Character.add_percent_`) |
| `set_custom_spawn_` | 1380-1432 | Saves the fighter position, facing and ledge id into the port struct and sets Spawn to Custom |
| `reset_counter`, `special_model_display` | 1438-1445 | Reset count (shown in the menu); mirror of the special model toggle |
| `draw_modal_frame_` | 1452-1467 | Reuses the vanilla pause frame routine for the custom menu border |
| `create_action_strings_` | 1475-1565 | Creates the action and frame strings for one port |
| `setup_` | 1569-1791 | Builds the custom menu overlay (legends, dpad images, hold-A bar, action room, the three per-frame object routines) |
| `run_` | 1795-2086 | Per-frame custom menu: input, cursor, costume max update, B close, Z open, L show action, hold-A bar |
| `action_control_object` | 2090 | Pointer to the action and frame control object |
| `update_frame_counts_` | 2097-2152 | Per-frame action change detection and frame counter per port |
| `update_actions_` | 2156-2277 | Chooses action names (shared table or character table) and recreates strings when the room changes |
| `play_custom_music_` | 2281-2298 | Hook 0x80190174: plays the Music entry from `bgm_table` |
| `player_shield_status` | 2302 | One byte per port for the shield break and OOS state machine |
| `shield_break_mode_` | 2308-2451 | CPU shield: infinite shield, or shield break with out-of-shield (OOS) inputs taken from the option table |
| `di_practice_mode_` | 2457-2492 | Every frame advance, sets CPU hitlag to 2 and the DI routine (practice of DI) |
| `spam_practice_` | 2496-2594 | Hook 0xB4E10: CPU repeats one move from `spam_command_table` on a timer (fixed or random interval) |
| `spam_practice_timer_` | 2600-2641 | Decrements the per-port spam timer, only for CPU ports outside hitlag |
| `fix_sector_z_crashes_` | 2645-2662 | Null player guard in Sector Z when fewer than two ports are loaded |
| `fix_pokemon_crashes_` | 2666-2725 | Hitmonlee, Starmie, Blastoise: use the throwing player when no opponent exists |
| `skip_crowd_cheer_` | 2730-2798 | Skips the crowd cheer on training, bonus 1/2 and bonus 3 / HRC (per toggle) |
| `handle_reset_menu_selections_` | 2803-2833 | On reset, keeps or clears CPU, item and speed menu values (Preserve Menu Selections) |
| string data (dpad, hold-A, action legend, CPU options, OOS, DI, spam) | 2837-2925 | Text for the custom menu |
| `oos_inputs_frame_1/2` and string tables | 2929-3042 | Button and stick values per OOS option and frame; option tables for every enum |
| character names and `string_table_char` | 3046-3246 | Names by char id; table of the 1P-style order shown in the Character entry (incl. Random at the end) |
| `scope id` | 3251-3366 | Named training character ids (order = display order) |
| `entry_id_to_char_id`, `char_id_to_entry_id` | 3368-3572 | Maps the menu entry index to the internal char id and back |
| spawn strings, `string_table_spawn` | 3576-3588 | Port 1..4 and Custom |
| macro `set_custom_spawn`, `spawn_func_1_..4_` | 3592-3620 | Set Custom Spawn action per port |
| `set_percent_` | 3622-3656 | Percent entry: resets the port, then adds the chosen percent |
| macro `tail_px`, `tail_p1..4`, `tail_table`, `toggle_table`, `type_table` | 3658-3696 | Per-port menu chain (Character, Costume, Type, Spawn, Set Custom Spawn, Percent, Reset Sets Percent) |
| macro `struct_to_tail`, `struct_to_tail_` | 3700-3750 | Copies the port struct into the menu values after a load |
| `info`, `head` | 3752-3757 | Custom menu root (Menu.info, 0x17/0x16 rooms, first entry "Port:") |
| `string_training_mode`, `string_table_music`, `bgm_table` | 3759-3819 | Music choices: Training plus every music toggle entry that can be randomised |
| `play_bgm_` | 3823-3857 | Plays the selected Music entry immediately (not on hammer or star) |
| `show_stage_bg_` | 3861-3897 | Hooks 0x803F4 and 0x1146C4: shows the stage background or the training background |
| custom menu entries | 3899-3913 | Shield Break, OOS, Music, Stage BG, CPU Teching, CPU DI (type, strength, direction, start on hit), D-pad controls, DI practice, spam (practice, interval, random), Preserve Menu Selections |
| data: `initial_model_display`, `p*_action_pointer`, `owner_player_port`, `hold_A_rect_*` | 3917-3934 | Runtime state |
| end | 3936-3938 | |

## 1. What the mode is

Training is a sandbox, not a scored mode: no stocks, no timer (`gSCManagerBattleState->time_limit` is infinite in
`sc1PTrainingModeInitVars`, decomp `sc1ptrainingmode.c:696`), no win condition. The mode is the vanilla training scene
with extra controls: up to four ports (human, CPU or disabled), per-port spawn and percent, a custom menu, frame advance
and quick reset from the D-pad, and CPU behaviour options (shield break, out of shield, DI, tech, spam).

Selection: the training screen is `Global.screen.TRAINING_MODE` = 0x36 (`Global.asm:113`), its character select is
`TRAINING_CSS` = 0x12 (`Global.asm:77`). The stage byte is `Training.stage` (0x80190969). No `VsRemixMenu.vs_mode_flag`,
no `SinglePlayerModes` flag and no `Global.match_info` flag identifies training. The screen id is the only selector
(`Global.current_screen`). `SinglePlayerModes.singleplayer_mode_flag` is read by the Bonus code only.

## 2. Flow

Entry from the main menu goes through the training CSS, the training stage select, then the match.

| Step | asm file:line | patched RAM address | what it does | decomp match |
|---|---|---|---|---|
| CSS save and return | `Toggles.asm:3325-3326` | 0x801375D8 (routine, read only) | Training CSS return routine, reached by the stored previous screen | `mnPlayers1PTrainingFuncRun` (`mnplayers1ptraining.c:3256`) for the CSS itself |
| Training loaded from SSS | `Training.asm:814` | 0x80190600 (ROM 0x116E20) | Clears reset counter, shield status, control object; sets spawn ids | no match found (the vanilla SSS load is not a named function in the training file) |
| Training loaded from reset or exit | `Training.asm:866` | 0x80190668 (ROM 0x116E88) | Counter +1, revoke owner, fix disabled human port | no match found |
| Scene start | `Render.asm:2406` | (jal `Training.setup_`) | Builds the custom menu, the pause groups and the action strings | `sc1PTrainingModeFuncStart` (`sc1ptrainingmode.c:1984`) |
| Per-frame update | `Training.asm:1044` (`advance_frame_`) | 0x8018DA40 (ROM 0x114260) | Frame advance, quick reset, calls `run_` | `sc1PTrainingModeFuncUpdate` (`sc1ptrainingmode.c:680`) and `sc1PTrainingModeUpdateAll` (`:650`) |
| Pause and menus | `Training.asm:627`, `674`, `951` | 0x8018D110, 0x8018D218, 0x8018D22C | Start opens pause, B or Start closes it, HUD hide | `sc1PTrainingModeCheckEnterMenu` (`:283`), `CheckLeaveMenu` (`:311`) |
| Exit | `Training.asm:756` | 0x8018D858 (ROM 0x114078) | Exit only after hold A for 33 frames, if the toggle is on | `sc1PTrainingModeUpdateExitOption` (`:577`) |

The stage select and results are shared with vanilla training. No Remix change to the results screen.

## 3. State and data

| Name | Address (RAM, from Training.asm) | Size | Written by | Read by |
|---|---|---|---|---|
| `Training.toggle_menu` | 0x80190979 | byte | `run_` (`:1907` sets SSB_UP on close, `:1939` sets CUSTOM_UP on Z) | `run_`, `update_actions_` (`:2183`), `SingleButtonMode.asm:58` |
| `Training.stage` | 0x80190969 | byte | training setup | `Spawn.asm:134` |
| struct port_1..4 | (table at `struct.table`) | 0x20 each | menu entries, `load_character_`, `set_custom_spawn_` | `load_character_`, `init_character_`, `Spawn.asm:127` |
| `reset_counter` | label | word | `load_from_reset_` (+1), `load_from_sss_` (0) | `setup_` (draw_number, `:1618`) |
| `special_model_display` | label | word | `load_from_sss_` (`:839`, copies the toggle) | not read in this file |
| `Toggles.entry_special_model` | 0x80461E38 | word | `advance_frame_` (D-pad down, `:1241`) | `advance_frame_`, the model display code |
| `allow_reset`, `skip_advance` | labels | word | `advance_frame_`, `Speed.handle_rate_training` | `advance_frame_` |
| `freeze`, `du_pressed`, `dr_pressed` | labels inside `advance_frame_` | word | D-pad handling | `advance_frame_` |
| `action_control_object` | label | pointer | `setup_`, `load_from_reset_` (clear), `command.asm` | `update_frame_counts_`, `update_actions_` |
| `player_shield_status` | label | 4 bytes | `shield_break_mode_` | `shield_break_mode_` |
| `spam_practice_.timer` | label | 4 bytes | `spam_practice_`, `load_from_reset_` (clear) | `spam_practice_`, `spam_practice_timer_` |
| `owner_player_port` | label | word (-1 = none) | `check_enter_menu_` | `check_leave_menu_`, `load_from_reset_` |
| `hold_A_rect_object`, `hold_A_rect_width` | labels | word | `setup_`, `run_` | `run_` |
| `show_action` | label (`run_`) | word | L button | `setup_`, `update_actions_` |
| `held_frames` | label (`hold_to_exit_`) | word | `hold_to_exit_` | `hold_to_exit_` |
| `bgm_table` | label | halfword per entry | data | `play_custom_music_`, `play_bgm_` |
| `oos_inputs_frame_1/2` | label | 12 x (halfword mask, halfword stick) per frame | data | `shield_break_mode_` |
| `dSC1PTrainingModeLagIntervals` (decomp) | `sc1ptrainingmode.c:153` | 4 x 2 bytes | data | `sc1PTrainingModeCheckLagTic` |

Remix-only files: none of the training data is in a file id. The Remix training menu uses `Render.load_file(0xC5)` (button
images, `file_pointer_1`) and `Render.load_file(File.CSS_IMAGES)` for the D-pad image (`Training.asm:1574-1575`). `Item.asm`
adds `TRAINING_MENU_IMAGE_TABLE` 0x02A0 and `TRAINING_HUD_IMAGE_TABLE` 0x0330 to file 0xFE (`Item.asm:22-23`), which
are Remix-only table offsets.

Strings are plain constants in the asm (`Training.asm:2837-3142`, `3759`, `3576`). The character names are in
`string_table_char` order (`:3145`). None of the strings is a Remix-only file.

Port symbols: `port/remix/remix_symbols.h` has no `REMIX_SYM_Training__*` entries (count 0). It has the Toggles entries
used here: `entry_game_speed` 0x80462910, `entry_disable_hud` 0x8046210C, `entry_hold_to_exit_training` 0x80461EBC,
`entry_skip_start_cheer` 0x804621C4, `entry_special_model` 0x80461E38, `entry_special_zoom` 0x80462944,
`entry_item_containers` 0x804628D8.

## 4. In-match logic (per frame, in the order it runs)

1. Game frame starts. `advance_frame_` (`Training.asm:1044`) runs in place of the vanilla frame-advance branch.
2. Save vanilla `skip_advance` in `t6` and in `skip_advance` (`:1053-1056`).
3. If D-pad menu is not "off" (`entry_dpad_menu` 0 or 1): if reset allowed, a D-pad left press plays FGM 0xA2, clears
   `allow_reset`, sets the screen interrupt with reset flag 1. Reset is then done by `load_from_reset_`.
4. Frame advance: a D-pad up press flips `freeze` (`:1257`). A D-pad right press sets `freeze` to 1 and forces one frame
   (`:1266`). If not frozen, `Speed.handle_rate_training` runs (`:1276`); it may force skip or extra frames. If frozen,
   `ra` is set to the freeze return (`:1285`), so the frame does not advance.
5. D-pad down (if not "off") cycles `Toggles.entry_special_model` 0 -> 1 -> 2 -> 3 -> 0 (`:1233-1241`).
6. `_skip`: if the screen interrupt is set (reset in progress), skip the menu update (`:1293-1296`). Otherwise `run_` (`:1299`).
   If frozen, call `Render.update_live_string_` for the selected entry so the value updates while paused.
7. `run_after_object_routines_` (`:1339`) runs after the object routines and calls `update_actions_` (also in freeze).
   The object routine group 0xD runs `update_frame_counts_` after action changes (`setup_`, `:1658`).
8. Per port, `di_practice_mode_` (group routine registered in `setup_`) sets hitlag 2 and the DI routine for every
   non-dead fighter when DI practice is on. `spam_practice_timer_` decrements the spam timer for CPU ports outside hitlag.
9. CPU behaviour hooks run inside the game (not in this file): `shield_break_mode_` (`:2308`) for CPU shield, `spam_practice_`
   (`:2496`) for the spam action, and `AI.asm` reads for tech (`:541-545`), DI first hit (`:6573-6577`), DI type (`:6604-6608`),
   DI strength (`:6630`) and direction (`:6631`).

Respawn (`init_character_`, `:399`): if the port spawn id is custom (>= 4), facing and ledge id come from the port struct;
Reset Sets Percent clears percent. Custom spawn is saved by `set_custom_spawn_` (`:1380`).

Reset (`load_from_reset_`, `:866`): reset counter +1; owner port revoked; `spam_practice_.timer` cleared; shield status
cleared; freeze cleared; a human port marked Disabled is changed back to Human (avoids a crash).

Stage change: the training stage id is stored in `Training.stage` and read in `Spawn.asm:134`. The Remix stage backgrounds
are chosen by `show_stage_bg_` (`:3861`) from the Stage BG toggle. No mid-match stage change.

Camera: none in this file (vanilla camera). The View option (vanilla decomp `sc1PTrainingModeUpdateViewOption`,
`sc1ptrainingmode.c:532`) is not replaced by Remix: the Remix custom menu has no View entry. Item spawning (vanilla
`sc1PTrainingModeUpdateItemOption`, `:471`) is also kept, plus the Remix extension below.

Items: the training item menu is extended in `Item.asm:262-308` (`extend_training_mode_item_menu_`): `maxIndex` = 0x11 +
`NUM_STANDARD_ITEMS` (13) = 0x1E, custom entries map to item id + 0x19. The HUD image for the held item is extended in
`Item.asm:311-365`. The taunt spawn (`Item.asm:1936-1990`, table `taunt_item_table_` at `:1910`) is not training-specific:
its per-port choice is a CSS option (`css/TauntItem.asm:12`). "Spawn por taunt" in the roadmap T6.3 is therefore a shared
item feature, not part of Training.asm.

Stage hazards and items in the match are the normal modes; training does not change them.

## 5. HUD and text

Each element of the Remix custom menu and overlay, with its drawing path. The N64 text path (`Render.draw_string`,
`Render.draw_number`, `Render.draw_texture_at_offset`) is what Training.asm uses, not the ImGui overlay.

| Element | Where drawn (asm) | Needs |
|---|---|---|
| "Press [Z] for Custom Menu" | `setup_` `:1583` | font + button sprite (file 0xC5, `c5_offsets.Z`) |
| "(Hold --)" and the hold-A bar | `setup_` `:1597-1611`, `run_` `:2020-2038` | font, A sprite, a rectangle (`Render.draw_rectangle_`) |
| "Reset Count:" + number | `setup_` `:1617-1618` | font, `Render.draw_number` of `reset_counter` |
| D-pad legend (Toggle Pause, Frame Advance, Model Display, Quick Reset) | `setup_` `:1620-1632` | D-pad image (`Render.file_pointer_2`, offset 0x0218), 4 rectangles, 4 strings |
| Action legend ("Show Action") | `setup_` `:1635-1637` | font, L sprite |
| Pagination legend ("Next/Prev") | `setup_` `:1640-1644` | font, R and Z sprites |
| Modal frame and transparent background | `setup_` `:1647-1649`, `draw_modal_frame_` | vanilla frame routine (0x8018EC28) |
| Action and frame strings per port | `create_action_strings_` `:1475` | font, room 0x39 or 0x17 (depends on menu), per-port number |
| Custom menu entries (names, values) | `Menu.info`/`Menu.entry` `:3752-3913` | font, value strings (tables above) |
| Spawn names "Port 1".."Custom" | `:3576-3580` | font |
| Item name image in the training HUD | `Item.asm:311-365` | file 0xFE table, item name image (file 0x1D appended) |

The port: the N64 text and sprite path is what the port already runs for vanilla, so no new host text is required for the
Remix training menu if the custom menu stays on that path. Whether the port can draw `Render.draw_string` in the training
scene is not verified here: the decomp scene uses its own sprite objects (`sc1PTrainingModeMakeMenuLabels`, `:1249`).
This is a risk, see section 8.

## 6. Dependencies

- StockMode, TimedStock, Stamina and Neutral Spawns: none used. Training has no stocks or timer.
- Results screen: not used in training (exit goes to the menu, not results).
- Game Speed: `Speed.asm` `handle_rate_training` (scope at `:159`, table at `:274`) reads `Toggles.entry_game_speed` (0 = 1/1, 1..12), the
  Stopwatch flag, and the training speed multiplier at RAM 0x80190B70. Its waiting table (`waiting_time_table`, `:274`)
  holds the 1.2x to 3x and 1/8 to 3/4 speeds. The port twin is `ftRemixSpeedTrainingRunFrames`
  (`decomp/src/ft/ftremix/ftremixspeed.c:166`), already hooked in `sc1PTrainingModeUpdateAll` (`sc1ptrainingmode.c:665`).
- Single Button Mode: `SingleButtonMode.asm:37-58` reads `Training.toggle_menu` to gate its pause check. The port twin is
  `FTREMIX_SBM_GATE_TRAINING_MENU` (`decomp/src/ft/ftremix/ftremixhooks.h:391`).
- Hazards: training uses the normal hazard checks, which skip hazards on the training screen only where the code says so
  (`Hazards.asm` many `TRAINING_MODE` branches). The training background (`Training.entry_bg`) is read at `Hazards.asm:7289`.
- CPU AI (`AI.asm`): tech (`:541-545`), DI first hit, DI type, strength and direction (`:6573-6631`). These depend on the M7
  AI milestone (T7.1). Without AI, the CPU options have no effect in the port.
- Item system: the custom item menu entries and HUD depend on the item steps of M5/M6 (`Item.asm`, `it/itremix/`).
- CSS: the training CSS (`mnplayers1ptraining.c`) is done (T5.1b). Not part of this mode's extension.
- Vanilla CPU routine `0x80190220` and the vanilla pause menu (CPU, Item, Speed, View, Reset, Exit) stay in use. The Remix
  custom menu is an extra menu (Z), not a replacement. Two menus exist: the vanilla SSB pause (`toggle_menu` = 0x02) and the
  Remix menu (0x03).

## 7. Port plan

The port runs the decomp (C), not the Remix ASM. The Remix ASM logic must be written in C, with the Remix RAM image only
for data. Existing port code: Game Speed only (`ftremixspeed.c`), Single Button Mode gate and the hold-A/held-frame data
are not ported. Each step names the files and what the lead sees on screen.

Ordering: steps 1-4 first (no dependency). Step 5 needs 3 and 4. Steps 6-9 need the item steps of M5/M6. Steps 10-13 need the
CPU AI milestone (M7, T7.1) for their effect, but their menu part can be built earlier. Steps marked "(M7)" depend on M7.

1. **Verify the speed gate.** Confirm that RAM 0x80190B70 (Speed.asm:161) is the vanilla training speed option
   (`sSC1PTrainingModeMenu.speed_menu_option` in `sc1ptrainingmode.c`). Docs only, no code. Files: this note. Lead sees: nothing.
   Depends on: none.
2. **Test Game Speed in Training (T6.3 step).** Set Game Speed to 2x and 1/4 in Training, with the vanilla speed at 1/1. Check
   the extra frames run. Files: none (test only, `remix-smoke-tests` skill). Lead sees: the Training match runs at the set
   speed and the vanilla Speed option at 1/1 does not override it. Depends on: step 1.
3. **Freeze in the port.** Add a freeze flag to `sc1ptrainingmode.c` and skip `ftRemixSpeedTrainingRunFrames` and `gcRunAll`
   while frozen, as the ASM does (`Training.asm:1268-1288`). Files: `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` (one
   function). Lead sees: pausing with the freeze flag holds the frame. Depends on: none.
4. **D-pad frame advance (one frame).** Add a C function for D-pad right (advance one frame) and D-pad up (toggle freeze),
   read from the man's button tap as the ASM does (`Joypad.check_buttons_all_`). Files:
   `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` (new static function, called from `sc1PTrainingModeUpdateAll`). Lead sees:
   D-pad right advances one frame; D-pad up freezes and unfreezes. Depends on: step 3.
5. **D-pad quick reset and reset counter.** Port `advance_frame_` D-pad left reset with the FGM 0xA2 sound, and the reset
   counter (+1 on reset, shown as "Reset Count:"). Files: `sc1ptrainingmode.c`, plus the reset path in `sc1PTrainingModeFuncStart`
   (or a new `sc1ptrainingreset.c`). Lead sees: D-pad left resets the training match and the count increases. Depends on: step 4.
6. **Item menu entries 0x11..0x1D (custom items).** Extend the item option count to 30 entries and map custom entries to the
   Remix item kind (menu index + 0x19 adjustment, `Item.asm:285-291`). Files: `sc1ptrainingmode.c` (item count and spawn call),
   `decomp/src/it/itremix/itremixspawn.h` (kind lookup). Lead sees: a custom item appears in the training item menu and spawns.
   Depends on: the item steps of M5/M6 (custom item spawn).
7. **Item count cap and spawn gate.** Resolve the conflict in section 8 (the port zeroes `gITRemixExtendedToggles` at training
   init, `sc1ptrainingmode.c:699-702`). Files: `sc1ptrainingmode.c`. Lead sees: custom items spawn from the training menu and
   the vanilla four-item cap still holds. Depends on: step 6.
8. **Training HUD item image.** Show the custom item name image in the training HUD (ASM `extend_training_mode_hud_`,
   `Item.asm:311-365`). Files: `decomp/src/if/ifremixhud.c` (or the item name table file). Lead sees: the held item name image
   for a custom item. Depends on: step 6.
9. **Taunt spawn check.** Confirm the shared taunt spawn (`Item.asm:1936`) works in training, with no training-only code.
   Files: none unless a bug is found. Lead sees: taunt with a chosen item in training spawns it. Depends on: the shared item
   steps.
10. **Custom menu: data and open/close.** Port the Remix menu structure (the entries in `Training.asm:3899-3913` and the per-port
    chain `tail_px`, `:3658`) and the Z open / B close flow (`run_`, `:1795`). Files: new `decomp/src/sc/sc1pmode/sc1ptrainingmenu.c`
    (entries and input) and `.h`. Lead sees: Z opens the custom menu, B closes it, the entries show. Depends on: step 2 (pause flow).
11. **Custom menu: per-port Character, Costume, Type, Spawn, Percent.** Port `tail_px` values to the training struct
    (`struct.port_1..4`) and `load_character_` (`:141`). Files: `sc1ptrainingmenu.c`, and a new `sc1ptrainingstruct.c` for
    the struct and load. Lead sees: a chosen character and costume load on the next reset. Depends on: step 10.
12. **Custom spawn and Reset Sets Percent.** Port `set_custom_spawn_` (`:1380`), `init_character_` (`:399`) and `reset_percent_`
    (`:1357`). Files: `sc1ptrainingstruct.c`. Lead sees: "Set Custom Spawn" saves the position and reset returns there.
    Depends on: step 11.
13. **Custom menu: CPU options, shield break and OOS, DI practice, spam practice, stage BG, music, D-pad menu, hold to exit.**
    Split into one file per option group (max three options per file): (a) shield break and OOS, `shield_break_mode_`
    (`:2308`), needs the OOS tables (`:2929-3042`); (b) spam practice, `spam_practice_` (`:2496`), with `spam_command_table`
    (`:2574`); (c) DI practice `di_practice_mode_` (`:2457`); (d) stage BG and music `show_stage_bg_` (`:3861`) and
    `play_custom_music_` (`:2281`); (e) D-pad menu and hold to exit (`hold_to_exit_`, `:751`). Files: `decomp/src/sc/sc1pmode/`
    (one new file per group). Lead sees: each option changes the CPU or the screen as in Training.asm. Effects of (a) to (c)
    need M7 (AI, T7.1). (a) and (b) depend on step 10. (d) and (e) do not depend on M7.
14. **Crash guards.** Check the port against `fix_sector_z_crashes_` (`:2645`), `fix_pokemon_crashes_` (`:2666`) and
    `skip_crowd_cheer_` (`:2730`). Files: only if a crash is seen. Lead sees: no crash with one player in Sector Z, with
    Hitmonlee, Starmie or Blastoise without an opponent, and no crowd cheer on training start when the toggle is on.
    Depends on: none.

## 8. Risks and unknowns

1. **Custom item spawn conflict.** `sc1ptrainingmode.c:699-702` sets `gITRemixExtendedToggles = 0` at training init, with a
   comment that cites `Item.asm prevent_1p_custom_item_spawns_`. In the ASM, that hook (`Item.asm:894-930`) zeroes the extended
   bitmask for any single-player flag other than Remix 1P or Allstar. It is not proven that training is excluded, and the
   training item menu may depend on the same bitmask. Needs a test in step 7 before any change. The hook's RAM address
   (0x8018D6FC) falls inside the decomp range of `sc1PTrainingModeUpdateViewOption` (0x8018D6DC), which suggests a different
   overlay shares the same RAM. Not resolved.
2. **Speed gate is a port variable, not the Remix RAM.** The port gates Game Speed with
   `sSC1PTrainingModeMenu.speed_menu_option == nSC1PTrainingModeMenuSpeedFull` (`sc1ptrainingmode.c:665`). The ASM reads RAM
   0x80190B70. The two are assumed equal (step 1). Not verified.
3. **Ordering of Game Speed and freeze.** The ASM runs `handle_rate_training` only on the non-frozen path (`Training.asm:1276`).
   The port runs `ftRemixSpeedTrainingRunFrames` every update, after `sc1PTrainingModeCheckLagTic` (`:661-665`), and has no freeze.
   Frozen frames with Game Speed above 1/1 may run extra frames in the port. Fix is step 3.
4. **Save state does not exist.** Training.asm has no save-state feature. The only reset is D-pad left (quick reset) and the
   Reset option. The brief's "save-state" item is not in this file. Nothing to port for that.
5. **No Remix training practice code in the port.** Shield break, OOS, DI practice, spam practice, CPU tech, the Remix menu,
   hold to exit and the D-pad controls are not in `decomp/src/ft/ftremix/`, `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` or
   `decomp/src/ai/` (grep for their names found only unrelated hits). Work is the whole list in section 7, steps 10 to 13.
6. **Text path.** Training.asm draws with `Render.draw_string` and `Render.draw_number` (N64 font). The decomp scene uses sprite
   objects built in `sc1PTrainingModeMake*`. Whether the port can run the Remix N64 text path in this scene is not verified.
   If not, the custom menu needs port-drawn text. The port overlay (`port/remix/remix_overlay.h`) is an option, but only for
   host-side text.
7. **Symbols.** `port/remix/remix_symbols.h` has no Training symbols. Any RAM read in the port must come from the decomp globals
   (e.g. `sSC1PTrainingModeMenu`) or from a new symbol added with the setup script. Do not hard-code 0x80190B70 in the port.
8. **Two menus.** The vanilla SSB pause menu and the Remix Z menu share `toggle_menu` (values 0x02 and 0x03). The port must keep
   that state byte or the two menus will open at the same time. Not checked in the decomp.
9. **Taunt spawn is shared.** Roadmap T6.3 lists "spawn por taunt". It is in `Item.asm` and `css/TauntItem.asm`, shared with other
   modes. A training-only test cannot prove the shared feature; the lead must test it in step 9.
10. **Hardcoded maxima.** `tail_px` costume max is 5 (`Training.asm:3667`) and the Classic Sonic offset is +6 (`:1871`). `run_`
    recomputes the costume max every frame, so the static 5 is only the start value. Keep the dynamic logic in the port.
11. **Item cap.** The vanilla pause menu caps training items at four on screen (`sc1ptrainingmode.c:492`,
    `sc1PTrainingModeGetItemCount`). The Remix file does not change this cap. Keep it.
12. **Not verified.** I did not build, run or read the Remix RAM image; all statements are from the source files. Line numbers
    were read from the current files on 2026-10-22.
