# F7 hazard: Banzai Bill (Peach's Castle II, stage id nGRRemixStage_PEACH2 = 0x44)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` peach_castle_II_setup_ (8072-8132) and banzai_bill_* (8133-8420),
`src/items/BulletBill.asm` (unused car-type item, not ported as a spawn of this factory).

## Files
- `decomp/src/gr/grremix/grremixhazards_peach2.c`: grRemixHazardPeachCastle2, calls the factory when hazards are on.
- `decomp/src/gr/grremix/grremixbanzai.c/.h`: grRemixBanzaiFactoryMake(stage_header), the factory process.
- `decomp/src/it/itremix/itremixbulletbill.c/.h`: dITRemixBulletBillItemDesc (kind 0x3D), banzai hitbox and routines.

## Findings
1. The asm factory returns for every screen except training mode, so Banzai Bill only spawns in training mode.
   The port keeps that gate (grRemixBanzaiFactoryIsActive).
2. The asm spawns `Item.RobotBee.id` (0x3B) with the Robot Bee item_info_array. banzai_bill_stage_setting_ is a
   copy of the Robot Bee setup, and banzai_bill_main_ is commented out in its laser part. The port spawns kind 0x3D.
   Decide in game which kind the lead wants.
3. `src/items/BulletBill.asm` is the car-type item marked "Unused" in its header. Its main is not ported.

## REMIX-TODO
- ITStruct raw fields (0x33E, 0x158 bit 0x10, 0x2D3 bit 0x08, 0x1D0, 0x1D4, 0x35C) in the item setup.
- Blast zone slot 0x398: banzai_bill_blast_zone_ returns destroy, not wired.
- Laser pointer 0x0C of the stage state (t4 is not set in the active code), skipped.
- Factory spawn kind (see finding 2).

## Final finding (2026-10-31, user decision "same as Remix")

`Stages.asm:3256` registers Peach's Castle II with `function.CLONE`, and no file references `peach_castle_II_setup_`,
`banzai_bill_stage_setting_`, `banzai_bill_main_` or `banzai_bill_destroy_`. In Remix 2.0.1 Banzai Bill is dead code and never
appears in any mode (confirmed in the port: the stage function of gkind 0x44 is the original Peach's Castle routine 0x801056F8, the
hazard dispatcher is never called for it). The factory port now spawns what the asm spawns (Robot Bee 0x3B, the Banzai tracks, no
laser because the asm stores an uninitialised register as the laser file), but it is unreachable, exactly like the original.
The item-id question is closed.
