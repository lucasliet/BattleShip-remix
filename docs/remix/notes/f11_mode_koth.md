# F11 mode research: King of the Hill (koth)

Source: `.toolchain/pkgs/smashremix-src/src/KingOfTheHill.asm` (1809 lines, read in full) and the hook sites in `VsRemixMenu.asm`, `TimedStock.asm`, `Render.asm`, `TwelveCharBattle.asm`, `GFXRoutine.asm`, `FGM.asm`, `File.asm`, `Global.asm`. Decomp side is `decomp/src/`. Mode id: `VsRemixMenu.mode.KOTH` = 3 (`VsRemixMenu.asm:16`), which is `gMNVSModeRemixMode` = 3 in the port (page 2, row 2, see `f10_menu_vsmodes.md`). The port reads nothing yet, so today a KOTH pick plays as a normal VS match.

Status of this note: research only. Nothing was built or run. Addresses are the Remix RAM addresses written in the asm. The `// 0x...` comments in the decomp were not used for matching (see menus.md caveat), only names.

## 1. What the mode is

Rules: a team (or FFA player) that holds the hill (the ground line picked at random) on its own, with no contest, accumulates on-hill frames until it reaches the KOTH time target (the CSS "time" value, default 30, in seconds; the code multiplies by 60 to get frames). After the target is reached, a win timer runs: 210 units, +2 per frame in FFA (about 1.75 s) or +1 per frame in teams (about 3.5 s), and then the match ends (`check_match_end_`). The vanilla time limit is ignored (infinite) and sudden death is off.

Selectors:
- `VsRemixMenu.vs_mode_flag` == 3 (set by page 2 of the VS Remix Modes menu). This is the only switch; no `Global.vs.game_mode` branch is in this file.
- `Global.vs.game_mode` is forced to 1 (TIME) in `before_css_setup_` (`KingOfTheHill.asm:197-199`, RAM `0x800A4D0B` and `0x8013BDAC`).
- `Global.vs.teams` follows `KingOfTheHill.teams` (set from the CSS teams toggle, `:209-213`, `:245-254`).
- `Global.match_info` is read only for the Dragon King stage check (`:1571-1578`).
- Single player: not used. KOTH is not referenced by `SinglePlayerMenus.asm` or the 1P modes; it is reachable only from the VS page 2 menu.

Other flags: `KingOfTheHill.time`, `teams`, `timer`, `king`, `contested_buffer` (see section 3).

## 2. Flow

Entry to exit, with each vanilla screen the mode changes.

1. **VS Game Mode menu, page 2** (scene 9): button row `VsRemixMenu.asm:95` (`db 0x10, mode.KOTH, 0x1, 0x0`, text at `0x42AA, 0x42E0`, text offset `0x84B8`). Selecting it stores mode 3 (port: `mnVSModeSelectRemixMode`, `mnvsmode.c:1406`). Screen-level: `no match found` by name for a KOTH-specific menu function; the generic path is `mnVSModeSelectRemixMode`.
2. **Before CSS** (`mode_specific_setup_`, `VsRemixMenu.asm:476`, table at `:529-538`): `KingOfTheHill.before_css_setup_` (`KingOfTheHill.asm:193`). Sets game_mode TIME, `Global.vs.time` = KOTH time, `Global.vs.teams` = KOTH teams, CSS title offset `0x29A8` (`0x8013B7C8`, both FFA and teams slots), clears the 4 `StockMode.stockmode_table` entries (`:220-224`). Port: `mnVSModeSaveSettings` (`decomp/src/mn/mnvsmode/mnvsmode.c:1173`) is the closest by name.
3. **Save before CSS** (`save_global_settings_`, `VsRemixMenu.asm:388`, costume part at `:444-446`): when mode is KOTH and teams, the team costume/shade path is used (`mnVSModeSetCostumesAndShades`, `mnvsmode.c:1274`, match by name).
4. **CSS** (`mnPlayersVSFuncStart`, `mnplayersvs.c:5714`, match by name): the time picker uses the KOTH value array (see 2a). The header is `update_css_header_` (`TwelveCharBattle.asm:1865`, KOTH branch at `:1886-1888` and `:1916-1918`, image `0x29A8`). The announcer on entry is `update_announcer_on_entry_` (`TwelveCharBattle.asm:1746`, KOTH branch `:1768-1770`, FGM `FGM.announcer.css.KOTH` = 775, `FGM.asm:2014`).
   - 2a. Time picker: `get_next_value` / `get_prev_value` (`VsRemixMenu.asm:872`, `:919`). KOTH array `KingOfTheHill.time_value_array` (`KingOfTheHill.asm:28-29`: 5, 10, 15, 20, 25, 30, 45, 60, 90, 120, 240, 480, 960) when `showing_time` is off. The hook sites are `0x138D80` (RAM `0x8013AB00`) and `0x138E5C` (RAM `0x8013ABDC`).
   - 2b. 3-digit picker: `allow_3_digits_for_picker_` (`KingOfTheHill.asm:303-322`, patch `0x1321F8` -> `0x80133F78`) lets the yellow value scroller show 3 digits for KOTH and moves x when it has 3 digits.
5. **Leave CSS** (`mode_specific_setup_returning_`, `VsRemixMenu.asm:545`, table `:610-620`): `KingOfTheHill.leave_css_setup_` (`KingOfTheHill.asm:233-264`). Saves the CSS time (or the 'Goal' value when Time is not showing) and teams to KOTH globals, then restores the VS global time (`VsRemixMenu.global_time`).
6. **Start match** (`mode_specific_setup_start_`, `VsRemixMenu.asm:627`, table `:665-677`): `KingOfTheHill.start_match_setup_` (`KingOfTheHill.asm:268-298`). Stores the KOTH time and teams, and sets the displayed time (`0x8013BD7C`) to the VS global time. Match-start function in the decomp: `mnBattleSaveMatchInfo` is called by `control_match_start_` (`VsRemixMenu.asm:684`, not KOTH specific), and the match start is `scVSBattleStartBattle` (`decomp/src/sc/sccommon/scvsbattle.c:269`, match by role).
7. **Match setup** (`Render.asm:2299`, inside `Render.asm` scope `setup_` at `:2115`, `_vs` branch): `jal KingOfTheHill.setup_`. Initializes timer = 120 frames, scores, team scores, winner times, gfx and indicator to NULL, hill_line_id = -1, king = -1; loads `File.KING_OF_THE_HILL_GFX` (`File.asm:3244`, id `0x14F3`, Remix-only, the Remix file range starts at `File.asm:14`) into `Render.file_pointer_4`; registers `hill_update_routine_` and `track_score_` per frame; creates the per-port indicators and registers `update_indicators_`. Decomp match by role: `scVSBattleStartBattle` (`scvsbattle.c:269`).
8. **Match end** (`check_match_end_` dispatch, `VsRemixMenu.asm:721-741`, patch `0x8E980` -> `0x80113180`): KOTH calls `KingOfTheHill.check_match_end_` (`:741`), then the vanilla path with `at = 0x64` (infinity, `:750`). The game-set jump is `0x80113218` with `t9 = 0x80114C80` (`KingOfTheHill.asm:388-389`); decomp name: `no match found` (the address comments in the decomp are not reliable here).
9. **Results** (`add_row_to_results_`, `VsRemixMenu.asm:1007`, comment "Adds Time to the results screen for KOTH"): the extra KOTH time row. Not ported: `REMIX-TODO(results-mode-rows)` in `decomp/src/gm/gmremixgameend.c:150`. Results uses vanilla place ordering (`mnVSResultsGetWinPlayer`, `decomp/src/mn/mnvsmode/mnvsresults.c:515`).
10. **Results/return**: no KOTH-specific return. Sudden death is off by `prevent_sudden_death_` (`TimedStock.asm:173-190`, patch `0x10A4F4` -> `0x8018D604`; KOTH branch at `:181-183`). Name match: `scVSBattleSetScoreCheckSuddenDeath` / `scVSBattleStartSuddenDeath` (`scvsbattle.c:394`, `:579`), not verified by address.

Vanilla screens changed: VS page 2 menu, VS CSS (time picker, header, announcer), match start, match end, results (missing row), in-match scoring and HUD, respawn, CPU AI.

## 3. State and data

Globals (`KingOfTheHill.asm`, labels inside `scope KingOfTheHill`). Sizes are as written in the asm.

| Name | Size | Written by | Read by |
|---|---|---|---|
| `time` | dw | `before_css_setup_`, `leave_css_setup_`, `start_match_setup_` (`:249`, `:284`) | `setup_` (`:1696`), `track_score_` (`:1297`), CSS picker |
| `teams` | dw | `leave_css_setup_` (`:254`) | `render_ground_gfx_`, `track_score_`, `setup_`, `VsRemixMenu` (`:445`) |
| `time_value_array` | 13 dw | constant | picker `:890`, `:937` |
| `scores` | 4 dw (frames on hill per port) | `setup_` (zero), `track_score_` (`:1324-1325`), `check_match_end_` winner bonus (`:381`) | `update_indicators_`, `TimedStock` (`TimedStock.asm:136`) |
| `team_scores` | 4 dw (per team) | `setup_`, `track_score_` (`:1326-1354`) | `track_score_`, `update_indicators_` (via `:1666` pointer) |
| `winner_times` | 4 dw (per team, win timer units) | `setup_`, `track_score_` (`:1333-1381`) | `check_match_end_` |
| `hill_line_id` | dw, -1 = none | `setup_`, `set_random_hill_` (`:1041`) | `track_score_`, `ai_hill_behavior_` |
| `timer` | dw frames | `setup_` (120), `hill_update_routine_` | `hill_update_routine_` |
| `king` | dw; -1 none, -2 contested, else team/port | `setup_`, `track_score_` (`:1280`) | `render_ground_gfx_`, `track_score_`, `check_match_end_` |
| `contested_buffer` | dw frames | `track_score_` (`:1268`, `:1275`), `set_random_hill_` (`:1047`) | `render_ground_gfx_` (gray), `track_score_` |
| `use_cpu_color` | 4 db | `setup_` (`:1605`) | `render_ground_gfx_`, `track_score_` |
| `indicators` | 4 dw (indicator object per port) | `setup_` | `update_indicators_` |
| `indicator` | dw (one shared object, the hill marker) | `create_ground_gfx_` (`:718`) | `ai_hill_behavior_`, `is_hill_outside_blast_zone_` |
| `gfx` | dw (first ground GFX object; chain of segments via +0x40) | `create_ground_gfx_` | `set_random_hill_`, `render`, `setup_` |
| `hill_status`, `team_hill_status`, `king_eligible` | 4 db each | `track_score_` each frame | `track_score_` |
| `winner_time_updated` | 4 db | `track_score_` | `track_score_` |
| `gfx_routines`, `team_gfx_routines` | 5 / 4 dw | constants (`GFXRoutine.KOTH_*`) | `track_score_` |
| `colors`, `team_colors`, `colors_meter`, `team_colors_meter`, `colors_bar`, `team_colors_bar` | constants | | `render_ground_gfx_`, `setup_` |
| `koth_ground_gfx_struct`, `koth_indicator_gfx_struct` | GFX object templates | | `create_ground_gfx_`, `create_indicator` |
| `hidden_frames` | dw (inside `hill_update_routine_`) | | |
| `DELAY` | constant 120 | | `delay_respawn_` |
| `CONTESTED_BUFFER` 30, `CONTESTED_GRAY`, `WIN_FRAMES` 210, `ALPHA_DELTA` 5, `ALPHA_DELTA_MAX` 100 | constants | | |

Note: `get_surface_center_coords_` (`:797-900`) is defined but not called anywhere in the file (grep). It is dead code for the port.

Remix globals read from outside: `VsRemixMenu.vs_mode_flag`, `VsRemixMenu.global_time`, `CharacterSelect.showing_time`, `CharacterSelect.saved_nontime_value`, `Global.vs.time` (`0x800A4D0E`), `Global.vs.teams` (`0x800A4D0A`), `Global.vs.game_mode` (`0x800A4D0B`), `Global.vs.game_status` (`0x800A4D19`), `Global.vs.timer` (`0x800A4D1C`, not read here), `Global.match_info` (`0x800A50E8`), `Global.p_struct_head` (`0x80130D84`), `StockMode.stockmode_table`, `Render.file_pointer_4`, `Global.get_random_int_safe_` (returns a value below its argument, from frame-count RNG, `Global.asm:248-266`; the range is assumed, not proven), gMPCollision globals `0x80131300` (ground data incl. blast zone bounds at `+0x74..+0x7A`), `0x80131304` (yakumono dobjs), `0x80131368` (geometry), `0x80131370/0x80131374/0x80131378` (vertex data/ids/links).

Port-side symbols: `port/remix/remix_symbols.h` has only `GFXRoutine.KOTH_*` (`:1398-1402`). No `KingOfTheHill.*` symbol exists, so the RAM addresses of these globals must come from the build or from label lookup; this is an open item.

Files the mode loads (plain constants in the asm):
- `File.KING_OF_THE_HILL_GFX` = `0x14F3` (`File.asm:3244`), Remix-only. Its image footer is at `+0x0AA8` (progress bar texture, `KingOfTheHill.asm:1611`). The `0x29A8` offset is a sprite offset used by title and header code (`:216`, `TwelveCharBattle.asm:1888`, `:1918`), which file it lives in is not determined here.
- Sound ids: 0xA2 (hill change, `:1044`), 0x115 (king gained, `:1283`), 0xA9 (king lost or contested, `:1285`), `FGM` countdown `FGM.announcer.fight.THREE_NONBLOCKING`, `TWO_NONBLOCKING`, `ONE` (`:336-342`), CSS announcer `FGM.announcer.css.KOTH` = 775 (`FGM.asm:2014`).
- GFX routines: `GFXRoutine.KOTH_RED`, `KOTH_BLUE`, `KOTH_GREEN`, `KOTH_YELLOW`, `KOTH_GRAY` (`GFXRoutine.asm:299-319`, registered at `:399-403` with priority 100 and `OS.FALSE`). All are Remix-only.
- Vanilla routine addresses used: `0x800FDB1C` (create GFX object, used for ground and indicator, `:627`, `:713`), `0x8000DF34` (GFX update, `:421`, `:453`), `0x80014038` (GFX render, `:497`), `0x800269C0` (play sfx, `:1043`, `:1289`), `0x80134E88` (CPU target return, `:1758`). Decomp names for these are `no match found` by name, except `0x800FDB1C` which the decomp comments put in `ef/efmanager.c:2128` (comment only).

## 4. In-match logic (per frame, order of registration)

Registration order (`setup_`): `hill_update_routine_` first, then `track_score_`, then `update_indicators_`. Each runs once per frame in the Render loop. Scores and win times are in frames at 60 fps.

**A. `hill_update_routine_`** (`KingOfTheHill.asm:1112-1168`), runs only when `Global.vs.game_status == 1` (ongoing):
1. If `timer != 0`, decrement it. Else go to step 4.
2. If the hill GFX exists: if the display flag is on (`0x0054 == 0`), reset `hidden_frames` to 0; if it is off, increment `hidden_frames`. When it reaches 120 (2 s hidden), go to step 4.
3. Check the hill against the blast zone (`is_hill_outside_blast_zone_`, `:1059-1108`, reads the indicator position against `gMPCollisionGroundData` blast bounds). If outside, go to step 4.
4. New hill: `timer = 1200 + random(0..1199)` (20 to 40 s), then `set_random_hill_`.

**B. `set_random_hill_`** (`:1001-1053`): up to 3 tries. Each try: `get_random_line_id_` picks a random line from the collision line groups (`:904-997`), skipping surfaces that are not valid (valid types: 0x00-0x06, 0x0C-0x0E, 0x11, 0x1F-0x23; invalid: damage 0x07-0x0B, 0x0F-0x10, 0x12-0x1E). Then `create_ground_gfx_` builds the segments. Retry if outside the blast zone, if the GFX is hidden after `update_ground_gfx_`, or if the id equals the current hill (keep the same hill when tries run out, no sound). On success: `hill_line_id` = new id, play 0xA2, `contested_buffer = 0`.

**C. `track_score_`** (`:1172-1390`), runs when `game_status == 1`:
1. Clear `hill_status`, `team_hill_status`, `king_eligible`.
2. For each player struct (`Global.p_struct_head` list): on hill if `line_id` (0xEC) == `hill_line_id`, grounded (`0x14C` == 0), not captured by another player (`0x844` == 0). If the player holds another (`0x840`): if the held player's team is the same team (or same port in FFA), eligible; otherwise contested (a1 = -2) and not eligible. Set `hill_status[port]`, `team_hill_status[team]`, and `king_eligible[port]`.
3. King candidate `a1`: starts -1. The first eligible team on the hill sets it. A second team on the hill, or a contested player, makes it -2.
4. Buffer (`_king_exists` path, `:1263-1270`, reached for `a1 >= -1`): if the buffer is non-zero, it decrements and `a1 = -2` (contested). If `a1 == -2` (`_king_contested`, `:1272-1275`), the buffer is reset to 30 (`CONTESTED_BUFFER`) every frame. So after a contest ends, or when a team first takes the hill, it counts as contested for 30 frames before it becomes king. When nobody is on the hill and the buffer is 0, the king is -1. A king that leaves the hill with the buffer at 0 loses the crown on that frame.
5. `king = a1`. If it changed: play 0x115 if `a1 >= 0`, else 0xA9.
6. Scoring: for each player whose team == king, on hill, eligible: if `team_scores[team] != target` (target = `time * 60` frames), increment `scores[port]` and `team_scores[team]` by 1 frame. If equal, do not increment; add to the team's win timer instead (once per team per frame, `winner_time_updated`): `winner_times[team] += 2` in FFA, `+= 1` in teams.
7. Per scoring player, if the fighter's gfx routine slot (`0x0A28`) is empty, set it to the team/port KOTH routine (CPU players use the gray routine, index 4; teams always use team colors).
8. For every player not on the king team (including all players when king is -1 or -2), reset `winner_times[team] = 0`.

Note: the per-player increment in step 6 means that in teams mode with two eligible players on the hill, the team score rises by 2 per frame. See risks.

**D. `check_match_end_`** (`:326-391`, called from the dispatch at `VsRemixMenu.asm:741`):
1. For each team entry in `winner_times` (4), compare with 210 (`WIN_FRAMES`). If at 30, 90 or 150 (the values for "3", "2", "1" seconds left), play the FGM for that count and return with no match end (only one countdown beep per frame; the remaining entries are not checked on that frame).
2. If a team entry is >= 210, the match ends: each player on the king team gets `scores[port] += 1` (one extra frame), then jump to game-set (`0x80113218`, `t9 = 0x80114C80`).
3. Otherwise, normal: the vanilla time limit is replaced by infinity (`at = 0x64`).

**E. `update_indicators_`** (`:1434-1484`): for each indicator (per port or team), `width = trunc(score / target * 40)` px. When `width >= 40` (full), the flash rectangle alpha changes by 5 per frame and reverses at 0 and at 100.

**F. Respawn** (`delay_respawn_`, `:1768-1806`): in KOTH the rebirth wait is 120 frames (`DELAY`), not 45. Patch `0xB6B6C` -> `0x8013C12C` sets `t6 = DELAY`. The camera for that player is turned off once the wait timer equals `DELAY - 45` = 75 (`0xB6B38` -> `0x8013C0F8`, flag bit 0 at fighter struct `0x191`). Assumes the wait timer counts down (the `t7 = t6 - 1` at `:1805` suggests it). The spawn position is not changed by this file.

**G. CPU hill seeking** (`ai_hill_behavior_`, `:1711-1764`, patch `0xAF5C4` -> `0x80134B84`): in KOTH, a CPU's target becomes the hill indicator (x = top-joint x + texture offset x, y = same minus 500 units). If the indicator is hidden or its clipping is off, the target line becomes -1. Returns TRUE, which makes the CPU skip item and tornado handling (the comment says so: "this makes them ignore items and tornados, too").

**H. Items and stage change**: no item or stage change code in this file. Stages are the normal stage (hill can land on any valid line of the stage).

## 5. HUD and text

Every on-screen element of the mode:

1. **Hill ground strip** (`create_ground_gfx_`, `render_ground_gfx_`, `update_ground_gfx_`): one GFX object per clipping segment of the chosen platform line group, joined to form a strip; unused objects are hidden, not destroyed. Colors (`render_ground_gfx_`, `:461-511`): white (0xFFFFFFFF) when king is -1; gray (`CONTESTED_GRAY`, fading with `contested_buffer * 3` per channel) while the buffer is non-zero; team color (`team_colors`, teams) or FFA color (`colors`, FFA); CPU players use the gray entry (`colors[4]`). Team mode always uses team colors (`use_cpu_color` is cleared).
2. **Hill indicator** (`koth_indicator_gfx_struct`, `:150`): a small object placed above the middle of the hill (`create_ground_gfx_`, `:709-754`, Y = midpoint of slope + 500). Its AI target is used by `ai_hill_behavior_` (section 4G).
3. **Progress bars**: one per port (FFA) or team (teams), at x = 40 + 70 * port, y = 180.0 (float) / 188 (int) in the default case; y = 8 / 16 with Dragon King HUD on or on a Dragon King stage (`setup_` `:1560-1580`). Colors `colors_bar` (FFA) or `team_colors_bar` (teams), `0xC96A6AFF`, `0x7E7EC9FF`, `0xB2B244FF`, `0x4EAD4EFF`, `0x888888FF`. The bar texture is a `Render.draw_texture_` from `file_pointer_4 + 0x0AA8`. The fill is `Render.draw_rectangle_` 40 px wide, 5 px high (`:1656`).
4. **Full-bar flash**: white 40 px rectangle over the bar, alpha pulses (`update_indicators_`).
5. **Title**: "King of the Hill" image at offset `0x29A8` in the CSS/results header (see section 2, step 4).
6. **Time picker**: 3-digit values (`allow_3_digits_for_picker_`).
7. **Results row**: a KOTH time row (see section 2, step 9). Not in the port.
8. **Announcer**: CSS voice line 775 (`TwelveCharBattle.asm:1768`), and countdown "3", "2", "1" through `FGM.announcer.fight` (section 4D).

Port constraints: the port draws text only through the ImGui overlay (`port/remix/remix_overlay.h`) and vanilla sprites. Elements 1-6 and 8 use only GFX objects, rectangles, textures and sprites, so they need no new text. The title image (5) needs the vanilla sprite path from the right file (unknown file, see open items). The results row (7) needs text for "Time" unless it reuses the vanilla KO text art (`add_row_to_results_` draws alt textures, `VsRemixMenu.asm:1020-1030`, so it is possible without new text).

## 6. Dependencies

- **Mode dispatch tables** (`VsRemixMenu.asm`): `mode_setup_table`, `mode_setup_returning_table`, `mode_setup_start_table` (`:529`, `:610`, `:665`). The port needs a KOTH entry in each equivalent (CSS, leave CSS, start match) with the same index as the mode (3).
- **Teams flag** (`VsRemixMenu.asm:443-450`): KOTH teams is read in the costume-sync path, so the team costume path depends on this mode.
- **TimedStock** (`TimedStock.asm`): `calculate_time_score_` (`:121-136`) returns `KingOfTheHill.scores[port]` for KOTH (frames on hill as the score). `pick_scoring_` (`:150-170`) selects the TIME path. `prevent_sudden_death_` (`:173`) turns sudden death off for KOTH. Shared with Smashketball, which has its own branch (`:182-186`). If the TimedStock base lands for another mode first, KOTH only adds its branch.
- **Stock mode** (`StockMode`): `before_css_setup_` clears the stock mode table (`:220-224`). KOTH does not use stock counts. The vanilla stock count in TIME mode still applies to the match, not to the win.
- **Neutral Spawns, Stamina, Items**: this file does not touch them. Respawn point selection is vanilla. Stamina does not appear in this file. The interaction with `gmremixspawnsetup.c` is not verified.
- **Results**: vanilla results ordering (`mnVSResultsGetWinPlayer`, `mnvsresults.c:515`). Winner is not set by this file; the place comes from results.
- **Shared Remix modes**: 12CB (`TwelveCharBattle.asm:1768-1770`, `:1814-1816`, `:1886`, `:1916`) and Smashketball/Tug of War share the same announcer and title code paths. KOTH only adds its own branch.
- **Dragon King HUD**: `Toggles.entry_dragon_king_hud` (`setup_` `:1561`). Dragon King stages (`Stages.id.DRAGONKING`, `DRAGONKING_REMIX`) move the bars.
- **Port existing code**: `gMNVSModeRemixMode` (`decomp/src/mn/mnvsmode/mnremixvsmode.c:26`), `gmremixgameend.c` (results, REMIX-TODO at `:150`), `gmremixstockmode.c`, `gmremixtoggle.c`, `gmremixspawnsetup.c`, `ftremix/` (fighter hooks), `port/remix/remix_overlay.h` (text).

## 7. Port plan

Order matters. Steps 1-2 are shared with other modes. Each step is one file or one hook group, small enough for one Haiku worker. The port proposal (file names) follows the existing `gm/gmremix*.c` and `mn/mnvsmode/mnremix*` naming. Each step is `PORT && REGION_REMIX` guarded like the existing code.

1. **Mode constant and predicate** (shared with 12CB, Tag Team, Smashketball, Tug of War). Add `MNREMIX_VSMODE_KOTH` (3) to `decomp/src/mn/mnvsmode/mnremixvsmode.h`, and `gmRemixKothIsActive(void)` in new `decomp/src/gm/gmremixkoth.h` / `.c` returning `gMNVSModeRemixMode == MNREMIX_VSMODE_KOTH`. Lead sees: nothing new on screen. Accept by setting KOTH on page 2 (the f10 input script) and checking `p gMNVSModeRemixMode == 3` at a break.
2. **Settings before CSS and at leave**: in `gmremixkoth.c`, `gmRemixKothBeforeCss` and `gmRemixKothLeaveCss` (time, teams, title offset, stock table clear). Call sites: `mnVSModeSaveSettings` (`mnvsmode.c:1173`) and the CSS leave path. Lead sees: CSS time picker starts at 30 for KOTH; VS Start after KOTH shows the VS time again.
3. **Time picker values**: KOTH array (5..960) in `mnvsmode.c` or the CSS time code (`get_next`/`get_prev` equivalents, the current `mnBattleGetNextTimerValue` by name is `mnplayersvs.c:5470`), plus the 3-digit picker. The vanilla picker function for that is not located by name in this brief (the asm hooks are `get_next_value` / `get_prev_value`, `VsRemixMenu.asm:872` and `:919`). Lead sees: arrows step 5, 10, 15, ..., 960; 3-digit values render; default 30.
4. **Start match and state**: `gmremixkoth.c` `gmRemixKothStartMatch` (`scVSBattleStartBattle`, `scvsbattle.c:269`), initializes the state struct (section 3). State lives in one struct, no globals with import-time side effects. Lead sees: nothing yet (state only). Accept with step 6.
5. **Hill selection** (new `gmremixkothhill.c`): `get_random_line_id_` (valid surface filter), `set_random_hill_` (3 tries, blast zone check), timer and hidden logic (`hill_update_routine_`). Uses the port's collision access (`mpcollision.c` names) through a thin interface. Lead sees: after 2 s a new hill is chosen; changes every 20-40 s; sound on change. Visible through step 8.
6. **Scoring and king** (new `gmremixkothscore.c`, split from `track_score_` by responsibility: hill status, king resolution, scoring): steps 4A-C in section 4. Lead sees: bars move in step 9. Before that, a debug print or the breakpoint on `scores[0]` increments.
7. **Match end** (new `gmremixkothend.c`): section 4D, countdown sounds, +1 frame. Depends on 6. Lead sees: the match ends 1.75 s (FFA) or 3.5 s (teams) after the target is reached; FGM 3, 2, 1 at the 30, 90, 150 unit marks; the game-set goes to results.
8. **Hill strip GFX** (new `gmremixkothgfx.c`): `create_ground_gfx_`, `update_ground_gfx_`, `render_ground_gfx_`, hill indicator object. Needs the KOTH GFX file `0x14F3` loaded (file loader, see `docs/remix/file_table_and_loader.md`). Depends on 5. Lead sees: a strip over the chosen platform, white when no king, team color when held, gray when contested.
9. **Progress bars** (new `gmremixkothhud.c`): `update_indicators_`, positions, DK move, full-bar flash. Depends on 6 and 8 (texture file). Lead sees: bars at the bottom, x = 40, 110, 180, 250; fill to full at the target; flash when full; top position on DK stages.
10. **Respawn delay and camera** (`ftremix` or `ftcommonrebirth` hook): 120 frames, camera off at wait 75. Independent. Lead sees: KOTH death lasts 2 s; camera turns off after 45 frames of the wait.
11. **CPU hill seeking** (`ftcomputer.c` hook): `ai_hill_behavior_`. Depends on 5 and 9 (indicator). Lead sees: CPUs walk to the hill and ignore items.
12. **TimedStock branch** (the port's TimedStock code, not located in this brief): KOTH score = `scores[port]`, sudden death off. Depends on 6, and on the TimedStock base if Smashketball lands it first. Lead sees: the time-stock score for KOTH players is their frames on hill.
13. **Results time row** (`gmremixgameend.c`, REMIX-TODO `results-mode-rows`): KOTH row. Depends on 12. Lead sees: a "Time" row on the results screen with the KOTH time.
14. **CSS header and announcer**: `update_css_header_` (KOTH image `0x29A8`) and `update_announcer_on_entry_` (FGM 775). Depends on 2. Lead sees: "King of the Hill" title on the CSS header and the voice line on entry.
15. **Verification pass**: teams and FFA, Dragon King stage, CPU level, pause and re-entry, results. Lead runs the smoke scripts (`remix-smoke-tests`).

## 8. Risks and unknowns

- **Team score rate**: `team_scores` increments once per eligible player on the hill (`:1326-1354`). With two teammates on the hill in teams mode, the team reaches the target in half the time. This may be the intended design or a bug. Verify in game before porting exactly.
- **Winner ranking**: this file does not pick the winner. The results place (`mnVSResultsGetWinPlayer`) decides it, from the TimedStock score (KOTH: `scores[port]`). Not traced step by step; confirm how places are computed for KOTH.
- **Countdown beeps**: `check_match_end_` returns after one beep (`:350-362`), so other teams' win timers are not checked that frame. Minor.
- **Respawn timer semantics**: the 45-frame vanilla value and the countdown direction are inferred from the asm comments and `t7 = t6 - 1`. Not verified against a decomp function: `no match found` for the rebirth timer owner. The closest decomp file is `ft/ftcommon/ftcommonrebirth.c` (`ftCommonRebirthWaitSetStatus` at `:208` does not set a timer in the decomp).
- **Gfx routine slot `0xA28`**: set once per fighter (`:1358-1373`) and never cleared in this file. Whether it resets on respawn or match start is not verified.
- **`get_random_int_safe_` range**: assumed `[0, a0)`. Not checked against `Global.asm:248-266` past the RNG advance.
- **Dead code**: `get_surface_center_coords_` is not called. Do not port it unless needed by another step.
- **Address matches**: many vanilla addresses are only address-matched through decomp comments, which the menus.md caveat says are unreliable. Matches in this note are by name or by role only. `0x80114C80` (game-set) and `0x8000DF34` / `0x80014038` (GFX update/render) have no name match.
- **Symbols**: `remix_symbols.h` does not contain `KingOfTheHill.*`. The port must find these globals by label or by a build-time table. Open.
- **Title image file**: which file holds the sprite at `0x29A8` (used by the CSS header) is not determined. Open.
- **Results row text**: whether the KOTH time row can reuse vanilla KO art alone or needs text. Open (see section 5).
- **Project memory**: ai-memory project scope for BattleShip could not be resolved (`project 'JRickey/BattleShip' not found`, and no `.ai-memory.toml`). The `_global` rules were read. No project rules were loaded.
- **Not built, not run, not verified in game.** Every behavior above is from reading the asm.
