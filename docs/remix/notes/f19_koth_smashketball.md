# F19: King of the Hill and Smashketball in the match (M11 Fase 5)

Worker note, 2026-11-05. Nothing was built or run (rules of the Fase 5 workers): every item below is checked by reading the asm
and the port, with the unit tests compiled alone. The lead builds and validates. Asm files: `KingOfTheHill.asm`,
`Smashketball.asm` (plus `Hazards.asm`, `TimedStock.asm`, `Camera.asm`).

## 1. Smashketball.asm scopes and port status

| Asm scope (line) | Port | State |
|---|---|---|
| `before_css_setup_` (139), `leave_css_setup_` (177) | `gmRemixModeBeforeCSS/LeaveCSS` (`gmremixmodecss.c`) | ported earlier |
| `start_match_setup_` (211): stage id, `player_spawns`, `spawn_table` | `gmRemixModeStartMatch`, `gmRemixSmashketballGetSpawnSlot` | ported earlier, re-read for 3 and 4 players: same fill order (P1 always S3, home left to right 2,0,1, away right to left 3,1,0) |
| `setup_` (324): teams, scores, `scoreboard`, digit colours | `gmRemixSmashketballSetup`, `gmremixsmashketballboard.c` | digits ported earlier; **digit colours added** (`gmRemixSmashketballBoardSetTeamColors`, `MObj.sub.primcolor` = asm `+0x58`) |
| `setup_` time only flag (Smashketball stage in another mode) | `gmRemixSmashketballSetupTimeOnly` | **added**: scoreboard time runs, the four score digits show the dash |
| `prevent_despawn` (476) | `itRemixBasketballIsDespawnBlocked`, hook in `itProcessProcItemMain` (`itprocess.c`) | **added**: the ball in Smashketball never counts its pickup wait down |
| `camera_follow` (501) | `gmremixsmashketballcamera.c`, hook at the end of the weapons loop of `gmCameraUpdateInterests` (`gmcamera.c`), rule `gmRemixSmashketballRulesFollowBall` | **added**: padding 1000 on x and top, bottom bound always -1000, nothing while the ball respawns (item `0x1E8`) |
| `basketball_main_` (613): ball spawn, held position, rim and goal detection, `_scored` | `gmRemixSmashketballProcUpdate`, `itRemixBasketballCheckHoop`, `gmRemixSmashketballScore` | ported earlier (soccer goal |x| 4288 = `0x45860000`) |
| `_scored` chant: streak of 3 or more calls `0x80164AB0` with a0 = `0x1234`, knockback 160 | `ftPublicRemixForceCall` (`ftpublic.c`), called from `gmRemixSmashketballScoreOwner` | **added** (`force_chant_` at `0x80164AE0` skips the damage, wait and repeat checks) |
| `scoreboard_main_` (955) | `gmRemixSmashketballBoardUpdate` | ported earlier. Differs by one tic, see 4 |
| `check_match_end_` (1067) | `gmRemixSmashketballCheckMatchEnd` | ported earlier (goal 100 = none, sudden death on any difference) |
| `play_end_buzzer_` (1108) | `ifCommonBattleEndAddSoundQueueID` (`ifcommon.c`) | **fixed**: the port played FGM 0x431 at once on a goal only; the asm queues it after the announcer sound, so it also plays on TIME UP. `GMREMIX_SMASHKETBALL_FGM_END_BUZZER` moved to `gmremixsmashketball.h` |
| `ai_ball_behavior_` (1152), `ai_ball_attack_range` (1210) | none | open: CPU AI (Fase 6, `ftcomputer`) |
| `overtime` x3 (1264-1331) | `gmRemixSmashketballPrepareSuddenDeath`, `GetSuddenDeathDamage`, `scvsbattle.c` | ported earlier; seen in `b10_sk_time` (TIME UP, SUDDEN DEATH, GO) |
| `css.*` (1337-1469) type toggle, GOAL picker, two teams at START | CSS worker | out of scope |

### The "--:--" scoreboard and the match not ending by time

Checked, no port defect. The asm gives the match the VS menu time (`start_match_setup_` sets `Global.vs.time` from
`VsRemixMenu.global_time`) and the score target is a separate value. The port does the same: `gmRemixModeStartMatch` restores
the VS time, 8 minutes in the tournament default (`scremixtournament.c`), so a smoke run of 2200 frames shows 08:00 and cannot
end by time. The dash appears only for an infinite time (and in sudden death, where the asm sets the time to 100). The existing
evidence: `build/agent-shots/vs/b10_sk_time` (`SSB64_VS_TIME=1`): the board counts 00:04..00:00, TIME UP, tie goes to SUDDEN DEATH.
`gmRemixSmashketballPrepareSuddenDeath` returns FALSE for a non-tied score (asm `_no_sudden_death`), so the normal results run.

## 2. KingOfTheHill.asm scopes and port status

| Asm scope | Port | State |
|---|---|---|
| `before_css_setup_`, `leave_css_setup_`, `start_match_setup_`, `allow_3_digits_for_picker_` | `gmremixmodecss.c`, `mnremixgoalpicker.c` | ported earlier |
| `setup_` (1530-1700) state, meters, indicators | `gmRemixKothSetup`, `gmremixkothhud.c` | ported earlier (bars, frame sprite, DK position) |
| `hill_update_routine_`, `set_random_hill_`, `get_random_line_id_`, `is_hill_outside_blast_zone_` | `gmremixkoth.c`, `gmremixkothhill.c` | ported earlier |
| `track_score_` (1172): king, buffer, scoring | `gmremixkothking.c`, `gmremixkothrules.c` | ported earlier |
| `track_score_` `_apply_gfx_routines`: fighter overlay | `gmremixkothglow.c` | **added** (see 3) |
| `check_match_end_`, `delay_respawn_`, TimedStock branch, `prevent_sudden_death_` | `gmremixkoth.c`, `gmremixtimedstock.c` | ported earlier |
| `render_ground_gfx_` (461): prim colour | `gmremixkothdraw.c` | **fixed**: colours and gray formula (see 3) |
| `create_ground_gfx_` (517), `update_ground_gfx_` (400), `koth_ground_gfx_struct` | `gmremixkothmodel.c` (`gmRemixKothModelSync`) | **ported (follow-up)**: one effect object per segment made from the file 0x14F3 (see 3b); `gmremixkothdraw.c` keeps the colour and a flat strip fallback when the file is missing |
| indicator object (`koth_indicator_gfx_struct`, `update_indicator_gfx_`, `create_ground_gfx_` 709-790) | `gmremixkothmodel.c` (`gmRemixKothModelPlaceMarker`) | **ported (follow-up)**: the marker (crown over an arrow) |
| `get_surface_center_coords_` (797) | none | dead code in the asm (never called) |
| `ai_hill_behavior_` (1711) | none | open: CPU AI (Fase 6) |
| `update_indicators_` | `gmRemixKothHudUpdate` | ported earlier |

## 3. What changed in KOTH

- Strip colours were invented (`0xE0,0x40,0x40`...). Now `gmRemixKothRulesTag/TagColor` give the real `Color.tag` values
  (RED `0xED3636FF`, BLUE `0x4E4EE9FF`, GREEN `0x4EB94EFF`, YELLOW `0xFFDF1AFF`, GRAY `0xACACACFF`). Free-for-all order is red, blue,
  yellow, green by port, a CPU in free-for-all is gray (`use_cpu_color`), teams use red, blue, green, yellow and never gray. The
  CPU test is now by team (the asm indexes `use_cpu_color` by team) instead of by port.
- Contested gray: the asm subtracts `2 * buffer` from `0xAA` per channel (`render_ground_gfx_` `_set_gray_color`; it shifts
  `buffer * 2`, not `* 3`, although the comment says 3): dark `0x6E` at buffer 30 and lighter as it runs out. The port had it
  reversed (`0x60 + 3 * buffer`). `gmRemixKothRulesContestedGray`.
- The fallback flat strip (only when 0x14F3 is missing) uses the prim colour at alpha `0xFF` and vertex alpha `0xBF` at the floor
  fading to `0x20` at 128 units (read from the end quads of the model).
- Fighter glow: `track_score_` gives each fighter that scores the pulsing overlay `GFXRoutine.KOTH_*` when it has no routine
  (`0x0A28` empty = no colour animation override in the port). `gmRemixKothGlowApply` uses the same pattern as
  `grremixspawnedfear.c`: `gmRemixColScriptsSetOverride` plus `ftParamCheckSetFighterColAnimID`. Colour animation ids: KOTH_RED
  `0x80`, BLUE `0x81`, GREEN `0x82`, YELLOW `0x83` (position in the `add_gfx_routine` list, `BALL_FIRE` is `0x84`). KOTH_GRAY is
  not in that list (the asm writes its address to the fighter), so `gmRemixColScriptsRegisterRoutine(REMIX_SYM_GFXRoutine__KOTH_GRAY)`
  appends it as a new id at run time (priority 100, as the other KOTH routines).

## 3b. The hill model (follow-up, after the reference capture)

The reference emulator capture (`build/ref-shots/koth/smash_remix-003..006.png`, Peach's Castle) shows a gray marker (crown over a
down-pointing arrow) above the tower top and a faint slab over the platform. The port now draws the real model:
- `gmremixkothmodel.c`: `efManagerMakeEffectForce` with two `EFDesc` like the asm `koth_ground_gfx_struct` and
  `koth_indicator_gfx_struct`: flags `0x04` (custom joint tree), display link `0x0B`, matrix kinds `0x1C`, `o_dobjsetup` `0x400`
  (strip piece: root, joint 1, joint 2-0 with joint 3 below it, joint 2-1) and `0xF50` (marker: root and texture joint, a 400 x 400
  textured quad). The file comes from `ifRemixSpriteGetFile(0x14F3)` (now public, shared with the meter frame sprite); the loader
  resolves the display list tokens.
- Like the asm, `func_run` is replaced right after the creation (the default effect routine would eject an object without
  animation) by a routine that runs `gcPlayAnimAll` (asm `0x8000DF34`); `proc_display` is the asm `render_ground_gfx_`: prim colour of
  the owner (`gmRemixKothDrawGetPrimColor`, rules unchanged) then `gcDrawDObjTreeForGObj` (`0x80014038`).
- Placement, per segment `i` of `n = vertices - 1` (`create_ground_gfx_ _update_position`): joint 1 at the start vertex, joint 2-0 at
  the end vertex minus the start (the asm `0x1C` and `0x20` words), joint 3 (left end cap) shown only on the last piece, joint 2-1
  (right end cap) only on the first, extra pieces hidden (GObj hidden flag, asm `0x7C`). Marker: x halfway between the first and the
  last vertex, y 500 above the start of the segment that holds that x plus the slope times the distance (asm `_loop_find_mid_line`).
- `gmRemixKothModelSync` runs every frame from the KOTH update process, so a moving platform and a new hill are followed.

### 3c. Validation of `b11_koth_pc2` (analysis only)

- **Marker colour is correct.** The reference marker is exactly `(110,110,110)` in all four captures: that is the contested gray
  `0xAA - 2 * 30 = 0x6E` (`render_ground_gfx_` with the contest buffer at 30), because Ness and Bowser both stand on the hill line
  (the whole bridge is one floor line of 7 vertices) and two teams on the hill keep the buffer at 30. In the port capture the hill is
  empty (Mario on the lower platform, Kirby in the air), so the king is -1 and the prim colour is white: the marker is `(253,253,253)`.
  The marker DL combiner is `TEXEL0 * PRIM` (decoded), so it shows the prim colour exactly. To compare, put both fighters on the bridge.
- **The stray diamond-plate quad is not drawn by the KOTH model.** Facts: the log shows exactly 7 effect objects made at frame 390
  (6 strip pieces and the marker, nothing else); the strip DLs (`0x40`, `0x128`, `0x240`, `0x340` in the file) have the combiner
  `RGB = PRIM`, `A = SHADE` and `G_TEXTURE` off, so they cannot draw a textured plate; the marker DL is one 400 x 400 quad at z 0.
  The same model on Dream Land (`b11_koth_ffa2`, frames 500 to 1100) shows the slab and marker and no plate, and the plate also
  exists in `b11_koth_pc` (older run). It moves across the sky from the right (frames 500 to 520) to the left (760 to 880) like a
  stage object, and the reference shows a gold object at the right end of the bridge where the port has nothing, so it is most likely
  a Peach's Castle stage object (the bumper area) that the port draws wrong, which only shows when the KOTH camera zooms out. Check:
  `SSB64_VS_STAGE=0 vs.sh 0,8 none.txt pc_plain2 900` with `SHOTS=480,520,560,600` and no `SSB64_REMIX_MODE`, then the same with
  `SSB64_REMIX_MODE=koth SSB64_KOTH_NO_MODEL=1` (new switch: the flat strip, no model file, no effect objects). If the plate is in both,
  it is the stage.
- How the model works (decoded from the file, for the record): the strip body is not skinned by one DL. The joint 1 DL loads 4 vertices
  (the start cap, vertex buffer 0..3) and draws nothing; the joint 2-0 DL loads 4 more (buffer 4..7, at the end vertex) and draws the body
  triangles between buffers 0..7, so the joint 1 DL must run right before it (it always does, a piece is a joint tree drawn in order).

## 4. Deviations

- **KOTH hill model**: ported through the model (3b). Differences left: the world vertex positions of the line are used with the top
  joint at the origin (the asm keeps raw vertices and follows the platform object with the top joint), which gives the same place for
  a moving platform; the marker is recomputed every frame instead of once per hill. The slope of the marker height uses |dx| as the asm
  does (so it is mirrored on a segment that runs right to left). The old flat wall stays only as the fallback when 0x14F3 is missing.
- **Scoreboard time**: the asm shows `time_remain - 1` plus 59 tics, the port `(time_remain + 59) / 60` seconds: a one-tic
  difference (the asm draws the board before the timer updates; the port runs after).
- **Ball spawn wait**: the asm waits 0x135 frames since screen load, the port counts frames of the Go status.
- **Smashketball CPU**: `ai_ball_behavior_` and `ai_ball_attack_range` are not ported (Fase 6). `ai_ball_attack_range` line 1237 loads
  `lui t3, 4348` (decimal, `0x10FC0000`) where the comment says 200.0: a likely asm bug to check before porting.
- The fixed Smashketball camera of `Camera.asm` is only used with Camera Mode FIXED (already in `gmremixcameramode.c`); the default
  mode uses the follow camera plus `camera_follow`.

## 5. Files

New: `gm/gmremixkothglow.c/.h`, `gm/gmremixsmashketballcamera.c/.h`, `gm/gmremixkothmodel.c/.h` (follow-up).
Edited: `gm/gmremixkothdraw.c/.h` (colour export, fallback), `gm/gmremixkoth.c` (sync call), `if/ifremixsprite.c/.h` (public file getter), `gmremixkothking.c`, `gmremixkothrules.c/.h`, `gmremixsmashketball.c/.h`,
`gmremixsmashketballboard.c`, `gmremixsmashketballscore.c`, `gmremixsmashketballrules.c/.h`, `gmremixcolscripts.c/.h`,
`gm/gmcamera.c` (hook), `it/itremix/itremixbasketball.c/.h`, `it/itprocess.c` (hook), `ft/ftpublic.c/.h` (forced chant),
`if/ifcommon.c` (buzzer), `port/remix/remix_hud_text.h` (slot range 9..13 reserved, none used).
Tests: `tests/remix/test_koth_rules.c` (+4: tags, colours, contested gray), `test_smashketball_rules.c` (+4: team colours, chant
streak, camera bounds). Compiled alone: 87 checks, 0 failed. Syntax check OK on every touched C file.

## 6. In-game recipe for the lead

Build first (`cmake -S` again for the two new files). Scripts under `.claude/skills/remix-smoke-tests/scripts`, `inputs/none.txt`.

- KOTH hill colours: `SSB64_REMIX_MODE=koth SSB64_KOTH_LINE=2 SSB64_KOTH_GOAL=6 SSB64_VS_STAGE=0 vs.sh 0,8,6 none.txt k19_ffa 1500`
  (the same setup that showed the red strip). Frames 130 (white hill, no king), 160 to 200 (dark gray that lightens while the
  buffer runs, frame ~190), 250 (Mario red `ED3636`), dense shots `SHOTS=$(seq -s, 300 4 380)` for the pulse of the fighter (overlay
  alpha `0x88` to `0x22`, 20 frames each). Expect the slab over the platform and the marker (crown over an arrow) above the middle of
  the line. Reference compare: `SSB64_REMIX_MODE=koth SSB64_VS_STAGE=0 vs.sh 11,9 none.txt k19_pc 900` (or `0,8`) against
  `build/ref-shots/koth/smash_remix-003..006.png`, the hill on the tower top (`SSB64_KOTH_LINE=<id>` to force it). If the marker or slab
  is missing check the log for `koth: model file` or `the model piece has no joints`.
- KOTH CPU gray: put the hill under a CPU (try other `SSB64_KOTH_LINE` ids) in free-for-all: strip and glow gray `ACACAC`. The
  log has `[remix-gfx]` lines; the gray id is created on first use (a fourth line is not logged, check the colour on screen).
- KOTH teams: add `SSB64_VS_TEAMS=1` with four fkinds (`0,8,6,5`: P1+P2 team 0 red, P3+P4 team 1 blue): hill red when team 0 holds it,
  blue for team 1, gray while contested, bars by team.
- Smashketball board colours: `SSB64_REMIX_MODE=smashketball SSB64_VS_STAGE=90 SSB64_VS_TIME=1 vs.sh 0,8 none.txt s19_board 600`
  frame 200: home digits red (P1 team 0) and away digits blue; `SSB64_VS_TEAMS=1` with `0,8,6,5` for 2 v 2.
- Camera: `SSB64_SMASH_BALL_X=2500` drops the ball far from the fighters; compare frames 330 to 420 with the old build: the camera
  widens to keep the ball in view (`camera_follow`).
- End buzzer and chant: `SSB64_SMASH_GOAL=1 SSB64_SMASH_BALL_X=<x past a hoop> SSB64_AUDIO_FGM_LOG=1`: after the goal the log has
  `[fgm] play id=...` for GAME (announcer) followed by `id=1073` (0x431) from the queue, and no `1073` before the game set. With
  `SSB64_VS_TIME=1` the buzzer also plays after TIME UP. The chant needs three baskets in a row by the same owner (a goal 20
  with human input); the `[fgm]` log shows the cheer id of `nSYAudioVoicePublicCheer` without the 100% damage and 1200 frame wait.
- Time only board: `SSB64_VS_STAGE=90 SSB64_VS_TIME=1 vs.sh 0,8 none.txt s19_timeonly 600` without `SSB64_REMIX_MODE`: time counts, the four
  score digits show the dash.
- Despawn: a ball left on the court for longer than the item pickup wait must stay (no sparkle and respawn).
- 3 and 4 players: `SSB64_REMIX_MODE=smashketball SSB64_VS_TEAMS=1 vs.sh 0,8,6,5 none.txt s19_four 700`: spawns P1 x -1615 side (S3), P2 S1,
  P3 and P4 on the away side (+1615, +3042); the board shows the two team colours.
