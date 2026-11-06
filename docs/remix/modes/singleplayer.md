# Single Player modes (Remix 1P family)

The Remix extends the 1P menu with a second page (Remix Modes) and adds Bonus 3: Remix 1P, All-Star, Multi-Man, Cruel Multi-Man and the Home-Run Contest (HRC),
plus the Remix versions of Break the Targets / Board the Platforms (Bonus 1/2), the Single Player Enemy option, Practice_1P and the extended records.
Asm source (in `.toolchain/pkgs/smashremix-src/src`): `SinglePlayerModes.asm` (about 8800 lines), `HRC.asm`, `SinglePlayerMenus.asm`, `SinglePlayer.asm`,
`SinglePlayerEnemy.asm`, `Bonus.asm`, `Practice_1P.asm`, `CharacterSelectDebugMenu.asm` (high score switch) and `BGM.asm` (own ending song).
Port code: `decomp/src/sc/sc1pmode/sc1premix*.c` and `sc1pbonusremix.c` (modes, records, enemy), hooks in `sc1pgame.c`, `sc1pmanager.c`, `sc1pintro.c`,
`sc1pbonusstage.c`, `sc1pstageclear.c`, `sc1pgameboss.c`; menus in `decomp/src/mn/mn1pmode/mnremix1pmode.c`, `mn/mnplayers/mnremixbonusmodes.c`,
`mnplayers1pgame.c`, `mnplayers1pbonus.c`, `mnremixenemylabel.c`, `mnremixcsshighscores.c`; `decomp/src/gm/gmremixpractice.c`.
Tests: `tests/remix/test_1p_*.c` (mode names, mode rules, Remix 1P picks, bonus modes, bonus Remix stages, HRC rules / signs / travel, enemy, afk, leftovers) and
`test_practice_rules.c`; `cmake --build build --target run_remix_mode_tests`. Smoke recipes at the end.

## Mode selection

`SinglePlayerModes.singleplayer_mode_flag` holds the mode; the port keeps the asm ids (`sc1premixmodename.h`) so records keep their meaning.

| Id | Mode | Menu |
|---|---|---|
| 0 | standard 1P | page 1 |
| 1 | Bonus 3 | page 1, through the Bonus 2 CSS |
| 2 | Multi-Man | page 2, through the Bonus 2 CSS |
| 3 | Cruel Multi-Man | page 2, through the Bonus 2 CSS |
| 4 | Remix 1P | page 2, 1P CSS |
| 5 | All-Star | page 2, 1P CSS |
| 6 | HRC | page 2, through the Bonus 2 CSS |

Selecting a row of `menu_button_table` / `remix_menu_button_table` writes the mode and `page_flag`; the flag is cleared when the 1P menu is left
(`remove_flag`); `game_over_end` returns the menu to page 1. Multi-Man, Cruel, Bonus 3 and HRC first open the Bonus 2 CSS (`bonus2_css_2_1p`), which on
exit steps the RNG 1 to 64 times by the frame count, rolls a random fighter and costume when the Random token was picked, and starts the 1P game on the
mode's stage. Page 2 has five rows (Remix 1P, All-Star, Multiman, Cruel Multiman, HRC), the cursor wraps at the last row.

The 1P game keeps using the 1P stage table (`SC1PGameStage` rows) and the per-stage CPU table; each mode edits those in place through hooks.

## Remix 1P

`remix_1p_randomization` runs once when START is pressed on the 1P CSS: it clears `metal_bros_flag`, steps the RNG, builds the Kirby pool (7 random powers and
hats plus a fixed Magic Hat Kirby, power 0x1C hat 0x25), runs `duo_select` and fills six random slots of the stage table from `match_pool` (each with a fighter and
one of its three stages that no earlier slot took); the Star Fox line is a random duo and the next to last line one of three bosses (Mad Piano, Metal Mario
Bros., Super Sonic on Green Hill or Casino Night, the Green Hill twice as often). The final stage is Giga Bowser.

Match settings per row: Remix / Fighting Polygon / Tiny Team, Giant (CPU 1 is a giant, `remix_1p_giant`), Metal Bros, Yoshi team (shade rules `remix_team_shade_*`),
Kirby team, doubles. The CPU level is 10 on Very Hard on every stage (`_opponent_cpu` compares the stage flag against an overwritten register, so its
exceptions never match). `_final_stage_spawn` keeps the standard spawn on the final stage. Giga Bowser takes the Master Hand path (`master_hand_`,
`giga_fix_`, `giga_stamina_check_`: defeated at 200 damage on Very Easy and Easy, 250 on Normal and Hard, 300 on Very Hard; `giga_change_action(_2)`: no hurtbox
or armor on defeat, KO sound and the damage pose; `no_laugh_`; `giga_usp_prevent`: Mad Piano, Fox and Falco CPUs do not start the grounded up special). The title
cards were rewritten (`title_card_*`): name text (Marina, Banjo and Crash cut to a short width, only on the Team and Giant cards: `title_card_text_routine` does it in
those two branches and every other stage shows the full name), team and giant texts, voice, model scale, Yoshi card, Metal Bros, tiny team (three tiny opponents,
costume `member index % fighter kind`: 0, 1, 2 for most fighters; the asm divides by the kind and traps on Mario, the port uses costume 0 there), doubles order.
`SinglePlayer.asm` adds the per-fighter name texture, FGM and name delay tables (`get_name_texture_`, `get_fgm_`, `get_name_delay_`), the VS preview fixes
(`singleplayer_vs_preview_fix_`, `_with_allies_fix_`, `duo_team_costume_preview_fix_`, `singleplayer_gdk_polygon_team_costume_preview_fix_`,
`singleplayer_mm_costume_preview_fix_`) and the `fix_port_index_` patches (`desc.player` of title card fighters = the human port). The costume fixes run when a card
fighter is made (`sc1PIntroMakeVSFighter`): costume 1 for the opponent when the human has costume 0 and is Giant DK facing DK or a polygon facing an original polygon,
and in Remix 1P on the Giant stage when the human is the Giant's fighter; the Metal Mario of the Metal Bros card takes costume 1 only when the human is Metal Mario with
costume 0 (otherwise costume 0). The Metal Bros card is laid out as the Mario Bros. duo, so its opponents also use the duo's Z speed. The title card fighters of the human
and allies take the human port (allies: the other of P1 and P2), the opponents the other of P1 and P2, so the Visibility option and the Classic Sonic flag of the right
port apply (`sc1pintrorules.c`).

Records: the 1P score, bonus count and best difficulty per fighter in `Character.REMIX_1P_HIGH_SCORE_TABLE` (and `ALLSTAR_HIGH_SCORE_TABLE`). The run has its own ending
song (`BGM.asm` `remix_1P_ending_song`: `Character.remix_1p_end_bgm.table[fkind]`).

## All-Star

`allstar_randomization` (called from `remix_1p_randomization`): steps the RNG, clears `allstar_hearts`, `allstar_progress`, `allstar_percent`, `allstar_limbo`, `end_game_flag`,
the score and `match_begin_flag`, then fills 30 slots (18 Remix and 12 vanilla fighters; the asm comment says 28) with a random unused fighter and one of its three
unused stages. The stage loop retries until a stage is free and never ends if all three are taken.

- Progress counts fighters: 1 opponent per match below 4 faced, 2 below 0x12, 3 up to 0x1B; `advance_allstar` adds 1, 2 or 3 per win, `progress_advancement` goes to the
  Rest Area (stage 0) or the next stage type (2, 4, 5 and the final 0xC), `end_allstar` forces stage 0xD when `end_game_flag` is set (the Game Clear screen),
  `screen_set_end`: a Practice reset restarts the match, otherwise the 1P menu; `pause_exit` exits to the 1P menu (and clears the size settings in Remix 1P).
- Rest Area (`rest_area_setup`, `rest_area_routine`): three hearts (`allstar_hearts` counts those used, `heart_counter`), `heart_persist` keeps them from fading,
  series icons of the fighters left (file 0xD8B, `icon_offset_table`, ten per row at x 30 + 9n, y 30 + 11 * row, or y 180 with the Dragon King HUD) and world-space
  portraits of the next opponents (400 x 400, y 1000, z -20, alpha 0xA0, x -1000 / -600 / -200); no CPU spawn, no time limit, `skip_go`, the damage meter is shown
  (`display_percent`), the exit is a clipping id (`rest_area_routine`); with the "afk" tag it is left after 120 frames.
- Percent carries between fights (`player_percent`, `record_percent`; `clear_percent` zeroes it at every stock lost by the human, `_ko_save` puts the run in limbo only when the
  human is eliminated so the percent recorded at the end of that match is 0, `clear_percent_continue` on a continue); the Single Player Enemy human (the port after the playing one)
  starts at 0 instead of the carried percent;
  stocks, score and total damage are kept (`stock_prevent`, `prevent_erase`, `damage_prevent`, flagged by `match_begin_flag`); No Miss and Bros. Calamity bonuses are
  not awarded (`no_miss_2`, `no_calamity`).
- The final stage (`_nSC1PGameStageAllstarFinal_`): a team of 24 members, the last three fighters of the order taking turns, costume random of 6 (4 at most for
  vanilla fighters), each member's own stock icon.
- `costume_fix` (Yoshi team costume loop) is dead in All-Star, which routes stage 0xC to the final.

## Multi-Man and Cruel Multi-Man

The Fighting Polygon Team stage turned into an endless horde. Stage rules (`stage_flag`, `_team_attack`, `_item_spawn`, `_time_limit`, `load_player_stocks`, `_opponent_*`,
`_infinite_spawn_1/2`, `_infinite_ko`, `_death_crash`): one stock for the player, no items, no team attack, no time limit; the polygons respawn forever as random
Fighting Polygons (`multiman_random`: `random(12) + 14`) and never end the stage; Multi-Man polygons use the easy knockback ratio and CPU level 4, Cruel the very hard
ratio, CPU level 9 and a raised ratio on the human. The preview timer, button check and preview song are skipped. The HUD (`setup_`) is a polygon stock icon (file 0x17,
tint 0x848484FF) at (30, 20), an "x" at (39, 18) and the KO number at (50, 18) with scale 0.9, left aligned, in the Remix font (y 212 / 214 with the Dragon King HUD on).
`_ko_save` writes the KO count to SRAM when the player's stock is lost, only for Multi-Man and Cruel, when it passes the fighter's record (`MULTIMAN_HIGH_SCORE_TABLE`,
`CRUEL_HIGH_SCORE_TABLE`). `multiman_ko_count_stop` tests `player_ko`, which nothing in the asm ever sets, so the counter never stops. The pause menu shows a reset legend
for these modes (and HRC, and Bonus 3) and the run ends through `_end_match_screen`: only an L reset (`reset_flag`) restarts, A + B + R + Z and death go to the Bonus 2 CSS.
Polygon files and stock icons are removed from the 1P icons (`icon_removal`).

## Bonus 3

The Race to the Finish stage of the 1P game, entered from the Bonus 2 CSS, with Normal or Remix course (`Bonus.mode`, the header toggle on the Bonus CSS; the Remix
course is `REMIX_RTTF`). Three polygons (Remix polygons on the Remix course, `polygon_selection_match_rttf`), CPU level 9 and knockback ratio 9 (Very Hard), team attack on, no
items, no time limit, one stock, no title card, and the Bonus count-up timer (`_load_timer`, `bonus3_timer`, Dragon King HUD: y 210 and 200). The fighter faces left on the
Remix course. A finish saves the time in ticks (`TIME_SAVE_POINTER` is `time_passed`) when the record is empty or strictly faster (`_time_save`, not when the Single Player
Enemy port is on); a beaten record plays New Record (FGM 0x1D0) instead of Complete (0x1CB) (`_new_record`).

## Home-Run Contest (HRC)

The first 1P stage becomes the HRC stage with a Sandbag (CPU type 5) as the only opponent, one stock, no items, blast zones off (`turn_off_blastzones_`), the bat never
despawns (`prevent_item_despawn_`), no BGM volume ramp, "Are You Ready?" (FGM 509, 66 tics) and a 600 tic countdown in the Bonus timer layout (`apply_countdown_`, `_load_timer`).
`HRC.main_` runs per frame: it finds the human (type 0) and the Sandbag (type 5), and while the Sandbag is behind `START_X` (-19392) the camera holds `DEFAULT_CAMERA_X`
(-21312), after which it follows one third of the way each frame; past 12528 the camera warps back 9120 (100 ft) and `warp_distance` grows. Distance in tenths of a foot =
`(sandbag x - START_X + warp_distance) / 9.12`, truncated. The attempt ends after 600 tics once the Sandbag has stopped and the human is not in an exempt action (Lanky
Orangstand, Donkey Kong cargo, the action table); `_complete` animates the counter to the centre over 45 tics and plays FGM 0x1CB, `_failure` counts the same timer. Projectiles
and the human are repositioned relative to the warped camera (`update_projectile_and_hmn_positions_`); the distance signs show 100 / 200 / 300 ft and the fourth sign moves on each
warp; the clouds scroll against the Sandbag (`animate_background_image_`, speed factor -0.001, x scale 2.2334); Big Blue (stage 0x48) scrolls its background in VS the same way.
The counter is the distance with one decimal ("12.3", right aligned at x 278, y 44, scale 0.9) and "FT." (x 282, y 47, scale 0.625), growing to scale 1.35 / 1.075 and moving
to the centre while COMPLETE. The record is `Character.HRC_HIGH_SCORE_TABLE` (tenths of a foot, drawn on the CSS with a period at x 216).

## Music, stage setup and items of the modes (`sc1pgame.c` hooks)

`sc1PGameFuncStart` and the stage setup apply these asm patches (rules in `sc1premixmusicrules.c` and `sc1premixgamerules.c`, glue in `sc1premixmusic.c` and `sc1premixgame.c`, unit tests in `test_1p_game_rules.c`):

- **Music start** (`skip_cflag_music_function`, `skip_cflag_music_function_2`): Multi-Man, Cruel, HRC and the mini boss stage (0xA) of Remix 1P start the stage song with the crowd cheer, whatever the stage kind;
  the prologue stages (Metal Mario, Fighting Polygon Team) of every other run wait for GO as before. `sc1PGameIsBgmDeferredToGo` is the shared test of the start and of the GO hook.
- **Forced songs** (`set_bgm`): written into `gMPCollisionGroundData->bgm_id` before `mpCollisionSetPlayBGM`: Multi-Man 0x7E, or 0xB0 (Multi-Man Melee 2) on a 1-in-2 roll; Cruel 0x7F; HRC 0x8E (Targets!);
  Remix 1P final stage 0x4D; Remix 1P mini boss on Casino Night, Green Hill or Metal Madness: Live and Learn / Open Your Heart / Everything on a 1-in-3 roll; Remix 1P duo stage: Aloof Soldier (Edo), Dangerous Foe (Onett, NPC),
  Pika Cup (Pokemon Stadium 2, Gym Leader Castle, Saffron DL), Wizpig (Spiral Mountain, Mad Monster Mansion, Frosty Village, Windy Valley), Battle Among Friends (Mt. Dedede, Dream Land SR). The rolls are taken only in
  those cases, so the RNG sequence of the other runs is unchanged. Every other stage keeps the header song.
- **Master Hand music** (`master_hand_music_fix_`): with the Random Music toggle on, the Master Hand fight keeps the random track instead of the Final Destination theme (`gMPCollisionBGMDefault` is not set).
- **Spawn direction** (`spawn_human_left_remix_rttf_`): every fighter spawns facing left when the battle stage is the Remix Race to the Finish (the asm comment says only the human, but the patch forces the direction of every spawn).
- **Screen flash** (`battle_style`): alpha 0x80 in Bonus 3, Multi-Man and Cruel (the asm tests "mode is not 0, 4, 5 or 6"); the stage table value otherwise.
- **Enemy trait** (`_opponent_behavior`): Race to the Finish trait in Bonus 3, polygon team trait in Multi-Man and Cruel, standard trait on the All-Star triple battle (the stage row byte there is not a trait), the table value otherwise.
- **Enemy level** (`_opponent_cpu`): 4 in Multi-Man and 9 in Cruel, before the continue level drop; Bonus 3 (9) and Remix 1P (10 on Very Hard) keep their own functions. `ft/ftcomputer.c` needs no change for this.
- **Enemy stocks** (`_infinite_ko`): the KO of an enemy leaves `sSC1PGameEnemyStocksRemaining` alone in Bonus 3, Multi-Man, Cruel and HRC (Multi-Man already returned before it). Killing the three polygons of Bonus 3 no longer ends the run; only the
  finish line does.
- **Items** (`prevent_1p_custom_item_spawns_`, `Item.asm`): right after the stage table item switches are copied, Remix 1P keeps them, All-Star masks them with 0xDFFFFFCF and sets `gITRemixExtendedToggles` to the default (Tomato, Heart and Dango stay), every
  other run (standard 1P, Bonus 3, Multi-Man, Cruel, HRC) masks with 0xFFFFF and clears the extended switches, so no Remix item spawns on the vanilla stages. The extended switches are restored at the next VS character select (`itRemixRestoreExtendedToggles`).
- **Polygon team costume** (`singleplayer_polygon_team_costume_fix_`): the enemies start with costume 1 when the first human plays a default-costume polygon (vanilla polygons, or any Remix fighter id from Wario's polygon up) on Duel Zone to Race to the Finish,
  Battlefield or the Remix Race to the Finish.
- **Bonus count-up timer** (`_load_timer`): `sc1PBonusTimerIsCountUpRun` (Bonus 3, HRC, Practice) picks `sc1PBonusStageMakeCountUpTimer` in `sc1PGameInitTimeUpMessage`.
- **Rest Area damage meter** (`display_percent`): `sc1PGameWaitStageBonus3Update` shows the damage meters for the Rest Area too, only the GO object and voice are skipped.
- **All-Star final** (`_SC1PGameEnemyTeamSprites_expansion`, `_stock_loading_allstar`, `_allstar_final_series_stock_set`, `end_allstar`, `no_miss_2`, `no_calamity`, `clear_percent`): the enemy stock row shows the own stock icon and costume palette of each of the 24 members
  (`sc1PRemixAllstarFinalIconsSetStock`; the three fighters come from the enemies made at the start, `sc1premixallstarfinal.c`), and when a member spawns the series logo and stock icon of the CPU damage panel of its slot are replaced
  (`sc1PRemixAllstarFinalIconsOnSpawn`, found through the display procs of the interface objects). A won final turns the 1P stage into 0xD before the bonus stats (`sc1PRemixGameOnStageEnd`), so the stage clear is the Game Clear one with its bonuses and music.
  All-Star does not award No Damage (the asm skips it with No Miss), and Bros. Calamity is only awarded in the standard run (the asm clears it for every nonzero mode id). The limbo of the kept percent is set when the human is eliminated
  (`sc1PRemixAllstarOnPlayerEliminated` from `sc1PGameSetPlayerDefeatStats`), not at every stock lost.

Deviations: the Game Clear stage swap is only done when the final was won (the asm also does it on a loss and repairs the stage in `progress_advancement`; the port retries the stage it has, so a forced 0xD would replay the Master Hand fight).
Alternate and Random Music are still applied by `mpCollisionInitGroundData` in the VS battle only (ledger MS-3); the Remix 1P duo songs above are forced after it, the Master Hand guard only has an effect once MS-3 covers the 1P game.

## Bonus 1/2 on the Remix

`Bonus.asm` adds a Normal / Remix header toggle to the Bonus 1, 2 and 3 CSS (`handle_header_toggle_`), with the title "Option", the `MODE:` row (NORMAL `0xE4BE41`, REMIX `0xE44141`),
blinking arrows and, for Bonus 1/2 on Remix, a STAGE row: 33 stages (`btt_stage_table`, `btp_stage_table`, Remix fighters included) shown with one or two stock icons (file 0xD8B),
and "L: Set Stage" while a token is held. Remix selection disables the automatic start (`manual_remix_bonus_start_`, 140 tics otherwise). `set_btt_stage_id_` / `set_btp_stage_id_` pick the stage:
the fighter's own (`Character.BTT_TABLE`, `BTP_TABLE`) for added fighters, the STAGE row for a Remix choice, the vanilla stage otherwise; `bonus_fix_1_..4_` supply the targets / platforms
of the added stages (`Stages.bonus_pointer_table`). Records for added fighters and Remix stages are `Bonus.REMIX_BONUS_HIGH_SCORE_TABLE` (33 stages x fighters x targets / platforms), and the
Bonus 2 CSS shows the per-mode records (KOs `0x34E8` or feet `0x3C08` label, 5 or 7 digits, a total line at x 235 / 225, y 205, header images `0x1DD8` Bonus 3, `0x14D8` Multi-Man,
`0x1958` Cruel, `0x24C8` HRC). In the 1P run itself the `_1p` variants of the stage id patches ignore `Bonus.mode` and use the default tables.

### Bonus bookend and menu hooks

`CharacterSelect.asm` `draw_portraits_` draws the bonus bookend on every CSS but the 12CB one; on the Bonus screens Z or R held scrolls the ten bonus fighters
(`bonus_char_index_bonus`, `check_variant_input_`, `draw_bonus_scroll_indicators_`), a blinking hint (arrows and Z / R) sits over the panel card, and portraits and bookends slide in at the start. The
Random bookend gives the Random token (`save_random_char_._bonus`, `load_random_1p_`). `SinglePlayerMenus.asm` adds the headers (`header_graphic`; `header_graphic_remix_1p` uses file-5 offset
0x3758 for Remix 1P and 0x39C8 for All-Star instead of the vanilla 0x228), alternative announcer voices (`_1P_FGM_alter`: FGM 1014 on the All-Star CSS; `_BTP_FGM_alter` on the Bonus 2 CSS: Bonus 3 0x1EF,
Multi-Man 0x3C7, Cruel 0x3C8, HRC 1015), the header click swap (`set_original_flag_swap`: Bonus 2, Bonus 3, Bonus 1; Multi-Man and Cruel toggle; nothing in HRC) and an unplugged-controller B handler.

## Single Player Enemy and "afk"

`SinglePlayerEnemy.asm`: on the 1P CSS, Z of another port makes that port play the first enemy of the 1P game instead of the CPU (laugh FGM 0x1ED the first time), B of another port takes it
back. `onep_control_port` hands the first-enemy slot (spawn 0x25, not in HRC) to that port with its colour and tag, `onep_control_respawn` and `onep_control_shield` extend that to team respawn and shield
colour. The CSS shows "Enemy Player" and "Press Z" labels (blinking 0x50 period, visible below 0x29). With the custom player tag number 2 named "afk" the 1P game plays itself:
the playing slot becomes a level 10 CPU (spawn 0x21), the stage clear, preview and continue screens advance by themselves, the bonus time-up and the Race to the Finish take their finish branch, and the
credits slow down at the first Remix name.

## High scores and Practice_1P

- `SinglePlayer.high_score_enabled` is switched by `CharacterSelectDebugMenu.asm` `check_high_scores_enabled` on the 1P, Bonus and All-Star select: it needs the Gameplay toggle word
  `& 0xFFEFFF7F` to be 0 (Hitstun, Hitlag, DI, Japanese sounds / stun / sleep, Momentum, Shield Stun, Z-Cancel, Punish defaults; Improved AI is bit 20 and bit 7 is Japanese Sounds
  value 2), Gameplay word 2 to be 0, the Pokemon word `& 0xFFFC` to be `0x7FFC` and no enemy port; otherwise "High Scores Disabled" is drawn (x 192, y 217, scale 0.8125, `0xFF8080FF`) and no
  record is written.
- Practice_1P (panel option): the 1P game starts on the chosen stage (values 1..11 map to Link, Yoshi, Fox, Mario, Pikachu, DK, Kirby, Samus, Metal Mario, Polygon team, Boss, with the Bonus
  stages skipped); L held on any controller at stage clear, or L in the pause menu, retries the stage without a continue, a level drop or the stage preview. Practice runs take the Bonus timer branch.

## How the port does it

| Asm | Port |
|---|---|
| mode id, `remove_flag`, page flag | `gSC1PRemixMode` / `sc1PRemixModeGet` (menu choice, or `SSB64_REMIX_1P_MODE`) in `sc1premixmode.c`; `gMNRemix1PModePage` and the rows in `mnremix1pmode.c` (hooks in `mn1pmode.c`, `mn1pcontinue.c` for `game_over_end`) |
| `bonus2_css_2_1p` | `sc1PRemixModeExitBonusCSS` (`sc1premixbonusexit.c`; rules `sc1PRemixModeLeavesBonusCSSFor1P`, `sc1PRemixModeSeedAdvanceCount`), called from `mnplayers1pbonus.c`; Random roll `mnRemixBonusModesResolveRandom` |
| `remix_1p_randomization` | `sc1PRemix1PRandomize` (`sc1premix1p.c`, picks in `sc1premix1prules.c`: fighter, stage, distinct Kirby powers, boss, Sonic stage), called from `mnplayers1pgame.c` at START; the path is read through `sc1PRemix1PGetStage`, `GetComputer`, `GetCard`; stage rows from the Remix image (`sc1premix1pdata.c`) |
| Remix 1P matches | hooks in `sc1PGameSetupStageAll` (`sc1pgame.c`): polygon / team / giant / Yoshi / Kirby / Metal Bros rows, `sc1PRemix1PSetupEnemy`, `sc1PRemix1PEnemyLevel` (level 10 on Very Hard) |
| title cards | `sc1pintro.c` (name text, team and giant text, Yoshi and tiny cards, doubles, Metal Bros, costume and port fixes) with `sc1premixintro.c` for the per-fighter tables and `sc1pintrorules.c` for the state-free decisions (test `test_1p_intro_rules.c`) |
| Giga Bowser | `sc1PRemix1PIsFinalBoss`, `sc1PRemix1PBossHitPoints` (`SSB64_REMIX_1P_BOSS_HP` overrides), `ftbosscommon.c`, `ftmain.c` boss branch, `sc1PRemix1PBlocksCpuUpSpecial` in `ftcommonspecialhi.c`, `sc1PGameBossSetBossPlayer`, `sc1PRemix1POnPauseExit`, the "H.P." HUD (`ifCommonPlayerDamageUpdateDigits` in `ifcommon.c`: max H.P. minus damage, 200 / 250 / 300 by difficulty for Giga Bowser, 300 for Master Hand) |
| `allstar_randomization` and progress | `sc1PRemixAllstarRandomize/StepStage/GetStage/GetComputer` (`sc1premixallstar.c`; `sc1PRemixAllstarPickOrder` in `sc1premix1prules.c` sends the slot to another fighter instead of looping when a fighter's three stages are taken); progress thresholds `SC1PREMIX_ALLSTAR_*_PROGRESS` (`SSB64_ALLSTAR_PROGRESS` sets the start); hooks in `sc1pmanager.c` |
| Rest Area | `sc1PRemixAllstarSetupRestArea`, `OnHeartUsed`, `Update` (exit clipping), icons `sc1premixallstarrest.c` (rules `sc1premixallstarrestrules.c`), portraits `sc1premixallstarrestportraits.c` (display proc on link 6, RGBA16 32 x 32 from file 0xA05); hearts counted in `ftcommonget.c` |
| percent carry, limbo, final | `sc1PRemixAllstarPlayerDamage/RecordPercent/OnPlayerKo/OnContinue`, `sc1PRemixAllstarKeepsRunState`, `sc1PRemixAllstarPrepareFinal/FinalFkind/FinalCostume` |
| Multi-Man, Cruel | `sc1premixmultiman.c` (`SetupBattle/SetupHud/SetupPlayer/SetupEnemy/PolygonKind/OnEnemyKo/OnPlayerKo/SaveRecord/LeaveRun/DrawHud`), pure rules in `sc1premixmoderules.c`; HUD sprites through `ifRemixSpriteMake` + `remix_hud_text_putf` slot 28 |
| Bonus 3 | `sc1premixbonus3.c` (`SetupBattle`, `SetCourse`, `OnComplete`, `SaveRecord`; rules in `sc1premixbonus3rules.c`), the count-up timer is `sc1PBonusStageMakeCountUpTimer` (also the HRC timer, which counts down 600 tics in `sc1PBonusStageTimerProcUpdate` and holds at 0, and the Practice timer; `sc1PBonusTimerIsCountUpRun` in `sc1pbonustimer.c` tells which runs use it, `sc1PGameInitTimeUpMessage` asks it), finish hook in `grbonus3.c` (`grBonus3FinishProcUpdate`), `SSB64_REMIX_BONUS3_COURSE=normal|remix` forces the course |
| HRC | `sc1premixhrc.c` (stage, Sandbag, stocks, Are You Ready, end of run), `sc1premixhrcrun.c` (per-frame, counter, `sc1PRemixHrcCounterAt`), `sc1premixhrctravel.c`, `sc1premixhrcsigns.c`, `sc1premixhrcclouds.c`, `sc1premixhrcrecord.c`; rules in `sc1premixhrc*rules.c`; blast zones in `ftmain.c`, bat in `itmap.c` (`sc1PRemixHrcKeepsItem`), countdown in `ifcommon.c` |
| `_end_match_screen`, `_reset`, `_reset_button`, `hold_l_to_retry_` (fake screen 0x77), `bonus2` exits | The L retry of Bonus 3, Multi-Man, Cruel and HRC lives in `sc1premixretry.c` (rules in `sc1premixretryrules.c`): L in the pause menu (`sc1PRemixRetryTryByPause`) or L held on any controller while GAME SET shows (`sc1PRemixRetryTryByGameSet`, also for Bonus 1/2 practice runs) sets the retry flag (`reset_flag`) and ends the scene from `ifcommon.c`. `sc1PRemixModeLeaveRun` (`sc1premixbonusexit.c`) restarts the run only when it consumes that flag; A + B + R + Z and a KO return to the Bonus 2 CSS, as `_end_match_screen` does. The pause legend (`ifCommonBattlePauseMakeSObjsAll`) shows "L: RETRY" in these four modes, HRC included. In a Bonus 1/2 practice run `sc1PBonusStageStartScene` consumes the flag and keeps the Bonus stage and its CSS as the scenes (`hold_l_to_retry_bonus_1/_2`), skipping the unlock checks |
| Bonus Remix | `sc1pbonusremix.c` (`sc1PBonusRemixGetBattleGkind`, tables read from the Remix image, class table for platforms, `TrySave`), `sc1pbonusstage.c` hooks (`sc1PBonusStageIsNewRecord`: the New Record voice compares with the Remix table record of the stage played when it is not the fighter's default stage, the record must be complete and beaten; `sc1PBonusStageGetGkind1PGame`: the Bonus stages inside the 1P game ignore `Bonus.mode` and the Remix 1P table, rules in `sc1pbonusstagerules.c`), CSS `mnremixbonusmodes.c` (Option panel, MODE row, STAGE row, "L: Set Stage", manual start, records, total, headers; rules `mnremixbonusmodesrules.c`) |
| Bonus bookend, Random token | `mnRemixCssBonusMakeBookend/GetFighterKind/Scroll/UpdateHint` and `mnRemixCssRandomMakeBookend` (`mnremixcssbonus.c`, `mnremixcssrandom.c`) through `mnRemixBonusModesUpdateScroll/UpdateHint`; slide-in by `mnRemixCssSlideAttach` on the 30 grid portraits (`mnPlayers1PBonusMakeRemixPortrait`); the puck returns `REMIX_ID_RANDOM` over the bookend |
| records and switch | `LBBackup1PRecord` fields `hrc_distance`, `multiman_kos`, `cruel_kos`, `bonus3_time`, `bonus3_remix_time`, `remix_1p`, `allstar` and `LBBackupData.remix_bonus` (`lbtypes.h`; `GMCOMMON_FIGHTERS_SAVE_NUM` is 97, one row per Remix fighter); `sc1premixscore.c`, `sc1premixhighscores.c` (writers call `sc1PRemixHighScoresCanSaveRecord` first), `mnremixcsspanelmenurules.c` (`mnRemixPanelMenuRulesHighScoresEnabled`) with `remix_toggle_group_snapshot` (`port/remix/remix_toggles.cpp`) |
| Single Player Enemy, afk | `sc1premixenemy.c` (port state, hooks in `sc1pgame.c`, `efmanager.c` shield colour), `sc1premixenemyrules.c`, `sc1premixenemyafk.c` (afk tag 2 = `SSB64_REMIX_TAG_NAMES="2=afk"`, hooks in `sc1pstageclear.c`, `sc1pintro.c`, `mn1pcontinue.c`, `sc1pbonusstage.c`, `grbonus3.c`), CSS labels `mnremixenemylabel.c`, Z / B in `mnplayers1pgame.c` |
| Practice_1P | `gmRemixPracticeStartStage/Begin/TryRetryByHold/TryRetryByPause/ConsumeRetry` (`gmremixpractice.c`), hooks in `sc1pmanager.c`, `sc1pstageclear.c`, `ifcommon.c`; `prevent_masterhand_softlock_`: `sc1PGameBossSetBossPlayer` takes a slot as Master Hand only with player kind CPU and fighter kind Boss (`sc1PRemixSlotIsCpuMasterHand`, `sc1premixmoderules.c`), so a stale slot of another port type left in `gSCManager1PGameBattleState` by a practice retry is never the boss |
| Rest Area hearts | `heart_persist`: `itRemixHeartIsDespawnBlocked` (`itremixheartpersist.c`) stops `itProcessProcItemMain` from counting the pickup wait of a heart down while `sc1PRemixAllstarIsRestArea`, so the hearts never flash or vanish |
| env mapping | `env_mapping_fix_`: `sysRemixEnvMapAppendLookAt` (`sysremixenvmap.c`, screen rule `sysRemixEnvMapIsLookAtScreen` in `sysremixenvmaprules.c`) appends the two look-at commands (`custom_lighting_1/2`, byte for byte) at the end of the camera display list setup (`func_8001663C`, `sys/objdisplay.c`) on the 1P VS preview, the CSS screens 0x10 to 0x14, the VS results, the ending room and the Continue screen, so Metal Mario and the polygons get environment mapping; a camera with its own look-at matrix replaces them later in the list. The asm tests the first loaded file (0xB, 0x4F); the port tests the scene only, since its Continue screen is a scene of its own |
| own ending song | `sc1PRemixEndingGetCreditsSong` (`sc1premixending.c`) from `scStaffrollFuncStart` |

Save format: the Remix record fields extend `LBBackup1PRecord` / `LBBackupData` under `PORT && REGION_REMIX`; the checksum covers the whole struct, so a save from before a field was added is reset once to the
defaults. Vanilla builds are untouched.

### Deliberate deviations from the asm

- **All-Star stage loop.** The asm retries the stage forever when all three stages of a fighter are taken; the port sends the slot to another fighter and, if none has a free stage, to a random one on its first stage (the only place
  a stage repeats).
- **Multi-Man KO counter.** As in the asm, the counter never stops: `player_ko` is never written as 1, so `multiman_ko_count_stop` always saves the new count. `SC1PRemixMultimanKos.is_player_ko` only records the player KO for the log. The register mix-up in its Cruel compare is moot and not reproduced.
- **CPU level in Remix 1P.** Level 10 on Very Hard on every stage, as the asm effectively does (its stage exceptions never match).
- **Costume fix in All-Star.** `costume_fix` is dead in the asm (it hooks the Yoshi row); its effect (at most four costumes for vanilla fighters of the final team) is `sc1PRemixAllstarPrepareFinal`.
- **Bonus 3 course.** `SSB64_REMIX_BONUS3_COURSE` forces the course in tests; the CSS toggle sets it in play.
- **Records.** Records live in the port's save structures with their own fields rather than the Remix SRAM layout.
- **Option bar.** The Option panel bar is a HUD rectangle (slot 46), drawn over the game sprites; the asm draws it under the outline and the title.
- **Rest Area.** The port uses the race wait for the Rest Area and enters `sc1PManagerUpdateScene` once per run, so `match_begin_flag` has no visible effect (`sc1PRemixAllstarKeepsRunState` keeps the asm
  rules for a re-entry).
- **Total label.** The "TOTAL" label keeps y 209 (the asm stores a register whose value is not traced); the number is at y 205 as in the asm.

Audit ledger (every item closed): `docs/remix/patch_audit.md`, group g1.

## Tests and smoke recipes

Run from the repository root with the scripts of `.claude/skills/remix-smoke-tests/scripts/`; scene ids: 17 = 1P CSS, 18 = Training CSS, 19 = Bonus 1 CSS, 20 = Bonus 2 CSS, 52 = 1P game.

- Boot a mode: `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=<hrc|multiman|cruel|remix1p|allstar|bonus3> SSB64_SPGAME_FKIND=<fkind>`; Multi-Man and Cruel also need `SSB64_SPGAME_STAGE=12` (the
  Polygon Team stage). `SSB64_SPGAME_STAGE=13` boots the final stage (Giga Bowser), `SSB64_REMIX_1P_BOSS_HP=<n>` changes its HP and `SSB64_REMIX_1P_BOSS_PRESET_DAMAGE=<n>` starts it with n percent (the backdrop
  steps at 90 and 180 damage and the stage ends after the defeat); `SSB64_SPGAME_AUTOWIN=<tics>` wins a stage that many tics after the start; `SSB64_ALLSTAR_PROGRESS=<n>` starts All-Star at that progress;
  `SSB64_REMIX_BONUS3_COURSE=normal|remix` forces the Bonus 3 course; `SSB64_REMIX_TOGGLES="Hitstun=1"` / `SSB64_REMIX_PROFILE` set toggles; `SSB64_REMIX_TAG_NAMES="2=afk"` turns the afk tag on.
- Bonus 2 CSS: `SSB64_START_SCENE=20 SSB64_REMIX_1P_MODE=multiman` shows the mode title, KOs label (HRC: feet with a period), the total line at the lower right, the Option row for Bonus 3 (`MODE: NORMAL`, click the
  right arrow at x 280, y 157: REMIX in red). With a Random token on the left bookend (x 24..39, y 44..116) START rolls a fighter. Records must be seeded first (play Multi-Man and die, or run `hrc`).
- High scores: scene 20 with `SSB64_REMIX_TOGGLES="Hitstun=1"` shows the red "High Scores Disabled" text; `Pokemon SFX=2` and `Improved AI=0` do not; no `bonus3: record saved` line appears in the log while it is shown.
- Records for every fighter: `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=remix1p SSB64_SPGAME_FKIND=<a Remix fkind such as 0x3F>`, finish or lose with a score, then `SSB64_START_SCENE=17 SSB64_REMIX_1P_MODE=remix1p`
  shows the Remix 1P score while the unmoded 1P CSS still shows the standard one. The first run after a build that grows the save resets it once (checksum).
- Bonus with a Remix fighter: `SSB64_START_SCENE=19` / `20`, token on a Remix fighter, START: the log shows the fighter's own `gkind`; Remix choice + STAGE row + "L: Set Stage".
- All-Star Rest Area: `SSB64_START_SCENE=52 SSB64_REMIX_1P_MODE=allstar SSB64_SPGAME_FKIND=<fkind>`, frame ~120 of the first Rest Area: one translucent portrait at x -200; two from progress 4, three from 0x12; the log
  has `allstar: rest portrait skipped` if the sprite is not RGBA16 32 x 32.
- Regression: Bonus 1/2 with vanilla fighters still start the bonus stage; the Bonus screen without a mode shows no Option row.
