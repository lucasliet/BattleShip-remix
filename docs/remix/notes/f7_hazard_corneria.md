# F7 hazard group Corneria: notes

Stage: nGRRemixStage_CORNERIA2 = 0x46 (`decomp/src/gr/grremix/grremixids.h`). Routine: `Hazards.corneria_setup` (Hazards.asm 1985-2016)
and `great_fox_` (2026-2138). Projectile table and spawn data: 2140-2172.

## Files
- `decomp/src/gr/grremix/grremixcorneria.c` / `.h` (new): setup, Great Fox process, shot spawn.
- `decomp/src/gr/grremix/grremixhazards_corneria.c` (dispatch stub): returns TRUE when `function == REMIX_SYM_Hazards__corneria_setup`.

## Ported
- corneria_setup: `grRemixHazardsAreOn()` gate, `grSectorMakeGround()` (vanilla 0x80107FCC), process object, timer reset.
- great_fox_: timer 0x10E, then 0x97E..0x9C6 step 9 (10 shots), restart at 0x107D. Hazard state in a file-local struct.
- Fire: fgm 0x109, left laser at (-3100, -230, 200), right laser at (-3100, -230, -100) only if the left one spawned.
- Laser: projectile kind nWPKindArwingLaser2D (id 0x12) via `dGRSectorArwingWeaponLaser2DWeaponDesc`, vel_air.x = -230,
  rotation = great_fox_spawn_rotation for both shots.

## Skipped or approximated
- 0x80008188 is `gcAddGObjProcess` (kind 1 = nGCProcessKindFunc, priority 4).
- 0x8010719C (`func_ovl2_8010719C`) is not called. The asm writes its output to a stack buffer that is overwritten by the
  spawn rotation, and the routine has no other side effect.
- Object 0x3F2 (Render.CREATE_OBJECT_, group 1) has no display setup in the asm. The port creates a process-only GObj.
- REMIX-TODO(Remix header attribute 0x130): the Remix projectile reads its attributes at header offset 0x130, a Remix addition
  to the Sector Z header. The port uses the vanilla Arwing laser attribute offset.
- The left shot's parent (a0) at 0x801655C8 is undefined in the asm (the register is clobbered by the fgm call). The port
  passes the Great Fox object, as the right shot does.

## Semantics to check
- hazard_mode: the asm skips both the Sector Z ground and the Great Fox when the toggle bit is set. The port mirrors that
  (`grRemixHazardsAreOn()` FALSE returns without building anything). The asm comment says bit set = hazards disabled, and
  grRemixHazardsAreOn() is TRUE for now.
- The timer is in the asm at 0x801313F0+0x60 (gGRCommonStruct). The port moves it to a file-local struct, so Sector Z's own
  fields in that union are not touched.

## Lead test
- Run with `SSB64_VS_STAGE=70` (0x46 decimal). Expect the Sector Z arwing flight and volleys at frames 270 (0x10E), then
  2430, 2439, ..., 2502 (0x97E..0x9C6), and the timer restarts at 4221 (0x107D).
