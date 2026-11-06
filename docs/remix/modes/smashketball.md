# Smashketball

Remix VS mode 4 (`MNREMIX_VSMODE_SMASHKETBALL`, VS menu page 2). Asm source: `Smashketball.asm` (in `.toolchain/pkgs/smashremix-src/src`) plus
`items/Basketball.asm`, `Hazards.asm` (barrel and baskets of the stage), `Camera.asm`, `TimedStock.asm`, `Spawn.asm` and the CSS hooks of
`VsRemixMenu.asm` / `TwelveCharBattle.asm`.
Port code: `decomp/src/gm/gmremixsmashketball*.c` (match, score, scoreboard, camera), `decomp/src/it/itremix/itremixbasketball.c` (the ball),
`decomp/src/gm/gmremixmodecss.c`, `mn/mnplayers/mnremixcssmode.c`, `mnremixgoalpicker.c`, `gm/gmremixgameendrows.c`.
Tests: `tests/remix/test_smashketball_rules.c`, `test_css_vsmodes_rules.c`, `test_gameend_rows_rules.c`; `cmake --build build --target run_remix_mode_tests`.
Shared plumbing: [`../modes_base.md`](../modes_base.md).

## How the Remix does it

### Rules

Two teams, Home and Away (`Global.vs.teams` forced on, exactly two teams required to start), play basketball or soccer with one ball.
`Smashketball.type` (0 basketball, 1 soccer) is toggled on the CSS and chooses the stage: Smashketball (stage id 0x5A) or Soccer (0xD1).
A team wins when its score reaches `winning_score` (CSS goal picker, default 10, values 1-7, 10, 15, 20, 50, 99, 100; 100 means no goal). A basket
or goal is worth 1 point, 3 for the golden ball. The match runs on the Time rule: the VS menu time limit (restored at match start) ends it, and a
tied score at the end goes to sudden death (`TimedStock.asm` `prevent_sudden_death_`); any score difference ends sudden death.

Scoring (`basketball_main_`, `_scored`):

- Basketball: the ball goes from above the rim (baskets joint y minus `HOOP_OFFSET` 267) to below it while its x at hoop height is at least
  `HOOP_MIN` (3504) from the centre. Above the rim while passing clears `scoreable`; below while `scoreable` is set scores. Soccer: `|x|` of the
  ball at least 4288 (`0x45860000`) is a goal. The left hoop credits the Away team, the right hoop the Home team.
- Attributed basket (the ball's last owner is on the scoring team): `scores[port]` +1 per point, `winner_scores[port]` +6 per point, the owner's
  `consecutive_baskets` +1 (the other streaks reset). Three or more in a row force the crowd chant (`force_chant_`, knockback 160).
- Unattributed basket (no owner, or the owner is on the other team): every teammate gets `winner_score_distribution[teammates]` (6 for a team of
  one, 3 for two, 2 for three) times the points in `winner_scores`. The team score is the same in both cases.
- Effects: crowd sound 0x272 (cheer) or 0x267 (groan for an own-goal owner), basket explosion effect plus sound 0x118.
- `winner_scores` order the results (`TimedStock.asm` `_smashketball`); `scores` are the numbers shown by the results row.

### Match plumbing

| Asm | What it does |
|---|---|
| `before_css_setup_`, `leave_css_setup_`, `start_match_setup_` | Time rule, teams on, stage select off, goal 10; at start: stage id, `player_spawns` (port 1 always spawn 2, then home side fills 2, 0, 1 left to right and away side 3, 1, 0 right to left) |
| `setup_` | zeroes scores, builds team lists (first fighter's team is Home, the last other team seen is Away), finds the scoreboard digits in the stage model, colours them with `Color.tag` of the team (RED `0xED3636FF`, BLUE `0x4E4EE9FF`, GREEN `0x4EB94EFF`, YELLOW `0xFFDF1AFF`) and registers `basketball_main_` and `scoreboard_main_`; on the Smashketball stage in any other mode it only runs the time with dashes for the scores |
| `basketball_main_` | after 0x135 frames since the screen load, if no ball exists it is spawned (item 0x40, x 0, y 2048, 5632 when golden); reads the ball position (the owner's item joint while held) and detects the basket |
| `scoreboard_main_` | time from the battle state (`time_remain - 1` plus the 59 second rollover) or dashes when the match time is infinite; scores as two digits, 99 shown for anything larger |
| `check_match_end_` (`0x80113180`) | after each score a team at the goal wins (`GAMESET`); sudden death ends on any difference |
| `play_end_buzzer_` | queues FGM 0x431 after the announcer sound at the end (also on TIME UP) |
| `prevent_despawn`, `HRC.asm` `prevent_item_despawn_` | the ball's pickup wait is never decremented, so it never despawns |
| `camera_follow` | when the ball is not respawning the camera bounds grow to include it: padding 1000 on x and top, bottom bound always -1000 |
| `overtime` | `no_initial_damage_`: sudden death fighters start at 0% instead of 300%; `allow_overtime_` and `prevent_match_stats_reset_` keep the main battle state and the match stats across overtime |
| Hazards `_nba_jam` | the stage barrel and baskets joint |
| `Spawn.asm` | neutral spawns are always used in Smashketball, even with the Neutral Spawns toggle off; the two stages share the four spawn positions (x -3042, 3042, -1615, 1615) |
| CSS | `enable_toggling_mode_` (the game mode button flips Basketball / Soccer instead of teams; titles `0x2C18` "Smashketball 1" and `0x2E88` "Smashketball 2" in the game modes file), `use_goal_yellow_picker_` (GOAL image `0x27FC8`), `prevent_start_unless_two_teams_`, announcer FGM 776 |
| Results | `add_row_to_results_`: an extra "FGs" row (label image `0xE9E0`) with the baskets per port (limit 999) |
| AI | `ai_ball_behavior_` (CPUs target the ball height and x +-400 by team) and `ai_ball_attack_range` |

Music is `MIDI.id.NBA_JAM`. Remix-only stage files `STG_SMASHKETBALL_HEADER`, `STG_SMASHKETBALL`, `STG_SMASHKETBALL_BG` (0x991-0x993). The
sounds are `bounce`, `fire_bounce`, `spawn_buzz`, `end_buzzer`.

### HUD

The scoreboard is part of the stage model: four score digits (two per team) and four time digits, written as texture indexes into the joint image
structs (0-9, 10 for a dash). There is no text; the CSS uses title sprites.

## How the port does it

| Asm | Port |
|---|---|
| mode profile, stage id | Time rule, goal 10, teams on, stage select off in `gmremixmodecss.c`; `gmRemixSmashketballGetStage` (`nGRRemixStage_SMASHKETBALL` / `nGRRemixStage_SOCCER`) sets `gkind` in `gmRemixModeStartMatch` |
| spawns | `gmRemixSmashketballGetSpawnSlot` / `GetSpawnPos` (`gmremixsmashketball.c`), used by `gmremixspawnsetup.c` for every Smashketball match regardless of the toggle |
| `setup_` | `gmRemixSmashketballSetup` (called from `scVSBattleStartBattle` and the sudden death restart in `scvsbattle.c`): state `gGMRemixSmashketball` (`gmremixsmashketballstate.h`), `gmRemixSmashketballFindTeams`, baskets (`gmRemixSmashketballMoveBaskets`), scoreboard (`gmremixsmashketballboard.c`: `BoardSetup`, `BoardSetTeamColors`, `BoardUpdate`, `ShowDashScores`, `UpdateTimeOnly`), update process `gmRemixSmashketballProcUpdate`; outside the mode `gmRemixSmashketballSetupTimeOnly` handles the time-only board |
| `basketball_main_` | spawn after `GMREMIX_SMASHKETBALL_BALL_WAIT_FRAMES` (0x135) frames of the go status; `itRemixBasketballCheckHoop` detects the basket; `gmRemixSmashketballScore` (`gmremixsmashketballscore.c`) applies the attribution rules, streak, effects, cheer or groan; pure rules in `gmremixsmashketballrules.c` |
| chant | `ftPublicRemixForceCall` (`ft/ftpublic.c`, the `force_chant_` bypass), called with knockback 160 from `gmRemixSmashketballScoreOwner` |
| `check_match_end_`, sudden death | `gmRemixSmashketballCheckMatchEnd`; `gmRemixSmashketballPrepareSuddenDeath` (called from `scVSBattleSetScoreCheckSuddenDeath`) returns FALSE for unequal scores; `gmRemixSmashketballGetSuddenDeathDamage` (called from `scvsbattle.c`) gives 0% |
| `play_end_buzzer_` | `ifCommonBattleEndAddSoundQueueID` path in `ifcommon.c` queues `GMREMIX_SMASHKETBALL_FGM_END_BUZZER` (0x431) |
| `prevent_despawn` | `itRemixBasketballIsDespawnBlocked`, called from `itProcessProcItemMain` (`itprocess.c`) |
| `camera_follow` | `gmremixsmashketballcamera.c` (`gmRemixSmashketballCameraFollowBall`, rule `gmRemixSmashketballRulesFollowBall`), called at the end of the weapons loop of `gmCameraUpdateInterests` (`gmcamera.c`) |
| the ball | `itremixbasketball.c`: physics, bounce squash (hooks in `itmap.c`), fire state, hit multipliers (`itprocess.c`, `ftmain.c`), barrel respawn; `Fixed` camera mode of the stage in `gmremixcameramode.c` |
| CSS | `mnRemixCssModeToggleSmashketballType` (`mnremixcssmode.c`, stored as the CVar `gRemix.Modes.SmashketballType`, `remix_modes_smashketball_set_type`), title offset by type (`mnRemixCssModeTitleOffset`), announcer 776, `mnRemixCssModeTeamsAllowStart` (exactly two teams) hooked in `mnplayersvs.c`, GOAL picker image (`mnRemixGoalPickerImageOffset`) and values (`mnremixgoalrules.c`) |
| results | `gmRemixGameEndRowsGetValue` returns `gGMRemixSmashketball.scores[port]`; the order uses `gmRemixSmashketballGetWinnerScore` in `mnvsresults.c` |

### Deviations from the asm

- **Scoreboard time.** The asm shows `time_remain - 1` plus 59 tics because it draws before the timer updates; the port computes
  `(time_remain + 59) / 60` seconds after the update, a one tic difference.
- **Ball spawn wait.** The asm waits 0x135 frames from the screen load; the port counts the frames of the go status.
- **Mode type storage.** The Basketball / Soccer choice is a port CVar (also editable in the port menu, Remix > Modes) and `SSB64_SMASH_TYPE`
  overrides it; it is flipped on the CSS by the game mode button as in the asm.
- **Overtime.** As in the asm, the overtime runs on the main battle state: `gmRemixSmashketballPrepareSuddenDeath` only sets the time limit to infinite and
  `is_suddendeath` (`allow_overtime_`), and `scVSBattleStartScene` does not switch `gSCManagerBattleState` to the VS state; `ftParamInitPlayerBattleStats` keeps the
  place, falls, score, KOs, damage and combo totals while `gmRemixSmashketballKeepsMatchStats` (`prevent_match_stats_reset_`), so the results show the whole match.
  The scores survive because the setup does not clear them while `is_suddendeath` is set. Deviation: the time limit before the overtime is put back when the next
  match sets up, so a salty runback does not start with an infinite clock.
- **Fixed camera.** `Camera.asm`'s fixed Smashketball camera is only used with Camera Mode FIXED (`gmremixcameramode.c`); the default mode uses the follow
  camera plus `camera_follow`.
- **CPU range constant.** `ai_ball_attack_range` loads `lui t3, 4348` (decimal, `0x10FC0000`) where its comment says 200.0; this is a likely asm bug
  and matters only if the CPU code is ported.

Audit ledger (every item closed): `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

Run from the repository root; scripts are in `.claude/skills/remix-smoke-tests/scripts/` and results land in `build/agent-shots/vs/<name>/`.

- Shortcuts: `SSB64_REMIX_MODE=smashketball`, `SSB64_SMASH_TYPE=basketball|soccer`, `SSB64_SMASH_GOAL=<score>`, `SSB64_SMASH_BALL_X=<x>` (drops the ball
  at that x), `SSB64_VS_TIME=<minutes>`, `SSB64_VS_TEAMS=1` (ports 1-2 team 0, ports 3-4 team 1), `SSB64_AUDIO_FGM_LOG=1`.
- Scoreboard colours: `SSB64_REMIX_MODE=smashketball SSB64_VS_STAGE=90 SSB64_VS_TIME=1 vs.sh 0,8 inputs/none.txt s_board 600`, frame 200: home digits red,
  away blue; add `SSB64_VS_TEAMS=1` with `0,8,6,5` for 2 v 2. With `SSB64_VS_TIME=1` the board counts 00:04..00:00, TIME UP appears and a tie goes to
  SUDDEN DEATH.
- Time-only board: `SSB64_VS_STAGE=90 SSB64_VS_TIME=1 vs.sh 0,8 inputs/none.txt s_timeonly 600` without `SSB64_REMIX_MODE`: the time counts and the four
  score digits show the dash.
- Camera: `SSB64_SMASH_BALL_X=2500` drops the ball far from the fighters; the camera widens to keep it in view.
- End buzzer and chant: `SSB64_SMASH_GOAL=1 SSB64_SMASH_BALL_X=<x past a hoop> SSB64_AUDIO_FGM_LOG=1`: after the goal the log has `[fgm] play id=` for the
  announcer followed by `id=1073` (0x431), none before the game set; with `SSB64_VS_TIME=1` the buzzer also plays after TIME UP. The chant needs three
  baskets in a row by the same owner.
- Four players: `SSB64_REMIX_MODE=smashketball SSB64_VS_TEAMS=1 vs.sh 0,8,6,5 inputs/none.txt s_four 700`: port 1 spawns at x -1615, port 2 on the
  home side, ports 3 and 4 on the away side (+1615, +3042); the board shows the two team colours.
- A ball left on the court for longer than the item pickup wait must stay (no sparkle and respawn).
- The "--:--" scoreboard appears only for an infinite VS time; a 2200 frame run with the default 8 minute tournament time shows 08:00 and cannot end by time.
