# 4C LANKY DSP (Lanky Kong down special)

Port file: `decomp/src/ft/ftremix/lanky/ftremixlankydsp.c`, table `dFTRemixLankyDSPProcs` / `dFTRemixLankyDSPProcsNum`.
Source: `.toolchain/pkgs/smashremix-src/src/Lanky/LankySpecial.asm`, scope `LankyDSP` (lines 1012 to 2141).
Status ids: `Lanky.asm` action table, DSP entries 0xEB (DSPGBegin) to 0xF9 (DSPACancel).

## Coverage

- Worklist routines ported and in the table (15 of 15): `air_collision_`, `air_initial_`, `air_wait_main_`,
  `begin_main_`, `cancel_main_`, `ground_collision_`, `ground_initial_`, `ground_physics_`, `ground_wait_main_`,
  `jump_main_`, `jumpsquat_interrupt_`, `jumpsquat_main_`, `landing_main_`, `move_main_`, `turn_main_`.
- Helpers kept `static`, not in the table (called by the worklist routines, not listed in 4C_LANKY.md):
  `ground_to_air_`, `air_to_ground_` (passed as proc_map), `wait_initial_`, `move_initial_`, `turn_intial_`,
  `landing_initial_`, `taunt_initial_`, `jumpsquat_initial_`, `jump_initial_`, `plat_drop_initial_`,
  `cancel_initial_`, `end_initial_`, and the shared input chains.
- Not ported: `lanky_fighter_stance_bonus_` (see Vanilla patches). It is a patch, not a status routine.
- No weapons (`wpManagerMakeWeapon`) and no custom Remix items are spawned by this scope, so no WPDesc and no
  `REMIX-TODO(Item.*)` markers.

## Vanilla patches

1. `lanky_fighter_stance_bonus_` (asm lines 2141-2162): `OS.patch_start(0x10EEB0, 0x80190650)`. It makes Lanky's
   DSP taunt (ending action 0xF1, OrangStandTaunt) count as a taunt bonus in 1P modes. Not verified in
   `vanilla_funcs.txt`: the address 0x80190650 falls inside `sc1PTrainingModeStartScene` (0x801905F4 to 0x801906D0
   in the map), which is probably not the owning function. Confirm the owner before inserting.
   Suggested C, under `#ifdef REGION_REMIX` in the owning vanilla function:
   ```c
   /* Remix lanky_fighter_stance_bonus_ (RAM 0x80190650): Lanky's DSP taunt counts as taunt in 1P modes. */
   sb32 ftRemixLankyIsDSPTauntBonus(s32 ending_action, s32 char_id)
   {
       return (ending_action == 0xF1) && (char_id == REMIX_ID_LANKY);
   }
   ```
   The owning function then takes the bonus path (RAM 0x80190664, `lw t9, 0x0030(t2)`) when the helper returns TRUE,
   and otherwise uses `at = Action.Taunt`. The Action.Taunt value was not resolved.
   The helper is `sb32`, not static, if it is called from vanilla code. The patch is not ported in this worker's file.

## Shared subsystems and REMIX-TODO

- `REMIX-TODO(gobj-store)` in `ftRemixLankyDSPCancelProcUpdate`: asm line 1981 is the delay slot of
  `jal 0x800DEE54` (mpCommonSetFighterWaitOrFall) and stores the merged buttons with `sh t6, 0x01BE(a0)`.
  `a0` is the GObj here, not the player struct (`v1`), so the store lands on GObj+0x1BE. Not ported, since the
  target is outside the port's fighter state. Confirm this is a Remix bug and not a deliberate write.

## Vanilla functions used (identification)

| RAM | Name used | Source |
|-----|-----------|--------|
| 0x800E6F24 | ftMainSetStatus | vanilla_funcs.txt |
| 0x800E0830 | ftMainPlayAnimEventsAll | vanilla_funcs.txt |
| 0x800DEE54 | mpCommonSetFighterWaitOrFall | vanilla_funcs.txt |
| 0x800DEE98 | mpCommonSetFighterGround | vanilla_funcs.txt |
| 0x800DEEC8 | mpCommonSetFighterAir | vanilla_funcs.txt |
| 0x800DDDDC | mpCommonProcFighterOnFloor | vanilla_funcs.txt |
| 0x800DE80C | mpCommonProcFighterCliff | NOT in vanilla_funcs.txt. Inferred: it follows mpCommonCheckFighterCliff (0x800DE7D8) and its comment matches. Banjo's notes use the same call. |
| 0x800D8EB8 | ftPhysicsClampAirVelXMax | vanilla_funcs.txt. The Remix asm comment says "momentum capture?" |
| 0x8013F474 | ftCommonKneeBendGetInputTypeCommon | vanilla_funcs.txt |
| 0x80141E60 | ftCommonPassCheckInputSuccess | vanilla_funcs.txt |
| 0x8014019C | ftCommonJumpAerialCheckInterruptCommon | vanilla_funcs.txt |
| 0x8000BB04 | gcSetAnimSpeed | NOT in vanilla_funcs.txt. Inferred from `ChargeSmashAttacks.asm` and `command.asm` (fsm subroutine) and `ftremixcommand.c` (fsm command calls gcSetAnimSpeed). |
| 0x8014DB30 | tail of ftDonkeyThrowFJumpSetStatus | `ftdonkeythrowffall.c`. `jump_initial_` jumps into its jump velocity switch. Ported as `ftRemixLankyDSPSetJumpVelocity`. |

## Uncertainties

- Status ids 0xEB-0xF9 were inferred from the `action_string_table` order (Lanky.asm lines 45-80, entries 0xEB
  "OrangStandBeginGround" to 0xF9 "OrangStandCancelAir") and the `add_new_action` order. Cross-check: the stance
  bonus uses 0xF1 for OrangStandTaunt, which is DSPTaunt in the list.
- `jumpsquat_main_` (asm 1796-1838): the button bits are read from FTStruct 0x01C0 (`input.pl.button_release`) and
  masked with 0x000F. The asm does not say what those bits mean. Check against the Remix input layout.
- `move_main_` (asm 1670-1676, `_begin_wait`): the asm tests the x velocity with an integer compare on its bits.
  The port uses a float compare (`== 0.0F`), so -0.0 counts as zero here but not in the asm.
- `ground_physics_` (asm 1991-2078): the move-timer write is `*ftRemixFighterWord(fp, 0x0B20) = -1`. The overlay
  for cancel (`FTRemixLankyDSPCancelVars`, 8 bytes at N64 0x0B20) shares those words. The port keeps the same
  word addresses, but the byte layout of the overlay is the port's, not N64's. Only the port's own readers see
  it, so this is consistent within the port.
- `cancel_initial_` (asm 1268-1311): the `bltz` checks on `lbu` values can never be taken (zero-extended), so they
  were dropped. The `bnel` stores become "tap < limit ? 1 : tap".
- `jump_initial_` (asm 1221-1237): the 5th ftMainSetStatus argument is supplied by the delay slot of the jal at
  0x8014DB30 (flags = FTSTATUS_PRESERVE_NONE). The port passes 0 explicitly.
- `jumpsquat_interrupt_` and `jump_main_`: `jump_main_` has no shield cancel (asm 1870-1927), but `ground_wait_main_`,
  `move_main_`, and `turn_main_` do. The port keeps this split (`ftRemixLankyDSPTryCancel`'s `is_shield_cancel`).
- `wait_initial_` picks the status id from `fp->ga` (kinetic state 0x014C), with nMPKineticsGround as "grounded".
- `end_initial_` (asm 1317-1354): the velocity adjustments run for both ground and air. Ground and air differ only
  in the status id. The port follows that.
