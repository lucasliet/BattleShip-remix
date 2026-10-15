# F7 hazard group pirate (Pirate Land): notes

Stage: `Hazards.pirate_land_setup` (nGRRemixStage_PIRATE = 0x9E). Source: `Hazards.asm` 3672-4355 (Cannon scope, pirate_land_setup,
pirate_land_main_, pirate_land_water_, cannon_main_, cannonball_stage_setting_/main_/explosion_, projectile data tables).

## Files
- `decomp/src/gr/grremix/grremixhazards_pirate.c`: dispatch, runs the setup when hazards are on.
- `decomp/src/gr/grremix/grremixpiratecannon.c/.h`: setup, cannon GObj, turn/wait/fire state machine, frenzy.
- `decomp/src/gr/grremix/grremixpiratecannonball.c/.h`: cannonball WPDesc (kind 0x28), flight, smoke, explosion.
- Water: reused `grRemixWaterMakeGround(-600.0F, -360.0F)` from `grremixwater.c` (owned by the Great Bay group). Same per-frame
  logic as pirate_land_water_. Not a separate copy.

## Ported
- Cannon states WAITING / TURNING / FIRING, position table, turn speed 4 (frenzy 8), wait 90 + rand(60), frenzy chance 5%,
  frenzy moves 10 + rand(10), fire scale 1.125 for 12 frames, FGM 1012 (turn) and 1013 (shot), light screen shake.
- Cannon file pointers from the map header (offsets 0x80 cannon, 0xAC hitbox), read as relocation tokens with PORT_RESOLVE.
- Joint animation tracks of the cannon file (0xBB4 joints, 0xBF0 materials), barrel start pose x=0 y=60.
- Cannonball: WPDesc, spawn at base + barrel translate, track offset per position (0x0000 / 0x0610 / 0x0C50), first-frame flag,
  smoke every 9 frames, explosion hitbox (size 360, damage 25, fire, angle 361, kb scale 90, kb base 50, FGM 25), explosion
  sparkle, FGM 1, moderate shake, 15-frame explosion timer.

## Uncertain or skipped (REMIX-TODO markers in the code)
1. Cannonball launch velocity: the props struct says speed 100 at angle 270 with gravity 0. Sign and facing convention not
   verified, so the ball starts at rest. Asm: `cannonball_properties_struct`.
2. Explosion trigger: the asm explodes at the end of the cannonball model animation (gobj frame 0x78). Not read. The port
   explodes when the 999-frame flight lifetime runs out.
3. Cannonball joint rotation to 90 degrees (asm `sw at, 0x0034(a0)` of the projectile's first joint): DObj field not mapped.
4. Influence-camera flag 0x80 at 0x026C of the projectile special struct: no port accessor.
5. Explosion: the asm copies Samus bomb hit setup from ROM 0xE396C / 0xE39A8 (vanilla 0x80168F2C) and resets four hit-object
   pointers. Not ported. The explosion smoke function 0x800FF590 is mapped to efManagerDustExpandLargeMakeEffect; its second
   argument (1.0) is dropped because the port signature takes only the position.
6. Cannon model at cannon file 0x0970 (asm 0x8000F8F4 call) not wired. Only the joint descriptors at 0x07A8 are set up, through
   grModelSetupGroundDObjs. Expect the model to be incomplete until this is checked in game.
7. Asm 0x8000DF34 (apply animation part 2) and 0x8001DF34 (per-frame routine registration) are replaced by gcAddAnimAll and
   gcPlayAnimAll as a process. Not verified.
8. Match state: the asm skips the cannon in training mode and before the timer starts. Same gap as the water module.
9. Transform kinds for the cannon DObjs are zero (nGCMatrixKindNull) for all DOBJ_ARRAY_MAX entries. Verify the DObj count.
10. Barrel rotation/scale field mapping: the asm offsets (joint + 0x1C translate, + 0x24.. scale) are mapped to the named
    translate.vec.f and scale.vec.f fields. The N64 offsets and the port DObj layout were not cross-checked.

## Stage ids served
- nGRRemixStage_PIRATE (0x9E).

## Exported
- No item descs. Weapon desc is file-local (`sGRRemixPirateCannonballWeaponDesc`).

## Test
- Run with `SSB64_VS_STAGE=158` (0x9E).
