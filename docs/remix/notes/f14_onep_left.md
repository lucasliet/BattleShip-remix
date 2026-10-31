# F14 onep_left: Remix 1P leftovers (ROADMAP "Sobras do M6", list `1P`)

Status: investigation only. No source file was edited in this pass, so no `REMIX-TODO(1P)` marker was added and the syntax check was not needed.
The brief's rules (no build, no game, no git) were followed.

Sources read: `docs/remix/ROADMAP.md` ("Sobras do M6", T6.2), `docs/remix/notes/f11_mode_singleplayer.md` (section 0 scope table, sections 1 and 2),
`.toolchain/pkgs/smashremix-src/src/SinglePlayerModes.asm` (lines 84-92, 6536-6552, 7408-7420, 7985-8030, 8083-8110, 8109-8215), and the port in
`decomp/src/sc/sc1pmode/sc1premixallstar.{c,h}`, `sc1pgame.c` (1156, 1480-1520, 1773, 2794, 2836), `mnplayers/mnplayers1pbonus.c`.

`docs/remix/f11_mode_singleplayer.md` does not exist; the mode note is `docs/remix/notes/f11_mode_singleplayer.md`.

## Per-item state

| # | Leftover | State | What was verified | What is missing |
|---|---|---|---|---|
| 1 | Records and CSS title of Bonus 2 | not started | Port has the Bonus 2 CSS (`mnplayers1pbonus.c`) and the HRC record (`sc1premixhrcrecord.c`, `LBBackup1PRecord.hrc_distance`). The Multi-Man KO record (`_ko_save`) is not ported. | Record draw (`draw_hrc_high_score_decimal_`, `_ko_save`) and the CSS title with the mode name. The asm for the title was not read. |
| 2 | Random fighter in the Bonus 2 CSS | not started | f11 section 0 row `bonus2_css_2_1p` (SinglePlayerModes.asm 130-266) covers the random-character roll, not read in this pass. | The CSS has no Random cell (f11 P3 note). Needs the asm for the roll and a cell. |
| 3 | Bonus 3 | not started | The port has the Bonus 3 stage IDs and the wait (`sc1PGameWaitStageBonus3Update`) and the RNG exit (`sc1premixbonusexit.c`). | Timer (`_load_timer`, `bonus3_timer`, SinglePlayerModes.asm 989-1170), `_time_save`, `_reset_button`/`_reset`, `_end_match_screen` (1172-1208). Not read in full. |
| 4 | End of the Giga Bowser stage with more than 180 damage | open (R5 in the ROADMAP) | f11 and ROADMAP R5: the backdrop steps are 90 and 180 and the HP minimum is 200, so the test with HP 1 never reaches the last step. | Needs a run with 200 damage dealt, or a hook that advances the backdrop. Not an asm read. |
| 5 | Single card of Remix Team | not started | f11 row `remix_yoshi_card` (SinglePlayerModes.asm 5303-5399) and `title_card_*` (4297-4852) are indexed only. | Asm of the Remix Team card not read. |
| 6 | Piano card size | not started | Asm 3896-3923: Mad Piano sets its icon (`0x00015240`), title card and announcement, and `title_card_model_scale` (4762-4852) is indexed only. | Scale value for the Piano card not read. Mapped to the title card scale path. |
| 7 | Metal Mario Bros texture | not started | Asm 3859-3891 (Metal Mario and Metal Luigi IDs, progress icon `metal_mario_brothers`, texture slot written at 3879) and 4952-5111 (`metal_bros_magic`, `metal_bros_text`, FGM 0x542). | Not ported. The texture word at 3879 needs the sprite offset from the Remix data image. |
| 8 | All-Star Rest portraits and icons | not started | Port has the Rest Area hearts (`sc1PRemixAllstarSetupRestArea`, `dSC1PRemixAllstarHeartX`) and the progress state. | `rest_area_setup` / `rest_area_routine` (SinglePlayerModes.asm 7411-7810) are indexed only; the portraits and icons are not ported. |
| 9 | Series icons of the ending | not started | Asm 8693-8737: `_allstar_final_series_stock_set` writes the series icon footer of the stock icon, and the commented lines 8693-8699 show the old version. | Port has no final series icon path. Needs `sc1PGameFuncStart`-level series icon code, not read. |
| 10 | `costume_fix` | open, partly mapped | Asm 8179-8214, hook 0x10C350 / 0x8018DAF0. For All-Star, a vanilla fighter (`id < Character.id.BOSS`) with a costume index of 4 or more gets a new random costume from 0 to 3 (`Global.get_random_int_`). | Port has `sc1PRemixAllstarPrepareFinal` (the final team, which does the same reroll) and applies the costume at `sc1pgame.c` 1515 and 1725 for the final team only. The asm hook runs in the generic spawn routine (0x8018DAF0), so the port may miss the non-final spawns. The spawn routine was not read to confirm. Do not port until confirmed. |
| 11 | `match_begin_flag` | open, mapped | Flag declared at SinglePlayerModes.asm 88-89. Writers: `allstar_randomization` clears it (6544), `rest_area_setup` sets it to 1 (7415). Readers: `stock_prevent` (8140), `damage_prevent` (8165), `prevent_erase` path. | Port keeps its own All-Star carry-over (`sSC1PRemixAllstarPercent`, `sc1PRemixAllstarPlayerDamage` at sc1pgame.c 1773 and 2794), so the asm readers have no direct equivalent. Whether the port's stock and damage reset matches the asm readers has not been tested in game. Needs a test in game (see below) before any change. |
| 12 | `pause_exit` | open, asm read | Asm 7988-8026, hook 0x8FAC4 / 0x801142C4. Exiting pause in All-Star goes to the 1P menu (screen 8, the byte at 0x800A4AD0). In Remix 1P it calls `CharacterSelectDebugMenu.Size.clear_settings_for_1p_` with the primary human port (`0x800A4AE3`). | Port has no equivalent of the screen byte write nor of `clear_settings_for_1p_` (Size clear). `REMIX-TODO(1P): pause_exit` belongs in the pause exit of the 1P game, not added here. |
| 13 | All-Star's own ending | open | Asm 8085-8107 `end_allstar`: when `end_game_flag` is set, the stage ID becomes 0x0D (final stage), hook 0x10F768 / 0x80190F08. Port has `SC1PREMIX_ALLSTAR_STAGE_END 14` (0xE) and `sc1PRemixAllstarStepStage`. | Port's stage value for the final stage (0x0D in asm) and its ending screen were not compared. The `end_game_flag` writer in the port is not mapped. |

## Next steps (for the lead or a follow-up worker)

1. Read the spawn routine at 0x8018DAF0 (asm `costume_fix`) and decide whether the port's final-team reroll covers the non-final spawns. Only then port item 10.
2. Read SinglePlayerModes.asm 989-1208 (Bonus 3 timer, reset, end screen) and 1-2 blocks of `title_card_*` (4297-4852) for items 3, 5 and 6.
3. Read `rest_area_setup` (7411-7810) for item 8, and `_allstar_final_series_stock_set` (8713-8740) for item 9.
4. Add a `REMIX-TODO(1P)` with the symbol name at each unported spot once the code location is confirmed.

## Lead must capture in game

- Item 11 (`match_begin_flag`): play All-Star past the first Rest Area, lose a stock, and check that the stock count and the damage carried into the next fight match the vanilla All-Star rules. The asm readers are `stock_prevent` and `damage_prevent`.
- Item 12 (`pause_exit`): pause during an All-Star match and exit to the menu; the screen must go to the 1P menu. Also pause in Remix 1P and exit; the Size settings must clear.
- Item 13 (All-Star ending): finish the All-Star run and check the final stage is the Giga/final stage and the end screen appears.
- Item 4 (Giga Bowser): deal more than 180 damage to Giga Bowser in the final stage and check the backdrop reaches its last step.
