# F8 notes: Camera Mode, Cinematic Entry, Disable HUD (Camera.asm)

Source asm: `.toolchain/pkgs/smashremix-src/src/Camera.asm`. Port: `decomp/src/gm/gmremixcameramode.c`,
`decomp/src/gm/gmremixcamerapause.c`, hooks in `decomp/src/gm/gmcamera.c` and `decomp/src/if/ifcommon.c` (`#ifdef REGION_REMIX`),
prototypes appended to `decomp/src/ft/ftremix/ftremixhooks.h`. Already ported before this worker: `override_camera_type_`
and `camera_adjust_` (grremixcamera.c), and the Venom / GB Land zoom.

## Settings

| Setting | Toggles.asm entry | Title (`SSB64_REMIX_TOGGLES`) | Values | Read by |
|---|---|---|---|---|
| Camera Mode | `entry_camera_mode` | `Camera Mode` | 0 NORMAL, 1 BONUS, 2 FIXED, 3 SCENE (default 0 in CE/TE/NE/JP) | `gmRemixToggle(camera_mode)` |
| Cinematic Entry | `entry_cinematic_entry` | `Cinematic Entry` | 0 default, 1 always, 2 never (default 0) | `gmRemixToggle(cinematic_entry)` |
| Disable HUD | `entry_disable_hud` | `Disable HUD` | 0 off, 1 pause, 2 all (default 0) | `gmRemixToggle(disable_hud)` |

Hazard Mode (`entry_hazard_mode`, `Hazard Mode`) is read here by the flat zone checks (bit 0). Its own port is T4.3.

The reads are direct (`entry + 4`), so all three use `gmRemixToggle`, not `gmRemixToggleVS`.

## Patches

| Camera.asm | RAM | Vanilla function | Status |
|---|---|---|---|
| 46 `override_camera_type_` | 0x8010DAA4 | gmCameraMakeDefaultCamera | already ported (grRemixCameraDefaultStatus) |
| 107 `set_initial_coordinates_` | 0x8010DA8C | gmCameraMakeDefaultCamera, default branch | applied: `gmRemixCameraSetupDefault` (routine list edits and view for BONUS, FIXED, SCENE) |
| 446 `get_custom_camera_` | (read in gmCameraRunFuncCamera) | gmCameraRunFuncCamera / gmCameraMakeDefaultCamera | applied as `gmRemixCameraApplyFixedRun` (func_camera = anim routine in FIXED), see uncertainty 2 |
| 461 `fixed_camera_boss_` | 0x8010CBEC | gmCameraUpdateAnimVel | applied: gcPlayCamAnim skipped in FIXED |
| 485 `cinematic_entry_` | 0x80112A50 | ifCommonEntryAllThread | applied: `gmRemixCameraCinematicFocus` on the random focus |
| 521 `fixed_camera_always_` | 0x8010CF10 | gmCameraSetStatus | applied: returns before the func store when `gmRemixCameraSetStatusIsFrozen` (status_prev and status_curr still set, as in asm) |
| 571 `prevent_fixed_camera_pause_bonus_` | 0x80114118 | ifCommonBattleGoUpdateInterface (bonus branch) | applied: bonus zoom and kind skipped when `gmRemixCameraPauseBonusZoomAllowed` is FALSE |
| 604 `vs_pause_pan_` | 0x8010C9CC | gmCameraUpdatePlayerZoom | not ported: adds `camera_pan_offsets_`, which only `extended_movement_` writes (not ported, not toggle-gated). With zero offsets it is a no-op |
| 649 `pause_initial_` | 0x801141B4 | ifCommonBattleGoUpdateInterface (else-if chain) | not ported, no observable effect: it writes the pause kind 1 in FIXED, and the default branch assigns the kind again (ifcommon.c, `sIFCommonBattlePauseKindInterface = ...`). The offset clear is a no-op (see uncertainty 6) |
| 844 `unpause_camera_` | 0x80114638 | ifCommonBattlePauseRestoreInterfaceAll | applied: `gmRemixCameraRestorePauseHud` (legend destroy, HUD hidden on ALL or SCENE, magnifier flag). Special Zoom magnifier clear: REMIX-TODO(SpecialZoom) |
| 916 `disable_hud_` | 0x80112228 | ifCommonAnnounceGoSetStatus | applied: `gmRemixCameraHudHiddenOnGo` (ALL) hides the interface and clears the magnifier |
| 945 `frozen_camera_` (4 sites) | 0x8010C5A8, 0x8010C658, 0x8010C6C8, 0x8010C8AC | gmCameraPan, func_ovl2_8010C5C0, gmCameraApplyVel, gmCameraUpdateAcidZoom | applied: `gmRemixCameraFreezeUpdate` replaces the `syVectorAdd3D` of the site (the asm callee 0x80018FBC is `syVectorAdd3D`, see uncertainty 1) |
| 1388 `pause_disable_control_flag_` | 0x80114230 | ifCommonBattleGoUpdateInterface (default branch) | applied: pause kind PlayerNA when `gmRemixCameraPauseControlEnabled` is FALSE |
| 1447 `zoom_to_max_` | (Special Zoom) | none | no toggle read here; left to the Special Zoom task |
| 593 float table (`OS.patch_start`) | 0x80130C94 | efGroundMakeAppearActor (data) | not toggle-gated; "360 control" limits, not ported |

## Tables

Stage ids are the `nGRRemixStage_*` values (`grremixids.h`). Fixed view table: first match wins, the duplicate
SECTOR_Z_DL / SECTOR_Z_O checks of the asm are unreachable and left out. Frozen views: `frozen_world1`, `hrc`,
`fixed_pokefloats`, `gb_land`, `frozen_flat_zone_2`, `frozen_flat_zone`. Bonus pause view: `gMPCollisionGroundData->zoom_start`
(focal) and `zoom_end` (eye), the asm `lh` reads at 0x9A..0xA4.

## What the lead should see

- Camera Mode = 2 (`SSB64_REMIX_TOGGLES="Camera Mode=2"`): on a normal stage, the camera stops following the fighters
  (status changes are ignored) and the pause analog control is off. On Mushroom Kingdom-type stages the camera sits at
  the Remix fixed view; on WORLD1, Pokefloats, GB Land and flat zones the frozen view applies. No countdown cinematic.
  Short match, with pause (Start) during play.
- Camera Mode = 1 (`Camera Mode=1`): the default and Planet Zebes camera routines become the Mushroom Kingdom routine, so
  every stage that takes the default branch (not Planet Zebes or Mushroom Kingdom themselves) uses the Mushroom Kingdom
  camera. The Bonus pause zoom still runs.
- Camera Mode = 3 (`Camera Mode=3`): the camera uses the anim routine on default stages (HRC excluded); no cinematic
  countdown; HUD hidden at unpause.
- Cinematic Entry = 1 (`Cinematic Entry=1`): the countdown always uses the cinematic focus (id 2). `=2` never uses it.
  Requires Camera Mode NORMAL or BONUS (FIXED and SCENE force the default).
- Disable HUD = 2 (`Disable HUD=2`): interface and magnifier hidden after "GO!" and after unpausing.

## Uncertainties

1. `frozen_camera_` (Camera.asm:945) loads the pointer at 0x80131460 (`gGMCameraGObj`, a GObj) and writes x/y/z/focal at
   offsets 0xC4..0xD8. In the decomp those offsets are the CObj `vec.eye` / `vec.at` (`CObjGetStruct(gGMCameraGObj)`), so
   the port writes the CObj. Check that a frozen stage (WORLD1) keeps the camera still in game.
2. `get_custom_camera_` replaces a routine pointer read in gmCameraRunFuncCamera (asm sets it in set_initial at a0+0xA4,
   which the port does not reproduce). The port sets `gGMCameraStruct.func_camera` to `gmCameraAnimFuncCamera` after the
   status routine is chosen, which matches because fixed_camera_always_ blocks later status changes.
3. The asm uses `Global.match_info` stage id; the port uses `gSCManagerBattleState->gkind` (same as grremixcamera.c).
4. Hazard bit 0 comments disagree: `fixed_camera_always_` says "if hazards are disabled" and `frozen_camera_` says "if hazard
   enabled skip". The code branches on bit 0 being set (skip flat zones). The port follows the code.
5. Skipped writes to the pause kind (`0x80131828`) in `frozen_camera_` and `pause_initial_`: every pause start assigns
   `sIFCommonBattlePauseKindInterface` before reading it, so the value written during play is never read.
6. `camera_pan_offsets_` clears in `pause_initial_` and `unpause_camera_`: the offsets are written only by
   `extended_movement_` (not ported), so the clears are no-ops here.
7. `set_initial_coordinates_` edits `dGMCameraFuncList` in place. The asm does the same on RAM that is not reloaded per
   match, so the edits persist across matches on both sides.
8. The Bonus and Scene routine edits are overwritten by the vanilla `func_camera` store after the status is chosen; the
   port keeps only the list edits, which is what the vanilla store reads.

## REMIX-TODO

- `REMIX-TODO(SpecialZoom)` in `gmremixcamerapause.c` (`gmRemixCameraRestorePauseHud`): the magnifier is not cleared while
  the Special Zoom is on (`Zoom.zoom_type`).
