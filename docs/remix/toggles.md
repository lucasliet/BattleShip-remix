# Remix settings and toggles

Covers the 531 entries of `Toggles.asm` (Remix, Gameplay, Music, Stage and Pokemon pages), how the port stores and reads them, and the port's own Remix menu pages. Code: `port/remix/remix_toggles.{h,cpp}` (registry), `port/remix/remix_display_settings.cpp`, `port/gui/PortMenu.cpp` (`AddMenuRemix`), `decomp/src/gm/gmremixtoggle.{h,c}` (C API) and the per-setting readers named below.
Test a setting with `SSB64_REMIX_TOGGLES="<title>=<value>"` on any smoke script, for example `SSB64_REMIX_TOGGLES="Hitstun=1" .claude/skills/remix-smoke-tests/scripts/vs.sh 0,8 .claude/skills/remix-smoke-tests/inputs/none.txt hitstun` (see `.claude/skills/remix-smoke-tests/SKILL.md`). The pure rules of several settings have headless tests: `cmake --build build --target run_remix_mode_tests` (`tests/remix`).
Audit ledger (every item closed): `docs/remix/patch_audit.md`, groups g3, g5, g8 and g9. Announcer details: `docs/remix/announcer.md`.

Profile columns used below are CE/TE/NE/JP = Community, Tournament, Netplay, Japanese (`Toggles.profile_defaults_*`); a single number means the same default in all four.

## 1. Registry, storage and the C API

### 1.1 Registry

`remix_toggles_init` (called from `port/port.cpp`) walks the five `Menu.entry` chains of the Remix RAM image (`Toggles.head_remix_settings`, `head_gameplay_settings`, `head_music_settings`, `head_stage_settings`, `head_pokemon_settings`), skips title nodes, and records for each entry the address (`REMIX_SYM_Toggles__entry_*`), type, min, max, A-press function, string table, title and the four profile default words. It expects exactly 531 entries; any other count logs an error, leaves the registry empty and every reader returns 0 (no Remix menu appears).

| Group (page) | Entries | Notes |
|---|---|---|
| Remix Settings | 32 | miscellaneous, display, accessibility |
| Gameplay Settings | 31 | |
| Music Settings | 301 | 7 fixed settings, one sub-profile row, 16 built-in Random Music tracks and 277 custom MIDI tracks |
| Stage Settings | 153 | 8 fixed settings, one sub-profile row, 16 built-in Random Stage toggles and 128 custom stages |
| Pokemon Settings | 14 | Pokemon SFX and 13 Pokemon |

The asm packs the values into an SRAM block and the Options screen is replaced by the Remix button of Mode Select. The port keeps each value as the CVar `gRemix.Toggle.<n>` (n is the position of the entry sorted by RAM address, that is declaration order in `Toggles.asm`; the CVar default is the Community value) and edits them in the ImGui menu. There is no `Custom` profile value.

### 1.2 Setting a value

| Way | Effect |
|---|---|
| ImGui menu, Remix section | Writes the CVar (persists). See 1.4 for what the page shows. |
| Profile buttons (Remix Settings page) | `remix_toggles_load_profile(0..3)` writes the profile column of every entry on all five pages and clears run overrides. |
| `SSB64_REMIX_PROFILE=<0..3>` | Run override: every entry takes the CE, TE, NE or JP column. Not written to CVars. |
| `SSB64_REMIX_TOGGLES="<title>=<int>;<title>=<int>"` | Run override applied after the profile. The title is the exact `Toggles.asm` title, case-insensitive (`Parrying (BETA)`, `Default CPU LVL (V.S.)`, `SFX  Volume` with two spaces); the value is clamped to the entry range; an unknown title logs `names no toggle`. Not written to CVars. |
| `remix_toggle_set(entry_addr, value)` | Clears the override of that entry and writes the CVar. Used by the stage panel (Hazard Mode) and the Training D-pad cycle (Special Model Display). |

Caveat: the title lookup sets every entry with that title. Built-in track and stage names repeat between the Music and Stage pages (`Congo Jungle`, `Dream Land`, `Duel Zone`, `Final Destination`, `How To Play`, `Hyrule Castle`, `Meta Crystal`, `Mushroom Kingdom`, `Peach's Castle`, `Planet Zebes`, `Saffron City`, `Sector Z`, `Yoshi's Island`), and the three `Load Profile:` rows share one title, so `Congo Jungle=0` changes both the Random Music and the Random Stage toggle. To change one of them alone write its CVar or use the ImGui page.

### 1.3 Reading a value in C

```c
#include "gm/gmremixtoggle.h"
gmRemixToggle(hitstun)     /* Toggles.entry_hitstun, no guard */
gmRemixToggleVS(hitstun)   /* 0 in Bonus and 1P Game (Toggles.single_player_guard) */
```

`name` is the `Toggles.asm` label without `entry_`; the symbol is `REMIX_SYM_Toggles__entry_<name>` in the generated `port/remix/remix_symbols.h`. `remix_toggle_get`, `remix_toggle_set` and `remix_toggle_group_snapshot` take the symbol address (the CSS high-score panel lists the Gameplay and Pokemon values with the snapshot, `mn/mnplayers/mnremixcsshighscores.c:27`). `gmRemixToggleModeIsGuarded` (`gm/gmremixtoggle.c`) is true when `gSCManagerBattleState->game_type` is Bonus or 1P Game. The asm uses `Toggles.guard` (any mode) for most settings and `Toggles.single_player_guard` only for Hitstun, Hitlag, DI, Shield Stun, Momentum Slide and Japanese Stun/Sleep; the port follows the same split. A setting that is off must leave the vanilla path untouched, and every reader sits under `#ifdef REGION_REMIX`.

### 1.4 The ImGui pages

`PortMenu::AddMenuRemix` (`port/gui/PortMenu.cpp:1368`) adds a Remix menu with the sidebar sections Remix Settings, Gameplay Settings, Music Settings, Stage Settings, Pokemon Settings (one per group, two columns; bool as checkbox, string-table entries as combo box, others as int slider), then Player Panel P1..P4, Player Tags, Modes, Tag Team and Training (section 7). It exists only in `REGION_REMIX` builds with a built registry. The menu is built when the window opens, before the game archive and the toggles load, so `PortInit` adds the Remix section afterwards with `PortMenu::AddRemixSection` (right after Settings); before 2026-11-05 the section was never shown because `AddMenuRemix` found no toggles. Screenshots of a page: `SSB64_PORT_MENU_OPEN=1 SSB64_PORT_MENU_PAGE="Remix/<page>"` through `scene.sh`.

A-press routines (`port/remix/remix_toggle_actions.{h,cpp}`): in `Toggles.asm` a row with an A function runs it when A is pressed on the row. The ImGui widgets have no A press, so the page runs the same routine through `remix_toggle_action_apply(entryAddr)` after the value changes (a combo box, slider or checkbox callback). Every row is listed on its page. What each routine does:

| Row | Asm routine | Port |
|---|---|---|
| BGM Volume | `update_bgm_volume` | sets the BGM volume again (only while a song plays) |
| SFX Volume | `update_fgm_volume` | sets the sound volume and plays the test sound `0x020` |
| Punish Failed Z-Cancel | `punish_fgm_` | plays the FGM of the chosen punishment (`gmRemixToggleActionGetPunishFgm`) |
| Default CPU LVL (V.S.) | `set_cpu_level` | `gmRemixDefaultCPULevelRefresh` (`gm/gmremixspawnsetup.c`): a value other than 0 (DEFAULT) becomes the CPU level of the four slots at once and marks the default as set, so the once-per-boot `gmRemixDefaultCPULevelSetup` does not overwrite it |
| Menu Music | `play_menu_music_` | `gmRemixMenuMusicAudition` (`gm/gmremixmenumusic.c`): plays the chosen track now and always restarts it. DEFAULT rolls the 64 track (85 of 88) or MELEE, MENU 2, BRAWL (1 of 88 each) as the asm does (the title screen roll of `alt_menu_music_` is 85/5/5/5 of 100, a different one); RANDOM, RANDOM CLASSICS and RANDOM ALL pick a random track; OFF stops the music. Four presses in a row that land on FILESELECT_SM64 play its `itsatrap` variant (MAIN_TRAP_SM64), which the menus keep until a press lands on another track; a fifth press starts the count again (`_sm64_trap_check`) |
| every per-track Random Music bool (Music page) | `preview_bgm_` | plays the BGM id held in the entry word `0x24` (`remix_toggles::Toggle::extra`) |
| "Random Music Toggles:", "Random Stage Toggles:", "Toggle:" (Pokemon) title rows | `toggle_all_` | one button per title on its page ("Random Music Toggles: toggle all", "Random Stage Toggles: toggle all", "Toggle all"), `remix_toggle_action_toggle_all`: sets every toggle after the title, up to the end of the page, to the opposite of the first one (`gmRemixToggleActionGetToggleAllValue`) |
| "Load Profile:" rows (Music and Stage pages) | `load_sub_profile_` | the INT row is hidden (its value only picked the profile) and the page lists one button per profile instead ("Load <name> music profile", "Load <name> stage profile", names from the row's string table), `remix_toggle_action_load_sub_profile`: copies `Toggles.music_profiles[n]` or `stage_profiles[n]` word by word into the toggles after the title that follows the row |

Deviation, Random Music preview: the asm sets `reset_menu_music` and restarts the menu music through `play_menu_music_` when the settings screen is left. The port menu is an overlay that also opens during a match, so the first preview remembers the BGM id playing at that moment (`syAudioGetBGMPlaying`, -1 when none) and `remix_toggle_action_menu_closed`, called by `PortMenu::UpdateElement` on the visible-to-hidden change, puts that track back (or stops the music when none was playing). Pressing Menu Music after a preview forgets the remembered track: the track just chosen stays.

Deviation, scope of the profile loaders: the asm loops until the chain ends and reads one word per remaining node; the port writes exactly the toggles that follow the title and stops there, so a profile table longer than the page cannot spill into another page.

The asm Options-screen port (`disable_options_functionality_`) and the fifth Mode Select button are replaced by this menu; the Options screen stays vanilla. The asm `Other Screens` entries (Debug, View Intro, View How to Play, View Credits, View Gallery) are the buttons of Remix > Other Screens (`scRemixScreensOpen`); the Gallery also opens with `SSB64_START_GALLERY` (`sc/scmanager.c:959`).

## 2. Remix Settings page

- **Skip Results Screen** (`skip_results_screen`; 0 OFF, 1 ON, 2 ON RESET; default 0/0/1/0)  
  Asm: GameEnd.asm `update_screen_`: at the end of a VS match 1 goes straight to the CSS, 2 does so only when the match ended with the reset combo (A+B+R+Z). Holding R or L always shows the results.  
  Port: `gmRemixGameEndSelectScene` (gm/gmremixgameend.c:97), called where `scVSBattleStartScene` picks the next scene. Replaces the port's Skip Results enhancement in Remix builds.

- **Hold To Pause** (`hold_to_pause`; 0 off, 1 on; default 0/1/1/0)  
  Asm: Pause.asm `hold_`: START opens the pause only after being held for more than 30 frames (one counter per port); a tap does nothing.  
  Port: `ftRemixPauseStartRequested` (ft/ftremix/ftremixpause.c:27), used by `ifCommonBattleGoUpdateInterface`.

- **CSS Panel Menu** (`css_panel_menu`; 0 off, 1 on; default 1/0/1/1)  
  Asm: CharacterSelectDebugMenu.asm: draws the per-port panel button on the CSS (the panel holds the per-port options, section 7).  
  Port: mn/mnplayers/mnremixcsspanelmenu.c:182 and mnremixcsspanelmenuinput.c:270. The same options are editable in Remix > Player Panel P1..P4.

- **Color Overlays** (`practice_overlay`; 0 off, 1 on; default 0)  
  Asm: Practice.asm `overlay_colour_`: tints each fighter with the colour of its state (landing, hard landing, slow and fast turn first actionable frames and others).  
  Port: `ftRemixOverlay*` (ft/ftremix/ftremixoverlay.c:209), hook in `ftMainProcPhysicsMapDefault` (ft/ftmain.c:2242).

- **Cinematic Entry** (`cinematic_entry`; 0 DEFAULT, 1 ALWAYS, 2 NEVER; default 0)  
  Asm: Camera.asm `cinematic_entry_`: ALWAYS forces the cinematic camera focus on the countdown, NEVER denies it, DEFAULT keeps the random focus. Camera Mode FIXED and SCENE force the default.  
  Port: `gmRemixCameraCinematicFocus` (gm/gmremixcameramode.c:388), hook in `ifCommonEntryAllThread` (if/ifcommon.c:2636).

- **Flash On Z-Cancel** (`flash_on_z_cancel`; 0 off, 1 on; default 0)  
  Asm: Practice.asm `flash_on_z_cancel_`: the fighter flashes (colanim 8) when an aerial lands inside the Z-cancel window.  
  Port: `ftRemixZCancelFlashOnLanding` (ft/ftremix/ftremixzcancel.c:253), called after the landing status in `ftCommonAttackAirProcMap`.

- **FPS Display *BETA** (`fps`; 0 OFF, 1 NORMAL, 2 OVERCLOCKED; default 0)  
  Asm: FPS.asm: N64 frame-rate overlay; OVERCLOCKED counts against the overclocked frame time.  
  Port: Adapted. `remix_display_settings_sync` (port/remix/remix_display_settings.cpp:43) opens the port Stats window on the off-to-on edge of the toggle; closing the window sticks until the next edge. NORMAL and OVERCLOCKED behave the same.

- **Model Display** (`model_display`; 0 DEFAULT, 1 HIGH POLY, 2 LOW POLY; default 0/0/1/0)  
  Asm: Costumes.asm `force_hi_lo_poly_`: forces the high or low polygon model of every fighter at spawn and in the VS pause model swap. The per-port Model Display of the panel (css/ModelDisplay.asm) is a second source.  
  Port: `ftRemixModelDisplayDetail` (ft/ftremix/ftremixhitboxdisplay.c:62), applied in `ftManagerMakeFighter` and at the top of `ftParamSetModelPartDetailAll`. No effect on the how-to-play, demo and intro scenes.

- **Special Model Display** (`special_model`; 0 OFF, 1 HITBOX, 2 HITBOX+, 3 ECB; default 0)  
  Asm: Hitbox.asm `hitbox_mode_` and the render patches: 1 draws filled hitboxes and hurtboxes without the model, 2 (HITBOX+) draws the model with outlined (translucent) hurtboxes and hitboxes, 3 draws the model with the ECB. The mode is written every frame to fighters, items and projectiles (reflect and absorb spheres included), also on the CSS, results and 1P intro screens (`fix_hitbox_position_`).  
  Port: `ftRemixHitboxDisplayApply` and friends (ft/ftremix/ftremixhitboxdisplay.c:87-171, ftremixhitbox.c), `itDisplayRemixHitCollisions` (it/itdisplay.c), `wp->display_mode` in wp/wpdisplay.c. Training cycles the value with D-pad Down (`remix_toggle_set`, sc/sc1pmode/sc1ptrainingremix.c:84). The CSS, 1P intro and results graphics buffers are enlarged for the hitbox lists (mn/mnplayers/mnplayersvs.c, mnplayers1pgame.c, sc/sc1pmode/sc1pintro.c), and an invisible (star KO) fighter stops drawing at the top of `ftDisplayMainProcDisplay`. Check: `SSB64_REMIX_TOGGLES="Special Model Display=2"` shows the model with outlined hurtboxes and red hitboxes; `=1` the filled boxes without the model; `=3` the ECB. Deviation: the Kirby Cube debug display (`kirby_cube_enabled`, needs the editable 8th tag text `cube`) has no counterpart.

- **Advanced Hurtbox Display** (`advanced_hurtbox`; 0 off, 1 on; default 0)  
  Asm: Hitbox.asm `grab_immunity_and_armor_frames_`: with the hurtbox display on, joints are cyan while reflecting or absorbing, grey during armor frames (Remix Armour command), dark cyan when not grabbable (Giga Bowser in Remix 1P), otherwise yellow.  
  Port: `ftRemixHitboxDisplayNormalHurtboxColor` (ft/ftremix/ftremixhitbox.c:89).

- **Hold To Exit Training** (`hold_to_exit_training`; 0 off, 1 on; default 0/1/0/0)  
  Asm: Training.asm: leaving the Training needs A to be held, with a `(Hold A)` label on the menu.  
  Port: sc/sc1pmode/sc1ptrainingremixexit.c:62 and :95 (label through the `remix_hud_text` overlay).

- **Improved Combo Meter** (`improved_combo_meter`; 0 off, 1 on; default 1/0/1/1)  
  Asm: Combo.asm `improve_meter_`: the combo count is not reset while the victim is in one of 21 listed actions (captured, thrown, stunned, tornado, barrel, egg lay and others) or in knockback hitstun.  
  Port: `ftRemixComboKeepsCount` (ft/ftremix/ftremixcombo.c:76), hook in the `combo_count_foe` reset of `ftMainUpdate` (ft/ftmain.c:1771).

- **Tech Chase Combo Meter** (`tech_chase_combo_meter`; 0 off, 1 on; default 1/0/1/1)  
  Asm: Combo.asm: adds the 5 tech and down-bounce actions to the list of Improved Combo Meter.  
  Port: ft/ftremix/ftremixcombo.c:80.

- **Combo Meter** (`combo_meter`; 0 off, 1 on; default 1/0/1/1)  
  Asm: ComboMeter.asm: shows a COMBO meter with hit count and damage over each fighter in VS and 1P battles (colour by port or team, silver for an unattributed hit, fades after about 30 frames, INCREDIBLE sound at 20 hits, Pokemon announcer clip at 10 hits).  
  Port: if/ifremixcombometer.c:431 and :455 (setup in `syTaskmanLoadScene`, `ifRemixComboMeterHideForScore` in `ifCommonPlayerScoreMakeEffect`), hit state in if/ifremixcombohits.c, textures from Remix file 0x9A6. The max-combo stats of the VS stats pages read the same toggle (mn/mnvsmode/mnremixvsstatspages.c:385, :456).

- **1v1 Combo Meter Swap** (`1v1_combo_meter_swap`; 0 off, 1 on; default 0)  
  Asm: ComboMeter.asm `_swap_for_singles`: in a 1v1 each meter sits where the other player's meter would be.  
  Port: if/ifremixcombometer.c:439.

- **Neutral Spawns** (`neutral_spawns`; 0 off, 1 on; default 1)  
  Asm: Spawn.asm `load_spawn_`: VS matches start from `Spawn.neutral_table` slots (valid setups for singles and teams; no valid setup keeps the vanilla spawn; Smashketball is always neutral).  
  Port: `gmRemixNeutralSpawnApply` (gm/gmremixspawnsetup.c:221) through `port_comp_ruleset_get_spawn` (sc/sccommon/scvsbattle.c). Stage row from `gSCManagerBattleState->gkind`. The neutral rows of every stage id (0x00-0xD7) are read from the Remix image (`REMIX_SYM_Spawn__neutral_table`). Open: the asm also uses `Spawn.original_table` for the non-neutral spawn of every stage; the port keeps the stage's own spawn points there. The Training branch is separate (sc/sc1pmode/sc1ptrainingmode.c).

- **Salty Runback** (`salty_runback`; 0 OFF, 1 A+B+Z+R+START, 2 A+B+Z+R+Dpad-Left; default 1/0/1/1)  
  Asm: GameEnd.asm `update_screen_`: holding the combo (A+B+Z+R+START, or A+B+Z+R+D-Pad Left) at the end of a VS match restarts it at once on the same stage without results (Great Bay plays FGM 0x566). R or L held shows the results instead.  
  Port: `gmRemixGameEndSelectScene` and `gmRemixGameEndStartSaltyRunback` (gm/gmremixgameend.c:57); the runback flag is the C static read through `gmRemixGameEndIsSaltyRunback`.

- **Widescreen** (`widescreen`; 0 off, 1 on; default 0)  
  Asm: Widescreen.asm: sets the 16:9 viewport aspect (0x3FE8BA2F) in matches and corrects the menu aspect on other screens.  
  Port: No reader, and the Remix menu page leaves it out (`IsRemixToggleWithoutPortEffect`, port/gui/PortMenu.cpp). The port renders at the window aspect with its own widescreen handling (`port_widescreen_clip_x_scale`, Settings menu).

- **Stereo Fix for Hit SFX** (`stereo_fix`; 0 off, 1 on; default 1)  
  Asm: Stereo.asm `fix_sfx_pan_`: inverts the hit-SFX pan (a vanilla bug) when ON; Sector Z Remix and the Home-Run Contest platform are always centred.  
  Port: No reader, and the Remix menu page leaves it out (`IsRemixToggleWithoutPortEffect`). Deviation: the port always applies the corrected pan (lb/lbcommon.c:760-775, with the Sector Z Remix centring), because the inverted pan is an outright vanilla bug.

- **Random Select** (`variant_random`; 0 DEFAULT, 1 INCLUDE BONUS, 2 ALL VARIANTS, 3 VANILLA ONLY; default 0)  
  Asm: CharacterSelect.asm `get_random_char_id_`: Random Select pool. INCLUDE BONUS adds the ten bonus fighters, ALL VARIANTS turns a pick into one of its four variants one time in five, VANILLA ONLY rerolls non-vanilla fighters.  
  Port: `mnRemixRandomPickFighterKind` and `mnRemixRandomResolveFighterKind` (mn/mnplayers/mnremixvariants.c:124, :144).

- **Disable HUD** (`disable_hud`; 0 OFF, 1 PAUSE, 2 ALL; default 0)  
  Asm: Camera.asm `disable_hud_`/`unpause_camera_`, Training.asm and Pause.asm `disable_hud_`: ALL (bit 2) hides the match HUD after GO, after unpausing and in Training (the Training menu stays reachable); PAUSE and ALL hide the pause legends (Render groups 0xE-0x10). With the setting OFF, D-pad Left on the pause toggles the legends.  
  Port: `ifRemixHudDisabledAll` (if/ifremixhud.c:16), gm/gmremixcamerapause.c, sc/sc1pmode/sc1ptrainingremixlegend.c:88, if/ifremixpauselegend.c:150-213 (`ifRemixPauseLegend*`).

- **Disable Anti-Aliasing** (`disable_aa`; 0 off, 1 on; default 0)  
  Asm: AA.asm: clears the VI anti-alias bits.  
  Port: Adapted. `MsaaLevelForToggles` (port/remix/remix_display_settings.cpp:28) forces MSAA level 1 while the toggle is on and restores the Anti-aliasing (MSAA) CVar when it is off. Applied every frame from `PortPushFrame` (port/gameloop.cpp:689).

- **Always Show Full Results** (`full_results`; 0 off, 1 on; default 1)  
  Asm: TimedStock.asm `show_full_results_`: stock matches use the KO/TKO layouts of the time modes on the results screen (stock royal to time royal, stock team to time team).  
  Port: `gmRemixGameEndResultsDrawKind` (gm/gmremixgameend.c:148) in `mnVSResultsMakeLabel`.

- **Skip Start Cheer** (`skip_start_cheer`; 0 OFF, 1 TRAINING, 2 TRAINING + BONUS; default 0)  
  Asm: Training.asm `skip_crowd_cheer_`: skips the crowd cheer FGM 0x272 at the start of Training (1) or Training and Bonus/1P (2).  
  Port: sc/sc1pmode/sc1ptrainingmode.c:2317 (Training) and `sc1PRemixIsBonusStartCheerSkipped` (sc/sc1pmode/sc1premixmode.c:59).

- **Default CPU LVL (V.S.)** (`default_cpu_level`; 0 DEFAULT, 1..10 level; default 0/10/0/0)  
  Asm: AI.asm `force_default_cpu_lvl_setup`: on the first VS setup after boot the four slots start at this CPU level (0 means level 3). Toggles.asm `set_cpu_level` refreshes already-set levels when A is pressed.  
  Port: `gmRemixDefaultCPULevelSetup` (gm/gmremixspawnsetup.c:255), once per boot from `mnPlayersVSInitVars`. Deviation: the A-press refresh does not exist and the entry is not in the ImGui page (it carries an A function), so set it before launch.

- **Jigglypuff Sing GFX Anims** (`puff_sing_anim`; 0 off, 1 on; default 0)  
  Asm: Size.asm: ON keeps the animated scale of Jigglypuff's sing effect; OFF forces scale 1.0.  
  Port: `efRemixPurinSingProcUpdate` (ef/efremixpuffsing.c, the sing effect's update): the second joint uses matrix kind 0x2E every frame and, unless the toggle is on, its scale is reset to 1.0 (IT-9).

- **'L' Selects Random Character** (`l_random_char`; 0 off, 1 on; default 0)  
  Asm: CharacterSelect.asm:6569: L selects a random character with the token.  
  Port: mn/mnplayers/mnplayersvs.c:5388 (VS CSS).

- **Dpad CSS Cursor Control** (`dpad_css_control`; 0 off, 1 on; default 0)  
  Asm: css/DpadControl.asm `css_cursor_and_dpad_controls_`: the D-pad moves the CSS cursor.  
  Port: `ftRemixDpadMenu` screen state (ft/ftremix/ftremixdpadmenu.c:105): on a CSS with the Player Panel D-pad option off, the toggle turns on the D-pad cursor. The match-side D-pad options live in the Player Panel (ft/ftremix/ftremixdpad*.c).

- **PK Thunder Reflect Crash Fix** (`pk_thunder_reflect_crash_fix`; 0 off, 1 on; default 1)  
  Asm: nessshared.asm: marks the PK Thunder trail objects as discarded when the head is reflected and fixes the trail rotation.  
  Port: `gmRemixToggle(pk_thunder_reflect_crash_fix)` in `wpNessPKReflectHeadProcReflector` (wp/wpness/wpnesspkthunder.c): the reflected head destroys its trails and the reflected trail rotation is `atan2(vel.y, vel.x) - 90°` (SH-5).

- **Flash Guard** (`flash_guard`; 0 off, 1 on; default 0)  
  Asm: Accessibility.asm `flash_guard` and the Flashbang, Deku Nut, Lightning and Special Zoom checks: no white screen flashes.  
  Port: if/ifscreenflash.c:71, `ITREMIX_FLASH_GUARD_ON` (it/itremix/itremixscreenflash.h:11), gm/gmremixspecialzoom.c:178.

- **Screenshake** (`screenshake`; 0 DEFAULT, 1 LIGHT, 2 OFF; default 0)  
  Asm: Accessibility.asm `screenshake_toggle`: LIGHT forces the lightest quake severity, OFF removes quakes. On Remix RTTF a quake created by an off-screen CPU's effect does not shake either (`ParamMakeEffect` store).  
  Port: `efRemixQuakeSeverity` (ef/efremixscreenshake.c:20) called from `efManagerQuakeMakeEffect`; RTTF branch in gm/gmremixaccessibility.c:25 fed by ft/ftparam.c:2159-2172.

- **BlastZone GFX** (`blastzone_gfx`; 0 DEFAULT, 1 OFF, 2 REDUCED; default 0)  
  Asm: Accessibility.asm `blastzone_explode_gfx`: OFF skips the blast-zone explosion, REDUCED scales the effect to 0.64 and the star particles to 0.75.  
  Port: `efRemixBlastZoneGfxIsOff`, `efRemixBlastZoneScaleEffect`, `efRemixBlastZoneScaleParticle` (ef/efremixblastzone.c:23-40) in `efManagerDeadExplodeMakeEffect`.

## 3. Gameplay Settings page

- **Hitstun** (`hitstun`; 0 NORMAL, 1 MELEE; default 0)  
  Asm: Hitstun.asm: MELEE divides knockback by 2.5 instead of 1.875 (hitstun is 0.75x). Single-player guard.  
  Port: `ftParamGetHitStun` (ft/ftparam.c:1793), `gmRemixToggleVS(hitstun)`.

- **Hitlag** (`hitlag`; 0 NORMAL, 1 JAPANESE, 2 MELEE, 3 ULTIMATE, 4 NONE; default 0/0/0/1)  
  Asm: Japan.asm `hitlag_`: hitlag = damage * k + c with JAPANESE (1/3, 4), MELEE (1/3, 3), ULTIMATE (0.65234375, 6), NONE (0, 0); NORMAL is the vanilla 1/3, 5. Single-player guard.  
  Port: `ftRemixHitlagTics` (ft/ftremix/ftremixhitlag.c:33) in `ftParamGetHitLag`.

- **DI** (`di`; 0 NORMAL, 1 JAPANESE, 2 ULTIMATE; default 0/0/0/1)  
  Asm: DI.asm `apply_di_modifiers`: smash DI range multiplier 1.5 (JAPANESE) or 0.59765625 (ULTIMATE; weaker, 'not accurate' per the asm) instead of the vanilla 2.1. Single-player guard.  
  Port: `ftRemixDamageSmashDIRangeMul` (ft/ftremix/ftremixdi.c:29) in `ftCommonDamageCommonProcLagUpdate`.

- **Japanese Sounds** (`japanese_sounds`; 0 DEFAULT, 1 ALWAYS, 2 NEVER; default 0/0/0/1)  
  Asm: Japan.asm `japanese_sfx_*`: swaps hit, beam sword and projectile hit sounds to the Japanese ids. DEFAULT only for fighters of sound type J, ALWAYS for all, NEVER for none. No guard.  
  Port: `ftRemixJapaneseMotionSoundFgm`, `ftRemixJapaneseProjectileSoundFgm`, item/weapon sound conversion (ft/ftremix/ftremixjapanesesounds.c:52; hooks ft/ftmain.c:417, :2743, wp/wpmanager.c:245, it/itmanager.c:371). Sound type from `kRemixRoster[fkind].sound_type`, not from the RAM table.

- **Japanese Stun/Sleep** (`j_stun_sleep`; 0 off, 1 on; default 0/0/0/1)  
  Asm: Japan.asm `japanese_stun_mash`/`japanese_sleep_mash`: Fura-Fura breakout wait minus 30, Fura-Sleep minus 35 (60 and 40 frames of base wait instead of 90 and 75). Single-player guard.  
  Port: `ftRemixFuraFuraBreakoutWait`, `ftRemixFuraSleepBreakoutWait` (ft/ftremix/ftremixjstunsleep.c:25, :38).

- **Momentum Slide** (`momentum_slide`; 0 off, 1 on; default 0/0/0/1)  
  Asm: Japan.asm `momentum_slide_`: skips the US clamp of run-brake velocity (the Japanese momentum slide). Single-player guard.  
  Port: Inline in `ftCommonRunBrakeCheckInterruptTurnRun` (ft/ftcommon/ftcommonrunbrake.c:69).

- **Shield Stun** (`shieldstun`; 0 DEFAULT, 1 JAPANESE, 2 MELEE, 3 BRAWL, 4 ULTIMATE; default 0/0/0/1)  
  Asm: Japan.asm `japanese_shieldstun_`: shield stun = shield damage * k + c with JAPANESE (1.75, 3), MELEE (0.451171875, 1.9921875), BRAWL (0.345703125, 0), ULTIMATE (0.7265625, 2). Single-player guard.  
  Port: `ftRemixShieldStunFrames` (ft/ftremix/ftremixshieldstun.c:40) in `ftCommonGuardSetOffSetStatus`.

- **Z-Cancel** (`z_cancel_opts`; 0 DEFAULT, 1 DISABLED, 2 MELEE (7 frames), 3 AUTO, 4 GLIDE MODE; default 0)  
  Asm: ZCancel.asm `_z_cancel_opts`: window of 11 frames (DEFAULT), never (DISABLED), 7 frames (MELEE), always (AUTO), or a missed Z-cancel keeps gliding without landing lag (GLIDE MODE).  
  Port: `ftRemixZCancelResult` and `ftRemixZCancelAttackAirLanding` (ft/ftremix/ftremixzcancel.c:220, :303), replacing the landing branch of `ftCommonAttackAirProcMap`. The port's Auto Z-Cancel enhancement still turns every miss into a normal landing.

- **Punish Failed Z-Cancel** (`punish_on_failed_z_cancel`; 0 OFF, 1 7% Damage, 2 Lava Floor, 3 Shield-Break, 4 Instant K.O., 5 Force Taunt, 6 Bury, 7 Laugh Track, 8 Egg, 9 Sleep, 10 Trip, 11 Random; default 0)  
  Asm: ZCancel.asm `_cruel_z_cancel`: 11 punishments on a failed Z-cancel (7% damage, lava floor, shield break, instant KO, forced taunt, bury, laugh track, egg, sleep, trip) plus RANDOM (rolls 1..10). Instant KO is skipped in the 1P final stage.  
  Port: `ftRemixZCancelPunish` and `ftRemixZCancelPunishMode` (ftremixzcancel.c:112, :204); TRIP also uses ft/ftremix/ftremixtripping.c:140. RANDOM uses `syUtilsRandIntRange`, not the Remix RNG, so the sequence differs. The entry is not in the ImGui page (A function).

- **Improved AI** (`improved_ai`; 0 off, 1 on; default 1/0/1/1)  
  Asm: AI.asm, nessshared.asm and the Improved AI hooks: better CPU teching, DI, recovery and combat decisions. Toggles.guard.  
  Port: ft/ftremix/ftremixaicombat.c:24, ftremixaidi.c:44, ftremixaipostprocess.c:26, ftremixairecovery.c:38-58, ftremixaiteching.c:141, :186.

- **Tripping** (`tripping`; 0 OFF, 1 LOW, 2 HIGH, 3 BRAWL; default 0)  
  Asm: Tripping.asm: dash, dash-dance and turn-run can trip (LOW about 1.2%/1.0%/1.5% per roll, HIGH divides the range by 4, BRAWL by 16). A trip enters DamageLw3 with FGM 0x456 and 8 hitstun frames, then DownBounceD on the ground or Tumble in the air. A failed Z-cancel punished with TRIP or RANDOM trips even with Tripping OFF.  
  Port: `ftRemixTrippingEnterStatus` and `ftRemixTrippingFinishDamage` (ft/ftremix/ftremixtripping.c:92-160), hooks in ftcommonturn.c, ftcommondash.c, ftcommonturnrun.c, ftcommondamage.c.

- **Rage** (`rage`; 0 OFF, 1 ULTIMATE, 2 SMASH 4, 3 BERSERK, 4 FATIGUE; default 0)  
  Asm: Rage.asm: the knockback taken is multiplied by 1 + k * (percent - 35) / 115, with the attacker's percent clamped to [35, 150]: k = 0.1 for ULTIMATE (applies only to moves with knockback weight 0, kept literal from the asm), 0.15 for SMASH 4, +0.5 for BERSERK, -0.5 for FATIGUE. Fighters at 100% or more also emit smoke, faster at 125% and 150%.  
  Port: `ftRemixRageKnockbackScale`, `ftRemixRageSaveAttackerPercent`, `ftRemixRageSmokeEffect` (ft/ftremix/ftremixrage.c:96-150); hooks ft/ftmain.c:2127, :3083, :3162, ft/ftparam.c:1740, ft/ftcommon/ftcommonthrown2.c:201. The attacker percent is one global, as in the asm.

- **Footstool Jumping** (`footstool`; 0 off, 1 on; default 0)  
  Asm: FootStool.asm: a mid-air jump press over another fighter footstools it (victim to Tumble or LandingHeavy at fall speed -25, FGM 0x4B0, jumper gets a JumpSquat); jumps are not reset when leaving the ground. No guard.  
  Port: `ftRemixFootStoolTryAerialJump` and helpers (ft/ftremix/ftremixfootstool.c:278), hooks in ft/ftcommon/ftcommonjumpaerial.c:373-436 and mp/mpcommon.c:925. The asm team check reads the jumper's GObj byte 0x0C, ported literally.

- **Air Dodging** (`air_dodge`; 0 OFF, 1 Melee, 2 Ultimate, 3 Air Dash; default 0)  
  Asm: AirDodge.asm: Melee (directional air dodge), Ultimate (neutral dodge that drifts) and Air Dash (one dash per air time) replace the aerial shield press.  
  Port: ft/ftremix/ftremixairdodge.c:183-260, hooks in ftcommonjumpaerial.c:348, mp/mpcommon.c:452, ft/ftmanager.c:782. Deviation: the turnaround call that the asm makes with the wrong argument is not reproduced.

- **Jab Locking** (`jab_lock`; 0 off, 1 on; default 0)  
  Asm: JabLock.asm: weak hits during a DownBounce restart the bounce instead of damaging; after the bounce the next hit ends it in DownStand.  
  Port: ft/ftremix/ftremixjablock.c:19-55, hooks in ftcommondownwaitbounce.c:160, ftcommondamage.c:654, ftcommondownattack.c:26.

- **Edge C-Jumping** (`ledge_jump`; 0 off, 1 on; default 0)  
  Asm: LedgeJump.asm: a C-button tap while hanging climbs (quick or slow by damage) and the climb ends in a stick jump.  
  Port: ft/ftremix/ftremixledgejump.c:28, :49 (cliff wait and climb procs in ft/ftcommon/ftcommoncliffcatchwait.c, ftcommoncliffclimb.c).

- **Perfect Shielding** (`perfect_shield`; 0 off, 1 on; default 0)  
  Asm: PerfectShield.asm: a hit on the first frames of the shield (status tics < 2) causes no shield stun, a swirl effect, and reflects projectiles and items back to the shielder (reflect multiplier 0.5).  
  Port: ft/ftremix/ftremixperfectshield.c:112-158, hooks ft/ftmain.c:4413, wp/wpprocess.c:507, it/itprocess.c:1111.

- **Parrying (BETA)** (`parry`; 0 off, 1 on; default 0)  
  Asm: Parry.asm (BETA): releasing the shield within 5 frames of raising it while a hitbox reaches the shield parries (ClangRecoil, intangible hurtbox, sound 0x5BB, attacker hitlag +14, no shield damage); works for direct hits, projectiles and items.  
  Port: ft/ftremix/ftremixparry.c:75, :319; hooks ft/ftmain.c:3593, :3768, :3983, :4486. Kept from the asm on purpose: the previous hitbox position is always used for direct and projectile hits, and a multi-hitbox projectile can be parried by a hitbox outside the shield range.

- **Spot Dodging** (`spot_dodge`; 0 off, 1 on; default 0)  
  Asm: SpotDodge.asm: down-slam of the stick while holding Z on the ground is a spot dodge (DamageHigh2 with intangible moveset); on a soft platform above -52 it is a platform drop.  
  Port: `ftRemixSpotDodgeCheckInterrupt` and `ftRemixSpotDodgePassInput` (ft/ftremix/ftremixspotdodge.c:89, :109), hook in ft/ftcommon/ftcommonpass.c.

- **Fast Fall Aerials** (`fast_fall_aerials`; 0 off, 1 on; default 0)  
  Asm: AerialAttackFastFall.asm: fast fall is allowed while an aerial attack is active.  
  Port: ft/ftremix/ftremixaerialfastfall.c:22-44, hook ft/ftmain.c:5619.

- **Ledge Trumping** (`ledge_trump`; 0 off, 1 on; default 0)  
  Asm: LedgeTrump.asm: a fighter that jumps to an occupied ledge knocks the holder away (DamageAir3, velocity 50, 30 hitstun frames) and grabs it.  
  Port: `ftRemixLedgeTrumpSkipsHolder` (ft/ftremix/ftremixledgetrump.c:41) in `mpCommonRunFighterSpecialCollisions` (mp/mpcommon.c:577); the ledge-grab counter hook is mp/mpcommon.c:591.

- **Wall Teching** (`wall_teching`; 0 off, 1 on; default 0)  
  Asm: WallTeching.asm: Z pressed within 20 frames before a wall hit techs (Passive status, no wall bounce).  
  Port: `ftRemixWallTechTryStart` (ft/ftremix/ftremixwalltech.c:34), hook in ft/ftcommon/ftcommonwalldamage.c:63; the tech counters hook ftcommonpassive.c:19 and ftcommonpassivestand.c:19.

- **Charge Smashes** (`charged_smashes`; 0 OFF, 1 ON, 2 UNLIMITED CHARGE; default 0)  
  Asm: ChargeSmashAttacks.asm: holding A on the charge frame of a smash (or an item swing) freezes the animation with a shake, a dust puff every 16 frames and sound 0x528; ON releases at 60 frames, UNLIMITED never.  
  Port: `ftRemixChargeSmashLogic` (ft/ftremix/ftremixchargesmash.c:336-433) called at the end of `ftMainSetStatus` (ft/ftmain.c:5640); hold in `ftAnimEndSetWait`; shuffle in ft/ftdisplaymain.c:1398 and lb/lbcommon.c:1913. The hold is gated on the toggle (the asm takes the charging branch whenever the animation speed is 0).

- **Item Containers** (`item_containers`; 0 DEFAULT, 1 OFF, 2 NEVER EXPLODE, 3 ALWAYS EXPLODE; default 0)  
  Asm: Item.asm `item_containers_*`: OFF spawns no crates, barrels or capsules (eggs spawn except on Yoshi's Island variants), NEVER EXPLODE and ALWAYS EXPLODE force the explosion outcome of hit and thrown containers.  
  Port: `itRemixContainersApplyToggle`, `itRemixContainerKeeps`, `itRemixContainerSmashFgm` (it/itremix/itremixcontainers.c:83-112), hooks in itmanager.c, itbox.c, itcapsule.c, ittaru.c, itegg.c.

- **Game Speed** (`game_speed`; 0 1/1, 1 1.2x, 2 1.3x, 3 1.5x, 4 1.75x, 5 2.0x, 6 3.0x, 7 1/8, 8 1/4, 9 1/3, 10 1/2, 11 2/3, 12 3/4; default 0)  
  Asm: Speed.asm `handle_rate_`: runs more or fewer `gcRunAll` frames per game frame. 2.0x and 3.0x run 2 and 3, the other values follow `waiting_time_table` (active frames out of total, for example 1.2x = 5 of 6, 1/8 = 1 of 8). The Stopwatch backfire speed replaces the toggle while active.  
  Port: `ftRemixSpeedRunFrames` and `ftRemixSpeedTrainingRunFrames` (ft/ftremix/ftremixspeed.c:56), hooks if/ifcommon.c:3412 and sc/sc1pmode/sc1ptrainingmode.c:791; poison damage skips the extra frames (ft/ftremix/ftremixpoison.c:202). Remix's own Training freeze (D-pad up/right) is not ported.

- **Special Zoom (BETA)** (`special_zoom`; 0 OFF, 1 MATCH END, 2 ANY KO; default 0)  
  Asm: SpecialZoom.asm (BETA): the camera zooms on the victim of a predicted KO (MATCH END: only on the decisive stock of a two-team match; ANY KO: every KO) and on listed special moves, with slow motion (one object update every 16 frames for 40 updates), a wallpaper behind the fighters and FGMs 152 and 187. Not used in Stamina battles.  
  Port: gm/gmremixspecialzoom.c:196-281 (+ gmremixspecialzoomko.c, ...attack.c, ...wallpaper.c); hooks `ftCommonDamageInitDamageVars`, `gmCameraUpdatePlayerZoom`, `gmCameraMakeWallpaperCamera`, `gcRunAll`. Stage groups 1 and 2 are hidden with `gmRemixGroupDisplayTempSet`; Stamina battles are skipped (`gmRemixStaminaIsActive`).

- **BlastZone Warp (BETA)** (`blastzone_warp`; 0 OFF, 1 LEFT/RIGHT, 2 TOP/BOTTOM, 3 ALL; default 0)  
  Asm: Blastzone.asm (BETA): a fighter leaving the left/right (bit 1) or top/bottom (bit 2) blast zone reappears on the opposite edge, with no KO; stuck actions (ground-pound drop, Bowser bomb, Kirby throw fall, shield-break fall) release or fall after 250 tics.  
  Port: `ftRemixBlastZoneWarpFrom{Right,Left,Top,Bottom}` (ft/ftremix/ftremixblastzone.c:41-47) in `ftCommonDeadCheckInterruptCommon`; bit 2 also gates the bottom blast SFX skip (gr/grremix/grremixblastzone.c:11).

- **Single Button Mode** (`single_button_mode`; 0 OFF, 1 A, 2 B, 3 R, 4 "A"+"C", 5 "B"+"C", 6 "R"+"C"; default 0)  
  Asm: SingleButtonMode.asm: a human fighter only registers the chosen button (A, B or R, with C for jumping in modes 4-6) plus START and the D-pad; the CPU's A/B and Z/L presses are filtered the same way.  
  Port: `ftRemixSingleButtonModeFilter` (ft/ftremix/ftremixsinglebutton.c:101) in `ftMainProcessInput`, CPU hooks in ft/ftcomputer.c:3482-3518; also read by ftremixairdodge.c:260, ftremixaiteching.c:40, ftremixdpadmap.c:75, ftremixtauntbutton.c:36.

- **All Items R Drop (Aerial)** (`item_dropping`; 0 off, 1 on; default 0)  
  Asm: Item.asm `item_drop`: R in the air drops any held item instead of throwing it.  
  Port: `ftRemixAerialItemIsThrowType` (ft/ftremix/ftremixitemdrop.c:21) in `ftCommonAttackAirCheckInterruptCommon`.

- **Move Staling** (`move_staling`; 0 DEFAULT, 1 DISABLED, 2 LENIENT, 3 STRICT, 4 WAIT FOR IT, 5 REVERSE, 6 CHEAP SHOT; default 0)  
  Asm: StaleMoves.asm: replaces the stale-move multiplier. DISABLED 1.00x; LENIENT 0.75x on the first repeat; STRICT uses a harsher table (the repeat of a move deals about 1 damage); WAIT FOR IT 0.75x, 0.82x, 0.89x, then 1.75x; REVERSE 0.75x rising to 1.00x; CHEAP SHOT 1.00x rising to 1.75x. The walk counts every matching entry of the 4-entry queue, unlike the vanilla loop.  
  Port: `ftRemixStaleMovesGetMultiplier` and `ftRemixStaleMovesTable` (ft/ftremix/ftremixstalemoves.c:39-108) inside `ftParamGetStale`; the preserved value is a per-port static that starts at -1.

- **Stopwatch Item** (`stopwatch_behaviour`; 0 DEFAULT, 1 SLOW ALWAYS, 2 FAST ALWAYS, 3 BACKFIRE ALWAYS, 4 BACKFIRE NEVER; default 0)  
  Asm: items/Stopwatch.asm `pickup_stopwatch`, `clock_out_`: forces the outcome of the pickup (SLOW or FAST ALWAYS) or of the backfire roll (ALWAYS or NEVER; DEFAULT backfires 10% of the time).  
  Port: `itRemixStopwatchGetBehaviour` (it/itremix/itremixstopwatch.c:175).

## 4. Music Settings page

- **Play Music** (`play_music`; 0 off, 1 on; default 1)  
  Asm: BGM.asm `play_music_`: OFF stops the stage BGM at the start of every track.  
  Port: `syAudioPlayBGM` (sys/audio.c:1661) stops all BGM and returns -1; also read by gm/gmremixbgm.c:75 and gm/gmremixmusictitle.c:60.

- **Random Music** (`random_music`; 0 off, 1 on; default 0/0/1/0)  
  Asm: BGM.asm `random_music_`: picks the stage BGM uniformly from the tracks whose Random Music toggle is on (empty list keeps the stage song). Holding a C button on the stage load also selects the alternate songs and blocks the random pick.  
  Port: `grRemixRandomMusicPick` (gr/grremix/grremixrandommusic.c:93) from `grRemixGetStageBGM` (gr/grremix/grremixstage.c:256), called by `mpCollisionInitGroundData` in VS only. The C-button override is not read and Training, 1P, Bonus and the VS demo keep the stage song.

- **Salty Runback Preserves Song** (`preserve_salty_song`; 0 off, 1 on; default 0)  
  Asm: BGM.asm: a salty runback replays the previous random pick.  
  Port: `grRemixRandomMusicPickIndex` (grremixrandommusic.c:78).

- **Menu Music** (`menu_music`; 0 DEFAULT, 1 64, 2 MELEE, 3 MELEE MENU 2, 4 BRAWL, 5 GOLDENEYE 007, 6 MARIO TENNIS, 7 SUPER MARIO 64, 8 BLAST CORPS, 9 DIDDY KONG RACING, 10 MARIO KART 64, 11 SNOWBOARD KIDS, 12 MARIO PARTY, 13 MARIO ARTIST, 14 PERFECT DARK, 15 CRASH TEAM RACING, 16 KIRBY 64, 17 BOMBERMAN 64, 18 SONIC R, 19 RANDOM, 20 RANDOM CLASSICS, 21 RANDOM ALL, 22 OFF; default 0/0/1/0)  
  Asm: BGM.asm `alt_menu_music_` and Toggles.asm `play_menu_music_`: replaces the menu BGM. 0 DEFAULT (vanilla; on the title screen 85% 64 theme, 5% each Melee, Melee Menu 2, Brawl), 1 64, 2-18 a fixed track, 19 RANDOM (tracks 1-18), 20 RANDOM CLASSICS (5-18), 21 RANDOM ALL (the Random Music list, or a hidden GameBoy song when empty), 22 OFF. A listed track already playing is not restarted.  
  Port: `gmRemixMenuMusicPlay` and `gmRemixMenuMusicStop` (gm/gmremixmenumusic.c:261-307) at 9 play sites and 4 stop sites of the menu screens. RANDOM ALL does not include the custom MIDI tracks. The entry is not in the ImGui page (A function).

- **Music Title at Match Start** (`show_music_title`; 0 off, 1 on; default 1/0/1/1)  
  Asm: BGM.asm `show_music_title_`: at the GO lights a translucent bar shows the track name and the game name; it disappears with GO!.  
  Port: gm/gmremixmusictitle.c:60 with names from port/remix/remix_music_title.cpp, drawn through the `remix_hud_text` overlay.

- **BGM Volume** (`bgm_volume`; 0..10; default 10)  
  Asm: BGM.asm `master_bgm_volume`: BGM volume is scaled by level/10 before the 30720 clamp.  
  Port: `gmRemixMasterVolume` (gm/gmremixmusic.c:9) in `syAudioSetBGMVolume` (sys/audio.c:1719). Not in the ImGui page (A function).

- **SFX  Volume** (`fgm_volume`; 0..10; default 10)  
  Asm: FGM.asm `master_fgm_volume`: sound-effect volume scaled by level/10.  
  Port: `gmRemixMasterVolume` in `func_80020E64` (sys/audio.c:1846). Not in the ImGui page (A function).

**Load Profile:** (`load_profile_music`, hidden) loads the Music sub-profile (`string_table_music_profile`) for the Random Music rows.

**Random Music toggles** (`random_music_<track>`, bool, default 1 in every profile). One toggle per track of the Random Music list. Built-in (16): Bonus, Congo Jungle, Credits, Data, Dream Land, Duel Zone, Final Destination, How To Play, Hyrule Castle, Meta Crystal, Mushroom Kingdom, Peach's Castle, Planet Zebes, Saffron City, Sector Z, Yoshi's Island. The 277 custom MIDI tracks follow, sorted by title (`entry_random_music_sorted_<id>`, 328 labels in `remix_symbols.h`, one per MIDI whether toggleable or not). The asm `A` press previews the track (`preview_bgm_`) and the title row `Random Music Toggles:` flips all rows.
Port: only the 16 built-in rows are read (`sRandomTracks`, `gr/grremix/grremixrandommusic.c:22-41`, in the asm list order which fixes the index a salty runback replays); the BGM ids come from `BGM.asm`. The custom rows are registered and storable but the list never includes them, because their ids are assembly-time values with no symbol; Menu Music RANDOM ALL has the same limit.

Stage BGM path: `grRemixGetStageBGM` picks the alternate song (chances 65/16/10/9 from `Stages.default_music_track_table` and the alternate tables), then `grRemixRandomMusicPick`. The pause D-pad song cycle (`Pause.asm set_alternate_music_`) is `gm/gmremixbgm.c` with the rules in `gm/gmremixbgmrules.c`; the pause music legend reads the title from `remix_music_title`. The 30-second Hurry swap, the FD intro fix and the C-button stage-load override are partial or differ (g9: `BGM.asm:65`, `BGM.asm:1252`, `FD.asm:59`).

## 5. Stage Settings page

- **Stage Select Layout** (`sss_layout`; 0 NORMAL, 1 TOURNAMENT; default 0/1/0/0)  
  Asm: Stages.asm: TOURNAMENT switches the stage select to the tournament layout: the random cell only picks tournament-legal stages and the hazard mode of the chosen stage is set from `Stages.tournament_hazard_mode_table`.  
  Port: `mnRemixTournamentLatchLayout` (mn/mnmaps/mnremixstagetournament.c:24) and the stage-select panel (mnremixstagepanel.c). Defaults: TOURNAMENT only in the Tournament profile.

- **Hazard Mode** (`hazard_mode`; 0 NORMAL, 1 HAZARDS OFF, 2 MOVEMENT OFF, 3 ALL OFF; default 0)  
  Asm: Surface.asm, Hazards.asm, Stages.asm: bit 0 turns stage hazards off, bit 1 stage movement. VS and Training only. L on the stage select cycles it.  
  Port: `grRemixHazardsAreOn`, `grRemixMovementIsOn` (gr/grremix/grremixhazards.c:24); stage panel L toggle `mnRemixStagePanelToggleHazards` writes the toggle with `remix_toggle_set` (mn/mnmaps/mnremixstagepanel.c:251); also gm/gmremixcameramode.c:126, ft/ftremix/ftremixbury.c:67. The Congo Jungle barrel ignores the setting in the port.

- **Whispy Mode** (`whispy_mode`; 0 NORMAL, 1 JAPANESE, 2 SUPER, 3 HYPER; default 0/0/0/1)  
  Asm: Hazards.asm: Dream Land's Whispy wind: JAPANESE (slower decay), SUPER and HYPER (stronger wind, bigger blow box). HYPER is not applied in the 1P battle.  
  Port: gr/grremix/grremixdreamland.c:33; the stage panel shows the value (mnremixstagepanel.c:227).

- **Saffron Pokemon Rate** (`saffron_poke_rate`; 0 NORMAL, 1 SUPER, 2 HYPER, 3 QUICK ATTACK; default 0)  
  Asm: Hazards.asm `saffron_city_pokemon_rate*`: SUPER halves the Pokemon wait, HYPER also opens the door at once, QUICK ATTACK uses 15-tic and 1-tic waits. VS and Training only.  
  Port: `grremixsaffron.c` (the rate on the VS and Training screens, the default elsewhere) sets the Saffron gate and Pokemon timers (patch_audit ST-13); the stage-select panel shows the value (mn/mnmaps/mnremixstagepanel.c:228).

- **Pokemon Announcer** (`announcer_mode`; 0 STADIUM, 1 ALL STAGES, 2 OFF; default 0/2/0/2)  
  Asm: PokemonAnnouncer.asm `toggle_announcer_`: STADIUM speaks on Pokemon Stadium, Pokemon Stadium 2 and Gym Leader Castle, ALL STAGES everywhere, OFF never. See docs/remix/announcer.md.  
  Port: `ifRemixAnnouncerIsActive` (if/ifremixannouncer.c:374).

- **Dragon King HUD** (`dragon_king_hud`; 0 DRAGON KING, 1 ALL STAGES, 2 OFF; default 0/2/0/0)  
  Asm: DragonKingHUD.asm and the HUD placement of ComboMeter, KotH, Multi-Man, All-Star rest area and Bonus: DRAGON KING (default) and ALL STAGES use the lower Dragon King HUD layout, OFF never.  
  Port: Read for the lowered combo meter, KotH HUD and the 1P placements (if/ifremixcombometer.c:99, gm/gmremixkothhud.c:73, sc/sc1pmode/sc1pbonusstage.c:1170, sc1premixallstarrest.c:33, sc1premixmultiman.c:79). The Dragon King damage number set (file 0xA4 digits, Dragon King colours, y 38), the lowered timer (y 212), the hidden series logo, the instant KO digit fall and the 1P, bonus, Training, How to Play and VS demo placements are in `if/ifremixdragonkinghud.c` (`ifRemixDragonKingHudIsOn`, `ifRemixDragonKingHudPickY`).

- **Camera Mode** (`camera_mode`; 0 NORMAL, 1 BONUS, 2 FIXED, 3 SCENE; default 0)  
  Asm: Camera.asm: BONUS routes default-camera stages to the Mushroom Kingdom camera and keeps the Bonus pause zoom, FIXED freezes the camera on a per-stage fixed view and disables pause analog control, SCENE uses the anim camera, hides the HUD at unpause and keeps the pause camera eye.  
  Port: gm/gmremixcameramode.c:120 (`gmRemixCameraSetupDefault`, frozen/fixed views, pause control), gm/gmremixcamerapause.c, `ftRemixPauseKeepsSceneCamera` (ft/ftremix/ftremixpause.c:49), gm/gmremixspecialzoom.c:196. The stage ids use `gSCManagerBattleState->gkind`.

- **Yoshi's Island Cloud Anims** (`yi_clouds`; 0 off, 1 on; default 0)  
  Asm: Hazards.asm `yoshis_island_cloud_anims_`: ON animates the Yoshi's Island clouds (scale) and disables their clipping.  
  Port: gr/grremix/grremixyoshiclouds.c:26 (hook in gr/grcommon/gryoster.c:241).

**Load Profile:** (`load_profile_stage`, hidden; Community, Tournament, Semi-Competitive; default 0/1/2/0) loads the Random Stage sub-profile.

**Random Stage toggles** (`random_stage_<stage>`, bool). They choose the stages of the RANDOM cell of the stage select and of Stage Select OFF. The 16 built-in ones (CE/TE/NE/JP defaults):

| Stage | Default |
|---|---|
| Congo Jungle | 1/0/1/1 |
| Dream Land | 1 |
| Dream Land Beta 1 | 0 |
| Dream Land Beta 2 | 0 |
| Duel Zone | 1/0/0/1 |
| Final Destination | 1/0/0/1 |
| How to Play | 0 |
| Hyrule Castle | 1/0/1/1 |
| Meta Crystal | 1/0/1/1 |
| Mushroom Kingdom | 1/0/0/1 |
| Peach's Castle | 1/0/1/1 |
| Planet Zebes | 1/0/0/1 |
| Saffron City | 1/0/1/1 |
| Sector Z | 1/0/0/1 |
| Yoshi's Island | 1/0/0/1 |
| Mini Yoshi's Island | 1/0/1/1 |

The 128 custom stages have a toggle each (CE and JP default 1, TE and NE from `Stages.STAGE_<id>_TE/NE`). Port: `mnRemixStageRandomPick` and the entry list of `mn/mnmaps/mnremixstagerandom.c` (the 16 vanilla stages then `mnremixstagerandomtable.inc`, generated by `tools/gen_remix_actions.py`); the tournament flag of an entry is the TE column read from the image.

## 6. Pokemon Settings page

- **Pokemon SFX** (`pokemon_voices`; 0 DEFAULT, 1 JAPANESE, 2 RANDOM; default 0/0/0/1)  
  Asm: Item.asm `pokemon_variant_sfx`: JAPANESE plays the Japanese voice of 10 Pokemon sounds (Saffron Chansey uses Chansey J); RANDOM picks the Japanese voice on RNG bit 3.  
  Port: `itRemixPokemonVoiceFgm` (it/itremix/itremixpokemonvoices.c:47), called at the ten Pokemon voice sites (itkabigon, ittosakinto, itkamex, itmlucky, itdogas, itpippi, ithitokage, itfushigibana, itglucky).

**Toggle:** row (`toggle_all_`) flips the Pokemon rows. Thirteen Pokemon rows (`Onix`, `Snorlax`, `Goldeen`, `Meowth`, `Charizard`, `Beedrill`, `Blastoise`, `Chansey`, `Starmie`, `Hitmonlee`, `Koffing`, `Clefairy`, `Mew`) are bool, default 1 in every profile; they are the candidates of the Poke Ball. Psyduck, Shellder and Staryu are commented out in the asm.
Port: `itRemixPokemonMakeMonster` and `itRemixPokemonResetPicker` (`it/itremix/itremixpokemonpicker.c`, replacing the Mew check and the monster roll of `itMainMakeMonster` and the vanilla `itManagerInitMonsterVars`). The roll excludes the last two picks when 3 or more Pokemon are on, the roll range shrinks from 12 to 10 over successive spawns, and Mew is a 1 in 151 roll made without the vanilla newcomer unlock check. With every row off the Poke Ball spawns a plain Poke Ball item (kind 0x13). `SSB64_REMIX_ITEM_LOG=1` prints `monster kind=0x..` per spawn and `containers_mask`.

## 7. Other Remix pages of the port menu

These are port registries over the Remix CSS and Training menus, not `Toggles.asm` entries. They persist as CVars and accept run overrides.

| Page | Registry | CVar | Run override | Contents |
|---|---|---|---|---|
| Player Panel P1..P4 | `port/remix/remix_player_panel.cpp` (`remix_panel_get/set`) | `gRemix.Panel.<Key>.P<port>` | `SSB64_REMIX_PANEL="Size.2=1;Damage.1=50"` | 21 options per port: Shield, PlayerTag, Visibility, Skeleton, ModelDisplay, InputDisplay (Input HUD Off/Top/Bottom, drawn by `port/remix/remix_input_display.cpp`), Size, StockMode, Knockback, InputDelay, Handicap, StartWith, TauntItem, TauntButton, KirbyHat, DpadFunctions, DpadControl, Damage, PoisonDmg, Practice1P, StockCount. The header comment still says 20; the enum has 21 (`REMIX_PANEL_COUNT`). |
| Player Tags | same file | `gRemix.Panel.TagName.<1..20>` | `SSB64_REMIX_TAG_NAMES="2=afk"` | Names of the 20 custom tags (`css/PlayerTag.asm`). A port shows the name when its PlayerTag option selects the tag. The tag text `afk` enables the 1P AFK helpers (`sc/sc1pmode/sc1premixenemyafk.h`); `cube` (Kirby Cube) has no counterpart. |
| Training | `port/remix/remix_training.cpp` (`remix_training_get`) | `gRemix.Training.<Key>` | `SSB64_REMIX_TRAINING="ShieldBreak=1;Oos=3;SpamPractice=7"` | Keys ShieldBreak, Oos, Music, StageBg, Tech, DiType, DiStrength, DiDirection, DiFirstHit, Dpad, DiPractice, SpamPractice, SpamInterval, SpamRandom, Percent1..4, ResetSetsPercent1..4, Type1..4, Character1..4, Costume1..4, Spawn1..4 (`Training.asm` 3899-3913). |
| Modes | `port/remix/remix_modes.cpp` | `gRemix.Modes.SmashketballType` (0 basketball, 1 soccer) | none | The Smashketball type toggle of the VS character select. |
| Tag Team | same file | `remix_modes_tagteam_cvars()` | none | Fighters 2..6 of each port's Tag Team queue (slot 1 is the CSS pick). |

## 8. Behaviour outside the registry

These Remix features have no `Toggles.asm` entry; they are always on or driven by something else.

| Feature | Asm | Port |
|---|---|---|
| Sword trails | `SwordTrail.asm`: trail id in `fp->afterimage.is_itemswing`, 21 custom trails by fighter, joint, axis and colours (always on). | `ft/ftremix/ftremixswordtrail.c` (`ftRemixSwordTrailRecord`, `ftRemixSwordTrailGetDraw`), wired in `ft/ftmain.c:4603` and `ft/ftdisplaymain.c:476`, `:1027`. The custom draw uses a 30 degree rotate that the asm does not set. |
| DK Mode and Lanky Mode | `DKMode.asm _resize_head`: scales the head (1.5x, or 0.5x in Lanky Mode) and arms (1.5x) of every fighter from `Character.fighter_DK_mode.table`; `dk_mode_enabled` is written by the 12-Character Battle portrait easter egg. | `ftRemixDKModeResizeHead` (`ft/ftremix/ftremixdkmode.c`), called in `ftDisplayMainProcDisplay` (`ft/ftdisplaymain.c:1172`). The setting is a C static (the RAM image is read-only); set `SSB64_DK_MODE=1` (DK) or `2` (Lanky) to test. Skips Master Hand and Sandbag. |
| Magnifying glass zoom | `MagnifyingGlass.asm apply_custom_zoom_`: per-fighter zoom from `Character.magnifying_glass_zoom.table`. | `gmRemixMagnifyApplyCustomZoom` (`gm/gmremixmagnify.c`), wired in `ifCommonPlayerMagnifyUpdateViewport` (`if/ifcommon.c:1739`). |
| Cheats.asm | One live patch: unlock everything (`0x007F0C90`). The AA, stock-number, Pupupu and BGM-store patches are commented out in the source and emit nothing. | Default save in `sc/scmanager.c:342`; the port's own unlock cheats stay in `port/enhancements/UnlockCheats.cpp`. |
| Pause legends | `Pause.asm`: R legend, music and camera legends, D-pad Left HUD toggle, live music title. | `if/ifremixpauselegend.c` (sprites plus `remix_hud_text` rows, groups 0xE-0x10 through `gm/gmremixgroupdisplay.c`); the R press is ignored while a Gentlemen's Reset is requested (`ifRemixGentlemensResetIsRequested`). |
