# F7 notes: Surface, Blastzone, Camera and Spawn patches

Source asm: `.toolchain/pkgs/smashremix-src/src/{Surface,Blastzone,Camera,Spawn}.asm`. Helpers: `decomp/src/gr/grremix/grremix{surface,blastzone,camera,spawn}.c`
(declared in the appended block of `decomp/src/ft/ftremix/ftremixhooks.h`). Every vanilla call site is under `#ifdef REGION_REMIX`.
Disassembly checked with `tools/remix_disasm_rom.py <rom_offset> <size> --vram <addr>` on `baserom.us.z64`, using the ROM offsets of the asm patch lines.

## Toggles (Remix defaults, from `Toggles.asm`)

| Toggle | Remix default | Port |
|---|---|---|
| `entry_blastzone_warp` | 0 (OFF) | `GRREMIX_BLASTZONE_WARP_TOP_BOTTOM FALSE` (`grremixblastzone.c`), REMIX-TODO(toggles) |
| `entry_camera_mode` | NORMAL | not read by applied patches (skipped patches only) |
| `entry_hazard_mode` | NORMAL | not read by applied patches (skipped patches only) |
| `entry_disable_hud` | OFF | not read by applied patches |
| `entry_cinematic_entry` | 0 | not read by applied patches |
| `entry_neutral_spawns` | ON | not read (Spawn load_spawn_ skipped, see below) |

## Applied patches

| Asm | RAM | Vanilla function (file:line) | Status |
|---|---|---|---|
| Surface.asm:197 `get_friction_` | 0x800D8BDC | `ftPhysicsApplyGroundVelFriction` (ftphysics.c:176) | applied, `grRemixSurfaceFriction` |
| Surface.asm:209 `get_friction_` | 0x800E21DC | `ftMainProcPhysicsMap` damage-ground branch (ftmain.c, +0x194) | applied, `grRemixSurfaceFriction` |
| Blastzone.asm:90 `skip_bottom_blast_sfx` | 0x800E234C | `ftMainProcPhysicsMap` altitude warning, FGM 0x99 = `nSYAudioFGMAltitudeWarn` (ftmain.c) | applied; toggle FALSE = sound plays, as vanilla |
| Camera.asm:46 `override_camera_type_` | 0x8010DAA4 | `gmCameraMakeDefaultCamera` switch `default:` (gmcamera.c, +0x2BC) | applied, `grRemixCameraDefaultStatus` (MK-camera list -> `nGMCameraStatusInishie`, which is camera routine 3) |
| Camera.asm:1343 `camera_adjust_` | 0x800E7270 | `ftMainSetStatus` `camera_zoom_range = 1.0F` (ftmain.c, +0x34C) | applied, `grRemixCameraAdjustZoomRange` (1.75F on Venom / GB Land in VS, Training, 1P game; 1.0F otherwise) |
| Spawn.asm:19 `load_respawn_point_` | 0x800FC8B0 | `mpCollisionGetMapObjPositionID` (mpcollision.c, +0x1C) | applied, `grRemixSpawnRespawnPointOverride` (Dream Land beta 1/2 -> (0, 2526, 0); Sector Z Remix -> (6580, 3150, 0)) |

Disassembly of 0x800FC8B0 (ROM 0x780B0) confirms the vanilla body after the hook writes `pos->x`, `pos->y` from the mapobj shorts and `pos->z = 0`, matching the C.

## Skipped patches

| Asm | RAM | Reason |
|---|---|---|
| Surface.asm:221 `get_friction_` (Kirby down special turn) | 0x8016187C | the friction read is an asm-only inline in `ftKirbySpecialNTurnProcMap` (+0x30); no C site to port. Remix surfaces 0x10+ still index past the vanilla table there. **Gap.** |
| Surface.asm:236 `get_struct_` | 0x800E5C74 | knockback struct table (6 vanilla structs copied from ROM 0xA4530 + new structs) and `_bounce` (on-hit routine, JumpF action, FGM 0x5C2) are not ported. **Gap.** |
| Surface.asm:348 `fgm_override_` | 0x800E3D38 | depends on the knockback struct `fgm` field of `get_struct_`. **Gap.** |
| Surface.asm:382 `zebes_acid_fix_` | 0x80108208 | redirects to the Remix `zebes_acid_struct`; needs the knockback struct port. |
| Surface.asm:394 `fighter_apply_conveyor_surface` | 0x800E23D8 | conveyor push (clipping ids 0x1F..0x22, RTTF stage 0xBD, hazard toggle). Not ported. **Gap: Remix conveyors do not move fighters.** |
| Surface.asm:454 `item_apply_conveyor_surface` | 0x8016F8E4 | same conveyor logic for items, in `itProcessProcItemMain`. Not ported. **Gap.** |
| Blastzone.asm:112/165/213/267/322 `loop_screen_*` | 0x8013CE9C, 0x8013CECC, 0x8013CF20, 0x8013CF30, 0x8013CE68 | BETA warp path behind `entry_blastzone_warp` (default OFF, so vanilla KO runs). Needs `grab_release_check_`, `set_aerial_from_grounded`, `reset_hitbox_interpolation` and the epilogue at 0x8013CF44. Not ported. |
| Camera.asm:462 `fixed_camera_boss_` | 0x8010CBEC | `entry_camera_mode` (default NORMAL). Not ported. |
| Camera.asm:486 `cinematic_entry_` | 0x80112A50 | `entry_cinematic_entry` (default 0). Not ported. |
| Camera.asm:522 `fixed_camera_always_` | 0x8010CF10 | `entry_camera_mode` (default NORMAL). Not ported. |
| Camera.asm:572 / 593 `prevent_fixed_camera_pause_bonus_` | 0x80114118, 0x80130C94 | `entry_camera_mode`; the second hook is unmatched in the decomp. Not ported. |
| Camera.asm:605 `vs_pause_pan_` | 0x8010C9CC | adds the Remix pause-camera offsets (`camera_pan_offsets_`, state written by the Remix pause input, zeroed by `pause_initial_`). With zero offsets it is a no-op; the state is not ported. |
| Camera.asm:650 `pause_initial_` | 0x801141B4 | clears the Remix pause-camera state; that state is not ported. |
| Camera.asm:845 `unpause_camera_` | 0x80114638 | `entry_camera_mode` / `entry_disable_hud`. Not ported. |
| Camera.asm:917 `disable_hud_` | 0x80112228 | `entry_disable_hud` (default OFF). Not ported. |
| Camera.asm:947, 951, 955, 959 `frozen_camera_` | 0x8010C5A8, 0x8010C658, 0x8010C6C8, 0x8010C8AC | `entry_hazard_mode` (default NORMAL). Not ported. |
| Camera.asm:1389 `pause_disable_control_flag_` | 0x80114230 | `entry_camera_mode` / `entry_hazard_mode`. Not ported. |
| Spawn.asm:103 `load_spawn_` | 0x800FAF64 | neutral spawns (VS teams, Smashketball, valid team/singles tables, `entry_neutral_spawns` default ON) plus the Training custom spawn branch (spawn-point editing: out of scope). **Largest gap, default ON.** Not ported. |

## Uncertainties

- Camera.asm:1343 `camera_adjust_` screen gate. The asm tests `Global.current_screen` against 0x16 (VS), 0x36 (Training), 0x77 (Remix special 1P, Allstar/Multiman) and 0x01 ("1p"). The decomp enum `SCKind` gives 0x16 = `nSCKindVSBattle`, 0x36 = `nSCKind1PTrainingMode`, 0x34 = `nSCKind1PGame`, and 0x01 = `nSCKindTitle`, which does not fit the asm comment. The port uses VS, Training and `nSCKind1PGame`. 0x77 has no decomp scene kind. Confirm on the N64 scene ids before relying on 1P/Allstar Venom/GB Land zoom.
- Blastzone.asm:90 maps to the altitude warning sound (`nSYAudioFGMAltitudeWarn`, FGM 0x99 in the asm, inside the `fkind != nFTKindBoss` condition). The asm comment says "bottom blast zone sound"; the C sound is the same call.
- Surface friction table: the 16 vanilla entries are read from `dMPCollisionMaterialFrictions`, which the asm copies from ROM 0xA7CE0 (0x40 bytes). Values were not compared to the ROM bytes.
- Spawn.asm:19: the hook is at +0x1C of `mpCollisionGetMapObjPositionID`, so the override runs for every caller of that function, as in the asm.

## REMIX-TODO markers

- `REMIX-TODO(toggles)`: `GRREMIX_BLASTZONE_WARP_TOP_BOTTOM` (grremixblastzone.c) in place of `Toggles.entry_blastzone_warp`.
- Skipped patches above are not marked in vanilla code. They are listed here only.

## Build

- The new `grremix*.c` files are picked up by the `GLOB_RECURSE` in `CMakeLists.txt` (line 523). Re-run the cmake configure before building.
- Syntax checks: `scripts/remix/syntax-check.sh` prints `syntax-check OK` for the four new files and for `ftphysics.c`, `ftmain.c`, `gmcamera.c` and `mpcollision.c`.
