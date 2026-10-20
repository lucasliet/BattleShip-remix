# F10 worker brief: Remix character grid in one CSS screen (ROADMAP T5.1b)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Use absolute paths and `builtin cd`. Read
`docs/remix/f7_leftover_worker_brief.md` first (rules, final report). You only write code and one notes file. The lead builds
and runs the game; do not build, do not commit, no attribution trailers anywhere.

## Goal

Your screen still shows the vanilla 12-fighter grid. Give it the Remix 3x10 grid (30 portraits) exactly like the VS screen
already has it, so every Remix fighter can be picked, shown with portrait, name and logo, centred by the token, and so the
screen never reads a 12-entry table with a fkind above 11 (fkinds are Remix ids, up to 0x60).

## Model to copy

The VS screen: `decomp/src/mn/mnplayers/mnplayersvs.c`. Every `#ifdef REGION_REMIX` block in it is part of the pattern. The
original diff is `git -C decomp show 0c5ad918a -- src/mn/mnplayers/mnplayersvs.c` (grid, portraits, name and emblem, flash,
announce, puck fighter kind, centre puck, random). Later additions are in the current file: the costume cycle
(`mnPlayersVSUpdateCostume`), D-pad variants (`mnRemixVariantHandleInput` call in the cursor update, applied in
`mnPlayersVSGetRemixPuckFighterKind`), the Random Select setting in `mnPlayersVSRandFighterKind`.

Shared helpers already exist, use them instead of copying the VS `static`-style helpers:

- `mn/mnplayers/mnremixcss.h`: `mnRemixCssCellX/Y(slot)`, `mnRemixCssSlotAtPoint`, `mnRemixCssGetFighterKind(slot)`,
  `mnRemixCssGetPortrait(fkind)` (variants map to their base fighter's cell).
- `mn/mnplayers/mnremixcostumes.h`: `mnRemixCostumeApplyButton(fkind, button, &costume, &shade)` (C right/left costume, C
  up/down shade), `mnRemixCostumeGetDefaultRoyal/Team`. `ftParamGetCostumeCommonID/TeamID` already read the Remix table
  for every fkind, so the `fkind >= nFTKindEnumCount` guards of VS are not needed.
- `mn/mnplayers/mnremixvariants.h`: `mnRemixVariantHandleInput(held_player, button_tap)`,
  `mnRemixVariantGetFighterKind(held_player, fkind)`, `mnRemixRandomResolveFighterKind(fkind)`.
- Data: `port/remix/remix_css_data.h` (`kRemixCssSlotIds`, `kRemixCssPortraitOffset`, `kRemixCssNameOffset`,
  `kRemixCssLogoOffset`, `kRemixCssAnnounceFgm`, `REMIX_CSS_PORTRAITS_FILE_ID`, `REMIX_CSS_NUM_*`).

## What to do in your screen file

1. Add the portrait file id to the screen's file list under `REGION_REMIX` (VS has it as index 8; use the next free index and
   remember the index in a macro).
2. Portraits: make the 30 portraits (the VS `mnPlayersVSMakeRemixPortrait`), replace the vanilla portrait creation.
3. Every table indexed by fkind with 12 entries (portraits, fkinds, names, logos, announce, poses) gets a Remix branch; name
   and logo come from the Remix tables with the "offset 0 draws nothing" rule.
4. Token position to fkind (`...GetPuckFighterKind` or the screen's equivalent), fighter kind to token position
   (`...CenterPuckInPortrait`), portrait flash position, random fighter, the in-grid hit tests.
5. Costume buttons: use `mnRemixCostumeApplyButton` and drop the costume-used denial, as in VS (`mnPlayersVSUpdateCostume`).
6. D-pad variants: call `mnRemixVariantHandleInput` where the screen reads the held token's controller and wrap the fkind
   under the token with `mnRemixVariantGetFighterKind`.
7. When a Remix fighter is hovered for the first time call `ftManagerSetupFilesAllKind(fkind)` before `ftManagerMakeFighter`
   (VS `mnPlayersVSMakeFighter` does this).
8. Look for other places that index by fkind with a bound of 12 or `nFTKindPlayableEnd` (grep the file) and note them.

Vanilla behaviour must stay identical without `REGION_REMIX` (guard with `#ifdef REGION_REMIX` / `#else`).

Write `docs/remix/notes/f10_css_<screen>.md`: the list of edited functions, what differs from VS and why, anything left. Final
report under 30 lines.
