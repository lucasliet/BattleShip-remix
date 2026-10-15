# 0x48 CRASH: CrashNSP port notes

Source: `.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm`, scope `CrashNSP` (lines 5-783 and 783-907).
Output: `decomp/src/ft/ftremix/crash/ftremixcrashnsp.c`, table `dFTRemixCrashNSPProcs` / `dFTRemixCrashNSPProcsNum`.
`ftremixcrash.c` and `ftremixported.c` are untouched. The syntax check passes with no warnings.

## Coverage

- Worklist routines ported: 12 of 12 (`ground_initial_`, `air_initial_`, `ground_main_`, `air_main_`, `blocked_main_`,
  `ground_physics_`, `air_physics_`, `blocked_air_physics_`, `ground_collision_`, `air_collision_`,
  `blocked_ground_collision_`, `blocked_air_collision_`).
- Static helpers ported (not in the table): `ground_to_air_`, `air_to_ground_`, `ground_to_air_jump_`,
  `blocked_ground_to_air_`, `blocked_air_to_ground_`, `blocked_ground_initial_`, `blocked_air_initial_`,
  `shield_hit_routine_` (installed through `proc_shield`, N64 0x09F4).
- Not ported, REMIX-TODO stubs: `create_spin_gfx_` and `setup_spin_gfx_` (GFX), `spin_detect_item_`, and the item fling
  inside `ground_main_` and `air_main_` (Item). See "REMIX-TODO markers" below.

## Registration (integration task, not done here)

- `decomp/src/ft/ftremix/ftremixported.c` must add `extern const FTRemixProc dFTRemixCrashNSPProcs[];`,
  `extern const s32 dFTRemixCrashNSPProcsNum;` and a `{ dFTRemixCrashNSPProcs, &dFTRemixCrashNSPProcsNum }` entry in
  `dFTRemixPortedProcTables`. Until then every CrashNSP routine falls back to the parent (MARIO).
- `dFTRemixCrashProcs` in `ftremixcrash.c` stays as the empty placeholder (USP/DSP workers own the other scopes).
- Shield hitlag hook (see "Vanilla patches"): the Crash descriptor, and the Kirby and J Kirby descriptors, must set
  `shield_hitlag_skip = ftRemixCrashNSPShieldHitlagSkip` in `port/fighter_registry`.

## Vanilla patches

1. `shield_hitlag_patch_` (asm 909-963, RAM 0x800E651C, ROM 0x61D1C). Already handled by the port, not by this file:
   `decomp/src/ft/ftmain.c:4167-4180` (`ftMainProcParams`, `vanilla_funcs.txt` 0x800E61EC) calls
   `port_fighter_shield_hitlag_skip(fkind, gobj, status_id)`, which zeroes `hitlag_tics` when it returns nonzero.
   The C to add, if the registry route is not used, is:
   ```c
   #ifdef REGION_REMIX
   if (ftRemixCrashNSPShieldHitlagSkip(fighter_gobj, fp->status_id)) { fp->hitlag_tics = 0; }
   #endif
   ```
   `ftRemixCrashNSPShieldHitlagSkip` is non-static in this file, with prototype
   `int ftRemixCrashNSPShieldHitlagSkip(GObj *fighter_gobj, int status_id);`. It returns TRUE for the Crash ids 0xE1
   and 0xE2 and the Kirby ids 0x1B0 and 0x1B1. The asm's Kirby hat check (`lb 0x0980`, must be 0x2A, asm 941-944) is
   not repeated: the Kirby blocked ids are set only by the Crash copy.
2. No other vanilla patch is needed. The patch-free code calls only vanilla or libultra functions, listed below.

Vanilla calls: `ftMainSetStatus` (0x800E6F24), `ftMainPlayAnimEventsAll` (0x800E0830), `mpCommonSetFighterWaitOrFall`
(0x800DEE54), `mpCommonSetFighterGround` (0x800DEE98), `mpCommonSetFighterAir` (0x800DEEC8),
`mpCommonProcFighterOnFloor` (0x800DDDDC), `mpCommonProcFighterLanding` (0x800DE6E4), `ftPhysicsApplyGravityClampTVel`
(0x800D8D68), `ftPhysicsApplyAirVelFriction` (0x800D91EC), `ftCommonKneeBendGetInputTypeCommon` (0x8013F474),
`efManagerDustHeavyDoubleMakeEffect` (0x800FF3F4).

Note: `mpCommonProcFighterOnFloor` (0x800DDDDC) is used by the ground collisions. It is not `mpCommonProcFighterOnEdge`
(0x800DDE84) as in the Conker port.

## Status ids

- Crash (from `Crash.asm` `scope Action`): `NSPG` 0xDF, `NSPA` 0xE0, `NSPGBlocked` 0xE1, `NSPABlocked` 0xE2.
- Kirby: `CRASH_NSP_Ground` 0x1AE, `CRASH_NSP_Air` 0x1AF, `CRASH_NSP_Ground_Blocked` 0x1B0, `CRASH_NSP_Air_Blocked` 0x1B1.
  Derived with the same method as the Conker and Mewtwo notes: KIRBY parent size 0x67C / 0x14 = 83, plus the index of
  the `add_new_action_params(KIRBY, ...)` calls (Kirby.asm 307-310 is index 127 to 130, counted from line 180, all
  contiguous), plus 0xDC. 220 + 83 + 127 = 430 = 0x1AE. Verify against the KIRBY action array in `remix.z64`
  (as the Mewtwo note did) before trusting these.
- The Crash/Kirby choice is `fp->fkind == REMIX_ID_CRASH` (asm `Character.id.CRASH` compare, lines 31, 76, 709, 767,
  818, 846). Any other fkind gets the Kirby id.

## Translation choices

- Transition flags 0x807 (asm argument 4, "3C FGM, attached gfx, gfx routines, hitboxes") are written as
  `LOOPSFX | EFFECT | COLANIM | HIT`. The mapping of the four bits to these names follows the asm comment only.
- Shield velocities and the hitlag-over flag (N64 status vars 0x0B18, 0x0B1C, 0x0B20, 0x0B24) are per player in
  `sFTRemixCrashNSPShieldVars[fp->player]`, as Mewtwo's teleport state is (`ftremixmewtwousp.c`).
- Shield test (`Character.get_hitbox_collision_flags_`, Character.asm 3322-3346): the asm ORs flag bytes at hitbox
  offsets 0x60, 0x68, 0x70, 0x78 for 3 hitboxes (stride 0xC4, `attack_state` at +0). The port tests
  `attack_colls[0..2].attack_records[0..3].victim_flags.is_interact_shield`. See uncertainty 2.
- `ground_main_`: when the animation has ended, the asm transitions to idle and then still runs the item check (asm
  344-347). The jump check is skipped on that frame. The port matches this.
- `ground_to_air_jump_`: the jump smoke is `efManagerDustHeavyDoubleMakeEffect(p_translate, 1, 1.0)`
  (asm 744-748, `0x800FF3F4` is `efManagerDustHeavyDoubleMakeEffect`).
- Float constants are exact decimals; each is annotated with its hex bit pattern in the source.
- `ground_physics_` and `air_physics_` follow the asm control flow exactly, including the `bc1fl` and `bnezl` delay
  slot semantics, and the neutral-stick path that zeroes x velocity and returns (ground) or falls through to gravity
  (air).
- `blocked_air_initial_` does not clear fast fall (the asm has no fast-fall code there). The aerial branch of
  `shield_hit_routine_` does clear it (asm 200-204).

## REMIX-TODO markers

- `REMIX-TODO(GFX)`: `ftRemixCrashNSPCreateSpinGFX` is an empty stub, called by `ground_initial_` and `air_initial_`.
  The attached spin GFX and its colour tables (asm 119-155, 862-887, 967-1043) are not ported. `setup_spin_gfx_` is
  therefore not called.
- `REMIX-TODO(Item)`: `ftRemixCrashNSPSpinItem` is an empty stub, called in place of the item check in `ground_main_`
  and `air_main_`. The skipped logic is `spin_detect_item_` (asm 264-329) and the fling (asm 371-440) with
  `item_fling_` (asm 891-907) and `flung_item_state_table` (asm 1045, a Remix item table). Crash does not pick up
  items with the spin.

## Uncertainties

1. Kirby action ids (0x1AE to 0x1B1) are derived, not read from `remix.z64` (see Status ids).
2. Shield test. The asm reads hitbox flag bytes at 0x60, 0x68, 0x70, 0x78 (Character.asm 3322-3346). The port reads
   `GMHitFlags.is_interact_shield` of the attack records instead. This is the bit 0x40 of the first byte, the same
   bit the asm tests. `itprocess.c:222` sets this bit on a shield collision, so the mapping is plausible. Verify it
   against a shield bounce in the Remix build.
3. J Kirby jump speed. Asm 740-742 compares `at` (the float bits of `JUMP_SPEED`) with the character id, so J Kirby
   never takes `KIRBY_JUMP_SPEED`. The port keeps this (J Kirby uses 72.0). It is probably a Remix bug.
4. `shield_hit_routine_` checks `ga != nMPKineticsGround` as the asm's `bnez` on the kinetic state (asm 190-192). This
   assumes `nMPKineticsGround` is 0 (`decomp/src/mp/mpdef.h`).
5. Fast-fall clear in `air_initial_` and in the aerial shield branch uses the same mask as the Falco pilot
   (`andi 0x07`): it clears `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall`.
6. `shield_hit_routine_` clears `vel_air.y` before the grounded or aerial branch (asm 190-192 delay slot). Kept.
7. The Kirby hat check of the shield hitlag hook (asm 941-944) is not repeated (see Vanilla patches 1).
