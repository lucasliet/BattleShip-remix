# F7 Size phase (c): effect GFX of the owner (Size.asm scopes yoshi_egg, ground_gfx, kirby, falcon)

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm` 293-342 (kirby_ausp_) and 1331-1790. Helpers are in
`decomp/src/ft/ftremix/ftremixsizefx.c` (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`), declared in a
block appended to `decomp/src/ft/ftremix/ftremixhooks.h`. Multiplier source is `ftRemixSizeGetMultiplier` (1.0F default).

## Patches

| Asm address (Size.asm line) | Vanilla function | Status |
|-----|-----|-----|
| 0x801011A4 (1334) adjust_size_egg_shield_ | efManagerYoshiShieldProcDisplay (ef/efmanager.c) | applied: DObj scale = 1.5F * m before the draw |
| 0x80102F64 (1398) adjust_size_egg_entry_ | efManagerYoshiEntryEggMakeEffect callers: ft/ftport.c (case nFTKindYoshi), ft/ftremix/ftremixentry.c (ftRemixEntryMakeYoshiEgg) | applied: uniform scale = egg scale.x * m, NULL egg ignored |
| 0x8014C994 (1362) adjust_size_egg_lay_ | ftCommonYoshiEggMakeEffect (ft/ftcommon/ftcommoncaptureyoshi.c) | skipped: needs the default egg size per character (`Character.yoshi_egg.table`, Character.asm 1696-1720), not in the port |
| 0x800EB0E8, 0x800EB0F8 (1429, 1434) save_player_struct_ grounded/aerial | efManagerImpactWaveMakeEffect / AirWave in ftParamMakeEffect (ft/ftparam.c, nEFKindImpactWave) | applied: owner attached to the effect (`ftRemixSizeAttachGroundGfx`) |
| 0x80141B40 (1439) save_player_struct_ wall bounce | ftCommonWallDamageSetStatus (ft/ftcommon/ftcommonwalldamage.c) | applied: owner attached |
| 0x801551BC (1444) save_player_struct_ ness PK thunder wall bounce | ftNessSpecialAirHiJibakuBoundSetStatus (ft/ftchar/ftness/ftnessspecialhi.c) | applied: owner attached |
| 0x8014C40C (1449) save_player_struct_ kirby star clipping | ftCommonCaptureKirbyCollide area (ft/ftcommon/ftcommoncapturekirby.c, ImpactWave call) | applied: owner attached |
| 0x8012DFC4 + 0x10 (1456) update_routine_ | dEFManagerImpactWaveEffectDesc proc update (ef/efmanager.c) | applied: `ftRemixSizeGroundGfxProcUpdate` runs efManagerImpactWaveProcUpdate, then scales the DObj by m (owner-less waves untouched) |
| 0x8012E0B4 + 0x14 (1541) samus_gun_blast render_routine_ | Samus gun blast desc | skipped: the descriptor install point is not mapped to a port descriptor |
| 0x8010429C (1584) adjust_suck_gfx_ | efManagerKirbyInhaleWindProcUpdate (ef/efmanager.c) | applied: offsets lr*800*m and 230*m, transform scale m |
| 0x8012E37C + 0x14, 0x8012E3A4 + 0x14 (1611, 1615) kirby usp render_routine_ | Kirby USP sword gfx | skipped: install point not mapped to a port descriptor |
| 0x8012E3F4 + 0x14 (1619) kirby usp render_routine_descend_ | Kirby USP descend gfx | skipped: same as above |
| 0x80160F34, 0x80160F60 (293, 298) kirby_ausp_ | ftKirbySpecialAirHiProcPhysics (ft/ftchar/ftkirby/ftkirbyspecialhi.c) | skipped: writes Kirby's base body size table and multiplies a Kirby physics constant (`lwc1 f4, 0xC9C0(at)`), a physics/base size change (phase b) |
| captainshared.asm 0xA9B00 (Size.asm falcon.punch.render_routine_) | dEFManagerCaptainFalconPunchEffectDesc proc display (ef/efmanager.c) | applied: `ftRemixSizeFalconPunchProcDisplay` sets DObj scale to m, then lbCommonDObjScaleXProcDisplay |
| captainshared.asm 0x8012E2D4 (Size.asm falcon.kick.update_routine_) | efManagerNoEjectProcUpdate for the kick | skipped: the asm skips the scale while the projectile struct hitlag bit is set (`lw t3, 0x0010(t7)`, bit 31); no port mapping for that field |

## Deviations

- D1. The Yoshi shield is 1.5F * m. Size.asm sets this in Remix builds, so the shield draws at 1.5 at multiplier 1.0
  (not at the vanilla scale). Remix-only, guarded by `REGION_REMIX`. Verify in game.
- D2. The ground gfx wrapper scales the DObj after `efManagerImpactWaveProcUpdate`, which may eject the effect. The asm
  does the same (scale after the original update), so the behaviour matches; no extra guard was added.
- D3. Vanilla edits are one-line hooks under `#ifdef REGION_REMIX` (efmanager.c, ftport.c, ftparam.c, ftnessspecialhi.c,
  ftcommonwalldamage.c, ftcommoncapturekirby.c, ftremixentry.c). Only ftport.c and ftnessspecialhi.c and
  ftcommoncapturekirby.c gained an include under `REGION_REMIX`.

## REMIX-TODO(Size)

Listed in the header of `decomp/src/ft/ftremix/ftremixsizefx.c`: Yoshi egg lay (egg size table), Samus gun blast,
Kirby USP render routines, Falcon kick update (hitlag gate), Kirby USP base table (kirby_ausp_).

## Uncertainties

- U1. ftparam.c: the impact wave attach is placed after the if/else in the `nEFKindImpactWave` case; `effect` is assigned
  there and not checked for NULL. The helper accepts NULL.
- U2. Other workers edit ftparam.c and ftremixhooks.h in parallel (a warning at ftparam.c:1793 comes from another
  worker's line, not from this change).

## Not verified

No build, no game run, no git (per brief). `scripts/remix/syntax-check.sh` returned `syntax-check OK` for
ftremixsizefx.c, efmanager.c, ftport.c, ftremixentry.c, ftparam.c, ftnessspecialhi.c, ftcommonwalldamage.c and
ftcommoncapturekirby.c.
