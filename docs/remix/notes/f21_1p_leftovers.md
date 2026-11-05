# F21 1P leftovers (ROADMAP M11 Fase 5, 1P line + Fase 4 leftover `check_high_scores_enabled`)

Worker note, 2026-11-05. Asm = `.toolchain/pkgs/smashremix-src/src/`. Nothing here was built or run: every C file touched passed the syntax check
(`syntax_check.py` / `syntax_check_new.py`, `remix_toggles.cpp` with the flags of `build/compile_commands.json` and `-fsyntax-only`), and the three
unit test files were compiled alone with `cc` (182 checks of `test_css_panel_menu_rules.c`, 61 of `test_1p_leftover_rules.c`, 61 of the new
`test_1p_bonus_modes_rules.c`, all passing). The lead builds, runs and signs off.

## 1. asm -> port

| Asm | Port |
|---|---|
| `SinglePlayer.asm` `high_score_enabled` (:15) | `sc1premixhighscores.{c,h}`: flag `sc1PRemixHighScoresIsEnabled` (starts on, survives the scene like the asm variable) |
| `extend_high_score_1p_write_` (:1306, 0x800D6744) | `sc1PManagerTrySaveBackup` (`sc1pmanager.c`): returns when `sc1PRemixHighScoresCanSaveRecord()` is FALSE (switch off or enemy port on) |
| `extend_high_score_btt/btp_count_write_`, `_time_write_` (:257, :346, :558, :648) | `sc1PBonusStageWriteBackup` (`sc1pbonusstage.c`): same check at the top |
| `_time_save` (SinglePlayerModes.asm :1296, Bonus 3 and HRC) | `sc1PRemixBonus3SaveRecord` (`sc1premixbonus3.c`, replaces `sc1PRemixBonus3CanSaveRecord`) and `sc1PRemixHrcRecordSave` (`sc1premixhrcrecord.c`, did not check the enemy port before) |
| `_ko_save` (:1210, Multi-Man and Cruel) | `sc1PRemixMultimanSaveRecord`: only the switch (the asm does not look at the enemy port there): `sc1PRemixHighScoresCanSaveKoRecord` |
| `CharacterSelectDebugMenu.asm check_high_scores_enabled` (:2366-2417) | `mnRemixPanelMenuRulesHighScoresEnabled(mode, values, globals)` (`mnremixcsspanelmenurules.c`) plus `mnremixcsshighscores.c`, called from `mnRemixPanelMenuRefreshHighScores` every frame on the 1P, Bonus and All-Star select |
| the same, `_check_toggles` | Gameplay word 1 `& 0xFFEFFF7F` must be 0 (Toggles.asm 2419-2428: Hitstun 1 bit, Hitlag 3, DI 2, Japanese Sounds 2 on bits 6-7, Japanese Stun/Sleep 1, Momentum 1, Shield Stun 3, Z-Cancel 3, Punish 4, so Improved AI is bit 20 and bit 7 is Japanese Sounds value 2); Gameplay word 2 must be 0; Pokemon word `& 0xFFFC` must be `0x7FFC` (Toggles.asm 2623-2637: Pokemon SFX bits 0-1, 13 booleans on bits 2-14, all on by default). The words are packed by `mnRemixPanelMenuRulesPackToggles` with the layout of `update_block_size` (Toggles.asm 1016-1038: width from the highest value, lowest bit first, a toggle ending on or past a word boundary moves to the next word) from `remix_toggle_group_snapshot` (new, `port/remix/remix_toggles.{h,cpp}`) |
| the same, enemy port (:2396-2417) | `globals->enemy_port` from `sc1PRemixEnemyPortGet()`; a non-zero value turns the records off |
| `CharacterSelect.asm set_high_scores_disabled_visibility_` (:4767) | HUD text "High Scores Disabled" (slot 96, x 192, y 217, scale 0.8125, 0xFF8080FF) while the switch is off, in `mnRemixCssHighScoresUpdate` |
| `SinglePlayerModes.asm bonus2_css_2_1p` random roll (:233-243) and `CharacterSelect.asm save_random_char_._bonus` (:6893-6953) | `mnRemixBonusModesResolveRandom` (rolls with `mnRemixRandomPickFighterKind`, the locked check of the screen and `mnRemixCssRandomKeepRoll`, costume by `mnRemixCostumeGetRandomLegal`), called from `mnPlayers1PBonusSetSceneData`, which runs before `sc1PRemixModeExitBonusCSS` |
| Random bookend of `draw_portraits_` (CharacterSelect.asm :3791) on the Bonus screen | `mnRemixCssRandomMakeBookend` in `mnPlayers1PBonusMakePortraitAll`; token over it gives `REMIX_ID_RANDOM` (`mnPlayers1PBonusGetRemixForcePuckFighterKind/PuckFighterKind`); the puck nudge and the white flash use the bookend box (`mnRemixBonusModesGetCell*`) |
| `SinglePlayerMenus.asm header_graphic` (:780-803) | `mnPlayers1PBonusMakeLabels`: Bonus 3 0x1DD8, Multi-Man 0x14D8, Cruel 0x1958, HRC 0x24C8 in the game modes file (`mnRemixBonusModesHeaderOffset`) |
| `counter_graphic` (:748), `_ko_amount_or_feet` (:458), `draw_hrc_high_score_decimal_` (:681) | `mnRemixBonusModesMakeRecord` from `mnPlayers1PBonusMakeBestTaskCount`: KOs label 0x34E8 or feet label 0x3C08 in the 1P mode file, 5 or 7 fixed digits, and for the HRC the tenths digit moved 4 right and a period (colon image, `bmheight` 5) at x 216 |
| `load_bonus3_time` (:508), `_timedisplay_skip` (:566) | `mnPlayers1PBonusGetBestTime` reads `sc1PRemixBonus3RecordGet` (course in use) in Bonus 3; `mnPlayers1PBonusMakeHiScore` uses `mnRemixBonusModesShowsTime` |
| `total_kos_or_feet_` (:603-668) | `mnRemixBonusModesShowsTotal` (always for Multi-Man, Cruel, HRC) in `FuncStart` and `UpdateGameMode`; `mnRemixBonusModesMakeTotal` from `mnPlayers1PBonusMakeTotalTime`: label cut to 24 wide at x 235, the sum of the records right aligned at x 225, y 205, 6 or 8 digits, console filler 0xAAAAAAAA counted as 0 |
| `Bonus.asm setup_`, `arrow_routine_`, `handle_arrow_press_` (Bonus 3 part), `Bonus.mode` | `mnRemixBonusModesToggleMake/Update/Press`: Option title (1P mode file 0x1EC8 at 198, 129, tint 0xAFB1CC), "MODE:" (right aligned x 208, y 156) and the course name (centred x 249; "NORMAL" 0xE4BE41, "REMIX" 0xE44141) as HUD text, arrows `llMNPlayersCommonArrowL/RSprite` at x 212 and 280, y 157, blinking 11 of 20 frames, the left one hidden on Normal and the right one on Remix, press by the cursor hot spot (+20, +3) with the paddings of `check_image_footer_press_`; the choice goes to `sc1PRemixBonus3SetCourse`, so the race uses `nGRRemixStage_REMIX_RTTF` and saves `bonus3_remix_time` |
| `rest_area_setup` `_portrait` (:7596-7660) with `Render.draw_stage_texture_` | `sc1premixallstarrestportraits.{c,h}`: world-space quad (400 x 400, centre y 1000, z -20, prim alpha 0xA0, RGBA16 32 x 32 from the Remix portrait file 0xA05 at `kRemixCssPortraitOffset[fkind]`) drawn by a display proc on link 6; count from `sc1PRemixAllstarOpponentsAt` capped by the fighters left, x -1000 / -600 / -200 (the asm loop takes the slot from its own down counter, so the next fighter is the leftmost of three) |
| `costume_fix` (:8179) | already covered by `sc1PRemixAllstarPrepareFinal` (see 3); no new code |

## 2. Files

New: `sc/sc1pmode/sc1premixhighscores.{c,h}`, `sc1premixhighscoresrules.{c,h}`, `sc1premixallstarrestportraits.{c,h}`,
`mn/mnplayers/mnremixcsshighscores.{c,h}`, `mnremixbonusmodes.{c,h}`, `mnremixbonusmodesrules.{c,h}`, `tests/remix/test_1p_bonus_modes_rules.c`.

Edited (all under `decomp/src/` unless noted): `sc/sc1pmode/sc1pmanager.c`, `sc1pbonusstage.c`, `sc1premixbonus3.{c,h}`, `sc1premixbonus3rules.{c,h}`
(`sc1PRemixBonus3CanSaveRecord` removed, replaced by `sc1PRemixHighScoresAllowSave`), `sc1premixhrcrecord.c`, `sc1premixmultiman.{c,h}`
(`sc1PRemixMultimanRecordGet`), `sc1premixallstar.c`, `sc1premixallstarrest.h`, `sc1premixallstarrestrules.{c,h}`;
`mn/mnplayers/mnplayers1pbonus.c` (shared with the Classic Sonic worker; hooks: includes, header offset, `GetBestTime`, `MakeBestTaskCount`,
`MakeHiScore`, `MakeTotalTime`, total condition in `FuncStart` and `UpdateGameMode`, Random bookend, toggle make/update/press, two puck kinds, flash and
puck nudge cells, roll in `SetSceneData`), `mnremixcsspanelmenu.c` (one line, the refresh), `mnremixcsspanelmenurules.{c,h}`;
`port/remix/remix_toggles.{h,cpp}` (`remix_toggle_group_snapshot`), `port/remix/remix_hud_text.h` (slot map: 96..104), `tests/remix/CMakeLists.txt`,
`test_main.c`, `test_1p_leftover_rules.c`, `test_css_panel_menu_rules.c`.

Unit tests: `test_css_panel_menu_rules.c` (packing widths, boundary rule, Improved AI bit 20, Japanese Sounds bits, Pokemon default 0x7FFC, masks, enemy port),
`test_1p_leftover_rules.c` (record switch, KO switch, Rest Area portrait count and slots), `test_1p_bonus_modes_rules.c` (titles, labels, digits, time or count,
total, filler, course steps, arrows, blink, press padding).

## 3. Item by item

1. Bonus 2 CSS: titles, records (KOs, feet with the decimal point, Bonus 3 times by course), total line and Random token with roll are ported (table above).
2. Title cards: **not changed**. The asm only swaps the character of the Yoshi card (`remix_yoshi_card`, :5303), the Piano and Metal Bros use the camera animation at
   0x7070 (`_mad_piano`, `_metal_bros`: :3896-3925, :3869-3893), the Marina / Banjo / Crash widths and the Team / Giant / Tiny texts are in the port already. What the
   leftover notes describe (18 Marinas on one pose, a large Piano, a scrambled Metal Mario Bros texture) is rendering of the Remix models and needs the running game
   against the emulator (there is no reference screenshot in `build/ref-shots`); the code path differs from the asm in no place I could find. The likely cause of the
   single Marina is that the card status 0x1000E of a Remix fighter has one pose whatever `card_anim_frame_id` is (the vanilla Yoshi file has one pose per index).
3. Rest Area: portraits ported (table). Icons and hearts were there. The asm sprite comes from `CharacterSelect.portrait_offset_by_character_table`; the port uses the
   generated `kRemixCssPortraitOffset`, which comes from the same table. If the sprite is not RGBA16 32 x 32 the portrait is skipped and `allstar: rest portrait skipped`
   goes to the log (the texture setup assumes it, like `model_part_image` does).
4. Bonus 3 Remix course: the stage (`nGRRemixStage_REMIX_RTTF`), the Remix polygons and the `bonus3_remix_time` record were there; what was missing was the choice in
   the CSS (`Bonus.mode`), now the MODE row of Bonus 3. The record shown in the CSS follows the course.
5. High scores: the writers listed above read the switch; the rule has the Gameplay and Pokemon toggles and the enemy port.
6. `costume_fix`: the hook (rom 0x10C350, in `sc1PGameSetupStageAll`) is on the Yoshi row, which All-Star reuses for the final team; the effect (at most four costumes for
   vanilla fighters of the final team, `random(4)` instead of the drawn value when it is 4 or more) is `sc1PRemixAllstarPrepareFinal`, which draws one of six and redraws
   one of four for vanilla fighters. `_nSC1PGameStageAllstarFinal_` (:8353) has its own "modified version of costume_fix" with the same rule.

## 4. Deviations and open points

- Correction of the first report: `GMCOMMON_FIGHTERS_SAVE_NUM` is 97 in the Remix build, so `spgame_records` already has a row per Remix fighter (the "12 entry" comments in
  the code are stale). Multi-Man, Cruel, HRC and Bonus 3 records, and the standard Bonus 1/2 records, were already kept for every fighter. What was missing is below (section 6).
- Bonus screen: the Option panel keeps the bar as a HUD rectangle (slot 46), so it is drawn over the game sprites (the asm draws it first, under the outline and the title).
- The "TOTAL" label keeps y 209 (the asm stores `f6`, a register whose value I could not trace); the number is at y 205 as in the asm.
- The Random roll uses the lock rule of the Bonus screen (vanilla Ness, Jigglypuff, Captain Falcon, Luigi).
- The portraits are drawn on display link 6 like the KOTH hill strip; if the Rest Area stage draws its map after that link, they could be hidden behind it.
- `remix_toggle_group_snapshot` returns 0 toggles when the registry is not built; then the Pokemon word is 0 and the high scores stay off. The registry is built at start-up in the
  Remix build.
- Title cards (item 2 of the first brief) are still not changed, see section 3.
- Save format: `LBBackup1PRecord` grew (`remix_1p`, `allstar`) and `LBBackupData` got `remix_bonus`; the checksum covers the whole struct, so an old Remix save fails the
  checksum once and is reset to the defaults (the same as when `bonus3_time` was added). Vanilla builds are untouched (all under `defined(PORT) && defined(REGION_REMIX)`).
- `page_flag` (the 1P menu page that Remix 1P and All-Star reset at the end of a run, SinglePlayer.asm :1340, :1346) is not touched: the port keeps no page flag of the menu.

## 5. In-game recipe for the lead

Scenes: 20 = Bonus 2 CSS (19 = Bonus 1), 17 or the 1P CSS for item 5, 52 = 1P game, 18 = Training CSS. Mode by `SSB64_REMIX_1P_MODE` (`bonus3`, `multiman`, `cruel`, `hrc`,
`allstar`, `remix1p`), toggles by `SSB64_REMIX_TOGGLES="Hitstun=1"` (title=value, case does not matter) or `SSB64_REMIX_PROFILE`. Cursor hot spot = cursor sprite position + (20, 3).

1. High scores (item 5): `SSB64_START_SCENE=20 SSB64_REMIX_1P_MODE=multiman`, capture frame ~60: no "High Scores Disabled" text. Same with
   `SSB64_REMIX_TOGGLES="Hitstun=1"`: the red text at the lower right (x 192, y 217). With `Pokemon SFX=2` and with `Improved AI=0` it must stay hidden / hidden. Panel option: open the
   panel menu (button at x + 4, y 130 of the panel), Size to Large: text appears; reset: text goes. Play Bonus 3 to the finish with the text up and check that
   `bonus3: record saved` does not appear in `ssb64.log` (it does with the text down).
2. Bonus 2 titles and records: scene 20 with each of `bonus3`, `multiman`, `cruel`, `hrc`; frames ~60 and ~200 with the hand on a vanilla fighter (input line
   `0 40 0 -` then move to a portrait; the grid hot spots are in `remix_css_data.h`): title sprite per mode, KOs / feet label, 5 or 7 digits, HRC period, the total line at the right bottom.
   Seed a record first (play Multi-Man and die with KOs, or run `hrc`).
3. Random on Bonus 2: scene 20, put the token on the Random bookend (x 279..294, y 44..116) with A and START; `mnRemixBonusModesResolveRandom` should leave a
   rolled fighter in `gSCManagerSceneData.bonus_fkind` (the 1P game starts with a fighter, never the "?").
4. Bonus 3 course: `SSB64_START_SCENE=20 SSB64_REMIX_1P_MODE=bonus3`, frame ~60: the Option title and `MODE: NORMAL` with the right arrow blinking; click the right arrow
   (A with the cursor on x 280..296, y 157..168): `REMIX`, red, the left arrow shows; start the race and check `nGRRemixStage_REMIX_RTTF` in the log and the Remix polygons; finish:
   `bonus3: record saved ... time=` goes to `bonus3_remix_time`, back on the CSS the record is that of the Remix course. `SSB64_REMIX_BONUS3_COURSE=normal|remix` still forces it.
5. Rest Area portraits: `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=allstar SSB64_SPGAME_FKIND=<fkind>`, frame ~120 of the first Rest Area: one translucent portrait on the wall at the
   far right of the three slots (x -200); after the first win (progress 1) still one; from progress 4 two, from 0x12 three. Compare the position with the icons and the hearts.
6. Regression: `b9_bonus3`, `b9_giga` unchanged; Bonus 1/2 vanilla (Mario, `0 40 0 -`...) still start the bonus stage; the Bonus screen without a mode must show no Option row.

## 6. Follow-up (2026-11-05, same worker): records for every fighter, Remix bonus stages, Option panel, bonus bookend

### 6.1 Records (item 1)

| Asm | Port |
|---|---|
| `extend_high_score_1p_write_` `_remix_1p` / `_allstar` (SinglePlayer.asm :1306-1363), `extend_high_score_1p_` (:1136), `_bonus_count_` (:1226), `_difficulty_` (:1262) | `LBBackup1PScore remix_1p` and `allstar` per fighter in `LBBackup1PRecord` (`lbtypes.h`), `sc1premixscore.{c,h}` (`sc1PRemixScoreTrySave` called from `sc1PManagerTrySaveBackup`, getters for the 1P CSS in `mnplayers1pgame.c`: high score, bonus count, best difficulty), table choice in `sc1premixscorerules.{c,h}`; `lbBackupClear1PHighScore` clears them |
| `Bonus.REMIX_BONUS_HIGH_SCORE_TABLE` (Bonus.asm :863) and the `_remix` branches of `extend_high_score_btt/btp_count_write_`, `_time_write_`, `_btx_count_`, `_btx_time_`, `_btx_count_check_` | `LBBackupBonusRecord remix_bonus[33][97][2]` in `LBBackupData` (count and time per stage, fighter and targets/platforms; the asm packs a 3 byte time and an 0xA marker, same meaning), `sc1pbonusremix.{c,h}`: `sc1PBonusRemixTrySave` (called first in `sc1PBonusStageWriteBackup`; a run on the fighter's own stage, `Character.BTT_TABLE`/`BTP_TABLE`, stays in the standard record), readers behind `mnPlayers1PBonusGetBestTime`, `GetBestTaskCount` and `CheckBonusComplete`. `lbBackupClearBonusStageTime` clears them |
| the page flag reset | not ported, see 4 |

Every writer still goes through `sc1PRemixHighScoresCanSaveRecord` first.

### 6.2 BTT/BTP Remix stage tables, STAGE row, "L: Set Stage" (item 2)

- Tables read from the Remix image (new symbols in `tools/gen_remix_actions.py`: `Bonus.btt_stage_table`, `btp_stage_table`, `stock_icon_table`, `Stages.bonus_pointer_table`,
  `Character.BTT_TABLE`, `BTP_TABLE`, `REMIX_BTT_TABLE`, `REMIX_BTP_TABLE`).
- Stage of a bonus run (`set_btt_stage_id_`, `set_btp_stage_id_`): `sc1PBonusRemixGetBattleGkind`, used for the four `gkind` assignments of `sc1PBonusStageInitVars`: the Remix 1P
  run uses `REMIX_BTT_TABLE`/`REMIX_BTP_TABLE`, the Remix choice uses the stage picked on the STAGE row, fighters from `Character.id.BOSS` up use their own stage; the first 12
  keep the vanilla `fkind + start`.
- Targets and platforms of the added stages: `bonus_fix_2_`/`bonus_fix_4_` (`sc1PBonusStageGetTarget`, `sc1PBonusStageGetBumperOffsets` from `Stages.bonus_pointer_table`),
  `bonus_fix_1_` (`grRemixMakeGround` calls `sc1PBonusStageMakeBonus1Ground` / `sc1PBonusStageInitBonus2` for class BTT / BTP) and `bonus_fix_3_` (`sc1PBonusStageIsPlatforms`
  uses the class table outside the vanilla range, in the camera, task sprites, backup writer and end of stage).
- CSS: `mnremixbonusmodes.c` shows the Option panel on Bonus 1, 2 and 3, the STAGE row (label slot 99, arrows at x 212 / 280, y 176, one or two stock icons of
  file 0xD8B at x 245 or 240/250, y 176) when the choice is Remix, and "L: Set Stage" (slot 100, text at 78.5, 217, L button at 61.75, 216) while a token is held; L released
  sets the row to the stage of the held fighter (the Random token picks one of the 33); `mnRemixBonusModesIsManualStart` stops the 140 tic auto start with the Remix choice
  (`manual_remix_bonus_start_`).
- Not checked (needs the game): the ground functions of the added BTT/BTP stages, the stage files and the hazards of those stages (`Hazards.asm` has BTT entries); unported ground
  code would leave the stage without targets, and `grRemixMakeGround` logs the stage if it reaches the old default branch. `ftcommondead.c` and `ifcommon.c` still test the vanilla
  bonus ranges (death sound size, failure message), which is harmless for the added stages but not what the asm does.

### 6.3 Option panel bar and outline (item 3)

The filled bar (175, 136, 144 x 5, 0x576088FF) is HUD rectangle slot 46; the outline is the Option box sprite of the 1P game (1P mode file 0x1208) at (146, 139) with
`cms` wrap, `cmt` mirror, `maskt` 5, `lrs` 184, `lrt` 64, the values of the mirror words in Bonus.asm setup_ (0x01000500 and 0x00B80040).

### 6.4 Bonus bookend and slide-in (item 4)

CharacterSelect.asm `draw_portraits_` (:3775) draws the bonus bookend on every screen but the 12CB one, and `check_variant_input_` / `draw_bonus_scroll_indicators_` handle it
on the Bonus screen as well (`bonus_char_index_bonus`, :58). Ported with the existing modules: `mnRemixCssBonusMakeBookend`, `mnRemixCssBonusGetFighterKind`,
`mnRemixCssBonusScroll` and `mnRemixCssBonusUpdateHint` (new `mnRemixCssBonusSetHintPanelX`, because the hint is anchored to the card: x 58 on this screen) through
`mnRemixBonusModesUpdateScroll` and `mnRemixBonusModesUpdateHint`; the puck kinds return the bonus fighter over the bookend; the puck nudge and flash use the bookend box.
The slide-in is `mnRemixCssSlideAttach`, which the bookends already call, now also attached to the 30 grid portraits of the Bonus screen (`mnPlayers1PBonusMakeRemixPortrait`).

### 6.5 Tests

`test_1p_bonus_remix_rules.c` (47 checks: stage index, own stage, arrow wrap, count and time rules, stage source per mode, platform class, "L: Set Stage" visibility, icon
positions, score table per mode and replace rule), added to the suites already named above. New rules files in `tests/remix/CMakeLists.txt`: `sc1pbonusremixrules.c`,
`sc1premixscorerules.c`, `sc1premixhighscoresrules.c`, `mnremixbonusmodesrules.c`.

### 6.6 Recipe for the lead

1. Save reset: the first run after the build resets the save once (checksum); check `ssb64.log` does not crash on it.
2. Remix 1P / All-Star score: `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=remix1p SSB64_SPGAME_FKIND=<a Remix fkind, 0x3F>`, finish or lose with a score, then `SSB64_START_SCENE=17
   SSB64_REMIX_1P_MODE=remix1p`: HIGH SCORE shows the Remix 1P value and the standard 1P CSS (no mode) still shows the standard one. Same with `allstar`.
3. Bonus 1/2 with a Remix fighter: `SSB64_START_SCENE=19` (Bonus 1) / `20`, put the token on a Remix fighter (Falco, Lucas...) and START: the bonus stage must be that
   fighter's own (log `gkind`), targets/platforms appear, finish it and see the record on the CSS. This is the main risk (section 6.2).
4. Remix choice: scene 19, right MODE arrow (x 280, y 157): REMIX in red, the STAGE row with the icons of stage 0 and the arrows; hold a token on Mario, release L: the row
   goes to Mario's stage; Random token + L: a random stage; START starts (no countdown); play and check the record is stored under the picked stage (`remix_bonus`), and that
   the standard record of the fighter is untouched. Dump `SSB64_REMIX_*` is not needed.
5. Bonus bookend: scene 20, token over the left bookend (x 24..39, y 44..116), Z or R held scrolls the ten bonus fighters, the hint (arrows and Z/R) blinks over the panel
   card; the portraits and both bookends slide in at the start.
