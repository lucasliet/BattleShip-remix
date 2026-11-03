# F8 notes: Special Zoom (BETA) (Toggles.asm entry_special_zoom), source SpecialZoom.asm

Toggle: title `Special Zoom (BETA)`, values 0 = OFF (default in every profile), 1 = MATCH END (zoom on a KO only when the
hit's fighter is on the decisive stock of a two-team match, plus special moves), 2 = ANY KO (zoom on any predicted KO, plus
special moves). Run with `SSB64_REMIX_TOGGLES="Special Zoom (BETA)=2"` (or `=1`). The asm reads the toggle with
`Toggles.read`, no single-player guard, so the API is `gmRemixToggle(special_zoom)`.

## Files written

- `decomp/src/gm/gmremixspecialzoom.c`: zoom state, apply and end, the hit entry `ftRemixSpecialZoomFinalKnockback`,
  update_offset, the camera offset, the slow motion (`ftRemixSpecialZoomSkipsObjectsUpdate`), the wallpaper hook.
- `decomp/src/gm/gmremixspecialzoomko.c`: check_ko (trajectory simulation and blast zones), CheckDecisiveStock,
  CheckPlayersInMatch.
- `decomp/src/gm/gmremixspecialzoomattack.c`: check_special_zoom (attacker fighter kind and status table).
- `decomp/src/gm/gmremixspecialzoomwallpaper.c`: background file 0x04B, wallpaper camera, background object, its
  animation and its hide.
- Declarations appended to `decomp/src/ft/ftremix/ftremixhooks.h` (block "Special Zoom").

## Vanilla patches (asm patch, RAM, vanilla function, status)

1. `final_knockback_was_set`, `OS.patch_start(0xBBF24, 0x801414E4)`: `ftCommonDamageInitDamageVars` (`ft/ftcommon/ftcommondamage.c`).
   Applied: `#ifdef REGION_REMIX` call `ftRemixSpecialZoomFinalKnockback(this_fp, damage_player_num)` right before
   `this_fp->is_hitstun = TRUE;` (the asm's `ori t6,t7,1 / sb 0x18F` is that line).
2. `create_zoom_camera`, `OS.patch_start(0x89410, 0x8010DC10)`: `gmCameraMakeWallpaperCamera` (`gm/gmcamera.c`).
   Applied: call `ftRemixSpecialZoomMakeWallpaper()` before the return.
3. `zoomed_in_camera_offset`, `OS.patch_start(0x881BC, 0x8010C9BC)`: `gmCameraUpdatePlayerZoom` (`gm/gmcamera.c`).
   Applied: call `ftRemixSpecialZoomCameraOffset(&pos)` after the Size cam offset, before `gmCameraAdjustFOV`.
4. `objects_update`, `OS.patch_start(0xB1EC, 0x8000A5EC)`: `gcRunAll` (`sys/objman.c`).
   Applied: `if (ftRemixSpecialZoomSkipsObjectsUpdate()) return;` at the top of the body (slow motion).
5. `hat_test`, `hat_test_2` (asm lines 20-100): commented out in the source. Skipped (dead code).
6. `OS.copy_segment(0x8F08C, 0x4 * 36)` in `CheckDecisiveStock` (asm line 1485): decoded with
   `tools/remix_disasm_rom.py`; it is the live-teammates loop of `ifCommonBattleUpdateScoreStocks`
   (`if/ifcommon.c`). Ported as the same loop in C (`ftRemixSpecialZoomCountTeammates`).
7. Shared state, not a patch: `ifRemixBattlePlace()` added to `if/ifcommon.c` under `REGION_REMIX`. It returns
   `sIFCommonBattlePlace` (RAM 0x801317F4, the `gBattlePlacement` that the asm reads), which is static in the vanilla file.

## Routines of the scope (all ported)

check_ko, CheckDecisiveStock, CheckPlayersInMatch, check_special_zoom, apply_zoom, end_zoom, update_offset,
zoomed_in_camera_offset body, objects_update body, create_zoom_camera body, create_gfx (background show), background
update (every odd game update), and the final knockback body. Commented asm and `temp_toggle_group_display_` see below.

## Vanilla functions used by the zoom (asm call -> decomp function)

- `0x8010CF44` gmCameraSetStatusPlayerZoom, `0x8010CEF4` gmCameraSetStatus, `0x8010CECC` gmCameraRunFuncCamera,
  `0x8010CCC0` gmCameraMapZoomFuncCamera: vanilla_funcs.txt exact.
- `0x80009968` gcMakeGObjSPAfter, `0x80009DF4` gcAddGObjDisplay, `0x80008188` gcAddGObjProcess, `0x80014038`
  gcDrawDObjTreeForGObj, `0x80017EC0` func_80017EC0, `0x80007080` syRdpSetViewport, `0x800E7ED4` ftParamGetPlayerNumGObj,
  `0x800269C0` (FGM.play) func_800269C0_275C0: vanilla_funcs.txt exact.
- `0x8000B93C` gcMakeCameraGObj (`sys/objhelper.c:563`), `0x80009A84` gcEjectGObj (Render.DESTROY_OBJECT_): not exact
  entries in vanilla_funcs.txt. Identified by the signature and by the Render.asm macro comment, and the call arguments
  match the C signatures.
- `0x8000DF34` gcPlayAnimAll: the same identification as `ftremixsamusshared2.c` (its note). Not in vanilla_funcs.txt.
- `0x8000F120` gcSetupCommonDObjs (obj, DObjDesc, NULL): identified by the three-argument shape with a2 = 0, NOT in
  vanilla_funcs.txt. Lead: confirm the address if the wallpaper DObjs look wrong.
- `0x8000F8F4` gcAddMObjAll: the grremix notes name this address; not in vanilla_funcs.txt.
- `0x8000BE28` gcAddAnimJointAll (obj, AObjEvent32 **, 0.0F): identified by the signature. Not in vanilla_funcs.txt.
- `Render.load_file_` (`0x800CDEEC` size, `0x800CDE04` load, `0x80004980` malloc) ported as
  `lbRelocGetExternHeapFile(0x04B, syTaskmanMalloc(lbRelocGetFileSize(0x04B), 0x10))`, the pattern of `ftremixkirbyhats.c`.
- `atan2f`, `__cosf`, `__sinf`: the libm and decomp names of `0x8001863C`, `0x80035CD0`, `0x800303F0` (they are not
  vanilla_funcs.txt entries).

## REMIX-TODO markers (no port yet)

- RESOLVED (M11 I1, `gmRemixGroupDisplayTempSet`): `temp_toggle_group_display_` (asm lines 917, 922, 1125, 1130) hides and shows the stage groups 1
  and 2 through `Render.GROUP_TABLE` (RAM 0x800466F0). Those lists are walked with the Remix `link_next` (0x4) and the
  GObj flags (0x7C) and have no port. Effect: with the toggle on, the stage is NOT hidden during a KO zoom.
- REMIX-TODO(Stamina): `check_gamemode_2` (asm line 141) skips Stamina battles through `Stamina.VS_MODE` (RAM
  0x800A4D0B, Stamina.asm:16). No port. Effect: a zoom can also start in a Stamina battle.
- REMIX-TODO(unknown global): `create_gfx` (asm line 1016) sets `dobj->rotate.vec.f.y` from the float at RAM
  0x80132704. No port names it, so the DObj keeps its default rotation. Lead: read the value if the background looks
  rotated wrong.

## Mapping choices to check

- Roy's `DSP_Ground_Strong_End` = 0xF2 and `DSP_Air_Strong_End` = 0xF6 (asm lines 1426, 1430): by Roy.asm add order, the
  same as `4A_ROY_DSP.md`.
- Ids: vanilla kinds use `nFTKind*` (Captain, Donkey, Luigi, Kirby, Purin, Ness), the Remix clones use `REMIX_ID_*`.
- Luigi's SuperJumpPunch counts only on the first animation frame (`status_total_tics == 1`, asm line `lw t2, 0x001C`).
- Camera "NORMAL" is 0 (Camera.asm:24). The screens are 0x01 (title and 1P), 0x16 (VS), 0x36 (training), 0x77 (Remix
  modes), from Global.asm.
- Pause: `SCBattleState.game_status >= 2` (asm `sltiu t0,t0,2`, match_info + 0x11).
- The hit zoom runs before `is_hitstun` is set, like the asm.

## What to see with the toggle on (`Special Zoom (BETA)=2`)

- A hit that is a predicted KO (high knockback near a blast zone, KO prediction in `check_ko`): the camera zooms in on
  the hit fighter, the game runs in slow motion (one object update every 16 game updates) for 40 game updates, and the
  wallpaper background appears behind the fighters. The two FGM sounds (152 and 187) play. The stage is not hidden (REMIX-TODO).
- A special move hit from the attacker list (Captain Falcon and Kirby punches, DK, Luigi Super Jump Punch on frame 1,
  Ganondorf Warlock Punch and Up Tilt, Jigglypuff Rest, Ness and Lucas PK Thunder, Roy DSP end): the zoom runs only in a
  two-player match for the camera zoom-in; the slow motion and the background run in any match.
- `=1`: only KO zooms on a decisive stock of a two-team match; special move zooms still run.
- `=0`: nothing runs and the vanilla path is untouched.

Not built and not run (per the brief). Syntax checks: `syntax-check OK` for the four new files, `if/ifcommon.c`,
`ft/ftcommon/ftcommondamage.c`, `gm/gmcamera.c` and `sys/objman.c`.
