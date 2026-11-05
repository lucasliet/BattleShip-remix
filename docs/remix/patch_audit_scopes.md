# Smash Remix patch audit: per-scope map

One line per Remix scope (or table): asm location, verdict and the port location that implements it, or the reason for NA. The open
items are in [`patch_audit.md`](patch_audit.md), with the same group prefixes. Produced on 2026-11-05 by
`tools/audit_remix_patches.py` plus a manual read of every scope; line numbers are those of that date.

## OP: 1P, Bonus, All-Star, Remix 1P

Format: asm file:line | scope | verdict | evidence.

##### Practice_1P.asm
- Practice_1P.asm:22 | `_1p_practice` | PORTED | gm/gmremixpractice.c:51-75 (stage map 1..11 -> Link..Boss matches asm 41-60 with Bonus stages skipped), mnplayers1pgame.c:3689; `_reset` FGM-count fix NA (N64 FGM table)
- Practice_1P.asm:90 | `prevent_masterhand_softlock_` (7 patches) | GAP | sc1pgameboss.c:1039-1052 tests fkind only
- Practice_1P.asm:123 | `hold_l_to_retry_` | GAP (partial) | practice path ifcommon.c:3596-3608; Bonus/Remix-mode branches missing
- Practice_1P.asm:220 | `hold_l_to_retry_stage_clear_` | PORTED | sc1pstageclear.c:2176, sc1pmanager.c:327-345, 771-775 (stage not advanced, score/continues/bonus cleared, stocks restored)
- Practice_1P.asm:292 | `hold_l_to_retry_bonus_1` | GAP | depends on GAME SET L retry
- Practice_1P.asm:314 | `hold_l_to_retry_bonus_2` | GAP | same

##### TimedStock.asm
- TimedStock.asm:20 | FFA save mode 2 -> 3 | PORTED | mnvsmode.c:1217-1222
- TimedStock.asm:26 | Teams save mode 2 -> 3 | PORTED | mnvsmode.c:1228-1233
- TimedStock.asm:33 | CSS stock icon test (== time, not bit) | ADAPTED | mnplayersvs.c:6676-6680 (`is_single_stockicon` on `== TIME`)
- TimedStock.asm:41 | results scoring check 2 -> 3 | ADAPTED | mnvsresults.c:2936-2941
- TimedStock.asm:59 | `score_fix_` | PORTED | mnvsresults.c:2749-2764, gmremixtimedstock.c:34-52, gmremixtimedstockrules.c:9-21 (stocks left << 12 minus damage; Tug branch)
- TimedStock.asm:151 | `pick_scoring_` | PORTED | gmremixtimedstock.c:25-31, mnvsresults.c:2936
- TimedStock.asm:174 | `prevent_sudden_death_` | PORTED | scvsbattle.c:413-432, gmremixtimedstock.c:54-60, gmremixsmashketball.c:197-227
- TimedStock.asm:215 | `show_full_results_` | ADAPTED | gmremixgameend.c:~160; wait 370 vs 410 deviation

##### HRC.asm
- HRC.asm:565 | `spawn_visible_` | ADAPTED | `is_skip_entry` = TRUE for HRC (sc1pgame.c:1141, 1324) -> fighter starts in wait (ftmanager.c:1282)
- HRC.asm:583 | `apply_countdown_` | ADAPTED | ifcommon.c:2829-2870, 2914 (600-tic countdown, announce under 5 s)
- HRC.asm:616 | `play_are_you_ready_` | PORTED | sc1premixhrc.c:78-83 (FGM 509, 66 tics), sc1pgame.c:1974
- HRC.asm:642 | `turn_off_blastzones_` | PORTED | ftmain.c:2097-2103
- HRC.asm:666 | `prevent_item_despawn_` | PORTED | itmap.c:342 (`sc1PRemixHrcKeepsItem`); Smashketball half ADAPTED: the basketball never calls `itMapCheckDestroyDropped` (itremixbasketball.c:962)
- HRC.asm:706 | `prevent_volume_reduction_` | PORTED | ifcommon.c:2865 HRC branch skips the BGM volume ramp
- HRC.asm:730 | `animate_background_image_` | GAP (Big Blue); HRC half PORTED | sc1premixhrcclouds.c, grwallpaper.c:156; no `nGRRemixStage_BLUE` wallpaper code

##### Bonus.asm
- Bonus.asm:228 | `handle_header_toggle_` | ADAPTED | mnremixbonusmodes.c:401-412 (`mnRemixBonusModesUpdateStageRow`)
- Bonus.asm:414 | `manual_remix_bonus_start_` | PORTED | mnplayers1pbonus.c:3222-3228

##### SinglePlayerEnemy.asm
- :28 | `onep_control_port` | PORTED | sc1premixenemy.c:73-97, sc1pgame.c:2794, 2840
- :78,82,86,90,94 | `_afk_1p` | PORTED | sc1premixenemyafk.c, sc1pstageclear.c:2155, sc1pintro.c:2519, mn1pcontinue.c:1109, sc1pbonusstage.c:1123, grbonus3.c:113, tag 2 = string_table[2]
- :203 | `onep_control_respawn` | PORTED | sc1pgame.c:1798
- :235 | `onep_control_shield` | PORTED | efmanager.c:4286, sc1premixenemy.c:111-121
- :442 | `z_button` | PORTED | mnplayers1pgame.c:3784, sc1premixenemyrules.c:12-24 (laugh FGM 0x1ED, high-score switch)
- :490 | `b_button` | PORTED | mnplayers1pgame.c:2987, rules `IsClearedByB` (button_tap)
- (labels `setup_`, `enemy_routine_`) | not patch rows | PORTED | mnremixenemylabel.c

##### SinglePlayerMenus.asm
- :16 | `remove_flag` | ADAPTED | mnRemix1PModeSelect sets mode and clears enemy (mnremix1pmode.c:152-166)
- :38 | `stop_restart` (2 patches) | ADAPTED | mn1pmode.c:1026-1036 (no restart when prev is ModeSelect or 1PMode)
- :69 | button creation overwrite | ADAPTED | mnremix1pmode.c:23-100 (rows, positions and text offsets match)
- :212 | `render_menu_buttons_` | ADAPTED | mn1pmode.c:699-706
- :253,257,263 | `fix_cursor_wrap_` | ADAPTED | mn1pmode.c:709-731 (last option 5 / 4)
- :291 | `set_cursor_index_` | ADAPTED | mnremix1pmode.c:100-138 (verified: the BSS option is 0 on a reload)
- :342-400 | `page_2_exit` (7 patches) | ADAPTED | mnRemix1PModeBack; array-index patches are N64 button-array extension, NA by design
- :387 | cursor highlight by type | ADAPTED | mnremix1pmode.c:61-74
- :444,447 | `fix_a_and_b_button_crashes_` | NA | asm crash from the overwritten click handler; the port code returns after the load scene
- :461 | `_ko_amount_or_feet` | ADAPTED | mnremixbonusmodes.c `MakeRecord`, rules (5 / 7 digits)
- :509 | `load_bonus3_time` | ADAPTED | mnplayers1pbonus.c:1098-1111
- :540 | `_timedisplay_skip` | ADAPTED | mnremixbonusmodesrules.c:63-78
- :573,578 | `total_kos_or_feet_` | ADAPTED | mnremixbonusmodes.c:156-200 (x 235 / 225, y 205, filler 0xAAAAAAAA, 6 / 8 digits)
- :682,687 | `draw_hrc_high_score_decimal_` | ADAPTED | mnremixbonusmodesrules.h (shift 4, period at x 216, +5)
- :742 | `counter_graphic` | ADAPTED | rules 0x34E8 / 0x3C08
- :767 | `header_graphic` | ADAPTED | mnplayers1pbonus.c:1035-1037 (0x1DD8, 0x14D8, 0x1958, 0x24C8)
- :808 | `header_graphic_remix_1p` | GAP | mnplayers1pgame.c:1517
- :837 | `set_original_flag` | ADAPTED | the port reads the mode directly
- :874 | `set_original_flag_swap` | GAP | mnplayers1pbonus.c:2471
- :946 | `_BTP_FGM_alter` | GAP | mnplayers1pbonus.c:1051-1055
- :982 | `_1P_FGM_alter` | GAP | mnplayers1pgame.c ~4005
- :1006,1011,1016 | `_1P_unplugged_b_handler` | GAP | no port code

##### SinglePlayer.asm
- :22 | `extend_high_score_btx_count_check_` | ADAPTED | mnplayers1pbonus.c:1340-1360 (Remix table / save rows)
- :103 | `extend_high_score_btx_count_` | ADAPTED | mnplayers1pbonus.c:1268-1285
- :175 | `extend_high_score_btx_time_` | ADAPTED | mnplayers1pbonus.c:1098-1111
- :249,340,550,642 | `extend_high_score_btt/btp_count/time_write_` | PORTED | sc1pbonusstage.c:1378-1440 (`sc1PRemixHighScoresCanSaveRecord`, `sc1PBonusRemixTrySave`, sc1pbonusremix.c:179-210)
- :459,762 | `..._new_record_check_` | STUB | see section 2
- :854-884 | `extend_btx_tally_` | GAP | see section 2
- :1035,1086 | `set_btt_stage_id_`, `set_btp_stage_id_` | ADAPTED | sc1pbonusremix.c:78-103 (practice path)
- :1136 | `extend_high_score_1p_` | ADAPTED | sc1premixscore.c:35-55
- :1194 | `extend_high_score_1p_bonus_count_` | ADAPTED | same
- :1262 | `extend_high_score_1p_difficulty_` | ADAPTED | same
- :1300 | `extend_high_score_1p_write_` | ADAPTED | sc1pmanager.c:278-290, sc1premixscore.c:57-82
- :1372 | `extend_high_score_1p_write_difficulty` | NA | register repair after the table base patch
- :1393 | `extend_1p_tally_` | GAP | see section 2
- :1401,1450 | `set_btt_stage_id_1p_`, `set_btp_stage_id_1p_` | STUB | see section 2
- :1496,1535 | `write/load_extended_high_score_table_` | ADAPTED | LBBackup extension (lbtypes.h `remix_1p`, `allstar`, `remix_bonus`)
- :1768 | `get_name_texture_` (2 patches) | PORTED | sc1premixintro.c:30-62, sc1pintro.c:627, 820-822, 874
- :1834 | `get_fgm_` | PORTED | sc1premixintro.c:64-71, sc1pintro.c:2428
- :1960 | `get_name_delay_` | PORTED | sc1premixintro.c:73-82, sc1pintro.c:2350, 2415
- :1979 | `singleplayer_vs_preview_fix_` | PORTED | sc1pintro.c:2113
- :2001 | `singleplayer_vs_preview_with_allies_fix_` (2) | PORTED | same
- :2053 | `singleplayer_gdk_polygon_team_costume_preview_fix_` | GAP
- :2133 | `singleplayer_mm_costume_preview_fix_` | GAP
- :2179 | `singleplayer_polygon_team_costume_fix_` | GAP
- :2240 | `duo_team_costume_preview_fix_` (4) | PORTED | sc1pintro.c:265-277, 1707-1731
- :2383-2420 | `replace_victory_image_` (6) | ADAPTED | mncongra.c:288-310, mnremixgalleryrules.c:58+
- :2551 | `env_mapping_fix_` | GAP (low confidence)
- :2633 | `fix_port_index_` (4) | GAP

##### SinglePlayerModes.asm, lines < 3600 (auditor A)
- 131 `bonus2_css_2_1p` ADAPTED (sc1premixbonusexit.c:29-51; random roll mnplayers1pbonus.c:3146)
- 278 `opponent_type` ADAPTED (sc1PGameSetupStageAll cases, sc1pgame.c ~1469, ~1581)
- 342 `battle_style` STUB
- 376 `_1p_stage` PORTED (sc1premixbonus3.c:71-74, sc1premixhrc.c:42)
- 431 `_team_attack` PORTED (multiman.c:65, bonus3.c:75)
- 472 `_item_spawn` PORTED (multiman.c:66, bonus3.c:76, hrc.c:43)
- 501 `_hmn_knockback` PORTED (multiman.c:95-100)
- 523 `_opponent_knockback` PORTED (multiman.c:110, bonus3.c:109)
- 560 `_opponent_number_1` ADAPTED (sc1pgame.c:1700-1703)
- 598 `_opponent_number_2` ADAPTED (sc1premixallstar.c ~100)
- 638 `_opponent_2` PORTED (sc1premixhrc.c:63)
- 672 `_time_limit` PORTED (sc1pgame.c:1216, multiman.c:67, bonus3.c:77, ifcommon.c:2829)
- 735 `_percent_fix` NA (Multi-Man returns before the bonus stats store, sc1pgame.c:2362; asm fixes an out-of-bounds index)
- 765 `_spawning_style` PORTED (sc1pgame.c ~2085-2113)
- 846 `spawn_human_left_remix_rttf_` GAP
- 865 `_initial_screen_set` ADAPTED (sc1pmanager.c:588-594)
- 892 `_reset_button` STUB
- 931 `_reset` GAP
- 990 `_load_timer` STUB
- 1173 `_end_match_screen` STUB
- 1211 `_ko_save` PORTED (multiman.c:145-168, ftcommondead.c:124)
- 1301 `_time_save` PORTED (bonus3.c:112-159, hrcrecord.c:23-55)
- 1399 `_new_record` PORTED (bonus3.c:176, hrcrun.c:164-168)
- 1443 `_opponent_behavior` STUB
- 1490 `_infinite_spawn_1` ADAPTED (sc1pgame.c:1703)
- 1519 `_death_crash` NA (low confidence; same crash avoidance as `crash_prevent`)
- 1548 `_infinite_spawn_2` ADAPTED
- 1576 `_infinite_ko` STUB
- 1606 `_preview_skip` ADAPTED (sc1pmanager.c:589)
- 1655 `_preview_song_skip` ADAPTED
- 1694 `skip_cflag_music_function` GAP
- 1731 `set_bgm` GAP
- 1877 `skip_cflag_music_function_2` GAP
- 1914 `load_player_stocks` PORTED (multiman.c:101, bonus3.c:95, hrc.c:54)
- 1960 `load_1p_difficulty_1`, 2001 `load_1p_difficulty_2` NA (every consumer of the overridden difficulty is replaced by `_opponent_cpu` / `_opponent_knockback` constants; `saved_difficulty` has no reader)
- 2033 `_opponent_cpu` STUB
- 2087 `crash_prevent` NA (same as `_percent_fix`)
- 2123 `stage_flag` ADAPTED (sc1premixmultiman.c:39-46)
- 2156 `icon_removal` PORTED (sc1pgame.c ~2223)
- 2187 `multiman_random` PORTED (multiman.c:113-116)
- 2225 `multiman_ko_count_stop` STUB
- 2273 `player_ko_refresh` PORTED (multiman.c:63)
- 2405 `master_hand_` PORTED (ftmain.c:4360)
- 2443 `master_hand_music_fix_` GAP
- 2468 `giga_fix_` PORTED (ftbosscommon.c:244-262)
- 2493 `giga_stamina_check_` PORTED (sc1premix1p.c:399-417)
- 2530 `master_hand_HP_display_` GAP
- 2565 `giga_change_action_`, 2626 `giga_change_action_2_` PORTED (ftbosscommon.c:224-238)
- 2688 `no_laugh_` PORTED (sc1pgame.c:2570)
- 3581 `remix_1p_randomization` PORTED (sc1premix1p.c:296-310, sc1premix1prules.c, mnplayers1pgame.c:3809)

##### SinglePlayerModes.asm, lines >= 3600 (auditor B)
- 4036 `polygon_selection_match` PORTED; 4084 `polygon_selection_match_rttf` PORTED; 4134 `polygon_file_loading` ADAPTED; 4214 `polygon_allocation` ADAPTED
- 4299 `title_card_text_name`, 4322 `title_card_text_routine` (width deviation), 4487 `title_card_text_position`, 4507 `title_card_text_location_routine`, 4575 `title_card_giant_announce`, 4689 `title_card_yoshi_team_skip`, 4723 `title_card_flag_refresh`, 4763 `title_card_voice_name`, 4787 `title_card_model_scale`, 4810 `title_card_1`, 4833 `title_card_2` PORTED (sc1pintro.c:597-679, 745, 967-975, 1590, 1735-1771, 2313-2356, 2495, 2578-2606)
- 4856 `remix_1p_pointer_part1`, 4887 `remix_1p_pointer_part2` ADAPTED (sc1pgame.c:1157-1166, sc1premix1pdata.c:41-57)
- 4917 `remix_icon_pointer` PORTED (sc1pintro.c:485-489)
- 4954 `metal_bros_magic`, 5007 `metal_bros_bug_fix`, 5045 `metal_bros_text`, 5078 `metal_bros_stage_text` ADAPTED (layout stage, sc1pintro.c:248-255, 1736, 2284, 2315)
- 5115 `remix_1p_giant` PORTED (sc1premix1p.c:305-319)
- 5304 `remix_yoshi_card`, 5327 `remix_yoshi_card_shade_skip`, 5353 `remix_yoshi_card_costumes` PORTED (sc1pintro.c:1675-1688, 1750)
- 5401 `remix_team_shade_start`, 5447 `remix_team_shade_ko` PORTED (sc1pgame.c:1448-1458, 1722-1727)
- 5492/5497 `tiny_team_screen_card` STUB
- 5644 `doubles_card_1`, 5674 `doubles_card_2` PORTED (sc1pintro.c:265-277); 5707 `starfox_costume` PORTED (sc1pmanager.c:531-535); 5736 `doubles_file_loading`, 5772 `giant_file_loading`, 5801 `giant_card` PORTED (sc1pintro.c:2583-2591, 1763)
- 5824 `remix_kirby_card` PORTED (sc1pintro.c:1280); 5848 `remix_kirby_selection`, 5873 `remix_kirby_skip` PORTED (sc1pgame.c:1747); 5899 `remix_kirby_selection_initial` PORTED (sc1pgame.c:1570); 5921 `remix_1p_kirby_load` ADAPTED
- 5942 `_final_stage_spawn` PORTED (sc1pgame.c:1308-1313); 6022 `_giga_usp_prevent` PORTED (ftcommonspecialhi.c:73, sc1premix1p.c:430); 6081 `game_over_end` PORTED (mn1pcontinue.c:1037-1045)
- 6955 `player_percent` STUB; 7003 `clear_percent` STUB; 7030 `clear_percent_continue` PORTED (sc1pmanager.c:708)
- 7081 `rest_area_and_hrc_spawning`, 7111 `..._fix` ADAPTED; 7154 `instant_spawn` PORTED (sc1pgame.c:1324); 7186 `limbo_clear` GAP (ammo half); 7221 `skip_go` PORTED (sc1pgame.c:1980)
- 7815 `heart_persist` GAP; 7850 `progress_advancement` ADAPTED; 7891 `screen_set_end` ADAPTED; 7918 `heart_counter` PORTED (ftcommonget.c:118); 7950 `advance_allstar` ADAPTED; 7989 `pause_exit` PORTED (sc1pmanager.c:668-675)
- 8030 `display_percent` GAP; 8058 `record_percent` PORTED (sc1pgame.c:3222); 8085 `end_allstar` STUB
- 8110 `prevent_erase`, 8129 `stock_prevent`, 8154 `damage_prevent` ADAPTED (sc1pmanager.c:386-415)
- 8180 `costume_fix` NA (dead in All-Star); 8217 `no_miss` PORTED; 8246 `no_miss_2` STUB; 8274 `no_calamity` STUB
- 8454, 8517, 8569, 8601 enemy team sprites / stock loading GAP; 8630 `_allstar_final_character_address_fix`, 8663 `_allstar_final_costume_set` ADAPTED; 8714 `_allstar_final_series_stock_set` GAP

### Caveats

- Two statements have medium or low confidence and need one in-game check: Bowser ammo at creation (`limbo_clear`), and `env_mapping_fix_` on the CSS.
- `no_miss_2` rests on delay-slot loads (the vanilla code around it was not disassembled).
- The patch split (PORTED vs ADAPTED) in section 1 is exact by scope; the patch counts are derived from the TSV and are approximate.

## MN: CSS, menus and screens

Format: asm file:lines (patch count if more than 4) | scope | verdict | evidence.

- CharacterDataScreen.asm:365 | setup_files file array (0x162288) | **PORTED** | mncharacters.c:1238-1242 appends 0x10F5/0x10F6 (File.asm:2222)
- CharacterDataScreen.asm:371,374 | setup_files file count 4 to 6 | **ADAPTED** | lbRelocLoadFilesListed uses ARRAY_COUNT (lbreloc.h:43), both call sites
- CharacterDataScreen.asm:381 | extend_id_table | **PORTED** | mnremixchardata.c:56-63 order table, mncharacters.c:1384
- CharacterDataScreen.asm:392 | extend_character_id_to_data_screen_id_table | **PORTED** | mnremixchardata.c:66-76 mnRemixCharDataGetPage
- CharacterDataScreen.asm:403,414,554 | extend_biography_offsets + extend_biography_file | **PORTED** | mncharacters.c:1433 mnCharactersMakeRemixStory, mnremixchardata.c:36-60 and 105-148 (values and 0x30 addend match asm)
- CharacterDataScreen.asm:423,445,449,453 | extend_series_logo_offsets (emblem) | **STUB** | mncharacters.c:1698-1704 returns before attaching any model; asm reads ResultsScreen.series_logo_offset_table
- CharacterDataScreen.asm:460,479,483,617 | extend_name_offset_and_coordinates + extend_names_file | **PORTED** | mncharacters.c:1463 mnCharactersMakeRemixName, name x/y copied from the asm (mnremixchardata.c:46-65)
- CharacterDataScreen.asm:492,503,574 | extend_works_offsets + extend_works_file | **PORTED** | mncharacters.c:1448 mnCharactersMakeRemixWorks, hooked at 1893
- CharacterDataScreen.asm:509,517 | extend_special_action_pointers | **GAP** | known gap (special-move actions): mncharacters.c:1953-1958 uses common motion descs
- CharacterDataScreen.asm:523,594 | extend_special_attack_offsets + extend_special_attacks_file | **GAP** | known gap (special-move names): mncharacters.c:2368-2372 early return
- CharacterDataScreen.asm:541 | extend_jab_action_pointers | **GAP** | known gap (jab actions): mncharacters.c:1953-1958
- CharacterDataScreen.asm:635 | extend_name_border_check | **GAP** | new: DRM/GND/YLINK/BANJO never get the tall name tag (mncharacters.c:1765 only Purin/Captain)
- CharacterDataScreen.asm:663 | allow_infinite_characters_ | **NA** | N64 heap threshold 0x80700000 and dynamic unload of the previous fighter; port heap not limited, fighter files loaded by ftManagerSetupFilesAllKind (mncharacters.c:2239)
- CharacterDataScreen.asm:984,989 | extend_index_ (scroll max 29) | **PORTED** | mncharacters.c:38-39 MNCHARACTERS_LAST_PAGE = 29, used at 2802-2826
- CharacterDataScreen.asm:996 | anim end check <= 0.1 (fsm freeze fix) | **GAP** | new: mncharacters.c:2017 still anim_frame == 0.0F
- CharacterSelect.asm:116,122,128,134 | load_character_model_ (alt req list, model-only load) | **NA** | N64 RAM saving; port loads all fighter files (ftManagerSetupFilesAllKind in the four CSS MakeFighter, mncharacters.c:2239)
- CharacterSelect.asm:142 | get_alternate_malloc_size_ | **NA** | N64 malloc sizing for load_character_model_
- CharacterSelect.asm:173 | get_alternate_req_list_ | **NA** | N64 req-list swap for load_character_model_
- CharacterSelect.asm:451..479 (6) | get_character_id_ (position to fighter, bookends) | **ADAPTED** | mnremixcss.c:30-39 mnRemixCssSlotAtPoint, mnplayersvs.c:4679, mnplayers1pbonus.c:2098; 1P/training have no bookends (see draw_portraits_)
- CharacterSelect.asm:649..752 (6) | token_autoposition_ (portrait id, x position) | **ADAPTED** | mnRemixCssGetPortrait (mnremixcss.c:51), mnplayersvs.c:5515-5516 centre offsets (cell + w/2 - 13 instead of START_X + 12)
- CharacterSelect.asm:767,772 | token_autoposition_y_fix_ | **ADAPTED** | mnplayersvs.c:5516 (cell + h/2 - 12 instead of START_Y + 12)
- CharacterSelect.asm:809,814,819,825 | adjust_created_token_ (token scale 0.8125) | **GAP** | mnplayersvs.c:5819-5870 MakePuck, no sprite.scalex/scaley on any CSS token
- CharacterSelect.asm:855,860 | adjust_changed_token_ (token scale on PLAYER/CPU change) | **GAP** | mnplayersvs.c:5469-5486 UpdatePuck, no scale
- CharacterSelect.asm:884..920 (8) | adjust_scaled_token_position_ (held token offset +12/-10) | **GAP** | mnplayersvs.c:5606-5607, mnplayers1pgame.c:3168-3169, mnplayers1pbonus.c:2652-2653 keep +11/-14
- CharacterSelect.asm:949,954 | fix_right_boundary_check_ (bookend width) | **PORTED** | mnplayersvs.c:569 mnPlayersVSRemixCellWidth
- CharacterSelect.asm:971,976 | fix_bottom_boundary_check_ (bookend height) | **PORTED** | mnplayersvs.c:574 mnPlayersVSRemixCellHeight
- CharacterSelect.asm:992,996,1000,1004 | cursor max y 205 to 212 | **GAP** | new: mnplayersvs.c:4821, mnplayers1pgame.c:2677, mnplayers1ptraining.c:2204, mnplayers1pbonus.c:2290 still 205.0F
- CharacterSelect.asm:1011,1018 | _fix_variant_overlap_detection (portrait id compare) | **GAP** | new: mnplayersvs.c:5953-5990 and mnplayers1ptraining.c:2918-2927 compare fkind
- CharacterSelect.asm:1076 | VS token overlap radius 15 to 10 | **GAP** | new: mnplayersvs.c:5975 and 5986 still 15.0F
- CharacterSelect.asm:1082..1121 (8) | default portraits off (draw and load, four CSS) | **ADAPTED** | port builds its own 30 portraits (MakeRemixPortrait) and never draws the vanilla ones
- CharacterSelect.asm:1130,1138 | get_random_char_id_ (random pool incl. bonus, variants) | **PORTED** | mnremixvariants.c:134-141 mnRemixRandomPickFighterKind; training CPU pick uses 30 slots only (mnplayers1ptraining.c:3489), see STUB list
- CharacterSelect.asm:1409,1417,1425 | place_token_from_id_ (token placement) | **ADAPTED** | mnplayersvs.c:5515-5516, mnplayers1pgame.c CenterPuckInPortrait; offsets assume the unscaled token (see token scale GAP)
- CharacterSelect.asm:1614..1676 (12) | white flash (set_white_flash_texture, four CSS) | **STUB** | mnplayersvs.c:3984-3993 draws the 43x41 vanilla white square at cell+1, no scale; asm uses the 32x32 flash image (portrait + 0x860) at scale 0.8125
- CharacterSelect.asm:1689 | move_filetable_ (file table length 0x300) | **NA** | port resolves reloc files by id from the .o2r; ids up to 0x10F6 exist
- CharacterSelect.asm:1710,1729,1749,1756 | get_name_texture_ (+training, 1p/btx) | **PORTED** | mnplayersvs.c:1590, mnplayers1pgame.c:834, mnplayers1ptraining.c:549, mnplayers1pbonus.c:701 (kRemixCssNameOffset)
- CharacterSelect.asm:1774,1782,1802,1810 | get_series_logo_offset_ (+1p/btx) | **PORTED** | mnplayersvs.c:1572, mnplayers1pgame.c:825, mnplayers1ptraining.c:529, mnplayers1pbonus.c:692 (kRemixCssLogoOffset)
- CharacterSelect.asm:1826,1833 | 1P and Bonus token drift disabled (PuckAdjustPortraitEdge returns) | **GAP** | new: mnplayers1pgame.c:3375-3413 and mnplayers1pbonus.c:2857 still run the edge spring
- CharacterSelect.asm:1843,1860,1877,1894 | get_zoom_ (spotlight size table, four CSS) | **PORTED** | mnRemixCssGetSpotlightScale used at mnplayersvs.c:6145, mnplayers1pgame.c:3489, mnplayers1ptraining.c:3071, mnplayers1pbonus.c:2971
- CharacterSelect.asm:1911,1917,1923,1929 | get_action_ (selection action table) | **PORTED** | mnRemixCssGetSelectionStatus (mnremixcss.c:84), four CSS
- CharacterSelect.asm:1949,1960 | get_default_costume_ (C-button and team ids) | **PORTED** | mnremixcostumes.c:35-67 reads Character.default_costume.table from the RAM image
- CharacterSelect.asm:1971 | get_default_costume_ max costume id (debug) | **NA** | ftParamGetCostumeDebug has no caller in decomp/src (ftparam.c:3141 only); asm comment says battle debug screen
- CharacterSelect.asm:1987,1993,1999,2005 | load_additional_characters_ (preload loop) | **NA** | N64 heap preload of every model; port loads on first hover
- CharacterSelect.asm:2131 | dynamically_load_character_ | **NA** | N64 custom heap slots (dynamic_css); not needed with a host heap
- CharacterSelect.asm:2228,2239,2250,2261 | get_fgm_ (announcer voice table) | **PORTED** | kRemixCssAnnounceFgm at mnplayersvs.c:4099, 1pgame:2362, training:1664, bonus:1952
- CharacterSelect.asm:2304 | increase_heap_ (expansion RAM) | **NA** | N64 expansion pak heap growth
- CharacterSelect.asm:2351 | clear_expansion_expansion_ram_ | **NA** | N64 expansion RAM bookkeeping
- CharacterSelect.asm:2369,2375,2381,2387 | check_variant_input_ (D-pad variants, L random, Z/R bonus scroll) | **PORTED** | mnremixvariants.c, mnplayersvs.c:5377-5390; open items: Master Hand variant gate, Settings L shortcut (ADAPTED to ImGui)
- CharacterSelect.asm:3816,3830 | draw_portraits_ slide-in 1P and Training | **GAP** | new: no mnRemixCssSlideAttach in mnplayers1pgame.c / mnplayers1ptraining.c; also no bonus/random bookends there
- CharacterSelect.asm:3844,3865 | draw_portraits_ slide-in VS and Bonus | **PORTED** | mnplayersvs.c:596, mnplayers1pbonus.c:652 mnRemixCssSlideAttach + generated kRemixCssPortraitX/Velocity
- CharacterSelect.asm:4474 | use_custom_heap_structs_ | **NA** | N64 alternate heap for dynamic_css
- CharacterSelect.asm:4828 | character_change_ VS (variant indicators) | **PORTED** | mnplayersvs.c:6833 mnRemixCssIndicatorsUpdate
- CharacterSelect.asm:4834,4840,4846 | character_change_ 1P / Training / Bonus (variant D-pad, icons, J/E flag) | **GAP** | new: mnremixcssindicators is only called from mnplayersvs.c
- CharacterSelect.asm:6679,6699 | save_random_char_ VS and Bonus | **PORTED** | mnplayersvs.c:6590-6637, mnRemixBonusModesResolveRandom (mnplayers1pbonus.c)
- CharacterSelect.asm:6685,6691 | save_random_char_ Training and 1P (RANDOM token) | **GAP** | new: no RANDOM bookend or REMIX_ID_RANDOM handling in mnplayers1ptraining.c / mnplayers1pgame.c
- CharacterSelect.asm:6960,6965,7000 | load_random_training_ / load_random_1p_ | **GAP** | new: same cause (no RANDOM token on those CSS)
- CharacterSelect.asm:7031,7060,7083,7101 | awayMode (afk tag: auto START on VS CSS, silent denied FGM, results auto exit) | **GAP** | new: sc1PRemixAfk* only reaches 1P and credits; nothing in mnplayersvs.c / mnvsresults.c
- CharacterSelectDebugMenu.asm:1274,1279,1284,1289 | press_a_handler_ | **PORTED** | mnremixcsspanelmenuinput.c mnRemixPanelMenuPress, called in the four CSS
- CharacterSelectDebugMenu.asm:1439,1444,1449,1457 | hold_a_handler_ | **PORTED** | mnRemixPanelMenuHoldPress / mnRemixPanelMenuRulesHoldAdvance
- CharacterSelectDebugMenu.asm:2076 | team buttons moved 5 px right (0x22 to 0x27) | **GAP** | new: mnplayersvs.c:1486 x = panel*69 + 34, hit test 3314 34..58
- CharacterSelectDebugMenu.asm:2084,2090,2096,2102 | disable_panel_buttons_ | **PORTED** | mnRemixPanelMenuIsOpen gates the type, handicap and team hit tests (mnplayersvs.c:3304); the team button +5 px part is in the GAP above
- CharacterSelectDebugMenu.asm:2137,2142,2147,2153 | prevent_player_model_display_ | **PORTED** | mnRemixPanelMenuSetHideTargets (f16 note)
- CharacterSelectDebugMenu.asm:2192,2197 | prevent_series_logo_and_name_display_ | **PORTED** | same hide targets
- CharacterSelectDebugMenu.asm:2226,2232,2238,2244 | prevent_white_circle_display_ | **PORTED** | same hide targets, spotlight proc asks mnRemixPanelMenuIsOpen
- CharacterSelectDebugMenu.asm:2298 | clear_debug_menu_settings_for_1p_ | **PORTED** | gmRemixPanel returns 0 outside VS/Training (f10_panel_*), ftremixpanel.c:30-37
- CharacterSelectDebugMenu.asm:2429 | CSS heap size 0x55F0+0x4000 | **NA** | N64 memory for the panel menu GObjs (comment: prevents memory overflow)
- Costumes.asm:621,627,633,643 | select_ (19XX costume and shade cycle) | **PORTED** | mnremixcostumes.c:127-148 mnRemixCostumeApplyButton, ftRemixPartImageGetNext/PrevCostume
- Costumes.asm:1051,1059,1064,1069 | disable_update (C buttons in team battle) | **PORTED** | mnRemixCssCostumeButtonsActive (mnplayersvs.c:5399), shade kept in team battle
- Costumes.asm:1098 | cpu_costumes_ VS | **PORTED** | mnplayersvs.c:5109 mnPlayersVSGetCostumeButtonPanel
- Costumes.asm:1103 | cpu_costumes_ Training | **GAP** | new: no equivalent in mnplayers1ptraining.c
- Costumes.asm:1195,1200 | force_hi_lo_poly_ (Model display) | **PORTED** | ftmanager.c:1061, ftparam.c:1130 ftRemixModelDisplayDetail (ftremixhitboxdisplay.c:56)
- Costumes.asm:1284,1290,1296,1304 | prevent_illegal_sound_ and same-colour check | **PORTED** | costume-used denial removed (mnPlayersVSUpdateCostume/SelectFighterPuck and the Training twins)
- Costumes.asm:1311,1316 | check_for_custom_costume_ (extra costume parts) | **ADAPTED** | ftRemixPartImageApplyFighter from ftDisplayMainProcDisplay + mnRemixCostumeApplyPartImage (i7_part_image.md)
- Costumes.asm:1584,1587 | icon_palette_override_ (stock icon palettes) | **PORTED** | ifcommon.c:45 ftRemixPartImageGetStockIconLUT
- PlayerTag.asm:20 | use_custom_tag_image_ (in game) | **PORTED** | ifremixplayertag.c:20-27, ifcommon.c:2154
- PlayerTag.asm:26,32 | use_custom_tag_image_ (results human/CPU) | **GAP** | new: mnvsresults.c:1178-1221 mnVSResultsMakePlayerTag has no Remix branch
- PlayerTag.asm:72 | create_custom_tag_ (in game, name string) | **ADAPTED** | ifRemixPlayerTagDrawName via remix_hud_text (one layer, no black outline copy)
- PlayerTag.asm:78 | create_custom_tag_ (results) | **GAP** | new: same function as above
- PlayerTag.asm:275 | position_custom_tag_ | **ADAPTED** | name is drawn each frame at the sprite centre (ifcommon.c:2124)
- PlayerTag.asm:312 | always_display_custom_tag_ | **PORTED** | ifcommon.c:2100 ifRemixPlayerTagAlwaysShown
- Teams.asm:118 | team_shadow_colors | **PORTED** | ftshadow.c FTSHADOW_TEAM_COLOR -> mnRemixTeamsGetShadowColor (f22 note)
- Teams.asm:128..185 (10) | add_team YELLOW tables (symbolic) | **ADAPTED** | tables read from the RAM image through REMIX_SYM_Teams__* (mnremixteams.c)
- Teams.asm:247..313 (8) | team button, panel colour and click tables | **PORTED** | MNPLAYERSVS_TEAM_BUTTON_OFFSET / TEAM_COLOR / TEAM_NEXT in mnplayersvs.c
- Teams.asm:328 | get_team_costume_ | **PORTED** | mnRemixCostumeGetDefaultTeam / mnRemixTeamsGetCostume
- Teams.asm:362..468 (5) | classic_sonic_costume_fix_ + port colour table + team name | **PORTED** | mnRemixSonicClassicGetTeamCostume; MNPLAYERSVS_TEAM_PORT_COLOR; mnVSResultMakeTeamName
- Teams.asm:489..572 (7) | results screen: logo/wallpaper/name palette/columns/announcer/winner loop | **PORTED** | mnvsresults.c MNVSRESULTS_TEAM_COLOR, MNVSRESULTS_TEAM_ANNOUNCER, MNVSRESULTS_TEAM_LAST (f22 note)
- VsRemixMenu.asm:104,109,114,120 | add_button_ (5th button, wrap) | **PORTED** | mnvsmode.c mnVSModeMain up/down wrap, mnRemixVSModeMakeButton
- VsRemixMenu.asm:147,193,335 | menu button creation and click handler overwrite | **ADAPTED** | mnremixvsmode.c rows; mnVSModeSelectRemixMode (in-place page change, not scene reload)
- VsRemixMenu.asm:271 | render_menu_buttons_ | **PORTED** | mnvsmode.c mnVSModeMakePage1Buttons / mnVSModeMakeRemixButtons
- VsRemixMenu.asm:305 | page_2_exit | **PORTED** | mnvsmode.c B on page 2 returns to page 1
- VsRemixMenu.asm:327 | fix_a_button_crash_ | **NA** | fixes an N64 crash in the replaced code; the port rewrote that block
- VsRemixMenu.asm:477,546,628 | mode_specific_setup_ / _returning_ / _start_ | **ADAPTED** | gmremixmodecss.c gmRemixModeBeforeCSS / LeaveCSS / StartMatch (scmanager.c:1044, mnplayersvs.c:4985, 7077, 7275)
- VsRemixMenu.asm:685 | control_match_start_ | **PORTED** | mnPlayersVSCheckModeTeams in mnPlayersVSCheckReady (f17 note)
- VsRemixMenu.asm:722 | check_match_end_ (KOTH, Smashketball) | **PORTED** | gmremixkoth.c:224, gmremixsmashketball.c:137
- VsRemixMenu.asm:757,765,773 | page_2_no_arrows_ | **PORTED** | mnvsmode.c:1387, 1625-1684
- VsRemixMenu.asm:810 | set_cursor_index_ (cursor and page when returning from a mode CSS) | **GAP** | new: mnvsmode.c:1200-1201 resets page and mode on every entry; asm returns to page 2 with the cursor on the mode used
- VsRemixMenu.asm:867,873,920 | change_time_options (VS time list 1..99, mode lists) | **PORTED** | mnplayersvs.c:7107-7140 mnRemixGoalPickerNext/Prev
- VsRemixMenu.asm:969 | disable_mode_toggle_ | **PORTED** | mnplayersvs.c:3179-3185
- VsRemixMenu.asm:992 | update_announcer_on_toggle_ | **PORTED** | mnplayersvs.c:3243 mnRemixCssModeKeepsToggleVoice
- VsRemixMenu.asm:1009..1047 (8) | add_row_to_results_ (KOTH Times, Smashketball FGs) | **PORTED** | mnvsresults.c:2411-2416, 2473-2478, 3472-3582
- css/Damage.asm:51 | char_init_ (initial damage) | **PORTED** | ftremixpanel.c:23-40 ftRemixPanelGetInitialDamage
- css/DpadControl.asm:47,55 | dont_set_shield_mask_ / set_shield_mask_ | **PORTED** | ftRemixDpadControlSetButtonMasks
- css/DpadControl.asm:99 | set_R_macro | **PORTED** | ftremixdpad.c ftRemixDpadControlController (R becomes A+L)
- css/DpadControl.asm:136 | css_cursor_and_dpad_controls_ (global controller hook) | **GAP** | new: only the match-side transform exists (ftremixdpad.c); nothing reads Toggles.entry_dpad_css_control, no D-pad cursor on the CSS, no menu-wide Stick Swap, no Training pause D-pad menu
- css/DpadControl.asm:506,546,639 | check_jump_type_idle_ / running_ / dash_attack_check_ | **PORTED** | ftRemixDpadControlDisablesStickJump, ...HandlesRunStickUp, ...SkipsDashAttackOf
- css/DpadFunctions.asm:36,552 | dpad_macro_check_ / _held_ (D-pad map) | **PORTED** | ftremixdpadmap.c ftRemixDpadMapApplyPress / HeldButtons
- Costumes.asm:1761-1869 | 13 direct origin writes (attribute data offset of Metal Mario and the 12 polygon fighters) | **ADAPTED** | values come from the built Remix ROM character structs: remix_roster.cpp:51-52 (fkinds 13..25), 345-352 patchVanillaAttributeOffsets, called at 412; default costumes via Character.default_costume.table (mnremixcostumes.c:35-67)

## ST: Stages, hazards, camera, surfaces

Format: asm file:line, scope, verdict, evidence.

##### Blastzone.asm
- Blastzone.asm:91 skip_bottom_blast_sfx PORTED ftmain.c:2108-2116 + grremixblastzone.c:17 (`grRemixBlastZoneSkipsBottomSfx`, bit 2 of the toggle, FGM 0x99 = AltitudeWarn).
- Blastzone.asm:112 loop_screen_right_ PORTED ftcommondead.c:749 + ftremixblastzone.c:`ftRemixBlastZoneWarpFromRight` (x = map_bound_left, release, aerialise, hitbox reset; left/right bound order checked: right bound 0x7A is the left bound).
- Blastzone.asm:165 loop_screen_left_ PORTED ftcommondead.c:761 `WarpFromLeft`.
- Blastzone.asm:213 loop_screen_top_1_ PORTED ftcommondead.c:775 `WarpFromTop`.
- Blastzone.asm:267 loop_screen_top_2_ PORTED ftcommondead.c:785 `WarpFromTop`.
- Blastzone.asm:322 loop_screen_bottom_ PORTED ftcommondead.c:737 `WarpFromBottom` (stuck-action table 0xE3 / 0xE6 / 0x9F / 0xE5, 250-tic fall).

##### Camera.asm
- Camera.asm:46 override_camera_type_ PORTED gmcamera.c:1282 + grremixcamera.c:20 (19 stages -> Inishie routine).
- Camera.asm:108 set_initial_coordinates_ ADAPTED gmcamera.c:1281 `gmRemixCameraSetupDefault` (gmremixcameramode.c:262; fixed-view tables match Camera.asm:1060-1330; the `camera_pan_offsets_` clear depends on #20).
- Camera.asm:462 fixed_camera_boss_ PORTED gmcamera.c:841.
- Camera.asm:486 cinematic_entry_ PORTED ifcommon.c:2636 `gmRemixCameraCinematicFocus`.
- Camera.asm:522 fixed_camera_always_ PORTED gmcamera.c:950 `gmRemixCameraSetStatusIsFrozen`.
- Camera.asm:572 prevent_fixed_camera_pause_bonus_ PORTED ifcommon.c:3358.
- Camera.asm:593 prevent_fixed_camera_pause_bonus_ (data) GAP #21.
- Camera.asm:605 vs_pause_pan_ GAP #20.
- Camera.asm:650 pause_initial_ ADAPTED (FIXED control flag via `gmRemixCameraPauseControlEnabled`, ifcommon.c:3382; the offset clear belongs to #20).
- Camera.asm:845 unpause_camera_ PORTED ifcommon.c:3565 `gmRemixCameraRestorePauseHud` (Special Zoom check via `ftRemixSpecialZoomIsActive`; legend groups replaced by per-frame labels; offset clear belongs to #20).
- Camera.asm:917 disable_hud_ PORTED ifcommon.c:2334.
- Camera.asm:947/951/955/959 frozen_camera_ PORTED gmcamera.c:590, 616, 646, 718 `gmRemixCameraFreezeUpdate` (view tables match Camera.asm:1030-1090).
- Camera.asm:1343 camera_adjust_ ADAPTED ftmain.c:5159 + grremixcamera.c:62. Port tests VS, Training and `nSCKind1PGame` (0x34); the asm tests 0x16, 0x36, 0x77 and 0x01 (see Global.asm:60, 0x01 = title and what the author calls 1P), so the 1P game zoom on Venom / GB Land is a port-side deviation, low impact, low confidence.
- Camera.asm:1389 pause_disable_control_flag_ PORTED ifcommon.c:3382 `gmRemixCameraPauseControlEnabled`.

##### DragonKing/DragonKingSpecial.asm
- :367 drain_flash_fix_ PORTED ifcommon.c:697 + ftremixdragonkingdrainflash.c.
- :647 augment_ball_update_routine_ ADAPTED ftremixdragonkingnsp.c:155 `BallProcUpdate` (hitlag freeze, hand attach via `SetBallPosition`:86).
- :714 destroy_ball_object_ ADAPTED ftremixdragonkingnsp.c:110, :124.

##### DragonKingHUD.asm (all GAP, see #4)
- :49 set_damage_number_offsets_ GAP. :83 set_horizontal_spacing_ GAP. :120/126/132/138 set_vertical_position_ GAP. :193 set_damage_color_ GAP. :280/286/291 adjust_timer_position_ GAP. :368/398 hide_series_logo_ GAP. :434 disable_ko_falling_damage_animation_ GAP. :467 reposition_training_hud_labels_ GAP. :509 reposition_training_hud_damage_ GAP. :541 reposition_training_hud_combo_ GAP. :574/580 reposition_training_hud_speed_enemy_ GAP. :613/619/624 reposition_training_hud_item_ GAP. :659/665 reposition_targets_ GAP. :699 reposition_multi_enemy_icons_ GAP. :732 move_name_down_ GAP.

##### Hazards.asm
- :76 dream_land_wind_ PORTED grpupupu.c:452.
- :87 sector_z_arwing_ PORTED grsector.c:1151.
- :98 mushroom_kingdom_pow_ PORTED grinishie.c:529.
- :109 mushroom_kingdom_piranha_plants_ STUB #16.
- :120 mk_clone_hazard_removal_ STUB #16.
- :153 planet_zebes_acid_ (no hazards) PORTED grzebes.c:277; :159 (no movement) PORTED grzebes.c:246; :165 (AI) GAP known (Phase 6).
- :260 hyrule_castle_tornadoes_ PORTED grhyrule.c:368.
- :271 ai_extend_hyrule_tornado_check GAP known (Phase 6).
- :299 congo_jungle_barrel_ GAP #5.
- :384 saffron_city_pokemon_ PORTED gryamabuki.c:253.
- :431 saffron_city_pokemon_rate_init GAP #13. :477 saffron_city_pokemon_rate GAP #13.
- :538 peach_bumper_ PORTED grcastle.c:77-83 (bumper stage list and hazard toggle; BOWSERB bombs via `grRemixBowserBombsStart`).
- :604 yoshis_island_clouds_ STUB #14.
- :638 yoshis_island_cloud_anims_ PORTED gryoster.c:241 (`gmRemixToggle(yi_clouds)`).
- :655 platform_movement_ STUB #15.
- :829 barrel_angle_fix_ GAP #6. :842, :849, :858 many_barrels_fix_ GAP #6.
- :1122 alt_acid_levels_ PORTED grzebes.c + grremixzebes.c.
- :1261 peach_bumper_hitbox_ PORTED grremixpeachbumper.c (itgbumper.c).
- :1421 bowser_bomb_explosion_ PORTED ittarubomb.c:220. :1458 bowser_bomb_air_collision_ PORTED ittarubomb.c:299. :1482 bomb_hitstun_ PORTED ftcommondamage.c:537. :1553 track_hit_object_ PORTED ftmain.c:2967.
- :1816 klaptrap_room_ GAP #18.
- :2357 btp_bomb_or_barrel_setup GAP #7. :2554 banjo_btp_barrel_firing_fix GAP #7. :2575 marina_btt_setup GAP #8. :2634 rolling_bomb_position GAP #8. :2666/2700/2735 rolling_bomb_explosion_1/2/3 GAP #8.
- :2957 rttf_bumpers_ PORTED grremixrttf.c:12 (0x1E8 for REMIX_RTTF), used at grbonus3.c:11.
- :2980 dreamland_downtime_initially_ PORTED grremixdreamland.c:56. :3009 dreamland_downtime_ PORTED :64. :3038 dreamland_wind_speed_multiplier_ PORTED :99 (Super and Hyper share 0.0006 in the asm; Japanese 0.0003; 1P keeps vanilla). :3092 dreamland_wind_speed_max_ PORTED :80 (the Super-gets-30 quirk is kept). :3130 dream_land_wind_parameters PORTED :132 (Dream Greens edges +-3811, top 1000, bottom -10 equal the vanilla constants; Super 1.09375 and Hyper 1.1953125 box scale).
- :3201/3232/3257/3279 dream_greens_hardcoding_1-4 PORTED grpupupu.c:771 block. :3303/3325/3349/3373 dream_greens_hardcoding_animation_1-4 PORTED grpupupu.c:638-723 (offsets 0x00, 0x20, 0x60, 0x78 match the asm).
- :3397/3419/3441 mk_remix_hardcoding_1-3 PORTED (dead behind #16): grinishie.c:377, 394, 273. :3463 mk_remix_hardcoding_4 PORTED grinishie.c:606. :3485/3507/3531 mk_remix_hardcoding_5-7 PORTED (dead behind #16): itpakkun.c:184, 185, 295. :3555 mk_remix_hardcoding_8 PORTED itpowerblock.c:117.
- :3580 mk_remix_hardcoding_9 GAP known (Phase 6).
- :4356 sonic_btp_setup GAP #9.
- :4437 casino_night_side_bumper_bounce PORTED ftcommonwalldamage.c:47 + grremixcasino.c:36 (0.8 -> 1.1, FGM 0x3D6, surface > 0x16 check).
- :4481 casino_night_side_bumper_knockback GAP #3.
- :6140 chain_chomp_projectile_counter ADAPTED grremixrainbowchomp.c:`grRemixRainbowChompRefreshHitSlots` (refresh 0x19 per record, kept inside the weapon update instead of the hit-record routine).
- :6191 blast_zone_destroy_prevent STUB #19.
- :6224/6265/6309/6350 stoplight_1/2/3/go GAP #17.
- :9484 peach_btt_pipe_override GAP #10.
- :9510 pipe_enter_skip_piranha_plant_check ADAPTED grinishie.c:433 (`grInishiePakkunSetWaitFighter` skips NULL handles; relies on `pakkun_gobj` being cleared on BTT_PEACH, which was not checked for a stale pointer from an earlier Mushroom Kingdom match).
- :9533 platform_despawn_crash_fix GAP #11. :9576 platform_despawn_crash_fix_2 GAP #11.
- :9620 congo_falls_hazard GAP #12.

##### SpecialZoom.asm
- :112 final_knockback_was_set PORTED ftcommondamage.c:698 `ftRemixSpecialZoomFinalKnockback`.
- :237 create_zoom_camera PORTED gmcamera.c (call `ftRemixSpecialZoomMakeWallpaper`).
- :361 zoomed_in_camera_offset PORTED gmcamera.c:749 region `ftRemixSpecialZoomCameraOffset`.
- :420 objects_update PORTED sys/objman.c `ftRemixSpecialZoomSkipsObjectsUpdate`; Stamina skip at gmremixspecialzoom.c:289.

##### Stages.asm
- :562 id_fix_1_ STUB #25.
- :605, :611 id_fix_2_ ADAPTED grremixstage.c:`grRemixMakeGround` (function table from the Remix image) + `ifRemixAnnouncerSetup` at scvsbattle.c:299, sc1pgame.c:2649.
- :673 id_fix_5_ GAP #1.
- :714, :720, :726 id_fix_6_ STUB #26.
- :756 bonus_fix_1_ ADAPTED grremixstage.c:`grRemixMakeGround` (class BTT/BTP -> `sc1PBonusStageMakeBonus1Ground` / `sc1PBonusStageInitBonus2`).
- :800 bonus_fix_2_ ADAPTED sc1pbonusstage.c:396 `sc1PBonusStageGetTarget` (bonus_pointer_table).
- :822, :827, :832, :837 bonus_fix_3_ ADAPTED sc1pbonusstage.c:390 `sc1PBonusStageIsPlatforms` (class table).
- :868 bonus_fix_4_ ADAPTED sc1pbonusstage.c:414 `sc1PBonusStageGetBumperOffsets`. :893, :897, :901, :905 (stage-select direction patches grouped here by the TSV; the Joypad.DL/DR/DD/DU rebinds) ADAPTED mnmaps.c:2989 `mnMapsFuncRun` with `mnremixstagepager.c`.
- :913, :933, :938, :957, :964, :971, :977, :983, :990, :996, :1003, :1009 extend_series_logos_ PORTED mnremixemblem.c (series logo table, positions), mnmaps.c:798-1419 labels/plaque/name hooks (ROADMAP T5.1c).
- :1023 random_fix_1_ ADAPTED mnmaps.c:2035 `mnMapsMakePreview` (stage id from the cursor cell). :1047 random_fix_2_ ADAPTED mnmaps.c:2256-2290 (camera at (1600, 1600), RANDOM -> castle).
- :1064 random_fix_3_ STUB #24.
- :1104 set_preview_position_ PORTED mnmaps.c:1914-1930 (`grRemixGetPreviewZoom` / `grRemixGetPreviewPosition`).
- :1136 get_header_ ADAPTED mnmaps.c:1664 (`grRemixGetGroundFileInfo`, Time Twister SSS id in the table). :1174, :1207 get_type_ ADAPTED mnmaps.c:1664.
- :1214 adjust_heap_check_ NA: extends the free-heap check into the 8 MB expansion RAM (CharacterSelect.expansion_expansion_ram), PC has no fixed heap limit.
- :1252, :1256, :1260, :1264, :1268 reduce_heap_space_ NA: lowers the vanilla heap end 0x80392A00 -> 0x803903E0 so Remix code in RDRAM is not overwritten.
- :1334, :1349 update_stage_icons_ ADAPTED mnmaps.c:1549-1560 + mnremixstagepages.c (icon offsets from `Stages.icon_offset_table`).
- :1842 right_check_, :2153 right_, :2204 left_, :2252 down_, :2300 up_, :2436 a_, :2497 b_, :2533 directional_checks_ ADAPTED mnmaps.c:2989 `mnMapsFuncRun` + mnremixstagepager.c / mnremixstagegridrules.c (3 x 6 grid, 5 pages, page flip).
- :2645, :2652, :2661 training_id_fix_ STUB #24. :2698 (mpCollisionInitGroundData) PORTED mpcollision.c:4113. :2704, :2707 (mnMapsAllocModelHeaps) PORTED mnmaps.c:474.
- :2715, :2721, :2730 bg_animation_ PORTED: ef/efground.c:1804 gates on `gkind <= nGRKindBattleEnd`; asm table is zero for added stages.
- :2778, :2781 bg_type PORTED gr/grwallpaper.c:381-390 (REMIX_RTTF -> Bonus3, other added stages common; but see #25 for the NULL-function stages).
- :4517 setup_ ADAPTED mnremixstagepanel.c (Hazards / Movement rows, cursor colour, L toggle).
- :5323 swap_stage_ (stage select on) PORTED mnmaps.c:2623 `mnMapsRemixPickRandomGroundKind`; :5331 swap_stage_ (stage select off) GAP #23.
- :5455 random_fix_ ADAPTED/NA (low confidence): advances the N64 RNG by frame count on entering the stage select; port stage pick uses `syUtilsRandTimeUCharRange` (osGetTime, sys/utils.c:235). Music RNG not checked.

##### Surface.asm
- :197 get_friction_ PORTED ftphysics.c:177. :209 get_friction_ PORTED ftmain.c:2069. :221 get_friction_ GAP #22.
- :236 get_struct_ GAP #2. :348 fgm_override_ GAP #2.
- :382 zebes_acid_fix_ NA: only gives the Zebes acid knockback struct the extra fgm field (-1 = default); the first seven words are stage data (type 0, damage 16, angle 80, growth 130, base 30, effect 1), and the port reads the stage file struct (grzebes.c:151). Values not compared with the ROM struct.
- :394 fighter_apply_conveyor_surface PORTED ftmain.c:2131 + grremixsurface.c:68.
- :454 item_apply_conveyor_surface PORTED itprocess.c:206-221.

##### Widescreen.asm
- :19, :24 enable_widescreen_ NA (out of scope, game stays 4:3). :53 menu_fix_aspect_ratio_ NA (same).

## IT: Items and Size

Format: asm file:line | scope | verdict | evidence. `n` = number of patch rows in the scope.

- Item.asm:154 | extend_item_spawn_ (n=1) | PORTED | it/itmanager.c:518-529 (itRemixIsCustomKind -> itRemixMakeItem; desc table it/itremix/itremixitems.c)
- Item.asm:178 | extend_item_spawn_gfx_ (n=1) | PORTED | it/itmanager.c:523 (desc->is_show_spawn_gfx gates swirl + itMainSetAppearSpin)
- Item.asm:202 | extend_item_pre_pickup_table_ (n=1) | PORTED | it/itmain.c:578 (desc->proc_hold)
- Item.asm:232 | extend_item_throw_table (n=1) | PORTED | it/itmain.c:494 (desc->proc_thrown)
- Item.asm:267 | extend_training_mode_item_menu_ (n=3) | ADAPTED | sc/sc1pmode/sc1ptrainingmode.c:538-570,1597-1608 + sc1ptrainingremix.c:268-271 (menu entry n -> kind 0x2D+n, name sprite from file 0xFE tables)
- Item.asm:313 | extend_training_mode_hud_ (n=2) | ADAPTED | sc/sc1pmode/sc1ptrainingmode.c:1321-1360 + sc1ptrainingleftoverrules.c:66 (held kind 0x2D..0x39 -> HUD name sprite, others none)
- Item.asm:396 | item_pickup_check_ (n=1) | PORTED | ft/ftcommon/ftcommonget.c:131 (desc->proc_consume in the consume-type switch default)
- Item.asm:495 | extend_item_drop_check_ (n=1) | PORTED | it/itmain.c:455 (desc->proc_dropped)
- Item.asm:522 | extend_player_collision_check_ (n=1) | PORTED | ft/ftmain.c:2903 (desc->proc_touch; hit_normal_damage = 1 is the sw v0,0x264)
- Item.asm:555 | nonzero_check_ (n=1) | PORTED | it/itmanager.c:627,763 (itRemixAnyStandardItemIsOn; training sets gITRemixExtendedToggles = 0 at sc1ptrainingmode.c:841, same as the match-type-7 skip)
- Item.asm:577 | rate_sum_ (n=1) | PORTED | it/itmanager.c:649 + itRemixStandardItemGetWeight in it/itremix/itremixspawn.c (weights read from Stages.custom_item_spawn_rate_table in the RAM image)
- Item.asm:615 | item_count_ (n=1) | PORTED | it/itmanager.c:698 (itRemixStandardItemGetWeight != 0 counted)
- Item.asm:653 | item_arrays_ (n=1) | PORTED | it/itmanager.c:725-734 (kinds/blocks arrays extended per ported standard item)
- Item.asm:694 | rate_sum_item_generators_ (n=1) | PORTED | it/itmanager.c:783 (container drop weights)
- Item.asm:732 | item_count_item_generators_ (n=1) | PORTED | it/itmanager.c:803
- Item.asm:770 | item_arrays_item_generators_ (n=1) | PORTED | it/itmanager.c:830-838
- Item.asm:836 | fix_item_id_check_item_generators_ (n=2) | GAP | itBoxCommonCheckSpawnItems (it/itcommon/itbox.c:273) and itMainMakeContainerItem (it/itmain.c:769) still test `kind <= nITKindCommonEnd`; a Remix kind (>= 0x2D) put into gITManagerRandomWeights by itManagerSetupContainerDrops is treated as a dud, so custom items never drop from crates, barrels, capsules and eggs (and the container opens empty).
- Item.asm:877 | always_update_item_frequency_ (n=1) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:929-955 (mnVSItemSwitchSetItemToggles keeps item_appearance_rate even with every toggle off)
- Item.asm:895 | prevent_1p_custom_item_spawns_ (n=1) | GAP | sc/sc1pmode/sc1pgame.c:1221 copies stagesetup->item_toggles unmasked; no mask 0xFFFFF/zero extended toggles for vanilla 1P, no mask 0xDFFFFFCF + default extended toggles for All-Star (sc1premixallstar.c:270 only handles the rest area). itremixspawn.c:76 relies on the rate rows of gkind 0x13..0x1C being zero, but vanilla 1P stages use VS gkinds (sc1pgame.c:361-620).
- Item.asm:996 | initialize_ (n=1) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:778-820 (mnVSItemSwitchGetItemSettings reads bits 20-31 of item_toggles and gITRemixExtendedToggles bit n-12)
- Item.asm:1049 | save_ (n=1) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:823-900 (mnVSItemSwitchSetItemSettings + itRemixSaveExtendedToggles)
- Item.asm:1127 | item_switch_cursor_ (n=2) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:1159,1205 (L_TRIG/R_TRIG removed from the option masks), 1288-1297 (L = toggle all, R = flip page)
- Item.asm:1136 | handle_r_l_ (n=1) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:975 (mnVSItemSwitchToggleAllPageItems), :998 (mnVSItemSwitchFlipPage). Deviation: asm L flips from vanilla row 1 and sets all 15 vanilla + 13 custom values; port flips only the current page from that page's first row.
- Item.asm:1303 | restrict_cursor_ (n=2) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:600,1109,1136 (mnVSItemSwitchGetPageLastOptionID)
- Item.asm:1338 | handle_custom_item_values_ (n=5) | ADAPTED | mn/mnvsmode/mnvsitemswitch.c:567-630 + 1157-1285 (page-aware status accessors)
- Item.asm:1588 | clear_items_in_1p_ (n=2) | ADAPTED | sc/scmanager.c:1273 calls itRemixClearActiveItems on every scene change (superset of RTTF/BTT/BTP init). Poison.clear_poison_ part: see STUB row below.
- Item.asm:1621 | skip_item_spawn_gfx_ (n=2) | STUB | Spawn half (0x8016EAD8): start-with/taunt items are spawned without swirl (ft/ftremix/ftremixstartitem.c:83-92). Pickup half (0x80172DB8): itMainSetFighterHold (it/itmain.c:590) always calls efManagerItemGetSwirlProcUpdate, no skip flag exists.
- Item.asm:1670 | skip_item_pickup_sound_ (n=1) | GAP | it/itmain.c:598 always plays nSYAudioFGMItemGet for light items; no skip flag set by Start With / Taunt Item / respawn item.
- Item.asm:1729 | match_begin_with_item_ (n=1) | PORTED | if/ifcommon.c:2323 -> ftRemixStartItemMatchBegin (ft/ftremix/ftremixstartitem.c); training: sc/sc1pmode/sc1ptrainingmode.c:2288
- Item.asm:1940 | taunt_spawn_item_ (n=1) | PORTED | ft/ftcommon/ftcommonappeal.c:49 -> ftRemixTauntItemSpawn (ftremixstartitem.c)
- Item.asm:2281 | whos_that_pokemon (n=1) | PORTED | itRemixPokemonMakeMonster in it/itremix/itremixpokemonpicker.c (picker, slots, 1/151 Mew, range shrink), reached from itMainMakeMonster
- Item.asm:2501 | whos_that_pokemon_setup (n=1) | PORTED | it/itmanager.c:863 -> itRemixPokemonResetPicker
- Item.asm:2538 | pokemon_variant_sfx (n=10) | PORTED | itRemixPokemonVoiceFgm (it/itremix/itremixpokemonvoices.c) called at 10 sites (itkabigon.c:172,330; ittosakinto.c:130; itkamex.c:257; itdogas.c:212; itmlucky.c:218; itpippi.c:251; ithitokage.c:238; itfushigibana.c:215; itglucky.c:240). RANDOM mode reads syUtilsRandSeed() for RAM 0x800A0578 (unverified).
- Item.asm:2655 | item_containers_spawn_toggle (n=1) | PORTED | it/itmanager.c:761 -> itRemixContainersApplyToggle (it/itremix/itremixcontainers.c)
- Item.asm:2719 | item_containers_explosions (n=8) | PORTED | itRemixContainerKeeps/itRemixContainerSmashFgm (it/itremix/itremixcontainers.c); hooks itbox.c:262,362,429; itcapsule.c:165,229; ittaru.c:185; itegg.c:168,248
- Item.asm:2864 | item_drop (n=1) | PORTED | ft/ftcommon/ftcommonattackair.c:133 -> ftRemixAerialItemIsThrowType (ft/ftremix/ftremixitemdrop.c)
- Item.asm:2894 | prevent_item_despawn_ (n=1) | PORTED | it/itmap.c:329 -> ftRemixLeftoverItemSkipsDespawn (ft/ftremix/ftremixleftovers.c)
- Size.asm:62 | adjust_respawn_platform (n=1) | PORTED | ef/efmanager.c:6293-6297
- Size.asm:88 | adjust_player_label_ (n=1) | PORTED | if/ifcommon.c:2109
- Size.asm:107 | adjust_player_shadow_ (n=1) | GAP | ft/ftshadow.c:152 (`shadow_size = fp->attr->shadow_size` in ftShadowProcDisplay, which starts at 0x8013AE60 and contains 0x8013B144) is not scaled by the size multiplier.
- Size.asm:129 | passive_armor_ (n=1) | PORTED | ft/ftmain.c:4337 -> ftRemixSizePassiveArmor (ftremixsizebody.c; Stamina armor fix via ftRemixStaminaArmorIsZeroed)
- Size.asm:166 | adjust_ecb_display_ (n=4) | PORTED | ft/ftdisplaymain.c:1446-1493
- Size.asm:241 | adjust_top_joint_and_ecb_ (n=1) | PORTED | ft/ftmain.c:2024 -> ftRemixSizeApplyTopJointAndEcb (ftremixsize.c)
- Size.asm:294 | kirby_ausp_ (n=2) | PORTED | ft/ftchar/ftkirby/ftkirbyspecialhi.c:178,182 -> ftRemixSizeKirbyAirHiScale (ftremixsize.c)
- Size.asm:343 | initialize_base_multiplier_table_ (n=1) | STUB | Size part ported (ft/ftmanager.c:736-737 -> ftRemixSizeInitFighter). The KirbyHats part (magic_hat_on / taunt_loses_power per port, Size.asm:358-379) is not: ftmanager.c:736 says so, and ftRemixKirbyPowerSetTauntLosesPower/ftRemixKirbyPowerReset (ft/ftremix/ftremixkirbypower.c) have no caller, so a panel Kirby Hat never turns the power loss off and the magic hat never starts on.
- Size.asm:444 | fix_size_ (n=2) | PORTED | ft/ftmain.c:270,353 (ftRemixSizeScaleHitboxSize)
- Size.asm:471 | fix_damage_ (n=1) | PORTED | ft/ftmain.c:337 (ftRemixSizeScaleDamage + ftRemixChargeSmashScaleDamage)
- Size.asm:564 | adjust_player_walking_speed_ (n=1) | PORTED | ft/ftphysics.c:103,105
- Size.asm:600 | adjust_player_initial_dash_speed_ (n=1) | PORTED | ft/ftcommon/ftcommondash.c:125
- Size.asm:627 | adjust_player_dash_deceleration_ (n=1) | PORTED | ft/ftphysics.c:59
- Size.asm:655 | adjust_player_running_speed_ (n=1) | PORTED | ft/ftcommon/ftcommonrun.c:39
- Size.asm:682 | adjust_player_jump_squat_ (n=1) | PORTED | ft/ftcommon/ftcommonkneebend.c:32
- Size.asm:708 | adjust_jumping_height_multiplier_1 (n=1) | PORTED | ft/ftcommon/ftcommonjump.c:103
- Size.asm:741 | adjust_jumping_height_multiplier_2 (n=1) | PORTED | ft/ftcommon/ftcommonjumpaerial.c:176-178 (incl. Pwing 0.8 via itRemixPwingIsJumping)
- Size.asm:800 | adjust_jumping_height_multiplier_kirby (n=1) | PORTED | ft/ftcommon/ftcommonjumpaerial.c:253-255
- Size.asm:844 | adjust_player_weight_1 (n=1) | PORTED | ft/ftmain.c:3094
- Size.asm:887 | adjust_player_weight_2 (n=1) | PORTED | ft/ftmain.c:3163
- Size.asm:930 | adjust_player_weight_3 (n=1) | PORTED | ft/ftmain.c:3204
- Size.asm:973 | adjust_player_weight_4 (n=1) | PORTED | ft/ftcommon/ftcommontwister.c:109
- Size.asm:1017 | adjust_player_weight_5 (n=1) | PORTED | ft/ftcommon/ftcommonthrown2.c:202
- Size.asm:1060 | adjust_player_weight_6 (n=1) | PORTED | ft/ftcommon/ftcommonthrown2.c:315
- Size.asm:1103 | adjust_player_weight_7 (n=1) | PORTED | ft/ftcommon/ftcommoncapture.c:177
- Size.asm:1147 | adjust_player_weight_8 (n=1) | PORTED | gr/grcommon/grinishie.c:112
- Size.asm:1190 | adjust_camera_zoom_ (n=1) | PORTED | gm/gmcamera.c:219
- Size.asm:1219 | adjust_camera_zoom_multiplier_ (n=1) | GAP | gm/gmcamera.c:775 (gmCameraUpdatePlayerZoom, 0x8010C960): asm multiplies gGMCameraStruct.pzoom_dist (+0x50, gm/gmtypes.h:33) by the size of pzoom_fighter_gobj (+0x44) before `target_dist = pzoom_dist`; the port assigns it unscaled. Visible in the Training Close-up view with a Giant or Tiny fighter.
- Size.asm:1240 | adjust_camera_center_point_ (n=5) | PORTED | gm/gmcamera.c:315,765; ft/ftdisplaymain.c:1215-1250
- Size.asm:1334 | adjust_size_egg_shield_ (n=1) | PORTED | ef/efmanager.c:4315 -> ftRemixSizeScaleYoshiShield
- Size.asm:1362 | adjust_size_egg_lay_ (n=1) | PORTED | ef/efmanager.c:5588,5636 -> ftRemixSizeScaleYoshiEggLay (ftremixsizefx.c; Character.yoshi_egg.table = effect_size of the port egg desc)
- Size.asm:1398 | adjust_size_egg_entry_ (n=1) | PORTED | ft/ftport.c:130, ft/ftremix/ftremixentry.c:421 -> ftRemixSizeScaleEntryEgg
- Size.asm:1429 | save_player_struct_ (n=6) | PORTED | ft/ftparam.c:2374 + ftcommonwalldamage.c, ftnessspecialhi.c:828, ftcommoncapturekirby.c:402 -> ftRemixSizeAttachGroundGfx
- Size.asm:1541 | update_routine_ (n=1) | PORTED | ef/efmanager.c:238 ftRemixSizeGroundGfxProcUpdate (ground impact wave)
- Size.asm:1584 | adjust_suck_gfx_ (n=4) | PORTED | ef/efmanager.c:6409 -> ftRemixSizeKirbySuckTransform; Kirby USP gfx descriptors efmanager.c:950,984,1048 (ftremixsizemovefx.c:69-103)
- Size.asm:1795 | adjust_usp_projectile_size_ (n=1) | ADAPTED | ft/ftchar/ftlink/ftlinkspecialhi.c:188 -> ftRemixSizeScaleLinkWeapon (ftremixsizefx2.c) (scale set once at spawn; asm rewrites it on every render)
- Size.asm:1843 | adjust_usp_gfx_size_ (n=1) | ADAPTED | ft/ftchar/ftlink/ftlinkspecialhi.c:170 -> ftRemixSizeScaleLinkEffect (ftremixsizefx2.c) (set once at spawn)
- Size.asm:1900 | adjust_gfx_size_ (n=1) | PORTED | ef/efmanager.c:4373 -> ftRemixSizeScaleThunderAmp (ftremixsizefx.c)
- Size.asm:1928 | adjust_pkfire_position_ (n=3) | PORTED | ft/ftchar/ftness/ftnessspecialn.c:40-41
- Size.asm:2017 | update_routine_ (n=1) | PORTED | ef/efmanager.c:1175,5323 ftRemixSizeNessUSPProcUpdate/Initialize
- Size.asm:2022 | update_routine_ (n=1) | PORTED | ef/efmanager.c:1081,5164 ftRemixSizeNessDSPProcUpdate/Initialize
- Size.asm:2092 | update_routine_ (n=1) | GAP | Jigglypuff sing gfx wrapper (descriptor 0x8012E314+0x10) is not ported: no code reads Toggles.entry_puff_sing_anim (symbol only in port/remix/remix_symbols.h:4199); efManagerPurinSingMakeEffect (ef/efmanager.c:4907) keeps the vanilla routine/scale.
- Size.asm:2148 | update_routine_ (n=1) | ADAPTED | Dragon King ball scale applied per update instead of per render: ft/ftremix/dragonking/ftremixdragonkingnsp.c (ftRemixDragonKingNSPApplyBallScale)
- Size.asm:2245 | render_routine_descend_ (n=2) | PORTED | Arwing: ef/efmanager.c:5951 -> ftRemixSizeEntryArwingUpdate (ftremixsizeentry.c)
- Size.asm:2310 | animation_ (n=1) | PORTED | ft/ftcommon/ftcommonentry.c:169 -> ftRemixSizeEntryAnimationScale (ftremixsizeentry.c)
- Size.asm:2362 | gfx_ (n=7) | PORTED | ft/ftport.c:91-155 and ft/ftremix/ftremixentry.c:350-448 -> ftremixsizeentrygfx.c
- Size.asm:2620 | apply_size_to_animation_ (n=1) | PORTED | sys/objanim.c:1063-1077 -> itRemixCloudScaleAnimValue (itremixcloud.c:130-149)
- items/Basketball.asm:1161 | prevent_fighter_enter_barrel_ (n=1) | GAP | No hook in gr/grcommon/grjungle.c grJungleTaruCannCheckGetDamageKind (0x8010A018 region); moot today because the ball never enters a barrel (itremixbasketball.c:755 SKIPPED(barrel), check_basketball_barrel_collision_ not ported), so the Smashketball barrel gimmick is missing.
- items/Basketball.asm:1206 | bounce_patch_ (n=4) | PORTED | it/itmap.c:179,206,233,250 -> itRemixBasketballApplyBounce (itremixbasketball.c:499)
- items/Basketball.asm:1332 | prevent_bounce_fgm_ (n=3) | PORTED | it/itmap.c:191,218,262
- items/Basketball.asm:1371 | multiply_player_hitlag_ (n=1) | PORTED | ft/ftmain.c:2269
- items/Basketball.asm:1403 | multiply_ball_hitlag_hurtbox_ (n=1) | PORTED | it/itprocess.c:1188
- items/Basketball.asm:1427 | add_ball_hitlag_hitbox_ (n=1) | PORTED | ft/ftmain.c:2868
- items/CloakingDevice.asm:223 | prevent_damage_when_cloaked_ (n=1) | PORTED | ft/ftparam.c:1827
- items/CloakingDevice.asm:251 | prevent_shadow_when_cloaked_ (n=1) | GAP | ft/ftshadow.c:117 only tests is_invisible/is_shadow_hide; nothing in itremixcloakingdevice.c hides the drop shadow of a cloaked port.
- items/Cloud.asm:289 | enable_flash (n=1) | PORTED | it/itprocess.c:153-155 -> itRemixKeepsFlashFlags (itremixcloud.c:55)
- items/FranklinBadge.asm:285 | keep_on_action_change_ (n=1) | PORTED | ft/ftmain.c:5087 -> itRemixFranklinBadgeKeepsReflect (itremixfranklinbadge.c)
- items/FranklinBadge.asm:327 | grant_projectile_immunity_ (n=1) | PORTED | ft/ftmain.c:3838 (itRemixFranklinBadgeIsActive)
- items/FranklinBadge.asm:362 | grant_item_immunity_ (n=1) | PORTED | ft/ftmain.c:4035 -> itRemixFranklinBadgeIsItemImmune (itremixfranklinbadge.c)
- items/GoldenGun.asm:193 | projectile_struct_fix (n=1) | PORTED | it/itcommon/itlgun.c:357-359
- items/GoldenGun.asm:218 | projectile_shoot_fgm_fix (n=1) | PORTED | ft/ftcommon/ftcommonitemshoot.c:62 (FGM 0x4C2)
- items/GoldenGun.asm:241 | projectile_properties_fix (n=1) | PORTED | it/itcommon/itlgun.c:371-373 -> itRemixGoldenGunSetupBullet (itremixgoldengun.c; damage 99, KB 45/0/130, angle 7, speed asm 300)
- items/GoldenGun.asm:280 | projectile_no_ammo_fgm_fix (n=1) | PORTED | ft/ftcommon/ftcommonitemshoot.c:78 (FGM 0x4C3)
- items/GoldenGun.asm:307 | extend_grounded_shoot_item_id_check (n=1) | PORTED | ft/ftcommon/ftcommonitemshoot.c:332-356
- items/GoldenGun.asm:330 | extend_aerial_shoot_item_id_check (n=1) | PORTED | ft/ftcommon/ftcommonitemshoot.c:393-404
- items/ScuttleMinion.asm:128 | remove_bob_bomb_walk_fgm_ (n=1) | PORTED | it/itcommon/itbombhei.c:560-566
- items/Stopwatch.asm:959 | physics_map (n=3) | PORTED | ft/ftmain.c:2026,2157,2167 (itRemixStopwatchIsFunctionAbbr)
- items/Stopwatch.asm:1193 | platform_despawn_shadow_crash_fix (n=1) | NA | N64 crash guard: the asm dereferences the clipping/yakumono object table in the shadow display when a platform vanished. ftShadowProcDisplay (ft/ftshadow.c:117-146) only reads the line data through mpCollisionGetFloorEdgeL/mpCollisionGetVertexPositionID, which index the preallocated gMPCollisionYakumonoDObjs array; no equivalent null dereference. Not exercised in game.
- items/WaddleDee.asm:131 | remove_bob_bomb_walk_fgm_ (n=1) | PORTED | it/itcommon/itbombhei.c:560-566 (same hook as ScuttleMinion)
- items/WaddleDoo.asm:410 | beam_knockback_direction_fix (n=1) | GAP | ft/ftmain.c:3305-3314 (weapon branch of ftMainProcessHitCollisionStatsMain, :3050) has no Waddle Doo beam case: for the Doo beam (wp kind DEDEDE_WADDLEDOO_BEAM, kind set in itremixwaddledoo.c:106) the asm uses the projectile direction (lr) with a 0.5 threshold instead of the 5.0 velocity check; the port uses the normal velocity/position rule.

## GP: Gameplay systems

Format: asm file:line, scope, verdict, evidence.

##### Accessibility.asm
- Accessibility.asm:18 flash_guard PORTED if/ifscreenflash.c:81
- Accessibility.asm:37 ParamMakeEffect (save and clear) PORTED ft/ftparam.c:2159, :2167, :2172, :2516; gm/gmremixaccessibility.c
- Accessibility.asm:67 screenshake_toggle PORTED ef/efremixscreenshake.c; ef/efmanager.c:3969; RTTF branch gm/gmremixaccessibility.c:25 (`is_magnify_show` = bit 0x40000 = asm mask 0x4 of the halfword at 0x18C)
- Accessibility.asm:112 blastzone_explode_gfx (3 patches) PORTED ef/efmanager.c:4968, :4982, :5006; ef/efremixblastzone.c

##### AerialAttackFastFall.asm
- AerialAttackFastFall.asm:25 fast_fall_check (5 patches) PORTED ft/ftremix/ftremixaerialfastfall.c:20-44; ft/ftmain.c:5619

##### AirDodge.asm
- AirDodge.asm:65 reset_air_dash_grounded_ PORTED mp/mpcommon.c:452
- AirDodge.asm:84 reset_air_dash_initial_ PORTED ft/ftmanager.c:782
- AirDodge.asm:110 airdodge_check PORTED ft/ftcommon/ftcommonjumpaerial.c:348; ft/ftremix/ftremixairdodge.c (turnaround skipped on purpose: the asm passes the GObj, not the struct, to 0x800E8044)

##### CharEnvColor.asm
- CharEnvColor.asm:950 override_env_color_ (battle and menu) ADAPTED ft/ftdisplaymain.c:1347; ft/ftremix/ftremixenvcolor.c. Env colour yes; part display lists no (gap 6). The menu path (956) goes through the same code, since demo fighters take the override check before the pkind split.
- CharEnvColor.asm:1380 reset_custom_display_lists_during_load_ GAP (tied to gap 6)
- CharEnvColor.asm:1659 update_player_indicator_alpha_ PORTED if/ifcommon.c:2119; ftremixvisibility.c:70
- CharEnvColor.asm:1694 initialize_override_table_ STUB ft/ftmanager.c:738; Cloaked state not mapped (gap 5)

##### Character.asm (hooks)
- Character.asm:1727 get_fireball_struct_ STUB ftmariospecialn.c:43 (gap 7)
- Character.asm:1749 get_kirby_fireball_struct_ ADAPTED ftkirbycopymariospecialn.c:51 (same index function)
- Character.asm:1772 get_ai_behaviour_struct_ (also 1780, ai_attack_prevent 1791, ai_aerial_chase 1810, ai_long_range 1828) ADAPTED, Phase 6: ftremixaitables.c, ftremixaiprevent.c, ftremixailongrange.c; aerial chase is a known AI gap (ROADMAP T7.1c)
- Character.asm:1848 menu_zoom_patches (9 sites, mislabelled get_ai_long_range_table_) ADAPTED port/remix/remix_roster.cpp:329, :365 -> `port_fighter_scale` in mnvsresults.c:1061, mn1pcontinue.c:330, mncharacters.c:1911, CSS files
- Character.asm:1897 thrown_redirect_fix_ ADAPTED ft/ftmain.c:648 (`port_fighter_parent_fkind`; the Remix id table is parent-only: Character.asm:302)
- Character.asm:1925 f_thrown_action_fix_ and 1949 b_thrown_action_fix_ ADAPTED ft/ftcommon/ftcommonthrow.c:100 (clamp to Mario row, deviation noted above)
- Character.asm:1977, 2001 falcon_dive_x/y_fix_ PORTED ft/ftcommon/ftcommoncapturecaptain.c:20 (`ftRemixFalconDiveOffsetRow`)
- Character.asm:2025-2050 get_extended_inhale_struct_ (6 patches) PORTED ft/ftremix/ftremixkirbycopy.c:35; ftkirbyspecialn.c:242; ftcommoncapturekirby.c:524
- Character.asm:2483 apply_sound_type_ PORTED ft/ftmain.c:2442, ft/ftremix/ftremixjapanesesounds.c
- Character.asm:2995 on_action_changed_hook PORTED for `ChargeSmashAttacks.smash_attack_start` (ft/ftremix/ftremixchargesmash.c). The per-character table has no user outside Character.asm.
- Character.asm:3117 replace_action NA: `action_replace_map.table` has no entry set anywhere in the Remix source (the Kirby clone swap is separate: kirby/ftremixkirbyclone.c)

##### Character.asm direct origin writes: see section 5.

##### CustomGrabAction.asm
- CustomGrabAction.asm:12 capture_action_override_ ADAPTED ftcommoncapturepulled.c:129 via `port_fighter_custom_capture_action` (Wario, Piano registered in ftremixdeschooks.c)
- CustomGrabAction.asm:41 capture_position_fix_ ADAPTED same file (not compared against the frame-1 call to 0x8014AB64, as the note says)
- CustomGrabAction.asm:79 capture_dk_break_fix_ ADAPTED ftcommoncapture.c (SR block in `ftCommonCaptureShoulderedProcInterrupt`), Piano registration

##### Damage.asm
- Damage.asm:40 extend_on_hit_gfx_ PORTED ft/ftmain.c:3111-3118; ftremixdamage.c:77
- Damage.asm:60 extend_on_hit_routines_ PORTED ft/ftcommon/ftcommondamage.c:399; ftremixdamage.c:44
- Damage.asm:199 stun_damage_action_ STUB ftremixdamagestun.c:195 (stun, deku stun, flashbang OK; BURY branch has gaps 3 and 4)
- Damage.asm:888 prevent_wind_push_while_buried PORTED ft/ftparam.c:632
- Damage.asm:912 stun_electric_action_ ADAPTED ftcommondamage.c:629 (see gap 13)

##### FootStool.asm
- FootStool.asm:11 num_jumps_no_reset PORTED mp/mpcommon.c:925
- FootStool.asm:33, 545, 574, 603 (aerial and Kirby/Puff jump checks) PORTED ft/ftcommon/ftcommonjumpaerial.c:373-436; ftremixfootstool.c. The word at 0x148 is `jumps_used` plus a zero pad, so "aerial = jumps_used != 0" matches the asm. The Marina `jump_timing_patch_` shares site 0x80140310 and belongs to another group (not checked here).

##### GFX.asm
- GFX.asm:38 augment_gfx_instructions_loader_ (2 patches) ADAPTED lb/lbparticle.c:510, :574 (`lbRemixGfxTakeScriptOverride`)
- GFX.asm:72 extend_gfx_command_jump_table_ ADAPTED ft/ftparam.c:2161 (`lbRemixGfxPrepareEffect`); entry 0x64 is gap 12; the origin write 0x6669B (size check) is replaced by `lbRemixGfxIsNewEffect`
- GFX.asm:100, 128, 152, 174, 529 (2) extended script/texture maps ADAPTED lb/lbremixgfx.c:197-232 (grown arrays, 33 scripts, 9 texture blocks match the asm counts). The Orange Blast base id 0x62 invented as 0x1F is harmless (no caller).

##### Hitbox.asm
- Hitbox.asm:27 hitbox_mode_, :117 projectile_, :156 grab_immunity_and_armor_frames_, :195 force_grab_immunity_, :250 make_hurtboxes_transparent_ (2), :326 make_hitboxes_transparent_, :342 add_reflect_absorb_hitboxes (5), :511 render_hitbox_plus_model_, :547 render_hitbox_plus_projectile_ (2), :591 render_hitbox_plus_item_ (8), :714 fix_hitbox_position_ (2) PORTED ft/ftremix/ftremixhitbox.c, ftremixhitboxdisplay.c, ftmain.c:4227, it/itdisplay.c, wp/wpdisplay.c (details in toggles.md; Advanced Hurtbox colours match the asm values)
- Hitbox.asm:782 skip_rendering_hitboxes_ and :809 render_ecb_over_hurtboxes_ NA: only reachable with `kirby_cube_enabled`, which needs the 8th Player Tag text to read "cube" (Hitbox.asm:845-851); the port has no editable tag text

##### Parry.asm
- Parry.asm:17, 230, 430, 625 PORTED ft/ftremix/ftremixparry.c; ftmain.c:3593, :3768, :3983, :4486 (the asm always passes the previous position of the hitbox, the port too)

##### Rage.asm
- Rage.asm:17, 45, 83, 110, 227 PORTED ft/ftremix/ftremixrage.c; ftmain.c:3083, :3162, :2127; ftparam.c:1740; ftcommonthrown2.c:201 (Ultimate fixed-knockback check kept literal)

##### Reflect.asm
- Reflect.asm:55, 93, 131, 165, 251, 327, 421 (AI) GAP (gap 8)
- Reflect.asm:507 override_reflectability_ PORTED ftremixreflect.c:56 (weapon ids kept as kinds); ftmain.c:3774
- Reflect.asm:562 extend_reflect_types PORTED ftremixreflect.c:79; ftmain.c:4461
- Reflect.asm:597 override_projectile_reflect_routine_ PORTED wp/wpprocess.c:541 (Thunder trail: matches the head by owner rather than by instance id 0x264, close enough)
- Reflect.asm:676 override_item_reflect_routine_ PORTED it/itprocess.c:1143
- Reflect.asm:767 projectile_direction_fix_custom_ PORTED wp/wpmain.c:111
- Reflect.asm:818 item_direction_fix_custom_ PORTED it/itmain.c:917

##### Shield.asm
- Shield.asm:48 color_fix_ PORTED ef/efmanager.c:4252; ft/ftremix/ftremixshield.c (damage-flash deviation above)
- Shield.asm:190, 195 egg shield and escape PORTED ef/efmanager.c:4316 (`efManagerYoshiShieldProcDisplay` serves both effects); ftremixyoshiegg.c
- Shield.asm:200, 206, 211, 216 GAP, known I7

##### SwordTrail.asm
- SwordTrail.asm:137, 189 (4), 229, 258 PORTED ft/ftremix/ftremixswordtrail.c; ftmain.c:4603; ftdisplaymain.c:476, :1027 (table of 21 trails matches the asm)

##### Tripping.asm
- Tripping.asm:14, 19, 24, 112, 138, 164 PORTED ft/ftremix/ftremixtripping.c; ftcommonturn.c:107-134; ftcommondash.c:118; ftcommonturnrun.c:44; ftcommondamage.c:124

##### WallTeching.asm
- WallTeching.asm:8, 66, 88 PORTED ft/ftremix/ftremixwalltech.c; ftcommonwalldamage.c:63; ftcommonpassive.c:19; ftcommonpassivestand.c:19

##### command.asm
- command.asm:150 end_hitbox_ PORTED ft/ftparam.c:582
- command.asm:190 create_hitbox_ PORTED ft/ftmain.c:227
- command.asm:232 texture_form_fix_ PORTED ft/ftparam.c:1334
- command.asm:612, 664, 716, 768, 821 translation_multiplier_ apply_* STUB (gap 2)
- command.asm:921 apply_fgm_ PORTED ft/ftmain.c:2451; ftremixcommand.c:395
- Commands 0xD0-0xDC and the three `load_command_` hooks PORTED ft/ftmain.c:902, :966, :1070; ftremixcommand.c
- change_action_ (0x6272C): translation and env reset PORTED (ftmain.c:4992); training reset GAP (gap 10); respawn item STUB (gap 9)

##### fireball.asm
- fireball.asm:223 bounce_destroy_check PORTED wp/wpmario/wpmarioremixfireball.c:64
- fireball.asm:345 kirby_fix_ PORTED ftkirbycopymariospecialn.c:45
- fireball.asm:452, 457 load_capsule_subroutine_ ADAPTED palette roll at spawn (wpmariofireball.c:277, wpmarioremixfireball.c:48)
- fireball.asm:511 capsule_flag ADAPTED Dr. Luigi found from owner/copy id (wpmarioremixfireball.c:80)

Character.asm (45):
- 1048 (0x93074, `move_mario_parameter_array`) NA: N64 memory layout of Mario's parameter array. The port reads action data through `remix_action_layout.h`.
- 1172, 1186, 1198 (0x5284C, 0x52CC8, 0x53040: segment sizes, file pointer loop for NUM_CHARACTERS) ADAPTED: port loops over registered fkinds, ft/ftmanager.c:106-146, :366, :512.
- 1208 (0x62BD0, `move_action_array_table_`) ADAPTED: ft/ftremix/ftremixbind.c:274 and ftremixvanillaprocs.c read `REMIX_ACTION_ARRAY_TABLE`.
- 1233, 1236, 1253, 1256, 1271, 1293, 1297, 1314, 1318, 1335, 1339, 1356, 1360, 1375 (jab 3 / rapid jab tables and checks) GAP: gap 1.
- 1396, 1415, 1434, 1453, 1472, 1491, 1510, 1528 (Kirby/character NSP, USP, DSP air and ground tables) ADAPTED: `port_fighter_special_handler` (ftcommonspecialn.c:110, :169; ftcommonspecialair.c:172-257; ftcommonspecialhi.c:81; ftcommonspeciallw.c:79) with Remix actions bound by ftRemixBindSpecials.
- 1546, 1553, 1566 (0xB8664, 0xB86A4, 0xB86B8: entry action/script tables) ADAPTED: ftremixentry.c, bindEntryStatuses in remix_roster.cpp.
- 1583, 1586 (0x53588, 0x535A8: initial/grounded script tables) ADAPTED: registry descriptors (ftmanager.c:738+).
- 1603, 1606 (0x59C08, 0x59C14: electric_hit table) ADAPTED: ftremixdamage.c / ftcommondamage.c element handling.
- 1621, 1640 (0x65148, 0x65300: down_bound_fgm, crowd_chant_fgm) ADAPTED: registry fields.
- 1660 (0xBEEA0: yoshi_egg table) ADAPTED: ftremixyoshiegg.c (shield), rest known I7.
- 1680 (0xDF56C), 1697 (0xC7840), 1708 (0x7E89C), 1715 (0x7E8E4: AI/other tables and Kirby inhale hooks) ADAPTED: see get_extended_inhale_struct_ above and Phase 6 AI tables.
- 1791, 1810, 1828 (0xADF10, 0xAE2C8, 0xB3700: ai_attack_prevent, ai_aerial_chase, ai_long_range) ADAPTED: Phase 6 AI (aerial chase = known gap).
- 3592 (0x97A4C+0x68, GDONKEY menu array copy) NA: N64 menu action array layout.
- Others in the list (1727/1749 etc.) are `patch_start` hooks, covered in section 4.

command.asm (5): 581 (0x5FC6C `apply_direction_`) PORTED ft/ftmain.c:3274; 1155, 1159, 1163 (`load_command_` hooks) PORTED ftmain.c:902, :966, :1070; 1168 (0x6272C `change_action_`) see section 4 (partly STUB/GAP).

fireball.asm (3): 83 (0xE310C `hit_effect_` hook) and 88 (0xE3124 branch target) PORTED wpMarioFireballProcHit (wpmariofireball.c:196, ftRemix helper wpmarioremixfireball.c:111); 171 (0xE30E0 `bounce_effect_`) PORTED wpmariofireball.c:181 (smoke arguments assumed: `lr` and 1.0, flagged in the shared_fireball note).

GFX.asm (1): 400 (0x6669B GFX id limit) ADAPTED lb/lbremixgfx.c:276 (`lbRemixGfxIsNewEffect`).

Not in this group (other groups own them): moveset.asm:15, FGM.asm, Costumes.asm, midi.asm, PAL.asm.

## SH: Shared fighter patches

asm file:line | scope | verdict | evidence

- Crash.asm:10 | (unnamed patches) | NA (8 patches) | N64 crash screen (syDebugReportCPUBreakFault thread waiting for a button chord, OS_EVENT_FAULT/CPU_BREAK). The TSV function (syControllerParseEvent) is wrong: 0x80023xxx is the debug thread, sys/debug.c:850. osSetEventMesg is a stub on PC (port/stubs/n64_stubs.c:369) and nothing raises a fault event; debug.c PORT path logs and aborts (debug.c:1011).
- KirbyHats.asm:73 | use_extended_special_parts_copy_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:1034 -> ftRemixKirbyHatModelPart (ft/ftremix/ftremixkirbyhats.c:78), rows read from REMIX_SYM_KirbyHats__extended_special_parts
- KirbyHats.asm:112 | use_extended_special_parts_pause_ | STUB (1 patch) | decomp/src/ft/ftparam.c:916 hat joint (6) swap is ported; the gun/sword part branch (a1 == 0x11, KirbyHats.asm:122-150, hat ids Marth/Roy/Wolf/Slippy/Peppy/Goemon/Ebi/Lanky/Conker) is not: ftRemixKirbyHatModelPart returns the vanilla row for any joint != 6 (ftremixkirbyhats.c:80)
- KirbyHats.asm:244 | use_extended_special_parts_costume_init_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:1201 -> ftRemixKirbyHatModelPart
- KirbyHats.asm:526 | set_absorbed_hat_ | PORTED (2 patches) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:235 (ftRemixKirbyStolenPowerID) and :157 (ftRemixKirbyUpdateMagicHat); ft/ftremix/ftremixkirbypower.c:62-82
- KirbyHats.asm:575 | kirby_hat_select_ | STUB (1 patch) | decomp/src/ft/ftmanager.c:908 -> ftRemixKirbyHatPanelModelPart (ftremixkirbyhats.c:145) sets only the hat model part. The asm (and css/KirbyHat.asm onchange_handler 'sw t2, 0x0ADC(t1)') spawns Kirby holding that fighter's copy power (copy_id), no clear_magic_hat_charge for '???', no copy_id/is_ignore_losecopy
- KirbyHats.asm:652 | kirby_hat_files_training_ | ADAPTED (1 patch) | decomp/src/ft/ftremix/ftremixkirbyhats.c:56-63 loads KIRBY_CUSTOM_HATS(_2) lazily on first hat use; no per-mode preload needed on PC
- KirbyHats.asm:685 | kirby_hat_files_vs_ | ADAPTED (2 patches) | same lazy load, ftremixkirbyhats.c:56-63
- KirbyHats.asm:726 | kirby_hat_files_bonus_ | ADAPTED (1 patch) | same lazy load, ftremixkirbyhats.c:56-63
- KirbyHats.asm:759 | kirby_hat_files_1p_ | ADAPTED (1 patch) | same lazy load, ftremixkirbyhats.c:56-63
- KirbyHats.asm:791 | hat_loss_prevent_ | STUB (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:1002 -> ftRemixKirbyLoseCopyPrevented exists, but sFTRemixKirbyTauntLosesPower is never set to FALSE: ftRemixKirbyPowerSetTauntLosesPower and ftRemixKirbyPowerReset have no callers (Size.asm:376-381 sets it per port at spawn). A power given by the Kirby Hat is always lost on taunt/hit
- KirbyHats.asm:825 | kirby_power_loss_prevent_ | STUB (1 patch) | same as hat_loss_prevent_ (same hook, ftkirbyspecialn.c:1002)
- KirbyHats.asm:855 | set_absorbed_ | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommoncapturekirby.c:333 -> ftRemixKirbyPowerStolen (ftremixkirbypower.c:50)
- captainshared.asm:8 | (unnamed patches) | PORTED (3 patches) | decomp/src/ef/efmanager.c:828 (matrix 0x1C), :844 (ftRemixSizeFalconPunchProcDisplay), :805 (ftRemixSizeFalconKickProcUpdate, ft/ftremix/ftremixsizemovefx.c:113)
- captainshared.asm:122 | get_kick_anim_struct_ | PORTED (1 patch) | decomp/src/ef/efmanager.c:4812 -> ftRemixCaptainSharedKickDesc (ft/ftremix/ftremixdkcaptainshared.c:48); first-frame _apply_scale folded into the update proc
- captainshared.asm:172 | get_punch_anim_struct_ | GAP (1 patch) | see table: Wolf slash struct, Kirby-copy routing, PokemonAnnouncer.firepunch; only the GND/JFALCON/DRAGONKING file swap is done (ftremixdkcaptainshared.c:63)
- captainshared.asm:234 | get_entry_anim_struct_ | PORTED (1 patch) | decomp/src/ft/ftremix/ftremixentrydata.c:53-55 (JFALCON/BOWSER/CONKER rows) via ftRemixEntryMakeCaptainCar (ftremixentry.c:437). GND and DRAGONKING branches are dead: Ganondorf.asm:168 and DragonKing.asm:315 skip the entry script
- captainshared.asm:295 | get_entry_file_ptr_ | PORTED (1 patch) | same rows, ftremixentry.c:208-230 (file_n 7 / 8)
- captainshared.asm:339 | get_punch_bone_ | GAP (1 patch) | see table: Wolf must use joints[16] (efmanager.c:4866 only widens to the three Falcon clones)
- captainshared.asm:383 | clown_car_animation1 | PORTED (1 patch) | decomp/src/ef/efmanager.c:5854-5859 + ftRemixEntryGetCarAnim (ftremixentry.c:275), rows ftremixentrydata.c:62-64 (0x248C / 0x188C)
- captainshared.asm:412 | clown_car_animation2 | PORTED (1 patch) | same, wheel offsets 0x24C0/0x2530 and 0x18C0/0x1930 (ftremixentrydata.c:62-64)
- captainshared.asm:444 | kirby_power_change_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:112-114 (JFALCON, GND join the Captain case)
- dkshared.asm:9 | cargo_hold_fix_1 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonthrown1.c:27
- dkshared.asm:34 | cargo_hold_fix_2 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonthrow.c:71
- dkshared.asm:58 | item_fix_1 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonget.c:184
- dkshared.asm:82 | item_fix_2 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonitemthrow.c:94 (0x80146420 is inside ftCommonItemThrowProcUpdate, not the TSV function)
- dkshared.asm:106 | item_fix_3 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonitemthrow.c:251
- dkshared.asm:130 | item_fix_4 | PORTED (1 patch) | decomp/src/ft/ftparam.c:1862
- dkshared.asm:154 | item_fix_5 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommondamage.c:894
- dkshared.asm:178 | item_fix_6 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommondamage.c:479
- dkshared.asm:203 | fully_charged_check_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:2196 (case REMIX_ID_JDK)
- dkshared.asm:225 | kirby_power_check_flash_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:1534 (copy_id == REMIX_ID_JDK)
- dkshared.asm:246 | kirby_power_change_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:105
- dkshared.asm:268 | giant_punch_fix_1 | AI (1 patch) | AI hook (Phase 6); JDK is cased in decomp/src/ft/ftcomputer.c:6035 and :6156
- dkshared.asm:289 | cpu_fix_2 | AI (1 patch) | AI hook (Phase 6), not checked (func_ovl3_80138EE4, ftcomputer.c:7344); Marina cargo branch not found
- dkshared.asm:335 | barrel_alternate | ADAPTED (1 patch) | decomp/src/ft/ftremix/ftremixentry.c:391-401 + ftremixentrydata.c:49-51 (Sonic right/left, Lanky). The SSONIC branch is dead: SSonic.asm:340 skips the entry script. The asm also clears fp->lr (0x44) there; the port keys on entry.lr
- jigglypuffkirbyshared.asm:12 | jump_fix_1 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonjumpaerial.c:217-229
- jigglypuffkirbyshared.asm:44 | jump_fix_2 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonjumpaerial.c:266-285 (Dedede table 68/58/52 = Dedede.asm:735-741, ftremixpuffkirbyshared.c)
- jigglypuffkirbyshared.asm:91 | jump_fix_3 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonjumpaerial.c:358-360
- jigglypuffkirbyshared.asm:119 | jump_fix_4 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonjumpaerial.c:115-130
- jigglypuffkirbyshared.asm:151 | jump_fix_5 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonjumpaerial.c:384-409
- jigglypuffkirbyshared.asm:183 | kirby_boomerangfix_1 | PORTED (1 patch) | decomp/src/wp/wplink/wplinkboomerang.c:323
- jigglypuffkirbyshared.asm:203 | kirby_boomerangfix_2 | PORTED (1 patch) | decomp/src/wp/wplink/wplinkboomerang.c:296
- jigglypuffkirbyshared.asm:223 | kirby_special_struct_fix | PORTED (1 patch) | decomp/src/wp/wpkirby/wpkirbycutter.c:147 (ftRemixKirbyCutterWeaponDesc); 0x8016BEA0 is wpKirbyCutterMakeWeapon, not the TSV function
- jigglypuffkirbyshared.asm:253 | kirby_infinite_1 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonattack100.c:120
- jigglypuffkirbyshared.asm:273 | kirby_fthrow_1_fix | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonthrow.c:106-142 (JKIRBY plus Bowser/GBowser branch with status 0xE5 and the Seismic Toss call)
- jigglypuffkirbyshared.asm:307 | kirby_fthrow_2_fix | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonthrow.c:187
- jigglypuffkirbyshared.asm:327 | kirby_blast_fix_1 | STUB (1 patch) | decomp/src/ft/ftmanager.c:595-606 has JKIRBY and the Link clones; the BOWSER 'sh 0x14, 0x0ADE(v1)' ammo refill (asm 347-348) is missing. 0x0ADE is Bowser's flame ammo (Bowser/bowser.asm:533), so v1 is the fighter struct
- jigglypuffkirbyshared.asm:365 | kirby_load_with_hat_fix_ | STUB (1 patch) | decomp/src/ft/ftmanager.c:825 only adds the switch case; the second 'fkind == Kirby' (0x800D7E54, the hat-set block) at ftmanager.c:917 still excludes JKIRBY, so a J Kirby that spawns with copy_id != Kirby gets no hat (the CSS panel path ftmanager.c:908 does cover JKIRBY)
- jigglypuffkirbyshared.asm:387 | kirby_taunt_fix_1 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonappeal.c:38
- jigglypuffkirbyshared.asm:429 | pokemon_spawn_fix | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonentry.c:106
- jigglypuffkirbyshared.asm:460 | pokemon_hat_fix_1 | PORTED (1 patch) | decomp/src/ft/ftdisplaymain.c:868 (the TSV function mpCollisionGetVertexPositionID is wrong)
- jigglypuffkirbyshared.asm:487 | pokemon_hat_fix_2 | PORTED (1 patch) | decomp/src/ft/ftdisplaymain.c:815 and :859
- jigglypuffkirbyshared.asm:514 | kirby_cpu_inhale | AI (1 patch) | AI hook (Phase 6), not ported (no JKIRBY/DEDEDE in ftcomputer.c)
- jigglypuffkirbyshared.asm:540 | kirby_cpu_inhale_2 | AI (1 patch) | AI hook (Phase 6), not ported
- jigglypuffkirbyshared.asm:562 | kirby_cpu_inhale_3 | AI (1 patch) | AI hook (Phase 6), not ported
- jigglypuffkirbyshared.asm:618 | kirby_cpu_inhale_4 | GAP (1 patch) | NOT AI despite the name: 0x800E23E8 is the 'fkind == Kirby' test of ftParamKirbyTryMakeMapStarEffect inside ftMainProcPhysicsMap (ft/ftmain.c:2145). See table
- jigglypuffkirbyshared.asm:644 | fully_charged_check_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:2196 (case REMIX_ID_JKIRBY next to nFTKindKirby)
- jigglypuffkirbyshared.asm:665 | kirby_power_steal_check_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:230
- jigglypuffkirbyshared.asm:686 | kirby_power_stolen_check_ | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommoncapturekirby.c:327
- jigglypuffkirbyshared.asm:707 | kirby_power_change_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:121-122 (JPUFF, EPUFF)
- jigglypuffkirbyshared.asm:731 | kirby_ammo_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:158 -> ftRemixKirbyCopyAmmo (ftremixpuffkirbyshared.c:54)
- jigglypuffkirbyshared.asm:750 | kirby_rock | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspeciallw.c:58
- jigglypuffkirbyshared.asm:771 | kirby_rock_2 | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspeciallw.c:25
- jigglypuffkirbyshared.asm:792 | kirby_rock_3 | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspeciallw.c:98
- jigglypuffkirbyshared.asm:813 | multi_jump_recovery_fix_ | AI (1 patch) | AI hook (Phase 6), not ported
- jigglypuffkirbyshared.asm:860 | remix_skip_offensive_usp_air_ | AI (1 patch) | AI hook (Phase 6), not ported
- linkshared.asm:175 | get_entry_anim_struct_1 | ADAPTED (1 patch) | decomp/src/ft/ftremix/ftremixentry.c:343-358 + ftremixentrydata.c:21-42 (Young Link, Marth, Roy, Marina, Goemon/Ebi skip, Banjo swapped bases)
- linkshared.asm:232 | get_entry_anim_struct_2 | ADAPTED (1 patch) | same rows, nFTRemixEntryEffectLinkBeam
- linkshared.asm:288 | create_bomb_ | STUB (4 patches) | decomp/src/it/itfighter/itlinkbomb.c:628-664 ported: per-owner file, owner id in coll_data.cliff_id, Young Link bombchu blast zone/FGM. Missing: asm render_routine_ (0x801865DC) changes the first gcAddXObjForDObjFixed kind 0x2E to 0x48 for YLINK; port keeps 0x2E (itlinkbomb.c:643)
- linkshared.asm:382 | bomb_flash_ | PORTED (1 patch) | decomp/src/it/itfighter/itlinkbomb.c:161 (owner file from coll_data.cliff_id)
- linkshared.asm:414 | bomb_explosion_ | PORTED (1 patch) | decomp/src/it/itfighter/itlinkbomb.c:541
- linkshared.asm:444 | item_shield_fix_ | PORTED (2 patches) | decomp/src/ft/ftparam.c:657 and :673 (ftRemixLinkSharedIsLinkFamily)
- linkshared.asm:483 | grab_shield_fix_ | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommoncatch2.c:83
- linkshared.asm:517 | dair_fix_ | PORTED (2 patches) | decomp/src/ft/ftcommon/ftcommonattackair.c:22 and :47
- linkshared.asm:556 | sword_trail_fix_ | PORTED (1 patch) | decomp/src/ft/ftmain.c:4542 (ftRemixLinkSharedHasSwordTrail)
- linkshared.asm:590 | up_special_fix_ | PORTED (1 patch) | decomp/src/wp/wplink/wplinkspinattack.c:131
- linkshared.asm:629 | boomerang_fix_ | PORTED (1 patch) | decomp/src/wp/wplink/wplinkboomerang.c:593 (Kirby copy owner handled)
- linkshared.asm:676 | up_special_velocity_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftlink/ftlinkspecialhi.c:379 (71/72/52 match asm)
- linkshared.asm:714 | up_special_aerial_drift_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftlink/ftlinkspecialhi.c:245 (0.75 for JLINK)
- linkshared.asm:741 | cpu_pull_out_bombs_ | AI (1 patch) | AI hook, already ported: decomp/src/ft/ftcomputer.c:7167
- linkshared.asm:790 | boomerang_crash_fix_ | PORTED (3 patches) | decomp/src/wp/wplink/wplinkboomerang.c (CheckOwnerCatch returns the catch result, ProcUpdate returns it); wp/wpprocess.c:133 destroys the weapon
- marioshared.asm:205 | pipe_turn_enter | STUB (1 patch) | decomp/src/ft/ftcommon/ftcommondokan.c:109 -> ftRemixIsPipeTurnFkind (ftremixpuffkirbyshared.c:48) hard-codes only DRL; Character.pipe_turn is TRUE for 12 ids (marioshared.asm:157-200 plus DrLuigi.asm:150)
- marioshared.asm:231 | pipe_turn_exit | STUB (1 patch) | decomp/src/ft/ftcommon/ftcommondokan.c:298, same helper
- nessshared.asm:10 | pk_thunder_reflect_crash_fix_1_ | GAP (1 patch) | no equivalent: wp/wpness/wpnesspkthunder.c:557-565 and :664-707 still keep the vanilla behaviour (the decomp comments there describe the fix, not applied). Remix toggle defaults to ON (Toggles.asm:2407)
- nessshared.asm:36 | pk_thunder_reflect_crash_fix_2_ | GAP (1 patch) | no equivalent: wp/wpness/wpnesspkthunder.c:557-565 and :664-707 still keep the vanilla behaviour (the decomp comments there describe the fix, not applied). Remix toggle defaults to ON (Toggles.asm:2407)
- nessshared.asm:75 | get_pkfire_pointer_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:122 | get_pkfire_collision_pointer_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:196 | ness_jump1_ | GAP (1 patch) | ftCommonJumpAerialSetStatus (ft/ftcommon/ftcommonjumpaerial.c:154-165) tests only Ness/NNess; Character.ness_jump is TRUE for NESS, NNESS, MTWO, NMTWO, JNESS, LUCAS, NLUCAS, PEACH, NPEACH
- nessshared.asm:223 | ness_jump2_ | GAP (1 patch) | same table, vel_x site ftcommonjumpaerial.c:183-185
- nessshared.asm:253 | up_special_landing_fsm_ | GAP (1 patch) | no JNESS 0.24 / LUCAS 0.3 landing FSM or LUCAS 0.82 air speed multiplier in ft/ftchar/ftness/ftnessspecialhi.c
- nessshared.asm:288 | up_special_ending_delay_ | GAP (1 patch) | no JNESS ending delay 20 in ftNessSpecialHiInitStatusVars (ftnessspecialhi.c)
- nessshared.asm:308 | pkfire_ground_speed_1_ | GAP (2 patches) | ft/ftchar/ftness/ftness.h:15-21 picks 73/95 (REGION_US) for every Ness-kind fighter; JNESS/LUCAS (and Kirby copies) need 90/120 (asm 0x42B4 / 0x42F0)
- nessshared.asm:353 | pkfire_ground_speed_2_ | GAP (2 patches) | ft/ftchar/ftness/ftness.h:15-21 picks 73/95 (REGION_US) for every Ness-kind fighter; JNESS/LUCAS (and Kirby copies) need 90/120 (asm 0x42B4 / 0x42F0)
- nessshared.asm:399 | pkfire_air_speed_1_ | GAP (2 patches) | ft/ftchar/ftness/ftness.h:15-21 picks 73/95 (REGION_US) for every Ness-kind fighter; JNESS/LUCAS (and Kirby copies) need 90/120 (asm 0x42B4 / 0x42F0)
- nessshared.asm:444 | pkfire_air_speed_2_ | GAP (2 patches) | ft/ftchar/ftness/ftness.h:15-21 picks 73/95 (REGION_US) for every Ness-kind fighter; JNESS/LUCAS (and Kirby copies) need 90/120 (asm 0x42B4 / 0x42F0)
- nessshared.asm:489 | get_pillar_playerstruct_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:506 | pkfire_pillar_gravity_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:539 | get_pkthunder_anim_struct_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:572 | get_pkthunder_anim_struct2_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:611 | get_pkthunder_anim_struct3_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:645 | get_pkthunder_special_struct1_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:682 | get_pkthunder_special_struct2_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:717 | get_pkthunder_reflect_struct1_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:759 | get_pkthunder_reflect_struct2_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:796 | cpu_usp_usage_fix_ | AI (1 patch) | AI hook (Phase 6), not ported
- nessshared.asm:823 | cpu_usp_control_fix_ | AI (1 patch) | AI hook (Phase 6), not ported
- nessshared.asm:990 | get_pkthunder_playerstruct1_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:1008 | get_pkthunder_playerstruct2_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:1023 | get_pkthunder_playerstruct3_ | GAP (1 patch) | no JNESS/LUCAS (or Kirby-copy) selection in wp/wpness, it/itfighter/itnesspkfire.c or ef/efmanager.c; Ness weapon/effect descs keep &gFTNessFileSpecial1/2 (wpnesspkfire.c:14, wpnesspkthunder.c)
- nessshared.asm:1042 | reflect_fix_1_ | GAP (1 patch) | ft/ftcommon/ftcommonattacks4.c:55,105,123 and attackhi3/lw3 use nFTKindNess/NNess only: JNESS, LUCAS, NLUCAS get no fsmash reflector (special_coll never set)
- nessshared.asm:1073 | reflect_fix_2_ | GAP (1 patch) | ft/ftcommon/ftcommonattacks4.c:55,105,123 and attackhi3/lw3 use nFTKindNess/NNess only: JNESS, LUCAS, NLUCAS get no fsmash reflector (special_coll never set)
- nessshared.asm:1104 | reflect_fix_3_ | GAP (1 patch) | ft/ftcommon/ftcommonattacks4.c:55,105,123 and attackhi3/lw3 use nFTKindNess/NNess only: JNESS, LUCAS, NLUCAS get no fsmash reflector (special_coll never set)
- nessshared.asm:1134 | cpu_pk_thunder_attack_ | AI (1 patch) | AI hook (Phase 6), not ported
- nessshared.asm:1160 | improved_pk_thunder_ | AI (1 patch) | AI hook (Phase 6), not ported
- nessshared.asm:1246 | save_cpu_recovery_location_ | AI (1 patch) | AI hook (Phase 6), not ported
- pikashared.asm:199 | rapid_jab_fix_1 | GAP (1 patch) | ft/ftcommon/ftcommonattack1.c:55 and :108 test only Pikachu/NPikachu: EPIKA/JPIKA go to Attack12 (second jab) instead of the repeating jab
- pikashared.asm:221 | rapid_jab_fix_2 | GAP (1 patch) | ft/ftcommon/ftcommonattack1.c:55 and :108 test only Pikachu/NPikachu: EPIKA/JPIKA go to Attack12 (second jab) instead of the repeating jab
- pikashared.asm:243 | forward_smash_fix_1 | GAP (1 patch) | ft/ftcommon/ftcommonattacks4.c:19,99,115 test only Pikachu/NPikachu: no Thunder Shock effect/table for EPIKA/JPIKA fsmash
- pikashared.asm:265 | forward_smash_fix_2 | GAP (1 patch) | ft/ftcommon/ftcommonattacks4.c:19,99,115 test only Pikachu/NPikachu: no Thunder Shock effect/table for EPIKA/JPIKA fsmash
- pikashared.asm:287 | forward_smash_fix_3 | GAP (1 patch) | ft/ftcommon/ftcommonattacks4.c:19,99,115 test only Pikachu/NPikachu: no Thunder Shock effect/table for EPIKA/JPIKA fsmash
- pikashared.asm:310 | get_thunder_playerstruct1_ | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:340 | get_thunder_playerstruct2_ | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:358 | get_thunder_anim_struct_ | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:397 | get_thunder_jolt_anim_struct_ | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:431 | get_thunder_jolt_special_struct_1 | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:469 | get_thunder_jolt_special_struct_2 | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:500 | get_thunder_special_struct_1 | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- pikashared.asm:535 | get_thunder_special_struct_2 | GAP (1 patch) | no EPIKA/JPIKA file selection in wp/wppikachu, ef/efmanager.c:4662,4692 (Pikachu descs keep Pikachu's file globals)
- samusshared.asm:13 | get_bombinstructions_struct_ | PORTED (1 patch) | decomp/src/wp/wpsamus/wpsamusbomb.c:225 (ftRemixSamusSharedBombFile)
- samusshared.asm:50 | get_chargeinstructions_struct_ | PORTED (1 patch) | decomp/src/wp/wpsamus/wpsamuschargeshot.c:297 (Kirby copy handled); the MTWO struct is Mewtwo's own desc (ft/ftremix/mewtwo/ftremixmewtwonsp.c:285)
- samusshared.asm:99 | fully_charged_check_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:2181 (ftRemixSamusSharedChargeSparkleFkind)
- samusshared.asm:128 | kirby_power_check_flash_ | PORTED (1 patch) | decomp/src/ft/ftparam.c:1530 (ftRemixSamusSharedKirbyCopyChargeFlash); GFX ids >= 0x56 resolved by gmRemixColScriptsEnsureLoaded (ftparam.c:1423)
- samusshared.asm:209 | kirby_power_change_ | PORTED (1 patch) | decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c:93 (ftRemixSamusSharedKirbyCopyPowerFkind)
- samusshared.asm:242 | throw_ball_graphic | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommoncatch1.c:118
- samusshared.asm:268 | throw_ball_graphic_2 | PORTED (1 patch) | decomp/src/ft/ftcommon/ftcommonthrow.c:169
- samusshared.asm:291 | get_entry_anim_gfx_struct | ADAPTED (1 patch) | decomp/src/ft/ftremix/ftremixentrydata.c:45-46 (DSAMUS file 6, CRASH file 8)
- samusshared.asm:322 | get_grapple_anim_gfx_struct | PORTED (1 patch) | decomp/src/ef/efmanager.c:4781 -> ftRemixSamusSharedGrappleBeamDesc (ftremixsamusshared2.c:129)
- yoshishared.asm:9 | yoshi_dj_fix_1 | GAP (1 patch) | ft/ftcommon/ftcommonjumpaerial.c:154 tests only Yoshi/NYoshi: JYOSHI gets no Yoshi double-jump physics/knockback resist
- yoshishared.asm:32 | yoshi_shield_fix_1 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:54 | yoshi_shield_fix_2 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:76 | yoshi_shield_fix_3 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:98 | yoshi_shield_fix_4 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:121 | yoshi_shield_fix_5 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:143 | yoshi_shield_fix_6 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:165 | yoshi_shield_fix_7 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:187 | yoshi_shield_fix_8 | GAP (1 patch) | egg shield checks test only nFTKindYoshi (guard1.c:109,137,418,426,463; guard2.c:26,126; ftparam.c:2505): JYOSHI shields as a normal fighter
- yoshishared.asm:209 | yoshi_grab_fix_1 | GAP (1 patch) | ft/ftcommon/ftcommoncatch2.c:91 tests only Yoshi/NYoshi (tongue part 7,1)
- yoshishared.asm:231 | yoshi_grab_fix_2 | GAP (1 patch) | ft/ftcommon/ftcommoncapturewait.c:53 (victim invisible for Yoshi grabbers)
- yoshishared.asm:254 | yoshi_throw_fix_1 | GAP (1 patch) | ft/ftcommon/ftcommonthrown1.c:102 (ThrownSetStatusImmediate); asm adds JYOSHI and PIANO (Mad Piano throw)
- yoshishared.asm:279 | yoshi_cpu_fix_1 | AI (1 patch) | AI hook (Phase 6), not ported
- yoshishared.asm:310 | up_special_struct_fix | PORTED (1 patch) | decomp/src/wp/wpyoshi/wpyoshieggthrow.c:269 (ftRemixYoshiEggWeaponDesc)
- yoshishared.asm:341 | down_special_struct_fix | PORTED (1 patch) | decomp/src/wp/wpyoshi/wpyoshistar.c:183 (JYOSHI main file, DEDEDE file 7)

## FT: Fighters and AI.asm

Format: asm:line | scope | verdict | evidence (port file:line or reason).

AI.asm:22 | force_default_cpu_lvl_setup | PORTED | gm/gmremixspawnsetup.c:243 (called mn/mnplayers/mnplayersvs.c:7325)
AI.asm:57 | size_multiplier_check_ | PORTED | decomp/src/ft/ftremix/ftremixaisize.c:14, ftcomputer.c:4194
AI.asm:95 | evade_fix_ | STUB | decomp/src/ft/ftremix/ftremixaicombat.c:19, ftcomputer.c:3911; Remix 1P test missing (sc1PRemix1PIsActive now exists)
AI.asm:134 | recovery_fix_ | STUB | decomp/src/ft/ftremix/ftremixairecovery.c:34, ftcomputer.c:5346; Remix 1P test missing
AI.asm:183 | fix_target_pos_x | PORTED | decomp/src/ft/ftremix/ftremixairecovery.c:50, ftcomputer.c:5473
AI.asm:241 | fix_remix_recovery | PORTED | decomp/src/ft/ftremix/ftremixairecovery.c:114,127, ftcomputer.c:6835-6860
AI.asm:385 | custom_recovery_logic | GAP | dispatch ftcomputer.c:6863-6874 reads registry ai_recovery_routine, which no code ever assigns (see table)
AI.asm:444 | random_teching_ | STUB | decomp/src/ft/ftremix/ftremixaiteching.c:150, ftcommondamage.c:298; SBM block and sandbag miss count differ from asm
AI.asm:668 | z_cancel_ | PORTED | decomp/src/ft/ftremix/ftremixaiteching.c:190, ftcommonattackair.c:73
AI.asm:872 | charge_less_often | PORTED | decomp/src/ft/ftremix/ftremixaicombat.c:38, ftcomputer.c:6026
AI.asm:924 | improve_remix_specials | STUB | decomp/src/ftcomputer.c:6060-6100 (Samus family, DK, Bowser done); Sheik/Peppy needle charge is REMIX-TODO(AI) known
AI.asm:1034 | improve_remix_charged_NSP_1 | PORTED | decomp/src/ft/ftremix/ftremixaispecials.c:21, ftcomputer.c:6194
AI.asm:1131 | improve_remix_charged_NSP_2 | PORTED | decomp/src/ft/ftremix/ftremixaispecials.c:98, ftcomputer.c:7239
AI.asm:1181 | improve_remix_charged_NSP_3 | STUB | only the Kirby-hat half is tagged REMIX-TODO(AI); the own-fighter Sheik/Mewtwo/DSamus/JSamus/ESamus charge-state exit is also absent (ftcomputer.c:7219-7270)
AI.asm:1300 | remix_ignore_projectiles_ | PORTED | decomp/src/ft/ftremix/ftremixaicombat.c:51, ftcomputer.c:6576
AI.asm:1342 | usp_check_ | PORTED | decomp/src/ft/ftremix/ftremixaicloud.c:16, ftcomputer.c:3642,3657
AI.asm:1402 | recovery_stick_y_fix | PORTED | decomp/src/ft/ftremix/ftremixaicloud.c:83, ftcomputer.c:3754
AI.asm:2198 | extend_commands_1 | PORTED | decomp/src/ftcomputer.c:3765 + ft/ftremix/ftremixaicommands.c
AI.asm:2216 | extend_commands_2 | PORTED | decomp/src/ftcomputer.c:3765 + ft/ftremix/ftremixaicommands.c
AI.asm:2234 | extend_commands_3 | PORTED | decomp/src/ftcomputer.c:3765 + ft/ftremix/ftremixaicommands.c
AI.asm:2255 | extend_stick_x_commands | PORTED | decomp/src/ftcomputer.c:3557 ftRemixAiStickXFromCode
AI.asm:2474 | extend_routines | PORTED | decomp/src/ftcomputer.c:3743 ftRemixAiRunCustomCommand (jump table turned into a switch)
AI.asm:2503 | extend_polygon_check_ | GAP | ftcomputer.c:4405 switch(fkind) never sees Remix fighters; REMIX_SYM_Character__ai_aerial_chase__table is unused
AI.asm:2544 | extend_polygon_check_ | PORTED | decomp/src/ft/ftremix/ftremixailongrange.c:55, ftcomputer.c:7270
AI.asm:2569 | custom_move | PORTED | decomp/src/ft/ftremix/ftremixailongrange.c:105, ftcomputer.c:7328
AI.asm:3547 | custom_attacks_table | PORTED | decomp/src/ft/ftremix/ftremixaitables.c, ftcomputer.c:4109
AI.asm:3585 | auto_jab_grab_ | STUB | decomp/src/ft/ftcommon/ftcommoncatch1.c:200 REMIX-TODO(AI), known
AI.asm:3636 | check_invulnerable_ | PORTED | decomp/src/ft/ftremix/ftremixailv10.c:9, ftcomputer.c:3932
AI.asm:3663 | extend_cliff_attack_ | PORTED | decomp/src/ft/ftremix/ftremixailv10.c:19, ftcomputer.c:6240
AI.asm:3690 | taunt_replace_ | PORTED | decomp/src/ft/ftremix/ftremixailv10.c:33, ftcomputer.c:6544
AI.asm:3733 | cpu_post_process | STUB | decomp/src/ft/ftremix/ftremixaipostprocess.c:107 (generic parts ported; per-fighter table only Sheik; REMIX-TODO(AI) known)
AI.asm:3892 | force_objective_attack | PORTED | decomp/src/ft/ftremix/ftremixailv10.c:42, ftcomputer.c:8110
AI.asm:4007 | _fix_weird_air_drift_right | PORTED | decomp/src/ft/ftremix/ftremixailv10.c:59, ftcomputer.c:4727
AI.asm:4033 | _fix_weird_air_drift_left | PORTED | decomp/src/ft/ftremix/ftremixailv10.c:59, ftcomputer.c:4825
AI.asm:4337 | _improve_recovery_drift | GAP | no hook and no REMIX-TODO(AI) in code (only ROADMAP T8.2 / ai.md)
AI.asm:4799 | prevent_fastfall | STUB | decomp/src/ft/ftremix/ftremixaimovement.c:24; deviation: no-target case still applies the floor_dist test (asm goes straight to fastfall)
AI.asm:4869 | detect_offstage_opponents | PORTED | decomp/src/ft/ftremix/ftremixaimovement.c:97, ftcomputer.c:3841
AI.asm:4903 | clamp_target_pos_to_stage | STUB | decomp/src/ft/ftremix/ftremixaimovement.c:104, ftcomputer.c:3873; deliberate sense/bounds change vs asm (ai.md)
AI.asm:4987 | shorthop_followup_attack | PORTED | decomp/src/ft/ftremix/ftremixaimovement.c:42, ftcomputer.c:5525
AI.asm:5065 | jump_in_neutral | PORTED | decomp/src/ft/ftremix/ftremixaimovement.c:68, ftcomputer.c:5511
AI.asm:5136 | extend_attack_check_range | PORTED | decomp/src/ftcomputer.c:7497 ftRemixAiPredictionAttackViewBase
AI.asm:5158 | extend_attack_check_range_2 | PORTED | decomp/src/ftcomputer.c:7633 ftRemixAiPredictionAttackViewBase
AI.asm:5183 | fix_movement_prediction | PORTED | decomp/src/ft/ftremix/ftremixaiprediction.c, ftcomputer.c:4177
AI.asm:5448 | fix_multi_doublejump_usage | PORTED | decomp/src/ft/ftremix/ftremixairoll.c:25, ftcomputer.c:5332
AI.asm:5487 | weight_attack_options | PORTED | decomp/src/ft/ftremix/ftremixaiweight.c, ftcomputer.c:4360
AI.asm:5944 | skip_repeated_input_check | PORTED | decomp/src/ft/ftremix/ftremixairoll.c:36, ftcomputer.c:4518
AI.asm:5980 | skip_repeated_input_skip | PORTED | decomp/src/ft/ftremix/ftremixairoll.c:49, ftcomputer.c:4608
AI.asm:6016 | no_attack_when_all_odds_eq_zero | PORTED | port already returns FALSE for level 10 (ai.md; ftcomputer.c:4636)
AI.asm:6040 | stop_roll_spam_ | PORTED | decomp/src/ft/ftremix/ftremixairoll.c:100, ftcomputer.c:7458; Giga Bowser and level 10 ported; per-fighter branches NA (Character.close_quarter_combat.table patches AI.asm:6311-6359 are commented out)
AI.asm:6366 | shield_drop_ | PORTED | decomp/src/ft/ftremix/ftremixairoll.c:55, ftcomputer.c:5534
AI.asm:6534 | cpu_di_ | STUB | decomp/src/ftcomputer.c:7950 REMIX-TODO(AI), known
AI.asm:6823 | handle_right_arrow_press_ | STUB | decomp/src/mn/mnplayers/mnremixcpulevel.c:30 (CPU level to 10 done; human handicap to 40 is REMIX-TODO(AI), known)
AI.asm:6849 | render_right_arrow_ | STUB | decomp/src/mn/mnplayers/mnremixcpulevel.c:41 (same)
AI.asm:6877 | render_10_ | STUB | decomp/src/mn/mnplayers/mnremixcpulevel.c:80 (level 10 digits done; handicap icons above 9 REMIX-TODO(AI), known)
AI.asm:7105 | clamp_level_9_1 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7122 | clamp_level_9_2 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7140 | clamp_level_9_3 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7157 | clamp_level_9_4 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7174 | clamp_level_9_5 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7191 | clamp_level_9_6 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7208 | clamp_level_9_7 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7230 | clamp_level_9_8 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
AI.asm:7256 | clamp_level_9_9 | PORTED | decomp/src/ft/ftremix/ftremixailevel.c via FTCOMPUTER_LEVEL_CLAMPED/WALK_STOP_WAIT/COLL_SIZE_BASE (ftcomputer.c:39-41)
Bowser/BowserSpecial.asm:1061 | ledge_grab_patch_ | PORTED | decomp/src/ft/ftremix/ftremixchartables.c:64, ft/ftchar/ftyoshi/ftyoshispecialhi.c:142
Bowser/BowserSpecial.asm:1158 | skip_projectile_ | PORTED | decomp/src/ft/ftremix/ftremixchartables.c:73, ftyoshispeciallw.c:35
Bowser/bowser.asm:474 | thrown_mario_bros_fix_ | GAP | ftcommonthrown1.c:15 ProcUpdate has only the DK-cargo test, no Bowser/GBowser/NBowser skip
Bowser/bowser.asm:503 | bowser_nsp_recharge | GAP | no per-frame ammo regeneration (+2 per 30 frames outside NSP) in ft/ftremix/bowser/ftremixbowser.c or ftmain.c
Crash/Crash.asm:722 | dair_bounce_ | GAP | ftcommonattackair.c:17 ProcHit only handles Link family; no Crash/NCrash bounce
Crash/Crash.asm:762 | crash_eat_sfx | GAP | ftcommonget.c:50-135 heal-item cases have no Crash FGM 0x5BF
Crash/CrashSpecial.asm:912 | shield_hitlag_patch_ | PORTED | decomp/src/ft/ftremix/ftremixdeschooks.c:27 (shield_hitlag_skip)
Crash/CrashSpecial.asm:2159 | dig_gfx_main_ | PORTED | decomp/src/ft/ftremix/ftremixdeschooks.c:28 (ecb_override; scope is dig_ecb_patch_ in the asm)
DSamus/DsamusSpecial.asm:25 | initial_variable_override | PORTED | decomp/src/ft/ftremix/dsamus/ftremixdsamushooks.c:70, ftsamusspecialn.c:407
DSamus/DsamusSpecial.asm:51 | initial_variable_override_kirby | GAP | ftkirbycopysamusspecialn.c:367: no hook (Kirby with DSamus hat)
DSamus/DsamusSpecial.asm:77 | remove_shoot_flag_aerial_transition | PORTED | decomp/src/ftremixdsamushooks.c:81, ftsamusspecialn.c:137
DSamus/DsamusSpecial.asm:102 | remove_shoot_flag_aerial_transition_kirby | GAP | ftkirbycopysamusspecialn.c:111: no hook
DSamus/DsamusSpecial.asm:126 | check_aerial_charge | PORTED | decomp/src/ftremixdsamushooks.c:104, ftsamusspecialn.c:75
DSamus/DsamusSpecial.asm:173 | check_aerial_charge_kirby | GAP | ftkirbycopysamusspecialn.c (aerial start): no hook
DSamus/DsamusSpecial.asm:542 | charge_level_patch_ | STUB | decomp/src/wpsamuschargeshot.c:165,283 hook launch and MakeWeapon; wpSamusChargeShotProcUpdate (0x80168C64, line 204) still reads the vanilla attribute array for gfx_size
DSamus/DsamusSpecial.asm:820 | ground_x_physics_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:67
DSamus/DsamusSpecial.asm:846 | ground_y_velocity_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:134
DSamus/DsamusSpecial.asm:865 | air_x_physics_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:88
DSamus/DsamusSpecial.asm:887 | air_action_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:185
DSamus/DsamusSpecial.asm:908 | init_variables_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:154
DSamus/DsamusSpecial.asm:921 | air_to_ground_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:113
DSamus/DsamusSpecial.asm:937 | ground_to_air_patch_ | PORTED | decomp/src/ftsamusspeciallw.c:146
Dedede/Dedede.asm:745 | dedede_inhale_gfx_routine_ | GAP | ef/efmanager.c:6401,6460 use lr*800 on x; Dedede uses lr*1000 (dedede_gfx_), no hook
Dedede/Dedede.asm:807 | custom_initial_absorbed_timer_ | GAP | ftcommoncapturekirby.c:242 hardcodes 500; no Dedede 250 + victim damage
Dedede/DededeSpecial.asm:863 | inhale_begin_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:894 | inhale_begin_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1125 | ground_end_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1155 | air_end_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1185 | inhale_end_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1214 | inhale_end_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1245 | ground_pull_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1279 | air_pull_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1312 | inhale_pull_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1342 | inhale_pull_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1372 | ground_swallow_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1405 | air_swallow_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1438 | inhale_swallow_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1468 | inhale_swallow_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1498 | ground_idle_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1562 | inhale_idle_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1593 | inhale_idle_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:1623 | air_fall_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2230 | inhale_spit_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2260 | inhale_turn_to_fall_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2290 | inhale_spit_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2320 | ground_idle_to_turn_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2350 | ground_turn_to_idle_transition_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2380 | inhale_turn_ground_to_air_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Dedede/DededeSpecial.asm:2410 | inhale_turn_air_to_ground_ | PORTED | decomp/src/ft/ftremix/dedede/ftremixdededensp.c:463 table + ftmain.c:4982 ftRemixDededeMapKirbyInhaleStatus (one remap covers the 27 sites)
Falco/Falco.asm:244 | up_special_delay_ | PORTED | decomp/src/ftfoxspecialhi.c:14,155
Falco/Falco.asm:276 | up_special_velocity_1_ | PORTED | decomp/src/ftfoxspecialhi.c:26
Falco/Falco.asm:307 | up_special_velocity_2_ | PORTED | decomp/src/ftfoxspecialhi.c:26
Falco/Falco.asm:338 | up_special_velocity_3_ | PORTED | decomp/src/ftfoxspecialhi.c:26
Falco/Phantasm.asm:257 | set_variables_ | PORTED | decomp/src/ft/ftremix/falco/ftremixfalco.c (Phantasm state machine, header comment)
GBowser/gbowser.asm:450 | _remix_1p_prevent_bottom_blast_zone_KO | GAP | ftcommondead.c:690 1P enemy bottom bound KO has no Remix 1P Giga Bowser exemption (sc1PRemix1PIsFinalBoss unused here)
GBowser/gbowser.asm:543 | prevent_dash | GAP | ftcommondash.c:113 no CPU Giga Bowser Walk3 swap
Goemon/Goemon.asm:549 | heavy_item_hold_moveset | GAP | no use of REMIX_SYM_Goemon__HEAVY_ITEM_* (generated, unused)
Goemon/Goemon.asm:573 | heavy_item_hold_moveset_2 | GAP | no use of REMIX_SYM_Goemon__HEAVY_ITEM_* (generated, unused); no golden-hair model while carrying a heavy item
Goemon/Goemon.asm:595 | heavy_item_pickup_moveset | GAP | no use of REMIX_SYM_Goemon__HEAVY_ITEM_* (generated, unused); no golden-hair model while carrying a heavy item
Goemon/Goemon.asm:614 | heavy_item_turn_moveset | GAP | no use of REMIX_SYM_Goemon__HEAVY_ITEM_* (generated, unused); no golden-hair model while carrying a heavy item
Goemon/Goemon.asm:637 | heavy_item_turn_moveset_2 | GAP | no use of REMIX_SYM_Goemon__HEAVY_ITEM_* (generated, unused); no golden-hair model while carrying a heavy item
Goemon/GoemonSpecial.asm:2764 | item_hurtbox_patch_ | PORTED | decomp/src/ft/ftremix/ftremixchartables.c:78, it/itprocess.c:706
JDK/JDK.asm:44 | spinning_kong_startup_ | GAP | ftdonkeyspecialhi.c:115 uses FTDONKEY_SPINNINGKONG_AIR_VEL_Y (20.3, REGION_US) for JDK; asm gives 18.0
JDK/JDK.asm:80 | jdk_cargo_ | GAP | ftcommoncapture.c:149 uses 14.0 breakout base; JDK/Marina/NMarina carriers use 20.0 in the asm
JFalcon/JFalcon.asm:49 | up_special_aerial_drift_1_ | GAP | ftcaptainspecialhi.c:59 uses the US 1.1/0.8 factors for JFalcon; asm 1.2/0.84
JFalcon/JFalcon.asm:73 | up_special_aerial_drift_2_ | GAP | same as drift_1
JFox/JFox.asm:60 | get_laser_special_struct_ | GAP | wp/wpfox/wpfoxblaster.c:109 uses Fox laser weapon desc for JFox (own file_6 pointer in asm)
JPika/JPika.asm:56 | thunderjolt_duration_ | GAP | wppikachuthunderjolt.c:169 fixed WPPIKACHUJOLT_LIFETIME; JPika (and Kirby with JPika hat) 120 vs 100
JYoshi/JYoshi.asm:45 | joshi_armor | GAP | ftcommonjumpaerial.c:142 Yoshi branch tests nFTKindYoshi/NYoshi only; no JYoshi
JYoshi/JYoshi.asm:69 | joshi_egg_timer | GAP | ftcommoncaptureyoshi.c:353 fixed FTCOMMON_YOSHIEGG_ESCAPE_WAIT_MAX (250); JYoshi 210
JYoshi/JYoshi.asm:115 | joshi_mash | GAP | ftcommoncaptureyoshi.c:363 base inputs 750; JYoshi 630
Kirby/Kirby.asm:494 | (symbolic) | PORTED | decomp/src/ft/ftremix/dsamus/ftremixdsamusnsp.c:200 (aerial_b_button_buffer_kirby)
Kirby/Kirby.asm:653 | kirby_magic_hat | GAP | decomp/src/only the on/off flag exists (ft/ftremix/ftremixkirbypower.c:70); no random power roll or per-hat charge
Kirby/Kirby.asm:894 | restore_magic_hat_ | GAP | no restore of the magician hat after the special
Kirby/Kirby.asm:974 | clone_power_action_swap_ | PORTED | decomp/src/ftmain.c:4942 ftRemixKirbyCloneActionSwap
Lanky/LankySpecial.asm:954 | damage_patch_ | PORTED | decomp/src/ftcommondamage.c ftRemixLankyUpSpecialDamageStatus
Lanky/LankySpecial.asm:2142 | lanky_fighter_stance_bonus_ | PORTED | decomp/src/sc/sc1pmode/sc1pgame.c:3327 ftRemixLankyIsStanceBonus
Lucas/Lucas.asm:250 | pkfire_horizontal | GAP | ftpikachuspecialhi.c:561 / Kirby copy Ness: no Lucas branch (PK Fire stays horizontal in air)
Lucas/Lucas.asm:292 | pkfire_explosion | GAP | wpnesspkfire.c:71: no Lucas explosion gfx
Lucas/Lucas.asm:339 | usp_graphic_position_fix_ | GAP | efmanager.c:5302 only the Size hook; Lucas skip of the Y rotation not present (low impact)
Lucas/Lucas.asm:361 | usp_thunder_head_ | GAP | wpnesspkthunder.c:333: no head instance id for Lucas
Lucas/Lucas.asm:401 | usp_thunder_tail_ | GAP | wpnesspkthunder.c:432: same
Lucas/Lucas.asm:443 | usp_multi_hit_ | GAP | controlled head/tail (0xE5994, 0xE5F5C) have no hit-refresh; reflected hooks (AI: lines 454, 458) are commented out in the asm, so NA
Lucas/Lucas.asm:584 | dsp_graphic_position_fix_ | GAP | efmanager.c:5143 only the Size hook; Lucas 240 x translation missing
Lucas/Lucas.asm:626 | dsp_absorption_turnaround_prevent | GAP | ftnessspecialhi.c:828 sets lr for Lucas too
Lucas/Lucas.asm:648 | dsp_absorption_position_fix_ | GAP | ftnessspeciallw.c:240: no psi_magnet_struct_lucas
Lucas/Lucas.asm:672 | psi_magnet_multiplier_ | GAP | decomp/src/ft/ftremix/ftremixpianoshared.c:57 returns 2x for everyone but Piano; Lucas is 2.5x
MLuigi/MLuigi.asm:104 | initial_script_ | GAP | ftcomputer.c:3861 tests nFTKindMMario only (scope is metal_luigi_ai)
Marina/Marina.asm:734 | thrown_marina_patch_ | GAP | no DK_THROWN_MARINA / FALCON_THROWN_MARINA animation override in ftmain.c:4942 path
Marina/Marina.asm:805 | jump_height_patch_ | PORTED | decomp/src/ftcommonjumpaerial.c:142 Size hooks
Marina/Marina.asm:833 | jump_timing_patch_ | PORTED | decomp/src/ftcommonjumpaerial.c:341
Marina/MarinaSpecial.asm:168 | shake_check_ | PORTED | decomp/src/ftdonkeythrowff.c:117 ftRemixMarinaCargoShakeCheckInterrupt
Marina/MarinaSpecial.asm:206 | jump_patch_ | PORTED | decomp/src/ftdonkeythrowffall.c:63
Marina/MarinaSpecial.asm:233 | air_patch_ | PORTED | decomp/src/ftdonkeythrowffall.c:46
Marina/MarinaSpecial.asm:257 | jumpsquat_patch_ | PORTED | decomp/src/ftdonkeythrowfkneebend.c:58
Marina/MarinaSpecial.asm:280 | throw_f_patch_ | PORTED | decomp/src/ftcommoncapture.c:157 ftRemixMarinaCargoThrowFDamage
Marina/MarinaSpecial.asm:2349 | projectile_absorb_charge_ | PORTED | decomp/src/ftmain.c:3652 ftRemixMarinaDSPAbsorbProjectileCharge
Marina/MarinaSpecial.asm:2382 | item_absorb_charge_ | PORTED | decomp/src/ftmain.c:3871 ftRemixMarinaDSPAbsorbItemCharge
Marth/MarthSpecial.asm:1154 | detection_patch_ | PORTED | decomp/src/ftparam.c:1820 ftRemixMarthDSPCounterDamage
Mewtwo/Mewtwo.asm:517 | rapid_jab_patch_ | GAP | ftcommonattack1.c:170 has no Mewtwo rapid-jab start; FighterRapidJabStatusQueryEvent (ftcommonattack100.c:70) has no listener
Mewtwo/MewtwoSpecial.asm:1075 | charge_fgm_fix_ | PORTED | decomp/src/wp/wpsamus/wpsamuschargeshot.c:283 ftRemixSamusSharedChargeShotFile
Mewtwo/MewtwoSpecial.asm:1108 | shoot_velocity_patch_ | PORTED | decomp/src/ft/ftremix/mewtwo/ftremixmewtwonsp.c:721, ftsamusspecialn.c:238
Mewtwo/MewtwoSpecial.asm:1146 | kirby_shoot_velocity_patch_ | GAP | ftkirbycopysamusspecialn.c:219: no Mewtwo-hat velocity
Mewtwo/MewtwoSpecial.asm:1185 | ball_position_fix_ | STUB | decomp/src/ft/ftremix/mewtwo/ftremixmewtwonsp.c:708 for Mewtwo; Kirby branch of the asm (joint 0x960) missing
Peach/Peach.asm:579 | ledge_patch_ | PORTED | decomp/src/mp/mpcommon.c:485
Peach/Peach.asm:629 | restore_float_flag_ledge_action | PORTED | decomp/src/ftcommoncliffclimb.c:89
Peach/PeachSpecial.asm:88 | handle_physics_ | PORTED | decomp/src/ftphysics.c:256 ftRemixPeachFloatHoldsHeight
Peach/PeachSpecial.asm:132 | check_float_ | PORTED | decomp/src/ftcommonjump.c:18, ftcommonfall.c:18, ftcommonpass.c:18 (platform drop)
Peach/PeachSpecial.asm:241 | fall_override_ | PORTED | decomp/src/ftcommonfall.c:41
Peach/PeachSpecial.asm:284 | prevent_item_throw_ | PORTED | decomp/src/ftcommonattackair.c:125
Peach/PeachSpecial.asm:317 | auto_cancel_aerials_ | PORTED | decomp/src/ftcommonattackair.c:235
Peach/PeachSpecial.asm:350 | end_float_on_hit_ | PORTED | decomp/src/ftparam.c:435
Peach/PeachSpecial.asm:1035 | fall_special_patch_ | PORTED | decomp/src/ftcommonfallspecial.c:81 ftRemixPeachFallSpecialStatus
Peppy/Peppy.asm:342 | up_special_rotation_offset_air_ | GAP | ftfoxspecialhi.c:177 ftFoxSpecialHiUpdateModelPitch has no Peppy case (rotation 0)
Piano/Piano.asm:499 | extra_part_visibility_ | PORTED | decomp/src/ftmain.c:4942 ftRemixPianoExtraPartVisibility
Piano/PianoSpecial.asm:986 | ground_absorb_initial_patch_ | PORTED | decomp/src/ft/ftremix/piano/ftremixpianodsp.c
Piano/PianoSpecial.asm:1018 | air_absorb_initial_patch_ | PORTED | decomp/src/ft/ftremix/piano/ftremixpianodsp.c
Piano/PianoSpecial.asm:1050 | command_grab_action_fix_ | PORTED | decomp/src/ft/ftremix/ftremixpianoshared.c:37, ftcommoncaptureyoshi.c:121
Piano/PianoSpecial.asm:1099 | absorb_behaviour_ | PORTED | decomp/src/ft/ftremix/ftremixpianoshared.c:57, ftmain.c:2681
Sandbag/Sandbag.asm:158 | sandbag_land | GAP | no REMIX_ID_SANDBAG gameplay code (headstand landing action 0xE0, damage multiplier reset)
Sandbag/Sandbag.asm:230 | sandbag_getup_ | GAP | same
Sandbag/Sandbag.asm:252 | preserve_sandbag_momentum_ | GAP | ftmain.c:2006: knockback X decay not exempted for Sandbag
Slippy/Slippy.asm:303 | unique_jab_loop_ | GAP | ftcommonattack100.c:70: no Slippy jab 2 -> jab 1 loop
Slippy/Slippy.asm:329 | animate_part_patch_ | GAP | sys/controller.c:440 (animation part update) has no Y multiplier; no REMIX_SYM for hip_translation_table
Slippy/Slippy.asm:351 | apply_y_multiplier_ | GAP | same
Slippy/Slippy.asm:374 | hips_y_translation_ | GAP | ftparam.c:456 no Slippy hip multiplier
Slippy/Slippy.asm:420 | up_special_angle_patch_1_ | PORTED | decomp/src/ftfoxspecialhi.c:39 ftFoxSpecialHiGetStickY
Slippy/Slippy.asm:442 | up_special_angle_patch_2_ | PORTED | decomp/src/ftfoxspecialhi.c:39
Slippy/Slippy.asm:471 | up_special_angle_patch_3_ | PORTED | decomp/src/ftfoxspecialhi.c:39
Slippy/Slippy.asm:494 | up_special_duration_ | PORTED | decomp/src/ftfoxspecialhi.c:341
Slippy/Slippy.asm:516 | usp_collision_patch_ | PORTED | decomp/src/ftfoxspecialhi.c:258
Sonic/Sonic.asm:511 | handle_select_anim_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:546 | classic_fgm_patch_1_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:601 | classic_fgm_patch_2_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:640 | classic_voice_patch_1_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:672 | classic_voice_patch_2_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:704 | spin_dash_fgm_patch_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:740 | save_classic_flags_1p_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:761 | load_classic_flag_1p_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:782 | save_classic_flags_training_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:808 | load_classic_flags_training_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:835 | save_classic_flags_vs_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:853 | load_classic_flag_vs_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:870 | reset_classic_flag_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:1002 | load_classic_model_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:1036 | adjust_costume_comparison_ | PORTED | decomp/src/mn/mnplayers/mnremixsonicclassic.* + ft/ftremix/sonic/ftremixsonicclassic.c; scsubsysfighter.c:87; ftmain.c:179; ftparam.c:360
Sonic/Sonic.asm:1129 | sonic_facing_ | GAP | ftcommonentry.c:218 clears lr for every fighter; Sonic/SSonic/polygons keep it in the asm
Sonic/SonicSpecial.asm:1138 | destroyed_target_fix_ | PORTED | decomp/src/sys/objman.c:1914 ftRemixSonicNSPForgetTarget
Sonic/SonicSpecial.asm:1398 | action_check_patch_ | PORTED | decomp/src/ft/ftremix/sonic/ftremixsonicusp.c (spring bounce _check_action)
Sonic/SonicSpecial.asm:1432 | change_spring_room_ | GAP | ft/ftremix/sonic/ftremixsonicusp.c:661 spring uses default weapon link; no room 0x0B override (low impact)
Sonic/SonicSpecial.asm:3596 | teleport_cancel_patch_ | PORTED | decomp/src/ftmain.c:1438 ProcUpdateInterrupt hook
Wario/Wario.asm:319 | body_slam_recoil_ | GAP | no hit/shield/clang recoil trigger for Wario NSP, Sheik DSP attack, Banjo USP attack, Ebi NSP end, Kirby copies; Ebi/Kirby food items (Dango 99, Tomato 1, FGM 0x524) absent
Wario/Wario.asm:579 | body_slam_clang_1_ | GAP | ftmain.c:2366: no invincible-shoulder clang override
Wario/Wario.asm:591 | body_slam_clang_2_ | GAP | same
Wario/Wario.asm:603 | body_slam_clang_3_ | GAP | ftmain.c:2552: same
Wario/Wario.asm:615 | body_slam_clang_4_ | GAP | ftmain.c:2750: same
Wario/Wario.asm:640 | alternate_voice_ | GAP | ftparam.c:338: no FGM 0x2FB -> 0x300 swap while L is held
Wolf/Wolf.asm:391 | wolf_reflect_speed | PORTED | decomp/src/wp/wpprocess.c:468 ftRemixReflectWeapon / ftRemixJapanFixReflectMul
Wolf/Wolf.asm:436 | wolf_reflect_graphic | GAP | ef/efmanager.c:4211: no Wolf/Slippy reflect gfx struct
Wolf/Wolf.asm:554 | wolfen_entry | PORTED | decomp/src/efmanager.c:5963 ftRemixEntryGetArwingAnim
Wolf/Wolf.asm:584 | wolfen_entry_2 | PORTED | decomp/src/efmanager.c:5963
Wolf/Wolf.asm:617 | wolfen_entry_3 | PORTED | decomp/src/efmanager.c:5963
YoungLink/YoungLink.asm:186 | boomerang_speed_ | PORTED | decomp/src/wplinkboomerang.c:575
YoungLink/YoungLink.asm:260 | up_special_landing_ | PORTED | decomp/src/ftlinkspecialhi.c:234
YoungLink/YoungLinkSpecial.asm:626 | state_change_ | PORTED | decomp/src/itmain.c:635

## MD: Modes (12CB, Stamina, KOTH, Smashketball, Tag Team, Training)

Format: asm file:first line, `scope`, xN patches, verdict, evidence.

- KingOfTheHill.asm:304 `allow_3_digits_for_picker_` x1 - PORTED - mnplayersvs.c:2278 (3 digits, x via mnRemixGoalPickerNumberX) + mnremixgoalrules.c:67 (212 / 218 at >=100)
- KingOfTheHill.asm:1712 `ai_hill_behavior_` x1 - GAP - no CPU hill targeting in ftcomputer.c or ftremix/ (target = indicator pos, y-500, items/tornado ignored). Known open in f19:49
- KingOfTheHill.asm:1771 `delay_respawn_` x2 - PORTED - gmremixkoth.c:81 (120 frames), :86 (camera_mode Ghost at wait 75), hooks ftcommondead.c:238,265
- Smashketball.asm:477 `prevent_despawn` x1 - PORTED - itprocess.c:128 itRemixBasketballIsDespawnBlocked
- Smashketball.asm:502 `camera_follow` x1 - PORTED - gmcamera.c:475 -> gmremixsmashketballcamera.c:14 + gmRemixSmashketballRulesFollowBall (1000 padding, floor -1000)
- Smashketball.asm:1109 `play_end_buzzer_` x1 - PORTED - ifcommon.c:3056 queues FGM 0x431 (gmremixsmashketball.h:21)
- Smashketball.asm:1131 `force_chant_` x1 - PORTED - ftpublic.c:83/ftPublicTryStartCall is_forced; caller gmremixsmashketballscore.c:70 ftPublicRemixForceCall
- Smashketball.asm:1153 `ai_ball_behavior_` x1 - GAP - GAP: CPUs do not seek the ball (target = ball x +-400 by team). Known open (Phase 6 AI), f19:48
- Smashketball.asm:1211 `ai_ball_attack_range` x1 - GAP - GAP: attack-range prediction against the ball (hurtbox 200, ball vel/gravity) missing. Same as above
- Smashketball.asm:1268 `allow_overtime_` x1 - GAP - GAP: Remix keeps the main battle state in overtime; port copies TransferBattleState into VSBattleState (gmremixsmashketball.c:207), so the match stats are lost
- Smashketball.asm:1286 `no_initial_damage_` x1 - PORTED - scvsbattle.c:664 gmRemixSmashketballGetSuddenDeathDamage -> 0
- Smashketball.asm:1304 `prevent_match_stats_reset_` x1 - GAP - GAP: ftParamInitPlayerBattleStats (ftparam.c:202) has no Remix guard; overtime resets KOs/TKOs/damage
- Smashketball.asm:1339 `enable_toggling_mode_` x3 - ADAPTED - mnplayersvs.c:3226,3270 + mnremixcssmode.c:59. Minor: asm stores the blue channel (delay slot of jal) before skipping, port returns before blue (:3270); colour index uses is_team_battle, asm uses the type
- Smashketball.asm:1400 `use_goal_yellow_picker_` x1 - PORTED - mnplayersvs.c:2311 mnRemixGoalPickerImageOffset
- Stamina.asm:21 `stamina_display_` x1 - STUB - VS: ifcommon.c:719 ifRemixStaminaDisplayValue. STUB: the _remix branch (Remix 1P, Giga Bowser HUD shows HP 200/250/300 - damage) is not handled; ifcommon.c:706 only tests nFTKindBoss
- Stamina.asm:66 `stamina_dark_` x1 - PORTED - ifcommon.c:915 ifRemixStaminaDamageRatio (ifremixstaminahud.c:33)
- Stamina.asm:142 `stamina_dark_fix_2` x1 - ADAPTED - same ratio formula covers HP>300 (damage/HP); equivalent
- Stamina.asm:187 `stamina_knockback_` x1 - ADAPTED - single point ftRemixStaminaKnockbackDamageAdd in ftParamGetCommonKnockback (ftparam.c:1732), same as the throw patch that overrides every caller
- Stamina.asm:216 `stamina_knockback_item_` x1 - ADAPTED - same as stamina_knockback_
- Stamina.asm:274 `stamina_knockback_throw_` x1 - ADAPTED - ftparam.c:1732
- Stamina.asm:303 `stamina_knockback_projectile_` x1 - ADAPTED - same as stamina_knockback_
- Stamina.asm:383 `stamina_player_control_` x1 - ADAPTED - defeated fighter goes to Sleep (ftcommondead.c:254 ftRemixStaminaIsOutOfStock -> ftCommonSleepSetStatus), so no control
- Stamina.asm:434 `stamina_main_` x1 - PORTED - ftparam.c:1731 ftRemixStaminaStoredDamage -> ftremixstaminako.c:40 (KO once). Armor erase (0x7E4/0x7E8) not ported (comment in ftremixstaminako.c:23), passive part covered by stamina_armor_fix_
- Stamina.asm:515 `stamina_ko_action` x1 - STUB - ftmain.c:4984 ftRemixStaminaKOStatus (protected status lists match the asm). STUB: asm also sets a3 = anim speed 1.0 when it forces the KO action; port does not
- Stamina.asm:687 `stamina_shield_prevent` x1 - PORTED - ftcommonguard1.c:490 (tests percent>HP instead of stock==-1; same effect)
- Stamina.asm:747 `stamina_ko_prevent` x1 - PORTED - ftcommondead.c:254 / ftremixstaminadead.c
- Stamina.asm:787 `stamina_ko_prevent_front` x1 - PORTED - ftcommondead.c:528 ftRemixStaminaIsBlastRedeath
- Stamina.asm:825 `stamina_stock` x1 - PORTED - gmremixstockmode.c:128 gmRemixStockModeBeginMatch returns 0 in Stamina; scvsbattle.c:365
- Stamina.asm:853 `stamina_stock_2` x1 - PORTED - same
- Stamina.asm:889 `stamina_fix_1` x1 - ADAPTED - Remix encodes Stamina as game_rules 5; the port runs it as TIMED_STOCK + flag (gmremixstamina.c:23), so the "treat as stock/time" fixes are implicit
- Stamina.asm:908 `stamina_fix_2` x1 - ADAPTED - same (fix_2 also calls PokemonAnnouncer, other group)
- Stamina.asm:957 `stamina_fix_3` x1 - ADAPTED - same
- Stamina.asm:977 `stamina_fix_4` x1 - ADAPTED - same
- Stamina.asm:997 `stamina_fix_5` x1 - ADAPTED - same
- Stamina.asm:1017 `stamina_fix_6` x1 - ADAPTED - same; ftcommondead.c:158 ftRemixStaminaRebirthSleeps
- Stamina.asm:1037 `stamina_fix_7` x1 - PORTED - ftremixstaminadead.c:49 (star KO on last stock stays -1 in the fighter, -2 in match data)
- Stamina.asm:1069 `stamina_down_wait_down` x1 - PORTED - ftcommondownwaitbounce.c:34
- Stamina.asm:1105 `stamina_down_wait_input` x1 - PORTED - ftcommondownwaitbounce.c:46
- Stamina.asm:1141 `stamina_down_bounce_input` x1 - PORTED - ftcommondownwaitbounce.c:92
- Stamina.asm:1177 `stamina_ledge` x1 - PORTED - ftcommoncliffcatchwait.c:48 ftRemixStaminaLedgeBlocked
- Stamina.asm:1222 `stamina_percent_prevent` x1 - ADAPTED - ftRemixStaminaStoredDamage caps the fighter percent at HP+1 (the asm caps only the battle-state copy used by the HUD); same visible result
- Stamina.asm:1287 `percent_port_refresh` x1 - PORTED - gmRemixStaminaResetDefeated at match start (gmremixstamina.h:37)
- Stamina.asm:1307 `stamina_invis_prevent` x1 - PORTED - ftcommondead.c:274 ftRemixStaminaSkipsInvisible (action ranges checked against the asm loop)
- Stamina.asm:1365 `action_hit_crash_prevent` x1 - GAP - GAP: ftParamStopVoiceRunProcDamage (ftparam.c:429) lacks (1) the reset of AirDodge.air_dash_player_port_array on every hit (all modes; ftRemixAirDodgeResetAirDash is only called from mpcommon.c:452 and ftmanager.c:782, so Air Dash is not refreshed by a hit, toggles.md) and (2) the clear of dead.wait (0xB18) of a defeated Stamina fighter
- Stamina.asm:1430 `egg_crash_prevent` x1 - PORTED - ftcommondead.c:261 ftRemixStaminaSkipsDeadWaitInit
- Stamina.asm:1516 `overkill_prevent` x3 - ADAPTED - HUD half: ftCommonDeadInitStatusVars early return (ftcommondead.c:254) stops a second KO; menu halves: mnvsmode.c nMNVSModeRuleLast (mndef.h:176) and arrows :517,:531
- Stamina.asm:1578 `add_to_rule_button_` x10 - PORTED - mnvsmode.c:369,1184-1262,1342-1356; mnremixvsstamina.c:110 (positions 176/165/224, scale 0.875)
- Stamina.asm:1700 `add_to_stock_time_button_` x5 - PORTED - mnvsmode.c:728 mnRemixVSStaminaMakeTimeStockLabels
- Stamina.asm:1780 `add_to_stock_time_value_` x6 - STUB - value/position ported (mnremixvsstamina.c:132). STUB: scroll rules differ. Asm: D-pad/L/R = +-1, stick = +-10 with snaps (1<->10, 990, 999) in the VS menu; port mnRemixVSStaminaStepHP uses +-1 below 10 and above 990, +-10 else, for every input (:68-90)
- Stamina.asm:1992 `stamina_scrolling_` x2 - STUB - STUB: CSS HP scroll (mnplayersvs.c:5284,5320 -> mnRemixVSStaminaScrollHP) clamps instead of wrapping (asm wraps 999->1, 1->999, steps 1->10->20) and does not call CharacterSelectDebugMenu.Damage.restrict_damage_for_stamina_ (initial damage above the new HP stays; modes/stamina.md admits it)
- Stamina.asm:2066 `stamina_hp_display_` x2 - PORTED - mnplayersvs.c:2338 mnRemixVSStaminaMakeCSSValue (H.P. background via goal picker image offset)
- TagTeam.asm:694 `use_character_queues_on_load_` x3 - ADAPTED - gmRemixTagTeamApplyStartSlot / BuildQueues (gmremixtagteam.c, f20 table); scvsbattle.c:276
- TagTeam.asm:988 `load_next_char_` x1 - PORTED - ftcommondead.c:650 swap, gmRemixTagTeamGetSlotOfPort
- TagTeam.asm:1505 `load_all_files_` x1 - NA - N64 heap slots (always_load/alt_heap_pointer) - the port loads files per kind once (modes/tagteam.md)
- TagTeam.asm:1548 `load_particle_bank_stuff_in_normal_heap` x2 - NA - N64 custom heap slots only
- TagTeam.asm:1589 `use_largest_anim_heap_` x1 - ADAPTED - gmRemixTagTeamGetFigatreeHeap (one largest-size heap per port)
- TagTeam.asm:1618 `show_all_icons_` x2 - ADAPTED - gmremixtagteamicons.c + ifcommon.c:1172 gmRemixTagTeamApplyStockIcon
- TagTeam.asm:1747 `handle_stock_stealing_` x1 - ADAPTED - ifcommon.c:1481 gmRemixTagTeamStealStock
- TagTeam.asm:1819 `update_queue_` x10 - ADAPTED - mnPlayersVSTagTeamWriteSlot at mnplayersvs.c:336,3859,4897; toggle FFA/teams/team change (6 patches) have no write: the CSS icon costume is derived at draw time (mnremixtagteamcssicons.c:97) and the match recomputes team costumes at start - equivalent, low confidence
- TagTeam.asm:2365 `move_to_next_slot_on_pickup` x1 - PORTED - mnplayersvs.c:4517
- TagTeam.asm:2400 `clear_slot_and_go_to_previous_slot_` x1 - PORTED - mnplayersvs.c:5197-5228
- TagTeam.asm:2512 `adjust_current_slot_on_stock_count_change_` x1 - PORTED - mnplayersvs.c:2356
- TagTeam.asm:2713 `speed_up_token_pickup_` x1 - PORTED - mnplayersvs.c:4403
- Training.asm:142 `load_character_` x1 - ADAPTED - sc1ptrainingremix.c SetupPorts (per-port registry), sc1ptrainingmode.c:868
- Training.asm:400 `init_character_` x1 - ADAPTED - ftRemixPanelGetInitialDamage + sc1PTrainingRemixSpawnApplyDesc (:2262); KO respawn correctly untouched (modes/training.md)
- Training.asm:468 `set_action_` x1 - ADAPTED - sc1ptrainingmode.c:2279 sc1PTrainingRemixSpawnEnterLedge
- Training.asm:507 `obey_cp_state_` x1 - ADAPTED - sc1PTrainingModeUpdateDummyBehavior over every CPU (modes/training.md)
- Training.asm:550 `disable_during_pause_` x1 - ADAPTED - sc1PTrainingModeSetPortsLocked
- Training.asm:581 `enable_during_unpause_` x1 - ADAPTED - same
- Training.asm:627 `check_enter_menu_` x1 - ADAPTED - Z menu in sc1ptrainingremixmenu*.c (sc1ptrainingmode.c:768)
- Training.asm:674 `check_leave_menu_` x1 - ADAPTED - same
- Training.asm:720 `check_disabled_item_spawn_` x1 - ADAPTED - owner port logic in the port Z menu/pause (not read line by line; low confidence)
- Training.asm:755 `hold_to_exit_` x2 - PORTED - sc1ptrainingremixexit.c, sc1ptrainingmode.c:655,705
- Training.asm:814 `load_from_sss_` x1 - ADAPTED - sc1ptrainingmode.c:828 sc1PTrainingRemixStartScene
- Training.asm:866 `load_from_reset_` x1 - ADAPTED - sc1ptrainingmode.c:637,749 sc1PTrainingRemixOnReset
- Training.asm:951 `disable_hud_` x2 - PORTED - sc1ptrainingmode.c:376,2308 ifRemixHudDisabledAll
- Training.asm:1044 `advance_frame_` x1 - PORTED - sc1ptrainingremix.c:87 PollDpad (DL reset, DU freeze, DR advance, DD model display)
- Training.asm:1340 `run_after_object_routines_` x1 - PORTED - sc1ptrainingmode.c:803 sc1PTrainingRemixActionCountFrame
- Training.asm:2282 `play_custom_music_` x1 - PORTED - sc1ptrainingremix.c:130,142 (play_bgm_ port)
- Training.asm:2314 `shield_break_mode_` x1 - PORTED - ftmain.c:1438 ftremixtraining.c (CPU only, documented)
- Training.asm:2497 `spam_practice_` x1 - PORTED - ftRemixTraining spam in ftcomputer.c:7950 area (seen: jab every 40 frames, modes/training.md)
- Training.asm:2646 `fix_sector_z_crashes_` x1 - ADAPTED - grsector.c:901 has no NULL guard, but sc1ptrainingremix.c:244 sets pl_count/cp_count to the loaded fighters, so random never exceeds the list (low confidence)
- Training.asm:2668 `fix_pokemon_crashes_` x4 - PORTED - itsawamura.c, itstarmie.c, itkamex.c:~410 owner fallback + victim_gobj = NULL init
- Training.asm:2732 `skip_crowd_cheer_` x3 - PORTED - sc1ptrainingmode.c:2317, sc1pbonusstage.c:1338, sc1pgame.c:2894 (sc1premixmode.c:55)
- Training.asm:2804 `handle_reset_menu_selections_` x2 - ADAPTED - sc1ptrainingmode.c:873 sc1PTrainingRemixMenuIsPreserved
- Training.asm:3863 `show_stage_bg_` x2 - STUB - bg half ported (grWallpaperMakeDecideKind, modes/training.md). STUB: the magnifying-glass background of the asm (8018DEA4, stage bg address t6+0x4C) is not done (note admits it)
- TugOfWar.asm:283 `credit_stock_to_koer_` x1 - PORTED - gmremixtugofwar.c gmRemixTugOfWarCreditKO from ftcommondead.c:100, rules in gmremixtugofwarrules.c; deviations (last_hit read by port not team, teammate shield break, stock cap 127) documented in docs/bugs/remix_tugofwar_asm_deviations_2026-11-05.md, confirmed against the asm
- TugOfWar.asm:365 `credit_stock_when_no_koer_` x1 - PORTED - same function (no-KOer branch)
- TugOfWar.asm:434 `update_last_hit_by_` x2 - PORTED - ftcommondamage.c:716 RecordDamage, ftcommonshieldbreakfly.c:65 RecordShieldBreak
- TwelveCharBattle.asm:453 `setup_ports_and_characters_` x1 - STUB - STUB: 3P/4P N/A done (mnplayersvs.c:7354). Missing: at CSS entry a defeated human fighter is cleared to NONE (or moved to a valid portrait of the same character), costume legalised, character invalid for the port cleared. Port keeps it selected and only blocks START (gmRemixTwelveCBCheckSlot). mnplayersvs.c:7159 "hooked" is a false positive
- TwelveCharBattle.asm:593 `ignore_ports_3_and_4_` x1 - ADAPTED - slots 3/4 closed and hidden (mnplayersvs.c:7354, :6886)
- TwelveCharBattle.asm:620 `skip_rendering_panels_3_and_4_` x3 - ADAPTED - mnplayersvs.c:6886 mnPlayersVSTwelveCBFrameUpdate, :6852
- TwelveCharBattle.asm:696 `change_2p_panel_position_` x19 - PORTED - mnplayersvs.c:129 mnPlayersVSGetPanelIndex used at all 19 sites (lines 1226-6191)
- TwelveCharBattle.asm:1665 `force_ffa_and_stock_` x1 - ADAPTED - gmremixmodecss.c profile (FFA + stock) + gmremixtwelvecb.c BeginMatch; portrait restore in port state
- TwelveCharBattle.asm:1748 `update_announcer_on_entry_` x2 - GAP - GAP: when the battle is COMPLETE the asm plays PLAYER_1/PLAYER_2 (FGM 503/504) and WINS (533) at CSS frame 0x34; mnremixcssmode.c:36 returns FALSE and the vanilla voice plays (ROADMAP Phase 5 deviation, real gap). 1016 announcer for the unfinished battle is ported
- TwelveCharBattle.asm:1867 `update_css_header_` x2 - PORTED - mnremixcssmode.c:30 title offset; mnvsresults.c:2589; "Game N" gmremixtwelvecbhud.c
- TwelveCharBattle.asm:1958 `handle_stock_count_change_` x2 - STUB - STUB: 12CB part ported (mnplayersvs.c:944-957, lock after start). Missing: Tag Team stock range 2..6 and Tug of War 1..25 (_squad_strike_/_tug_of_war_ wrap); port arrows (mnplayersvs.c:5292,5328) always allow 1..99
- TwelveCharBattle.asm:2090 `handle_stock_change_fgm_` x2 - PORTED - mnplayersvs.c:952 nSYAudioFGMMenuDenied (FGM 165)
- TwelveCharBattle.asm:2654 `handle_custom_presses_` x1 - PORTED - mnplayersvs.c:1032 mnPlayersVSTwelveCBApplyCustomInput (+ mnremixtwelvecbcustom*.c)
- TwelveCharBattle.asm:2861 `prevent_defeated_char_select_` x1 - PORTED - mnplayersvs.c:4597 IsSlotSelectable + winner lock
- TwelveCharBattle.asm:2936 `defeated_action_override_` x1 - GAP - GAP (12CB part): hovering a defeated portrait shows the fighter in the DownStandU defeated pose (defeated_array :3198); port returns nFTKindNull for it (mnplayersvs.c:4650) and only darkens the card. Sonic classic / Remix 1P branches belong to other groups
- TwelveCharBattle.asm:3303 `prevent_token_pickup_` x1 - GAP - GAP: a human may pick up the token of a CPU that won the last game; the asm plays FGM ILLEGAL and refuses. mnPlayersVSSetCursorGrab (mnplayersvs.c:4514) has no 12CB check
- TwelveCharBattle.asm:3381 `set_initial_stock_count_` x1 - PORTED - scvsbattle.c:365 -> gmRemixStockModeBeginMatch -> gmRemixTwelveCBGetStocks
- TwelveCharBattle.asm:3425 `treat_selected_defeated_chars_as_unselected_` x2 - STUB - mnplayersvs.c:6539 gmRemixTwelveCBCheckSlot. STUB: asm also refuses START when a player has 0 stocks_remaining (battle complete, RESET required); port returns TRUE when complete (gmremixtwelvecb.c:213) and then auto-resets (:167)
- TwelveCharBattle.asm:3525 `gentlemens_reset_` x3 - PORTED - ifcommon.c:3256,3442,3504 ifremixgentlemensreset.c
- TwelveCharBattle.asm:4536 `portrait_change_` x1 - ADAPTED - per-frame portrait tracking mnplayersvs.c:6900 (GetPortrait), no char-id compare needed
- TwelveCharBattle.asm:5350 `update_stocks_remaining_` x2 - STUB - 12CB: ftcommondead.c:111 gmRemixTwelveCBOnKo. STUB: non-12CB path writes StockMode.previous_stock_count_table[port] = new stock count at every KO (Stock Mode Last); nothing in the port does (gmRemixStockModeSetLastCount only from Tug of War). Last therefore never follows lost stocks; salty-runback restore is a no-op
- VsStats.asm:79 `(unscoped pad)` x1 - NA - object heap pad for the stats text (N64 heap size constant)
- VsStats.asm:216 `increment_special_tracker` x6 - PORTED - 6/6: ftcommonspecialair.c:217,234,253; ftcommonspecialhi.c:77; ftcommonspecialn.c:165; ftcommonspeciallw.c:75
- VsStats.asm:875 `prevent_timed_row_flash_` x5 - ADAPTED - mnremixvsstatsview.c:70 hides group 0x16 while the page shows (rows are complete before the prompt appears, so no per-row call needed)
- resultsscreen.asm:12 `load_character_files_` x1 - NA - N64 memory: loads only the match characters instead of all; the port loads files on demand
- resultsscreen.asm:43 `vs_record_fix_` x2 - PORTED - mnvsresults.c:253,:277 skip fkinds >= 12 as the asm does (ids >= BOSS skipped)
- resultsscreen.asm:103 `fix_port_index_` x1 - PORTED - mnvsresults.c:1096 desc.player = player
- resultsscreen.asm:117 `get_winner_fgm_` x1 - STUB - STUB: port uses port_fighter_results_announce_fgm (mnvsresults.c:381), a descriptor copied from the parent (remix_roster.cpp:325); Character.winner_fgm.table is not read
- resultsscreen.asm:136 `winner_logo_fix_` x1 - GAP - GAP: no spinning series emblem for fkinds >= nFTKindEnumCount (mnvsresults.c:714; nothing sets results_emblem_valid, fighter_registry.cpp:395). Also fkinds 13..26 (Metal Mario, polygons, Giant DK) index the 12-entry arrays out of bounds (:668-702)
- resultsscreen.asm:168 `winner_logo_zoom_fix_` x1 - GAP - same as winner_logo_fix_
- resultsscreen.asm:200 `winner_logo_color_fix_` x1 - GAP - same as winner_logo_fix_
- resultsscreen.asm:236 `label_height_fix_` x3 - NA - US table pos_y_kinds is all zeros (mnvsresults.c:1126), so the id substitution has no effect
- resultsscreen.asm:268 `get_str_wins_lx_` x1 - GAP - GAP: per-character x from Character.str_wins_lx.table and the singular "WIN!" for J sound type and Banjo (Dragon King keeps WINS). Port: constant 175 for every synth (remix_roster.cpp:268) and always "WINS!" (mnvsresults.c:1425)
- resultsscreen.asm:306 `get_str_winner_info_` x1 - STUB - STUB: name = internal roster id ("GND", "YLINK", "DRM", "DSAMUS", "ELINK"...), lx 30, scale 1.0/0.6 by length > 7, all invented (remix_roster.cpp:262-269); asm reads Character.str_winner_ptr/lx/scale tables
- resultsscreen.asm:334 `add_ampersand_` x3 - GAP - GAP: no "&" glyph (index 0x1D, width 0x4208 = 34, texture 0x8348+0x10); mnVSResultsGetCharacterID (mnvsresults.c:1253) and the width table (28 entries, :1279) lack it (needed by "BANJO & KAZOOIE" style names; lower-case/odd chars would index out of range)
- resultsscreen.asm:388 `get_victory_bgm_` x1 - PORTED - mnvsresults.c:3396 mnRemixResultsGetWinBGM (mnremixwinbgm.c)
- resultsscreen.asm:419 `victory_animations_` x2 - GAP - GAP: the winner picks the victory animation with the C buttons (CR=3, CL=2, CU/CD=1 on the winning port, Joypad held mask); mnVSResultsGetStatusWin (mnvsresults.c:956) is random only and does not receive the port

## MS: Misc systems (boot, BGM, FGM, gallery, pause, VS demo)

Format: asm:lines | scope | verdict | evidence (port file:line or reason). Rows with the same scope and verdict are merged.

- AA.asm:17,23,29 `disable_aa_` | ADAPTED | port/remix/remix_display_settings.cpp:28 forces MSAA level 1 while REMIX_SYM_Toggles__entry_disable_aa is set (the VI AA bits do not exist in the GL renderer)
- BGM.asm:37 `master_bgm_volume` | PORTED | decomp/src/sys/audio.c:1719 + gm/gmremixmusic.c:9 (scale by level/10 before the 30720 clamp)
- BGM.asm:65 `apply_alt_or_random_music_` | STUB | gr/grremix/grremixstage.c:256 grRemixGetStageBGM, called only for scene VSBattle (mp/mpcollision.c:4147). Missing: C-button forcing of the alt tracks and random_disabled, Training/1P/Bonus/AutoDemo scenes, demo FD-intro skip, custom MIDIs in the random pool (grremixrandommusic.c:3). Alt chances 65/16/10/9 and Stages.default_music_track_table are ported
- BGM.asm:337 `play_music_` | PORTED | sys/audio.c:1660 (Play Music off -> stop). The FD.music_fix_.timer_start store has no reader (see FD.asm:59)
- BGM.asm:501,506,511,516,521,527,532,537,542,549,553,557,561 `alt_menu_music_` | PORTED | gm/gmremixmenumusic.c (9 play sites: mnmodeselect.c:1004, mn1pmode.c:1033, mnoption.c:1161, mndata.c:885, mnvsmode.c:1916, mnplayersvs.c:7489, mnplayers1pgame.c:4004, mnplayers1ptraining.c:3663, mnplayers1pbonus.c:3422; 4 stop sites mnplayersvs.c:4989, 1pgame:2973, 1ptraining:2350, 1pbonus:2428). Not ported: MAIN_TRAP_SM64 variant of FILESELECT_SM64 (Toggles.itsatrap, BGM.asm:696-700), see GAP list
- BGM.asm:1003,1008 `show_music_title_` | PORTED | gm/gmremixmusictitle.c, if/ifcommon.c:2559 (show) and :2296 (hide)
- BGM.asm:1252 `thirty_second_music_swap` | GAP | if/ifcommon.c:2880 keeps the vanilla gkind==Inishie check; the asm swaps by playing track (MK->MK Fast, SMB2 overworld->SMB2 Boss 0x124, Underground->Underground Hurry)
- BGM.asm:1355 `remix_1P_ending_song` | PORTED | sc/sccommon/scstaffroll.c:2321 + sc/sc1pmode/sc1premixending.c:22 (reads Character.remix_1p_end_bgm.table)
- BGM.asm:1407 `prevent_results_bgm_on_css_` | NA | the results audio wait is a GObj thread of the VS results scene (mn/mnvsmode/mnvsresults.c:3079); the scene change ejects it, there is no screen_interrupt race to guard (medium confidence)
- BGM.asm:1419 `set_safe_id_on_play_` | NA | BGM.safe_id works around a race of the N64 audio thread on 0x8009D974; the port reads syAudioGetBGMPlaying (sys/audio.c:1707)
- BGM.asm:1433 `set_safe_id_on_stop_` | NA | same as set_safe_id_on_play_ (syAudioStopBGMAll resets the playing id)
- Boot.asm:18 `(unnamed)` | GAP | N64 logo -> splash chain; mn/mncommon/mnstartup.c:168 goes straight to the title
- Boot.asm:37,42,47,53 `splash_` | GAP | Remix splash image (Character.id.NONE on the Congratulations scene, SinglePlayer.asm:2441-2511, files SPLASH_IMAGE_BOTTOM/TOP), 0x200-frame timer and jingle are not in the port (mnstartup.c has no REGION_REMIX code)
- Boot.asm:173,180 `draw_version_on_title_screen_` | PORTED | mn/mncommon/mntitle.c:83-88 (larger logo rows 0x245C8/0x3DC88) and mn/mncommon/mnremixtitleversion.c:10 (Smash Remix v2.0.1)
- Boot.asm:188 `draw_version_on_title_screen_` | GAP | the N64 logo is skippable in the port (mnstartup.c); the asm makes it unskippable. Cosmetic, tied to the splash (low)
- Boot.asm:196,201 `load_` | NA | boot DMA of the Remix image to 0x80400000 and Toggles.load_: the port reads the image through port/remix/remix_ram_image.cpp and keeps toggles in CVars (remix_toggles.cpp:116)
- ChargeSmashAttacks.asm:47 `smash_attack_main` | PORTED | ft/ftmain.c:5640 ftRemixChargeSmashLogic, ft/ftremix/ftremixchargesmash.c
- ChargeSmashAttacks.asm:298 `custom_shuffle` | PORTED | ft/ftdisplaymain.c:1398 ftRemixChargeSmashShufflePosition (table (8,0),(0,8),(-8,0),(0,-8) equals asm CUSTOM_SHUFFLE_TABLE)
- ChargeSmashAttacks.asm:338 `custom_shuffle_item` | PORTED | lb/lbcommon.c:1913
- Cheats.asm:14 `(unnamed)` | PORTED | lb/scmanager default save: sc/scmanager.c:342 (0x7F / 0x0C90)
- Cheats.asm:20,26,32,38 `(unnamed)` | NA | every instruction of this patch is commented out in Cheats.asm (lines 19-37: `// ori`, `// slti`, `// nop`), so the patch writes nothing
- Combo.asm:23 `improve_meter_` | PORTED | ft/ftremix/ftremixcombo.c:62, hook ft/ftmain.c:1771
- ComboMeter.asm:851 `hide_for_score_sprite_` | PORTED | if/ifcommon.c:3730, if/ifremixcombometer.c:448
- Credits.asm:15 `(unnamed)` | PORTED | sc/sccommon/scstaffroll.c:195 (first Remix section index 84 = 0x54)
- Credits.asm:20 `(unnamed)` | PORTED | scstaffroll.c:1523 (fast speed 0.11)
- Credits.asm:835,841,858 `load_files_` | ADAPTED | extended tables are generated by tools/gen_remix_credits.py into decomp/src/credits/remix/*.encoded and included at scstaffroll.c:24-260
- Credits.asm:865 `use_extended_section_info_table_` | ADAPTED | same (credits/remix/*.credits.*)
- Credits.asm:893 `use_extended_title_info_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:917,921 `use_extended_title_strings_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:962,966 `use_extended_name_info_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1003,1007 `use_extended_name_strings_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1048 `use_extended_details_info_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1071 `use_extended_details_strings_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1094 `use_extended_company_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1121 `use_extended_company_info_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1142 `use_extended_company_strings_table_` | ADAPTED | same (credits/remix/*.credits.*, scstaffroll.c:208-372)
- Credits.asm:1166,1171 `use_added_characters_` | PORTED | glyph ids 0x50-0x61, scstaffroll.c:440
- Credits.asm:1238,1242,1246 `use_added_characters_` | GAP | awayMode (Credits.asm:1237-1300) auto-shoot of names, fgm and gfx suppression for the `afk` tag is not in scstaffroll.c (only sc1PRemixAfkIsSpeedAssistActive, :1475)
- Credits.asm:1311 `remix_credits_speed_assist_` | PORTED | scstaffroll.c:1461-1476 + sc1premixenemyafkrules.c
- DI.asm:27 `apply_di_modifiers` | PORTED | ft/ftremix/ftremixdi.c:27
- DKMode.asm:17 `_resize_head` | PORTED | ft/ftremix/ftremixdkmode.c (variable kept in the port because the RAM image is read-only)
- FD.asm:18 `fix_` | ADAPTED | the port keeps the sc1PGameBossInitWallpaper call in gr/grwallpaper.c:405 and handles VS inside it (sc1pgameboss.c:1057 sets bossplayer = 0 in VS) instead of skipping it
- FD.asm:59 `music_fix_` | STUB | sc/sccommon/scvsbattle.c:48-55 scVSBattlePortFixupFDMusic forces the FD theme at once (no 840-frame intro) for gkind==nGRKindLast, overriding Remix alt/random music; FD DL/Tentination (clones 0x8F/0x90, file default track 24 = intro) have no fix and the stage track goes silent after the intro
- FGM.asm:17 `augment_sound_test_voice_` | PORTED | mn/mndata/mnremixsoundtest.c:41, mnsoundtest.c:807-812
- FGM.asm:48 `sound_test_R` | PORTED | mnsoundtest.c:949-950 (R held = +50)
- FGM.asm:68 `sound_test_L` | PORTED | mnsoundtest.c:894-895 (L held = -50)
- FGM.asm:97,102 `afk_soundTest` | GAP | `afk` tag auto-advance of the sound test every 7 minutes (FGM.asm:97-190) is not in mnsoundtest.c
- FGM.asm:194 `master_fgm_volume` | PORTED | sys/audio.c:1846 + gm/gmremixmusic.c:9
- FGM.asm:957 `master_fgm_volume` | PORTED | mnsoundtest.c:812 MNSOUNDTEST_NUMBER_DIGITS 4
- FGM.asm:966 `fix_0x2b7_` | GAP | libultra/n_audio/n_env.c:5335 func_800269C0_275C0 only rejects id >= fgm_ucode_count; in the Remix bank 0x2B7 is a valid slot (Ganondorf PLACEHOLDER), while vanilla tables use nSYAudioFGMVoiceEnd (0x2B7) as no sound (ft/ftcommondata.c:128)
- FPS.asm:83 `run_` | ADAPTED | port/remix/remix_display_settings.cpp:43 opens the port FPS overlay when Toggles.entry_fps is set
- GFXRoutine.asm:418 `extend_gfx_routine_table_` | PORTED | gm/gmremixcolscripts.c (routines copied from the RAM image, appended from id 0x56)
- GFXRoutine.asm:451 `action_change_fix_` | PORTED | gm/gmremixcolscripts.c + ft/ftparam.c:1425
- GFXRoutine.asm:487 `extend_gfx_check_` | PORTED | gmremixcolscripts.c overrides, ft/ftparam.c:1570
- Gallery.asm:799 `initial_` | STUB | mn/mncommon/mnremixgallery.c:93 only picks the victory image; no BGM per image/Gallery BGM, no idle mode (REMIX-TODO gallery-audio, gallery-idle in menus.md)
- Gallery.asm:930 `main_` | STUB | mnremixgallery.c:98-110 paging (L/R, stick) and B exit only; no A (change song), START (idle), L/R/Z drum kit
- Gallery.asm:1266 `screen_change_` | ADAPTED | mnremixgallery.c:16 reloads nSCKindCongra on every page (equivalent to forcing the CONGRATULATIONS id); the music reset part is not needed without gallery BGM
- Gallery.asm:1295 `get_fade_length_` | GAP | fades 45/4/100 frames not applied (menus.md REMIX-TODO gallery-fade)
- Gallery.asm:1312 `prevent_fgm_` | GAP | the Congratulations FGM is not suppressed per page
- Gallery.asm:1340 `add_button_` | GAP | no Gallery button in mn/mndata/mndata.c; the Gallery opens only with SSB64_START_GALLERY (sc/scmanager.c:959)
- Gallery.asm:1408,1416,1424,1432,1440,1456,1477,1480,1486 `cursor_` | GAP | Data screen cursor/wrap for the 4th button not ported (no button)
- GameEnd.asm:25 `update_screen_` | PORTED | gm/gmremixgameend.c:134 gmRemixGameEndSelectScene (salty runback, skip results, R+L)
- Handicap.asm:22 `use_handicap_` | PORTED | ft/ftremix/ftremixhandicap.c:19
- Hitstun.asm:6 `hitstun_` | PORTED | ft/ftparam.c:1792 (divisor 2.5)
- InputDelay.asm:19 `apply_input_delay_` | PORTED | ft/ftremix/ftremixinputdelay.c: delay applied to the fighters' controller reads of human fighters only; the asm delays the OS-level controller struct (also pause/menus). Narrower scope, low impact
- InputDisplay.asm:940,946,953 `adjust_timer_position_` | GAP | the timer stays at y=30 (if/ifcommon.c:2716); the asm lowers it to y=52 when port 4's Input Display is at the top. The ImGui row is centred at the top (port/remix/remix_input_display.cpp:211)
- JabLock.asm:7 `down_initial_` | PORTED | ft/ftcommon/ftcommondownwaitbounce.c:160
- JabLock.asm:29 `handle_lock_` | PORTED | ft/ftcommon/ftcommondamage.c:654
- JabLock.asm:67 `handle_reset_` | PORTED | ft/ftcommon/ftcommondownattack.c:26
- Japan.asm:30 `hitlag_` | PORTED | ft/ftremix/ftremixhitlag.c:28
- Japan.asm:107 `japanese_shieldstun_` | PORTED | ft/ftremix/ftremixshieldstun.c:21-28 (constants equal the asm)
- Japan.asm:206 `momentum_slide_` | PORTED | ft/ftcommon/ftcommonrunbrake.c:69
- Japan.asm:228 `japanese_stun_mash` | PORTED | ft/ftremix/ftremixjstunsleep.c
- Japan.asm:244 `japanese_sleep_mash` | PORTED | ft/ftremix/ftremixjstunsleep.c
- Japan.asm:272 `japanese_sfx_` | PORTED | ft/ftmain.c:417 ftRemixJapaneseMotionSoundFgm
- Japan.asm:349 `japanese_sfx_link_bomb_collide_` | PORTED | it/itmanager.c:371 ftRemixJapaneseItemSoundFgm (documented owner deviation in ftremixjapanesesounds.c:140)
- Japan.asm:418 `japanese_sfx_link_boomerang_collide_` | ADAPTED | wp/wpmanager.c:245 converts the sound of every weapon at creation (covers the boomerang)
- Japan.asm:455 `japanese_sfx_projectile_` | PORTED | ft/ftmain.c:2743 + wpmanager.c:245
- Japan.asm:532 `reflect_` | PORTED | wp/wpprocess.c:586 ftRemixJapanFixReflectMul
- Joypad.asm:390 `set_taunt_mask_` | PORTED | ft/ftmanager.c:1204 ftRemixTauntButtonMaskFor
- Joypad.asm:430,436 `prevent_taunt_jump_` | PORTED | ft/ftcommon/ftcommonkneebend.c:91 and ftcommonjumpaerial.c:312
- Knockback.asm:15 `randomize_angles_` | PORTED | ft/ftmanager.c:1051
- Knockback.asm:142 `get_angle_hitbox_` | PORTED | ft/ftmain.c:284
- Knockback.asm:183 `get_angle_throw_` | PORTED | ft/ftcommon/ftcommonthrown2.c:231
- LedgeJump.asm:24 `extend_ledge_input_check` | PORTED | ft/ftremix/ftremixledgejump.c:23
- LedgeJump.asm:73 `extend_under_100_ledge_options` | PORTED | ftremixledgejump.c:47
- LedgeJump.asm:89 `extend_over_100_ledge_options` | PORTED | ftremixledgejump.c:47
- LedgeTrump.asm:22 `ledge_check` | PORTED | mp/mpcommon.c:577 + ft/ftremix/ftremixledgetrump.c
- LedgeTrump.asm:86 `track_ledge_grabs` | PORTED | mp/mpcommon.c:591
- MagnifyingGlass.asm:22 `apply_custom_zoom_` | PORTED | if/ifcommon.c:1739 gmRemixMagnifyApplyCustomZoom
- OnHit.asm:7 `on_hit` | STUB | ft/ftparam.c:434 -> ft/ftremix/ftremixchartables.c:85 ftRemixRunOnHit has Sonic and Ebi only; Character.on_hit also lists SSONIC (SonicSpecial.asm:1194)
- PAL.asm:22 `update_tv_type_` | NA | PAL.asm is wrapped in `{defined MAKE_PAL}` (line 2), defined only by patch-pal.bat; the NTSC build behind this port does not assemble it
- Pause.asm:22 `hold_` | PORTED | ft/ftremix/ftremixpause.c:25, if/ifcommon.c:3328
- Pause.asm:76 `disable_hud_` | PORTED | if/ifremixpauselegend.c:148, if/ifremixhud.c
- Pause.asm:109 `extra_controls_` | PORTED | if/ifremixpauselegend.c:187-210 and gm/gmremixbgm.c:71 (D-pad song change), ifcommon.c:3440; scene camera keep ftremixpause.c:47
- Pause.asm:165 `show_legend_helper_` | PORTED | if/ifremixpauselegend.c:238 (R legend)
- Pause.asm:410 `extra_camera_controls_` | GAP | Camera.extended_movement_ (A/B zoom, D-pad/C pan in pause, camera_pan_offsets_) is not in the port (docs notes toggles.md toggles.md record it as skipped)
- PerfectShield.asm:24 `shield_stun_action_change_skip` | PORTED | ft/ftmain.c:4413 ftRemixPerfectShieldHandlesGuardSetOff
- PerfectShield.asm:58 `shield_reflect_projectiles` | PORTED | wp/wpprocess.c:507
- PerfectShield.asm:145 `shield_reflect_items` | PORTED | it/itprocess.c:1111
- Poison.asm:17 `star_fix` | PORTED | ft/ftremix/ftremixpoison.c (ftMainProcUpdateInterrupt hook)
- Poison.asm:36 `poison_hook` | PORTED | ftremixpoison.c ftRemixPoisonUpdate
- PokemonAnnouncer.asm:19 `there_goes_` | PORTED | if/ifcommon.c:3831 ifRemixAnnouncerGameSetFgm
- PokemonAnnouncer.asm:47 `delay_` | PORTED | ifcommon.c:3831 ifRemixAnnouncerGameSetDelay
- PokemonAnnouncer.asm:79 `pokemon_name_` | PORTED | it/itmanager.c:874
- PokemonAnnouncer.asm:262 `sleep_announcement_` | PORTED | ft/ftcommon/ftcommonfurasleep.c:42
- PokemonAnnouncer.asm:351 `hit_announcement_` | PORTED | ft/ftcommon/ftcommondamage.c:745
- PokemonAnnouncer.asm:487 `throw_hit_announcement_` | PORTED | ft/ftcommon/ftcommonthrown2.c:228
- PokemonAnnouncer.asm:591 `thunder_g_announcement_` | PORTED | ft/ftchar/ftpikachu/ftpikachuspeciallw.c:114
- PokemonAnnouncer.asm:598 `thunder_a_announcement_` | PORTED | ftpikachuspeciallw.c:128
- PokemonAnnouncer.asm:646 `quick_g_announcement_` | PORTED | ftpikachuspecialhi.c:111
- PokemonAnnouncer.asm:653 `quick_a_announcement_` | PORTED | ftpikachuspecialhi.c:126
- PokemonAnnouncer.asm:698 `charging_announcement_` | PORTED | wp/wpsamus/wpsamuschargeshot.c:287
- PokemonAnnouncer.asm:749 `sing_announcement_` | PORTED | ef/efmanager.c:4914
- PokemonAnnouncer.asm:948 `refresh_burrow_binary_` | NA | dig_match_history is never written: burrow_announcement_ (PokemonAnnouncer.asm:888-927) has `a2 = 1<<port` (always non-zero) before `bnezl a2,_burrow_again`, so the store at :913 is dead code; resetting the byte changes nothing (see burrow STUB)
- PokemonAnnouncer.asm:1149 `fireflower_` | PORTED | it/itcommon/itfflower.c:333
- PokemonAnnouncer.asm:1164 `charizard_` | PORTED | it/itmonster/itlizardon.c:438
- PokemonAnnouncer.asm:1260 `rest_g_announcement_` | PORTED | ft/ftchar/ftpurin/ftpurinspeciallw.c:52
- PokemonAnnouncer.asm:1267 `rest_a_announcement_` | PORTED | ftpurinspeciallw.c:65
- PokemonAnnouncer.asm:1315 `pound_g_announcement_` | PORTED | ftpurinspecialn.c:123
- PokemonAnnouncer.asm:1322 `pound_a_announcement_` | PORTED | ftpurinspecialn.c:137
- PokemonAnnouncer.asm:1431 `hung_tough_announcement_` | PORTED | ft/ftpublic.c:178
- PokemonAnnouncer.asm:1491 `recovery_announcement_` | PORTED | ft/ftparam.c:1879
- PokemonAnnouncer.asm:1540 `recovery_announcement_magnet_` | PORTED | ft/ftmain.c:2673
- PokemonAnnouncer.asm:1592 `taunt_announcement_` | PORTED | ft/ftcommon/ftcommonappeal.c:31
- PokemonAnnouncer.asm:1641 `clang_announcement_` | PORTED | ft/ftmain.c:2273
- PokemonAnnouncer.asm:1696 `stun_announcement_` | PORTED | ft/ftcommon/ftcommonfurafura.c:42
- PokemonAnnouncer.asm:1741 `time_up_announcement_` | PORTED | if/ifcommon.c:3041
- PokemonAnnouncer.asm:1780 `team_ko_announcement_` | PORTED | ft/ftcommon/ftcommonsleep.c:70
- PokemonAnnouncer.asm:1857 `air_specials_announcement_` | PORTED | ft/ftcommon/ftcommonspecialair.c:205
- PokemonAnnouncer.asm:1930 `ground_specials_announcement_` | PORTED | ft/ftcommon/ftcommonspecialn.c:150
- Practice.asm:26 `overlay_colour_` | PORTED | ft/ftremix/ftremixoverlay.c via ft/ftmain.c:2242
- Practice.asm:200 `flash_on_z_cancel_` | PORTED | ft/ftremix/ftremixzcancel.c via ft/ftcommon/ftcommonattackair.c:74
- Render.asm:2116 `setup_` | ADAPTED | per-screen Render object setup replaced by per-module setup in the port (ftRemixZCancelSetup ft/ftmanager.c:299, itRemixClearActiveItems sc/scmanager.c:1273, gmRemixColScriptsClearOverrides ef/efdisplay.c:119, ImGui/remix_hud_text overlays)
- SFXReplace.asm:66 `sfx_replace` | PORTED | ft/ftremix/ftremixsfxreplace.c via ft/ftmain.c:417
- SFXReplace.asm:176 `sfx_replace_down_bounce` | PORTED | ft/ftcommon/ftcommondownwaitbounce.c:129
- SRAM.asm:52,58 `(unnamed)` | ADAPTED | vanilla save/load restricted to the first 0x5EC block so the Remix blocks survive; the port persists lbBackup through port_save.cpp and Remix toggles/panels through CVars, no shared SRAM image
- Settings.asm:26 `set_vs_settings_` | PORTED | sc/scremixtournament.c:14, sc/scmanager.c:877
- SingleButtonMode.asm:119,124,129,134 `handle_cpu_inputs_` | PORTED | ft/ftcomputer.c:3482-3518 + ft/ftremix/ftremixsinglebutton.c:131-153
- Skeleton.asm:45 `skeleton_mode_1_` | PORTED | ft/ftdisplaymain.c:898-1005 ftRemixSkeletonDisplayId
- Skeleton.asm:59 `skeleton_mode_2_` | PORTED | ftdisplaymain.c:928
- Spawn.asm:19 `load_respawn_point_` | PORTED | mp/mpcollision.c:4453 + gr/grremix/grremixspawn.c:21
- Spawn.asm:103 `load_spawn_` | STUB | gm/gmremixspawnsetup.c:226 gmRemixNeutralSpawnApply (sc/sccommon/scvsbattle.c:149) has the table for stages 0x00-0x10 only (GMREMIX_SPAWN_STAGES_NUM 0x11); Spawn.neutral_table has rows for every stage id up to 0xD7. Training branch is ported separately (sc/sc1pmode/sc1ptrainingmode.c:2255)
- Speed.asm:14 `process_update_` | PORTED | sys/objman.c:2322 itRemixStopwatchCheckFunction
- Speed.asm:58 `handle_rate_` | PORTED | if/ifcommon.c:3412 ftRemixSpeedRunFrames, sc1ptrainingmode.c:791
- SpotDodge.asm:9 `check_input` | PORTED | ft/ftremix/ftremixspotdodge.c
- StaleMoves.asm:40,178,181 `stale_moves` | PORTED | ft/ftremix/ftremixstalemoves.c via ft/ftparam.c
- Stereo.asm:16 `fix_sfx_pan_` | PORTED | lb/lbcommon.c:775
- StockMode.asm:49 `set_initial_stock_count_` | PORTED | gm/gmremixstockmode.c, sc/sccommon/scvsbattle.c:365
- Toggles.asm:89 `disable_options_functionality_` | ADAPTED | the Remix settings are the ImGui menu (port/gui/PortMenu.cpp:1368 AddMenuRemix); the Options screen stays vanilla (ROADMAP decision T4.2)
- Toggles.asm:111,114,117,122,125,128,131,134,137,140,143,148,151,154,157 `add_mode_select_remix_button_` | ADAPTED | 5th Mode Select button replaced by the ImGui menu (PortMenu.cpp:1368; ROADMAP:355). Gap inside: nothing clears the 1P Remix mode when leaving Mode Select (Toggles.asm:272-276 clears singleplayer_mode_flag; gSC1PRemixMode is only set in mn/mn1pmode/mnremix1pmode.c:164)
- Toggles.asm:224,229 `rescale_mode_select_buttons_` | ADAPTED | 5th Mode Select button replaced by the ImGui menu (PortMenu.cpp:1368; ROADMAP:355). Gap inside: nothing clears the 1P Remix mode when leaving Mode Select (Toggles.asm:272-276 clears singleplayer_mode_flag; gSC1PRemixMode is only set in mn/mn1pmode/mnremix1pmode.c:164)
- Toggles.asm:272,316 `mode_select_screen_change_` | ADAPTED | 5th Mode Select button replaced by the ImGui menu (PortMenu.cpp:1368; ROADMAP:355). Gap inside: nothing clears the 1P Remix mode when leaving Mode Select (Toggles.asm:272-276 clears singleplayer_mode_flag; gSC1PRemixMode is only set in mn/mn1pmode/mnremix1pmode.c:164)
- Toggles.asm:376 `mode_select_setup_` | ADAPTED | 5th Mode Select button replaced by the ImGui menu (PortMenu.cpp:1368; ROADMAP:355). Gap inside: nothing clears the 1P Remix mode when leaving Mode Select (Toggles.asm:272-276 clears singleplayer_mode_flag; gSC1PRemixMode is only set in mn/mn1pmode/mnremix1pmode.c:164)
- Transitions.asm:65,71,77,83,89,95,101,107 `(unnamed)` | PORTED | lb/lbtransition.c:175-208 table with 15 entries (halfword layout not needed in C)
- Transitions.asm:113 `(unnamed)` | PORTED | mn/mnvsmode/mnvsresults.c:3758 uses ARRAY_COUNT(dLBTransitionDescs)
- Transitions.asm:119,125 `(unnamed)` | NA | labels of the vanilla debug transition test menu (Debug screen 0x03); the port has no Debug screen entry (not reachable)
- VsDemo.asm:13 `(unnamed)` | STUB | sc/sccommon/scremixvsdemo.c:~140 scRemixVsDemoGetNameTextureOffset exists but nothing calls it; sc/sccommon/scautodemo.c:664 shows the parent fighter's vanilla name
- VsDemo.asm:21 `fix_offset_` | STUB | same as :13 (the +0x10 fix lives in the unused helper)
- VsDemo.asm:34 `enable_focused_remix_chars_` | PORTED | sc/sccommon/scremixvsdemo.c:83 via mn/mncommon/mntitle.c:360
- VsDemo.asm:74 `enable_nonfocused_remix_chars_` | STUB | scremixvsdemo.c:108 excludes the two focused fighters only; the asm also makes port 4 differ from port 3 (VsDemo.asm:90-93)
- VsDemo.asm:108 `enable_remix_stages_` | PORTED | scremixvsdemo.c:119 via scautodemo.c:575
- ZCancel.asm:34 `CRUEL_Z_CANCEL_MODE` | PORTED | ft/ftremix/ftremixzcancel.c (all 11 punish modes)
- ZCancel.asm:235 `_z_cancel_opts` | PORTED | ftremixzcancel.c:225-242
- midi.asm:705 `replace_midi_block_` | NA | N64 expansion-RAM block for MIDI loading (midi_memory_block); the port loads sequences from the extracted bank
- midi.asm:716 `augment_sound_test_music_` | PORTED | mn/mndata/mnremixsoundtest.c:35
- midi.asm:1253 `dmaCopy_moved_bank_map_` | NA | reloads the instrument bank map relocated in ROM into RAM; the port's bank map comes from the Torch bank (port/bridge/audio_bridge.cpp)
- midi.asm:1363 `override_instrument_parameters_` | GAP | per-BGM priority and bend-range overrides (priority_override_table, bend_range_override_table, 49 add_* calls in midi.asm) are not applied by libultra/n_audio or Torch
- midi.asm:1426 `override_master_volume_` | GAP | per-BGM master volume overrides (master_volume_override_table, 178 add_master_volume_override lines) are not applied; no `override` code in decomp/src/libultra/n_audio, port/ or torch/
- moveset.asm:12 `get_cmd_address_` | ADAPTED | moveset pointers: ft/ftmain.c:5528 takes the descriptor offset as a pointer above 0x100000 and keeps the 0x80000000 none sentinel (:5502, :5552); custom command decoding in ft/ftremix/ftremixcommand.c. Not traced through the Remix action table loader (g7 scope), medium-low confidence
- timeouts.asm:17,22,27,32,37,42,47,52,57,62,67 `(unnamed)` | PORTED | mn/mncommon/mnremixtimeouts.c, 11 call sites (mnmodeselect.c:813, mn1pmode.c:795, mnoption.c:934, mndata.c:700, mnmaps.c:3018, mnvsmode.c:1500, mnvsoptions.c:1428, mnplayersvs.c:6981, mnplayers1pgame.c:3765, mnplayers1pbonus.c:3205, mnplayers1ptraining.c:3347)

Direct origin writes
- FGM.asm:945 | PORTED | mn/mndata/mnsoundtest.c:13,809 voice count 0xF4 + 850 (extended_voice_map_table is 850 words in build/remixasm/remix.sym)
- FGM.asm:947 | PORTED | same (second max index of the voice list)
- FGM.asm:949 | PORTED | same
- FGM.asm:951 | PORTED | same
- midi.asm:691 | PORTED | mnsoundtest.c:12 music count 375 (midi_count; the symbol table gives 330..333 extra slots, consistent)
- midi.asm:693 | PORTED | same
- midi.asm:695 | PORTED | same
- midi.asm:697 | PORTED | same
- PAL.asm:45 | NA | MAKE_PAL-only (region byte 'P')
- PAL.asm:51 | NA | MAKE_PAL-only (VI table)
- PAL.asm:72 | NA | MAKE_PAL-only (VI table)
- PAL.asm:93 | NA | MAKE_PAL-only (VI table)
- moveset.asm:15 | NA | not a ROM write: `origin 0x6316C / base 0x800E796C` sits inside the get_cmd_address_ patch only to place _return; no bytes are emitted

## CT: Per-fighter tables (Character.table_patch_start)

Format: `table (rows) | asm anchor | verdict | evidence`.

- `action_string` (66) | `Character.asm:2386` | ADAPTED | image read, `sc/sc1pmode/sc1ptrainingremixaction.c:119` (fkind*4 -> table pointer -> string)
- `remix_1p_end_bgm` (50) | `Character.asm:2200` | ADAPTED | image read, `sc/sc1pmode/sc1premixending.c:19` (halfword per id)
- `crowd_chant_fgm` (48) | `Character.asm:1677` | GAP | gap 1
- `menu_zoom` (46) | `CharacterSelect.asm:4029-4045` and the per-fighter files | ADAPTED | roster field, `tools/gen_remix_roster.py:317-354` (CharacterSelect.asm read first because Sheik/NSheik files come later in `main.asm:130,228,245`), applied at `remix_roster.cpp:329` and `:355-369` (vanilla ids 0x0C..0x1A)
- `kirby_inhale_struct` (44) | `Character.asm:2684`, readers `:2023-2060` | ADAPTED | image read, `ft/ftremix/ftremixkirbycopy.c:35` (0xC byte rows, hat id and scale)
- `ai_behaviour` (42), `lv10_ai_behaviour` (12), `ai_long_range` (25), `nsp_shoot_custom_move` (3) | `Character.asm:1769-1830`, `:2959` | ADAPTED (Phase 6) | image read, see AI table
- `ai_attack_prevent` (40), `recovery_logic` (37), `cpu_post_process` (33), `cpu_attack_weight` (15), `fighter_reflect` (10) | | AI, Phase 6 | not read, see AI table
- `fighter_DK_mode` (32) | `Character.asm:3213` | ADAPTED | image read, `ft/ftremix/ftremixdkmode.c:61-72` (own entry, else table base, as `DKMode.asm` `_loop_initial`)
- `air_usp` (32), `ground_nsp` (31), `air_nsp` (30), `ground_usp` (29), `ground_dsp` (28), `air_dsp` (28) | `Character.asm:1412-1540` | ADAPTED | `ftremixbind.c:208-232` reads `REMIX_SPECIAL_TABLE_*` per id from the image and resolves each address through the vanilla or ported proc tables. Checked all 217 explicit entries of the 8 special tables: the symbol `REMIX_SYM_<Scope>__<label>` of every entry is a `{ sym, fn }` row of `decomp/src/ft/ftremix/**`. Vanilla `0x80xxxxxx` targets resolve via `ftremixvanillaprocs.c` or fall back to the parent's handler (logged). Wolf `ground_nsp` row is commented out (`Wolf.asm:276`): parent Fox's NSP, same as asm. Sample of 5 fighters traced (Banjo USP, Crash DSP, Peppy NSP, Sonic NSP, Marth USP): same routine as the asm, no divergence found
- `entry_script` (28) | `Character.asm:1563` | ADAPTED | hand table `ftremixentry.c:49-90` (`dFTRemixEntryScripts`); compared with the 28 rows plus parent inheritance: all effective scripts match (polygons skip because `polygon_setup` is later in the file than e.g. `NCrash.asm:288`; Ness has no vanilla entry effect)
- `grounded_script` (22) | `Character.asm:2164` | GAP | gap 4
- `kirby_ground_nsp` (20), `kirby_air_nsp` (20) | `Character.asm:1393,1469` | STUB for SLIPPY and DRAGONKING (gap 12), ADAPTED for the other 18 | `ftremixbind.c:187-205`
- `magnifying_glass_zoom` (19) | `Character.asm:3255` | ADAPTED | image read, `gm/gmremixmagnify.c:32`
- `initial_script` (16) | `Character.asm:1580` | GAP (MLUIGI, GBOWSER armor, gap 6); the other rows only clear passive words at spawn and are redundant because `ftManagerAllocFighter` does `bzero` (`ft/ftmanager.c:273`); JKIRBY handled at `ftmanager.c:824-826`
- `entry_action` (15) | `Character.asm:1543` | ADAPTED | image read, `port/remix/remix_roster.cpp:286` (`bindEntryStatuses`)
- `variants_with_same_model` (14) | `TagTeam.asm:1425-1500` | NA | only reader is `load_variants_in_same_slot_`, which preloads sibling fighter files into a Tag Team heap slot with `ftManagerSetupFilesAllKind` (N64 heap management; the port loads from the o2r on demand)
- `variants` (14) | `Character.asm:2524` | ADAPTED | image read, `mn/mnplayers/mnremixvariants.c:100`
- `pipe_turn` (12) | `marioshared.asm:204-250` | GAP | gap 2
- `jab_3` (12), `jab_3_action` (7), `jab_3_timer` (6), `rapid_jab` (4), `rapid_jab_begin_action` (7), `rapid_jab_loop_action` (7), `rapid_jab_ending_action` (7), `rapid_jab_unknown` (3) | `Character.asm:1230-1390`, `:2069-2160` | GAP | gap 14 (g5 gap 1)
- `variant_original` (11) | `CharacterSelect.asm:4990` | ADAPTED | derived from `variants` in `mnremixcssindicators.c:66-90` (a grid fighter is its own original, a variant maps to the grid fighter that lists it); equivalent for the 11 rows
- `ness_jump` (9) | `nessshared.asm:195-250` | GAP | gap 8
- `gfx_routine_end` (7) | `Character.asm:2179,1620` | GAP | gap 5
- `yoshi_egg` (6) | `Character.asm:1696-1720` | GAP | gap 9
- `on_hit` (3) | `OnHit.asm` | GAP (SSONIC) | gap 3; SONIC and EBI ported (`ftremixchartables.c:85-103`)
- `custom_capture_dk_interrupt` (3) | `CustomGrabAction.asm:79` | GAP (WARIO, MARINA); PIANO ADAPTED | gap 11
- `custom_capture_action` (2) | `CustomGrabAction.asm:12,41` | ADAPTED | `ftremixdeschooks.c:30-38` (WARIO, PIANO -> 0xB8)
- `fireball` (2), `kirby_fireball` (2) | `Character.asm:1723,1745` | ADAPTED for Piano (Book, `ftkirbycopymariospecialn.c:45-48`, `ftmariospecialn.c:38`), GAP for MLUIGI and NDRM (g5 gap 7) | the macro row `fireball.asm:60,73` is the DRM/JMARIO/JLUIGI/DRL entries, ported at `wpmarioremixfireball.c:28-45`
- `electric_hit` (2) | `Character.asm:1637` | GAP | gap 10
- `winner_fgm` (1), `label_height` (1) | `resultsscreen.asm:671-698` | GAP | gap 13
- `tr_wins_lx` (1), `tr_winner_ptr` (1), `tr_winner_lx` (1), `tr_winner_scale` (1) | `resultsscreen.asm:700-720` | STUB | gap 13: layout invented in `remix_roster.cpp:263-269`
- `winner_bgm` (1) | `resultsscreen.asm:722-730` | ADAPTED | image read and decode of the `ori a1, zero, <id>` routine, `mn/mnvsmode/mnremixwinbgm.c:30-47`
- `default_costume` (1, `set_default_costumes` macro; all fighters through `Character.asm:806`) | | ADAPTED | image read, `mn/mnplayers/mnremixcostumes.c:44-60`
- `costume_shield_color` (1 + 32 `set_costume_shield_colors` callers) | `Character.asm:2793` | GAP | gap 7
- `close_quarter_combat` (17) | `AI.asm:6309-6360` | NA | every row is commented out, so the table is the default `_continue` (`ftremixairoll.c:5`)
- `ai_behaviour_heavy_attack` (1) | `Character.asm:2876` | NA | the patch is commented out
- `sound_type` (define_character `Character.asm:332-336`) | `Character.asm:2424`, reader `:2482-2520` | ADAPTED | roster field `sound_type` (`gen_remix_roster.py`), read at `ft/ftremix/ftremixjapanesesounds.c:67` and `ftremixjapanfix_reflect.c:36`
- `variant_type` (define_character `Character.asm:338-340`) | `Character.asm:2603` | ADAPTED | roster field (`remix_roster_data.h`), read at `sc/sccommon/scremixvsdemo.c:35`, `mnplayersvs.c:657`, `mnremixcssrules.c:234`

- Gaps 2, 3, 6, 9, 10, 11 are one-liners in code that already exists (a case or a descriptor field). Gap 1 and 7 need two new symbols in `tools/gen_remix_actions.py`
  (`Character.crowd_chant_fgm.table`, `Character.costume_shield_color.table`) and a regeneration of `port/remix/remix_symbols.h`.
- Gap 4 is the one with the largest play impact (Ebisumaru and Roy lose a special after one use; Super Sonic, N Sonic, Dr. Mario family).
  The cleanest fix is to resolve the effective parent through `port_fighter_parent_fkind` in the `switch (fp->fkind)` of `mp/mpcommon.c:454`
  and in the matching initial switch of `ftmanager.c:780`, then keep explicit rows for the overrides (EBI, PEACH, NPEACH, KIRBY, JKIRBY, and the
  `0x800DE428` / `DISABLED` rows listed in the asm).
- `polygon_setup` (`Character.asm:2847-2900`) patches `ai_behaviour` twice; the second write copies `action_string` of the parent
  (looks like an upstream typo). The port reads `ai_behaviour` from the image, so it reproduces whatever Remix ended up with.
- The unsourced `FighterParentKindResolveEvent` / `FighterRapidJabStatusQueryEvent` hooks in `ftcommonattack1.c` / `ftcommonattack100.c` are
  leftovers of the Super Remix mod system; no listener exists in the Remix build.
