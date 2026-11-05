# King of the Hill

Remix VS mode 3 (`MNREMIX_VSMODE_KOTH`, VS menu page 2). Asm source: `KingOfTheHill.asm` (in `.toolchain/pkgs/smashremix-src/src`) plus its
hooks in `VsRemixMenu.asm`, `TimedStock.asm`, `TwelveCharBattle.asm` (CSS header and announcer) and `GFXRoutine.asm`.
Port code: `decomp/src/gm/gmremixkoth*.c` (state, hill, king, strip model, meters, fighter glow), `decomp/src/gm/gmremixmodecss.c`,
`decomp/src/mn/mnplayers/mnremixgoalpicker.c`, `mnremixgoalrules.c`, `decomp/src/gm/gmremixgameendrows.c`.
Tests: `tests/remix/test_koth_rules.c` (king rules, meter positions, colours), `test_css_vsmodes_rules.c` (goal picker, title and announcer),
`test_gameend_rows_rules.c`, `test_timedstock_rules.c`; run `cmake --build build --target run_remix_mode_tests`. Smoke recipes at the end. Shared
mode plumbing (mode selection, CSS profiles, Timed Stock, HUD text) is in [`../modes_base.md`](../modes_base.md).

## How the Remix does it

### Rules

A floor line of the stage is the hill. It is picked at random and moves every 20 to 40 seconds. A team (the port in free-for-all) that stands
on the hill with nobody else, and has served a 30 frame contest buffer, is the king and gains one frame of score per frame. When its score
reaches the goal (the CSS "goal" value in seconds, default 30, times 60) a win timer starts: 210 units, +2 per frame in free-for-all (about
1.75 s) or +1 per frame in team battles (about 3.5 s). When it fills, the match ends (`check_match_end_`). The match runs on the Time rule
(`Global.vs.game_mode` forced to 1 in `before_css_setup_`), so the time limit of the VS menu still ends it (that value is restored at match
start); sudden death is off for KOTH (`TimedStock.asm` `prevent_sudden_death_`). The results ordering uses the frames on the hill
(`TimedStock.asm` `calculate_time_score_` returns `KingOfTheHill.scores[port]`). The only selector is `VsRemixMenu.vs_mode_flag == 3`.

### Per-frame logic (`KingOfTheHill.asm`)

All of it runs only while `Global.vs.game_status == 1` (go), in the order of registration: `hill_update_routine_`, `track_score_`,
`update_indicators_`.

- `hill_update_routine_`: while `timer != 0` it counts down. If the hill strip is hidden (display flag set) for 120 frames, or the hill leaves the
  blast zone, or the timer ends, a new hill is chosen: `timer = 1200 + random(0..1199)` and `set_random_hill_`. The first hill is picked 120 frames
  into the match.
- `set_random_hill_`: up to 3 tries. Each try `get_random_line_id_` picks a random line of the collision line groups; surface materials 0x00-0x06,
  0x0C-0x0E, 0x11 and 0x1F-0x23 are valid, the damaging and special ones (0x07-0x0B, 0x0F-0x10, 0x12-0x1E) are not. A try is rejected when the line
  is outside the blast zone, the strip would be hidden, or the line is the current hill. On success: sound 0xA2, `contested_buffer = 0`.
- `track_score_`:
  1. A fighter is on the hill when its floor line id is the hill line, it is grounded and not captured. If it holds another fighter, it is eligible
     only when the held one is on its own team; otherwise the hill is contested (candidate -2).
  2. King candidate: -1 none; the first eligible team on the hill sets it; a second team or a contested holder makes it -2.
  3. Buffer: when the candidate is not -2 and the buffer is non-zero, the buffer decrements and the result is -2 (contested); when it is -2 the
     buffer is set to 30 each frame. So a team that takes the hill (or retakes it after a contest) counts as contested for 30 frames first. A king
     that leaves with the buffer at 0 loses the crown that frame.
  4. When the king changes: sound 0x115 (gained, king >= 0) or 0xA9 (lost or contested).
  5. Scoring, per fighter whose team is the king, on the hill and eligible: while `team_scores[team] != target` the fighter's `scores[port]` and the
     team score go up by one **per eligible fighter** (two teammates on the hill raise the team score by 2 per frame); at the target the team's
     `winner_times` goes up once per frame (+2 FFA, +1 teams). Every fighter not on the king's team has its `winner_times` reset to 0.
  6. A scoring fighter with an empty `0x0A28` gfx routine slot gets the pulsing colour overlay of its colour (`GFXRoutine.KOTH_RED/BLUE/GREEN/YELLOW`,
     `KOTH_GRAY` for a CPU in free-for-all; teams never use gray).
- `check_match_end_` (hook `0x80113180`, called from `VsRemixMenu.asm`): for each of the 4 `winner_times` entries, at 30, 90 and 150 it plays the
  3 / 2 / 1 announcer (`FGM.announcer.fight.*`) and returns (one beep per frame; the other entries are not checked that frame); at >= 210 each
  fighter of the king team gets one more score frame and the game-set routine runs. Any other case falls through to the vanilla time check.
- `update_indicators_`: bar width = `trunc(score / target * 40)` px; when full, a white flash rectangle pulses alpha 0 to 100 in steps of 5.
- `delay_respawn_`: the rebirth wait is 120 frames instead of 45, and the fighter's camera is turned off when the wait equals 75.
- `ai_hill_behavior_`: a CPU targets the hill indicator (x of the indicator joint, y minus 500) and ignores items and tornados.
- `get_surface_center_coords_` is defined but never called.

### State

| Name | Meaning |
|---|---|
| `time`, `teams` | goal in seconds and team flag, saved between CSS visits (`before_css_setup_`, `leave_css_setup_`, `start_match_setup_`) |
| `scores[4]`, `team_scores[4]`, `winner_times[4]` | frames on the hill per port, per team, and the win timer per team |
| `hill_line_id`, `timer`, `king`, `contested_buffer` | current hill (-1 none), frames to the next hill, king (-1 none, -2 contested, else team or port), contest buffer |
| `use_cpu_color[4]`, `indicators`, `indicator`, `gfx` | CPU colour flags, per-port meters, the hill marker and the first strip object |

Constants: `DELAY` 120, `CONTESTED_BUFFER` 30, `WIN_FRAMES` 210, `ALPHA_DELTA` 5, `ALPHA_DELTA_MAX` 100. Files: `File.KING_OF_THE_HILL_GFX` (id 0x14F3, Remix
only) holds the strip pieces, the marker and the meter frame (image at +0x0AA8).

### Colours and HUD

- Strip colours (`render_ground_gfx_`): white with no king; while contested a gray of `0xAA - 2 * buffer` per channel (dark `0x6E` at buffer 30,
  lighter as it runs out); otherwise the king's `Color.tag`: RED `0xED3636FF`, BLUE `0x4E4EE9FF`, GREEN `0x4EB94EFF`, YELLOW `0xFFDF1AFF`,
  GRAY `0xACACACFF`. Free-for-all order by port is red, blue, yellow, green with a CPU gray; teams use red, blue, green, yellow.
- Meters: one per port (free-for-all) or team at x = 40 + 70 * port, frame image at y 180 (x - 3), bar and flash at y 188, 40 x 5 px. With the
  Dragon King HUD toggle on, or a Dragon King stage with the toggle on Auto, they move to y 8 / 16. Bar colours `colors_bar` (C96A6AFF, 7E7EC9FF,
  B2B244FF, 4EAD4EFF, gray 888888FF for a CPU) and `team_colors_bar` (C96A6AFF, 7E7EC9FF, 4EAD4EFF, B2B244FF).
- CSS: header image at offset `0x29A8` of the game modes file, announcer FGM 775 on entry, a 3-digit goal picker (values 5, 10, 15, 20, 25, 30, 45,
  60, 90, 120, 240, 480, 960; x shifts at 100 and over).
- Results: an extra "Times" row (label image at `0xE648`) with `M:SS` per port, drawn at y 124 with the place line pushed down 16 and delayed
  20 tics (`VsRemixMenu.asm` `add_row_to_results_`).

## How the port does it

| Asm | Port |
|---|---|
| mode selection, `before_css_setup_`, `leave_css_setup_`, `start_match_setup_` | `gmRemixModeBeforeCSS/LeaveCSS/StartMatch` with the KOTH profile (Time rule, goal 30, teams saved) in `gmremixmodecss.c` |
| `setup_` | `gmRemixKothSetup` (`gmremixkoth.c`), called from `scVSBattleStartBattle` (`scvsbattle.c`): resets `gGMRemixKoth` (`gmremixkothstate.h`), registers `gmRemixKothProcUpdate`, makes the strip model and the meters |
| `hill_update_routine_`, `set_random_hill_`, `get_random_line_id_` | `gmRemixKothUpdateHill` / `gmRemixKothPickHill` (`gmremixkoth.c`) and `gmremixkothhill.c` (`gmRemixKothHillPickLine`, material filter, blast zone test, 3 tries) |
| `track_score_` | `gmRemixKothTrackScore` (`gmremixkothking.c`); the pure king and buffer rules are `gmRemixKothRulesCandidate` / `gmRemixKothRulesSettle` (`gmremixkothrules.c`); the fighter test is `gmRemixKothIsOnHill` (floor line id, grounded, not captured) |
| `_apply_gfx_routines` | `gmRemixKothGlowApply` (`gmremixkothglow.c`): sets a fighter colour animation override (ids `0x80..0x83` for red, blue, green, yellow; the gray id is registered at run time with `gmRemixColScriptsRegisterRoutine`) |
| `check_match_end_` | `gmRemixKothCheckMatchEnd` (`gmremixkoth.c`): countdown at 30, 90, 150, end at 210, then `ifCommonAnnounceEndMessage` |
| `update_indicators_`, `setup_` meters | `gmRemixKothHudSetup/Update` (`gmremixkothhud.c`): the frame is a native sprite (`IFREMIX_SPRITE_KOTH_METER` from file 0x14F3), the bar and flash go through `remix_hud_bar_put`; positions and colours from `gmremixkothrules.c` |
| `create_ground_gfx_`, `render_ground_gfx_`, `update_ground_gfx_`, indicator | `gmremixkothmodel.c` (`gmRemixKothModelSync` every frame) builds effect objects from file 0x14F3 like `koth_ground_gfx_struct` and `koth_indicator_gfx_struct`; the prim colour is `gmRemixKothDrawGetPrimColor` (`gmremixkothdraw.c`), which also keeps a flat strip fallback when the file is missing |
| `delay_respawn_` | `gmRemixKothGetDeadWait` / `gmRemixKothDeadUpdateCamera`, called from `ftcommondead.c` |
| TimedStock `calculate_time_score_`, `prevent_sudden_death_` | `mnvsresults.c` score source (`gmRemixKothGetScore`) and `gmRemixTimedStockAllowsSuddenDeath` (`gmremixtimedstock.c`, called from `scVSBattleSetScoreCheckSuddenDeath`) |
| picker values and 3 digits | `mnRemixGoalPickerNext/Prev/NumberX` (`mnremixgoalpicker.c`, tables in `mnremixgoalrules.c`), hooked in `mnplayersvs.c`; a press on the picker toggles goal and Time (`showing_time`) |
| header and announcer | `mnRemixCssModeTitleOffset`, `mnRemixCssModeAnnounce` (`mnremixcssmode.c`, tables in `mnremixcssmoderules.c`) |
| results row | `gmRemixGameEndRowsHasRow/GetValue` (`gmremixgameendrows.c`) read by `mnvsresults.c`; numbers and layout in `gmremixgameendrowsrules.c` |

### Deviations from the asm

- **Team score rate.** The asm raises `team_scores` once per eligible fighter on the hill, so two teammates fill the team bar twice as fast. The
  port raises the team score once per team per frame (`gmRemixKothScoreKing`, `team_scored[]`) while each fighter still gets its own `scores[port]`
  point; the results row and the end timer are unaffected, only the bar speed of a team with two fighters on the hill differs.
- **Hill visibility and blast zone.** The asm tests the hill strip display flag (hidden for 120 frames) and the indicator position against the
  blast bounds. The port has no strip display flag to read: a hill is "hidden" when its collision is switched off (`gmRemixKothHillLineIsUsable`), and
  it leaves play when any vertex of the line is outside the map bounds (`gmRemixKothHillIsInsideBlastZone`).
- **Model placement.** The asm keeps raw vertices and follows a moving platform with the top joint; the port places the pieces on the world
  vertices every frame (`gmRemixKothModelSync`) with the top joint at the origin, which lands in the same place. The marker is recomputed every
  frame instead of once per hill.
- **Key.** The port keys scores by `fp->team` (equal to the port in free-for-all); the HUD keys by `players[port].team` in team battles.
- **Fighter glow.** The asm stores the routine address in the fighter at `0x0A28`; the port installs it as a colour animation override that
  restarts when the animation ends.
- **Dead code.** `get_surface_center_coords_` is not ported.

Open gaps: `docs/remix/patch_audit.md`, group g8.

## Tests and smoke recipes

Unit tests: `cmake --build build --target run_remix_mode_tests` (see `tests/remix/CMakeLists.txt`). In-game recipes use
`.claude/skills/remix-smoke-tests/scripts/vs.sh <fkinds> <input> <name> [frames]` from the repository root (results land in `build/agent-shots/vs/<name>/`).

- Shortcuts: `SSB64_REMIX_MODE=koth` forces the mode; `SSB64_KOTH_GOAL=<seconds>` replaces the CSS goal; `SSB64_KOTH_LINE=<floor line id>` makes the
  first hill that line; `SSB64_KOTH_NO_MODEL=1` draws the flat strip without the model file; `SSB64_VS_TEAMS=1` puts ports 1-2 on red and 3-4 on blue.
- Colours and glow: `SSB64_REMIX_MODE=koth SSB64_KOTH_LINE=2 SSB64_KOTH_GOAL=6 SSB64_VS_STAGE=0 vs.sh 0,8,6 inputs/none.txt k_ffa 1500` shows a
  white hill with no king, a dark gray that lightens while the buffer runs, then the king colour and the pulsing fighter overlay. Use dense shots
  (`SHOTS=$(seq -s, 300 4 380)`) for the pulse.
- Teams: add `SSB64_VS_TEAMS=1` with four fkinds (`0,8,6,5`); the hill is red or blue by holder, gray while contested, the bars are per team.
- Hill model: in the log `koth: hill line N vertices ...` appears at each hill change; a missing model logs `koth: model file`. A stray plate
  that appears when the camera zooms out on Peach's Castle belongs to the stage, not to the KOTH model (check by running the same match without
  `SSB64_REMIX_MODE`, and with `SSB64_KOTH_NO_MODEL=1`).
- Goal picker and CSS: `css.sh` with `SSB64_REMIX_MODE=koth` and `inputs/css_pick_four.txt` shows the header, the 3-digit picker and the voice.
