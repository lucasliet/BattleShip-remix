# F22 Yellow team (Teams.asm, M11 Fase 5)

Asm: `.toolchain/pkgs/smashremix-src/src/Teams.asm` (577 lines), plus `ComboMeter.team_offset_table`, `Shield.color_fix_.table_team` and
`Spawn.load_spawn_.valid_teams` (the three tables the file asks to update by hand), and the `Teams.add_team_costume` calls of the character
files. Written, not built, not run.

## Design

The Remix adds one team, yellow (team id 3, `MNREMIX_TEAM_YELLOW`), after red 0, blue 1, green 2. Its values are not copied: they are read
from the Remix RAM image through `REMIX_SYM_Teams__*` (the generator `tools/gen_remix_actions.py` now emits every `Teams.*` symbol, and
`Shield.color_fix_.table_ffa` / `table_team`), with `remix_ram_image_read_*`. The vanilla teams keep the decomp values, so red, blue and
green are byte-identical to before.

- `mn/mnplayers/mnremixteamsrules.c/.h` (pure, tested in `tests/remix/test_teams_rules.c`, 8 tests): team cycle, added-team test, vanilla colour
  index (red 0, blue 1, green 3), costume table index, big text palette offset (6 bytes), word to float, Default shield source,
  results winner loop bound, the yellow "WINS!" x.
- `mn/mnplayers/mnremixteams.c/.h` (readers): button sprite offset, colour index (panel, gate, series logo, wallpaper, columns, port colour),
  shadow colour, costume of a character, name, name x, name scale, name palette, big text colours, announcer FGM, shield team and port colours.

## asm -> port

| Teams.asm | port |
|---|---|
| `team_button_table` + `use_extended_team_table_` (button image) | `mnPlayersVSMakeTeamSelect`, macro `MNPLAYERSVS_TEAM_BUTTON_OFFSET` -> `mnRemixTeamsGetButtonOffset` (yellow sprite offset 0x1F648 in the CSS common file, read from the table) |
| `extend_button_click_` (max team NUM_TEAMS - 1) | `mnPlayersVSCheckTeamSelectInRangeAll`, `MNPLAYERSVS_TEAM_NEXT` -> `mnRemixTeamsRulesNextTeam` |
| `team_panel_table` + `use_extended_panel_table_` (5 patches: button press, screen load, FFA/Team toggle, HMN/CPU button, controller plug) | the 5 `(team == Green) ? 3 : team` panel colour expressions of `mnplayersvs.c` -> `MNPLAYERSVS_TEAM_COLOR` -> `mnRemixTeamsGetColor` |
| `team_port_color_table` (0x8013A6A0, start of the match) | `mnPlayersVSSetSceneData`, `MNPLAYERSVS_TEAM_PORT_COLOR` (was `dIFCommonPlayerTeamColorIDs[team]`, whose index 3 is the CPU colour 4) |
| `new_costume_table`, `add_team_costume` (all characters, from every character file) + `get_team_costume_` (0x800EC104) | `mnRemixCostumeGetDefaultTeam` (reached by `ftParamGetCostumeTeamID`) reads `Teams.new_costume_table + fkind` for team 3 |
| `classic_sonic_costume_fix_` (3 hooks) | `mnRemixSonicClassicGetTeamCostume` (see `f18_classic_sonic.md`) |
| `team_shadow_colors` (0x8013B71C, fighter shadow in team battle) | `ftshadow.c` `FTSHADOW_TEAM_COLOR` -> `mnRemixTeamsGetShadowColor` (the array was indexed by `fp->team`, out of bounds for 3) |
| `team_name_table`, `team_name_x_position_table`, `team_name_scale_table`, `team_name_palette_index_table`, `big_text_palette_table` | `mnVSResultMakeTeamName` and `mnVSResultsMakeString` (`mnvsresults.c`): the name "YELLOW" is copied byte by byte from the image (the image keeps words in host order), x 26.0, scale 0.875, palette index 5 |
| `team_name_fgm_table` + `extend_results_screen_announcer_` | `MNVSRESULTS_TEAM_ANNOUNCER` (announcer FGM of "Yellow team wins") |
| `extend_results_screen_series_logo_color_index_`, `..._background_palette_index_`, `..._column_color_index_` (reuse `team_panel_table`) | `MNVSRESULTS_TEAM_COLOR` in the series logo, the wallpaper and `mnVSResultsGetNumberColorID` |
| winner calculation (`slti at, s1, 3 + NUM_NEW_TEAMS`, 0x80137240) | `mnVSResultsSetTeamPlaceAll` loop bound `MNVSRESULTS_TEAM_LAST` (`mnRemixTeamsRulesLastResultTeam`) |
| `ComboMeter.team_offset_table`, `Spawn.valid_teams` | already had the yellow row (`dIFRemixComboTeamSets`, `sGMRemixValidTeams`); nothing to add |
| `Shield.color_fix_.table_team` (and `table_ffa`) | `ftRemixShieldGetDefaultEnvColor` in `ftremixshield.c` (see the behaviour change below) |

Also checked, no change needed: the VS ready check (`mnPlayersVSCheckSingleTeam`) compares team ids; `members[5]` in `ifcommon.c` fits
team 3; `scvsbattle.c` team results are generic; Smashketball and KotH arrays have 4 entries.

## Behaviour changes to know about

- **Default shield colour.** Shield.asm sends option Default (0) through `_teams_check`: team colour in a VS team battle (red, blue, green,
  yellow), black for a CPU, otherwise the port colour table_ffa (red, blue, yellow, green). The earlier port returned the vanilla colour
  for Default (f10_panel_shield.md), so Default shields now change colour for every match, as in the Remix. Vanilla (16), Costume (17),
  colours 1 to 15 and the damage flash are as before.
- **Tag Team team costume** uses the team (`data->team`), not the port colour (a green team has colour 3 and would have taken the yellow row).

## Deviations

- `mnVSResultsMakeWinnerText` x_teams has 3 entries and the Remix does not patch the team branch (only the FFA branch at
  0x8013433C jumps to 0x805370A4), so for yellow it reads past the array: x_teams is copied to sp+0x28 and x_fkinds to sp+0x34,
  so x_teams[3] is x_fkinds[0], 175.0 (same bytes in the US and Remix ROMs). The port uses that value.
- `dIFCommonPlayerTeamColorIDs` is still the vanilla table in `sys/netreplay.c` (netplay replays); a yellow team there would read colour 4.
- The announcer FGM id of yellow is the Remix id read from the table (`FGM.announcer.team.YELLOW`); red, blue and green keep the vanilla ids.

## Env override

`SSB64_VS_TEAMS=1` (existing): P1 and P2 red, the rest blue. New: `SSB64_VS_TEAMS=2` puts P1 on red and every other port on yellow, with the
team costume of each fighter (`ftParamGetCostumeTeamID`) and the yellow colour index.

## Test recipe for the lead (not run)

1. Match: `SSB64_START_SCENE=22 SSB64_VS_FKINDS=0,0 SSB64_VS_TEAMS=2 SSB64_VS_STOCKS=1` with `vs.sh`: P2 must wear Mario's yellow costume (7), a
   yellow shadow under it (red under P1) and the yellow HUD colour; the shield (hold Z) of P1 is red and of P2 yellow (Default option).
   Let P2 die to see the results: "YELLOW" in 0.875 scale with the gold palette, the yellow wallpaper, the yellow announcer call.
2. CSS: `css.sh` with the team button: `SSB64_START_SCENE=16`, click Free-for-all to Team Battle, then the team button of a panel four times:
   red, blue, green, yellow, red (button image and gate colour change at each press); the picked fighter changes costume to its yellow
   costume. For a Sonic panel on yellow, classic Sonic gets costume 1 (`mnRemixSonicClassicGetTeamCostume`).
3. Unit test: `cmake --build <build> --target run_remix_mode_tests` (92 checks of `test_teams_rules.c` + `test_sonic_classic_rules.c` ran as 0
   failed when compiled alone).
