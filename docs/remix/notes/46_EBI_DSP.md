# 0x46 EBI DSP: port notes

Source: `.toolchain/pkgs/smashremix-src/src/Ebi/EbiSpecial.asm`, scope `EbiDSP` (lines 979-1485).
Port: `decomp/src/ft/ftremix/ebi/ftremixebidsp.c`, table `dFTRemixEbiDSPProcs` / `dFTRemixEbiDSPProcsNum`.
Actions: `Ebi.Action.DSPGROUND..DSPRECOIL` = 0xEF..0xF2 (`Ebi/Ebi.asm` scope Action).

## Coverage

- Worklist routines ported: 10 of 10.
- Table entries: `air_initial_`, `collision_`, `ground_initial_`, `ground_move_`, `grounded_main_`, `landing_main_`, `physics_`, `recoil_air_collision_`, `recoil_move_`, `recoil_physics_`.
- Static helpers (not in table): `ftRemixEbiDSPBeginLanding` (`begin_landing_`), `ftRemixEbiDSPBeginRecoil` (`begin_recoil_`), `ftRemixEbiDSPRecoilGroundTransition` (`recoil_ground_transition_`), `ftRemixEbiDSPApplyControlledAirPhysics`, `ftRemixEbiDSPResetSpecialVars`, `ftRemixEbiDSPClearAirFlags`.

## Not ported (outside the worklist)

- `EbiDSP.air_move_` (0x8057BA90, asm 1130-1166): not in the worklist, so no action array or special table references it. Its logic is the Wario-style air decay (x * 0.875, and y * 0.875 on BEGIN). Port it if a reference shows up.
- `EbiDSP.recoil_main_` (0x8057BCF0, asm 1375-1387): not in the worklist. It transitions to idle when temp variable 1 is set. Same status as above.

## Vanilla patches

- None. `EbiDSP` contains no `OS.patch_start` blocks or `j`/`jal` hooks into vanilla code. Every `jal` is a call from the Remix routine into vanilla.

## Shared subsystems

- No weapons (`jal 0x801655C8` absent), no custom items, no `GFX.*`/`Size.*` calls. Nothing to mark with REMIX-TODO.

## Uncertainties

1. `grounded_main_` (asm 1170-1177): the comment says `begin_recoil_` but the code loads `begin_landing_`. The port follows the code (`ftRemixEbiDSPBeginLanding`).
2. `begin_landing_` (asm 1313-1326): the asm sets the status before `mpCommonSetFighterGround`. `ftremixwario.c` does the reverse for the same routine. The port follows the asm order.
3. `air_initial_` (asm 1024-1048): `lbu 0x18D; andi 0x07; sb` clears bits 0-4 of the byte, which are the `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall` bitfields (`struct_offsets.txt`). The port assigns those four fields.
4. The `andi` in `air_initial_` runs after `ftMainSetStatus(..., FTSTATUS_PRESERVE_FASTFALL)`, so the order in the port matches the asm.
5. `physics_` (asm 1234-1238): the asm tests the sign bit of y for the BEGIN clamp. The port uses `< 0.0F`. The two differ only for -0.0, which has no effect on the result.
6. `recoil_move_` (asm 1407-1410): the ceiling test reads `0xCE` (`coll_data.mask_curr`, `MAP_FLAG_CEIL` 0x400). The asm comment says `a2` is the player struct, so the port reads it through `ftGetStruct`.
7. Remix word `0x0AE0` (asm 1021, 1347): the port reads and writes it through `ftRemixFighterWord`. In `ftremixfields.c` this offset lies in the `passive_vars` union. `struct_offsets.txt` names it `samus.charge_recoil` and `kirby.copysamus_charge_level`, so another fighter that uses this word shares it. Nothing in `EbiDSP` clears it. `air_initial_` returns early while it is set, so something outside this scope must clear it on landing or on other paths. I did not check where it is cleared.
8. `ground_initial_`, `air_initial_` and `grounded_main_` set `FTSTATUS_PRESERVE_NONE` (arg 4 = 0) or `FTSTATUS_PRESERVE_FASTFALL` (arg 4 = 8, asm 1031). The port matches both.
