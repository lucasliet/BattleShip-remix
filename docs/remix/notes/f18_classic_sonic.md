# F18 Classic Sonic (M11 Fase 5)

Asm: `.toolchain/pkgs/smashremix-src/src/Sonic/Sonic.asm`, `Costumes.asm select_`, `Teams.asm classic_sonic_costume_fix_`, `Shield.asm color_fix_`,
`TwelveCharBattle.asm defeated_action_override_`, `Training.asm load_character_`, `TagTeam.asm`, `Character.asm:3737`. Written, not built, not run.

## How the Remix does it

- Sonic (`Character.id.SONIC`, Remix id 0x3B = fkind 59, `REMIX_ID_SONIC`) has `File.CSONIC_MAIN` (0xE38) as his *special 2* file
  (`define_character` argument 9). `load_classic_model_` (`Sonic.asm:1001`, `0x800D7FDC`, inside `ftManagerMakeFighter`) takes the
  attributes from that file instead of `SONIC_MAIN` when `Sonic.classic_table[port]` is set. Everything the model needs (parts, joints,
  stock icon, damage colls) hangs off the attributes, so this is the whole model swap. The same hook clears the port flag when the
  fighter made is not Sonic, and clears `select_anim_frame[port]`.
- The CSS toggle is in `Costumes.asm select_` (`_sonic`): there is no indicator or sprite. C right / C left cycle the costume; the model
  swaps when C right wraps to costume 0 or C left leaves costume 0 (in a VS team battle every C right/left swaps). So on the CSS, C left
  once on a modern Sonic at costume 0 gives classic Sonic with his last costume; C right from his last costume goes back to modern.
  The panel fighter is remade, the animation frame is carried over (0 becomes 193.0) through `Sonic.select_anim_frame[port]`, which
  `handle_select_anim_` (`0x803905DC`, `scSubsysFighterSetStatus`) uses as the start frame of status 0x10004.
- Persistence: `classic_table` is the live state. VS save (`mnPlayersVSSetSceneData`) / load (end of `mnPlayersVSInitVars`) copy all four
  bytes to `classic_flags_vs`; 1P and Training do it for the human (and CPU) port with a table clear on load; Bonus has none. Deselect
  (A: cursor grabs the puck, B: puck recalled) and closing a VS port clear the port.
- Match: FGM swaps on the 0x38 and 0x3C moveset commands, voice commands 0x44 and 0x48 skipped, shield colour index +6, spin dash and
  spring routines read the table (already ported in `ftremixsonicdsp.c` / `ftremixsonicusp.c`).

## asm -> port

| asm | port |
|---|---|
| `load_classic_model_` 1001 | `ftRemixSonicClassicGetMainFile` (`ft/ftremix/sonic/ftremixsonicclassic.c`), called from `ftManagerMakeFighter` (`ftmanager.c`, `fp->attr`) with `desc->player` |
| `classic_fgm_patch_1_` 545 (0x38) | `ftRemixSonicClassicPlayFgm` + rules `ftRemixSonicClassicRulesPlayFgm`, hook in `ftmain.c` `nFTMotionEventPlayFGM` |
| `classic_fgm_patch_2_` 600 (0x3C) | `ftRemixSonicClassicPlayLoopFgm`, hook in `nFTMotionEventPlayLoopSFXStoreInfo` |
| `classic_voice_patch_1_/2_` 639/671 (0x44/0x48) | `ftRemixSonicClassicIsVoiceMuted`, hooks in `nFTMotionEventPlayVoiceStoreInfo` and `nFTMotionEventPlayLoopVoiceStoreInfo` |
| `spin_dash_fgm_patch_` 703 | already in `ftremixsonicdsp.c` (0x3D8/0x3DE restart) |
| `handle_select_anim_` 509 | `ftRemixSonicClassicGetStatusFrame`, hook in `scSubsysFighterSetStatus` (`scsubsysfighter.c`) |
| `Costumes.asm select_` `_sonic` (toggle, rebuild, frame) | `mnRemixSonicClassicToggle` / `mnRemixSonicClassicRestoreFrame` (`mn/mnplayers/mnremixsonicclassic.c`) called from the `Update*Costume` of the four CSS files |
| `Costumes.asm` yellow team fix | `mnRemixSonicClassicCycleTeamCostume` (VS only) |
| `Teams.asm classic_sonic_costume_fix_` (3 hooks) | `mnRemixSonicClassicGetTeamCostume`, the macro `MNPLAYERSVS_GET_TEAM_COSTUME` replaces the 3 `ftParamGetCostumeTeamID` calls of the VS CSS (team button, FFA to team, free costume on hover) |
| `adjust_costume_comparison_` 1034 (VS) | `mnRemixSonicClassicFudgeCostume` in `mnPlayersVSGetFreeCostumeRoyal` |
| `save/load_classic_flags_vs_/1p_/training_` | `mnRemixSonicClassicSaveVS/LoadVS/Save1P/Load1P/SaveTraining/LoadTraining`, hooks in `mnPlayersVSSetSceneData` + end of `mnPlayersVSInitVars`, `mnPlayers1PGameSetSceneData` + start of `mnPlayers1PGameInitPlayer`, `mnPlayers1PTrainingSetSceneData` + `mnPlayers1PTrainingInitVars` |
| `reset_classic_flag_` (9 hooks) | `mnRemixSonicClassicReset` in `mnPlayersVSSetCursorGrab`, `mnPlayersVSRecallPuck`, `mnPlayersVSUpdatePlayerKind` (close port), and `SetCursorGrab` + `RecallPuck` of Training, 1P Game and Bonus |
| `Shield.asm color_fix_` (+6) | `ftRemixSonicClassicShieldCostumeOffset` in `ftRemixShieldGetCostumeColorIndex` (`ftremixshieldcostume.c`) |
| `classic_sonic_anim_swap_` 1189 + `defeated_action_override_` 2955 | `ftRemixSonicClassicSwapMenuMotion`, hook in `ftMainSetStatus` right after `motion_desc` is picked: on scene 0x0E (1P intro) and 0x18 (VS results) the 1P pose (0xDD1) becomes 0xDD2 and the second victory pose (0xDD7) becomes 0xE3C, with no script (`offset` 0x80000000) and flags 0 |
| `Sonic.classic_table` | `ftRemixSonicIsClassic` / `ftRemixSonicSetClassic` (`ftremixsonic.c`, now range checked) |

Pure decisions (`ftremixsonicclassicrules.c/.h`): FGM tables, toggle rule, yellow team costume, +6 fudge, Training costume split,
select frame (193), menu animation files and scenes, load-one/load-two table rules. Test: `tests/remix/test_sonic_classic_rules.c`
(61 checks, compiled alone with `cc` and run: 0 failed; registered in `tests/remix/CMakeLists.txt` and `test_main.c`).

## Tag Team, Training menu and yellow team (follow-up)

- **Tag Team carry** (`gm/gmremixtagteam.c`, `gmremixtagteamicons.c`, `gmremixtagteamrules.c`): `GMRemixTagTeamSlot` has `is_classic`. Slot 0
  takes `ftRemixSonicIsClassic(port)` when the CSS pick is Sonic (`TagTeam.asm:778`), menu slots are FALSE, the Stock Mode Last start slot
  sets the flag of the slot it replaces the pick with. In `gmRemixTagTeamLoadNextFighter` the flag is set after the old fighter is
  destroyed and before `ftManagerMakeFighter` (asm 1061-1071), and `gmRemixTagTeamRulesNeedsSwap` has two new parameters so a changed
  flag also forces the swap (test `testASwapIsNeededWhenClassicSonicChanges`). The stock icon of a classic slot is read from the classic
  main file (`p_file_special2`, `gmRemixTagTeamIconsSet(..., is_classic)`, the cache key includes the flag). `gmRemixTagTeamGetCostume`
  used `data->color` as the team (green is color 3, the asm passes the team index `0x40(css)`): it uses `data->team` now, otherwise a
  green team would get the yellow costume.
- **Training menu costume 6 and above**: the Costume item has 13 values (Auto, 1 to 12). For Sonic the highest value is the modern one plus 6
  (`ftRemixSonicClassicRulesTrainingMenuMax`, `sc1PTrainingRemixMenuGetMax`; registry range 0 to 12 in `remix_training.cpp`,
  `SC1PTRAININGMENUENTRIES_COSTUME_MAX`). `sc1PTrainingRemixSetupPort` splits an edited costume with
  `ftRemixSonicClassicRulesSplitTrainingCostume` and sets the port flag (7 to 12 are classic costumes 0 to 5); Auto keeps what the CSS
  left (save/load pair). The asm shows the item as a plain number; the port shows the same numbers.
- **Yellow team**: see `f22_teams_yellow.md`; the Sonic rule (`mnRemixSonicClassicGetTeamCostume`, `CycleTeamCostume`) is live now.

## Deviations and open items

- 12CB: `TwelveCharBattle.asm` only touches classic Sonic in `defeated_action_override_` (menu animation swap, ported above). The 12CB CSS
  uses the VS flags (save/load), nothing else to do.
- `adjust_costume_comparison_` of Training (`training HMN/CPU`): the Training CSS of the port has no used-costume check in the Remix path, so
  only the VS one is applied.
- Menu animations (1P intro / VS results) replace the move set with an empty script, as the asm does (`standard_moveset` = End).
  Unverified in game.
- Not verified: `File.CSONIC_MAIN` (0xE38) is `file_ids[6]` of the Sonic row of the `remix/fighters` blob (it is the 9th `define_character`
  argument, so it should be). If `p_file_special2` is NULL for Sonic the classic flag silently keeps the regular model. The log line
  `SSB64: ATTR fkind=59` prints the attributes pointer per fighter.
- Voice/FGM patches check `fp->fkind == REMIX_ID_SONIC`, so Super Sonic and Polygon Sonic are not affected (the asm checks `Character.id.SONIC`).
- The Remix has no visual classic indicator on the CSS; none was added.
- Not ported (not in the asm scope given): `VsStats.asm:579` reads the classic main file for the stats icons; the port draws them from the
  results fighters (`fp->attr`), which already follow the flag.

## Test recipe for the lead (not run)

1. Match with classic model, FGM, shield (no CSS needed). The override follows the `SSB64_REMIX_COSTUME_Pn` pattern:
   `SSB64_START_SCENE=22 SSB64_VS_FKINDS=59,0 SSB64_REMIX_CLASSIC_SONIC_P1=1 SSB64_SCREENSHOT_FRAMES=520,540` with `vs.sh`
   (fkind 59 = Sonic). P1 must show the Classic Sonic model (rounder, different colours) against the normal one with the override
   unset. Spin dash (`dsp_tap.txt`) plays the classic charge sound (FGM 0x3DE) and jump 0x3E0; Sonic says no voice. With
   `SSB64_REMIX_PANEL="Shield.1=17"` plus an input script holding Z the shield colour reads costume + 6 of the Fox row.
2. VS CSS toggle (calibrate the cursor with one screenshot, Sonic is portrait slot 10, second row of the grid):
   `css.sh <script> sonic_classic 600` with `SHOTS=250,300,400`, script:
   ```
   100 128 0 - 0 80
   129 140 0 - 80 0
   141 150 0 - 0 -80     (tune: down one row, then right/left until the cursor is on Sonic)
   160 162 0 A           (pick Sonic)
   200 202 0 CL          (modern costume 0 + C left: classic Sonic, last costume, animation keeps its frame)
   260 262 0 CR          (back to modern)
   300 302 0 CL
   330 332 0 START
   ```
   The panel model must change on frames 250 and 400 and not on 300; with the VS shortcut on, A on the stage select starts the match
   and Sonic must still be classic. Then back to the CSS: the flag must be restored (`mnRemixSonicClassicLoadVS`). Deselect (B) and pick
   Sonic again: modern. A team battle (any C right/left toggles) does not need yellow.
3. Training (`SSB64_START_SCENE=18`), 1P (scene `SSB64_START_SCENE=17`) and Bonus use the same C left toggle; check the match keeps the
   classic model (save/load pair).
4. Tag Team: `SSB64_REMIX_MODE=tagteam` with a Sonic slot 0 picked classic on the CSS (the env override acts when the fighter is made, after the queue is built, so it does not set slot 0): the first fighter is classic, after the first KO the log shows the swap and the HUD icon of the classic slot uses the classic stock icon.
5. Training menu (Z menu, Costume item of a Sonic port): values 7 to 12 start the match with classic Sonic, 1 to 6 with modern Sonic.
6. Menu animations: classic Sonic wins a VS match and the results screen plays the classic victory pose (`SSB64_REMIX_CLASSIC_SONIC_P1=1`
   with a Sonic that KOs the CPU); 1P intro for classic Sonic shows the classic 1P pose.
