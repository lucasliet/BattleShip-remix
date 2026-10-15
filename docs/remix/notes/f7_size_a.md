# F7 Size phase (a): tables, top joint, ECB and camera

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm` (2724 lines). Phase (a) only: per-player
tables and accessors, top joint, ECB (map collision diamond and ledge box), camera zoom, camera
centre offset and the ECB display. Physics (phase b) and cosmetic GFX (phase c) are not ported.

## Files

- New: `decomp/src/ft/ftremix/ftremixsize.c` (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`).
- Appended block: `decomp/src/ft/ftremix/ftremixhooks.h` (Size block at the end, nothing reordered).
- Vanilla hooks (each under `#ifdef REGION_REMIX`, vanilla line kept in `#else` where it is a
  replacement): `decomp/src/ft/ftmain.c`, `decomp/src/ft/ftmanager.c`, `decomp/src/gm/gmcamera.c`,
  `decomp/src/ft/ftdisplaymain.c`.
- Placeholder: `decomp/src/it/itremix/itremixboomerang.c` (size_multiplier now reads the accessor).

## Accessors (ftremixsize.c, declared in ftremixhooks.h)

- `f32 ftRemixSizeGetMultiplier(s32 player)`: 1.0F for every player until a source sets it.
- `void ftRemixSizeSetMultiplier(s32 player, f32 multiplier)`: no caller yet (no source ported).
- `void ftRemixSizeInitFighter(FTStruct *fp)`: base = `fp->attr->size`, multiplier = 1.0F.
- `void ftRemixSizeApplyTopJointAndEcb(FTStruct *fp)`: top joint scale, ECB, ledge box.
- `f32 ftRemixSizeCameraZoomFactor(FTStruct *fp)`: `1 + (m - 1) * 0.5`.
- `f32 ftRemixSizeScaleValue(FTStruct *fp, f32 value)`: `value * m`, used for the camera offset and ECB display.

The tables are `static f32 sFTRemixSize*[GMCOMMON_PLAYERS_MAX]`, indexed by `fp->player`.
An out-of-range player logs through `port_log` and falls back to 1.0F (getter) or is ignored (setter).

## Vanilla patches (Size.asm RAM address -> decomp function)

Address mapping uses `build/remixasm/refs/vanilla_funcs.txt`, compared numerically.

| RAM | Vanilla function (file) | Size.asm | Status |
|-----|-------------------------|----------|--------|
| 0x800E2068 | ftMainProcPhysicsMap (ft/ftmain.c) | 236-289 adjust_top_joint_and_ecb_ | applied: call after the TopN copy |
| 0x800D7CB4 | ftManagerInitFighter (ft/ftmanager.c) | 338-410 initialize_base_multiplier_table_ | applied for the size part; magic hat part skipped (see below) |
| 0x8010BB9C | gmCameraCalcFighterZoomRange (gm/gmcamera.c) | 1186-1213 adjust_camera_zoom_ | applied: zoom factor before the frame and range multipliers |
| 0x8010BD78 | gmCameraUpdateInterests (gm/gmcamera.c, line ~312) | 1237-1327 adjust_camera_center_point_._1 | applied: `target_pos.y` cam offset |
| 0x8010C9AC | gmCameraUpdatePlayerZoom (gm/gmcamera.c, line ~730) | _5 | applied: `pos.y` cam offset |
| 0x800F2A5C | ftDisplayMainProcDisplay (ft/ftdisplaymain.c) | _2 | applied: `ft_pos.y` cam offset |
| 0x800F2A80 | ftDisplayMainProcDisplay | _3 | applied: compare, both AVOID_UB and #else branches |
| 0x800F2A9C | ftDisplayMainProcDisplay | _4 | applied: scale argument, both branches |
| 0x800F3004 | ftDisplayMainProcDisplay | 163-234 adjust_ecb_display_._lower_y | applied: map_coll.bottom |
| 0x800F3058 | ftDisplayMainProcDisplay | _center_y_and_width | applied: map_coll.center and width |
| 0x800F30F0 | ftDisplayMainProcDisplay | _center_y | applied: map_coll.center |
| 0x800F3148 | ftDisplayMainProcDisplay | _center_y_upper_y_and_width | applied: map_coll.top, center, width |
| 0x8010C9C4 | gmCameraUpdatePlayerZoom | 1215-1235 adjust_camera_zoom_multiplier_ | skipped (uncertain, see U3) |
| 0x800DF394, 0x800DF6AC | ftMainParseMotionEvent (ft/ftmain.c) | 443-465 fix_size_ | skipped: attack hitbox size, see D2 |
| 0x800DF34C | ftMainParseMotionEvent | 470-560 fix_damage_ | skipped: phase (b) damage |
| 0x80160F34, 0x80160F60 | Kirby USP (ft/ftchar/ftkirby, not mapped here) | 293-336 kirby_ausp_ | skipped: writes base table, Kirby USP, not mapped in phase (a) |
| 0x80104078, 0x80111B04, 0x8013B144 | respawn platform, player label, player shadow | 61-127 | skipped: cosmetic / display, phase (c) |
| 0x800E6324 | ftMainProcParams | 128-162 passive_armor_ | skipped: Stamina armor, not Size |
| 0x800D8A74 ... 0x80108DE8 ... | physics and weight (walk, dash, run, jump, weight_1..8, yoshi_egg...) | 563-1185, 1331-1424 | skipped: phase (b) physics and weight, phase (c) gfx |

Applied: 12 of the 73 `OS.patch_start` sites (1 top joint, 1 init, 1 zoom, 1 interests, 1 pzoom
offset, 3 display cam centre, 4 ECB display). The remaining sites belong to physics (b), GFX and
cosmetic (c), or are skipped as listed above.

## Deviations and skipped parts

- D1. Magic hat part of `initialize_base_multiplier_table_` (Size.asm 358-379: `KirbyHats.magic_hat_on`,
  `KirbyHats.taunt_loses_power`, `KirbyHats.spawn_with_hat`). The port has `magic_hat_on` and
  `taunt_loses_power` in `src/ft/ftdef.h` but no `KirbyHats.spawn_with_hat` source was found. Left for
  the Kirby hat owner, `REMIX-TODO(KirbyHat)`. Not in this file.
- D2. `fix_size_` (hitbox size and offset in ftMainParseMotionEvent, Size.asm 443-465). It scales attack
  hitboxes, not the ECB. Deferred to phase (b) with the damage patch. The plan wording "ECB/collision shape"
  was read here as body collision only (map_coll and ledge box).
- D3. Match state (`match_state_table`, GIANT 2.25F / TINY 0.5F, and the how-to-play and demo screen
  check at Size.asm 390-397) is not ported. NORMAL is the only reachable state, so the multiplier is 1.0F.
  The Size menu that sets match states is not ported either.
- D4. `adjust_camera_zoom_multiplier_` (Size.asm 1215-1235) is skipped, see U3.
- D5. `gmCameraUpdatePlayerFollow` (gmcamera.c line ~866, `pos.y += cam_offset_y`) is not patched by the
  asm and stays unscaled.
- D6. In ftdisplaymain.c the N64 branch (`#else` of AVOID_UB, now lines ~1166 and ~1169) is patched, as the asm
  does. The AVOID_UB branch (now lines ~1156 and ~1159) is also scaled, so both builds stay consistent.

## Uncertainties (verify before merging)

- U1. Base table source (Size.asm 343-353, `lwc1 f0, 0(a2)` with `addiu a2, v1, 0xF0` in the delay path).
  Assumed `a2` is `&attr->size`, so base = `attr->size`. If the vanilla top joint scale is normally 1.0 and
  `attr->size` is not 1.0, the top joint write in ftMainProcPhysicsMap changes the rendered body even with
  multiplier 1.0F. The base ROM is not in this tree, so the disassembly could not be checked. Check with
  `tools/remix_disasm_rom.py` at the vanilla RAM 0x800D7CB4 once `baserom.us.z64` is present.
- U2. `vanilla_funcs.txt` line numbers are stale: ftMainProcPhysicsMap is listed at ftmain.c:1877 but is at
  1922 in the current decomp, and ftDisplayMainProcDisplay at 1090 (now 1105). Function names are right.
- U3. `adjust_camera_zoom_multiplier_` (Size.asm 1217-1235): the vanilla value `lwc1 f10, 0x0050(v1)` and
  the struct behind `v1` (camera object, close-up in Training) were not identified in the port, so it is skipped.
- U4. Kirby USP base write (Size.asm 293-336) implies a base table of the Kirby custom multiplier; not
  reached in phase (a), so the base stays `attr->size` for Kirby too.

## REMIX-TODO(Size) sites (grep of decomp/src, after this change)

Accessors listed are what each site should call once its subsystem (or source) is ported. Only the
1.0F placeholder was replaced; the others are comments or constants with no 1.0F read.

| Site | Subsystem | Accessor to use | Replaced now |
|------|-----------|-----------------|--------------|
| itremix/itremixboomerang.c:629 | boomerang catch range | `ftRemixSizeGetMultiplier(fp->player)` | yes, `REGION_REMIX` guard, 1.0F kept in the `#else` |
| itremix/itremixlightning.c:181 | Lightning zap shrink | `ftRemixSizeSetMultiplier(fp->player, 0.5F)` (setter, needs the zap source wired) | no |
| itremix/itremixlightning.c:251 | Lightning shrink timer expiry | `ftRemixSizeSetMultiplier(port, <NORMAL value 1.0F>)` once the match state is ported | no |
| itremix/itremixsupermushroom.c:93 | Super Mushroom tracked size | `ftRemixSizeSetMultiplier(fp->player, size_multiplier)` when the mushroom sets it | no (field, no read) |
| itremix/itremixsupermushroom.c:579 | Super Mushroom size state on pickup | match state accessor (D3), not in phase (a) | no |
| ft/ftremix/crash/ftremixcrashdsp.c:73 | Dig GFX scale | `ftRemixSizeGetMultiplier(fp->player)` on the GFX scale, phase (c) | no |
| ft/ftremix/banjo/ftremixbanjousp.c:140 | Wall bounce graphic | none, `save_player_struct_._wall_bounce` is GFX, phase (c) | no |
| ft/ftremix/goemon/ftremixgoemonusp.c:93 | Cloud render routine | none, `Size.item.render_routine_`, phase (c) | no |
| ft/ftremix/dragonking/ftremixdragonkingnsp.c:165 | Dragon ball render wrapper | none, render wrapper, phase (c) | no |
| ft/ftremix/dragonking/ftremixdragonkingnsp.c:174 | Dragon ball scale | `scale * ftRemixSizeGetMultiplier(fp->player)` at lines 203-207, only once the asm confirms it is applied here (multiplier 1.0F is a no-op) | no |

## For the lead

- Wire: nothing to register. `ftremixsize.c` is picked up by the CMake GLOB of `decomp/src/ft/*.c`.
  The itremix boomerang and the vanilla files include `ft/ftremix/ftremixhooks.h` under `REGION_REMIX`.
- Test: the default is 1.0F everywhere, so the only expected change is the top joint and the ECB write
  (U1). A Remix build should match the previous one for every fighter if `attr->size` is 1.0F; otherwise
  check the top joint scale first.
- Not verified: no build, no game run (per brief). `scripts/remix/syntax-check.sh` returned `syntax-check OK`
  for ftremixsize.c, ftmain.c, ftmanager.c, gmcamera.c, ftdisplaymain.c and itremixboomerang.c.
