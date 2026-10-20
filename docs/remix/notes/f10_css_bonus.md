# F10: Remix 3x10 grid in the bonus CSS (Break the Targets / Board the Platforms)

Screen: `decomp/src/mn/mnplayers/mnplayers1pbonus.c` (+ `mnplayers1pbonus.h`). Model: the VS screen
(`mnplayersvs.c`), using the shared helpers `mnremixcss.c`, `mnremixcostumes.c`, `mnremixvariants.c`. Everything new is
under `#ifdef REGION_REMIX` with the vanilla code in `#else`. Not built, not committed. Syntax-checked with
`scripts/remix/syntax-check.sh` (REMIX flags) and with a manual `-fsyntax-only` of the same flags minus
`-DREGION_REMIX=1` (vanilla path): both clean.

## Edited

| Function / block | Change |
|------------------|--------|
| includes | remix_css_data.h, mnremixcss.h, mnremixcostumes.h, mnremixvariants.h |
| `dMNPlayers1PBonusFileIDs` | `REMIX_CSS_PORTRAITS_FILE_ID` at index 11, macro `MNPLAYERS1PBONUS_REMIX_PORTRAITS_FILE` |
| `sMNPlayers1PBonusShade` (new global) | Remix C up/down shade; reset to 0 when the hovered fighter changes and on select |
| `mnPlayers1PBonusGetFighterKind` | slot to fkind via `mnRemixCssGetFighterKind` |
| `mnPlayers1PBonusGetPortrait` | fkind to cell via `mnRemixCssGetPortrait` (variants map to their base cell) |
| `mnPlayers1PBonusMakeRemixPortrait` (new) | 30 cell portraits from file 11, as `mnPlayersVSMakeRemixPortrait` |
| `mnPlayers1PBonusMakePortraitAll` | Remix: loop over the 30 slots |
| `mnPlayers1PBonusMakeRemixNameAndEmblem` (new) | logo (FTEmblem file) and name (common file) from the Remix tables, "offset 0 draws nothing" |
| `mnPlayers1PBonusMakeNameAndEmblem` | Remix dispatch to the function above |
| `mnPlayers1PBonusMakePortraitFlash` | flash at the cell (`+1`, as VS) |
| `mnPlayers1PBonusAnnounceFighter` | plays `kRemixCssAnnounceFgm`; synth fkinds without an FGM play nothing |
| `mnPlayers1PBonusGetRemixForcePuckFighterKind` (new) | puck centre to slot to fkind, no lock/cross check (vanilla force semantics) |
| `mnPlayers1PBonusGetForcePuckFighterKind` | Remix dispatch |
| `mnPlayers1PBonusGetRemixPuckFighterKind` (new) | puck centre to slot, crossed/locked gives null, D-pad variant via `mnRemixVariantGetFighterKind(player, fkind)` |
| `mnPlayers1PBonusGetPuckFighterKind` | Remix dispatch |
| `mnPlayers1PBonusPuckAdjustPortraitEdge` | edges from the cell, width/height 24; vanilla keeps 45/43 in the same expressions |
| `mnPlayers1PBonusUpdateCostume` | Remix: `mnRemixCostumeApplyButton` on the current costume and shade, no vanilla shade reset |
| `mnPlayers1PBonusMakeFighter` | `desc.shade`, and `ftManagerSetupFilesAllKind(fkind)` before `ftManagerMakeFighter` (loaded on first hover) |
| `mnPlayers1PBonusCursorProcUpdate` | `mnRemixVariantHandleInput(held_player, button_tap)` after the cursor adjust |
| `mnPlayers1PBonusPuckProcUpdate` | shade reset when the hovered fkind changes |
| `mnPlayers1PBonusSelectFighterPuck` | shade reset on select |
| `mnPlayers1PBonusGetBestTime`, `GetBestTaskCount`, `CheckBonusComplete` | Remix fkinds (>= `GMCOMMON_FIGHTERS_SAVE_NUM`) have no row in the 12-entry `spgame_records`: return 0 / FALSE. Same guard as `sc1PBonusStageWriteBackup` and `sc1pmanager.c` |
| `mnplayers1pbonus.h` | declarations of the four new functions, under `REGION_REMIX` |

## Differences from VS and why

- No random fighter here: this screen has no Random Select, so no `mnPlayersVSRandFighterKind` equivalent.
- No centre-puck function: the bonus puck is free-floating (it is placed by the cursor, then recalled), so no
  `CenterPuckInPortrait` is needed.
- Portraits: only the sprite and the scale, as VS. The vanilla wallpaper fire background, the lock shadow and the cross
  are not drawn for Remix cells (`mnPlayers1PBonusCheckFighterLocked` returns FALSE for every Remix fkind anyway).
- Name and logo keep the bonus positions (68/144 logo, 66/202 name, black logo), not the VS per-player positions.
- Shade: VS keeps a per-player shade in its slot struct. The bonus slot struct is shared (`mndef.h`), so the shade is a
  screen-local global instead, reset wherever the costume is reset to the default.
- Costume: no `mnPlayers1PBonusGetCostume` change; `ftParamGetCostumeCommonID` already reads the Remix table for every
  fkind, so the vanilla select path (`SelectFighterPuck`) stays as is.
- Variant input is keyed by `held_player` of the single bonus slot (`-1` when the puck is not held, which the variant
  helpers ignore), not by a per-player slot as in VS.

## Known gaps (left on purpose)

- Force puck (`GetRemixForcePuckFighterKind`, used for the best time / task count display) returns the base fkind under
  the puck, not the D-pad variant, because it has no player argument. Records do not exist for Remix fkinds anyway, so
  the display is 0 for them; the only visible effect is a base fighter's record shown while a variant is on the puck.
- Remix fighters show 0 best time / 0 tasks (no record). Totals (`GetTotalMins/Sec/CSec`), `CheckBonusCompleteAll` and
  the FuncStart preload still loop `nFTKindPlayableStart..nFTKindPlayableEnd` (the 12 vanilla fighters). Remix fighters
  do not count toward the total time and are loaded on first hover.
- Spotlight `sizes[]` and `announce_names[]` (vanilla arrays) are only read on the vanilla path under REMIX; the PORT
  fallbacks (`port_fighter_css_spotlight_scale`) stay for any out-of-range fkind.
- Vanilla `mnPlayers1PBonusMakePortrait`, `MakePortraitShadow`, `PortraitProcUpdate` and the vanilla
  `GetFighterKind`/`GetPortrait` tables stay as they were; they are not called under REMIX.
- `announce_names[]` and the `pos[]`/`colors` arrays of the vanilla name function are unused under REMIX (no warning with
  the project flags: `-Wno-unused-variable`).
- Not built or run: the lead needs to build and check the bonus screen in game (hover, select, C-button costume and
  shade, D-pad variant, the best-time display for a vanilla fighter).

## Memory

ai-memory: global rules read in full (`_rules/no-agent-commit-attribution.md`, `no-live-provider-tests.md`,
`ai-memory-operational-recall.md`). The project scope could not be resolved from `.ai-memory.toml` (no file at the repo
root) and `workspace "JRickey" / project "BattleShip"` was rejected, so project `_rules/` were not confirmed.
