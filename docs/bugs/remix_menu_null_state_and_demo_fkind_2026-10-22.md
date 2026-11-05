# Remix menus: NULL battle state in the panel guard, Character Data status buffer, Auto-Demo null fighter

Code: `decomp/src/mn/mndata/mncharacters.c`, `decomp/src/gm/gmremixtoggle.c`, `decomp/src/sc/sccommon/scautodemo.c`. Test:
`SSB64_START_SCENE=26` (Character Data) and `SSB64_START_SCENE=61` (Auto-Demo) with `SSB64_MAX_FRAMES=500`
(`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptoms and root causes

1. **Character Data (scene 0x1A) SIGABRT** with `Relocatable Data Manager: Status Buffer is full !!` in a loop. The screen
   preloads the playable fighters; with the Remix roster each fighter loads its main file plus extern dependencies and the
   vanilla 100/7-entry status buffers overflow.
2. **Character Data SIGSEGV** in `gmRemixToggleModeIsGuarded`: the guard (used by `gmRemixPanel` and the toggles) read
   `gSCManagerBattleState->game_type`, which is NULL outside a match (the screen builds a fighter through
   `ftManagerMakeFighter`). Same family as `remix_css_camera_zoom_null_battle_state_2026-10-20.md`.
3. **Auto-Demo SIGSEGV** in `ftManagerSetupFilesAllKind(28)`: the scene data starts with `nFTKindNull` in both demo slots and
   only the title fills them, so entering the demo from another scene (idle on a data screen, `SSB64_START_SCENE=61`) read the
   file table of the sentinel.

## Fixes

1. `mncharacters.c`: under `REGION_REMIX` the status buffer is 256 entries and the force buffer 64, like the VS CSS.
2. `gmremixtoggle.c`: `gmRemixToggleModeIsGuarded` returns FALSE when there is no battle state.
3. `scautodemo.c` (`scAutoDemoInitDemo`): a `nFTKindNull` demo slot becomes Mario (slot 0) or Fox (slot 1).

## Verification

Character Data pages Mario, Bowser, DK, Link, Sheik, Yoshi and Kirby without error; `SSB64_START_SCENE=61` runs 500 frames.
