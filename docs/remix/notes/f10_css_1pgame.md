# F10: 1P GAME CSS on the Remix 3x10 grid (ROADMAP T5.1b, screen 1)

File: `decomp/src/mn/mnplayers/mnplayers1pgame.c` (no header change). Model: `mnplayersvs.c` under `REGION_REMIX`.
Shared helpers used instead of copies: `mnremixcss.h` (cells, slot at point, fkind of slot, portrait of fkind),
`mnremixcostumes.h` (`mnRemixCostumeApplyButton`), `mnremixvariants.h` (`mnRemixVariantHandleInput`,
`mnRemixVariantGetFighterKind`). Data from `remix/remix_css_data.h`. Vanilla paths are kept under `#else` or inside
the existing `#ifdef PORT` blocks, so a build without `REGION_REMIX` compiles the same logic.

Not built and not run (lead builds). `scripts/remix/syntax-check.sh` passes with `REGION_REMIX` (the build flags),
and a copy with `-UREGION_REMIX` also passes.

## Edited functions

| Function | Change |
|----------|--------|
| file ids (`dMNPlayers1PGameFileIDs`) | index 11 = `REMIX_CSS_PORTRAITS_FILE_ID`, macro `MN_PLAYERS_1PGAME_REMIX_PORTRAITS_FILE` |
| `sMNPlayers1PGameShade` (new static) | Remix C up/down shade of the held fighter; reset in InitVars, SelectFighterPuck, MakeFighter, UpdateFighter costume reset |
| `mnPlayers1PGameGetFighterKind` | Remix: `mnRemixCssGetFighterKind` |
| `mnPlayers1PGameGetPortrait` | Remix: `mnRemixCssGetPortrait` (variants map to base cell) |
| `mnPlayers1PGameMakeRemixPortrait` (new) | 30 static portraits from the portrait sheet, scale `REMIX_CSS_PORTRAIT_SCALE`, no slide proc (as VS) |
| `mnPlayers1PGameMakePortraitAll` | Remix: loop over the 30 slots |
| `mnPlayers1PGameMakeRemixNameAndEmblem` (new) and `mnPlayers1PGameMakeNameAndEmblem` | Remix logo + name from the Remix tables, offset 0 draws nothing; positions are the vanilla 1P ones (35,144) and (33,202) |
| `mnPlayers1PGameMakeFighter` | Remix: `sMNPlayers1PGameShade = 0` and `ftManagerSetupFilesAllKind(fkind)` before `ftManagerMakeFighter` |
| `mnPlayers1PGameUpdateFighter` | Remix: shade reset where the default costume is re-applied |
| `mnPlayers1PGameSelectFighterPuck` | Remix: shade reset |
| `mnPlayers1PGameMakePortraitFlash` | Remix: flash at cell + 1 |
| `mnPlayers1PGameAnnounceFighter` | Remix: `kRemixCssAnnounceFgm` (0 or out of range plays nothing); vanilla path unchanged |
| `mnPlayers1PGameGetRemixForcePuckFighterKind` (new), `mnPlayers1PGameGetForcePuckFighterKind` | Remix: base fkind of the cell under the puck (no variant), used by hi score and bonus count |
| `mnPlayers1PGameGetRemixPuckFighterKind` (new), `mnPlayers1PGameGetPuckFighterKind` | Remix: cell hit test, crossed/locked check, then `mnRemixVariantGetFighterKind(player, fkind)` |
| `mnPlayers1PGameUpdateCostume` | Remix: `mnRemixCostumeApplyButton` cycles costume and shade; vanilla path reads the default costume as before |
| `mnPlayers1PGameCursorProcUpdate` | Remix: `mnRemixVariantHandleInput(held_player, button_tap)` before the C-button block |
| `mnPlayers1PGameCenterPuckInPortrait` | Remix: cell centre (same offsets as VS) |
| `mnPlayers1PGamePuckAdjustPortraitEdge` | Remix: cell edges and 24x24 size; the vanilla literals 45/43 became `portrait_width`/`portrait_height` (same values) |

## Differences from VS and why

- **Shade lives in a file static**, not in the slot. `MNPlayersSlot1PGame` (`src/mn/mntypes.h`) has no shade field and
  that header is outside this screen's scope. Follow-up: move it into the struct in a wave that owns `mntypes.h`.
- **No costume-used denial** was added: the 1P screen never had one (VS only removed it under Remix).
- **`ftManagerSetupFilesAllKind` is under `REGION_REMIX`**, not `PORT` as in VS, so PORT builds without Remix keep
  their vanilla behaviour.
- **Hover zone unchanged**: `mnPlayers1PGameUpdateCursorNoRecall` treats y in 38..124 as the portrait area; the Remix
  grid is y 44..116, so it stays inside. The cursor clamp (x 0..280, y 10..205) also covers it.
- **Records are fine under Remix**: `GMCOMMON_FIGHTERS_SAVE_NUM` is 97 under `REGION_REMIX`, so
  `spgame_records[fkind]` reads for Remix fkinds stay in range. No guard was needed here.

## Left (not done here)

- **Random select (L trigger)**: the 1P screen has no random pick at all, so there is nothing to convert. VS has it
  (`mnPlayersVSRandFighterKind`). Adding it would be a new 1P feature, not a port of an existing one.
- **Menu music**: VS plays `gmRemixMenuMusicPlay(nSYAudioBGMBattleSelect, FALSE)` under Remix; the 1P screen still
  calls `syAudioPlayBGM`. This is the T4.2 music hook, not part of this task.
- **Vanilla 12-entry tables still used on the vanilla path only**: `mnPlayers1PGameMakePortrait(Shadow)`, the
  slide-portrait tables (`mnPlayers1PGameGetNextPortraitX`, `portrait_pos_x`), `announce_names`, the vanilla fkind and
  portrait tables, and the spotlight `sizes[]`. Under Remix none of these run for fkinds >= 12. The spotlight already
  has a PORT guard (`port_fighter_css_spotlight_scale`) and `kRemixCssCircleSize` is not used here.
- **Stock and level/time option code**: no fkind-indexed tables.
- **Extra costumes beyond `num_costumes`, team colours, variant icons**: as in VS, handled by the shared helpers; not
  checked in game.
- **Not verified in game**: the lead should open the 1P CSS with a Remix fighter (e.g. Falco, 6 costumes), a variant
  (Mario up = Metal) and an announcer line, and confirm the puck clamp and the portrait flash sit on the cell.
