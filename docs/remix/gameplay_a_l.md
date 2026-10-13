# Smash Remix gameplay hooks, files A to L

Source: `.toolchain/pkgs/smashremix-src/src/*.asm` (top-level only). Decomp: `decomp/src/**/*.c`.

Method (generated, then reviewed):
- Hook = `OS.patch_start(ROM, RAM)`. RAM is the vanilla address patched.
- Decomp match = the `// 0xRAM` address comment with the greatest start at or below RAM, within 0x1000 bytes. `+0xNN` gives the offset inside that function.
- Behavior = the `// @ Description` comment block above the hook's `scope`.
- Toggle = first `Toggles.entry_*` referenced in the hook's scope body (up to the next `scope`). Heuristic.
- Port difficulty = heuristic from the scope body: table/origin refs -> needs table; data directives -> needs new state; otherwise trivial C edit. 'Trivial' only means no table or data in the hook scope; the Remix asm logic still has to be rewritten in C, and 'long asm body' marks scopes over 40 instructions. Spot-check before planning.
- Table origins (`origin` outside `OS.patch_start`) are data/table writes and are listed separately.
- Skipped by scope: Character*, CharacterSelect*, File, BGM, FGM, Menu*, and anything starting after L.

## Summary

| File | Hooks | Table origins |
|---|---|---|
| `src/AA.asm` | 3 | 0 |
| `src/AI.asm` | 70 | 2 |
| `src/Accessibility.asm` | 7 | 0 |
| `src/AerialAttackFastFall.asm` | 5 | 0 |
| `src/AirDodge.asm` | 3 | 0 |
| `src/Blastzone.asm` | 6 | 0 |
| `src/Bonus.asm` | 2 | 1 |
| `src/Boot.asm` | 10 | 0 |
| `src/Camera.asm` | 17 | 0 |
| `src/CharEnvColor.asm` | 5 | 0 |
| `src/ChargeSmashAttacks.asm` | 3 | 1 |
| `src/Cheats.asm` | 5 | 0 |
| `src/Combo.asm` | 1 | 0 |
| `src/ComboMeter.asm` | 1 | 0 |
| `src/Costumes.asm` | 20 | 33 |
| `src/Crash.asm` | 8 | 0 |
| `src/Credits.asm` | 24 | 7 |
| `src/CustomGrabAction.asm` | 3 | 0 |
| `src/DI.asm` | 1 | 0 |
| `src/DKMode.asm` | 1 | 0 |
| `src/Damage.asm` | 5 | 2 |
| `src/DragonKingHUD.asm` | 25 | 0 |
| `src/FD.asm` | 1 | 0 |
| `src/FPS.asm` | 1 | 0 |
| `src/FootStool.asm` | 5 | 0 |
| `src/GFX.asm` | 9 | 3 |
| `src/GFXRoutine.asm` | 3 | 0 |
| `src/Gallery.asm` | 15 | 1 |
| `src/GameEnd.asm` | 1 | 0 |
| `src/HRC.asm` | 7 | 0 |
| `src/Handicap.asm` | 1 | 0 |
| `src/Hazards.asm` | 74 | 0 |
| `src/Hitbox.asm` | 27 | 0 |
| `src/Hitstun.asm` | 1 | 0 |
| `src/InputDelay.asm` | 1 | 0 |
| `src/InputDisplay.asm` | 3 | 0 |
| `src/Item.asm` | 66 | 9 |
| `src/JabLock.asm` | 3 | 0 |
| `src/Japan.asm` | 10 | 0 |
| `src/Joypad.asm` | 3 | 0 |
| `src/KingOfTheHill.asm` | 4 | 0 |
| `src/KirbyHats.asm` | 14 | 1 |
| `src/Knockback.asm` | 4 | 0 |
| `src/LedgeJump.asm` | 3 | 0 |
| `src/LedgeTrump.asm` | 2 | 0 |
| `src/captainshared.asm` | 11 | 0 |
| `src/command.asm` | 9 | 5 |
| `src/dkshared.asm` | 14 | 0 |
| `src/fireball.asm` | 5 | 3 |
| `src/jigglypuffkirbyshared.asm` | 32 | 0 |
| `src/linkshared.asm` | 22 | 0 |

Totals: 55 files scanned, 576 hooks, 564 matched to decomp functions, 68 table origins.

## `src/AA.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/AA.asm:17 | `disable_aa_` | 0x00001CCC | 0x800010CC | `sySchedulerAddPausedQueue`+0x388 (sys/scheduler.c) | This function disables AA if the Disable AA toggle is enabled | `entry_disable_aa` | trivial C edit (long asm body, rewrite logic) |
| src/AA.asm:23 | `disable_aa_` | 0x00001DEC | 0x800011EC | `sySchedulerAddPausedQueue`+0x4A8 (sys/scheduler.c) | This function disables AA if the Disable AA toggle is enabled | `entry_disable_aa` | trivial C edit (long asm body, rewrite logic) |
| src/AA.asm:29 | `disable_aa_` | 0x000033F0 | 0x800027F0 | `sySchedulerSpTaskDone`+0x720 (sys/scheduler.c) | This function disables AA if the Disable AA toggle is enabled | `entry_disable_aa` | trivial C edit |

## `src/AI.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/AI.asm:22 | `force_default_cpu_lvl_setup` | 0x1391AC | 0x8013AF2C | `mnPlayersVSInitVars`+0x64 (mn/mnplayers/mnplayersvs.c) | set the default cpu level 800A3FE8, 800A4D28, 800A4F18 | `entry_default_cpu_level` | trivial C edit |
| src/AI.asm:57 | `size_multiplier_check_` | 0xADE68 | 0x80133428 | `scStaffrollGetPauseStatusHighlight`+0xC (sc/sccommon/scstaffroll.c) | Extend Giant DK check that multiplies the values from attack table | none in scope body (verify) | trivial C edit |
| src/AI.asm:95 | `evade_fix_` | 0xAD640 | 0x80132C00 | `mnPlayersVSUpdateShutter`+0xC (mn/mnplayers/mnplayersvs.c) | CPUs won't evade opponents while they are invulnerable. (super star, respawn) | `entry_improved_ai` | trivial C edit |
| src/AI.asm:134 | `recovery_fix_` | 0x000AFFBC | 0x8013557C | `mnPlayersVSCheckTeamSelectInRange`+0x28 (mn/mnplayers/mnplayersvs.c) | This removes the up b check allowing the CPU to recover multiple times [bit]. | `entry_improved_ai` | trivial C edit |
| src/AI.asm:183 | `fix_target_pos_x` | 0xAFDBC | 0x8013537C | `mnPlayersVSUpdateGameMode`+0x48 (mn/mnplayers/mnplayersvs.c) | If the target is set outside of the blastzone's bounds, move it inwards this is for the CPU to not walk off and SD on stages with walkoffs ftComputerFollowObjectiveWalk(FTStruct *fp) 80134E98+4E4 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:241 | `fix_remix_recovery` | 0xB298C | 0x80137F4C | `ftComputerFollowObjectiveRecover`+0x28 (ft/ftcomputer.c) | Allows remix characters to recover properly. Runs every frame while in the recovery state (off stage) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:385 | `custom_recovery_logic` | 0xB29FC | 0x80137FBC | `mnPlayersVSCheckManFighterSelected`+0x20 (mn/mnplayers/mnplayersvs.c) | 0x80137F24+0x98 | none in scope body (verify) | trivial C edit |
| src/AI.asm:444 | `random_teching_` | 0x000BB3C0 | 0x80140980 | `ftCommonDamageAirCommonProcMap`+0x44 (ft/ftcommon/ftcommondamage.c) | (no description comment) | `entry_single_button_mode`, `entry_improved_ai` | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:457 | `random_teching_` | 0x000BE034 | 0x801435F4 | `ftCommonDamageFallProcMap`+0x44 (ft/ftcommon/ftcommondamagefall.c) | (no description comment) | `entry_single_button_mode`, `entry_improved_ai` | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:668 | `z_cancel_` | 0x000CB478 | 0x80150A38 | `ftCommonAttackAirProcMap`+0x30 (ft/ftcommon/ftcommonattackair.c) | Usually, this function checks for a z-cancel press with 10 frames. At the end of this, at holds a boolean for successful z-cancel. This function has been modified to make sure that boolean is true for CPUs (Z_CANCEL_CHANCE)% of the time. [b | `entry_improved_ai`, `entry_single_button_mode` | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:872 | `charge_less_often` | 0xB146C | 0x80136A2C | `ftComputerCheckTryChargeSpecialN`+0xC (ft/ftcomputer.c) | Makes LV10 CPUs charge NSP less often Attempt to make them more active and fun to fight ftComputerCheckTryChargeSpecialN 80136A20 + C | none in scope body (verify) | trivial C edit |
| src/AI.asm:924 | `improve_remix_specials` | 0xB1494 | 0x80136A54 | `mnPlayers1PTrainingPuckAdjustPlaced`+0x10 (mn/mnplayers/mnplayers1ptraining.c) | Improves cpu ability to utilize charge attacks ftComputerCheckTryChargeSpecialN 80136A20 + 34 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:1034 | `improve_remix_charged_NSP_1` | 0xB1680 | 0x80136C40 | `mnVSResultsOrderResults`+0x14 (mn/mnvsmode/mnvsresults.c) | Add Remix characters to charge NSP cancel check in ftComputerCheckTryCancelSpecialN Cancels charge by pressing Z 80136C0C + 34 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:1131 | `improve_remix_charged_NSP_2` | 0xB3664 | 0x80138C24 | `mnPlayersVSMakePuckCamera`+0x18 (mn/mnplayers/mnplayersvs.c) | Samus checks the opposing fighter if they are capable of reflecting/absorb before shooting Sometimes this check is ignored by the AI | none in scope body (verify) | trivial C edit |
| src/AI.asm:1181 | `improve_remix_charged_NSP_3` | 0xB3684 | 0x80138C44 | `mnPlayersVSMakePuckCamera`+0x38 (mn/mnplayers/mnplayersvs.c) | jkirby added to kirby hat check remix characters added to samus check, should improve the rate they attempt a full charge attack | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:1300 | `remix_ignore_projectiles_` | 0xB2224 | 0x801377E4 | `mnVSResultsAudioThreadUpdate`+0x24 (mn/mnvsmode/mnvsresults.c) | Improves cpu ability to not care about or dodge projectiles hook after M.MARIO/GDK id check Used for Franklin Badge cpus and GBOWSER | none in scope body (verify) | trivial C edit |
| src/AI.asm:1342 | `usp_check_` | 0xACB80 | 0x80132140 | `sc1PStageClearMakeTextSObjs`+0x4 (sc/sc1pmode/sc1pstageclear.c) | Hook to routine that performs id check for fox. Extended to check for clones best when action ids are shared seems to run every aerial frame | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:1402 | `recovery_stick_y_fix` | 0xACF7C | 0x8013253C | `mvEndingEjectRoomGObjs` (mv/mvending/mvending.c) | fixed Goemons recovery so he always points upwards this hook checks ever frame for goemon cpus | none in scope body (verify) | needs new state |
| src/AI.asm:2198 | `extend_commands_1` | 0xAD174 | 0x80132734 | `mn1PModeMakeOptionsCamera`+0x4 (mn/mn1pmode/mn1pmode.c) | modifies a hard-coded routine which gets the ptr to an ai's command | none in scope body (verify) | trivial C edit |
| src/AI.asm:2216 | `extend_commands_2` | 0xAD378 | 0x80132938 | `mnBackupClearUpdateOptionConfirmMenu`+0x8 (mn/mnoption/mnbackupclear.c) | another hard-coded place | none in scope body (verify) | trivial C edit |
| src/AI.asm:2234 | `extend_commands_3` | 0xAD1A0 | 0x80132760 | `func_ovl60_8013275C`+0x4 (mn/mnoption/mnoption.c) | another hard-coded place | none in scope body (verify) | trivial C edit |
| src/AI.asm:2255 | `extend_stick_x_commands` | 0xAC7E4 | 0x80131DA4 | `mnVSRecordGetPowerOf`+0x4 (mn/mndata/mnvsrecord.c) | Extend stick commands to add new options By default there are 2 special numbers for stick towards target (full, half) Add commands for stick away from target too 80131C68 + 13C | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:2474 | `extend_routines` | 0x1065BC | 0x8018BB7C | `void`+0x80C (it/itmonster/itpippi.c) | (no description comment) | none in scope body (verify) | needs new state |
| src/AI.asm:2503 | `extend_polygon_check_` | 0xAE2C0 | 0x80133880 | `mnVSOptionsInitVars`+0x8 (mn/mnvsmode/mnvsoptions.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/AI.asm:2544 | `extend_polygon_check_` | 0xB36F8 | 0x80138CB8 | `mnPlayersVSMakeHandicapLevelCamera`+0xC (mn/mnplayers/mnplayersvs.c) | Extends a check that is meant to exclude polygons specifically for determining behaviour when a fighter is far away a0 = character ID | none in scope body (verify) | trivial C edit |
| src/AI.asm:2569 | `custom_move` | 0xB38E0 | 0x80138EA0 | `mnVSResultsStartScene`+0x3C (mn/mnvsmode/mnvsresults.c) | 80138AA8 + 3F8 | none in scope body (verify) | trivial C edit |
| src/AI.asm:2621 | `custom_move` | 0xB3764 | 0x80138D24 | `mnPlayersVSMakeHandicapLevelCamera`+0x78 (mn/mnplayers/mnplayersvs.c) | 80138AA8 + 3F8 | none in scope body (verify) | trivial C edit |
| src/AI.asm:3547 | `custom_attacks_table` | 0xADB7C | 0x8013313C | `mnVSResultsGetPlace`+0x8 (mn/mnvsmode/mnvsresults.c) | 80132EC8+274 This allows us to have a unique attack table for lv10 CPUs. Could be expanded to other unique tables too. This is where it loads the entry in the table, so we can override the value or keep the original. The current goal here i | none in scope body (verify) | trivial C edit |
| src/AI.asm:3585 | `auto_jab_grab_` | 0xC48BC | 0x80149E7C | `ftCommonCatchCheckInterruptAttack11`+0x58 (ft/ftcommon/ftcommoncatch1.c) | Hook where jab subroutine checks if player has input a grab during frame 1-2 of jab | `entry_single_button_mode` | trivial C edit |
| src/AI.asm:3636 | `check_invulnerable_` | 0xAD6C4 | 0x80132C84 | `sc1PStageClearMakeTargetTextSObjs`+0x4 (sc/sc1pmode/sc1pstageclear.c) | Run away from opponents who are coming off the respawn plat However, vanilla AI doesn't seem too great at evading. | none in scope body (verify) | trivial C edit |
| src/AI.asm:3663 | `extend_cliff_attack_` | 0xB1844 | 0x80136E04 | `mnPlayers1PTrainingMakePuckAdjust`+0x2C (mn/mnplayers/mnplayers1ptraining.c) | Allows for additional ways to get up a cliff | none in scope body (verify) | trivial C edit |
| src/AI.asm:3690 | `taunt_replace_` | 0xB2194 | 0x80137754 | `mnPlayers1PTrainingFuncRun`+0x54 (mn/mnplayers/mnplayers1ptraining.c) | Instead of taunting, do something else or nothing | none in scope body (verify) | trivial C edit |
| src/AI.asm:3733 | `cpu_post_process` | 0xB52C4 | 0x8013A884 | `ftComputerProcessAll`+0x50 (ft/ftcomputer.c) | ftComputerProcessAll 8013A834+50 This runs after ftComputerProcessTrait, ftComputerProcessBehavior, ftComputerProcessObjective and before ftComputerUpdateInputs which is all updates the CPU has in a frame This patch lets us add one more rou | none in scope body (verify) | trivial C edit |
| src/AI.asm:3892 | `force_objective_attack` | 0xB5158 | 0x8013A718 | `mnPlayersVSSetSceneData`+0xB4 (mn/mnplayers/mnplayersvs.c) | ftComputerProcessTrait 8013A63C+DC Here, from time to time the game does rand(3) and sets behavior to either default, unk2, ally, captain lv10 forces the result to 0 (default) so the objective keeps being "attack" | none in scope body (verify) | trivial C edit |
| src/AI.asm:3922 | `force_objective_attack` | 0xB5284 | 0x8013A844 | `ftComputerProcessAll`+0x10 (ft/ftcomputer.c) | ftComputerProcessTrait 8013A63C+DC Here, from time to time the game does rand(3) and sets behavior to either default, unk2, ally, captain lv10 forces the result to 0 (default) so the objective keeps being "attack" | none in scope body (verify) | trivial C edit |
| src/AI.asm:4007 | `_fix_weird_air_drift_right` | 0xAEA98 | 0x80134058 | `mnCharactersStartScene`+0x8 (mn/mndata/mncharacters.c) | 80134000+58 ftComputerCheckSetTargetEdgeRight here when the CPU has positive Y speed (going up) it sets the target to itself + a value in X which makes it just drift to this direction for no apparent reason | none in scope body (verify) | trivial C edit |
| src/AI.asm:4033 | `_fix_weird_air_drift_left` | 0xAEE00 | 0x801343C0 | `mnPlayersVSMakeLabels`+0x10 (mn/mnplayers/mnplayersvs.c) | read comment for the above patch 80134368+58 | none in scope body (verify) | trivial C edit |
| src/AI.asm:4337 | `_improve_recovery_drift` | 0xAFE40 | 0x80135400 | `mnVSResultsGetNumberColorID`+0xC (mn/mnvsmode/mnvsresults.c) | This patch improves the CPU's drift when recovering. It separates by situation (above ledge, under ledge, facing ledge, not facing ledge). Takes the character's ledge grab range in consideration. Also makes them less prone to getting pineap | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:4799 | `prevent_fastfall` | 0xB0144 | 0x80135704 | `mnPlayers1PTrainingRecallPuck`+0x34 (mn/mnplayers/mnplayers1ptraining.c) | CPUs will often fastfall and end up SDing because of it so we prevent them from fastfalling when they have no jumps 80134E98+86C | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:4869 | `detect_offstage_opponents` | 0xAD458 | 0x80132A18 | `mnPlayersVSMakeNameAndEmblem`+0x4 (mn/mnplayers/mnplayersvs.c) | ftComputerCheckFindTarget 8013295C+BC | none in scope body (verify) | trivial C edit |
| src/AI.asm:4903 | `clamp_target_pos_to_stage` | 0xAD524 | 0x80132AE4 | `mvOpeningRoomMakeDeskGround` (mv/mvopening/mvopeningroom.c) | ftComputerCheckFindTarget 8013295C+188 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:4987 | `shorthop_followup_attack` | 0xB0558 | 0x80135B18 | `mnPlayersVSCheckPlayerKindSelectInRange`+0x60 (mn/mnplayers/mnplayersvs.c) | ftComputerFollowObjectiveWalk 80134E98+c80 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:5065 | `jump_in_neutral` | 0xB04FC | 0x80135ABC | `mnPlayersVSCheckPlayerKindSelectInRange`+0x4 (mn/mnplayers/mnplayersvs.c) | the CPU is grounded com->dash_predict <= DISTANCE(com->target_pos.x, fp->joints[nFTPartsJointTopN]->translate.vec.f.x) the CPU is not below the opponent Here, the CPU would just walk towards the target ftComputerFollowObjectiveWalk 80134E98 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:5136 | `extend_attack_check_range` | 0xB3DB4 | 0x80139374 | `mnPlayersVSPuckAdjustOverlap`+0x54 (mn/mnplayers/mnplayersvs.c) | ftComputerFollowObjectiveAttack 801392C8+AC | none in scope body (verify) | trivial C edit |
| src/AI.asm:5158 | `extend_attack_check_range_2` | 0xB42E0 | 0x801398A0 | `func_ovl3_801397F4`+0xAC (ft/ftcomputer.c) | ftComputerFollowObjectiveAttack 801397F4+AC | none in scope body (verify) | trivial C edit |
| src/AI.asm:5183 | `fix_movement_prediction` | 0xADBC4 | 0x80133184 | `scStaffrollMakeCompanyTextGObj` (sc/sccommon/scstaffroll.c) | 80132EC8+2BC Vanilla does some weird decisions when predicting the opponent's position based on their movement In X, it uses the current facing direction when it doesn't affect the opponent's trajectory In Y, it uses gravity for the opponen | none in scope body (verify) | trivial C edit |
| src/AI.asm:5448 | `fix_multi_doublejump_usage` | 0xAFF90 | 0x80135550 | `mnPlayers1PGameCheckTimeArrowRInRange`+0x84 (mn/mnplayers/mnplayers1pgame.c) | CPUs will wait to be in nFTCommonStatusFall when using multiple double jumps, resulting in them falling more than necessary and SDing here we add a check if the current update function is ftCommonJumpAerialProcUpdate (used for any kind of a | none in scope body (verify) | trivial C edit |
| src/AI.asm:5487 | `weight_attack_options` | 0xAE1C4 | 0x80133784 | `mnTitleFireCameraProcUpdate`+0x14 (mn/mncommon/mntitle.c) | 80132EC8+8BC | none in scope body (verify) | trivial C edit |
| src/AI.asm:5944 | `skip_repeated_input_check` | 0xAE590 | 0x80133B50 | `sc1PStageClearMakeTextCamera`+0x8 (sc/sc1pmode/sc1pstageclear.c) | 80132EC8+C88 Here the CPU tracks recently used inputs and changes the perceived range on repeated use since we add different options for inputs, we're going to skip this check | none in scope body (verify) | trivial C edit |
| src/AI.asm:5980 | `skip_repeated_input_skip` | 0xAE8D8 | 0x80133E98 | `mnVSResultsMakePlayerTagCamera`+0x1C (mn/mnvsmode/mnvsresults.c) | 80132EC8+FD0 Here we already have the input for the CPU to use but before using it, the CPU checks if this same move was used 4 times in a row if so, it skips the input and jumps instead | none in scope body (verify) | trivial C edit |
| src/AI.asm:6016 | `no_attack_when_all_odds_eq_zero` | 0xAEA04 | 0x80133FC4 | `mnPlayers1PTrainingPortraitFlashThreadUpdate`+0x10 (mn/mnplayers/mnplayers1ptraining.c) | 80132EC8+10FC Here we already have the input for the CPU to use but before using it, the CPU checks if this same move was used 4 times in a row if so, it skips the input and jumps instead | none in scope body (verify) | trivial C edit |
| src/AI.asm:6040 | `stop_roll_spam_` | 0xB3CD4 | 0x80139294 | `mnPlayersVSMakePuck`+0x1FC (mn/mnplayers/mnplayersvs.c) | Adds alternate options for LVL 10 cpus to use instead of rolling back and forth. Not perfect but better than roll spamming | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:6044 | `stop_roll_spam_` | 0xB3CE4 | 0x801392A4 | `mnPlayersVSMakePuck`+0x20C (mn/mnplayers/mnplayersvs.c) | Adds alternate options for LVL 10 cpus to use instead of rolling back and forth. Not perfect but better than roll spamming | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:6366 | `shield_drop_` | 0xB057C | 0x80135B3C | `mnPlayers1PGameAnnounceFighter`+0xC (mn/mnplayers/mnplayers1pgame.c) | Allows cpus to perform a shield drop instead of a normal plat drop | none in scope body (verify) | trivial C edit |
| src/AI.asm:6397 | `shield_drop_` | 0xB04FC | 0x80135ABC | `mnPlayersVSCheckPlayerKindSelectInRange`+0x4 (mn/mnplayers/mnplayersvs.c) | Allows cpus to perform a shield drop instead of a normal plat drop | none in scope body (verify) | trivial C edit |
| src/AI.asm:6534 | `cpu_di_` | 0xB4E08 | 0x8013A3C8 | `mnPlayersVSSetPlayerNot`+0x1C (mn/mnplayers/mnplayersvs.c) | Adds DI to CPU behavior | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/AI.asm:6823 | `handle_right_arrow_press_` | 0x133AC8 | 0x80135848 | `mnPlayers1PTrainingDetectBack`+0x30 (mn/mnplayers/mnplayers1ptraining.c) | Handles right arrow press | none in scope body (verify) | trivial C edit |
| src/AI.asm:6842 | `handle_right_arrow_press_` | 0x134D44 | 0x80136AC4 | `mnVSResultsSetKOs`+0x28 (mn/mnvsmode/mnvsresults.c) | Handles right arrow press | none in scope body (verify) | trivial C edit |
| src/AI.asm:6849 | `render_right_arrow_` | 0x134DAC | 0x80136B2C | `mnPlayers1PBonusInitPlayer`+0x8 (mn/mnplayers/mnplayers1pbonus.c) | 0x80136B2C Check to determine if right arrow should be rendered | none in scope body (verify) | trivial C edit |
| src/AI.asm:6877 | `render_10_` | 0x135210 | 0x80136F90 | `mnPlayers1PTrainingMakeCostumeSync`+0xC (mn/mnplayers/mnplayers1ptraining.c) | Renders 10 for CPU Level or the appropriate Handicap indicators | none in scope body (verify) | needs new state |
| src/AI.asm:6883 | `render_10_` | 0x135270 | 0x80136FF0 | `mnPlayers1PTrainingSpotlightProcUpdate`+0x28 (mn/mnplayers/mnplayers1ptraining.c) | Renders 10 for CPU Level or the appropriate Handicap indicators | none in scope body (verify) | needs new state |
| src/AI.asm:7105 | `clamp_level_9_1` | 0xACFD0 | 0x80132590 | `mvEndingSetupOperatorCamera` (mv/mvending/mvending.c) | occurs often 80132564+2C | none in scope body (verify) | trivial C edit |
| src/AI.asm:7122 | `clamp_level_9_2` | 0xB2ED0 | 0x80138490 | `func_ovl3_8013837C`+0x114 (ft/ftcomputer.c) | occurs often | none in scope body (verify) | trivial C edit |
| src/AI.asm:7140 | `clamp_level_9_3` | 0xAD0A4 | 0x80132664 | `mnVSModeAnimateRuleArrows`+0x8 (mn/mnvsmode/mnvsmode.c) | while aerial against an opponent 80132564+100 | none in scope body (verify) | trivial C edit |
| src/AI.asm:7157 | `clamp_level_9_4` | 0xB3568 | 0x80138B28 | `func_ovl3_80138AA8`+0x80 (ft/ftcomputer.c) | ? | none in scope body (verify) | trivial C edit |
| src/AI.asm:7174 | `clamp_level_9_5` | 0xAD1E4 | 0x801327A4 | `scStaffrollMakeHighlightGObj` (sc/sccommon/scstaffroll.c) | ? | none in scope body (verify) | trivial C edit |
| src/AI.asm:7191 | `clamp_level_9_6` | 0xAD2C4 | 0x80132884 | `mnPlayersVSUpdateTeamSelect`+0xC (mn/mnplayers/mnplayersvs.c) | ? | none in scope body (verify) | trivial C edit |
| src/AI.asm:7208 | `clamp_level_9_7` | 0xADAA0 | 0x80133060 | `sc1PStageClearCheckHaveBonusStats`+0x4 (sc/sc1pmode/sc1pstageclear.c) | while near an opponent | none in scope body (verify) | trivial C edit |
| src/AI.asm:7230 | `clamp_level_9_8` | 0xB17AC | 0x80136D6C | `mnPlayers1PGameCursorProcUpdate`+0x14 (mn/mnplayers/mnplayers1pgame.c) | while hanging on cliff | none in scope body (verify) | trivial C edit |
| src/AI.asm:7256 | `clamp_level_9_9` | 0xB340C | 0x801389CC | `mnPlayersVSPuckProcUpdate`+0xD4 (mn/mnplayers/mnplayersvs.c) | 8013877C + 250 compares some timer to ((-this_fp->level * 2) + 18)). At lv9 this results in 0, but at level 10 it's -2. Clamp. | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/AI.asm:1574 | `recovery_stick_y_fix` | `(AI.TABLE_ORIGIN + (AI.NUM_VANILLA_CPU_INPUT_ROUTINES * 0x4) + (AI.NUM_ADDED_CPU_INPUT_ROUTINES * 0x4))` | data/table write, no RAM hook; decomp n/a |
| src/AI.asm:2790 | `ENDA` | `{table_entry}` | data/table write, no RAM hook; decomp n/a |

## `src/Accessibility.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Accessibility.asm:18 | `flash_guard` | 0x91610 | 0x80115E10 | `ifScreenFlashMakeInterface`+0x28 (if/ifscreenflash.c) | Reimplements the unused 'Anti-Flash' feature. This disables certain screen flashes (hard hits, Zebes acid, barrel etc) Vanilla reads value stored at 0x800A4930 which is always 1 (no option to toggle) Note: included checks in DekuNut.asm and | `entry_flash_guard` | trivial C edit |
| src/Accessibility.asm:37 | `ParamMakeEffect` | 0x663F4 | 0x800EABF4 | `ftParamMakeEffect`+0x18 (ft/ftparam.c) | Store structure for use later | none in scope body (verify) | needs new state |
| src/Accessibility.asm:42 | `ParamMakeEffect` | 0x66B88 | 0x800EB388 | `ftParamMakeEffect`+0x7AC (ft/ftparam.c) | Store structure for use later | none in scope body (verify) | needs new state |
| src/Accessibility.asm:67 | `screenshake_toggle` | 0x7C164 | 0x80100964 | `efManagerQuakeMakeEffect`+0x70 (ef/efmanager.c) | This handles screenshake intensity, which can be lowered or turned off altogether. | `entry_screenshake` | trivial C edit |
| src/Accessibility.asm:112 | `blastzone_explode_gfx` | 0x7D9C4 | 0x801021C4 | `efManagerDeadExplodeMakeEffect`+0x4 (ef/efmanager.c) | Handle the Blast Zone explosion graphic effect | `entry_puff_sing_anim`, `entry_blastzone_gfx` | trivial C edit (long asm body, rewrite logic) |
| src/Accessibility.asm:134 | `blastzone_explode_gfx` | 0x7DACC | 0x801022CC | `efManagerDeadExplodeMakeEffect`+0x10C (ef/efmanager.c) | Handle the Blast Zone explosion graphic effect | `entry_blastzone_gfx` | trivial C edit (long asm body, rewrite logic) |
| src/Accessibility.asm:164 | `blastzone_explode_gfx` | 0x7DA18 | 0x80102218 | `efManagerDeadExplodeMakeEffect`+0x58 (ef/efmanager.c) | Handle the Blast Zone explosion graphic effect | `entry_blastzone_gfx` | trivial C edit |

## `src/AerialAttackFastFall.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/AerialAttackFastFall.asm:25 | `fast_fall_check` | 0xA5638 | 0x80129E38 | unmatched | This routine checks if the remix toggle is enabled | none in scope body (verify) | needs new state |
| src/AerialAttackFastFall.asm:28 | `fast_fall_check` | 0xA564C | 0x80129E4C | unmatched | This routine checks if the remix toggle is enabled | none in scope body (verify) | needs new state |
| src/AerialAttackFastFall.asm:31 | `fast_fall_check` | 0xA5660 | 0x80129E60 | unmatched | This routine checks if the remix toggle is enabled | none in scope body (verify) | needs new state |
| src/AerialAttackFastFall.asm:34 | `fast_fall_check` | 0xA5674 | 0x80129E74 | unmatched | This routine checks if the remix toggle is enabled | none in scope body (verify) | needs new state |
| src/AerialAttackFastFall.asm:37 | `fast_fall_check` | 0xA5688 | 0x80129E88 | unmatched | This routine checks if the remix toggle is enabled | none in scope body (verify) | needs new state |

## `src/AirDodge.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/AirDodge.asm:65 | `reset_air_dash_grounded_` | 0x59C20 | 0x800DE420 | `mpCommonSetFighterLandingParams`+0xB8 (mp/mpcommon.c) | Resets air dash boolean for this player port when grounded script occurs v0 = player struct | none in scope body (verify) | trivial C edit |
| src/AirDodge.asm:84 | `reset_air_dash_initial_` | 0x535B4 | 0x800D7DB4 | `ftManagerInitFighter`+0x3C4 (ft/ftmanager.c) | Resets air dash boolean for this player port when initial script occurs v1 = player struct | none in scope body (verify) | trivial C edit |
| src/AirDodge.asm:110 | `airdodge_check` | 0xBABEC | 0x801401AC | `ftCommonJumpAerialCheckInterruptCommon`+0x10 (ft/ftcommon/ftcommonjumpaerial.c) | Hook that is placed in routine that checks if player inputs a jump | `entry_air_dodge`, `entry_single_button_mode` | trivial C edit |

## `src/Blastzone.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Blastzone.asm:91 | `skip_bottom_blast_sfx` | 0x5DB4C | 0x800E234C | `ftMainProcPhysicsMap`+0x304 (ft/ftmain.c) | skip bottom blast zone sound | none in scope body (verify) | trivial C edit |
| src/Blastzone.asm:112 | `loop_screen_right_` | 0xB78DC | 0x8013CE9C | `ftCommonDeadCheckInterruptCommon`+0x320 (ft/ftcommon/ftcommondead.c) | hook over routine that KOS players on right side of screen | none in scope body (verify) | trivial C edit |
| src/Blastzone.asm:165 | `loop_screen_left_` | 0xB790C | 0x8013CECC | `ftCommonDeadCheckInterruptCommon`+0x350 (ft/ftcommon/ftcommondead.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Blastzone.asm:213 | `loop_screen_top_1_` | 0xB7960 | 0x8013CF20 | `ftCommonDeadCheckInterruptCommon`+0x3A4 (ft/ftcommon/ftcommondead.c) | hook over routine that KOS players on top side of screen | none in scope body (verify) | trivial C edit |
| src/Blastzone.asm:267 | `loop_screen_top_2_` | 0xB7970 | 0x8013CF30 | `ftCommonDeadCheckInterruptCommon`+0x3B4 (ft/ftcommon/ftcommondead.c) | hook over routine that KOS players on top side of screen | none in scope body (verify) | trivial C edit |
| src/Blastzone.asm:322 | `loop_screen_bottom_` | 0xB78A8 | 0x8013CE68 | `ftCommonDeadCheckInterruptCommon`+0x2EC (ft/ftcommon/ftcommondead.c) | hook before player is KO'd at bottom blast zone | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |

## `src/Bonus.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Bonus.asm:228 | `handle_header_toggle_` | 0x149388 | 0x80133358 | `mnPlayers1PTrainingCheckCostumeUsed`+0x8 (mn/mnplayers/mnplayers1ptraining.c) | This toggles the CSS objects when cycling between Bonus 1/2/3 accordingly | none in scope body (verify) | trivial C edit |
| src/Bonus.asm:414 | `manual_remix_bonus_start_` | 0x0014CABC | 0x80136A8C | `mnPlayers1PGameCheckStockArrowPress`+0x8 (mn/mnplayers/mnplayers1pgame.c) | Disables automatic start after selecting character (in Remix Mode) | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/Bonus.asm:868 | `get_bonus_stage_index_` | `REMIX_BONUS_HIGH_SCORE_TABLE_ORIGIN` | data/table write, no RAM hook; decomp n/a |

## `src/Boot.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Boot.asm:18 | `Boot` | 0x0017EE54 | 0x80131C94 | `mvEndingMakeRoomBooks` (mv/mvending/mvending.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Boot.asm:37 | `splash_` | 0x17EA6C | 0x8013205C | `mnVSResultsGetPresentCount` (mn/mnvsmode/mnvsresults.c) | After the Nintendo 64 logo exits we display the 1p victory screen but we want it to display a different image than the Mario victory image, so we set the character ID to NONE which will never occur after beating 1p mode. See SinglePlayer.re | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Boot.asm:42 | `splash_` | 0x17E958 | 0x80131F48 | `mnBackupClearEjectOptionGObjs`+0x10 (mn/mnoption/mnbackupclear.c) | After the Nintendo 64 logo exits we display the 1p victory screen but we want it to display a different image than the Mario victory image, so we set the character ID to NONE which will never occur after beating 1p mode. See SinglePlayer.re | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Boot.asm:47 | `splash_` | 0x17E650 | 0x80131C40 | `sc1PChallengerMakeDecals` (sc/sc1pmode/sc1pchallenger.c) | After the Nintendo 64 logo exits we display the 1p victory screen but we want it to display a different image than the Mario victory image, so we set the character ID to NONE which will never occur after beating 1p mode. See SinglePlayer.re | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Boot.asm:53 | `splash_` | 0x17EA2C | 0x8013201C | `mnCongraStartScene`+0x10 (mn/mncommon/mncongra.c) | After the Nintendo 64 logo exits we display the 1p victory screen but we want it to display a different image than the Mario victory image, so we set the character ID to NONE which will never occur after beating 1p mode. See SinglePlayer.re | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Boot.asm:173 | `draw_version_on_title_screen_` | 0x1198EC | 0x8013426C | `mnVSResultsMakeWinnerText`+0x30 (mn/mnvsmode/mnvsresults.c) | Draws the version on the title screen | none in scope body (verify) | needs new state |
| src/Boot.asm:180 | `draw_version_on_title_screen_` | 0x1198F8 | 0x80134278 | `mnPlayers1PBonusMakeFighterCamera`+0x4 (mn/mnplayers/mnplayers1pbonus.c) | Draws the version on the title screen | none in scope body (verify) | needs new state |
| src/Boot.asm:188 | `draw_version_on_title_screen_` | 0x0017EE18 | 0x80131C58 | `func_ovl24_80131C58` (sc/sc1pmode/sc1pintro.c) | Draws the version on the title screen | none in scope body (verify) | trivial C edit |
| src/Boot.asm:196 | `load_` | 0x00001234 | 0x80000634 | `syMainThread5`+0x5C (sys/main.c) | Performs one DMA as part of the boot sequence. It transfers 0x400000 bytes to 0x80400000. | none in scope body (verify) | trivial C edit |
| src/Boot.asm:201 | `load_` | 0x00001038 | 0x80000438 | unmatched | Performs one DMA as part of the boot sequence. It transfers 0x400000 bytes to 0x80400000. | none in scope body (verify) | trivial C edit |

## `src/Camera.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Camera.asm:46 | `override_camera_type_` | 0x892A4 | 0x8010DAA4 | `gmCameraMakeDefaultCamera`+0x2BC (gm/gmcamera.c) | Hook in vanilla routine during camera set that sets the camera routine index (we're here if not Planet Zebes or Mushroom Kingdom) Wide/Walk off stages tend to look better with this camera due to less skewing | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:108 | `set_initial_coordinates_` | 0x8928C | 0x8010DA8C | `gmCameraMakeDefaultCamera`+0x2A4 (gm/gmcamera.c) | allows us to immediately replace camera values upon camera creation. runs once. TODO: replace with lookup table OR use bonus pause cam coordinates from editor (would have to add a check to see if the values != 0 first) | `entry_camera_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:462 | `fixed_camera_boss_` | 0x883EC | 0x8010CBEC | `gmCameraUpdateAnimVel`+0x8 (gm/gmcamera.c) | skip cutscene from changing camera angle during master hands entry for fixed camera mode. | `entry_camera_mode` | trivial C edit |
| src/Camera.asm:486 | `cinematic_entry_` | 0x0008E250 | 0x80112A50 | `ifCommonEntryAllThread`+0x1C (if/ifcommon.c) | This catches a call to Global.get_random_int and sets the result depending on the cinematic entry toggle value. | `entry_camera_mode`, `entry_cinematic_entry` | trivial C edit |
| src/Camera.asm:522 | `fixed_camera_always_` | 0x88710 | 0x8010CF10 | `gmCameraSetStatus`+0x1C (gm/gmcamera.c) | Don't allow camera to be changed with the fixed camera Modifies cmManagerSetCameraStatusID() | `entry_camera_mode`, `entry_hazard_mode` | trivial C edit |
| src/Camera.asm:572 | `prevent_fixed_camera_pause_bonus_` | 0x8F918 | 0x80114118 | `ifCommonBattleGoUpdateInterface`+0xDC (if/ifcommon.c) | Prevents Bonus 1/2 pausing in fixed camera mode | `entry_camera_mode` | needs new state |
| src/Camera.asm:593 | `prevent_fixed_camera_pause_bonus_` | 0x000AC494 | 0x80130C94 | unmatched | Prevents Bonus 1/2 pausing in fixed camera mode | none in scope body (verify) | needs new state |
| src/Camera.asm:605 | `vs_pause_pan_` | 0x000881CC | 0x8010C9CC | `gmCameraUpdatePlayerZoom`+0x6C (gm/gmcamera.c) | adds camera up/down/view distance/fov movement when player can control pause camera | none in scope body (verify) | trivial C edit |
| src/Camera.asm:650 | `pause_initial_` | 0x0008F9B4 | 0x801141B4 | `ifCommonBattleGoUpdateInterface`+0x178 (if/ifcommon.c) | clears x,y,z offset values for custom vs camera panning | `entry_camera_mode` | needs new state |
| src/Camera.asm:845 | `unpause_camera_` | 0x8FE38 | 0x80114638 | `ifCommonBattlePauseRestoreInterfaceAll`+0xB0 (if/ifcommon.c) | routine runs once when game is unpaused. Disables hud from drawing if camera mode = SCENE or entry_disable_hud = ALL | `entry_camera_mode`, `entry_disable_hud` | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:917 | `disable_hud_` | 0x8DA28 | 0x80112228 | `ifCommonAnnounceGoSetStatus`+0x64 (if/ifcommon.c) | Routine runs after "3, 2, 1 GO!". Disables HUD from drawing if entry_disable_hud = ALL | `entry_disable_hud` | trivial C edit |
| src/Camera.asm:947 | `frozen_camera_` | 0x87DA8 | 0x8010C5A8 | `gmCameraPan`+0x4C (gm/gmcamera.c) | Subroutine which freezes the camera Replaces a JAL to subroutine to 0x80018FBC which is used to update the camera''s position. | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:951 | `frozen_camera_` | 0x87E58 | 0x8010C658 | `func_ovl2_8010C5C0`+0x98 (gm/gmcamera.c) | Subroutine which freezes the camera Replaces a JAL to subroutine to 0x80018FBC which is used to update the camera''s position. | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:955 | `frozen_camera_` | 0x87EC8 | 0x8010C6C8 | `gmCameraApplyVel`+0x10 (gm/gmcamera.c) | Subroutine which freezes the camera Replaces a JAL to subroutine to 0x80018FBC which is used to update the camera''s position. | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:959 | `frozen_camera_` | 0x880AC | 0x8010C8AC | `gmCameraUpdateAcidZoom`+0xDC (gm/gmcamera.c) | Subroutine which freezes the camera Replaces a JAL to subroutine to 0x80018FBC which is used to update the camera''s position. | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Camera.asm:1343 | `camera_adjust_` | 0x62A70 | 0x800E7270 | `ftMainSetStatus`+0x34C (ft/ftmain.c) | This pushes back the camera when playing on Venom or GB Land. | none in scope body (verify) | trivial C edit |
| src/Camera.asm:1389 | `pause_disable_control_flag_` | 0x8FA30 | 0x80114230 | `ifCommonBattleGoUpdateInterface`+0x1F4 (if/ifcommon.c) | hook at routine that reads a flag to determine if player is off screen for pause routine. if flag = 0, then analog stick doesnt draw on top of screen. | `entry_camera_mode`, `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |

## `src/CharEnvColor.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/CharEnvColor.asm:950 | `override_env_color_` | 0x781DC | 0x800FC9DC | `mpCollisionSetLightColorGetAlpha`+0x14 (mp/mpcollision.c) | This checks the override table before reading the default env color. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/CharEnvColor.asm:956 | `override_env_color_` | 0x107B28 | 0x80390548 | `scSubsysFighterDrawLightColorGetAlpha`+0x14 (sc/scsubsys/scsubsysfighter.c) | This checks the override table before reading the default env color. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/CharEnvColor.asm:1380 | `reset_custom_display_lists_during_load_` | 0x52EA4 | 0x800D76A4 | `ftManagerSetupFilesMainKind`+0x10 (ft/ftmanager.c) | Resets custom display lists during main character file loading. | none in scope body (verify) | trivial C edit |
| src/CharEnvColor.asm:1659 | `update_player_indicator_alpha_` | 0x8D33C | 0x80111B3C | `ifCommonPlayerTagProcDisplay`+0x100 (if/ifcommon.c) | Updates player indicator to respect the env color's alpha. | none in scope body (verify) | trivial C edit |
| src/CharEnvColor.asm:1694 | `initialize_override_table_` | 0x534BC | 0x800D7CBC | `ftManagerInitFighter`+0x2CC (ft/ftmanager.c) | Populates our override table when initializing the character based on state. | none in scope body (verify) | trivial C edit |

## `src/ChargeSmashAttacks.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/ChargeSmashAttacks.asm:47 | `smash_attack_main` | 0x54CD0 | 0x800D94D0 | `ftAnimEndSetWait`+0xC (ft/ftanimend.c) | Main subroutine that deals with charge smash attacks | none in scope body (verify) | trivial C edit |
| src/ChargeSmashAttacks.asm:298 | `custom_shuffle` | 0x6E674 | 0x800F2E74 | `ftDisplayMainProcDisplay`+0x538 (ft/ftdisplaymain.c) | ftDisplayMainProcDisplay(GObj *fighter_gobj) 0x800F293C+0x538 This function handles fighter display; at this point it is about to load the array used for the shuffle/vibration/shake that happens during hitlag when hit The game has 2 arrays  | none in scope body (verify) | needs table |
| src/ChargeSmashAttacks.asm:338 | `custom_shuffle_item` | 0x459A0 | 0x800C9FC0 | `func_ovl0_800C9F70`+0x50 (lb/lbcommon.c) | Unnamed function 800C9F70 This updates item positions for shuffle on hitlag 800C9F70+50 | none in scope body (verify) | needs table |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/ChargeSmashAttacks.asm:590 | `custom_shuffle_item` | `ChargeSmashAttacks.frame_table_origin + ({character_id} * 4)` | data/table write, no RAM hook; decomp n/a |

## `src/Cheats.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Cheats.asm:14 | `Cheats` | 0x00042B3A | 0x800A3DEA | `scManagerRunPrintGObjStatus`+0xDAA (sc/scmanager.c) | (no description comment) | none in scope body (verify) | needs new state |
| src/Cheats.asm:20 | `Cheats` | 0x000337F8 | 0x80032BF8 | unmatched | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Cheats.asm:26 | `Cheats` | 0x0008B0D4 | 0x8010F8D4 | `ifCommonPlayerStockMultiProcDisplay`+0x5C (if/ifcommon.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Cheats.asm:32 | `Cheats` | 0x00081408 | 0x80105C08 | `grPupupuWhispyUpdateWait`+0x20 (gr/grcommon/grpupupu.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Cheats.asm:38 | `Cheats` | 0x000216FC | 0x80020AFC | `syAudioPlayBGM`+0x48 (sys/audio.c) | (no description comment) | none in scope body (verify) | trivial C edit |

## `src/Combo.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Combo.asm:23 | `improve_meter_` | 0x0005D1CC | 0x800E19CC | `ftMainProcUpdateInterrupt`+0x76C (ft/ftmain.c) | This function makes the combo meter take grabs and wall bounces into account. | `entry_improved_combo_meter`, `entry_tech_chase_combo_meter` | needs new state |

## `src/ComboMeter.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/ComboMeter.asm:851 | `hide_for_score_sprite_` | 0x901D8 | 0x801149D8 | `ifCommonPlayerScoreMakeEffect`+0xC (if/ifcommon.c) | This hides the combo meter when SCORE +/-1 is displayed for a port | `entry_combo_meter` | trivial C edit |

## `src/Costumes.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Costumes.asm:621 | `select_` | 0x001361B4 | 0x80137F34 | `ftComputerFollowObjectiveRecover`+0x10 (ft/ftcomputer.c) | This function replaces the original costume selection in favor of the 19XX style selection. CLeft and Cright will now cycle through colors while CUp and CDown cycle through shades. This is not a function that should be called. It is a patch | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:627 | `select_` | 0x00144BF8 | 0x80135618 | `mnPlayers1PTrainingUpdateCostume`+0x38 (mn/mnplayers/mnplayers1ptraining.c) | This function replaces the original costume selection in favor of the 19XX style selection. CLeft and Cright will now cycle through colors while CUp and CDown cycle through shades. This is not a function that should be called. It is a patch | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:633 | `select_` | 0x13ED5C | 0x80136B5C | `mnPlayers1PBonusInitVars`+0x8 (mn/mnplayers/mnplayers1pbonus.c) | This function replaces the original costume selection in favor of the 19XX style selection. CLeft and Cright will now cycle through colors while CUp and CDown cycle through shades. This is not a function that should be called. It is a patch | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:643 | `select_` | 0x14B628 | 0x801355F8 | `mnPlayers1PTrainingUpdateCostume`+0x18 (mn/mnplayers/mnplayers1ptraining.c) | This function replaces the original costume selection in favor of the 19XX style selection. CLeft and Cright will now cycle through colors while CUp and CDown cycle through shades. This is not a function that should be called. It is a patch | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:1051 | `disable_update` | 0x001361EC | 0x80137F6C | `ftComputerFollowObjectiveRecover`+0x48 (ft/ftcomputer.c) | These prevent the costume/shade from being overwritten when selecting with C buttons or when in team battle. | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1059 | `disable_update` | 0x0013679C | 0x8013851C | `func_ovl3_8013837C`+0x1A0 (ft/ftcomputer.c) | These prevent the costume/shade from being overwritten when selecting with C buttons or when in team battle. | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1064 | `disable_update` | 0x00137D04 | 0x80139A84 | `ftComputerFollowObjectiveAlly`+0x24 (ft/ftcomputer.c) | These prevent the costume/shade from being overwritten when selecting with C buttons or when in team battle. | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1069 | `disable_update` | 0x00144C28 | 0x80135648 | `mnPlayersVSCheckTeamSelectInRangeAll`+0x14 (mn/mnplayers/mnplayersvs.c) | These prevent the costume/shade from being overwritten when selecting with C buttons or when in team battle. | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1098 | `cpu_costumes_` | 0x136770 | 0x801384F0 | `func_ovl3_8013837C`+0x174 (ft/ftcomputer.c) | Allows CPU costume/shade to be controlled when pressing C buttons while hovering over CPU's panel. | none in scope body (verify) | needs new state |
| src/Costumes.asm:1103 | `cpu_costumes_` | 0x145010 | 0x80135A30 | `mnPlayers1PGameMakePortraitFlash`+0x34 (mn/mnplayers/mnplayers1pgame.c) | Allows CPU costume/shade to be controlled when pressing C buttons while hovering over CPU's panel. | none in scope body (verify) | needs new state |
| src/Costumes.asm:1195 | `force_hi_lo_poly_` | 0x53850 | 0x800D8050 | `ftManagerMakeFighter`+0x114 (ft/ftmanager.c) | Provides a way to force high/low poly models. | `entry_model_display` | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:1200 | `force_hi_lo_poly_` | 0x649C0 | 0x800E91C0 | `ftParamSetModelPartDetailAll`+0x28 (ft/ftparam.c) | Provides a way to force high/low poly models. | `entry_model_display` | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:1284 | `prevent_illegal_sound_` | 0x144C04 | 0x80135624 | `mnPlayers1PTrainingUpdateCostume`+0x44 (mn/mnplayers/mnplayers1ptraining.c) | Prevents illegal sound from playing when selecting the same costume | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1290 | `prevent_illegal_sound_` | 0x141194 | 0x80131BB4 | `func_ovl59_80131BB0`+0x4 (sc/sccommon/scstaffroll.c) | Prevents illegal sound from playing when selecting the same costume | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1296 | `prevent_illegal_sound_` | 0x12FF50 | 0x80131CD0 | `mvOpeningClashVoidProcDisplay`+0x4 (mv/mvopening/mvopeningclash.c) | Prevents illegal sound from playing when selecting the same costume | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1304 | `prevent_illegal_sound_` | 0x001361C8 | 0x80137F48 | `ftComputerFollowObjectiveRecover`+0x24 (ft/ftcomputer.c) | Prevents illegal sound from playing when selecting the same costume | none in scope body (verify) | trivial C edit |
| src/Costumes.asm:1311 | `check_for_custom_costume_` | 0x44764 | 0x800C8D84 | `lbCommonSetupTreeDObjs`+0x25C (lb/lbcommon.c) | Hook which allows for additional costumes by updating the palette array pointer for parts. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:1316 | `check_for_custom_costume_` | 0x446C0 | 0x800C8CE0 | `lbCommonSetupTreeDObjs`+0x1B8 (lb/lbcommon.c) | Hook which allows for additional costumes by updating the palette array pointer for parts. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:1584 | `icon_palette_override_` | 0x53090 | 0x800D7890 | `ftManagerSetupFilesAllKind`+0x24 (ft/ftmanager.c) | Patch which overrides the icon palette array pointer when the character has additional costumes. Runs when a character finishes loading. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Costumes.asm:1587 | `icon_palette_override_` | 0x530A8 | 0x800D78A8 | `ftManagerSetupFilesAllKind`+0x3C (ft/ftmanager.c) | Patch which overrides the icon palette array pointer when the character has additional costumes. Runs when a character finishes loading. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/Costumes.asm:84 | `part_type` | `PARTS_TABLE_ORIGIN + (0x{part_id} * 8)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:104 | `part_type` | `PARTS_TABLE_ORIGIN + (0x{part_id} * 8) + 4` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:178 | `part_type` | `PARTS_TABLE_ORIGIN + (0x{part_id} * 8) + 4` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:246 | `part_type` | `PART_0x{part_id}_SPECIAL_PARTS_ORIGIN + ({special_part_index} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:283 | `part_type` | `PART_0x{part_id}_IMAGES_ORIGIN + (0x{image_index} * 0x10)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:340 | `part_type` | `PART_0x{part_id}_{special_part_index}_IMAGES_ORIGIN + (0x{image_index} * 0x10)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:386 | `part_type` | `EXTRA_COSTUMES_TABLE_ORIGIN + ({char_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:389 | `part_type` | `STOCK_ICON_PALETTE_ARRAY_TABLE_ORIGIN + ({char_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:410 | `part_type` | `PART_0x{part_id}_{image_index}_PALETTE_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:426 | `part_type` | `PART_0x{part_id}_{image_index}_PALETTE_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:442 | `part_type` | `PART_0x{part_id}_{image_index}_PRIM_COLOR_ARRAY + ({extra_costume_id} * 8)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:458 | `part_type` | `PART_0x{part_id}_{image_index}_PRIM_COLOR_ARRAY + ({extra_costume_id} * 8) + 4` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:478 | `part_type` | `PART_0x{part_id}_{image_index}_DIFFUSE_AMBIENT_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:494 | `part_type` | `PART_0x{part_id}_{image_index}_DIFFUSE_AMBIENT_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:516 | `part_type` | `PART_0x{part_id}_{special_part_index}_{image_index}_PALETTE_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:533 | `part_type` | `PART_0x{part_id}_{special_part_index}_{image_index}_PALETTE_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:550 | `part_type` | `PART_0x{part_id}_{special_part_index}_{image_index}_PRIM_COLOR_ARRAY + ({extra_costume_id} * 8)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:571 | `part_type` | `PART_0x{part_id}_{special_part_index}_{image_index}_DIFFUSE_AMBIENT_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:588 | `part_type` | `PART_0x{part_id}_{special_part_index}_{image_index}_DIFFUSE_AMBIENT_ARRAY + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:606 | `part_type` | `EXTRA_STOCK_ICON_PALETTE_ARRAY_ORIGIN_{n} + ({extra_costume_id} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1761 | `get_random_legal_costume_` | `0x94490` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1770 | `get_random_legal_costume_` | `0x95A14` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1779 | `get_random_legal_costume_` | `0x96FCC` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1788 | `get_random_legal_costume_` | `0x98EF8` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1797 | `get_random_legal_costume_` | `0x9A310` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1806 | `get_random_legal_costume_` | `0x9B7F0` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1815 | `get_random_legal_costume_` | `0x9CC70` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1824 | `get_random_legal_costume_` | `0x9E178` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1833 | `get_random_legal_costume_` | `0x9FADC` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1842 | `get_random_legal_costume_` | `0xA0F8C` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1851 | `get_random_legal_costume_` | `0xA242C` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1860 | `get_random_legal_costume_` | `0xA39D0` | data/table write, no RAM hook; decomp n/a |
| src/Costumes.asm:1869 | `get_random_legal_costume_` | `0x93A80` | data/table write, no RAM hook; decomp n/a |

## `src/Crash.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Crash.asm:10 | `Crash` | 0x00024088 | 0x80023488 | `syDebugFramebufferWriteGlyph`+0xFC8 (sys/debug.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:13 | `Crash` | 0x000240A0 | 0x800234A0 | `syDebugFramebufferWriteGlyph`+0xFE0 (sys/debug.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:16 | `Crash` | 0x000240B8 | 0x800234B8 | `syDebugFramebufferWriteGlyph`+0xFF8 (sys/debug.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:19 | `Crash` | 0x000240D0 | 0x800234D0 | unmatched | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:22 | `Crash` | 0x000240E8 | 0x800234E8 | unmatched | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:29 | `Crash` | 0x23E04 | 0x80023204 | `syDebugFramebufferWriteGlyph`+0xD44 (sys/debug.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:33 | `Crash` | 0x2426C | 0x8002366C | unmatched | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Crash.asm:37 | `Crash` | 0x24080 | 0x80023480 | `syDebugFramebufferWriteGlyph`+0xFC0 (sys/debug.c) | (no description comment) | none in scope body (verify) | trivial C edit |

## `src/Credits.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Credits.asm:15 | `Credits` | 0x183F58 | 0x80136858 | `mnPlayers1PGameCheckLevelArrowPress`+0xC (mn/mnplayers/mnplayers1pgame.c) | (no description comment) | none in scope body (verify) | needs table |
| src/Credits.asm:20 | `Credits` | 0x187EB4 | 0x8013A7B4 | `mnPlayersVSSetSceneData`+0x150 (mn/mnplayers/mnplayersvs.c) | (no description comment) | none in scope body (verify) | needs table |
| src/Credits.asm:835 | `load_files_` | 0x182128 | 0x80134A28 | `mnVSRecordGetSDPercent`+0xC (mn/mndata/mnvsrecord.c) | Ensure our Remix credits file gets loaded | none in scope body (verify) | needs new state |
| src/Credits.asm:841 | `load_files_` | 0x182144 | 0x80134A44 | `mnVSRecordGetSDPercent`+0x28 (mn/mndata/mnvsrecord.c) | Ensure our Remix credits file gets loaded | none in scope body (verify) | needs new state |
| src/Credits.asm:858 | `load_files_` | 0x181F9C | 0x8013489C | `mnPlayers1PBonusPortraitFlashThreadUpdate`+0xC (mn/mnplayers/mnplayers1pbonus.c) | Ensure our Remix credits file gets loaded | none in scope body (verify) | trivial C edit |
| src/Credits.asm:865 | `use_extended_section_info_table_` | 0x182030 | 0x80134930 | `mnPlayers1PBonusMakePortraitFlash`+0x14 (mn/mnplayers/mnplayers1pbonus.c) | This allows us to use our extended section info table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:893 | `use_extended_title_info_table_` | 0x18117C | 0x80133A7C | `scStaffrollMakeJobDObjs`+0x4 (sc/sccommon/scstaffroll.c) | This allows us to use our extended title info table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:917 | `use_extended_title_strings_table_` | 0x181200 | 0x80133B00 | `sc1PIntroInitVSFighters`+0x38 (sc/sc1pmode/sc1pintro.c) | This allows us to use our extended title strings table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:921 | `use_extended_title_strings_table_` | 0x1812D4 | 0x80133BD4 | `mnPlayers1PBonusMakeHiScore`+0x8 (mn/mnplayers/mnplayers1pbonus.c) | This allows us to use our extended title strings table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:962 | `use_extended_name_info_table_` | 0x181718 | 0x80134018 | `ftComputerCheckSetTargetEdgeRight`+0x18 (ft/ftcomputer.c) | This allows us to use our extended name info table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:966 | `use_extended_name_info_table_` | 0x181A14 | 0x80134314 | `selected`+0xC (mn/mndata/mnsoundtest.c) | This allows us to use our extended name info table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1003 | `use_extended_name_strings_table_` | 0x181750 | 0x80134050 | `mnCharactersStartScene` (mn/mndata/mncharacters.c) | This allows us to use our extended name strings table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1007 | `use_extended_name_strings_table_` | 0x18181C | 0x8013411C | `mnPlayers1PBonusMakeFighter`+0x14 (mn/mnplayers/mnplayers1pbonus.c) | This allows us to use our extended name strings table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1048 | `use_extended_details_info_table_` | 0x1802D4 | 0x80132BD4 | `sc1PIntroFighterProcUpdate` (sc/sc1pmode/sc1pintro.c) | This allows us to use our extended details info table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1071 | `use_extended_details_strings_table_` | 0x18030C | 0x80132C0C | `mnPlayersVSUpdateShutter`+0x18 (mn/mnplayers/mnplayersvs.c) | This allows us to use our extended details strings table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1094 | `use_extended_company_table_` | 0x180654 | 0x80132F54 | `mnSoundTestMakeNumberSObj`+0x4 (mn/mndata/mnsoundtest.c) | This allows us to use our extended company map table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1121 | `use_extended_company_info_table_` | 0x180648 | 0x80132F48 | `mvOpeningRoomMakeWallpaper`+0x4 (mv/mvopening/mvopeningroom.c) | This allows us to use our extended company info table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1142 | `use_extended_company_strings_table_` | 0x180694 | 0x80132F94 | `sc1PIntroVSFighterProcUpdate`+0x10 (sc/sc1pmode/sc1pintro.c) | This allows us to use our extended company strings table | none in scope body (verify) | trivial C edit |
| src/Credits.asm:1166 | `use_added_characters_` | 0x1824D0 | 0x80134DD0 | `mnVSResultsMakeTint`+0x30 (mn/mnvsmode/mnvsresults.c) | This allows us to use our added characters | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Credits.asm:1171 | `use_added_characters_` | 0x181798 | 0x80134098 | `mnTitleFuncUpdate` (mn/mncommon/mntitle.c) | This allows us to use our added characters | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Credits.asm:1238 | `awayMode` | 0x180B44 | 0x80133444 | `mnVSRecordSetIconPositionForColumn`+0xC (mn/mndata/mnvsrecord.c) | Away mode credits demo (auto show details) | none in scope body (verify) | needs new state |
| src/Credits.asm:1242 | `awayMode` | 0x1809CC | 0x801332CC | `func_ovl28_801332CC` (mn/mnplayers/mnplayers1ptraining.c) | Away mode credits demo (auto show details) | none in scope body (verify) | needs new state |
| src/Credits.asm:1246 | `awayMode` | 0x17FEC0 | 0x801327C0 | `mnCharactersCheckRecentMotionKind` (mn/mndata/mncharacters.c) | Away mode credits demo (auto show details) | none in scope body (verify) | needs new state |
| src/Credits.asm:1311 | `remix_credits_speed_assist_` | 0x180CC4 | 0x801335C4 | `mnOptionStartScene`+0x4 (mn/mnoption/mnoption.c) | Helps reach Remix Credits while fast without overshooting Note: applies just once (press 'Start' again to speed through rest) | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/Credits.asm:74 | `Credits` | `start` | data/table write, no RAM hook; decomp n/a |
| src/Credits.asm:108 | `Credits` | `start` | data/table write, no RAM hook; decomp n/a |
| src/Credits.asm:136 | `Credits` | `start` | data/table write, no RAM hook; decomp n/a |
| src/Credits.asm:142 | `Credits` | `start` | data/table write, no RAM hook; decomp n/a |
| src/Credits.asm:178 | `string_type` | `start` | data/table write, no RAM hook; decomp n/a |
| src/Credits.asm:296 | `string_type` | `REMIX_CREDITS_FILE_ORIGIN + {pointer_offset} + 2` | data/table write, no RAM hook; decomp n/a |
| src/Credits.asm:829 | `offset` | `REMIX_CREDITS_FILE_ORIGIN` | data/table write, no RAM hook; decomp n/a |

## `src/CustomGrabAction.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/CustomGrabAction.asm:12 | `capture_action_override_` | 0xC534C | 0x8014A90C | `ftCommonCapturePulledProcCapture`+0xAC (ft/ftcommon/ftcommoncapturepulled.c) | Change the action used when a caracter is being grabbed rather than the usual CapturePulled action (0xAB) Used by Wario, Mad Piano to set the ThrownDK action (character is on their back, struggling) | none in scope body (verify) | trivial C edit |
| src/CustomGrabAction.asm:41 | `capture_position_fix_` | 0xC539C | 0x8014A95C | `ftCommonCapturePulledProcCapture`+0xFC (ft/ftcommon/ftcommoncapturepulled.c) | Attempts to fix the position of the grabbed character on frame 1 when using a custom action for the grabbed opponent on our GrabPull action. Not perfect, but a big improvement. | none in scope body (verify) | trivial C edit |
| src/CustomGrabAction.asm:79 | `capture_dk_break_fix_` | 0xC8F14 | 0x8014E4D4 | `ftCommonCaptureShoulderedProcInterrupt` (ft/ftcommon/ftcommoncapture.c) | Modifies the subroutine which handles mashing/breaking out of the ThrownDK action. | none in scope body (verify) | trivial C edit |

## `src/DI.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/DI.asm:27 | `apply_di_modifiers` | 0xBB31C | 0x801408DC | `ftCommonDamageCommonProcLagUpdate`+0x64 (ft/ftcommon/ftcommondamage.c) | (no description comment) | `entry_di` | trivial C edit |

## `src/DKMode.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/DKMode.asm:17 | `_resize_head` | 0x6E16C | 0x800F296C | `ftDisplayMainProcDisplay`+0x30 (ft/ftdisplaymain.c) | (no description comment) | none in scope body (verify) | needs new state |

## `src/Damage.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Damage.asm:40 | `extend_on_hit_gfx_` | 0x5F7C4 | 0x800E3FC4 | `ftMainProcessHitCollisionStatsMain`+0x108 (ft/ftmain.c) | Modifies an original routine (0x800E3EBC) to use a jump table rather than branches for each id. The jump table will be extended to accommodate 16 effects. | none in scope body (verify) | trivial C edit |
| src/Damage.asm:60 | `extend_on_hit_routines_` | 0xBB60C | 0x80140BCC | `ftCommonDamageCheckElementSetColAnim` (ft/ftcommon/ftcommondamage.c) | Modifies an original routine (0x80140BCC) to use a jump table rather than branches for each id. The jump table will be extended to accommodate 16 effects. | none in scope body (verify) | trivial C edit |
| src/Damage.asm:199 | `stun_damage_action_` | 0xBBFCC | 0x8014158C | `ftCommonDamageGotoDamageStatus`+0x2C (ft/ftcommon/ftcommondamage.c) | Patch which adds a check for the "stun" damage type to put opponents into the Stun action. If the character is already in the Stun action, acts like electric damage instead. Modifies the routine which originally handles the action change fo | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Damage.asm:888 | `prevent_wind_push_while_buried` | 0x63EB4 | 0x800E86B4 | `ftParamSetVelPush` (ft/ftparam.c) | Don't allow buried fighters to be pushed by Whispy (DL) | none in scope body (verify) | trivial C edit |
| src/Damage.asm:912 | `stun_electric_action_` | 0xBBD28 | 0x801412E8 | `ftCommonDamageInitDamageVars`+0x404 (ft/ftcommon/ftcommondamage.c) | Patch which adds a check for the "stun" damage type to put the opponents into DamageElec actions. This is only used if the character is already in the Stun action. Modifies the routine which originally loads action ids for electric damage. | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/Damage.asm:23 | `Damage` | `on_hit_gfx.table_origin + (id.{name} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/Damage.asm:26 | `Damage` | `on_hit_routine.table_origin + (id.{name} * 4)` | data/table write, no RAM hook; decomp n/a |

## `src/DragonKingHUD.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/DragonKingHUD.asm:49 | `set_damage_number_offsets_` | 0x8A8D0 | 0x8010F0D0 | `ifCommonPlayerDamageProcDisplay`+0x1D4 (if/ifcommon.c) | Changes the offsets table used to draw the player damage numbers | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:83 | `set_horizontal_spacing_` | 0x8A368 | 0x8010EB68 | `ifCommonPlayerDamageUpdateDigits`+0x274 (if/ifcommon.c) | Changes the horizontal spacing of the damage numbers | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:120 | `set_vertical_position_` | 0x8ABB0 | 0x8010F3B0 | `ifCommonPlayerDamageSetDigitPositions`+0x10 (if/ifcommon.c) | Changes the vertical position of the damage numbers and associated effects | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:126 | `set_vertical_position_` | 0x112D28 | 0x8018E5E8 | `sc1PBonusStageSetPlayerInterfacePositions`+0x10 (sc/sc1pmode/sc1pbonusstage.c) | Changes the vertical position of the damage numbers and associated effects | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:132 | `set_vertical_position_` | 0x18AA60 | 0x8018D450 | `scExplainSetPlayerInterfacePositions`+0x10 (sc/sccommon/scexplain.c) | Changes the vertical position of the damage numbers and associated effects | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:138 | `set_vertical_position_` | 0x10CEAC | 0x8018E64C | `scExplainSetupFiles`+0xC (sc/sccommon/scexplainfiles.c) | Changes the vertical position of the damage numbers and associated effects | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:193 | `set_damage_color_` | 0x8A984 | 0x8010F184 | `ifCommonPlayerDamageProcDisplay`+0x288 (if/ifcommon.c) | Sets color to shield color | none in scope body (verify) | needs new state |
| src/DragonKingHUD.asm:280 | `adjust_timer_position_` | 0x8E554 | 0x80112D54 | `ifCommonTimerProcDisplay`+0x13C (if/ifcommon.c) | Adjusts VS timer position if Dragon King HUD is enabled | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:286 | `adjust_timer_position_` | 0x8E478 | 0x80112C78 | `ifCommonTimerProcDisplay`+0x60 (if/ifcommon.c) | Adjusts VS timer position if Dragon King HUD is enabled | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:291 | `adjust_timer_position_` | 0x8E888 | 0x80113088 | `ifCommonTimerMakeDigits`+0x120 (if/ifcommon.c) | Adjusts VS timer position if Dragon King HUD is enabled | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:368 | `hide_series_logo_` | 0x8AD0C | 0x8010F50C | `ifCommonPlayerDamageInitInterface`+0x14C (if/ifcommon.c) | This hides series logos for Dragon King HUD | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/DragonKingHUD.asm:398 | `hide_series_logo_` | 0x90228 | 0x80114A28 | `ifCommonPlayerScoreMakeEffect`+0x5C (if/ifcommon.c) | This hides series logos for Dragon King HUD | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:434 | `disable_ko_falling_damage_animation_` | 0x8AFB4 | 0x8010F7B4 | `ifCommonPlayerDamageStartBreakAnim`+0x48 (if/ifcommon.c) | Disable the animation that makes the damage numbers fall during KO Really just speeds it up so it's not noticeable | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:467 | `reposition_training_hud_labels_` | 0x114728 | 0x8018DF08 | `sc1PTrainingModeMakeStatDisplay`+0x2C (sc/sc1pmode/sc1ptrainingmode.c) | Repositions Training HUD labels | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:509 | `reposition_training_hud_damage_` | 0x114BB0 | 0x8018E390 | `sc1PBonusStageMakeTimer`+0x4C (sc/sc1pmode/sc1pbonusstage.c) | Repositions Training HUD damage | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:541 | `reposition_training_hud_combo_` | 0x114ECC | 0x8018E6AC | `sc1PGameWaitStageCommonUpdate`+0x3C (sc/sc1pmode/sc1pgame.c) | Repositions Training HUD combo | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:574 | `reposition_training_hud_speed_enemy_` | 0x115008 | 0x8018E7E8 | `sc1PTrainingModeMakeSpeedDisplay`+0x74 (sc/sc1pmode/sc1ptrainingmode.c) | Repositions Training HUD speed and enemy | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:580 | `reposition_training_hud_speed_enemy_` | 0x115104 | 0x8018E8E4 | `sc1PBonusStageSetBonusStats`+0x14 (sc/sc1pmode/sc1pbonusstage.c) | Repositions Training HUD speed and enemy | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:613 | `reposition_training_hud_item_` | 0x11530C | 0x8018EAEC | `sc1PGameWaitThreadUpdate`+0x10 (sc/sc1pmode/sc1pgame.c) | Repositions Training HUD item | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:619 | `reposition_training_hud_item_` | 0x115338 | 0x8018EB18 | `sc1PGameWaitThreadUpdate`+0x3C (sc/sc1pmode/sc1pgame.c) | Repositions Training HUD item | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:624 | `reposition_training_hud_item_` | 0x115360 | 0x8018EB40 | `sc1PGameWaitThreadUpdate`+0x64 (sc/sc1pmode/sc1pgame.c) | Repositions Training HUD item | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:659 | `reposition_targets_` | 0x1125D0 | 0x8018DE90 | `mvOpeningYoshiFuncStart`+0x8 (mv/mvopening/mvopeningyoshi.c) | Moves the BTT targets and BTP platforms remaining down | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:665 | `reposition_targets_` | 0x11272C | 0x8018DFEC | `mvOpeningMarioFuncLights`+0x8 (mv/mvopening/mvopeningmario.c) | Moves the BTT targets and BTP platforms remaining down | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:699 | `reposition_multi_enemy_icons_` | 0x10D510 | 0x8018ECB0 | `sc1PTrainingModeInitMenuOptionSpriteAttrs`+0xC (sc/sc1pmode/sc1ptrainingmode.c) | Moves the enemy stock icons down | none in scope body (verify) | trivial C edit |
| src/DragonKingHUD.asm:732 | `move_name_down_` | 0x18C8FC | 0x8018DBBC | `mvOpeningYoshiMakeNameCamera`+0x2C (mv/mvopening/mvopeningyoshi.c) | (no description comment) | none in scope body (verify) | trivial C edit |

## `src/FD.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/FD.asm:59 | `music_fix_` | 0x9006C | 0x8011486C | `ifCommonBattleUpdateInterfaceAll`+0x10 (if/ifcommon.c) | Sets track to FD battle music after the intro in VS | none in scope body (verify) | needs new state |

## `src/FPS.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/FPS.asm:83 | `run_` | 0x00005FD0 | 0x800053D0 | `func_800053CC`+0x4 (sys/taskman.c) | Calculates the FPS | `entry_fps` | trivial C edit (long asm body, rewrite logic) |

## `src/FootStool.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/FootStool.asm:11 | `num_jumps_no_reset` | 0x5A6E4 | 0x800DEEE4 | `mpCommonSetFighterAir`+0x1C (mp/mpcommon.c) | dont reset jumps if jumping during jump squat | none in scope body (verify) | trivial C edit |
| src/FootStool.asm:33 | `aerial_jump_check` | 0xBAD48 | 0x80140308 | `ftCommonJumpAerialCheckInterruptCommon`+0x16C (ft/ftcommon/ftcommonjumpaerial.c) | if here, then player pressed JUMP while aerial | none in scope body (verify) | trivial C edit |
| src/FootStool.asm:545 | `kirby_jump_aerial_1_check` | 0xBAC60 | 0x80140220 | `ftCommonJumpAerialCheckInterruptCommon`+0x84 (ft/ftcommon/ftcommonjumpaerial.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/FootStool.asm:574 | `kirby_jump_aerial_2_check` | 0xBACCC | 0x8014028C | `ftCommonJumpAerialCheckInterruptCommon`+0xF0 (ft/ftcommon/ftcommonjumpaerial.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/FootStool.asm:603 | `puff_jump_aerial_2_check` | 0xBAD10 | 0x801402D0 | `ftCommonJumpAerialCheckInterruptCommon`+0x134 (ft/ftcommon/ftcommonjumpaerial.c) | (no description comment) | none in scope body (verify) | trivial C edit |

## `src/GFX.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/GFX.asm:38 | `augment_gfx_instructions_loader_` | 0x4A3CC | 0x800CE9EC | `lbParticleMakeScriptID`+0x4 (lb/lbparticle.c) | Modifies routines that load GFX Instructions so that we can use a custom GFX_INSTRUCTIONS_ID | none in scope body (verify) | trivial C edit |
| src/GFX.asm:43 | `augment_gfx_instructions_loader_` | 0x4A25C | 0x800CE87C | `lbParticleMakeCommon`+0xC (lb/lbparticle.c) | Modifies routines that load GFX Instructions so that we can use a custom GFX_INSTRUCTIONS_ID | none in scope body (verify) | trivial C edit |
| src/GFX.asm:72 | `extend_gfx_command_jump_table_` | 0x666A4 | 0x800EAEA4 | `ftParamMakeEffect`+0x2C8 (ft/ftparam.c) | Modifies the routine that maps GFX IDs in moveset commands to GFX routines so that we can use an extended table | none in scope body (verify) | trivial C edit |
| src/GFX.asm:100 | `extend_gfx_instructions_map_` | 0x4A0D0 | 0x800CE6F0 | `lbParticleMakeChildScriptID`+0x38 (lb/lbparticle.c) | Modifies the routine that maps GFX Instruction IDs to GFX instructions so that we can use an extended table | none in scope body (verify) | trivial C edit |
| src/GFX.asm:128 | `extend_gfx_texture_block_map_` | 0x4A13C | 0x800CE75C | `lbParticleMakeChildScriptID`+0xA4 (lb/lbparticle.c) | Modifies the routine that maps GFX Texture Block IDs to GFX Texture Blocks so that we can use an extended table | none in scope body (verify) | trivial C edit |
| src/GFX.asm:152 | `extend_referenced_gfx_texture_block_map_` | 0x4F190 | 0x800D37B0 | `lbParticleMakeGenerator`+0x1D4 (lb/lbparticle.c) | Modifies the routine that references GFX Texture Block IDs within GFX instructions so that we can use an extended table | none in scope body (verify) | trivial C edit |
| src/GFX.asm:174 | `extend_gfx_texture_block_map_render_` | 0x4D0D4 | 0x800D16F4 | `lbParticleDrawTextures`+0x9C0 (lb/lbparticle.c) | Modifies the render routine that maps GFX Texture Block IDs to GFX Texture Blocks so that we can use an extended table | none in scope body (verify) | needs table |
| src/GFX.asm:529 | `extend_referenced_gfx_instructions_map_` | 0x4F02C | 0x800D364C | `lbParticleMakeGenerator`+0x70 (lb/lbparticle.c) | Modifies the routine that references GFX Instruction IDs within GFX instructions so that we can use an extended table Instructions reference other GFX Instruction IDs via the A5 command | none in scope body (verify) | trivial C edit |
| src/GFX.asm:535 | `extend_referenced_gfx_instructions_map_` | 0x4EFF0 | 0x800D3610 | `lbParticleMakeGenerator`+0x34 (lb/lbparticle.c) | Modifies the routine that references GFX Instruction IDs within GFX instructions so that we can use an extended table Instructions reference other GFX Instruction IDs via the A5 command | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/GFX.asm:243 | `extend_gfx_texture_block_map_render_` | `gfx_texture_block_{n}_origin + 0x14 + ({i} * 4)` | data/table write, no RAM hook; decomp n/a |
| src/GFX.asm:247 | `extend_gfx_texture_block_map_render_` | `texture_end` | data/table write, no RAM hook; decomp n/a |
| src/GFX.asm:400 | `extend_gfx_texture_block_map_render_` | `0x6669B` | data/table write, no RAM hook; decomp n/a |

## `src/GFXRoutine.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/GFXRoutine.asm:418 | `extend_gfx_routine_table_` | 0x64F60 | 0x800E9760 | `ftParamCheckSetColAnimID`+0x14 (ft/ftparam.c) | Modifies the original routine (0x800E974C) to use the extended gfx routine table when a new id is used. | none in scope body (verify) | trivial C edit |
| src/GFXRoutine.asm:451 | `action_change_fix_` | 0x62898 | 0x800E7098 | `ftMainSetStatus`+0x174 (ft/ftmain.c) | Patch which adds the extended routine table to the check which occurs on action changes. | none in scope body (verify) | trivial C edit |
| src/GFXRoutine.asm:487 | `extend_gfx_check_` | 0x652E4 | 0x800E9AE4 | `ftParamResetStatUpdateColAnim`+0x210 (ft/ftparam.c) | Patch which extends the check for a characters gfx routine which occurs when a players gfx routine ends | none in scope body (verify) | trivial C edit |

## `src/Gallery.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Gallery.asm:799 | `initial_` | 0x17EA94 | 0x80132084 | `mnVSItemSwitchMakeItemList` (mn/mnvsmode/mnvsitemswitch.c) | Initial hook for the gallery. Runs once when the Congratulations screen loads. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:930 | `main_` | 0x17E658 | 0x80131C48 | `mnModeSelectMakeVSMode`+0x4 (mn/mncommon/mnmodeselect.c) | Main function for handling the Gallery. Runs once per frame while on the Congratulations screen. Fades the screen out when returning 1 in v0 | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1266 | `screen_change_` | 0x17E9D8 | 0x80131FC8 | `mvOpeningStandoffMakeWallpaper` (mv/mvopening/mvopeningstandoff.c) | Changes the screen id and resets BGM when cyling through the Gallery. | none in scope body (verify) | trivial C edit |
| src/Gallery.asm:1295 | `get_fade_length_` | 0x17E684 | 0x80131C74 | `mnPlayersVSSelectFighterPuck` (mn/mnplayers/mnplayersvs.c) | Use a custom fade length when cyling through the Gallery. | none in scope body (verify) | trivial C edit |
| src/Gallery.asm:1312 | `prevent_fgm_` | 0x17E940 | 0x80131F30 | `mnItemSwitchMakeSubtitle` (mn/mnvsmode/mnvsitemswitch.c) | Prevents the "Congratulations" FGM while cyling through the Gallery. | none in scope body (verify) | trivial C edit |
| src/Gallery.asm:1340 | `add_button_` | 0x1222C8 | 0x80132E78 | `scStaffrollMakeStaffRoleTextGObj`+0x4 (sc/sccommon/scstaffroll.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1408 | `cursor_` | 0x122154 | 0x80132D04 | `mnVSModeMakeTimeStockButton` (mn/mnvsmode/mnvsmode.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1416 | `cursor_` | 0x122108 | 0x80132CB8 | `mnPlayers1PBonusMakePortraitWallpaperCamera`+0x4 (mn/mnplayers/mnplayers1pbonus.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1424 | `cursor_` | 0x121FE0 | 0x80132B90 | `mnPlayers1PTrainingMakeTeamSelectCamera`+0xC (mn/mnplayers/mnplayers1ptraining.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1432 | `cursor_` | 0x12202C | 0x80132BDC | `sc1PIntroFighterProcUpdate`+0x8 (sc/sc1pmode/sc1pintro.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1440 | `cursor_` | 0x121C10 | 0x801327C0 | `mnCharactersCheckRecentMotionKind` (mn/mndata/mncharacters.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Gallery.asm:1456 | `cursor_` | 0x121DF8 | 0x801329A8 | `mn1PModeFuncRun` (mn/mn1pmode/mn1pmode.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Gallery.asm:1477 | `cursor_` | 0x121E6C | 0x80132A1C | `mnPlayersVSMakeNameAndEmblem`+0x8 (mn/mnplayers/mnplayersvs.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Gallery.asm:1480 | `cursor_` | 0x121E88 | 0x80132A38 | `mnVSResultsGetWinTeam`+0xC (mn/mnvsmode/mnvsresults.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/Gallery.asm:1486 | `cursor_` | 0x121EAC | 0x80132A5C | `mnPlayers1PBonusMakePortraitAll`+0x4 (mn/mnplayers/mnplayers1pbonus.c) | (no description comment) | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/Gallery.asm:499 | `id` | `bgm_table_origin + {bgm_id}` | data/table write, no RAM hook; decomp n/a |

## `src/GameEnd.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/GameEnd.asm:25 | `update_screen_` | 0x0010B204 | 0x8018E314 | `sc1PTrainingModeMakeDamageDisplay`+0x4 (sc/sc1pmode/sc1ptrainingmode.c) | This function changes the screen_id loaded into t6 if skip results screen is enabled | `entry_skip_results_screen`, `entry_salty_runback` | needs new state |

## `src/HRC.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/HRC.asm:565 | `spawn_visible_` | 0x10E454 | 0x8018FBF4 | `sc1PTrainingModeUpdateCursorPosition`+0x44 (sc/sc1pmode/sc1ptrainingmode.c) | Ensures players load visible (as opposed to invisible before entry animation) | none in scope body (verify) | trivial C edit |
| src/HRC.asm:583 | `apply_countdown_` | 0x8EA00 | 0x80113200 | `ifCommonTimerFuncRun`+0xFC (if/ifcommon.c) | Apply countdown SFX | none in scope body (verify) | trivial C edit |
| src/HRC.asm:616 | `play_are_you_ready_` | 0x10D1BC | 0x8018E95C | `sc1PBonusStageWriteBackup` (sc/sc1pmode/sc1pbonusstage.c) | Plays Are You Ready? | none in scope body (verify) | trivial C edit |
| src/HRC.asm:642 | `turn_off_blastzones_` | 0x5DAF0 | 0x800E22F0 | `ftMainProcPhysicsMap`+0x2A8 (ft/ftmain.c) | Turns off blastzones | none in scope body (verify) | trivial C edit |
| src/HRC.asm:666 | `prevent_item_despawn_` | 0xEE64C | 0x80173C0C | `itMapCheckDestroyDropped`+0xE8 (it/itmap.c) | Keeps the bat alive indefinitely (Also keeps basketball alive on Smashketball) | none in scope body (verify) | trivial C edit |
| src/HRC.asm:706 | `prevent_volume_reduction_` | 0x8EB6C | 0x8011336C | `ifCommonTimerFuncRun`+0x268 (if/ifcommon.c) | Stops volume from lowering after countdown. | none in scope body (verify) | trivial C edit |
| src/HRC.asm:730 | `animate_background_image_` | 0x8000C | 0x8010480C | `grWallpaperCalcPersp`+0x1EC (gr/grwallpaper.c) | Animates the background image. The background image is the same image twice, so we stretch it to allow scrolling. This also is used to animate Big Blue's background | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |

## `src/Handicap.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Handicap.asm:22 | `use_handicap_` | 0x5396C | 0x800D816C | `ftManagerMakeFighter`+0x230 (ft/ftmanager.c) | This allows us to override CPU handicap values. | none in scope body (verify) | trivial C edit |

## `src/Hazards.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Hazards.asm:76 | `dream_land_wind_` | 0x00081734 | 0x80105F34 | `grPupupuUpdateWhispyStatus`+0x40 (gr/grcommon/grpupupu.c) | Toggles for Dream Land wind | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:87 | `sector_z_arwing_` | 0x0008364C | 0x80107E4C | `grSectorProcUpdate`+0x44 (gr/grcommon/grsector.c) | Toggle for Sector Z arwing | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:98 | `mushroom_kingdom_pow_` | 0x000851AC | 0x801099AC | `grInishiePowerBlockProcUpdate`+0x44 (gr/grcommon/grinishie.c) | Toggle for Mushroom Kingdom POW blocks. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:109 | `mushroom_kingdom_piranha_plants_` | 0x00085424 | 0x80109C24 | `grInishieMakeGround`+0x18 (gr/grcommon/grinishie.c) | Toggle for Mushroom Kingdom Piranha Plants | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:120 | `mk_clone_hazard_removal_` | 0x0008541C | 0x80109C1C | `grInishieMakeGround`+0x10 (gr/grcommon/grinishie.c) | Remove pirhana plants and swing platform from the Mushroom Kingdom clones | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:153 | `planet_zebes_acid_` | 0x00083C50 | 0x80108450 | `grZebesMakeGround`+0x8 (gr/grcommon/grzebes.c) | Toggles and a CPU fix for Planet Zebes acid If no hazards mode is on, then no acid will be created If no movement mode is on, then the acid won't rise | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:159 | `planet_zebes_acid_` | 0x00083C10 | 0x80108410 | `grZebesProcUpdate`+0x4C (gr/grcommon/grzebes.c) | Toggles and a CPU fix for Planet Zebes acid If no hazards mode is on, then no acid will be created If no movement mode is on, then the acid won't rise | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:165 | `planet_zebes_acid_` | 0x000B1F28 | 0x801374E8 | `mnVSResultsSetIsPresent`+0x94 (mn/mnvsmode/mnvsresults.c) | Toggles and a CPU fix for Planet Zebes acid If no hazards mode is on, then no acid will be created If no movement mode is on, then the acid won't rise | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:260 | `hyrule_castle_tornadoes_` | 0x00086160 | 0x8010A960 | `grHyruleTwisterProcUpdate`+0x44 (gr/grcommon/grhyrule.c) | Toggle for Hyrule Castle tornadoes | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:271 | `ai_extend_hyrule_tornado_check` | 0xAF790 | 0x80134D50 | `mnPlayers1PGameMakeTotalBonusCount`+0x48 (mn/mnplayers/mnplayers1pgame.c) | Extends AI stage ID check so cpus walk away from tornados on Hyrule Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:299 | `congo_jungle_barrel_` | 0x000857BC | 0x80109FBC | `grJungleMakeGround`+0x8 (gr/grcommon/grjungle.c) | Toggle for Congo Jungle barrel | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:384 | `saffron_city_pokemon_` | 0x00086974 | 0x8010B174 | `grYamabukiGateProcUpdate`+0x44 (gr/grcommon/gryamabuki.c) | Toggle for Saffron City Pokemon | `entry_hazard_mode` | trivial C edit |
| src/Hazards.asm:431 | `saffron_city_pokemon_rate_init` | 0x000864F4 | 0x8010ACF4 | `grYamabukiGateUpdateSleep`+0x24 (gr/grcommon/gryamabuki.c) | Toggle for Saffron City Pokemon Rate (Inititialize) Door time is initalized at 0x8010B28C (open at start), but we don't need to modify it | `entry_saffron_poke_rate` | trivial C edit |
| src/Hazards.asm:477 | `saffron_city_pokemon_rate` | 0x000868CC | 0x8010B0CC | `grYamabukiGateSetClosedWait`+0x14 (gr/grcommon/gryamabuki.c) | Toggle for Saffron City Pokemon Rate Pokemon/door timers are halfwords stored at 0x8013140E | `entry_saffron_poke_rate` | trivial C edit |
| src/Hazards.asm:538 | `peach_bumper_` | 0x00086CB4 | 0x8010B4B4 | `grCastleMakeGround`+0x8 (gr/grcommon/grcastle.c) | Toggle for Peach's Bumper. registers v0 and v1 have current stage ID, but v0 is free | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:604 | `yoshis_island_clouds_` | 0x84488 | 0x80108C88 | `grYosterMakeGround`+0x8 (gr/grcommon/gryoster.c) | Toggle for Yoshi's Island clouds. | `entry_hazard_mode` | trivial C edit |
| src/Hazards.asm:638 | `yoshis_island_cloud_anims_` | 0x842F4 | 0x80108AF4 | `grYosterInitAll`+0x100 (gr/grcommon/gryoster.c) | Toggle for restoring Yoshi's Island cloud animations that are present but unused in vanilla | `entry_yi_clouds` | trivial C edit |
| src/Hazards.asm:655 | `platform_movement_` | 0x00080DA8 | 0x801055A8 | `grDisplayMakeGeometryLayer`+0x12C (gr/grdisplay.c) | This disables platform movement when hazard mode is on | `entry_hazard_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:829 | `barrel_angle_fix_` | 0x8592C | 0x8010A12C | `grJungleTaruCannGetRotate` (gr/grcommon/grjungle.c) | This makes it so additional barrels can have their own rotation values and the ejection angle will use the correct rotation value. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:842 | `many_barrels_fix_` | 0x5D71C | 0x800E1F1C | `ftMainSearchHitHazard`+0x34 (ft/ftmain.c) | Enables using a custom stage collision object array so we can have TOO MANY BARRELS! | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:849 | `many_barrels_fix_` | 0x5D500 | 0x800E1D00 | `ftMainClearGroundElementsAll`+0x10 (ft/ftmain.c) | Enables using a custom stage collision object array so we can have TOO MANY BARRELS! | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:858 | `many_barrels_fix_` | 0x5D554 | 0x800E1D54 | `ftMainCheckAddGroundObstacle`+0xC (ft/ftmain.c) | Enables using a custom stage collision object array so we can have TOO MANY BARRELS! | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:1122 | `alt_acid_levels_` | 0x000839E0 | 0x801081E0 | `grZebesMakeAcid`+0xF4 (gr/grcommon/grzebes.c) | Alt Y positions for Zebes level acid | none in scope body (verify) | needs new state |
| src/Hazards.asm:1261 | `peach_bumper_hitbox_` | 0xF8150 | 0x8017D710 | `itGBumperMakeItem`+0x94 (it/itground/itgbumper.c) | Use the correct Bumper Hitbox for Peach's Castle clones | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:1421 | `bowser_bomb_explosion_` | 0xFF810 | 0x80184DD0 | `itTaruBombCommonProcHit`+0xC (it/itground/ittarubomb.c) | Hard coded patch for the RTTF bomb explosion subroutine 0x80184DC4. Skips the shrapnel effect, sets the respawn flag, and plays a different FGM based on stage id. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:1458 | `bowser_bomb_air_collision_` | 0xFF9C4 | 0x80184F84 | `itTaruBombFallProcMap`+0xA8 (it/itground/ittarubomb.c) | Hard coded patch for the RTTF bomb air collision(and physics?) subroutine 0x80184EDC. Plays an alternate FGM based on stage id. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:1482 | `bomb_hitstun_` | 0xBB984 | 0x80140F44 | `ftCommonDamageInitDamageVars`+0x60 (ft/ftcommon/ftcommondamage.c) | Applies extra hitstun to a player hit by a bowser bomb. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:1553 | `track_hit_object_` | 0x5F714 | 0x800E3F14 | `ftMainProcessHitCollisionStatsMain`+0x58 (ft/ftmain.c) | Keeps track of the most recent object to hit each player. TODO: this could be useful elsewhere, so move it somewhere outside of this file? | none in scope body (verify) | needs new state |
| src/Hazards.asm:1816 | `klaptrap_room_` | 0xE8C1C | 0x8016E1DC | `itManagerMakeItem`+0x68 (it/itmanager.c) | Room ordering fix for Klaptrap. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2357 | `btp_bomb_or_barrel_setup` | 0x11228C | 0x8018DB4C | `scExplainMakeSpecialMoveRGB`+0x8 (sc/sccommon/scexplain.c) | Adds rolling bombs or barrels to board the platforms | none in scope body (verify) | needs new state |
| src/Hazards.asm:2554 | `banjo_btp_barrel_firing_fix` | 0xBEAAC | 0x8014406C | `ftCommonTaruCannShootFighter`+0x34 (ft/ftcommon/ftcommontarucann.c) | Fixes header issue related to barrels firing | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2575 | `marina_btt_setup` | 0x111AE0 | 0x8018D3A0 | `SCAutoDemoProcFocusPlayer1`+0x4 (sc/sccommon/scautodemo.c) | Adds rolling bombs to marina's break the targets | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:2634 | `rolling_bomb_position` | 0x86E1C | 0x8010B61C | `grBonus3TaruBombProcUpdate`+0x2C (gr/grbonus/grbonus3.c) | Changes how bombs position are set | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2666 | `rolling_bomb_explosion_1` | 0xFF614 | 0x80184BD4 | `itTaruBombContainerSmashMakeEffect`+0x90 (it/itground/ittarubomb.c) | Changes how bombs explosions are loaded | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2700 | `rolling_bomb_explosion_2` | 0xFFC98 | 0x80185258 | `itTaruBombExplodeInitVars`+0x64 (it/itground/ittarubomb.c) | Changes how bombs explosions are loaded | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2735 | `rolling_bomb_explosion_3` | 0xFFA44 | 0x80185004 | `itTaruBombExplodeProcUpdate`+0x30 (it/itground/ittarubomb.c) | Changes how bombs explosions are loaded | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2957 | `rttf_bumpers_` | 0x86D50 | 0x8010B550 | `grBonus3MakeBumpers`+0x48 (gr/grbonus/grbonus3.c) | Hardcoding for bumpers on Race to the Finish Stages | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:2980 | `dreamland_downtime_initially_` | 0x81E6C | 0x8010666C | `grPupupuInitAll`+0xE0 (gr/grcommon/grpupupu.c) | Toggle for Whispy Downtime Counter initially | `entry_whispy_mode` | trivial C edit |
| src/Hazards.asm:3009 | `dreamland_downtime_` | 0x81610 | 0x80105E10 | `grPupupuWhispyUpdateStop`+0x38 (gr/grcommon/grpupupu.c) | Toggle for Whispy Downtime Counter | `entry_whispy_mode` | trivial C edit |
| src/Hazards.asm:3038 | `dreamland_wind_speed_multiplier_` | 0x81274 | 0x80105A74 | `grPupupuWhispySetWindPush`+0x118 (gr/grcommon/grpupupu.c) | Toggle for the Dreamland wind speed multiplier to use the value in different modes. | `entry_whispy_mode` | trivial C edit |
| src/Hazards.asm:3092 | `dreamland_wind_speed_max_` | 0x81250 | 0x80105A50 | `grPupupuWhispySetWindPush`+0xF4 (gr/grcommon/grpupupu.c) | Toggle for the Dreamland wind speed max to determine maximum speeed | `entry_whispy_mode` | trivial C edit |
| src/Hazards.asm:3130 | `dream_land_wind_parameters` | 0x811A0 | 0x801059A0 | `grPupupuWhispySetWindPush`+0x44 (gr/grcommon/grpupupu.c) | Changes wind parameters for Dream Greens and super/hyper modes | `entry_whispy_mode` | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:3201 | `dream_greens_hardcoding_1` | 0x81DAC | 0x801065AC | `grPupupuInitAll`+0x20 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3232 | `dream_greens_hardcoding_2` | 0x81DE4 | 0x801065E4 | `grPupupuInitAll`+0x58 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3257 | `dream_greens_hardcoding_3` | 0x81E00 | 0x80106600 | `grPupupuInitAll`+0x74 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3279 | `dream_greens_hardcoding_4` | 0x81E24 | 0x80106624 | `grPupupuInitAll`+0x98 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3303 | `dream_greens_hardcoding_animation_1` | 0x81B4C | 0x8010634C | `grPupupuUpdateGObjAnims`+0x38 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens animation | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3325 | `dream_greens_hardcoding_animation_2` | 0x81B98 | 0x80106398 | `grPupupuUpdateGObjAnims`+0x84 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens animation | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3349 | `dream_greens_hardcoding_animation_3` | 0x81C14 | 0x80106414 | `grPupupuUpdateGObjAnims`+0x100 (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens animation | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3373 | `dream_greens_hardcoding_animation_4` | 0x81C60 | 0x80106460 | `grPupupuUpdateGObjAnims`+0x14C (gr/grcommon/grpupupu.c) | Changes hardcoding for Dream Greens animation | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3397 | `mk_remix_hardcoding_1` | 0x84D14 | 0x80109514 | `grInishieMakeScale`+0x74 (gr/grcommon/grinishie.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3419 | `mk_remix_hardcoding_2` | 0x84D74 | 0x80109574 | `grInishieMakeScale`+0xD4 (gr/grcommon/grinishie.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3441 | `mk_remix_hardcoding_3` | 0x84A50 | 0x80109250 | `grInishieScaleUpdateStep`+0x30 (gr/grcommon/grinishie.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3463 | `mk_remix_hardcoding_4` | 0x853EC | 0x80109BEC | `grInishieInitHeaders`+0x18 (gr/grcommon/grinishie.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3485 | `mk_remix_hardcoding_5` | 0xF7B4C | 0x8017D10C | `itPakkunWaitProcUpdate`+0x68 (it/itground/itpakkun.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3507 | `mk_remix_hardcoding_6` | 0xF7B70 | 0x8017D130 | `itPakkunWaitProcUpdate`+0x8C (it/itground/itpakkun.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3531 | `mk_remix_hardcoding_7` | 0xF7E40 | 0x8017D400 | `itPakkunAppearProcDamage`+0xCC (it/itground/itpakkun.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3555 | `mk_remix_hardcoding_8` | 0xF6BC0 | 0x8017C180 | `itPowerBlockWaitProcDamage`+0x24 (it/itground/itpowerblock.c) | Changes hardcoding for Mushroom Kingdom Remix | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:3580 | `mk_remix_hardcoding_9` | 0x000B1EA4 | 0x80137464 | `mnVSResultsSetIsPresent`+0x10 (mn/mnvsmode/mnvsresults.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:4356 | `sonic_btp_setup` | 0x112354 | 0x8018DC14 | `scExplainMakeSObjOffset`+0x8 (sc/sccommon/scexplain.c) | This alters the Bumper display list to use the casino bumper texture. Also updates the sfx for the bumper. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:4437 | `casino_night_side_bumper_bounce` | 0xBC5C8 | 0x80141B88 | `ftCommonWallDamageSetStatus`+0x80 (ft/ftcommon/ftcommonwalldamage.c) | Changes how hard the characters bounce of the side bumpers in Casino Night | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:4481 | `casino_night_side_bumper_knockback` | 0x61440 | 0x800E5C40 | `ftMainGetGroundHitObstacle`+0x10 (ft/ftmain.c) | Applies knockback to characters that bump into the side bumpers | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Hazards.asm:5628 | `robot_bee_laser_reflect_` | 0x5FDD0 | 0x800E45D0 | `ftMainProcessHitCollisionStatsMain`+0x714 (ft/ftmain.c) | Hit routine for Metallic Madness Laser | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:6140 | `chain_chomp_projectile_counter` | 0xE10E0 | 0x801666A0 | `wpProcessSetHitInteractStats`+0x10C (wp/wpprocess.c) | allows chain chomp hitbox to refresh by setting counter | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:6191 | `blast_zone_destroy_prevent` | 0xE0E90 | 0x80166450 | `wpProcessProcWeaponMain`+0x194 (wp/wpprocess.c) | Prevents Chain Chomp Projectile From being destroyed for being past blast | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:6224 | `stoplight_1` | 0x8DBE8 | 0x801123E8 | `ifCommonCountdownThread`+0xF4 (if/ifcommon.c) | Adds additional sound for 1 on stoplight | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:6265 | `stoplight_2` | 0x8DBFC | 0x801123FC | `ifCommonCountdownThread`+0x108 (if/ifcommon.c) | Adds additional sound for 2 on stoplight | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:6309 | `stoplight_3` | 0x8DC10 | 0x80112410 | `ifCommonCountdownThread`+0x11C (if/ifcommon.c) | Adds additional sound for 3 on stoplight | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:6350 | `stoplight_go` | 0x8DC3C | 0x8011243C | `ifCommonCountdownThread`+0x148 (if/ifcommon.c) | Adds additional sound for Go on stoplight | none in scope body (verify) | needs new state |
| src/Hazards.asm:9484 | `peach_btt_pipe_override` | 0xBCFA8 | 0x80142568 | `ftCommonDokanWaitSetStatus`+0xAC (ft/ftcommon/ftcommondokan.c) | forces player to always exit through side pipe on Peach's Break the Targets | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:9510 | `pipe_enter_skip_piranha_plant_check` | 0xBCC7C | 0x8014223C | `ftCommonDokanStartSetStatus`+0xD8 (ft/ftcommon/ftcommondokan.c) | fixes beach BTT crash on vanilla 1P by skipping the function that sets the piranha plant into the wait state | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:9533 | `platform_despawn_crash_fix` | 0xBF5AC | 0x80144B6C | `ftCommonCliffCommonProcPhysics`+0x18 (ft/ftcommon/ftcommoncliffcatchwait.c) | This prevents the game from crashing if a platform disappears while a player is hanging onto it. | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:9576 | `platform_despawn_crash_fix_2` | 0xBFD88 | 0x80145348 | `ftCommonCliffCommon2ProcPhysics`+0x94 (ft/ftcommon/ftcommoncliffclimb.c) | prevents a crash while some characters are performing cliff attack quick 2 hook in 0x801452B4 | none in scope body (verify) | trivial C edit |
| src/Hazards.asm:9620 | `congo_falls_hazard` | 0xBEBDC | 0x8014419C | `ftCommonTaruCannShootFighter`+0x164 (ft/ftcommon/ftcommontarucann.c) | makes the Congo Falls barrel apply a hitbox while exiting the barrel | none in scope body (verify) | needs new state |

## `src/Hitbox.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Hitbox.asm:27 | `hitbox_mode_` | 0x0006E3FC | 0x800F2BFC | `ftDisplayMainProcDisplay`+0x2C0 (ft/ftdisplaymain.c) | this is a hook into the function which loads the character's hitbox display state by default the value (at 0x0B4C(s8)) will always be 0 v1 contains hitbox display state: 0 = no hitbox display 3 = ECB display (all other values?) = hitbox dis | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:117 | `projectile_` | 0x000E1F78 | 0x80167538 | `wpDisplayMain`+0x18 (wp/wpdisplay.c) | this is a hook into the function which loads the display state for projectiles | `entry_special_model` | trivial C edit |
| src/Hitbox.asm:156 | `grab_immunity_and_armor_frames_` | 0x6DF9C | 0x800F279C | `ftDisplayMainDrawParts`+0x218 (ft/ftdisplaymain.c) | When the Advanced Hurtbox Display is on, colors hurtboxes gray when in active armor frames and grab-immune hurtboxes cyan. | `entry_advanced_hurtbox` | trivial C edit |
| src/Hitbox.asm:195 | `force_grab_immunity_` | 0x61864 | 0x800E6064 | `ftMainSearchFighterCatch`+0x20C (ft/ftmain.c) | Force Grab immunity under certain circumstances. Makes Giga Bowser CPU immune to grab in Remix 1P Makes Crash immune to grab during DSP. | none in scope body (verify) | trivial C edit |
| src/Hitbox.asm:250 | `make_hurtboxes_transparent_` | 0x6E030 | 0x800F2830 | `ftDisplayMainDrawParts`+0x2AC (ft/ftdisplaymain.c) | Makes hurtboxes transparent in hitbox+model mode. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:256 | `make_hurtboxes_transparent_` | 0xEC37C | 0x8017193C | `itDisplayHitCollisions`+0x52C (it/itdisplay.c) | Makes hurtboxes transparent in hitbox+model mode. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:326 | `make_hitboxes_transparent_` | 0x6EBB0 | 0x800F33B0 | `ftDisplayMainProcDisplay`+0xA74 (ft/ftdisplaymain.c) | Ensures hitboxes are transparent in hitbox+model mode. | `entry_special_model` | trivial C edit |
| src/Hitbox.asm:342 | `add_reflect_absorb_hitboxes` | 0x6EB58 | 0x800F3358 | `ftDisplayMainProcDisplay`+0xA1C (ft/ftdisplaymain.c) | Adds reflect/absorb hitboxes. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:491 | `add_reflect_absorb_hitboxes` | 0x139C48 | 0x8013B9C8 | `mnPlayersVSStartScene`+0x68C (mn/mnplayers/mnplayersvs.c) | Adds reflect/absorb hitboxes. | none in scope body (verify) | needs new state |
| src/Hitbox.asm:495 | `add_reflect_absorb_hitboxes` | 0x140EC8 | 0x80138CC8 | `mnPlayersVSMakeHandicapLevelCamera`+0x1C (mn/mnplayers/mnplayersvs.c) | Adds reflect/absorb hitboxes. | none in scope body (verify) | needs new state |
| src/Hitbox.asm:501 | `add_reflect_absorb_hitboxes` | 0x12EEB0 | 0x80135B70 | `mnPlayers1PGameAnnounceFighter`+0x40 (mn/mnplayers/mnplayers1pgame.c) | Adds reflect/absorb hitboxes. | none in scope body (verify) | needs new state |
| src/Hitbox.asm:504 | `add_reflect_absorb_hitboxes` | 0x12EEC0 | 0x80135B80 | `mnVSResultsBarProcDisplay`+0x8 (mn/mnvsmode/mnvsresults.c) | Adds reflect/absorb hitboxes. | none in scope body (verify) | needs new state |
| src/Hitbox.asm:511 | `render_hitbox_plus_model_` | 0x6E7D0 | 0x800F2FD0 | `ftDisplayMainProcDisplay`+0x694 (ft/ftdisplaymain.c) | Enables rendering the hitbox display over the model. | `entry_special_model` | trivial C edit |
| src/Hitbox.asm:547 | `render_hitbox_plus_projectile_` | 0xE1FB8 | 0x80167578 | `wpDisplayMain`+0x58 (wp/wpdisplay.c) | Enables rendering the hitbox display over projectiles. | `entry_special_model` | trivial C edit |
| src/Hitbox.asm:551 | `render_hitbox_plus_projectile_` | 0xE1FEC | 0x801675AC | `wpDisplayMain`+0x8C (wp/wpdisplay.c) | Enables rendering the hitbox display over projectiles. | `entry_special_model` | trivial C edit |
| src/Hitbox.asm:591 | `render_hitbox_plus_item_` | 0xEC760 | 0x80171D20 | `itDisplayOPAProcDisplay`+0xA4 (it/itdisplay.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:597 | `render_hitbox_plus_item_` | 0xEC81C | 0x80171DDC | `itDisplayXLUProcDisplay`+0xA4 (it/itdisplay.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:603 | `render_hitbox_plus_item_` | 0xECA30 | 0x80171FF0 | `itDisplayColAnimOPAProcDisplay`+0xA4 (it/itdisplay.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:609 | `render_hitbox_plus_item_` | 0xECD30 | 0x801722F0 | `itDisplayColAnimXLUProcDisplay`+0xA4 (it/itdisplay.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:615 | `render_hitbox_plus_item_` | 0xFDEAC | 0x8018346C | `itPippiCommonProcDisplay`+0x128 (it/itmonster/itpippi.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:621 | `render_hitbox_plus_item_` | 0xF900C | 0x8017E5CC | `itKabigonCommonProcDisplay`+0x128 (it/itmonster/itkabigon.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:627 | `render_hitbox_plus_item_` | 0xFE008 | 0x801835C8 | `itPippiCommonMoveDLProcDisplay`+0x128 (it/itmonster/itpippi.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:633 | `render_hitbox_plus_item_` | 0xF8C68 | 0x8017E228 | `itKabigonFallProcDisplay`+0x128 (it/itmonster/itkabigon.c) | Enables rendering the hitbox display over items. | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:714 | `fix_hitbox_position_` | 0x6EB10 | 0x800F3310 | `ftDisplayMainProcDisplay`+0x9D4 (ft/ftdisplaymain.c) | This makes it so the hitbox display is positioned correctly on the CSS screens and the VS results screen. I am doing it this way because I don't know if the positioning is important on other screens. See https://github.com/tehzz/SSB64-Notes | `entry_special_model` | trivial C edit (long asm body, rewrite logic) |
| src/Hitbox.asm:775 | `fix_hitbox_position_` | 0x6E1A0 | 0x800F29A0 | `ftDisplayMainProcDisplay`+0x64 (ft/ftdisplaymain.c) | This makes it so the hitbox display is positioned correctly on the CSS screens and the VS results screen. I am doing it this way because I don't know if the positioning is important on other screens. See https://github.com/tehzz/SSB64-Notes | none in scope body (verify) | trivial C edit |
| src/Hitbox.asm:782 | `skip_rendering_hitboxes_` | 0x6EB6C | 0x800F336C | `ftDisplayMainProcDisplay`+0xA30 (ft/ftdisplaymain.c) | Skips rendering hitboxes for Kirby Cube. | `entry_special_model` | trivial C edit |
| src/Hitbox.asm:809 | `render_ecb_over_hurtboxes_` | 0x6EE48 | 0x800F3648 | `ftDisplayMainProcDisplay`+0xD0C (ft/ftdisplaymain.c) | Render ECB over hurtboxes for Kirby Cube | `entry_special_model` | trivial C edit |

## `src/Hitstun.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Hitstun.asm:6 | `hitstun_` | 0x659B0 | 0x800EA1B0 | `ftParamGetHitStun` (ft/ftparam.c) | Toggle for Melee Style Hitstun. Divides Knockback by 2.5, instead of 1.875, thus less hitstun | `entry_hitstun` | trivial C edit |

## `src/InputDelay.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/InputDelay.asm:19 | `apply_input_delay_` | 0x4BF8 | 0x80003FF8 | `syControllerUpdateGlobalData`+0x60 (sys/controller.c) | Hijacks the controller struct writing routine to use a buffer if input delay is greater than 0. | none in scope body (verify) | needs new state |

## `src/InputDisplay.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/InputDisplay.asm:940 | `adjust_timer_position_` | 0x8E54C | 0x80112D4C | `ifCommonTimerProcDisplay`+0x134 (if/ifcommon.c) | Adjusts VS timer position if 4p has input display HUD positioned at top | none in scope body (verify) | trivial C edit |
| src/InputDisplay.asm:946 | `adjust_timer_position_` | 0x8E468 | 0x80112C68 | `ifCommonTimerProcDisplay`+0x50 (if/ifcommon.c) | Adjusts VS timer position if 4p has input display HUD positioned at top | none in scope body (verify) | trivial C edit |
| src/InputDisplay.asm:953 | `adjust_timer_position_` | 0x8E880 | 0x80113080 | `ifCommonTimerMakeDigits`+0x118 (if/ifcommon.c) | Adjusts VS timer position if 4p has input display HUD positioned at top | none in scope body (verify) | trivial C edit |

## `src/Item.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Item.asm:154 | `extend_item_spawn_` | 0xE94C4 | 0x8016EA84 | `itManagerMakeItemSetupCommon`+0xC (it/itmanager.c) | Checks the item ID for a custom item during item creation. | none in scope body (verify) | trivial C edit |
| src/Item.asm:178 | `extend_item_spawn_gfx_` | 0xE9508 | 0x8016EAC8 | `itManagerMakeItemSetupCommon`+0x50 (it/itmanager.c) | Checks the item ID for a custom item during item creation to determine if it should show spawn gfx. | none in scope body (verify) | trivial C edit |
| src/Item.asm:202 | `extend_item_pre_pickup_table_` | 0xED814 | 0x80172DD4 | `itMainSetFighterHold`+0x130 (it/itmain.c) | Checks the item ID for a custom item during the start of item pickup. | none in scope body (verify) | trivial C edit |
| src/Item.asm:232 | `extend_item_throw_table` | 0xED648 | 0x80172C08 | `itMainSetFighterThrow`+0x90 (it/itmain.c) | Checks the item ID for a custom item when being thrown. | none in scope body (verify) | trivial C edit |
| src/Item.asm:267 | `extend_training_mode_item_menu_` | 0x113D50 | 0x8018D530 | `scAutoDemoResetFocusPlayerAll`+0x8 (sc/sccommon/scautodemo.c) | Extends Training Mode Item menu. | none in scope body (verify) | needs table |
| src/Item.asm:272 | `extend_training_mode_item_menu_` | 0x113E54 | 0x8018D634 | `mvOpeningPikachuMakeMotionWindow` (mv/mvopening/mvopeningpikachu.c) | Extends Training Mode Item menu. | none in scope body (verify) | needs table |
| src/Item.asm:278 | `extend_training_mode_item_menu_` | 0x11586C | 0x8018F04C | `sc1PTrainingModeUpdateItemOptionSprite`+0xC (sc/sc1pmode/sc1ptrainingmode.c) | Extends Training Mode Item menu. | none in scope body (verify) | needs table |
| src/Item.asm:313 | `extend_training_mode_hud_` | 0x11522C | 0x8018EA0C | `sc1PTrainingModeItemDisplayProcDisplay`+0x60 (sc/sc1pmode/sc1ptrainingmode.c) | Extends Training Mode HUD for held item. | none in scope body (verify) | needs table |
| src/Item.asm:320 | `extend_training_mode_hud_` | 0x115150 | 0x8018E930 | `sc1PTrainingModeUpdateItemDisplaySprite`+0x24 (sc/sc1pmode/sc1ptrainingmode.c) | Extends Training Mode HUD for held item. | none in scope body (verify) | needs table |
| src/Item.asm:396 | `item_pickup_check_` | 0xC0654 | 0x80145C14 | `ftCommonLightGetProcDamage`+0x30 (ft/ftcommon/ftcommonget.c) | Hook into routine that checks if tomato or heart was picked up. Enables us to put a custom item ID check. This only runs when the item pickup type is 5. Check 0x10 in the item special struct (0x84 in item object). | none in scope body (verify) | trivial C edit |
| src/Item.asm:427 | `item_pickup_check_` | 0xC08C4 | 0x80145E84 | `ftCommonGetProcUpdate`+0x114 (ft/ftcommon/ftcommonget.c) | Hook into routine that checks if tomato or heart was picked up. Enables us to put a custom item ID check. This only runs when the item pickup type is 5. Check 0x10 in the item special struct (0x84 in item object). | none in scope body (verify) | trivial C edit |
| src/Item.asm:495 | `extend_item_drop_check_` | 0xED554 | 0x80172B14 | `itMainSetFighterDrop`+0x28 (it/itmain.c) | Hook into routine that checks if tomato or heart was picked up. Enables us to put a custom item ID check. This only runs when the item pickup type is 5. Check 0x10 in the item special struct (0x84 in item object). | none in scope body (verify) | trivial C edit |
| src/Item.asm:522 | `extend_player_collision_check_` | 0x5F218 | 0x800E3A18 | `ftMainUpdateDamageStatItem`+0x68 (ft/ftmain.c) | Hook into routine that checks if player collided with a star. Enables us to put a custom item ID check. | none in scope body (verify) | trivial C edit |
| src/Item.asm:555 | `nonzero_check_` | 0xE969C | 0x8016EC5C | `itManagerMakeAppearActor`+0x1C (it/itmanager.c) | extend item spawn - rate non-zero check | none in scope body (verify) | trivial C edit |
| src/Item.asm:577 | `rate_sum_` | 0xE96E4 | 0x8016ECA4 | `itManagerMakeAppearActor`+0x64 (it/itmanager.c) | extend item spawn - rate sum | none in scope body (verify) | trivial C edit |
| src/Item.asm:615 | `item_count_` | 0xE98A0 | 0x8016EE60 | `itManagerMakeAppearActor`+0x220 (it/itmanager.c) | extend item spawn - item count | none in scope body (verify) | trivial C edit |
| src/Item.asm:653 | `item_arrays_` | 0xE9954 | 0x8016EF14 | `itManagerMakeAppearActor`+0x2D4 (it/itmanager.c) | extend item spawn - item IDs and rate ranges | none in scope body (verify) | trivial C edit |
| src/Item.asm:694 | `rate_sum_item_generators_` | 0xE99EC | 0x8016EFAC | `itManagerSetupContainerDrops`+0x6C (it/itmanager.c) | extend exploded crate/barrel/egg/capsule item generation - rate sum | none in scope body (verify) | trivial C edit |
| src/Item.asm:732 | `item_count_item_generators_` | 0xE9A3C | 0x8016EFFC | `itManagerSetupContainerDrops`+0xBC (it/itmanager.c) | extend exploded crate/barrel/egg/capsule item generation - item count | none in scope body (verify) | trivial C edit |
| src/Item.asm:770 | `item_arrays_item_generators_` | 0xE9BD0 | 0x8016F190 | `itManagerSetupContainerDrops`+0x250 (it/itmanager.c) | extend exploded crate/barrel/egg/capsule item generation - item IDs and rate ranges | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:836 | `fix_item_id_check_item_generators_` | 0xF3EC8 | 0x80179488 | `itBoxCommonCheckSpawnItems`+0x64 (it/itcommon/itbox.c) | fix item ID checks for item generators | none in scope body (verify) | trivial C edit |
| src/Item.asm:842 | `fix_item_id_check_item_generators_` | 0xEDB3C | 0x801730FC | `itMainMakeContainerItem`+0x28 (it/itmain.c) | fix item ID checks for item generators | none in scope body (verify) | trivial C edit |
| src/Item.asm:877 | `always_update_item_frequency_` | 0x128FEC | 0x80132A4C | `mnVSModeMakeRuleButton` (mn/mnvsmode/mnvsmode.c) | Update item frequency regardless if all items are 'OFF' Note: Vanilla behaviour is lazy and will only do it if at least 1 Item is enabled | none in scope body (verify) | trivial C edit |
| src/Item.asm:895 | `prevent_1p_custom_item_spawns_` | 0x10BF5C | 0x8018D6FC | `scExplainProcUpdateControlStickSprite`+0x20 (sc/sccommon/scexplain.c) | Prevent custom items from spawning in 1P mode | none in scope body (verify) | trivial C edit |
| src/Item.asm:996 | `initialize_` | 0x128D48 | 0x801327A8 | `scStaffrollMakeHighlightGObj`+0x4 (sc/sccommon/scstaffroll.c) | initialize from bitmask | none in scope body (verify) | trivial C edit |
| src/Item.asm:1049 | `save_` | 0x129024 | 0x80132A84 | `mnVSRecordMakePortraitStatsArrows`+0x8 (mn/mndata/mnvsrecord.c) | save to bitmask | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:1127 | `item_switch_cursor_` | 0x129450 | 0x80132EB0 | `mnVSOptionsMakeWallpaper`+0x4 (mn/mnvsmode/mnvsoptions.c) | Modifies cursor movement and enables pagination Also allows L button to toggle all items ON/OFF | none in scope body (verify) | trivial C edit |
| src/Item.asm:1131 | `item_switch_cursor_` | 0x129330 | 0x80132D90 | `mnVSRecordDrawBattleScoreGrid` (mn/mndata/mnvsrecord.c) | Modifies cursor movement and enables pagination Also allows L button to toggle all items ON/OFF | none in scope body (verify) | trivial C edit |
| src/Item.asm:1136 | `handle_r_l_` | 0x129620 | 0x80133080 | `sc1PIntroSetKirbyTeamModelPartIDs` (sc/sc1pmode/sc1pintro.c) | handle R or L press | none in scope body (verify) | needs table |
| src/Item.asm:1303 | `restrict_cursor_` | 0x1291E0 | 0x80132C40 | `mnCharactersMakeFighter` (mn/mndata/mncharacters.c) | restrict cursor on page 1 | none in scope body (verify) | trivial C edit |
| src/Item.asm:1309 | `restrict_cursor_` | 0x1292C0 | 0x80132D20 | `mnPlayersVSMakePortraitCamera`+0x4 (mn/mnplayers/mnplayersvs.c) | restrict cursor on page 1 | none in scope body (verify) | trivial C edit |
| src/Item.asm:1338 | `handle_custom_item_values_` | 0x129518 | 0x80132F78 | `func_ovl56_80132F78` (sc/sc1pmode/sc1pstageclear.c) | handle ON/OFF updates for custom items | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:1344 | `handle_custom_item_values_` | 0x1293F4 | 0x80132E54 | `mnOptionFuncRun`+0x8 (mn/mnoption/mnoption.c) | handle ON/OFF updates for custom items | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:1350 | `handle_custom_item_values_` | 0x1294EC | 0x80132F4C | `mvOpeningRoomMakeWallpaper`+0x8 (mv/mvopening/mvopeningroom.c) | handle ON/OFF updates for custom items | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:1356 | `handle_custom_item_values_` | 0x1293CC | 0x80132E2C | `sc1PIntroMakeAllyText` (sc/sc1pmode/sc1pintro.c) | handle ON/OFF updates for custom items | none in scope body (verify) | trivial C edit |
| src/Item.asm:1362 | `handle_custom_item_values_` | 0x1295F0 | 0x80133050 | `mnPlayersVSMakePlayerKindCamera`+0x14 (mn/mnplayers/mnplayersvs.c) | handle ON/OFF updates for custom items | none in scope body (verify) | trivial C edit |
| src/Item.asm:1588 | `clear_items_in_1p_` | 0x10E5A0 | 0x8018FD40 | `sc1PTrainingModeUnderlineProcDisplay`+0x60 (sc/sc1pmode/sc1ptrainingmode.c) | This ensures custom items are cleared at the start of 1p matches. | none in scope body (verify) | trivial C edit |
| src/Item.asm:1594 | `clear_items_in_1p_` | 0x113000 | 0x8018E8C0 | `sc1PTrainingModeMakeCPDisplay`+0x50 (sc/sc1pmode/sc1ptrainingmode.c) | This ensures custom items are cleared at the start of 1p matches. | none in scope body (verify) | trivial C edit |
| src/Item.asm:1621 | `skip_item_spawn_gfx_` | 0xE9518 | 0x8016EAD8 | `itManagerMakeItemSetupCommon`+0x60 (it/itmanager.c) | Enables skipping item spawn gfx. | none in scope body (verify) | needs new state |
| src/Item.asm:1625 | `skip_item_spawn_gfx_` | 0xED7F8 | 0x80172DB8 | `itMainSetFighterHold`+0x114 (it/itmain.c) | Enables skipping item spawn gfx. | none in scope body (verify) | needs new state |
| src/Item.asm:1670 | `skip_item_pickup_sound_` | 0xED850 | 0x80172E10 | `itMainSetFighterHold`+0x16C (it/itmain.c) | Enables skipping item pick sound. | none in scope body (verify) | needs new state |
| src/Item.asm:1729 | `match_begin_with_item_` | 0x8D9F4 | 0x801121F4 | `ifCommonAnnounceGoSetStatus`+0x30 (if/ifcommon.c) | hook in routine that grants players control after "3, 2, 1 GO!". Runs for each player + port. Allows us to add a custom item as soon as player begins match | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:1940 | `taunt_spawn_item_` | 0xC918C | 0x8014E74C | `ftCommonAppealSetStatus`+0x6C (ft/ftcommon/ftcommonappeal.c) | Allows players to spawn items via taunt. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2281 | `whos_that_pokemon` | 0xEDCA0 | 0x80173260 | `itMainMakeMonster`+0x38 (it/itmain.c) | Pokemon item menu gate Handles Pokeball selecting Pokemon type (respecting ON/OFF values) | `entry_pokemon_mew` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2501 | `whos_that_pokemon_setup` | 0xE9C58 | 0x8016F218 | `itManagerInitMonsterVars` (it/itmanager.c) | Set the total number of Pokemon to pick | none in scope body (verify) | trivial C edit |
| src/Item.asm:2538 | `pokemon_variant_sfx` | 0xF8D14 | 0x8017E2D4 | `itKabigonFallInitVars`+0x78 (it/itmonster/itkabigon.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2543 | `pokemon_variant_sfx` | 0xF91B4 | 0x8017E774 | `itKabigonMakeItem`+0xB4 (it/itmonster/itkabigon.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2548 | `pokemon_variant_sfx` | 0xF9290 | 0x8017E850 | `itTosakintoAppearSetStatus`+0x28 (it/itmonster/ittosakinto.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2553 | `pokemon_variant_sfx` | 0xFB3CC | 0x8018098C | `itKamexAppearSetStatus`+0x28 (it/itmonster/itkamex.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2558 | `pokemon_variant_sfx` | 0xFBC10 | 0x801811D0 | `itMLuckyAppearSetStatus`+0x24 (it/itmonster/itmlucky.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2563 | `pokemon_variant_sfx` | 0xFD934 | 0x80182EF4 | `itDogasAttackInitVars`+0x7C (it/itmonster/itdogas.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2568 | `pokemon_variant_sfx` | 0xFE1B4 | 0x80183774 | `itPippiMakeItem`+0xE4 (it/itmonster/itpippi.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2573 | `pokemon_variant_sfx` | 0xFEB7C | 0x8018413C | `itHitokageMakeItem`+0xE4 (it/itground/ithitokage.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2578 | `pokemon_variant_sfx` | 0xFF240 | 0x80184800 | `itFushigibanaMakeItem`+0xF4 (it/itground/itfushigibana.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2583 | `pokemon_variant_sfx` | 0xF70AC | 0x8017C66C | `itGLuckyMakeItem`+0x78 (it/itground/itglucky.c) | Handles Pokemon regional variant Japanese Voices | `entry_pokemon_voices` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2655 | `item_containers_spawn_toggle` | 0xE99A4 | 0x8016EF64 | `itManagerSetupContainerDrops`+0x24 (it/itmanager.c) | Item Containers Spawn Handles disabling item containers (Crates, Barrels, Capsules) Note: Eggs are (almost) always enabled for Chansey container bit of mask is crate (1) + barrel (2) + capsule (4) + egg (8) | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2719 | `item_containers_explosions` | 0xF40EC | 0x801796AC | `itBoxCommonProcHit`+0x10 (it/itcommon/itbox.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2735 | `item_containers_explosions` | 0xF4290 | 0x80179850 | `itBoxThrownProcMap`+0x24 (it/itcommon/itbox.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2753 | `item_containers_explosions` | 0xF4038 | 0x801795F8 | `itBoxCommonCheckSpawnItems`+0x1D4 (it/itcommon/itbox.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2771 | `item_containers_explosions` | 0xEEA44 | 0x80174004 | `itCapsuleCommonProcHit`+0x10 (it/itcommon/itcapsule.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2786 | `item_containers_explosions` | 0xEEBC4 | 0x80174184 | `itCapsuleThrownProcMap`+0x24 (it/itcommon/itcapsule.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2801 | `item_containers_explosions` | 0xF468C | 0x80179C4C | `itTaruCommonProcHit`+0x2C (it/itcommon/ittaru.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit (long asm body, rewrite logic) |
| src/Item.asm:2817 | `item_containers_explosions` | 0xFC090 | 0x80181650 | `itEggCommonProcHit`+0x10 (it/itcommon/itegg.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit |
| src/Item.asm:2832 | `item_containers_explosions` | 0xFC25C | 0x8018181C | `itEggThrownProcMap`+0x24 (it/itcommon/itegg.c) | Item Containers Explosion Toggle Handles random explosiveness of item containers (Crates, Barrels, Capsules, Eggs) Note: the contents of the containers don't change, just whether or not they blow up 0 if DEFAULT, 1 if OFF, 2 if NO_EXPLODE,  | `entry_item_containers` | trivial C edit |
| src/Item.asm:2864 | `item_drop` | 0xCB5EC | 0x80150BAC | `ftCommonAttackAirCheckInterruptCommon`+0xAC (ft/ftcommon/ftcommonattackair.c) | item drop toggle for dropping items while aerial | `entry_item_dropping` | trivial C edit |
| src/Item.asm:2894 | `prevent_item_despawn_` | 0xEE644 | 0x80173C04 | `itMapCheckDestroyDropped`+0xE0 (it/itmap.c) | Patch which prevents Marina's items from being destroyed when landing on floors. | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/Item.asm:102 | `Item` | `{item_info_array_origin}` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:106 | `Item` | `Item.EXTENDED_ITEM_SPAWN_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:110 | `Item` | `Item.ITEM_INFO_ARRAY_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:114 | `Item` | `Item.ITEM_PICKUP_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:118 | `Item` | `Item.ITEM_DROP_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:122 | `Item` | `Item.ITEM_THROW_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:126 | `Item` | `Item.PLAYER_COLLISION_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:130 | `Item` | `Item.EXTENDED_ITEM_PRE_PICKUP_TABLE_ORIGIN + (Item.current_item * 0x4)` | data/table write, no RAM hook; decomp n/a |
| src/Item.asm:134 | `Item` | `Item.SPAWN_GFX_TABLE_ORIGIN + Item.current_item` | data/table write, no RAM hook; decomp n/a |

## `src/JabLock.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/JabLock.asm:7 | `down_initial_` | 0xBEF44 | 0x80144504 | `ftCommonDownBounceSetStatus`+0x6C (ft/ftcommon/ftcommondownwaitbounce.c) | When the character begins the DownBounceD or DownBounceU action, set the lock bool to FALSE | `entry_jab_lock` | trivial C edit |
| src/JabLock.asm:29 | `handle_lock_` | 0xBBDDC | 0x8014139C | `ftCommonDamageInitDamageVars`+0x4B8 (ft/ftcommon/ftcommondamage.c) | If the character gets hit by a low knockback hitbox while in DownBounceD or DownBounceU, restart that action rather than using a damage action. | `entry_jab_lock` | trivial C edit |
| src/JabLock.asm:67 | `handle_reset_` | 0xBF38C | 0x8014494C | `ftCommonDownAttackCheckInterruptDownBounce`+0x8 (ft/ftcommon/ftcommondownattack.c) | If the character reaches the end of DownBounceD/DownBounceU and the lock bool is set, force the character into DownStandD/DownStandU | `entry_jab_lock` | trivial C edit |

## `src/Japan.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Japan.asm:30 | `hitlag_` | 0x000659D0 | 0x800EA1D0 | `ftParamGetHitLag`+0x10 (ft/ftparam.c) | Toggle for Japanese Style Hitlag. The hitlag percentage is identical for the American and Japanese versions, it is present at 8012FF50. However, the calculation changes very slightly in a way that increases hitlag in the US Version, thus al | `entry_hitlag` | trivial C edit (long asm body, rewrite logic) |
| src/Japan.asm:107 | `japanese_shieldstun_` | 0xC3B78 | 0x80149138 | `ftCommonGuardSetOffSetStatus`+0x30 (ft/ftcommon/ftcommonguard2.c) | Toggle for Japanese Style Shield Stun. The length of shield stun is different in the international vs J versions. In the US 0x3FCF5C29 (1.62) and in Japan 0x3FE00000 (1.75) | `entry_shieldstun` | trivial C edit (long asm body, rewrite logic) |
| src/Japan.asm:206 | `momentum_slide_` | 0x000B9B74 | 0x8013F134 | `ftCommonRunBrakeCheckInterruptTurnRun`+0x48 (ft/ftcommon/ftcommonrunbrake.c) | Toggle for the Momentum Sliding Glitch present in the Japanese version. A very straightforward fix was added to the international version of Smash During a momentum slide input it checks the current velocity of the character against charact | `entry_momentum_slide` | trivial C edit |
| src/Japan.asm:228 | `japanese_stun_mash` | 0xC434C | 0x8014990C | `ftCommonFuraFuraSetStatus`+0x68 (ft/ftcommon/ftcommonfurafura.c) | Toggle for Japanese stun formula The formula for U stun is (400 - percentage) + 90. The J formula is (400 - percentage) + 60. | `entry_j_stun_sleep` | trivial C edit |
| src/Japan.asm:244 | `japanese_sleep_mash` | 0xC4428 | 0x801499E8 | `ftCommonFuraSleepSetStatus`+0x44 (ft/ftcommon/ftcommonfurasleep.c) | Toggle for Japanese sleep formula The formula for U sleep is (300 - percentage) + 75. The J formula is (300 - percentage) + 40. | `entry_j_stun_sleep` | trivial C edit |
| src/Japan.asm:272 | `japanese_sfx_` | 0x0005B06C | 0x800DF86C | `ftMainParseMotionEvent`+0x77C (ft/ftmain.c) | This changes the hit and beam sword sound effects for J characters or all characters if J sound toggle is ALWAYS | `entry_japanese_sounds` | trivial C edit (long asm body, rewrite logic) |
| src/Japan.asm:349 | `japanese_sfx_link_bomb_collide_` | 0x000E8F88 | 0x8016E548 | `itManagerMakeItem`+0x3D4 (it/itmanager.c) | This changes the sound effects for when Link's bomb hits another bomb for J characters or all characters if J sound toggle is ALWAYS It is coded to be more generic, so may apply to other sounds that we don't know about | `entry_japanese_sounds` | trivial C edit (long asm body, rewrite logic) |
| src/Japan.asm:418 | `japanese_sfx_link_boomerang_collide_` | 0xE045C | 0x80165A1C | `wpManagerMakeWeapon`+0x454 (wp/wpmanager.c) | This changes the sound effects for when Link's boomerang hits a Link bomb for J characters or all characters if J sound toggle is ALWAYS It is coded to be more generic, so may apply to other sounds that we don't know about | none in scope body (verify) | trivial C edit |
| src/Japan.asm:455 | `japanese_sfx_projectile_` | 0x0005ED9C | 0x800E359C | `ftMainUpdateDamageStatWeapon`+0x184 (ft/ftmain.c) | This changes the sound effects for when J characters' projectiles hit a character or for all characters if J sound toggle is ALWAYS | `entry_japanese_sounds` | trivial C edit (long asm body, rewrite logic) |
| src/Japan.asm:532 | `reflect_` | 0xE1814 | 0x80166DD4 | `wpProcessProcHitCollisions`+0x1F0 (wp/wpprocess.c) | This changes the reflect multiplier for J characters | none in scope body (verify) | trivial C edit |

## `src/Joypad.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Joypad.asm:390 | `set_taunt_mask_` | 0x53CD8 | 0x800D84D8 | `ftManagerMakeFighter`+0x59C (ft/ftmanager.c) | Allows overriding button mask for taunts per port. | `entry_single_button_mode` | needs new state |
| src/Joypad.asm:430 | `prevent_taunt_jump_` | 0xB9E94 | 0x8013F454 | `ftCommonKneeBendCheckButtonTap`+0x4 (ft/ftcommon/ftcommonkneebend.c) | This hook prevents the remapped taunt button from causing a jump | `entry_single_button_mode` | trivial C edit |
| src/Joypad.asm:436 | `prevent_taunt_jump_` | 0xBAB70 | 0x80140130 | `ftCommonJumpAerialMultiCheckJumpButtonHold`+0x4 (ft/ftcommon/ftcommonjumpaerial.c) | This hook prevents the remapped taunt button from causing a jump | `entry_single_button_mode` | trivial C edit |

## `src/KingOfTheHill.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/KingOfTheHill.asm:304 | `allow_3_digits_for_picker_` | 0x1321F8 | 0x80133F78 | `mnVSResultsMakeString`+0xC (mn/mnvsmode/mnvsresults.c) | This allows 3 digits to be rendered in the yellow value scroller at the top and repositions when 3 digits are present | none in scope body (verify) | trivial C edit |
| src/KingOfTheHill.asm:1712 | `ai_hill_behavior_` | 0xAF5C4 | 0x80134B84 | `mnPlayers1PGameGetTotalHiScore`+0x20 (mn/mnplayers/mnplayers1pgame.c) | Makes CPUs seek out the hill, sorta. This makes them ignore items and tornados, too... oh well! | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/KingOfTheHill.asm:1771 | `delay_respawn_` | 0xB6B6C | 0x8013C12C | `ftCommonDeadInitStatusVars`+0xC (ft/ftcommon/ftcommondead.c) | Delays respawn | none in scope body (verify) | trivial C edit |
| src/KingOfTheHill.asm:1776 | `delay_respawn_` | 0xB6B38 | 0x8013C0F8 | `ftCommonDeadCommonProcUpdate`+0xC (ft/ftcommon/ftcommondead.c) | Delays respawn | none in scope body (verify) | trivial C edit |

## `src/KirbyHats.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/KirbyHats.asm:73 | `use_extended_special_parts_copy_` | 0x64774 | 0x800E8F74 | `ftParamResetModelPartAll`+0xA8 (ft/ftparam.c) | This catches when Kirby copies a power and allows us to use custom "hats" | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:112 | `use_extended_special_parts_pause_` | 0x64504 | 0x800E8D04 | `ftParamSetModelPartID`+0x94 (ft/ftparam.c) | This catches when the Kirby hat switches to high poly from low poly during a pause and allows us to use custom "hats". Also runs when Kirby does Yoshi's NSP. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/KirbyHats.asm:244 | `use_extended_special_parts_costume_init_` | 0x64AFC | 0x800E92FC | `ftParamInitAllParts`+0xB4 (ft/ftparam.c) | This seems to be related to costume loading and initializing things to use the right special images/tracks. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/KirbyHats.asm:526 | `set_absorbed_hat_` | 0xDCB50 | 0x80162110 | `ftKirbySpecialNCatchProcUpdate`+0x98 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:530 | `set_absorbed_hat_` | 0xDC97C | 0x80161F3C | `ftKirbySpecialNCopyInitCopyVars`+0x30 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | (no description comment) | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:575 | `kirby_hat_select_` | 0x53608 | 0x800D7E08 | `ftManagerInitFighter`+0x418 (ft/ftmanager.c) | This hooks into a kirby spawning routine which loads his hat, we substitute the desired hat here | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/KirbyHats.asm:652 | `kirby_hat_files_training_` | 0x116B60 | 0x80190340 | `sc1PTrainingModeFuncStart`+0xE0 (sc/sc1pmode/sc1ptrainingmode.c) | Loads kirby files when needed for debug options in training mode | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:685 | `kirby_hat_files_vs_` | 0x10A29C | 0x8018D3AC | `SCAutoDemoProcFocusPlayer1`+0x10 (sc/sccommon/scautodemo.c) | Loads kirby files when needed for debug options in vs mode | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:690 | `kirby_hat_files_vs_` | 0x10ADE8 | 0x8018DEF8 | `sc1PTrainingModeMakeStatDisplay`+0x1C (sc/sc1pmode/sc1ptrainingmode.c) | Loads kirby files when needed for debug options in vs mode | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:726 | `kirby_hat_files_bonus_` | 0x112DFC | 0x8018E6BC | `sc1PGameWaitStageCommonUpdate`+0x4C (sc/sc1pmode/sc1pgame.c) | Loads kirby files when needed for debug options in Bonus mode Should only really check the player's port but I'm too lazy! | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:759 | `kirby_hat_files_1p_` | 0x10E2D4 | 0x8018FA74 | `func_ovl7_8018FA54`+0x20 (sc/sc1pmode/sc1ptrainingmode.c) | Loads kirby files when needed for debug options in 1p Should only really check the player's port but I'm too lazy! | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:791 | `hat_loss_prevent_` | 0xDE034 | 0x801635F4 | `ftKirbySpecialNLoseCopy`+0x8 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Prevents kirby from losing hat via hit or taunt when hat set | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:825 | `kirby_power_loss_prevent_` | 0xDE060 | 0x80163620 | `ftKirbySpecialNLoseCopy`+0x34 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Prevents kirby from having his power ID set to 0 | none in scope body (verify) | trivial C edit |
| src/KirbyHats.asm:855 | `set_absorbed_` | 0xC6A74 | 0x8014C034 | `ftCommonThrownCommonStarUpdatePhysics`+0x130 (ft/ftcommon/ftcommoncapturekirby.c) | Set magic_hat_on flag to false when absorbed | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/KirbyHats.asm:28 | `file_index` | `EXTENDED_SPECIAL_PARTS_ORIGIN + (current_custom_hat_id * 0x20)` | data/table write, no RAM hook; decomp n/a |

## `src/Knockback.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/Knockback.asm:15 | `randomize_angles_` | 0x53808 | 0x800D8008 | `ftManagerMakeFighter`+0xCC (ft/ftmanager.c) | Subroutine which randomizes knockback angles for each action when the character is initialized. | none in scope body (verify) | needs new state |
| src/Knockback.asm:142 | `get_angle_hitbox_` | 0x5AC24 | 0x800DF424 | `ftMainParseMotionEvent`+0x334 (ft/ftmain.c) | Gets the new randomized angle for the current action when a hitbox is created. | none in scope body (verify) | trivial C edit |
| src/Knockback.asm:183 | `get_angle_throw_` | 0xC5BAC | 0x8014B16C | `ftCommonThrownReleaseThrownUpdateStats`+0x19C (ft/ftcommon/ftcommonthrown2.c) | Gets the new randomized angle for the current action when a throw is performed. | none in scope body (verify) | trivial C edit |
| src/Knockback.asm:220 | `get_angle_throw_` | 0xBB3FC | 0x801409BC | `ftCommonDamageGetKnockbackAngle` (ft/ftcommon/ftcommondamage.c) | Gets the new randomized angle for the current action when a throw is performed. | none in scope body (verify) | trivial C edit |

## `src/LedgeJump.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/LedgeJump.asm:24 | `extend_ledge_input_check` | 0xBF81C | 0x80144DDC | `ftCommonCliffWaitProcInterrupt`+0x38 (ft/ftcommon/ftcommoncliffcatchwait.c) | Extend ledge hold input check to allow ledge jumping if the toggle is enabled. | `entry_ledge_jump` | trivial C edit |
| src/LedgeJump.asm:73 | `extend_under_100_ledge_options` | 0xBF964 | 0x80144F24 | `ftCommonCliffQuickProcUpdate`+0x44 (ft/ftcommon/ftcommoncliffclimb.c) | Jump if player is inputting a c jump while under 100 % | none in scope body (verify) | trivial C edit |
| src/LedgeJump.asm:89 | `extend_over_100_ledge_options` | 0xBF9E8 | 0x80144FA8 | `ftCommonCliffSlowProcUpdate`+0x44 (ft/ftcommon/ftcommoncliffclimb.c) | Jump if player is inputting a c jump while over 100 % | none in scope body (verify) | trivial C edit |

## `src/LedgeTrump.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/LedgeTrump.asm:22 | `ledge_check` | 0x59E6C | 0x800DE66C | `mpCommonRunFighterSpecialCollisions`+0x210 (mp/mpcommon.c) | Adds a hook to the logic that does nothing if a player is trying to grab a ledge s0 = aerial player (struct) v0 = player on cliff (struct) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/LedgeTrump.asm:86 | `track_ledge_grabs` | 0x59E88 | 0x800DE688 | `mpCommonRunFighterSpecialCollisions`+0x22C (mp/mpcommon.c) | Adds a hook to ledge grab logic that increments VS Stats ledge tracker | none in scope body (verify) | trivial C edit |

## `src/captainshared.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/captainshared.asm:8 | `CaptainShared` | 0xA9AF5 | none (ROM-only patch) | n/a (no RAM target) | (no description comment) | none in scope body (verify) | needs new state |
| src/captainshared.asm:11 | `CaptainShared` | 0xA9B00 | none (ROM-only patch) | n/a (no RAM target) | (no description comment) | none in scope body (verify) | needs new state |
| src/captainshared.asm:17 | `CaptainShared` | 0xA9AD4 | 0x8012E2D4 | unmatched | (no description comment) | none in scope body (verify) | needs new state |
| src/captainshared.asm:122 | `get_kick_anim_struct_` | 0x7D6E4 | 0x80101EE4 | `efManagerCaptainFalconKickMakeEffect`+0xC (ef/efmanager.c) | loads a different animation struct when Ganondorf uses his down special. | none in scope body (verify) | trivial C edit |
| src/captainshared.asm:172 | `get_punch_anim_struct_` | 0x7D790 | 0x80101F90 | `efManagerCaptainFalconPunchMakeEffect`+0xC (ef/efmanager.c) | loads a different animation struct when Ganondorf uses his neutral special. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/captainshared.asm:234 | `get_entry_anim_struct_` | 0x7EDA0 | 0x801035A0 | `efManagerCaptainEntryCarMakeEffect`+0x34 (ef/efmanager.c) | loads a different animation struct when Ganondorf uses his entry animation. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/captainshared.asm:295 | `get_entry_file_ptr_` | 0x7EDBC | 0x801035BC | `efManagerCaptainEntryCarMakeEffect`+0x50 (ef/efmanager.c) | loads the correct file pointer when Ganondorf uses his entry animation. | none in scope body (verify) | trivial C edit |
| src/captainshared.asm:339 | `get_punch_bone_` | 0x7D7C8 | 0x80101FC8 | `efManagerCaptainFalconPunchMakeEffect`+0x44 (ef/efmanager.c) | adds a check for Ganondorf to a routine which determines which bone the punch graphic is attached to | none in scope body (verify) | trivial C edit |
| src/captainshared.asm:383 | `clown_car_animation1` | 0x7EDC8 | 0x801035C8 | `efManagerCaptainEntryCarMakeEffect`+0x5C (ef/efmanager.c) | loads the correct t7 digit for Clown Car | none in scope body (verify) | trivial C edit |
| src/captainshared.asm:412 | `clown_car_animation2` | 0x7EDF4 | 0x801035F4 | `efManagerCaptainEntryCarMakeEffect`+0x88 (ef/efmanager.c) | loads the correct s3 and s4 digits for Clown Car animation | none in scope body (verify) | trivial C edit |
| src/captainshared.asm:444 | `kirby_power_change_` | 0xDC90C | 0x80161ECC | `ftKirbySpecialNInitPassiveVars`+0x18 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Extends a check on ID that occurs when Kirby absorbs or ejects a power. | none in scope body (verify) | trivial C edit |

## `src/command.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/command.asm:150 | `end_hitbox_` | 0x63D2C | 0x800E852C | `ftParamClearAttackCollAll`+0x14 (ft/ftparam.c) | patch which expands the existing end hitbox function to include resetting the directional override and FGM override | none in scope body (verify) | trivial C edit |
| src/command.asm:190 | `create_hitbox_` | 0x5A9F0 | 0x800DF1F0 | `ftMainParseMotionEvent`+0x100 (ft/ftmain.c) | patch which expands the existing create hitbox function to include resetting the directional override and FGM override | none in scope body (verify) | trivial C edit |
| src/command.asm:232 | `texture_form_fix_` | 0x64E2C | 0x800E962C | `ftParamSetTexturePartID` (ft/ftparam.c) | patch which causes the "Set Texture Form" command to not take effect for Metal Mario | none in scope body (verify) | trivial C edit |
| src/command.asm:612 | `apply_ground_z_` | 0x5442C | 0x800D8C2C | `ftPhysicsApplyGroundVelTransN`+0x18 (ft/ftphysics.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/command.asm:664 | `apply_ground_x_` | 0x54468 | 0x800D8C68 | `ftPhysicsApplyGroundVelTransN`+0x54 (ft/ftphysics.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/command.asm:716 | `apply_air_z_` | 0x54A94 | 0x800D9294 | `ftPhysicsGetAirVelTransN`+0x34 (ft/ftphysics.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/command.asm:768 | `apply_air_y_` | 0x54AB0 | 0x800D92B0 | `ftPhysicsGetAirVelTransN`+0x50 (ft/ftphysics.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/command.asm:821 | `apply_air_x_` | 0x54B64 | 0x800D9364 | `ftPhysicsGetAirVelTransN`+0x104 (ft/ftphysics.c) | (no description comment) | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/command.asm:921 | `apply_fgm_` | 0x5E4A8 | 0x800E2CA8 | `ftMainPlayHitSFX`+0x84 (ft/ftmain.c) | Overrides the FGM for a hitbox if the corresponding hitbox FGM parameter is set | none in scope body (verify) | needs new state |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/command.asm:581 | `apply_direction_` | `0x5FC6C` | data/table write, no RAM hook; decomp n/a |
| src/command.asm:1155 | `skip_size_8_` | `0x5BB88` | data/table write, no RAM hook; decomp n/a |
| src/command.asm:1159 | `skip_size_8_` | `0x5BD48` | data/table write, no RAM hook; decomp n/a |
| src/command.asm:1163 | `skip_size_8_` | `0x5BF2C` | data/table write, no RAM hook; decomp n/a |
| src/command.asm:1168 | `skip_size_8_` | `0x6272C` | data/table write, no RAM hook; decomp n/a |

## `src/dkshared.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/dkshared.asm:9 | `cargo_hold_fix_1` | 0xC5564 | 0x8014AB24 | `ftCommonThrownProcUpdate`+0x34 (ft/ftcommon/ftcommonthrown1.c) | character ID check add for when Donkey Kong Clones are performing cargo hold | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:34 | `cargo_hold_fix_2` | 0xC4BCC | 0x8014A18C | `ftCommonThrowProcUpdate`+0xCC (ft/ftcommon/ftcommonthrow.c) | character ID check add for when Donkey Kong Clones are performing cargo hold Performs action id check for Action.ThrowF | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:58 | `item_fix_1` | 0xC0860 | 0x80145E20 | `ftCommonGetProcUpdate`+0xB0 (ft/ftcommon/ftcommonget.c) | character ID check add for when Donkey Kong Clones are grabbing a barrel or box | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:82 | `item_fix_2` | 0xC0E60 | 0x80146420 | `ftCommonItemThrowProcUpdate`+0xD4 (ft/ftcommon/ftcommonitemthrow.c) | character ID check add for when Donkey Kong Clones are throwing an item | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:106 | `item_fix_3` | 0xC1488 | 0x80146A48 | `ftCommonHeavyThrowDecideSetStatus`+0x118 (ft/ftcommon/ftcommonitemthrow.c) | character ID check add for when Donkey Kong Clones are throwing a box or barrel | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:130 | `item_fix_4` | 0x65B44 | 0x800EA344 | `ftParamUpdateDamage`+0xFC (ft/ftparam.c) | character ID check add for when Donkey Kong Clones getting hit while holding a box or barrel | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:154 | `item_fix_5` | 0xBC3F4 | 0x801419B4 | `ftCommonDamageUpdateMain`+0x344 (ft/ftcommon/ftcommondamage.c) | character ID check add for when Donkey Kong Clones getting hit while holding a box or barrel | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:178 | `item_fix_6` | 0xBB804 | 0x80140DC4 | `ftCommonDamageCheckCatchResist`+0x94 (ft/ftcommon/ftcommondamage.c) | character ID check add for when Donkey Kong Clones getting hit while holding a box or barrel | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:203 | `fully_charged_check_` | 0x66410 | 0x800EAC10 | `ftParamMakeEffect`+0x34 (ft/ftparam.c) | Extends a check on ID that occurs when fully charged. | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:225 | `kirby_power_check_flash_` | 0x65200 | 0x800E9A00 | `ftParamResetStatUpdateColAnim`+0x12C (ft/ftparam.c) | Extends check in end_overlay that allows a DK-powered Kirby to retain the charged flashing effect when fully charged. | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:246 | `kirby_power_change_` | 0xDC8FC | 0x80161EBC | `ftKirbySpecialNInitPassiveVars`+0x8 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Extends a check on ID that occurs when Kirby absorbs or ejects a power. | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:268 | `giant_punch_fix_1` | 0xB1670 | 0x80136C30 | `mnVSResultsOrderResults`+0x4 (mn/mnvsmode/mnvsresults.c) | character ID check add for when Donkey Kong Clones CPUs are functioning Performs action id check for Giant Punch | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:289 | `cpu_fix_2` | 0xB39B8 | 0x80138F78 | `func_ovl3_80138EE4`+0x94 (ft/ftcomputer.c) | character ID check add for when Donkey Kong Clones CPUs are functioning Performs action id check for Giant Punch and Cargo. Modified for Marina | none in scope body (verify) | trivial C edit |
| src/dkshared.asm:335 | `barrel_alternate` | 0x7EC28 | 0x80103428 | `efManagerDonkeyEntryTaruMakeEffect`+0x10 (ef/efmanager.c) | character ID check add for when Characters use the barrel as a base for an entry object | none in scope body (verify) | needs new state |

## `src/fireball.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/fireball.asm:223 | `bounce_destroy_check` | 0xE30A4 | 0x80168664 | `wpMarioFireballProcMap`+0x74 (wp/wpmario/wpmariofireball.c) | Banjos egg won't destroy itself so easilly | none in scope body (verify) | trivial C edit |
| src/fireball.asm:345 | `kirby_fix_` | 0xD1448 | 0x80156A08 | `ftKirbyCopyMarioSpecialNProcAccessory`+0x34 (ft/ftchar/ftkirby/ftkirbycopymariospecialn.c) | Updates the joint used to get the initial position of the projectile when Kirby copies Piano's power. | none in scope body (verify) | trivial C edit |
| src/fireball.asm:452 | `load_capsule_subroutine_` | 0xD0A40 | 0x80156000 | `ftMarioSpecialNInitStatusVars`+0x4 (ft/ftchar/ftmario/ftmariospecialn.c) | Loads the capsule subroutine when Dr. Mario uses neutral special. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/fireball.asm:457 | `load_capsule_subroutine_` | 0xD15FC | 0x80156BBC | `ftKirbyCopyMarioSpecialNInitStatusVars`+0x4 (ft/ftchar/ftkirby/ftkirbycopymariospecialn.c) | Loads the capsule subroutine when Dr. Mario uses neutral special. | none in scope body (verify) | trivial C edit |
| src/fireball.asm:511 | `capsule_flag` | 0xE324C | 0x8016880C | `wpMarioFireballMakeWeapon`+0x6C (wp/wpmario/wpmariofireball.c) | Saves a value of 1 in the projectile struct when Dr. Luigi uses neutral special to help determine the GFX used on hit. | none in scope body (verify) | trivial C edit |

| Line | Table origin scope | ROM origin expression | Note |
|---|---|---|---|
| src/fireball.asm:83 | `hit_effect_` | `0xE310C` | data/table write, no RAM hook; decomp n/a |
| src/fireball.asm:88 | `hit_effect_` | `0xE3124` | data/table write, no RAM hook; decomp n/a |
| src/fireball.asm:171 | `bounce_effect_` | `0xE30E0` | data/table write, no RAM hook; decomp n/a |

## `src/jigglypuffkirbyshared.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/jigglypuffkirbyshared.asm:12 | `jump_fix_1` | 0xBA9A8 | 0x8013FF68 | `ftCommonJumpAerialMultiSetStatus`+0x30 (ft/ftcommon/ftcommonjumpaerial.c) | character ID check add for when Jigglypuff Clones are doing 2+ jumps. This is the first of the double jump routines | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:44 | `jump_fix_2` | 0xBAA8C | 0x8014004C | `ftCommonJumpAerialMultiSetStatus`+0x114 (ft/ftcommon/ftcommonjumpaerial.c) | character ID check add for when Jigglypuff Clones are doing 3+ jumps | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:91 | `jump_fix_3` | 0xBAC20 | 0x801401E0 | `ftCommonJumpAerialCheckInterruptCommon`+0x44 (ft/ftcommon/ftcommonjumpaerial.c) | character ID check add for when Jigglypuff Clones are jumping/in the air. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:119 | `jump_fix_4` | 0xBA6F0 | 0x8013FCB0 | `ftCommonJumpAerialProcPhysics`+0x64 (ft/ftcommon/ftcommonjumpaerial.c) | character ID check add for when Jigglypuff Clones are doing 2+ jumps. This is the second of the double jump routines. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:151 | `jump_fix_5` | 0xBAC78 | 0x80140238 | `ftCommonJumpAerialCheckInterruptCommon`+0x9C (ft/ftcommon/ftcommonjumpaerial.c) | character ID check add for when Jigglypuff Clones are jumping. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:183 | `kirby_boomerangfix_1` | 0xE7DFC | 0x8016D3BC | `wpLinkBoomerangCheckOwnerCatch`+0x60 (wp/wplink/wplinkboomerang.c) | character ID check add for when Kirby Clones receive their boomerang. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:203 | `kirby_boomerangfix_2` | 0xE7D70 | 0x8016D330 | `wpLinkBoomerangClearGObjs`+0x14 (wp/wplink/wplinkboomerang.c) | character ID check add for when Kirby Clones receive their boomerang. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:223 | `kirby_special_struct_fix` | 0xE68E0 | 0x8016BEA0 | `wpKirbyCutterMakeWeapon`+0x14 (wp/wpkirby/wpkirbycutter.c) | Kirby has a hardcoded projectile struct for his up special similar to Ness and Link. This code inserts a new pointer to the clones main file so the game doesn't crash. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:253 | `kirby_infinite_1` | 0xC9C0C | 0x8014F1CC | `ftCommonAttack100LoopKirbyUpdateEffect`+0x10 (ft/ftcommon/ftcommonattack100.c) | character ID check add for when Kirby Clones use their infinite. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:273 | `kirby_fthrow_1_fix` | 0xC4C78 | 0x8014A238 | `ftCommonThrowSetStatus`+0x50 (ft/ftcommon/ftcommonthrow.c) | character ID check add for when Kirby Clones use forward throw. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:307 | `kirby_fthrow_2_fix` | 0xC4D8C | 0x8014A34C | `ftCommonThrowSetStatus`+0x164 (ft/ftcommon/ftcommonthrow.c) | character ID check add for when Kirby Clones use forward throw. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:327 | `kirby_blast_fix_1` | 0x531B0 | 0x800D79B0 | `ftManagerDestroyFighterWeapons`+0x1C (ft/ftmanager.c) | character ID check add for when Kirby and Link Clones hit the blast wall. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:365 | `kirby_load_with_hat_fix_` | 0x53654 | 0x800D7E54 | `ftManagerInitFighter`+0x464 (ft/ftmanager.c) | character ID check add for loading Kirby with a hat. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:387 | `kirby_taunt_fix_1` | 0xC9138 | 0x8014E6F8 | `ftCommonAppealSetStatus`+0x18 (ft/ftcommon/ftcommonappeal.c) | character ID check add for when Kirby Clones use taunt. This ensures that he abandons his power. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:408 | `kirby_taunt_fix_1` | 0xC914C | 0x8014E70C | `ftCommonAppealSetStatus`+0x2C (ft/ftcommon/ftcommonappeal.c) | character ID check add for when Kirby Clones use taunt. This ensures that he abandons his power. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:429 | `pokemon_spawn_fix` | 0xB8480 | 0x8013DA40 | `ftCommonAppearUpdateEffects`+0x2C (ft/ftcommon/ftcommonentry.c) | character ID check add for when Pikachu and Jigglypuff Clones use spawn on stage. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:460 | `pokemon_hat_fix_1` | 0x6D730 | 0x800F1F30 | `ftDisplayMainDrawDefault`+0xD0 (ft/ftdisplaymain.c) | character ID check add for when Pikachu and Jigglypuff Clones wear hats. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:487 | `pokemon_hat_fix_2` | 0x6D8E0 | 0x800F20E0 | `ftDisplayMainDrawDefault`+0x280 (ft/ftdisplaymain.c) | character ID check add for when Pikachu and Jigglypuff Clones wear hats. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:514 | `kirby_cpu_inhale` | 0xB165C | 0x80136C1C | `mnPlayersVSHandicapLevelProcUpdate`+0x4 (mn/mnplayers/mnplayersvs.c) | character ID check add for when Kirby Clones CPUs inhale an opponent. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:540 | `kirby_cpu_inhale_2` | 0xB3948 | 0x80138F08 | `func_ovl3_80138EE4`+0x24 (ft/ftcomputer.c) | character ID check add for when Kirby Clones CPUs inhale an opponent. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:562 | `kirby_cpu_inhale_3` | 0xB1B30 | 0x801370F0 | `mnPlayers1PGameMovePuck`+0x4 (mn/mnplayers/mnplayers1pgame.c) | character ID check add for when Kirby Clones CPUs inhale an opponent. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/jigglypuffkirbyshared.asm:618 | `kirby_cpu_inhale_4` | 0x5DBE8 | 0x800E23E8 | `ftMainProcPhysicsMap`+0x3A0 (ft/ftmain.c) | character ID check add for when Kirby Clones CPUs inhale an opponent. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:644 | `fully_charged_check_` | 0x66424 | 0x800EAC24 | `ftParamMakeEffect`+0x48 (ft/ftparam.c) | Extends a check on ID that occurs when fully charged. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:665 | `kirby_power_steal_check_` | 0xDCB3C | 0x801620FC | `ftKirbySpecialNCatchProcUpdate`+0x84 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Extends a check on ID that occurs when a Kirby clone steals another Kirby clone's copied power. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:686 | `kirby_power_stolen_check_` | 0xC6A4C | 0x8014C00C | `ftCommonThrownCommonStarUpdatePhysics`+0x108 (ft/ftcommon/ftcommoncapturekirby.c) | Extends a check on ID that occurs when a Kirby clone's copied power is stolen by another Kirby clone. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:707 | `kirby_power_change_` | 0xDC914 | 0x80161ED4 | `ftKirbySpecialNInitPassiveVars`+0x20 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Extends a check on ID that occurs when Kirby absorbs or ejects a power. | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:731 | `kirby_ammo_` | 0xDC9C0 | 0x80161F80 | `ftKirbySpecialNCopyInitCopyVars`+0x74 (ft/ftchar/ftkirby/ftkirbyspecialn.c) | Fills Kirby's flame ammo after Bowser absorption | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:750 | `kirby_rock` | 0xDBE64 | 0x80161424 | `ftKirbySpecialLwSetDamageResist`+0x8 (ft/ftchar/ftkirby/ftkirbyspeciallw.c) | Modifies J Kirby's rock form to have the same HP as he does in the J Version | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:771 | `kirby_rock_2` | 0xDBDB4 | 0x80161374 | `ftKirbySpecialLwUpdateColAnim`+0xC (ft/ftchar/ftkirby/ftkirbyspeciallw.c) | 80161368+C Modifies J Kirby's rock form to apply a color overlay based on same remaining HP as J Version | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:792 | `kirby_rock_3` | 0xDBF24 | 0x801614E4 | `ftKirbySpecialLwCheckRelease`+0x30 (ft/ftchar/ftkirby/ftkirbyspeciallw.c) | 801614B4+30 Modifies J Kirby's rock form to have the same minimum aerial duration as he does in the J Version | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:813 | `multi_jump_recovery_fix_` | 0xAFBD8 | 0x80135198 | `scStaffrollFuncDraw`+0x80 (sc/sccommon/scstaffroll.c) | Allows characters with slow/short aerial hops to make contact with the ledge better during recovering | none in scope body (verify) | trivial C edit |
| src/jigglypuffkirbyshared.asm:860 | `remix_skip_offensive_usp_air_` | 0xAE9B0 | 0x80133F70 | `mnVSResultsMakeString`+0x4 (mn/mnvsmode/mnvsresults.c) | May prevent puff copies from doing an up special to an opponent while they are aerial Hooks into check for vanilla puff | none in scope body (verify) | needs new state |

## `src/linkshared.asm`

| Line | Hook scope | ROM off | Vanilla RAM | Decomp match | Behavior | Toggle | Port difficulty |
|---|---|---|---|---|---|---|---|
| src/linkshared.asm:175 | `get_entry_anim_struct_1` | 0x7E2E8 | 0x80102AE8 | `efManagerLinkEntryWaveMakeEffect`+0x4 (ef/efmanager.c) | loads a different animation struct when Young Link or Marth use their entry animation. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/linkshared.asm:232 | `get_entry_anim_struct_2` | 0x7E344 | 0x80102B44 | `efManagerLinkEntryBeamMakeEffect`+0x4 (ef/efmanager.c) | loads a different animation struct when Young Link or Marth use their entry animation. | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/linkshared.asm:288 | `create_bomb_` | 0x100FE0 | 0x801865A0 | `itLinkBombMakeItem` (it/itfighter/itlinkbomb.c) | modifies a subroutine which runs when a Link bomb is created uses character id to determine which bomb struct to load stores character id in an active item struct uses a different render routine for Young Link | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/linkshared.asm:301 | `create_bomb_` | 0x101014 | 0x801865D4 | `itLinkBombMakeItem`+0x34 (it/itfighter/itlinkbomb.c) | modifies a subroutine which runs when a Link bomb is created uses character id to determine which bomb struct to load stores character id in an active item struct uses a different render routine for Young Link | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/linkshared.asm:307 | `create_bomb_` | 0x10101C | 0x801865DC | `itLinkBombMakeItem`+0x3C (it/itfighter/itlinkbomb.c) | modifies a subroutine which runs when a Link bomb is created uses character id to determine which bomb struct to load stores character id in an active item struct uses a different render routine for Young Link | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/linkshared.asm:313 | `create_bomb_` | 0x101098 | 0x80186658 | `itLinkBombMakeItem`+0xB8 (it/itfighter/itlinkbomb.c) | modifies a subroutine which runs when a Link bomb is created uses character id to determine which bomb struct to load stores character id in an active item struct uses a different render routine for Young Link | none in scope body (verify) | trivial C edit (long asm body, rewrite logic) |
| src/linkshared.asm:382 | `bomb_flash_` | 0x10041C | 0x801859DC | `itLinkBombExplodeWaitUpdateScale`+0x1C (it/itfighter/itlinkbomb.c) | modifies a subroutine which runs when the bomb is flashing loads character id from active item struct uses character id to determine which bomb struct to load | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:414 | `bomb_explosion_` | 0x100DF0 | 0x801863B0 | `itLinkBombExplodeUpdateAttackEvent`+0x4 (it/itfighter/itlinkbomb.c) | modifies a subroutine which runs when the bomb explodes loads character id from active item struct uses character id to determine which bomb struct to load | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:444 | `item_shield_fix_` | 0x63F0C | 0x800E870C | `ftParamLinkResetShieldModelParts`+0x1C (ft/ftparam.c) | adds a check for Young Link to 2 asm routines which are responsible for swapping Link's shield between his hand and back when he starts and finishes holding an item | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:449 | `item_shield_fix_` | 0x63F60 | 0x800E8760 | `ftParamSetHammerParams`+0x1C (ft/ftparam.c) | adds a check for Young Link to 2 asm routines which are responsible for swapping Link's shield between his hand and back when he starts and finishes holding an item | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:483 | `grab_shield_fix_` | 0xC4A98 | 0x8014A058 | `ftCommonCatchWaitSetStatus`+0x58 (ft/ftcommon/ftcommoncatch2.c) | adds a check for Young Link to an asm routine which is responsible for swapping Link's shield between his hand and back when he grabs an opponent | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:517 | `dair_fix_` | 0xCB334 | 0x801508F4 | `ftCommonAttackAirLwProcHit`+0x14 (ft/ftcommon/ftcommonattackair.c) | adds a check for Young Link to 2 asm routines which are responsible for determining the unique properties of Link's dair | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:522 | `dair_fix_` | 0xCB3D4 | 0x80150994 | `ftCommonAttackAirLwProcUpdate`+0x14 (ft/ftcommon/ftcommonattackair.c) | adds a check for Young Link to 2 asm routines which are responsible for determining the unique properties of Link's dair | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:556 | `sword_trail_fix_` | 0x61F68 | 0x800E6768 | `ftMainProcParams`+0x57C (ft/ftmain.c) | adds a check for Young Link to an asm routine which is responsible for updating the position of and drawing(?) Link's sword trail | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:590 | `up_special_fix_` | 0xE7594 | 0x8016CB54 | `wpLinkSpinAttackMakeWeapon`+0x38 (wp/wplink/wplinkspinattack.c) | modifies a subroutine which runs when Link uses up special uses character id to determine which up special struct to load not 100% sure what this struct is for | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:629 | `boomerang_fix_` | 0xE8500 | 0x8016DAC0 | `wpLinkBoomerangMakeWeapon`+0x48 (wp/wplink/wplinkboomerang.c) | modifies a subroutine which runs when Link uses neutral special uses character id to determine which boomerang struct to load not 100% sure what this struct is for | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:676 | `up_special_velocity_` | 0xDEDC8 | 0x80164388 | `ftLinkSpecialAirHiSetStatus`+0x40 (ft/ftchar/ftlink/ftlinkspecialhi.c) | modifies a subroutine which runs when Link uses up special uses character id to determine y velocity | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:714 | `up_special_aerial_drift_` | 0xDEA38 | 0x80163FF8 | `ftLinkSpecialAirHiProcUpdate`+0x48 (ft/ftchar/ftlink/ftlinkspecialhi.c) | Modifies J Link's aerial drift during special fall | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:741 | `cpu_pull_out_bombs_` | 0xB3444 | 0x80138A04 | `mnPlayersVSPuckProcUpdate`+0x10C (mn/mnplayers/mnplayersvs.c) | Adds Link Variants to a Character ID check and pull out more bombs | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:790 | `boomerang_crash_fix_` | 0xE7DA4 | 0x8016D364 | `wpLinkBoomerangCheckOwnerCatch`+0x8 (wp/wplink/wplinkboomerang.c) | Fixes a crash-creating scenario when the boomerang is destroyed twice by being caught and being outside the blast zone on the same frame. Can happen on scrolling stages like 1-1. For the fix, we'll make wpLinkBoomerangCheckOwnerCatch() retu | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:796 | `boomerang_crash_fix_` | 0xE7E34 | 0x8016D3F4 | `wpLinkBoomerangCheckOwnerCatch`+0x98 (wp/wplink/wplinkboomerang.c) | Fixes a crash-creating scenario when the boomerang is destroyed twice by being caught and being outside the blast zone on the same frame. Can happen on scrolling stages like 1-1. For the fix, we'll make wpLinkBoomerangCheckOwnerCatch() retu | none in scope body (verify) | trivial C edit |
| src/linkshared.asm:811 | `boomerang_crash_fix_` | 0xE7FCC | 0x8016D58C | `wpLinkBoomerangProcUpdate`+0xB0 (wp/wplink/wplinkboomerang.c) | Fixes a crash-creating scenario when the boomerang is destroyed twice by being caught and being outside the blast zone on the same frame. Can happen on scrolling stages like 1-1. For the fix, we'll make wpLinkBoomerangCheckOwnerCatch() retu | none in scope body (verify) | trivial C edit |
