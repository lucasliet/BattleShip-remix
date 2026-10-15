# 0x42 PEPPY: PeppyDSP port notes

Source: `.toolchain/pkgs/smashremix-src/src/Peppy/PeppySpecial.asm`, scope `PeppyDSP` (lines 1409-1756).
Output: `decomp/src/ft/ftremix/peppy/ftremixpeppydsp.c`, table `dFTRemixPeppyDSPProcs` / `dFTRemixPeppyDSPProcsNum`.

## Coverage

- Worklist routines ported: 9 of 9 (`initial_`, `air_initial_`, `main`, `detonate_main_ground`,
  `detonate_main_air`, `ground_collision`, `ground_collision_fail`, `air_collision`, `air_collision_fail`).
- Static helpers (not in the table): `ftRemixPeppyDSPInitialSetStatus`, `ftRemixPeppyDSPSpawnFlashbang`,
  `ftRemixPeppyDSPDetonateCheck`, and the four transitions `ground_to_air` (asm 1637), `air_to_ground` (1705),
  `ground_to_air_fail` (1671) and `air_to_ground_fail` (1737), ported as `*Ignite*` because the fail
  variants land in the ignite statuses.
- Status ids (Peppy.asm 229-247): 0xEC ground, 0xEF ground ignite, 0xF1 air, 0xF5 air ignite.

## Vanilla patches

None in the PeppyDSP scope (no `OS.patch_start` or `j`/`jal` hooks into vanilla code).

## Shared Remix subsystems (REMIX-TODO)

1. `REMIX-TODO(Item.Flashbang)`, spawn in `ftRemixPeppyDSPSpawnFlashbang` (asm 1549-1553). The port calls
   `itRemixFlashbangMakeItem(fighter_gobj, &pos, &vel, 0x80000002)` with:
   - `parent_gobj` = fighter GObj (a0);
   - `pos` = right hand part 0xC world position (`fp->joints[16]`, N64 0x0928), with `pos.z = 0` (asm 1545);
   - `vel` = `{0, 0, 0}` (asm 1546-1553 zeroes the offset at sp+0x40);
   - `flags` = 0x80000002 (5th argument at sp+0x10, asm 1547);
   - a1 = `Item.Flashbang.item_info_array`, supplied by the item port, not passed.
   The prototype is declared locally in the file. Replace it with the item header once it exists.
2. `REMIX-TODO(Item.Flashbang)`, detonate in `ftRemixPeppyDSPDetonateCheck` (asm 1614-1619). When the grenade
   remaining duration (N64 ITStruct 0x02C0, Flashbang.asm:262) is 3 or more, the asm sets it to 3. The port has no
   named Flashbang duration field, so the write is skipped. Fuse-shortening on detonate is therefore not yet applied.

## Integration requirements for the Flashbang port (not done here)

- At spawn, store the grenade GObj in the owner's passive word at N64 0x0ADC as `*(GObj **)&fp->passive_vars`
  (Flashbang.asm:158). Clear it at detonation (Flashbang.asm:508). PeppyDSP reads it through
  `ftRemixPeppyDSPActiveGrenade`, which uses the same slot that Conker uses.
- The item reads the owner's throw power at N64 0x0B20 (Flashbang.asm:138-141, 161). PeppyDSP writes it in
  `initial_` (20) and raises it while B is held (`main`).
- Registration: `ftremixported.c` must declare and register `dFTRemixPeppyDSPProcs` and
  `dFTRemixPeppyDSPProcsNum`. That file is not edited here.

## Uncertainties (asm line numbers)

- 1545: `sw r0, 0x0008(a1)` zeroes z after `jal 0x800EDF24`. The port assumes `a1` still points at `pos`
  after the call, so it sets `pos.z = 0` directly.
- 1518: the B-held check uses `0x01BC` (`input.pl.button_hold`). The asm comment says "current button press".
  The port uses `button_hold`, which matches the offset.
- 1416-1456 and 1460-1500: `ftMainSetStatus` with arg4 = 0 maps to `FTSTATUS_PRESERVE_NONE`.
  The 1445-1448 second `ftMainSetStatus` is unreachable (`beq r0, r0`) and is not ported.
- 1637-1657 and 1705-1723: arg4 = 2 maps to `FTSTATUS_PRESERVE_COLANIM`, and the air transitions call
  `ftPhysicsClampAirVelXMax` as the Conker port does.
- Remix action 0xEC/0xEF/0xF1/0xF5 mapping assumed from Peppy.asm 244-247, where `ground_collision` pairs with
  0xEC and `air_collision` with 0xF1, and the fail routines pair with the ignite statuses.
