# F7 hazard notes: Metallic Madness (Hazards.asm metallic_madness_setup, bee factory, shrink timer, Robot Bee, Robot Bee laser)

Stage: `nGRRemixStage_MMADNESS` = 0xA2 (162 decimal). Run with `SSB64_VS_STAGE=162`.
Dispatch: `grRemixHazardMetallic` in `decomp/src/gr/grremix/grremixhazards_metallic.c` (matches `REMIX_SYM_Hazards__metallic_madness_setup`).

## Files

- `decomp/src/gr/grremix/grremixhazards_metallic.c`: dispatch, calls the setup when `grRemixHazardsAreOn()`.
- `decomp/src/gr/grremix/grremixmetallic.{c,h}`: setup, bee factory, shrink timer, shared state (stage header, laser file, facing).
- `decomp/src/gr/grremix/grremixmetallicbeelaser.{c,h}`: laser WPDesc (`sWPMetallicBeeLaserWeaponDesc`, kind 0x29), procs, fire.
- `decomp/src/it/itremix/itremixrobotbee.{c,h}`: item desc `dITRemixRobotBeeItemDesc` (kind 0x3B), make, main, destroy.

The lead must register the item in `itremixitems.c` (`extern const ITRemixItemDesc dITRemixRobotBeeItemDesc;` and the table entry), as `tools/gen_remix_items.py` does for Gem.

## Routines ported

| Asm label | Port |
|---|---|
| metallic_madness_setup | `grRemixMetallicSetup` (clears state, stage header from `gMPCollisionGroundData - 0x14`, laser file from header + 0x130 via PORT_RESOLVE, two ground GObjs) |
| bee_factory_ | `grRemixMetallicBeeFactoryProcUpdate` (1500 tic gate, roll 80 or timer 3000, training mode check via `nSCKind1PTrainingMode` = enum 54 = 0x36) |
| shrink_timer_ | `grRemixMetallicShrinkTimerProcUpdate` (countdown, end-of-shrink reset) |
| robot_bee_stage_setting_ | `itRemixRobotBeeMakeItem` + `itRemixRobotBeeSetupHitbox` |
| robot_bee_main_ | `itRemixRobotBeeMainProcUpdate`, `itRemixRobotBeeFireWindow`, `itRemixRobotBeeSfxTick` |
| robot_bee_destroy_ | `itRemixRobotBeeDestroyProcHurtbox` |
| robot_bee_blast_zone_ | not wired (see REMIX-TODO) |
| robot_bee_laser_ | `grRemixMetallicBeeLaserFire` |
| robot_bee_laser_stage_setting_ | `grRemixMetallicLaserMake` (wpManagerMakeWeapon with the bee as parent, speed, angle, glow) |
| robot_bee_laser_main_ | `grRemixMetallicLaserProcUpdate`, grow and shrink helpers |
| robot_bee_laser_collision_ | `grRemixMetallicLaserProcMap` (wpMapTestAllCheckCollEnd, fox blaster glow) |
| robot_bee_laser_hit_ | `grRemixMetallicLaserProcHit` (shrinks victims of player object id 0x3E8) |
| robot_bee_laser_reflect_ | `grRemixMetallicLaserProcReflector` |
| robot_bee_laser_projectile_struct | `sWPMetallicBeeLaserWeaponDesc` |
| acidrain block (commented out in the asm) | skipped |

Vanilla calls mapped:
- 0x8016EA78 `itManagerMakeItemSetupCommon` (stage spawn; Remix kinds route to `itRemixMakeItem`).
- 0x8016E174 `itManagerMakeItem`, 0x80008188 `gcAddGObjProcess`, 0x801655C8 `wpManagerMakeWeapon`.
- 0x80167C04 `wpMapTestAllCheckCollEnd`, 0x80103320 `efManagerFoxBlasterGlowMakeEffect`.
- 0x800269C0 `func_800269C0_275C0` (FGM player), FGM 0x3FB (buzz) and 0x3E8 (destroy).
- 0x8001863C is inside a decomp function, not a function start; the port uses `atan2f` from libm.

## Deviations and uncertainties

1. Stored state. The asm keeps the bee timers in the item GObj (0x40, 0x44, 0x48) and the factory state at RAM 0x801313F0. The port keeps them in file-local statics. The factory spawns one bee at a time, so this is equivalent in practice.
2. Laser duration. The asm writes 40 tics to WPStruct 0x268. The Remix main never reads it, so the port does not store it.
3. Laser max X scale. The asm reads RAM 0x8019CAA0, which is not mapped. The port assumes the Fox blaster clamp (160/3), which is the same bit pattern as the asm's minimum 0xC2555555. Verify this.
4. Laser angle. The asm loads the X speed into f12 and the Y speed into f14 before the angle call. The port calls `atan2f(vel.x, vel.y)` in that order. Verify the sign on the diagonal.
5. Immunity. The asm compares the whole FTStruct word 0x18C with 0x04000010 and 0x08800110. The port checks `is_reflect || is_absorb`.
6. Size restore. The asm restores NORMAL (1.0), GIANT (2.25) or TINY (0.5) from `Size.match_state_table`. The Remix size state is not readable from C, so the port always restores 1.0.
7. Item state. The asm item has three states (0 main, 1 main, 2 explosion, which calls 0x8017D298 = `itPakkunAppearProcUpdate`). The port sets only the ITDesc procs (update and hurtbox destroy), since the asm never sets a status. State 2 is not ported.
8. Item animation. `grRemixMetallicSpawnBee` does not set the Robot Bee animation tracks (0x8000BD1C and 0x8000BD54 are not mapped). REMIX-TODO(animation).
9. Transform types. The laser uses `nGCMatrixKindTraRotRpyR` because it rotates on Z. The asm word 0x0C (0x1C000000) is not decoded. The Robot Bee item uses the literal `{ 0x1B, 0, 0 }` from the asm word 0x1B000000.
10. Item spawn flags. The asm passes no flags to `itManagerMakeItemSetupCommon`; the port passes 0.
11. Stage header. The item p_file is the stage header pointer (asm 0x801313F4), with o_attributes 0xE0 as in the asm item info array. The Robot Bee file itself (header + 0xE0 pointer - 0x7E8) is only used by the asm animation, which is not ported.

## REMIX-TODO markers

- `REMIX-TODO(SuperMushroom)`: `grRemixMetallicShrinkPlayer` does not destroy the active shroom of the port (`Item.SuperMushroom.player_shrooms`). The shrink timer's shroom clear is also missing. No accessor exists in `itremixsupermushroom.h`.
- `REMIX-TODO(Size)`: end-of-shrink reset ignores GIANT and TINY states.
- `REMIX-TODO(FTStruct)`: immunity check is approximated by bitfields.
- `REMIX-TODO(vanilla RAM 0x8019CAA0)`: max laser scale assumed.
- `REMIX-TODO(transform)`: laser transform types.
- `REMIX-TODO(animation)`: Robot Bee animation tracks.
- `REMIX-TODO(ITStruct)`: unnamed item fields 0x158, 0x33E, 0x2D3, 0x1D0, 0x1D4, 0x35C.
- `REMIX-TODO(blast zone)`: blast-wall routine table (ITStruct 0x398) not wired.
- Vanilla address 0x80168964 (laser absorb / psi magnet proc): not ported.

## Vanilla patches

None. No `OS.patch_start` or `#ifdef REGION_REMIX` edits in vanilla files.

## For the lead to test

- Build, then run stage 162 with `SSB64_VS_STAGE=162` (the stage must load with hazards on).
- Check that the bee factory starts counting (match time > 0 or training mode) and that a Robot Bee appears within about 3000 tics.
- Check that a laser hit shrinks the victim to half size, and that the size returns to normal after 900 tics.
- Verify the open uncertainties: laser max scale (item 3), laser angle sign (item 4), immunity (item 5), and the transform of the laser (item 9).
- Register `dITRemixRobotBeeItemDesc` in `itremixitems.c`, then confirm kind 0x3B resolves through `itRemixGetItemDesc`.
