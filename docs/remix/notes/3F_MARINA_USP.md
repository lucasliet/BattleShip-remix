# 0x3F MARINA USP notes

Port: `decomp/src/ft/ftremix/marina/ftremixmarinausp.c`
Table: `dFTRemixMarinaUSPProcs` / `dFTRemixMarinaUSPProcsNum` (6 routines, all in the worklist).
Source: `.toolchain/pkgs/smashremix-src/src/Marina/MarinaSpecial.asm` lines 1089-1346 (scope MarinaUSP).

## Routines ported

| Remix label | Symbol | C routine | Asm line |
|---|---|---|---|
| `MarinaUSP.air_initial_` | `REMIX_SYM_MarinaUSP__air_initial_` | `ftRemixMarinaUSPAirInitial` | 1105 |
| `MarinaUSP.ground_initial_` | `REMIX_SYM_MarinaUSP__ground_initial_` | `ftRemixMarinaUSPGroundInitial` | 1139 |
| `MarinaUSP.main_` | `REMIX_SYM_MarinaUSP__main_` | `ftRemixMarinaUSPMain` | 1165 |
| `MarinaUSP.change_direction_` | `REMIX_SYM_MarinaUSP__change_direction_` | `ftRemixMarinaUSPChangeDirection` | 1188 |
| `MarinaUSP.physics_` | `REMIX_SYM_MarinaUSP__physics_` | `ftRemixMarinaUSPPhysics` | 1219 |
| `MarinaUSP.collision_` | `REMIX_SYM_MarinaUSP__collision_` | `ftRemixMarinaUSPCollision` | 1337 |

Helper not in the worklist, kept static and out of the table: `MarinaUSP.air_control_` (asm line 1313,
`REMIX_SYM_MarinaUSP__air_control_`). It is ported as `ftRemixMarinaUSPApplyAirControl` and the
`ftPhysicsClampAirVelXStickRange` call inside it.

## Vanilla patches

None. The up special makes no `OS.patch_start` or `j`/`jal` hook into vanilla code. Every vanilla
dependency is either a copied fragment, which is ported inline in `ftremixmarinausp.c`, or a plain call.
The only vanilla routine called by its decomp name is `ftCaptainSpecialHiProcInterrupt` (0x80160370,
`decomp/src/ft/ftchar/ftcaptain/ftcaptainspecialhi.c:41`), which is already in
`ftremixvanillaprocs.c`.

## Registration needed outside this file

`decomp/src/ft/ftremix/ftremixported.c` is not edited. The lead must add:

    extern const FTRemixProc dFTRemixMarinaUSPProcs[];
    extern const s32 dFTRemixMarinaUSPProcsNum;

and a `{ dFTRemixMarinaUSPProcs, &dFTRemixMarinaUSPProcsNum }` entry in `dFTRemixPortedProcTables`.
The other Marina workers need the same for their tables.

## Copied vanilla fragments (OS.copy_segment)

- `main_` line 1167: `OS.copy_segment(0xD7190, 0x20)`, 8 instructions of 0x8015C750 (Fox end of up
  special). The copied `bc1fl` gives `if (anim_frame <= 0) ftCommonFallSpecialSetStatus(...)`. The remaining
  stack setup in the ASM (`sw r0 0x10`, `sw r0 0x18`, `lui t6`, `sw t6 0x14`) supplies the call arguments.
- `collision_` line 1339: `OS.copy_segment(0xD0D98, 0x78)` and line 1344 `OS.copy_segment(0xD0E14, 0x44)`.
  The bytes are Mario's `ftMarioSpecialHiProcMap` (0x80156358), so the port follows
  `decomp/src/ft/ftchar/ftmario/ftmariospecialhi.c:111` and changes only the landing lag (see uncertainty 1).
- `physics_` line 1240: `OS.copy_segment(0x548F0, 0x40)`, 16 instructions of 0x800D90F0. The `bnez v0`
  at line 1241 tests the return of `ftPhysicsCheckClampAirVelXDecMax` (the `jal 0x800D8FA8` inside the copy).
  `ftPhysicsApplyFastFall` is chosen when `0x18D:4-4` (`is_fastfall`) is set.

## Uncertainties

1. Landing lag in `collision_` is 0x3EC05C29 (about 0.3757), not the 0.375F that the
   `LANDING_FSM` constant comment in asm line 1096 suggests. The asm changes only the `lui` at line 1342
   (`lui a2, LANDING_FSM`), and Mario's `ori a2, a2, 0x5C29` at asm line 1344 (the copy at 0xD0E14, the
   fragment's `ori` at 0xD0E34) survives. The port keeps the ROM value, `MARINA_USP_LANDING_LAG_COLLISION`.
   `main_` (asm line 1172) uses `lui` only, so 0.375F (0x3EC00000) is exact there.
2. Remix `ftCommonFallSpecialSetStatus` call in `main_` (asm lines 1170-1174) passes `a1 = 1.0F` as the
   drift argument. The Fox source passes `FTFOX_FIREFOX_AIR_DRIFT`, which is not the same. The ASM value
   is used. The interrupt flag is FALSE (asm line 1171), where the Fox source passes TRUE. The ASM value is used.
3. The `unknown` argument of `ftCommonFallSpecialSetStatus` is FALSE (asm `or a2,r0,r0`). Its meaning is
   not known.
4. `physics_` tests `ga` with `bnez`, so grounded means `ga == 0`. The port uses
   `ga != nMPKineticsAir`, which is the same only if `ga` is always 0 or 1.
5. `air_initial_` clears only the four bits that the `andi 0x07` mask clears. The mapping of byte
   `0x18D` follows `struct_offsets.txt` (bit 0 is the MSB). That is `is_absorb` 0x80, `absorb_lr` 0x60,
   `is_goto_attack100` 0x10 and `is_fastfall` 0x08. Those are the bits Falco's port clears, too.
6. Action ids `nFTRemixMarinaStatusUSPG = 0xFC` and `nFTRemixMarinaStatusUSPA = 0xFD` come from the
   commented constants at Marina.asm lines 166-167. The `Action` scope does not define them as live
   constants. Confirm against the action array when the build runs.
7. `change_direction_` calls the vanilla Captain turn `ftCaptainSpecialHiProcInterrupt` on
   temp variable 2 == 2 (asm lines 1195-1199). That routine also clears temp variable 2 and turns the
   fighter with the stick. The port calls it unchanged, as the ASM does.
8. `main_` uses `!(anim_frame <= 0.0F)` (negated) so a NaN frame returns, as `bc1fl` does.
