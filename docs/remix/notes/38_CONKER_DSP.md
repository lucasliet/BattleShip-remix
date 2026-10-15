# 0x38 CONKER: ConkerDSP port notes

Source: `.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm`, scope `ConkerDSP` (lines 984-2082).
Output: `decomp/src/ft/ftremix/conker/ftremixconkerdsp.c`, table `dFTRemixConkerDSPProcs` / `dFTRemixConkerDSPProcsNum`.

## Coverage

- Worklist routines ported: 7 of 7 (`initial_`, `air_initial_`, `main`, `ground_collision`,
  `ground_collision_fail`, `air_collision`, `air_collision_fail`).
- Static helpers ported: `ftRemixConkerDSPInitialSetStatus` (shared by `initial_` and `air_initial_`),
  and the four status transitions `ground_to_air(_fail)` and `air_to_ground(_fail)` (asm 1947-2051).
- Not ported: the grenade item (see Item below). Routines not ported: `grenade_stage_setting_`
  (1159), `grenade_main_` (1266), `grenade_collision_` (1478), `grenade_resting_collision_` (1530),
  `grenade_exploding_main_` (1546), `grenade_explosion_hitboxes_` (1575), `grenade_begin_main_` (1586),
  `grenade_begin_resting_` (1611), `grenade_explosion_` (1638), `grenade_begin_explosion_` (1688),
  `grenade_detect_collision_` (1715), `grenade_hurtbox_collision_` (1733), `grenade_hitbox_collision_` (1790),
  `grenade_blast_zone_` (2076). Data: `grenade_attributes` (1880), `grenade_item_info_array` (1897),
  `grenade_item_states` (1904).

## Registration (integration task, not done here)

- `decomp/src/ft/ftremix/ftremixported.c` line 19-20 and 49 currently reference only `dFTRemixConkerProcs`
  (the `ftremixconker.c` table for ConkerNSP and ConkerUSP). The DSP table is not registered yet.
  Whoever owns `ftremixported.c` must add `extern const FTRemixProc dFTRemixConkerDSPProcs[];`,
  `extern const s32 dFTRemixConkerDSPProcsNum;` and a `{ dFTRemixConkerDSPProcs, &dFTRemixConkerDSPProcsNum }`
  entry in `dFTRemixPortedProcTables`.
- Until then, every ConkerDSP routine falls back to the parent (FOX) routine in `ftRemixBindSpecials`.

## Vanilla patches

- None are needed by the seven ported routines. They call only vanilla functions (decomp names in
  `build/remixasm/refs/vanilla_funcs.txt`): `ftMainSetStatus` (0x800E6F24), `ftMainPlayAnimEventsAll`
  (0x800E0830), `mpCommonProcFighterOnEdge` (0x800DDE84), `mpCommonProcFighterLanding` (0x800DE6E4),
  `mpCommonSetFighterAir` (0x800DEEC8), `mpCommonSetFighterGround` (0x800DEE98),
  `mpCommonSetFighterWaitOrFall` (0x800DEE54), `ftPhysicsClampAirVelXMax` (0x800D8EB8).
- Item-side patch the grenade item will need (not for this scope): `grenade_detect_collision_` copies
  0x88 bytes from vanilla 0x801737B8 (`OS.copy_segment(0xEE0F4, 0x88)`) and 0x2C bytes at 0xEE1CC
  (`OS.copy_segment(0xEE1CC, 0x2C)`), and `grenade_explosion_hitboxes_` jumps to 0x801863B8 with
  `t6 = grenade_item_info_array + 4`. Both belong to the item port.

## Item (REMIX-TODO(Item))

- `grenade_stage_setting_` is called from `ConkerDSP.main` when temp variable 1 (`motion_vars.flags.flag0`)
  is set. It calls `itManagerMakeItem` (0x8016E174) with `grenade_item_info_array` (item id 0x15,
  `Character.CONKER_file_7_ptr`). Its state table (`grenade_item_states`) holds Remix item routines by
  address, which the vanilla item system (`itMainSetStatus`, 0x80172EC8) dispatches through ROM pointers.
  A fighter file cannot register those, so the spawn is skipped and marked in the port.
- Effect in the port: the grenade flag is cleared but no grenade is created. The active-grenade slot
  (0x0ADC) therefore stays 0, so `initial_`/`air_initial_` always pick the "ready" status (0xEC / 0xF1).
- Needed to finish: port the grenade item (its procs and state table) into the item system, then
  restore the call in `ConkerDSP.main` and pass the right hand part position (asm 1117-1128).

## Translation choices

- Action ids from `Conker.asm` `scope Action`: GrenadeToss 0xEC, GrenadeTossFail 0xEF, GrenadeTossAir 0xF1,
  GrenadeTossFailAir 0xF5, kept in a file-local enum.
- Grenade power (0x0B20) and hold counter (0x0B28) live in the parent's `status_vars` union, and the active
  grenade object (0x0ADC) lives in `passive_vars`. The port reads them through an overlay struct
  (`FTRemixConkerDSPStatusVars`) and a `GObj *` cast. These are not named fields in the decomp.
- `ftMainSetStatus` flag 2 (asm 1967, 2043) is `FTSTATUS_PRESERVE_COLANIM` (`1 << 1`), kept as the named constant.
- `is_special_interrupt` (0x0192 bit 0 = 0x80, asm 1012-1013) is set in `initial_`/`air_initial_` on both
  status paths, as in the asm.

## Uncertainties

1. asm 1019 (`jal 0x800E6F24` after the `beq r0,r0 branch_1` at 1015): unreachable, so not ported.
2. asm 993-994: the immediate is `0x14` (= 20 decimal), while the source comment says "14". The port stores 20.
3. asm 1093: the comment says `0x0020` for B, but `Joypad.B` is `0x4000` (`Joypad.asm` line 12, equals `B_BUTTON`).
   The port uses `B_BUTTON`.
4. asm 1103 and 1104: `beql` (branch-likely) with the `lw ra` delay slot. Only the taken path matters, and the
   port has no visible difference.
5. asm 1086-1088: the counter at 0x0B28 is an integer in these routines but `f32` in the decomp union
   (`status_vars.common.rebirth.pos.y`). The port treats it as `s32`.
6. asm 1957 and 2029: the frame passed to `ftMainSetStatus` is `anim_frame` of the GObj (offset 0x78). Used as-is.
