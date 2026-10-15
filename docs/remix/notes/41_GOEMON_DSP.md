# 0x41 GOEMON, GoemonDSP scope: port notes

Port file: `decomp/src/ft/ftremix/goemon/ftremixgoemondsp.c`, table `dFTRemixGoemonDSPProcs` (10 entries, all routines of the worklist). Source: `.toolchain/pkgs/smashremix-src/src/Goemon/GoemonSpecial.asm`, lines 1893-2798.

Action ids: DSPGround 0xEF, DSPGroundPull 0xF0, DSPGroundWallPull 0xF1, DSPGAttack 0xF2, DSPGBreak 0xF3, DSPAir 0xF4, DSPAirPull 0xF5, DSPAirWallPull 0xF6, DSPAAttack 0xF7, DSPABreak 0xF8, DSPEnd 0xF9. Derived from `action_string_table` in `Goemon.asm` (index = id - 0xDC; entry 19 is ChainPipeGround). Confirmed by the +5/-5 ground/air transitions, which map each ground id to its air id.

## 1. Vanilla patch (needs a C insertion in vanilla code)

### item_hurtbox_patch_ (GoemonDSP.item_hurtbox_patch_, asm 2761-2797)

- Hook: `OS.patch_start(0xEB0D8, 0x80170698)`, which places `j item_hurtbox_patch_` at RAM 0x80170698.
- Vanilla function: `itProcessSearchHitFighter`, `decomp/src/it/itprocess.c:672` (RAM 0x801705C4).
- Effect: the vanilla `sll t0, t8, 18; bltzl t0, 0x80170850` tests bit 0x2000 of t8 and jumps to 0x80170850 when it is set. For Goemon (`s7`'s `fkind` == Character.id.GOEMON) in DSPGround or DSPAir, the patch skips that jump and resumes at the instruction after the patch. Other fighters take the original path.
- `s7` is the FTStruct: `0x08` is fkind and `0x24` is status_id, which the patch reads.
- Exact C: I could not confirm which decomp variable holds t8 at this point, nor which decomp label 0x80170850 corresponds to, so I cannot write a verified C insertion. The required behaviour is:

  ```c
  #ifdef REGION_REMIX
  if (!((fp->fkind == REMIX_ID_GOEMON) && ((fp->status_id == 0xEF) || (fp->status_id == 0xF4))) && (<t8> & 0x2000))
  {
      goto <label at RAM 0x80170850>;
  }
  #endif
  ```

  The owner of `itprocess.c` must map `<t8>` and the label. Ids 0xEF and 0xF4 are the DSPGround and DSPAir ids above.

## 2. Shared Remix subsystems

None. The port has no items, GFX or announcer calls, and no `REMIX-TODO` markers.

## 3. Static helpers (in the file, not in the table)

Called from the table routines and not listed in the worklist:

- `ground_pull_initial_`, `air_pull_initial_` (command-grab callbacks passed to `ftYoshiSpecialNSetCatchParams`)
- `ground_wall_pull_initial_`, `air_wall_pull_initial_`, `ground_break_initial_`, `air_break_initial_`
- `attack_initial_` (from `pull_main_`), `end_initial_` (from `wall_pull_main_`)
- `grab_pull_setup_`
- `ground_to_air_`, `air_to_ground_`, `shared_ground_to_air_`, `shared_air_to_ground_`
- `chain_collision_`, `chain_detect_collision_`, `check_wall_distance_`

`pull_main_` is a C copy of `ftCommonThrowProcUpdate` (0x8014A0C0, `decomp/src/ft/ftcommon/ftcommonthrow.c:16`) with the final `mpCommonSetFighterWaitOrFall` replaced by `attack_initial_`. The copied fragment is `OS.copy_segment(0xC4B00, 0x10C)`. Nothing else is duplicated.

`end_collision_` is `ftMarioSpecialHiProcMap` (0x80156358, `decomp/src/ft/ftchar/ftmario/ftmariospecialhi.c:111`) with the landing lag patched. The fragments are `OS.copy_segment(0xD0D98, 0x78)` and `OS.copy_segment(0xD0E14, 0x44)`.

## 4. Uncertainties and deviations (asm line numbers)

1. **air_initial_, asm 1953-1955.** The asm does `and v1, v1, 0x07; sb v1, 0x18D(a0)`, but nothing loads v1 from 0x18D. v1 is whatever the previous call left in the register. The port sets `is_fastfall = FALSE` only, which is the documented intent.
2. **Wall side mask, asm 2377-2380 and 2432-2435.** The asm masks the chain result with 1 when DIRECTION >= 0 and with 2 otherwise. Bit 1 is the left wall, so facing right tests the left-wall bit. This looks inverted, but it is ported as written (`ftRemixGoemonDSPChainHitsWall`).
3. **Chain ECB copy, asm 2612-2634.** The asm copies only 0xD0 bytes of the fighter's `coll_data` (FTStruct 0x78). The port copies the whole `MPCollData` struct. Fields past 0xD0 are stale stack in the vanilla code and come from the fighter here. The wall functions were not verified to write those fields before reading them.
4. **Chain joint, asm 2617.** `lw a0, 0x094C(v0)` is read as `fp->joints[25]` (joints at 0x8E8, 4 bytes each). The decomp has no name for that slot, so the index is unverified.
5. **Landing lag, asm 2596 and 2598.** `lui a2, LANDING_FSM` (0x3EE0) is combined with the copied `ori a2, a2, 0x5C29`, giving 0x3EE05C29 = 0.43820312. The worklist comment says 0.4375, which is only the high half. The port uses the exact bit pattern.
6. **Catch NULL guard, copy of 0x8014A0C0 (ROM 0xC4B00).** The decomp's `ftCommonThrowProcUpdate` has a PORT guard for `catch_gobj == NULL`. The Remix copy does not. The port follows the asm, so `ftCommonThrownProcPhysics(NULL)` is possible if flag2 is set with no catch. This is not guarded.
7. **ground_to_air_ offset, asm 2470 and 2552.** The comments say id + 4 (ground_to_air_) and id + 5 (shared_ground_to_air_). The code says +5 in both. The port uses +5 and -5 as the code does, which matches the id mapping above.
8. **Grab callbacks.** `ground_pull_initial_` and `air_pull_initial_` are handed to `ftYoshiSpecialNSetCatchParams` (0x8015E310), which stores them as the catch callback. That behaviour is assumed from the call sites.
9. **Unused routine.** `GoemonDSP.attack_initial_` has its `b _end` and `_idle` branches commented out in the asm, so only the attack path is ported.
10. **FGM id 0x13** (grab sfx) is played through `func_800269C0_275C0`, declared `extern void *func_800269C0_275C0(u16);` as in `sc1pintro.c`.
