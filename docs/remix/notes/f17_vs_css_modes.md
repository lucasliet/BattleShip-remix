# F17: VS character select, Remix mode items (M11 Fase 5 + Fase 4 leftover)

Status: code written, syntax-checked (`syntax_check.py` OK for every touched C file) and the unit test compiled alone with
`cc` and run (189 checks, 0 failed). Not built (new `.c` files need `cmake -S . -B build` first), not run in the game, not
committed. HUD text slots 84..95 were not used (every item is a sprite), so the slot map is unchanged.

## asm -> port

| Item | asm | port |
|---|---|---|
| KOTH/Smashketball "GOAL" picker image | `Smashketball.asm` `css.use_goal_yellow_picker_` (hook 0x80133FD8): offset 0x27FC8 instead of 0x48B0 when mode is KOTH or Smashketball and `showing_time` is off | `mnRemixGoalPickerImageOffset` (`mnremixgoalpicker.c`), used by `mnPlayersVSMakeTimeSelect`; `REMIX-TODO(koth-goal-label)` removed |
| `showing_time` toggle (goal <-> Time) | `CharacterSelect.asm` `check_picker_press_` (6290-6400), `KingOfTheHill.asm`/`Smashketball.asm` `leave_css_setup_`/`start_match_setup_`, `VsRemixMenu.asm` `mode_specific_setup_` (resets it) | rules in `mnremixgoalrules.c` (`mnRemixGoalRulesToggleGoal`, `SettleGoal`, `PickerPressed`), state in `mnremixgoalpicker.c`; press branch `mnPlayersVSCheckPickerPress`/`mnPlayersVSTogglePicker` in `mnplayersvs.c` (after the arrows, before the game mode button); reset in `mnPlayersVSInitVars`; settle at the top of `mnPlayersVSSetSceneData`. The Time chosen becomes the VS menu time through `gmRemixModeGetVsTime/SetVsTime` (`gmremixmodecss.c`, the `global_time` of the asm) |
| Timed Stock picker toggle (stock <-> Time) | same `check_picker_press_`, non time-only branch (`Global.vs.game_mode` neither Time nor Stock), rule var set to Time while showing, restored after | `mnRemixGoalPickerCanToggle/Toggle/Settle`: Tag Team and Tug of War (rule Timed Stock) get it; the rule is restored in `mnPlayersVSSetSceneData` so the match keeps Timed Stock |
| Default VS time options 1/2/3/5/8/10/12/15/20/30/60/99/inf | `VsRemixMenu.asm` `change_time_options` (856-940) | found while auditing `get_next_value`; `mnRemixGoalRulesValues(MNREMIX_GOAL_TABLE_TIME)`, `mnPlayersVSGetNext/PrevTimeValue` now always go through `mnRemixGoalPicker*` under the Remix guard |
| Smashketball type selector | `Smashketball.asm` `css.enable_toggling_mode_` (hooks 0x8013537C, 0x80135400, 0x801354A4); `update_css_header_` | `mnPlayersVSUpdateGameMode`: in Smashketball the button flips the type (`mnRemixCssModeToggleSmashketballType` -> `remix_modes_smashketball_set_type`, the `gRemix.Modes.SmashketballType` CVar the port menu edits), the header is "Smashketball 1" (0x2C18) / "2" (0x2E88), and the routine returns before the blue channel, the shades, the gates and the team select (the asm jump to 0x80135544). The vanilla code at 0x80135334 was disassembled from the ROM to confirm which reads go through `a2` |
| Smashketball needs exactly two teams | `Smashketball.asm` `css.prevent_start_unless_two_teams_`, called from `VsRemixMenu.asm` `control_match_start_` after the normal ready checks | `mnRemixCssModeRulesHasTwoTeams` (pure), `mnRemixCssModeTeamsAllowStart`, `mnPlayersVSCheckModeTeams` in `mnPlayersVSCheckReady`. A panel counts when its player kind is not N/A and a fighter is picked or hovered |
| Tag Team title | `TwelveCharBattle.asm` `update_css_header_._vs` (0x2738) | `mnRemixCssModeRulesTitleOffset`, all modes in one table (12CB 0x2048, Tag Team 0x2738, KOTH 0x29A8, Smashketball 0x2C18/0x2E88, Tug of War 0x30F0). `mnPlayersVSGetModeHeaderOffset` (CSS) and the results header in `mnvsresults.c` use it. Toggling Free-for-all/Team in KOTH, Tag Team and Tug of War keeps the mode title (the asm patches the title table 0x8013B7C8 for both slots), the port used to bring the vanilla header back |
| Tag Team announcer | `update_announcer_on_entry_` (FGM `announcer.css.TAG_TEAM` 783); same table gives KOTH 775, Smashketball 776, Tug of War 479, 12CB 1016 | `mnRemixCssModeAnnounce`, called first in the entry voice block of `mnPlayersVSFuncStart` |
| Bookend slide-in | `CharacterSelect.asm` `draw_portraits_` (3639-3810), patched animation routine at 0x80131DC8, tables `portrait_x_position` and `portrait_velocity` (3878-3912) | `mnremixcsssliderules.c` (column, start x -20 / 310, step with clamp, tables are the generated `kRemixCssPortraitX/Velocity`), `mnRemixCssSlideAttach` (`mnremixcssslide.c`) called by `mnRemixCssBonusMakeBookend`, `mnRemixCssRandomMakeBookend` and, since `draw_portraits_` slides the whole row, by `mnPlayersVSMakeRemixPortrait` for the 30 grid portraits. Bonus bookend = column 10 (from the left, 1.1), Random = column 11 (from the right, -0.9) |

## Deviations

1. Finished 12-Character Battle: the asm announces "Player N" and later "Wins" (frame 0x34, `_wins`); the port plays the vanilla voice (`mnRemixCssModeAnnounce` returns FALSE for it). Not ported, out of the brief.
2. Smashketball type: the CVar is what the port menu edits, so the CSS toggle and the menu agree. `SSB64_SMASH_TYPE` still overrides everything, then the button does not change the header.
3. `Toggles.shortcut_L_flag` (L held 60 frames saves the CSS) has no port equivalent: leaving the CSS settles the picker in `mnPlayersVSSetSceneData`, which is what that shortcut emulated.
4. The asm ignores a held A on the picker ("no fast cycling"); the port branch sits in the A tap handler, so it is already tap only.
5. The asm checks the picker press before the other buttons; the port checks it after the two arrows. The zone (x + 16 .. x + width - 16 of the picker image) may touch the right arrow zone (x 210..230); the arrow wins there.
6. Tag Team `prevent_start_with_empty_slots_` and the queue icons are ported in the follow-up section below.
7. The Smashketball title after a toggle keeps the blue channel of the sprite file (the asm skips the blue store), as the asm does.
8. The 0x27FC8 picker image and the title offsets are values of the Remix asm, read from the files the port already loads (`sMNPlayersVSFiles[0]` for the picker, `[4]` for titles, like the 12CB header 0x2048). Nobody has seen them on screen yet.

## Files

New (follow-up): `mnremixtagteamcss.{c,h}`, `mnremixtagteamcssicons.{c,h}`, `mnremixtagteamcssrules.{c,h}`; edited `decomp/src/gm/gmremixtagteam.c` (CSS first slot and CSS slots in the queue), `port/remix/remix_modes.{h,cpp}` (`remix_modes_tagteam_set_slot`), `mnplayersvs.c` (queue hooks, announcer skip on the toggle), `mnremixcssmode.{c,h}` (`mnRemixCssModeKeepsToggleVoice`).
New: `decomp/src/mn/mnplayers/mnremixcssmoderules.{c,h}`, `mnremixcssmode.{c,h}`, `mnremixgoalrules.{c,h}`, `mnremixcsssliderules.{c,h}`, `mnremixcssslide.{c,h}`, `tests/remix/test_css_vsmodes_rules.c`.
Edited: `mnremixgoalpicker.{c,h}` (rewritten on top of the rules), `mnplayersvs.c` (includes, helpers before the cursor proc, press branch, game mode toggle, ready check, settle, reset, entry voice, slide attach, next/prev value), `mnremixcssbonus.c`, `mnremixcssrandom.c` (one attach call each), `mnvsresults.c` (header offset), `gmremixmodecss.c` + `gmremixmode.h` (`gmRemixModeGetVsTime/SetVsTime`), `port/remix/remix_modes.{h,cpp}` (`remix_modes_smashketball_set_type`), `tests/remix/CMakeLists.txt`, `tests/remix/test_main.c`.
The new `.c` files need `cmake -S $PWD -B $PWD/build` before building.

## Unit tests

`tests/remix/test_css_vsmodes_rules.c` (`remixTestCssVsModesRules`, registered in CMake and `test_main.c`): title and announcer tables, the two team rule, time and goal tables with next/prev wrap, the picker press zone, the toggle and settle, slide columns, start x, step, clamp and arrival of all 12 columns.

## In-game recipe (lead)

All with `SSB64_START_SCENE=16` and `css.sh <input> <name> [frames]`; the cursor hot spot is the cursor sprite position + (20, 3).

1. Slide-in (no input, `inputs/none.txt`): `SHOTS=1,5,10,15,20,30,40,50` and a sheet. Expected: the left columns come in from x -20, the right ones from 310, the outer columns slowest; the bonus bookend (x 24) takes about 40 frames and the Random bookend (x 279) about 35 frames. Run it in the default mode and with `SSB64_REMIX_MODE=12cb` (12CB must be unchanged).
2. KOTH goal: `SSB64_REMIX_MODE=koth`. Frame 60 shows the yellow picker with "GOAL" and 30, header "King of the Hill". Input to press the picker: the picker image is at (140, 22); put the hot spot on it, e.g. cursor sprite at (170, 30) means hot spot (190, 33): stick up and right from the P1 start, then `A` (line format `from to port buttons stick_x stick_y`; tune the stick frames with a screenshot, the cursor start is the P1 panel). After A the picker must read "TIME" with the VS time (the VS menu time, 8 on a clean save), another A returns "GOAL 30". Arrows step 5, 10, 15, ... on GOAL and 1, 2, 3, 5, 8, ... on TIME. Leave with B and START to confirm the VS menu time kept the Time chosen.
3. Smashketball: `SSB64_REMIX_MODE=smashketball`. Header "Smashketball 1"; A on the header (hot spot x 27..137, y 14..35, so cursor sprite near (20, 15)) gives "Smashketball 2" and the `gRemix.Modes.SmashketballType` CVar 1; panels and team buttons stay as they were. Picker shows "GOAL 10". Two team rule: the panels start Red, Red, Blue, Blue (`mnPlayersVSResetPlayer`, same as the reference capture), so `inputs/css_pick_four.txt` still starts. To see the rule, click the team button of one panel until three teams are present (or pick only P1 and P2 and put both on Red with the team button): the START must be refused (denied sound, no READY TO FIGHT).
4. Tag Team: `SSB64_REMIX_MODE=tagteam`. Header "Tag Team" and the announcer: run with `SSB64_AUDIO_FGM_LOG=1` and check `[fgm] play id=783` right after the CSS opens (775 koth, 776 smashketball, 479 tugofwar). The picker (Timed Stock) shows the stock value; A on it shows TIME and the arrows step the Time; A again returns to the stock value; START in either state must give a Timed Stock match (`remix mode:` log line has rules=3 and the stocks).
5. Tug of War: `SSB64_REMIX_MODE=tugofwar`, header "Tug of War" and the same picker toggle.
6. Results header: after a KOTH or Smashketball match the results screen header must show the mode title (before it showed Free-for-all/Team Battle).
7. Regression: default mode CSS (`b10_reg_cssflow` style flow), a Time rule VS (time arrows now step 1/2/3/5/8/10/12/15/20/30/60/99/inf), and the vanilla build (`b10_us_flow`).

## Tag Team CSS queue (follow-up, same note)

Reference captures (`build/ref-shots/{tagteam,smashketball,koth}/smash_remix-000.png`) were compared: the header colours (Tag Team and
King of the Hill use the Free-for-all yellow `E3AC04`, Smashketball the Team Battle green `61AD49`), the "GOAL 10" / "GOAL 30" /
"STOCK 4" pickers and the Red, Red, Blue, Blue team labels of Smashketball already match what the port builds. The "check mark" of
P2 in the Tag Team capture is the stock head of Bowser (red hair, green shell), not a mark.

| asm (TagTeam.asm css scope) | port |
|---|---|
| `character_queues`, `current_slot` | `MNRemixTagTeamCssQueue` (`mnremixtagteamcssrules.{c,h}`, pure) held in `mnremixtagteamcss.c`; they outlive the CSS, like the asm tables |
| `update_slot_` (pick, costume change, fighter sync) | `mnRemixTagTeamCssWrite`, called from the end of `mnPlayersVSUpdateFighter` (the sync, also run by the hover preview and by `SetCursorGrab`), `mnPlayersVSSelectFighterPuck` and `mnPlayersVSUpdateCostume` |
| `move_to_next_slot_on_pickup` | `mnPlayersVSSetCursorGrab`: not while the token flies back (`is_recalling`); the sync that follows copies the fighter into the new slot, which is why the capture shows Ness twice |
| `check_recall_with_a_` | `mnPlayersVSTagTeamCursorButtons`: A on the portraits with a placed token (not just placed) goes to the next slot and recalls the token |
| `clear_slot_and_go_to_previous_slot_` | same function: B with the token in hand empties the slot being picked, goes back one, writes the panel fighter again, plays FGM 158 |
| `adjust_current_slot_on_stock_count_change_` | `mnPlayersVSMakeStockSelect` entry -> `mnRemixTagTeamCssStocksChanged`; the panels whose slot went down sync again |
| `speed_up_token_pickup_` | `mnPlayersVSSelectFighter`: 2 frames instead of 30 for `recall_end_tic` in Tag Team |
| `prevent_start_with_empty_slots_` | `mnPlayersVSCheckReady` -> `mnPlayersVSCheckTagTeamQueues` -> `mnRemixTagTeamCssAllowsStart` (last slot of each panel in play filled) |
| `get_stock_count_` | `mnPlayersVSGetTagTeamStockCount`: the stock picker, lowered to the Manual stock count of the panel |
| `setup_`, `slot_icon_update_routine_` | `mnremixtagteamcssicons.c`: 6 icons per panel at x = panel x + 5 + 10 i, y 117, shown for stock count + 1 slots, hidden for N/A panels; an empty slot is the dot of file 0x19 (footer 0x80), a filled one the stock icon of the fighter (Master Hand at 0.625). `mnRemixTagTeamCssSetup` is called at the end of `mnPlayersVSFuncStart` |
| `icon_cache` | the slot being picked draws the stock sprite of the panel fighter (always loaded); the other slots read a private copy of the fighter main file (`lbRelocGetExternHeapFile`), because the CSS replaces the main file of the panel fighter while a token moves over the grid |
| `update_previous_icons_` (team palettes) | at draw time: team costume (`ftParamGetCostumeTeamID`) in a team battle, the picked costume in an FFA |
| `start_match_setup_` (Random slots, costumes, shades) | `gmRemixTagTeamBuildQueues` (gmremixtagteam.c): `gmRemixTagTeamApplyCssFirstSlot` starts the match with the first pick, which may differ from the last fighter of the panel (the CSS pick is put back after the fighters exist, as for Stock Mode Last), `gmRemixTagTeamFillMenuSlot` reads the CSS queue first (FFA keeps the CSS costume, teams the team costume and shade, a Random token rolls like a Random slot) |

One data set: the slots from 1 are mirrored into the CVars of the port menu (`remix_modes_tagteam_set_slot`; the menu names only the 30
fighters of the CSS grid, any other fighter reads as Random there). A slot the menu changed after the CSS wrote it takes the menu value
(the CSS entry and the match both check it), so Remix > Tag Team still sets queues, now as a prefill that the CSS shows in its icons.
A match without the CSS (`SSB64_START_SCENE=22`) has an empty CSS queue and uses the menu and Random as before.

Deviations of this part: the icon palette of a slot picked in an FFA does not shift for a duplicate fighter of an earlier port
(`update_previous_icons_` ffa loop); a Random token slot shows the dot unless the "?" fighter has a stock sprite; `current_slot` is not
re-read from the queue when the CSS opens (the panel fighter comes from the transfer state, which equals the last pick). The asm
overwrites the slots above the current one with the panel fighter at every pickup, so a menu prefill is replaced while the player picks.

Recipe (Tag Team, `SSB64_START_SCENE=16 SSB64_REMIX_MODE=tagteam`, `css.sh`): 1. Frame 60: four rows of icons above the panels (4 dots
each with Stock 4; each row follows the panel x 22 + 69 i; none above a N/A panel). 2. P1 picks a fighter (`css_pick_mario.txt`-style
input): the first icon becomes the stock head. 3. P1 picks the token up again (A on the placed token, or A over the portraits away from
the token): the second icon shows the same head at once (sync), picking another portrait changes only that icon, B while holding it
empties it and the previous icon stays. 4. Stock arrows (cursor hot spot on the arrows right of "STOCK"): the number of icons follows
the stock + 1, lowering it below the current slot clears the extra icons. 5. START with one empty slot is refused; with every queue
filled the match starts and the in-match HUD (`tagteam: port N queue ...` in the log) lists the CSS picks, the first fighter being the
first pick. 6. Remix > Tag Team menu: set slot 2 of P1 to Pikachu, reopen the CSS: the third icon is Pikachu.
Not seen in the game.

## ai-memory

Not consulted for this worker run (read-only research then edits under the brief of the lead); no rules page was read.
