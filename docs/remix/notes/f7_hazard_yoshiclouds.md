# F7 hazard notes: Yoshi's Island clouds and platform_movement_

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 603-719 (the barrel routine at 713 is another scope).

## Patches

| Asm patch | RAM | Vanilla function | Port | Status |
|-----------|-----|------------------|------|--------|
| `yoshis_island_clouds_` (ROM 0x84488) | 0x80108C88 | `grYosterMakeGround` (gryoster.c) | `grRemixYosterDisableCloudClipping()` at the top of the function, after `gcMakeGObjSPAfter` | Applied, see deviation 1 |
| `yoshis_island_cloud_anims_` (ROM 0x842F4) | 0x80108AF4 | `grYosterInitAll` (gryoster.c), matrix kind arg of `gcSetupCustomDObjs` | `grRemixYosterCloudMatrixKind()` | Applied, returns the vanilla kind while the toggle is off |
| `platform_movement_` (ROM 0x80DA8) | 0x801055A8 | `grDisplayMakeGeometryLayer` (grdisplay.c), `gcAddGObjProcess(proc_update)` in the anim branch | `grRemixPlatformMovementIsOn()` guards that call | Applied, see deviation 2 |

The address 0x801055A8 is not in `build/remixasm/refs/vanilla_funcs.txt` (its last entry below it is `grDisplayMakeGeometryLayer` at 0x8010547C). The owner was identified from the `jal 0x80008188` (`gcAddGObjProcess`) and the `lw a1, 0x0C(v0)` (a `proc_update` table entry).

## Logic

- Clouds: `Toggles.entry_hazard_mode` bit 0 set (hazards off) sets `DObj.user_data.s = 4` on yakumono DObjs 1, 2 and 3 (`gMPCollisionYakumonoDObjs->dobjs[]`, the same line ids as `dGRYosterCloudLineIDs`). The port uses `grRemixHazardsAreOn()`. The asm does not check the screen for clouds.
- Cloud anims: the asm sets the matrix kind to `nGCMatrixKindTraRotRpyRSca` (0x1C) when `Toggles.entry_yi_clouds` is on, otherwise `nGCMatrixKindTra` (0x12). The toggle defaults to OFF (`Toggles.asm` `entry_yi_clouds`), so the default port behaviour is vanilla.
- Platform movement, in the order of the asm:
  1. Not VS (0x16) or Training (0x36): init (TRUE). Covered by `grRemixHazardToggleIsOn()` / `grRemixMovementToggleIsOn()`.
  2. Onett (0x49), Tal Tal (0x2E) and Tal Tal Remix (0xA7): init iff hazards are on, `grRemixHazardToggleIsOn()`.
  3. Other stages: init iff movement is on, `grRemixMovementToggleIsOn()`.
  4. `Stages.dont_freeze_stage` set: init. Not ported, see TODO 1.

## Deviations and uncertainties

1. Clouds, asm line 627: the hazards-off branch jumps to `0x80108CBC` ("skip to the end of the function"), which skips the rest of `grYosterMakeGround`. That would skip the cloud init and the gobj return value. The port applies the clipping change and keeps the vanilla init, because the asm comment says the clouds still exist when hazards are off. The lead should decide this against the real game. The jump target is not mapped to a decomp line.
2. `DObj` offset 0x84 is `user_data` (`struct_offsets.txt`, DObj block). The asm writes 4 there. The comment in `objtypes.h` lists YakumonoID as a possible use of this field, so the write may clash with collision ids. Verify in play that clouds stay solid with hazards off.
3. Cloud clipping is disabled before `grYosterInitAll` in the port. The asm does it before the gobj is made. The order does not change the result because the yakumono DObjs exist from the collision load.

## REMIX-TODO

- `REMIX-TODO(Stages.stage_hazard_table)`: `dont_freeze_stage` (Stages.asm around line 5430, from the Remix `stage_hazard_table`) is not ported. Every stage counts as freezable. For Yoshi's Island (`Hazards.type.HAZARDS`) the asm always initialises the platform movement. This only matters when a movement toggle is off.
- `REMIX-TODO(settings)`: `grRemixYosterCloudScaleIsOn()` is a constant FALSE until the Remix settings menu wires `Toggles.entry_yi_clouds`.

## Files

- New: `decomp/src/gr/grremix/grremixyoshiclouds.c`, `decomp/src/gr/grremix/grremixyoshiclouds.h` (`#ifdef PORT` around the body).
- Edited under `#ifdef REGION_REMIX`: `decomp/src/gr/grcommon/gryoster.c` (2 hooks, 1 include), `decomp/src/gr/grdisplay.c` (1 hook, 1 include).
- Not touched: `grremixhazards.c`, `grremixstage.c`, `grremixids.h`, other groups' files.

## Test (lead)

Stage ids (decimal for `SSB64_VS_STAGE`): Yoshi's Island 5, Yoshi's Island DL 115 (0x73), Yoshi's Island O 119 (0x77), Yoshi's Island II 142 (0x8E), Yoshi's Island Melee 193 (0xC1), Onett 73 (0x49), Tal Tal 46 (0x2E), Tal Tal Remix 167 (0xA7).

Check: with hazard mode 1 (hazards off) the clouds stay solid and visible on Yoshi's Island. On Onett and Tal Tal, movement follows the hazard toggle.
