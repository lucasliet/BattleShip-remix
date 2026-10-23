# F11 mode note: Smashketball (VS Remix mode 4)

Source: `.toolchain/pkgs/smashremix-src/src/Smashketball.asm` (1473 lines, read in full), with the hook call sites in the
other Remix asm files listed below. Brief: `docs/remix/f11_mode_research_brief.md`. Nothing was built or run. Line numbers
are the asm lines unless a file is named. Items marked "nao verificado" were not confirmed by reading the code.

Memory: the three `_global` rules pages were read (`_rules/no-agent-commit-attribution.md`,
`_rules/no-live-provider-tests.md`, `_rules/ai-memory-operational-recall.md`). The project scope could not be resolved:
there is no `.ai-memory.toml`, `project: "JRickey/BattleShip"` is rejected, and `workspace: "JRickey"` is not found. The
project rules were not loaded.

## 1. What the mode is

Two teams (Home and Away, forced by `Global.vs.teams = TRUE`) play basketball or soccer with one ball. A team wins when one
of its scores reaches the target `winning_score` (default 10, picked on the CSS from `score_value_array`: 1 to 7, 10, 15,
20, 50, 99 and 100). The value 100 means no score target (`check_match_end_`, lines 1080-1082), and the scoreboard then
shows dashes. If the scores are tied when the match would end, sudden death starts: the time is set to infinity and the
first team that breaks the tie wins (`prevent_sudden_death_` in `TimedStock.asm:184-205`, `_sudden_death` at line 1092).

Selectors: `VsRemixMenu.vs_mode_flag == VsRemixMenu.mode.SMASHKETBALL` (value 4). The port menu stores the same value in
`gMNVSModeRemixMode` (`decomp/src/mn/mnvsmode/mnremixvsmode.h`, `MNREMIX_VSMODE_SMASHKETBALL`). `Smashketball.type`
(0 basketball, 1 soccer) is set on the CSS. `Global.vs.game_mode` is set to TIME (1) in `before_css_setup_` (lines 143-145).
Single player flag: not read by this file. The only single-player check in the Smashketball hooks is in `HRC.asm:680`, which is outside this mode.

Scoring rules, from the code:
- Basketball: a basket is when the ball goes from above the rim to below it while passing through the hoop (see section 4).
  A basket is worth 1 point, or 3 when `is_golden` is set (`_scored`, lines 806-811).
- Soccer (`type == 1`): a goal is when `|x|` of the ball is at least `0x4586` as f32 (computed here as about 4144.0, nao
  verificado against the float table). A goal is worth the same as a basket.
- Left hoop (x < 0) credits the Away team; right hoop (x >= 0) credits the Home team (lines 790-794 and 798-805).
- Player score: an attributed basket (the ball's last owner is on the scoring team) adds 1 to that port's `scores[port]`,
  adds 6 to `winner_scores[port]`, and increments that port's `consecutive_baskets` (lines 853-889).
- Unattributed basket (no owner, or an owner from the other team): `winner_scores` of each teammate on the scoring team
  gets `winner_score_distribution[teammates]` (6 for 1 player, 3 for 2, 2 for 3), times 3 when golden (lines 823-851).
- Three or more consecutive attributed baskets force the crowd chant (`force_chant_`, lines 895-901).

## 2. Flow

Entry and screens changed, in order. "Port" says the state of the port as seen in the decomp (no mode logic is ported yet).

| # | Screen | Asm hook (file:line, patched address, what it does) | Vanilla decomp counterpart | Port |
|---|---|---|---|---|
| 1 | VS menu page 2 | `VsRemixMenu.asm:96` (button data, mode 4, text at `dw 0x8C38`): page 2 index 3 ("Smashketball") | `decomp/src/mn/mnvsmode/mnvsmode.c` (VS menu). Name: no match found for a Smashketball function | `mnremixvsmode.c` row 3 and `mnvsmode.c:1413` store `gMNVSModeRemixMode = 4`. Done |
| 2 | Before CSS | `VsRemixMenu.asm:539` `mode_setup_table` entry calls `Smashketball.before_css_setup_` (`Smashketball.asm:139-173`). Sets `Global.vs.game_mode` 1 (TIME), `0x8013BDAC`; `Global.vs.time` = `winning_score`, `0x8013BD7C`; `Global.vs.teams` = TRUE, `0x8013BDA8`; `Global.vs.stage_select` = 0; clears `StockMode.stockmode_table` (4 words) | `mnvsmode.c` save settings path (`mnVSModeSaveSettings`). Name: no match found | Not ported |
| 3 | CSS (VS players) | `CharacterSelect.asm:6307-6314`: allows toggling in SMASHKETBALL. `Smashketball.asm:1337-1395` `css.enable_toggling_mode_`: hooks `0x1335FC` (toggle type, `0x8013537C`), `0x133680` (title offset: 0x2C18 "Smashketball 1" or 0x2E88 "Smashketball 2", `0x80135400`), `0x133724` (skip rest of team routine, `0x801354A4`) | Screen: `decomp/src/mn/mnplayers/mnplayersvs.c` (the ROM address comment in the asm points at `mnPlayersVSUpdateGameMode`). Name of the hook: no match found | Not ported. CSS Remix code is not in the port |
| 4 | CSS, GOAL picker | `Smashketball.asm:1399-1419` `use_goal_yellow_picker_` (hook `0x132258`, `0x80133FD8`): uses the GOAL background offset `0x27FC8` for Smashketball, TIME `0x48B0` otherwise | `mnplayers1ptraining.c` (ROM mapping only, name: no match found) | Not ported |
| 5 | CSS, start check | `VsRemixMenu.asm:698-704` (`control_match_start_`, hook `0x1388D4`, `0x8013A654`) calls `Smashketball.css.prevent_start_unless_two_teams_` (lines 1423-1469): start only with exactly two teams present | Name: no match found | Not ported |
| 6 | CSS, team costumes | `VsRemixMenu.asm:440-441`: teams costumes always applied in Smashketball | n/a | Not ported |
| 7 | Leave CSS | `VsRemixMenu.asm:621` `mode_setup_returning_table` calls `Smashketball.leave_css_setup_` (`Smashketball.asm:177-207`): saves the picker time or score, restores `Global.vs.stage` (from `VsRemixMenu.global_stage`) and `Global.vs.time` (from `VsRemixMenu.global_time`) | n/a | Not ported |
| 8 | Start match | `VsRemixMenu.asm:678` `mode_setup_start_table` calls `Smashketball.start_match_setup_` (`Smashketball.asm:211-320`): sets stage id at `0x800A4ADF` to `Stages.id.SMASHKETBALL` (0x5A, `Stages.asm:116`) or `Stages.id.SOCCER` (0xD1, `Stages.asm:235`) when type is soccer; computes `player_spawns` (section 3) | Name: no match found. Screen: VS battle start in `decomp/src/sc/sccommon/scvsbattle.c` (`scVSBattleStartBattle`, line 269) | Port has `gmRemixNeutralSpawnApply` (`scvsbattle.c:141-142`), not the Smashketball spawn |
| 9 | VS init | `Render.asm:2286-2287` (do not reset VsStats player count in sudden death), `Render.asm:2301` `jal Smashketball.setup_` (`Smashketball.asm:324-472`): zeroes scores, builds team lists, registers `basketball_main_` and `scoreboard_main_` routines | `scVSBattleStartBattle` (scvsbattle.c:269) is the screen. Name: no match found | Not ported. Port has `mnRemixVSStatsReset` (scvsbattle.c:279) |
| 10 | In match: score check | `VsRemixMenu.asm:731-747` (`check_match_end_`, hook `0x8E980`, `0x80113180`): calls `Smashketball.check_match_end_` (`Smashketball.asm:1067-1104`) | `scVSBattleSetScoreCheckSuddenDeath` (scvsbattle.c:394) is the closest screen function. Name: no match found | Not ported |
| 11 | In match: sudden death | `Smashketball.asm:1264-1331` (`overtime`): `allow_overtime_` (hook `0x10B18C`, `0x8018E29C`), `no_initial_damage_` (hook `0x10AED0`, `0x8018DFE0`, damage 0 instead of 300), `prevent_match_stats_reset_` (hook `0x63468`, `0x800E7C68`). `TimedStock.asm:184-205` `prevent_sudden_death_` (hook `0x10A4F4`) | `scVSBattleStartSuddenDeath` (scvsbattle.c:579) is the screen function. Name: no match found | Not ported |
| 12 | Match end buzzer | `Smashketball.asm:1108-1126` `play_end_buzzer_` (hook `0x8F040`, `0x80113840`): adds FGM 0x431 (end buzzer) to the queue | `ifCommonBattleEndAddSoundQueueID` (`decomp/src/if/ifcommon.c:2881`) per the old table, name: no match found | Not ported |
| 13 | Results | `VsRemixMenu.asm:1226` (`Smashketball.scores` read for stats), `1264-1284` (new "no contest" row for Smashketball, routine change at 1265) | `decomp/src/mn/mnvsmode/mnvsresults.c` (screen). Name: no match found | Not ported. ROADMAP "results-mode-rows" covers it |
| 14 | Return to VS menu | Follows the normal VS menu path. `gMNVSModeRemixMode` is reset to DEFAULT on each menu entry (`mnvsmode.c:1167`) | n/a | Done (menu side) |

Other shared hooks in the match (each is only active in this mode):
- `Hazards.asm:307-345`: stage hazard for stage 0x5A moves the barrel and baskets (`Smashketball.baskets`, line 345).
- `Camera.asm:353-355` and `1253`: fixed camera for stage 0x5A (`fixed_smashketball`). The port has the fixed camera in
  `decomp/src/gm/gmremixcameramode.c:68` and `:112` (`sGMRemixCameraFixedSmashketball`, `nGRRemixStage_SMASHKETBALL`).
- `ComboMeter.asm:212-213`: combo stats are not reset in Smashketball sudden death.
- `TwelveCharBattle.asm:1771-1773`, `1817-1819`, `1892-1927`: the CSS announcer call and the title image offsets use
  `Smashketball.type` and the mode flag.
- `VsRemixMenu.asm:994`: the CSS announcer is skipped for Tag Team, KOTH, Smashketball and Tug of War.
- `VsRemixMenu.asm:1033-1116`: draw code for alternate textures (KOTH and Smashketball).

## 3. State and data

Mode globals (`Smashketball` scope, file-local to the asm, all `dw` unless noted):
- `type` (line 59): 0 basketball, 1 soccer. Written by the CSS toggle hook (line 1358, `li a2, type`).
- `spawn_golden` (64): TRUE makes a soccer ball spawn golden. Written at line 1470 (`Basketball.asm` reads it at 124).
- `winning_score` (69): default 10. Written in `before_css_setup_` (line 150-153) and read at `start_match` and `check_match_end_`.
- `score_value_array` (74): 13 values (1, 2, 3, 4, 5, 6, 7, 10, 15, 20, 50, 99, 100).
- `scores[4]` (79), `team_scores[4]` (84), `winner_scores[4]` (92): per port and per team, written in `setup_` (lines 346-361) and
  in the basket code (lines 811, 841-846, 881-889). `team_scores` is indexed by team (0 to 3, red/blue/green/yellow).
- `teammates[4]` (97): player count per team, written in `setup_` (lines 384-387).
- `winner_score_distribution[4]` (103): 6, 6, 3, 2. Index 0 is the attributed amount (line 885). Index n (1 to 3) is the
  unattributed amount for a team with n players.
- `team_colors[4]` (111): `Color.tag.RED`, BLUE, GREEN, YELLOW.
- `consecutive_baskets[4]` (119, bytes): streak per port.
- `scoreboard` (124), `baskets` (129): pointers, set in `setup_` (line 407) and in `Hazards.asm:345`.
- `player_spawns[4]` (135): `2, 0, 0, 0`. Written in `start_match_setup_` (lines 240-309). Port 1 is always 2.
- `spawn_table` (319): `2, 0, 1, 3` (home team left to right: 2, 0, 1; away team right to left: 3, 1, 0).

Globals and RAM the mode reads or writes (raw addresses are the asm's own):
- `Global.vs.game_mode`, `Global.vs.time`, `Global.vs.teams`, `Global.vs.stage_select`, `Global.vs.stage`: VS settings.
  Raw copies at `0x8013BDAC` (mode), `0x8013BD7C` (time, also the CSS picker value), `0x8013BDA8` (teams), `0x800A4ADF` (stage id).
- `VsRemixMenu.global_time`, `VsRemixMenu.global_stage`: saved before CSS and restored after.
- `VsRemixMenu.vs_mode_flag` (dw), `CharacterSelect.showing_time`, `CharacterSelect.saved_nontime_value`, `CharacterSelect.CSS_PLAYER_STRUCT`
  (`0x8013BA88`, 0xBC stride; +0x40 team, +0x84 type, +0x48 char id).
- `StockMode.stockmode_table` (4 words, cleared).
- `0x800A4AE0`: `is_suddendeath` (byte).
- `0x8003B6E4`: frames since screen load (the ball waits 0x135 frames, line 623).
- `0x80046700`: item object linked list head. `0x80046810`: map object (scoreboard digit path, line 412).
- `0x801313F0`: barrel pointer (saved and restored around `Hazards.add_item_`, lines 641-664).
- `0x800A50E8`: `gSCManagerBattleState` (time remain at +0x14, line 981). `0x801317E0`: `sIFCommonTimerLimit`.
  `0x8012EF38`: `dIFCommonTimerDigitsUnitLengths`.
- `0x80046810`-based scoreboard digit structs: offsets 0x40 to 0x5C (digit images), 0x58 (color), 0x60/0x64 (score pointers),
  0x68/0x6C (team index), 0x84 (time-only flag).

Strings, sprites and files (plain constants, no string table):
- Remix-only stage files: `STG_SMASHKETBALL_HEADER` 0x991 (File.asm:332), `STG_SMASHKETBALL` 0x992 (333), `STG_SMASHKETBALL_BG` 0x993 (334).
  Also in `docs/remix/file_table_and_loader.md:396-398` (ids 2449 to 2451).
- Sound files: `sounds/smashketball/bounce`, `fire_bounce`, `spawn_buzz`, `end_buzzer` (`FGM.asm:1364-1367`, 16 kHz voice),
  and `sounds/Smashketball` (`FGM.asm:1070`). FGM 776 is the announcer name id (`FGM.asm:2015`).
- Music: `MIDI.id.NBA_JAM` (stage music, `Stages.asm:5117`).
- Sprite offsets into the CSS file: 0x2C18 and 0x2E88 (titles), 0x27FC8 (GOAL background), 0x48B0 (TIME background).
  Whether these offsets are in a Remix-only file was not verified.
- Text: the scoreboard shows numbers as texture indexes (0 to 9, and 10 for a dash), not text. Titles are sprite images.

## 4. In-match logic (order of execution)

Frame order is approximate. The asm uses Render routines and hooks, which run inside the vanilla update.

1. `basketball_main_` (render routine, `Smashketball.asm:613-940`):
   1. Wait until 0x135 frames after screen load. Then, if no ball exists, spawn one through `Hazards.add_item_` (item id 0x40,
      position x 0, y 0x4500 = 2048.0 or 0x45B0 = 5632.0 when golden, routine 0). `spawn_golden` is cleared after the spawn.
   2. Read the ball. If it is held (owner's held item is the ball), use the owner's item joint position
      (`gmCollisionGetFighterPartsWorldPosition` at `0x800EDF24`). Otherwise use the item position.
   3. Basketball (`type == 0`): rim height = baskets joint y minus `HOOP_OFFSET` (267.0). "Above rim" flag = ball y >= that height.
   4. If the above-rim flag changed since last frame, compute the x at hoop height from the previous and current positions
      (lines 724-748). Passing flag = `|x| >= HOOP_MIN` (3504.0, `0x455B0000`).
   5. Above rim and passing: clear `scoreable`. Below rim and passing and `scoreable` set: score. Otherwise nothing.
   6. Soccer (`type == 1`): score when `|x|` at the ball is at least about 4144.0 (line 781-789).
   7. `_scored`: set `is_scored` (side -1 left, +1 right), add 1 (or 3 if golden) to the scoring team's score, do the
      attributed or unattributed winner-score update (section 1), update streaks, play the chant if the streak is 3 or more,
      play the crowd sound (0x272 cheer, 0x267 groan, or none), and play the basket effect (`efManagerDeadExplodeMakeEffect`
      at `0x801021C0`, sound 0x118).
2. `scoreboard_main_` (`Smashketball.asm:955-1063`): time from `gSCManagerBattleState` time remain (minus one, with the
   59-second rollover at line 986), or dashes when `Global.vs.time == 100`. Scores are shown as two digits each; a score of
   100 or more shows 9 and 9.
3. `camera_follow` (hook `0x87914`, `0x8010C114`, lines 501-609): when a basketball is not respawning, the camera bounds grow
   to include the ball, with padding 1000.0 on x and y. The bottom bound is fixed at -1000.0.
4. `ai_ball_behavior_` (hook `0xAD584`, `0x80132B44`, lines 1152-1203): CPUs target the ball y and an x offset of -400 (home
   team) or +400 (away team), and the distance to that target.
5. `ai_ball_attack_range` (hook `0xADB84`, `0x80133144`, lines 1210-1260): CPU attack range check uses the ball position,
   velocity and gravity. See section 8 for the hurtbox size.
6. `prevent_despawn` (hook `0xEA004`, `0x8016F5C4`, lines 476-495): the ball's pickup wait is not decremented, so it does not
   despawn. `HRC.asm:664-700` (`prevent_item_despawn_`) does the same for the basketball in Smashketball.
7. `check_match_end_` (section 1, section 2 row 10): after each score, a team at the target wins and the match goes to GAMESET
   (`0x80114C80`, jump at `0x80113218`). In sudden death, any difference in score ends the match.
8. `play_end_buzzer_` (hook `0x8F040`, `0x80113840`): at match end, adds the buzzer FGM to the queue.

## 5. HUD and text

- Scoreboard (in the stage model, not an overlay): four score-digit sprites (home 2 digits, away 2 digits) and four time
  digits (minutes 2, seconds 2). Each digit is a texture index written into a joint's image struct at offset 0x80. Team color
  is written to offset 0x58 of the digit structs (`team_colors`). The port cannot draw these through an ImGui overlay if the
  requirement is the same look. It needs the vanilla sprite path, or a clearly approved overlay replacement (the decision is
  for the lead, see section 7 step 7).
- Time shows as MM:SS, or dashes for infinity (value 10).
- Score shows as 0 to 99, then 99 for anything larger (lines 1024-1025, 1042-1043).
- CSS: "Smashketball 1" or "Smashketball 2" title images (offsets 0x2C18 and 0x2E88), GOAL picker background (`0x27FC8`), and the
  type toggle. Text drawn by the CSS, not by this file: none beyond the sprites.
- Announcer: "Smashketball" on the CSS (FGM 776, TwelveCharBattle.asm:1773). Sounds listed in section 3.
- Items: the ball draws through `items/Basketball.asm`. The asm hides the ball while it waits (see the Basketball item note,
  `docs/remix/notes/item_Basketball.md`).

## 6. Dependencies

- StockMode: `before_css_setup_` clears the stock table, so Smashketball does not use stocks.
- TimedStock: in time mode, `TimedStock.asm:123-143` uses `Smashketball.winner_scores` as the score per port. In sudden death
  it reads `Smashketball.scoreboard` (lines 184-205).
- Neutral Spawns: `Spawn.asm:157-183` always uses the neutral spawns in Smashketball, even when the Neutral Spawns toggle is
  off (line 183 comment). `Spawn.asm:961` and `2259` are the table entries for stage 0x5A. The port's spawn toggle follows the
  Neutral Spawns setting only (`decomp/src/gm/gmremixspawnsetup.c:200-201`, REMIX-TODO).
- Stamina: no reference in this file. Not checked elsewhere.
- Results: the VS results screen reads `Smashketball.scores` (`VsRemixMenu.asm:1226`) and has the no-contest row (1264-1284).
- Combo meter: `ComboMeter.asm:212-213`.
- VS stats: `Render.asm:2286-2287`, sudden death keeps the player count.
- Shared dispatch tables (`VsRemixMenu.asm:539`, `621`, `678`) and `vs_mode_flag` with Tag Team, KOTH, 12CB and Tug of War.
- Shared stage 0x5A hazard and fixed camera (see section 2).
- Port state, from the decomp:
  - `decomp/src/it/itremix/itremixbasketball.c`: ball physics, fire and bounce are ported. The mode reads are REMIX-TODO
    (`itRemixBasketballIsSoccer` always FALSE, `itRemixBasketballIsGolden` always FALSE, line 282 and 290). The state is one
    file-local struct (line 112 comment: one ball at a time).
  - `decomp/src/gm/gmremixcameramode.c:68`, `112`: fixed camera for stage 0x5A ported.
  - `decomp/src/gm/gmremixspawnsetup.c:200`: Smashketball spawns are not ported.
  - `decomp/src/mn/mnvsmode/`: menu side ported (`gMNVSModeRemixMode` = 4 on page 2 index 3). No match logic reads it.
  - `port/remix/remix_symbols.h`: no `VsRemixMenu.vs_mode_flag` symbol. `gMNVSModeRemixMode` is the port's stand-in.

## 7. Port plan

Each step is small enough for one worker, touches one file or one hook group, and has a visible result for the lead. Steps
marked "after X" depend on another step or mode.

Step 0 (prerequisite): read the mode without the Remix RAM image. Add an accessor (`sb32 mnRemixVSModeIsSmashketball(void)`
in `decomp/src/mn/mnvsmode/mnremixvsmode.c`) that returns `gMNVSModeRemixMode == MNREMIX_VSMODE_SMASHKETBALL`. Lead sees: no
visual change; unit test passes.

Step 1: type and golden accessors for the ball. Replace the two REMIX-TODOs in `itremixbasketball.c` (lines 281-291) with
reads from a new `decomp/src/gm/gmremixsmashketball.c` state struct (type, spawn_golden). Lead sees: none until step 2.

Step 2: CSS type toggle (Basketball / Soccer), title offsets, GOAL picker. Files: the CSS VS screen in
`decomp/src/mn/mnplayers/mnplayersvs.c` (hooks in section 2 rows 3 and 4). After: the CSS port (not started). Lead sees:
the title shows "Smashketball 1" or "Smashketball 2" and toggles.

Step 3: before-CSS and leave-CSS settings (section 2 rows 2 and 7). File: `decomp/src/mn/mnvsmode/mnremixvsmode.c` (new function
`mnRemixSmashketballBeforeCSS`, `...LeaveCSS`). Writes `gSCManagerTransferBattleState` fields for time mode, teams and score
target. The field names must be checked in the decomp before writing (not verified). Lead sees: the VS CSS opens with time
mode and team setup.

Step 4: CSS start check (exactly two teams) (section 2 row 5). File: the CSS start path in `mnplayersvs.c`. After step 2.
Lead sees: start is blocked with one or three teams.

Step 5: match start: stage id (0x5A or 0xD1), spawn table (section 3 `player_spawns`). Files: new
`decomp/src/sc/sccommon/scremixsmashketball.c` (start setup) called from `scvsbattle.c` at `scVSBattleStartBattle`.
After step 0. Lead sees: stage 0x5A loads and players spawn in the team positions.

Step 6: match init (scores, teams, routines) (section 2 row 9). File: `scremixsmashketball.c` (new function `...Setup`). After
steps 0 and 5. Lead sees: the match starts with zero scores.

Step 7: scoreboard HUD. Needs a decision: draw the digits through the stage model (vanilla path, needs the map joints) or
through an ImGui overlay (`port/remix/remix_overlay.h`). The lead decides. After step 6. Lead sees: scores and time at the
top of the stage.

Step 8: basket detection and scoring (section 4 items 1 to 7). Files: `itremixbasketball.c` (`itRemixBasketballCheckScored`,
line 177 prototype) and a new `decomp/src/gm/gmremixsmashketballscore.c` (score and winner-score tables, attribution). After
steps 1 and 6. Lead sees: a basket adds the score on the HUD (after step 7).

Step 9: match end and sudden death (section 2 rows 10, 11 and 12, `scVSBattleSetScoreCheckSuddenDeath` at scvsbattle.c:394 and
`scVSBattleStartSuddenDeath` at 579). After step 8. Lead sees: match ends at the target score; tie goes to sudden death.

Step 10: camera follow bounds for the ball (section 4 item 3). File: `decomp/src/gm/gmcamera.c`
(`gmCameraPlayerFollowFuncCamera`, line 894). After step 8. Lead sees: the camera widens to include the ball.

Step 11: crowd chant and crowd sounds on baskets (section 4 item 7; `ftPublicTryStartCall` in `decomp/src/ft/ftpublic.c`,
forced-chant bypass). After step 8. Lead sees: the crowd chants after three baskets in a row.

Step 12: spawn rule (neutral spawns always in Smashketball, `gmremixspawnsetup.c:200`). After step 0. Lead sees: spawns
follow the team positions even with the toggle off.

Step 13: results row and winner scores in TimedStock (section 6). After steps 8 and 9, and the results work (T5). Lead sees:
the results screen shows the Smashketball row.

Step 14: CPU ball AI (section 4 items 4 and 5). Depends on the CPU AI work (M7, `mk_9`). Not planned here. Lead sees: CPUs
move toward the ball.

Verification for each step: the remix smoke tests skill (`remix-smoke-tests`), a VS match in mode 4, a CSS screenshot, and the
score or time read from the game. The lead does the build and the game sign-off.

## 8. Risks and unknowns

- Hurtbox size bug (suspected): `ai_ball_attack_range` line 1237 comment says 200.0, but `lui t3, 4348` (decimal 4348 is 0x10FC)
  writes `0x10FC0000`, about 1e-29, not `0x43480000` (200.0). If so, the CPU range check uses an almost zero box. Check in game
  before porting the value. Not verified by running.
- Hard-coded RAM and joint offsets: `setup_` (lines 412-433) walks the map object's joint tree from `0x80046810`
  (`0x74`, `0x10`, `0x08`, `0x80`) to find the digit images. The port has no matching struct. High risk for step 7.
- Stack writes: `ai_ball_attack_range` writes into the caller's stack at `0x174` to `0x194` (lines 1238-1255). Only valid for
  the vanilla frame layout.
- Time vs score: the asm uses `Global.vs.time == 100` for dashes and `winning_score == 100` for "no winner", and the CSS
  writes both. Exactly when `Global.vs.time` is set to a real timer versus the score is not traced through the CSS. Nao
  verificado.
- `Hazards.add_item_` gets `Stages.id.DREAM_LAND` as the stage id argument (line 650, comment "original stage ID"). Unclear
  what the item code does with it. Nao verificado.
- One ball at a time: `basketball_main_` only spawns when none exists (line 627). The port's file-local state assumes this
  (`itremixbasketball.c:112`).
- Sudden death in `prevent_match_stats_reset_` skips code by fixed addresses (`0x800E7D74`, lines 1325-1326). Port must match
  the vanilla path exactly, or skip this hook.
- `winner_score_distribution` mixes two meanings in one table: index 0 is the attributed amount (line 885), and indexes 1 to 3
  are the unattributed amounts by team size (line 832). A port that splits them into two tables must keep the same values.
- The hook addresses are ROM addresses for the Remix build (`OS.patch_start(rom, ram)`). The decomp function name matches
  are not confirmed for any row of section 2. Where the name is not in the decomp, the table says "no match found".
- Not read: the stage model and the map joints for the scoreboard (section 5), the CSS VS screen in `mnplayersvs.c`, and the
  full vanilla `scvsbattle.c` (only the function list was read).
