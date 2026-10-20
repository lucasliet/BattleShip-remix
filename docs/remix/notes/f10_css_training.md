# F10 notes: Remix character grid on the 1P training CSS

Scope: `decomp/src/mn/mnplayers/mnplayers1ptraining.c` only (no header change). Model: `mnplayersvs.c` under `REGION_REMIX`.
Shared helpers used: `mnremixcss.h` (cells, slot lookup, slot to fkind, fkind to portrait), `mnremixcostumes.h`
(`mnRemixCostumeApplyButton`), `mnremixvariants.h` (`mnRemixVariantHandleInput`, `mnRemixVariantGetFighterKind`,
`mnRemixRandomResolveFighterKind`), and `port/remix/remix_css_data.h`.

Status: syntax-check OK with the game flags (REGION_REMIX=1). Syntax-only compile without REGION_REMIX also passes. Not built, not run.

## Edited functions

- Top of file: includes under `REGION_REMIX`, macro `MN_PLAYERS_1P_TRAINING_REMIX_PORTRAITS_FILE 8`.
- `dMNPlayers1PTrainingFileIDs`: appends `REMIX_CSS_PORTRAITS_FILE_ID` at index 8 under `REGION_REMIX`.
- `sMNPlayers1PTrainingShade[GMCOMMON_PLAYERS_MAX]` (new static, Remix only).
- `mnPlayers1PTrainingGetFighterKind`, `mnPlayers1PTrainingGetPortrait`: Remix dispatch to `mnRemixCssGetFighterKind` / `mnRemixCssGetPortrait`.
- `mnPlayers1PTrainingMakeRemixPortrait` (new): one static sprite per slot, from the Remix sheet.
- `mnPlayers1PTrainingMakePortraitAll`: loops the 30 slots under Remix.
- `mnPlayers1PTrainingMakeRemixNameAndEmblem` (new): logo from `kRemixCssLogoOffset` (file 3), name from `kRemixCssNameOffset` (file 0), training positions kept.
- `mnPlayers1PTrainingMakeNameAndEmblem`: dispatches to the Remix version, vanilla body unchanged in `#else`.
- `mnPlayers1PTrainingMakePortraitFlash`: cell position under Remix.
- `mnPlayers1PTrainingAnnounceFighter`: `kRemixCssAnnounceFgm` under Remix.
- `mnPlayers1PTrainingSelectFighterPuck`: no costume-used denial under Remix, shade reset to 0.
- `mnPlayers1PTrainingMakeFighter`: `desc.shade` from the stored shade; `ftManagerSetupFilesAllKind(fkind)` on first hover under Remix.
- `mnPlayers1PTrainingUpdateFighter`: stored shade passed to `ftParamInitAllParts` under Remix.
- `mnPlayers1PTrainingRandFighterKind` (new): Remix CPU random over the 30 slots through `mnRemixRandomResolveFighterKind`.
- `mnPlayers1PTrainingInitVars`: uses the Remix random; zeroes the stored shade.
- `mnPlayers1PTrainingCursorProcUpdate`: `mnRemixVariantHandleInput` on the man's button_tap for the held slot.
- `mnPlayers1PTrainingUpdateCostume`: `mnRemixCostumeApplyButton` (C right/left costume, up/down shade), no denial.
- `mnPlayers1PTrainingGetRemixPuckFighterKind` (new) and `mnPlayers1PTrainingGetPuckFighterKind`: grid hit test through `mnRemixCssSlotAtPoint`, variant via `mnRemixVariantGetFighterKind`.
- `mnPlayers1PTrainingCenterPuckInPortrait`: cell centre under Remix.
- `mnPlayers1PTrainingPuckAdjustPortraitEdge`: cell edges and sizes under Remix. Vanilla literals 45/43 moved into `portrait_width`/`portrait_height` locals with the same values.

## Differences from VS and why

- No slide or wallpaper: the Remix portrait is a static sprite, as in VS. The training slide process (`PortraitProcUpdate`), wallpaper, cross and shadow are not created under Remix. `MakePortrait` and `MakePortraitShadow` still compile but are unreachable under Remix (they index 12-entry tables by fkind).
- Name and emblem: the training layout (man at x 63/61, CPU at 195/193) is kept, not the VS layout. Differs from VS in one way: the old sprites are removed before the "offset 0 draws nothing" check, so a fighter without a name or logo does not keep the previous fighter's text. VS returns before the removal.
- Shade: the training slot struct (`mntypes.h`) has no shade field and that header is outside this scope, so the shade lives in `sMNPlayers1PTrainingShade`. Move it into `MNPlayersSlotTraining` when `mntypes.h` is in scope.
- The CPU keeps the vanilla "not the same costume as the man" check in `InitVars` (`CheckCostumeUsed`). Only the button-press denial is dropped, as in VS.
- Costume on fkind change: `UpdateFighter` still calls `mnPlayers1PTrainingGetFreeCostume` (the PORT path, same as VS), so a newly hovered fighter gets the lowest costume not used by the opponent. The C-button costume chosen later is kept while the fighter stays the same.
- Random: the training CPU random is unconditional (vanilla behaviour). VS's random is behind the `l_random_char` toggle, which only the VS L-trigger uses. The training screen has no L-trigger random.
- D-pad variants: training reads only the man's controller (the CPU cursor is never created), so the variant call uses `gSYControllerDevices[sMNPlayers1PTrainingManPlayer]`.
- Cursor bounds (x 0 to 280, y 10 to 205) and the off-grid threshold (y below 38 or above 124) are unchanged, as in VS.

## 12-bound sites reviewed

- `sizes[]` in `mnPlayers1PTrainingSpotlightProcUpdate`: already has the PORT fallback to `port_fighter_css_spotlight_scale`, so Remix fighters are covered.
- `announce_names[]`: Remix branch added (see above).
- `portraits[]` in the vanilla `GetPortrait`: bypassed under Remix.
- `mnPlayers1PTrainingGetNextPortraitX` and `PortraitProcUpdate`: 12-entry tables, not reached under Remix (no slide process).
- `dMNPlayers1PTrainingUnknown0x80138070`, `mnPlayers1PTrainingCheckFighterLocked` (bit mask by fkind): the switch only tests the four vanilla fkinds, so Remix fkinds fall to `default` (not locked). Not changed.

## Left open

- Shade is not reset when the slot changes fkind (`UpdateFighter` keeps the stored value). Each shade is 0 to 2, valid for every fighter, but a new fighter may open with the previous shade. Decide whether to reset it on fkind change.
- `mnPlayers1PTrainingFuncStart` still preloads only the vanilla playables; Remix fighters load on first hover (`ftManagerSetupFilesAllKind`). Check the status buffer size with the lead's smoke run (PORT already uses 256 entries).
- New functions are not declared in `mnplayers1ptraining.h` (`MakeRemixPortrait`, `MakeRemixNameAndEmblem`, `RandFighterKind`, `GetRemixPuckFighterKind`). Add them if the lead wants header coverage.
- `mnPlayers1PTrainingMakeCursor` and the `pos` arrays are unchanged. The cursor keeps the vanilla 1P bounds.
- Not built, not run. The lead needs a smoke run: grid of 30 portraits, name and logo on hover, D-pad variants, C-button costume and shade cycling, CPU random, announce sound.
- ai-memory: the project-level rules could not be loaded. The repository has no `.ai-memory.toml`, and `JRickey/BattleShip` is not a workspace/project name the server knows. The global rules were read. No build or commit was done.
