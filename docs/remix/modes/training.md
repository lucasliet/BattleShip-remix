# Training

The Remix Training mode extensions: a custom menu (Z), up to four ports, frame advance and quick reset on the D-pad, and CPU practice options (shield
break, out of shield, tech, DI, spam). Asm source: `Training.asm` (about 3900 lines, in `.toolchain/pkgs/smashremix-src/src`), with `AI.asm` (CPU tech and DI),
`Speed.asm` (`handle_rate_training`), `Item.asm` (training item menu and HUD), `Spawn.asm`, `Toggles.asm` (Training entries) and `SingleButtonMode.asm`.
Port code: `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` (the vanilla scene with the hooks), `sc1ptrainingremix*.c` (D-pad, ports, music, menu, legend,
actions, exit, spawn, single button gate), `sc1ptrainingmenuentries.c` and the `*rules.c` files (pure rules), `decomp/src/ft/ftremix/ftremixtraining.c`
(fighter side), `ftremixaiteching.c` / `ftremixaidi.c` (CPU tech and DI), `port/remix/remix_training.cpp` (option registry).
Tests: `tests/remix/test_training_rules.c`, `test_training_menu_rules.c`, `test_training_menu_entries.c`, `test_training_port_rules.c`,
`test_training_leftover_rules.c`, `test_ai_*`; `cmake --build build --target run_remix_mode_tests`. Smoke recipes at the end. Shared plumbing:
[`../modes_base.md`](../modes_base.md).

## How the Remix does it

Training is a sandbox: no stocks, no timer (the time limit is infinite), no win condition. The only selector is the screen id (`Global.screen.TRAINING_MODE`
0x36, character select `TRAINING_CSS` 0x12); `Training.toggle_menu` (byte 1 playing, 2 vanilla pause menu, 3 Remix menu) and `Training.stage` are its state.
The stage select and the exit are the vanilla ones; there are no results.

### Scopes of `Training.asm`

| Scope | Behaviour |
|---|---|
| `load_character_`, `init_character_`, `set_action_` | on load, fills the per-port struct (below) from the match info the first time and writes character, type, costume and CPU tag colour into the VS player info afterwards; Classic Sonic is costume + 6 in the struct; when a fighter is created with a Custom spawn it takes the saved facing and ledge clipping id, "Reset Sets Percent" clears its percent, and a fighter saved on a ledge starts in CliffWait |
| `obey_cp_state_` | the vanilla CP option (Stand, Walk, Evade, Jump, Default) applies to every CPU port, not only one |
| `disable_during_pause_`, `enable_during_unpause_` | every port that is not N/A is locked while paused; on unpause B stays set so a held B is not replayed |
| `check_enter_menu_`, `check_leave_menu_`, `check_disabled_item_spawn_` | any human port can open and close the pause; the owner port is recorded; a disabled port spawning an item falls back to the owner |
| `hold_to_exit_` | with the `Hold to Exit (Training)` toggle, Exit needs A held 33 frames (32 plus the exit frame) |
| `load_from_sss_`, `load_from_reset_`, `handle_reset_menu_selections_` | counters, shield status, freeze and owner are cleared; reset counter +1 on a reset; a human port set to Disabled is changed back to Human; "Preserve Menu Selections" keeps the CP, Item and Speed values and the cursor of the vanilla menu across a reset |
| `disable_hud_` | Disable HUD = ALL hides the game HUD; the menu keeps working |
| `advance_frame_` | D-pad up toggles the freeze, right advances one frame, down cycles the model display (`Toggles.entry_special_model` 0..3), left is a quick reset (FGM 0xA2); D-pad Controls entry: On, Reset Only, Off; calls `Speed.handle_rate_training` only on the non-frozen path |
| `run_after_object_routines_`, `update_frame_counts_`, `update_actions_` | per fighter the action name and the frame counter in that action (frames outside hitlag plus the first hitlag frame, restarting on an action change), shared names for action ids below 0xDC and `Character.action_string.table` above |
| `play_custom_music_`, `play_bgm_` | Music entry: entry 0 is the Training theme, the others the toggleable tracks; A in the menu plays it now (not over the hammer or star theme) |
| `shield_break_mode_` | CPU shield: Off, On (shield break with out-of-shield inputs) or Infinite; a per-port state machine (shield, stun, OOS frame 1, OOS frame 2) with 12 OOS options read from `oos_inputs_frame_1/2` |
| `di_practice_mode_` | every frame advance sets the hitlag of non-dead fighters to 2 and runs the DI routine |
| `spam_practice_`, `spam_practice_timer_` | a CPU repeats one of 13 moves from `spam_command_table`, fixed or random interval (byte timer, 999 wraps), only outside hitlag and while Shield Break is off |
| `show_stage_bg_` | Stage Background entry: the stage background instead of the training one (also for the magnify bubble) |
| `fix_sector_z_crashes_`, `fix_pokemon_crashes_` | null player guard in Sector Z; Hitmonlee, Starmie and Blastoise use the throwing player when no opponent exists |
| `skip_crowd_cheer_` | skips the start crowd cheer in Training, Bonus 1/2 and Bonus 3 / HRC by the Skip Start Cheer toggle |

CPU tech (`AI.asm` 443-660) and CPU DI (`cpu_di_`, `di_input`, 6533-6650) read the Z menu options: Tech (none, backward, forward, in place, left, right
relative to facing), DI Type, Strength, Direction and Start on Hit. In Training the DI parameters come from the menu; outside Training level 10 rolls 0-8
(none, four smash and four slide DIs), lower levels need the advanced AI (Remix 1P, or Improved AI outside classic 1P). The asm reads stored per-port values
but writes a local copy that starts at -1 outside a combo. The hit count is `SCPlayerData.combo_count_foe`, and DI applies during hitlag (hence
DI Practice forces hitlag 2).

### Per-port struct

Four records, stride 0x24 (`struct.port_1..4`): ID (training entry index), type (0 Human, 1 CPU, 2 Disabled; default 2), costume, percent, spawn_id (0-3 Port 1-4,
4 Custom), spawn_pos x and y, spawn_dir (default FACE_LEFT), clipping_id (-1, else the ledge id when the fighter was in CliffWait). A change of character,
costume or type takes effect on the next training load (reset or exit-reset); Type, Percent and Spawn are fighter field values. "Set Custom Spawn"
saves the fighter of that port (position, facing, ledge) and makes the spawn Custom. A KO respawn is the vanilla halo respawn: `init_character_` tests the
return address of `ftManagerMakeFighter`, so Custom spawn, ledge clipping and Reset Sets Percent apply only when a fighter is created (match start, quick
reset, Reset).

### The Z menu

Z in the pause opens a menu (`Menu.asm`, 12 rows per page at (68, 50), row height 12): first entry "Port:", then the seven entries of that port (Character, Costume,
Type, Spawn, Set Custom Spawn, Percent, Reset Sets Percent), then the shared entries (Shield Break, OOS, Music, Stage Background, CPU Teching, CPU DI Type /
Strength / Direction / Start on Hit, D-pad Controls, DI Practice Mode, Spam Practice, Spam Interval, Spam Interval Random, Preserve Menu Selections): 23 entries
on two pages. R / Z turn the pages, B closes it; the vanilla pause menu stays hidden while it is open.

### Items

`Item.asm` `extend_training_mode_item_menu_` raises the item menu to the 13 Remix standard items after the vanilla ones (entry number + 0x1C is the item kind;
`TRAINING_MENU_IMAGE_TABLE` 0x02A0 holds the name images in file 0xFE) and the held item name image comes from `TRAINING_HUD_IMAGE_TABLE` 0x0330 (kinds 0x2D..0x39
map to entry kind - 0x2D). The vanilla cap of four items on screen stays. Start With (`css/StartWith.asm`, applies to Training) gives the chosen item at GO.

### Text and sprites

All Training text is `Render.draw_string` with the Remix font. Elements and positions (screen units, Y is the top of the glyph box):

| Element | Position | Group and condition |
|---|---|---|
| "Press [Z] for Custom Menu" (Z sprite at 117.5, 49) | centred x 160, y 50, scale 1.0 | with the vanilla pause menu up, hidden while the Z menu is open |
| "Reset Count:" and the count | label left x 99.5, value right x 221, y 25, scale 1.0 | only while paused |
| L glyph, "Show" / "Action" | glyph (20, 16) scale 0.875; text x 20, y 27.25 and 38.5 | with the pause |
| R / Z pagination legend | R (272, 16), "/" (288, 16), Z (294, 15); "Next/" and "Prev" x 272, y 27.25 and 38.5 | only with the Z menu open |
| D-pad legend | glyphs at (45.25, 200), (45.25, 215), (160, 200), (160, 215) with yellow 2x2 marks; "Toggle Pause" (63.25, 202), "Frame Advance" (178, 202), "Model Display" (178, 217), "Quick Reset" (63.25, 217) | toggles shown when D-pad Controls is On, Quick Reset when On or Reset Only |
| "(Hold --)" | x 169, y 166, scale 0.875, A glyph (202, 165) scale 0.9375, bar at (73, 179) 2 high, +2 per held frame to 64 | only with Hold to Exit on and the cursor on Exit |
| Action rows | name at (20, 55 + 26 * port), frame count at (20, 66 + 26 * port), scale 0.75 | while Show Action is on |

Button glyphs (A, B, L, R, Z, +, D-pad) are textures of files 0xC5 and 0xA06, not text; the item names are images, not text.

## How the port does it

### Registry and Z menu

The Remix options live in a registry (`port/remix/remix_training.h`): each is the CVar `gRemix.Training.<Key>`, editable in the port menu (**Remix > Training**) and
overridable with `SSB64_REMIX_TRAINING="ShieldBreak=2;SpamPractice=11;..."`. The Z menu is real: `sc1ptrainingremixmenu.c` (input, pages, actions), `sc1ptrainingremixmenudraw.c`
(frame, text, cursor), `sc1ptrainingmenuentries.c` (the 23 entries with the asm strings and ranges), `sc1ptrainingmenurules.c` (navigation) read and write the same
registry, so the port menu and the Z menu agree. The stick and the C buttons move the cursor, the D-pad keeps its shortcuts, A or START runs the action of an entry.

### Hooks and modules

| Asm | Port |
|---|---|
| `load_character_`, per-port chain | `sc1PTrainingRemixSetupPorts` (`sc1ptrainingremix.c`, rules in `sc1ptrainingportrules.c`) called from `sc1PTrainingModeInitVars`: Type (Auto, Human, CPU, Disabled), Character and Costume (Auto keeps the CSS choice); the menu owner is always Human; `sc1PTrainingRemixGetPortFighter` |
| `init_character_`, `set_custom_spawn_`, `set_action_` | `sc1PTrainingRemixSpawnSave/Select/Clear/ApplyDesc/EnterLedge` (`sc1ptrainingremixspawn.c`); `ftRemixPanelGetInitialDamage` for Reset Sets Percent; the KO respawn is left alone, as in the asm |
| `obey_cp_state_`, pause locks | `sc1PTrainingModeUpdateDummyBehavior` over every CPU; `sc1PTrainingModeSetPortsLocked` |
| `check_enter_menu_`, `check_leave_menu_` | `sc1PTrainingModeCheckEnterMenu` / `CheckLeaveMenu` |
| `hold_to_exit_` | `sc1ptrainingremixexit.c` (`sc1PTrainingRemixExitIsRequested`), hooked in `sc1PTrainingModeUpdateExitOption` |
| `load_from_sss_`, `load_from_reset_` | `sc1PTrainingRemixStartScene`, `sc1PTrainingRemixOnReset` (reset count), `ftRemixTrainingStartScene` |
| `handle_reset_menu_selections_` | `sc1PTrainingRemixMenuIsPreserved` in `sc1ptrainingmode.c` |
| `disable_hud_` | `ifRemixHudDisabledAll` |
| `advance_frame_` | `sc1PTrainingRemixPollDpad` (left reset, up freeze, right advance one frame, down model display), `sc1PTrainingRemixIsFrameHeld` skips the frame while frozen; Game Speed through `ftRemixSpeedTrainingRunFrames` only when the vanilla Speed option is 1/1 |
| `update_frame_counts_`, `update_actions_` | `sc1ptrainingremixaction.c` (names from the Remix RAM image tables `Action.shared_action_string_table` and `Character.action_string.table`) |
| `play_custom_music_`, `play_bgm_` | `sc1PTrainingRemixStartMusic/PlayMusic/PollMusic` (stores the track as the stage track `gMPCollisionBGMDefault`, keeps the pause volume; does not use `gmRemixBgm*`, which also write `BGM.current_track`) |
| `shield_break_mode_` | `ftRemixTrainingShieldBreakInput` (`ftremixtraining.c`, called after the player input of the fighter in `ftmain.c`), state machine `sc1PTrainingRulesShieldStep` and the OOS tables `sc1PTrainingRulesGetOosInput` (`sc1ptrainingrules.c`) |
| `spam_practice_`, `spam_practice_timer_` | `ftRemixTrainingSpamSetObjective` (called before the objective switch in `ftComputerProcessObjective`, `ftcomputer.c`), `sc1PTrainingRulesSpamReloadTimer` |
| `di_practice_mode_` | `ftRemixTrainingUpdateFighters` (hitlag 2 and `ftCommonDamageCommonProcLagUpdate` on fighters past the KO statuses) |
| CPU Teching, CPU DI | `ftRemixAiTechByMenu` (`ftremixaiteching.c`; a failed tech counts as a VsStats TechMiss) and `ftremixaidi.c` with the pure `ftremixaidirules.c` |
| `show_stage_bg_` | `grWallpaperMakeDecideKind` (`grwallpaper.c`) takes the non-training branch when Stage Background is on |
| items | the Item row adds `SC1PTRAININGREMIX_ITEM_COUNT` (13) Remix items (`sc1PTrainingRemixGetItemKind`) and the name images come from file 0xFE (`sc1PTrainingModeGetRemixItemName`); the held item entry is `sc1PTrainingLeftoverRulesGetHeldItemEntry`; the log has `training item hud: held entry N sprite WxH` |
| Start With | `ftRemixStartItemTrainingBegin` (`ftremixstartitem.c`), called in `sc1ptrainingmode.c` |
| Single Button Mode gate | `sc1PTrainingRemixGetSingleButtonGate` returns `FTREMIX_SBM_GATE_TRAINING_MENU` while a Training menu is up |
| crash guards | `itsawamura.c`, `itstarmie.c`, `itkamex.c` owner fallback; Sector Z: `sc1PTrainingRemixSetupPorts` sets `pl_count` / `cp_count` to the loaded fighters so the random player never exceeds the list; `skip_crowd_cheer_` via `gmRemixToggle(skip_start_cheer)` in `sc1ptrainingmode.c`, `sc1pbonusstage.c`, `sc1pgame.c` |
| text | `sc1ptrainingremixlegend.c` (Z prompt and D-pad legend), `sc1ptrainingremix.c` (Reset Count), `sc1ptrainingremixaction.c`, `sc1ptrainingremixexit.c`, `sc1ptrainingremixmenudraw.c` through `remix_hud_text` and native sprites for the glyphs |

### Deviations from the asm

- **Registry.** The Z menu entries are backed by the port registry, so the port menu and `SSB64_REMIX_TRAINING` set the same values; Auto choices (Type, Character,
  Costume, Spawn) are a port addition meaning "keep what the character select chose".
- **Shield Break only for CPUs.** `shield_break_mode_` has no controller kind check in the asm but its comment says "Forces CPUs to shield"; on a human it would
  lock the player into the shield, so the port applies it to CPU fighters.
- **Overlay order.** The asm draws the action rows in room 0x39 under the translucent Z menu panel; the port overlay is always on top, so the rows are hidden while
  the Z menu is open. The "(Hold --)" text leaves out the dashes so the A glyph is not crossed out, and its glyph group is 0x1C (the asm group 0xF is the battle pause
  legend group in the port).
- **Music list.** The Music entry has the 17 fixed entries of the table; the Remix list appends the toggleable MIDI tracks (an assembler loop with no symbol).
- **Single Button Mode gate** derives `Training.toggle_menu` from the battle pause status and the Z menu state (`sc1PTrainingSbmGetToggleMenu`) instead of reading the RAM byte; the Training menu path keeps START and, in mode B, B.

Open gaps: `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

Scripts: `.claude/skills/remix-smoke-tests/scripts/vs.sh` and `css.sh` (from the repository root); Training needs `dangerouslyDisableSandbox` like any game run.

- Boot without the CSS: `SSB64_TRAIN_FKINDS=<human>,<cpu>` (stage from `SSB64_VS_STAGE`); Training CSS: `SSB64_START_SCENE=18` with `inputs/training_css_pick.txt`
  (the 1P hand climbs to the Mario portrait, A, START, A on the stage select; the log shows `training port` lines for the picked fighters).
- Shield Break: `SSB64_TRAIN_FKINDS=0,8 SSB64_REMIX_TRAINING="ShieldBreak=2"` (Kirby holds the shield); `ShieldBreak=1;Oos=3` and hit the shield with
  `inputs/training_oos_platform.txt` (`SSB64_VS_STAGE=6`): after the stun the CPU grabs, the log has `training oos`.
- Tech: `inputs/training_tech_platform.txt` with `Tech=<1..5>;ShieldBreak=0;Percent2=30`: the log shows `training tech ... teched 1 status N` (Roll Left 73, Roll Right 74, In Place 81).
- Spam: `SpamPractice=<move 1..13>;SpamInterval=40`: the log has `training spam: port N option N timer N` once per interval (a Jab every 40 frames was seen).
- CPU DI: `DiType=3;DiStrength=1;DiDirection=1;DiFirstHit=1` and hit the CPU: the stick wiggles in hitlag.
- Percent: `Percent1=88;Percent2=123` starts the match at 88% and 123% (Reset Sets Percent on).
- Stage Background: `StageBg=1` shows the Dream Land background instead of the training one.
- Pause and menu: `inputs/pause_start.txt` opens the pause; Z opens the menu, R / Z turn the pages, B closes it; D-pad up freezes, right advances one frame, left resets
  and increments the reset count.
