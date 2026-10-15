# leftover_kirby_copies_A notes

Scope: Kirby/J Kirby copy routines missed by the earlier fighter ports. Each file was checked against its `dFTRemix...Procs[]` table before editing.

## Already present (skipped, verified in the tables)

- Bowser: `BowserNSP.gbowser_air_collision_`, `BowserUSP.air_direction_` (ftremixbowser.c).
- Mewtwo: all five `MewtwoNSP.kirby_*` routines (ftremixmewtwonsp.c).
- Marth: `MarthNSP.kirby_ground_collision_` (ftremixmarth.c).
- Sonic: all four `SonicNSP.ssonic_*` routines (ftremixsonicnsp.c).
- Lucas: `LucasNSP.air_move_` (ftremixlucasnsp.c).
- Slippy: `SlippyNSP.main_`, `SlippyNSP.air_collision_` (ftremixslippy.c).
- Lanky: `LankyNSP.main_` (ftremixlankynsp.c).

## Ported in this pass

- `decomp/src/ft/ftremix/sheik/ftremixsheiknsp.c`
  - `SheikNSP.kirby_ground_begin_collision_`, `kirby_ground_charge_collision_`, `kirby_ground_shoot_collision_`: floor check (`mpCommonProcFighterOnFloor`, 0x800DDDDC) with the existing Sheik transitions.
  - `SheikNSP.air_shoot_collision_`: landing (`mpCommonProcFighterLanding`, 0x800DE6E4) into the new `ftRemixSheikNSPGroundShootTransition`, a port of `SheikNSP.ground_shoot_transition_`.
  - Added `nFTRemixSheikStatusLandingLight = 0x1F` (`Action.LandingLight` in Action.asm line 36).
- `decomp/src/ft/ftremix/goemon/ftremixgoemonnsp.c`
  - `GoemonNSP.kirby_ground_collision_` and `kirby_air_collision_`, with new `ftRemixGoemonNSPKirbyGroundToAir` and `ftRemixGoemonNSPKirbyAirToGround`, and the two Kirby action tables (`kirby_ground_to_air_table`, `kirby_air_to_ground_table`).

## Vanilla patches

None. No `OS.patch_start` blocks or hooks are needed for these routines.

## Uncertainties

1. `SheikNSP.ground_shoot_transition_` (SheikSpecial.asm ~1870-1903): the asm comment says it transitions to NSP_Ground_Shoot, but the code sets `Action.LandingLight` (0x1F). The port follows the code. The assumed Samus lines are decoded from ROM 0xD83F0 (`mpCommonSetFighterGround`, flags 2 = `FTSTATUS_PRESERVE_COLANIM`).
2. The same routine's Kirby/J Kirby on-hit store (player 0x09EC, `proc_damage`) is covered by `ftRemixSheikNSPSetProcDamage`, which the port already uses for every Sheik transition.
3. The Goemon Kirby ground-to-air path calls 0x800D8EB8 ("momentum capture?"). The port uses `ftPhysicsClampAirVelXMax`, the same helper as the existing Goemon and Sheik paths; this is an assumption.
4. Existing Sheik non-Kirby ProcMaps (`ground_begin_collision_`, `ground_charge_collision_`, `ground_shoot_collision_`) call `mpCommonProcFighterOnEdge` (0x800DDE84), which matches their asm (`jal 0x800DDE84`). The Kirby copies call 0x800DDDDC (`OnFloor`), so they are not aliases of those procs; they have their own functions.
5. Pre-existing and not changed here: the Sheik and Goemon files already carry REMIX-TODO markers for the GFX routines (charge glow, charge level write).
