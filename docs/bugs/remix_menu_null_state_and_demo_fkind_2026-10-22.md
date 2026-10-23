# Remix menus: NULL battle state in the panel guard, Character Data status buffer, Auto-Demo null fighter

**Status:** FIXED (decomp), 2026-10-22. Found while checking the Remix menus and the per-port panel in game.

## Symptoms and root causes

1. **Character Data (scene 0x1A) SIGABRT** with `Relocatable Data Manager: Status Buffer is full !!` in a loop. The screen preloads
   the playable fighters; with the Remix roster each fighter loads its main file plus extern dependencies and the vanilla
   100/7-entry status buffers overflow.
2. **Character Data SIGSEGV** in `gmRemixToggleModeIsGuarded`: the guard (used by `gmRemixPanel` and the toggles) read
   `gSCManagerBattleState->game_type`, which is NULL outside a match (the screen builds a fighter through
   `ftManagerMakeFighter`). Same family as `remix_css_camera_zoom_null_battle_state_2026-10-20`.
3. **Auto-Demo SIGSEGV** in `ftManagerSetupFilesAllKind(28)`: the scene data starts with `nFTKindNull` in both demo slots and only
   the title fills them, so entering the demo from another scene (idle on a data screen, `SSB64_START_SCENE=61`) read the file
   table of the sentinel.

## Fixes

1. `mncharacters.c`: under `REGION_REMIX` the status buffer is 256 entries and the force buffer 64, like the VS CSS.
2. `gmremixtoggle.c`: `gmRemixToggleModeIsGuarded` returns FALSE when there is no battle state.
3. `scautodemo.c` (`scAutoDemoInitDemo`): a `nFTKindNull` demo slot becomes Mario (slot 0) or Fox (slot 1).

Verified: Character Data pages Mario, Bowser, DK, Link, Sheik, Yoshi and Kirby without error; `SSB64_START_SCENE=61` runs 500 frames.
