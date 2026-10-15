# 36 PIANO DSP (Mad Piano down special): port notes

Port file: `decomp/src/ft/ftremix/piano/ftremixpianodsp.c`, table `dFTRemixPianoDSPProcs` (14 routines, all in scope).
Source: `.toolchain/pkgs/smashremix-src/src/Piano/PianoSpecial.asm`, scope `PianoDSP` (lines 273-656, 754-930).

## Status ids

Verified from the assembled Remix ROM (`build/remixasm/remix.z64`), action array at RAM 0x804BBDD0. The `main`
and `collision` pointers of each entry match the worklist addresses:

| id | action | main / interrupt / physics / collision |
|----|--------|-----------------------------------------|
| 0xE5 | DSP_Ground_Begin | ground_begin_main_ / change_direction_ / 0x800D8BB4 / ground_collision_ |
| 0xE6 | DSP_Ground_Wait | ground_wait_main_ / 0 / 0x800D8BB4 / ground_collision_ |
| 0xE7 | DSP_Ground_Absorb | ground_absorb_main_ / 0 / 0x800D8BB4 / ground_collision_ |
| 0xE8 | DSP_Ground_End | 0x800D94C4 / 0 / 0x800D8BB4 / ground_collision_ |
| 0xE9 | DSP_Air_Begin | air_begin_main_ / change_direction_ / 0x800D90E0 / air_collision_ |
| 0xEA | DSP_Air_Wait | air_wait_main_ / 0 / 0x800D90E0 / air_collision_ |
| 0xEB | DSP_Air_Absorb | air_absorb_main_ / 0 / 0x800D90E0 / air_collision_ |
| 0xEC | DSP_Air_End | 0x800D94E8 / 0 / 0x800D90E0 / air_collision_ |
| 0xED | Ground_Cmd_Throw | 0x8014A0C0 / cmd_throw_hide_ / cmd_throw_ground_physics_ / cmd_throw_ground_collision_ |
| 0xEE | Air_Cmd_Throw | 0x8014A0C0 / cmd_throw_hide_ / 0x800D90E0 / cmd_throw_air_collision_ |

The ids match the `Piano.asm` action order (DSP_Ground_Begin is the first new action, after the Piano enum ends at
0xEE). The enum in `Piano.asm` does not list the DSP names, so the ids come from the ROM.

## Helpers ported as static (not in the worklist)

Reached through the transitions and the command grab, not through the action array:
`ground_wait_initial_`, `air_wait_initial_`, `ground_end_initial_`, `air_end_initial_`, `ground_to_air_`,
`air_to_ground_`, `cmd_throw_ground_to_air_`, `cmd_throw_air_to_ground_`, `cmd_throw_ground_initial_`,
`cmd_throw_air_initial_`, `absorb_setup_`, `command_throw_setup_`.

Table data copied from the asm: `ground_to_air_table`, `air_to_ground_table` (PianoSpecial.asm 952-970) and
`absorb_struct` (973-981).

## Vanilla patches

These patches sit in vanilla code and are not written in this file. The Ness absorb ones call routines of this port,
so those two routines are non-static and their prototypes are below.

### 1. Ness ground absorb initial (ROM 0xCFCB8, RAM 0x80155278)

Asm: PianoSpecial.asm 985-1013. Decomp function at RAM 0x80155278 is not listed in `vanilla_funcs.txt`. It is probably
`ftNessSpecialLwProcAbsorb` (`decomp/src/ft/ftchar/ftness/ftnessspeciallw.c:41`); verify before inserting.
The patch replaces the call to `ftNessSpecialLwHitSetStatus` (0x80155948) for Piano.

Insert in place of the call to `ftNessSpecialLwHitSetStatus(fighter_gobj)`:

```c
#ifdef REGION_REMIX
    if (fp->fkind == REMIX_ID_PIANO)
    {
        ftRemixPianoDSPGroundAbsorbInitialSetStatus(fighter_gobj);
    }
    else
#endif
    {
        ftNessSpecialLwHitSetStatus(fighter_gobj);
    }
```

Prototype (non-static in `ftremixpianodsp.c`):
`void ftRemixPianoDSPGroundAbsorbInitialSetStatus(GObj *fighter_gobj);`

### 2. Ness aerial absorb initial (ROM 0xCFCC8, RAM 0x80155278)

Asm: PianoSpecial.asm 1017-1045. Same RAM as patch 1. The vanilla call replaced is `ftNessSpecialAirLwHitSetStatus`
(0x8015598C). Verify the two ROM offsets before inserting, because both patches declare the same RAM address.

```c
#ifdef REGION_REMIX
    if (fp->fkind == REMIX_ID_PIANO)
    {
        ftRemixPianoDSPAirAbsorbInitialSetStatus(fighter_gobj);
    }
    else
#endif
    {
        ftNessSpecialAirLwHitSetStatus(fighter_gobj);
    }
```

Prototype: `void ftRemixPianoDSPAirAbsorbInitialSetStatus(GObj *fighter_gobj);`

### 3. Command grab action fix (ROM 0xC733C, RAM 0x8014C8FC), not ported

Asm: PianoSpecial.asm 1049-1093. This is a Piano-wide grab patch, not a DSP routine. It swaps the captured
fighter's action id to `Action.ThrownDK` when the grabber is Piano. It is outside this worker's scope and was not
ported. The owner of the grab code should check it.

### 4. Absorb behaviour (ROM 0x5EBAC, RAM 0x800E33AC), not ported

Asm: PianoSpecial.asm 1098-1148. Bonus ammo for Mad Piano's neutral special. Not a DSP routine, so it was not
ported. The owner of the neutral special (or the USP worker) should take it.

## Shared Remix subsystems

None. Every call goes to a vanilla decomp function: `ftMainSetStatus`, `ftMainPlayAnimEventsAll`,
`ftNessSpecialLw_*`, `ftYoshiSpecialNSetCatchParams`, `ftParamSetCaptureImmuneMask`, `ftAnimEndCheckSetStatus`,
`mpCommon*`, `ftPhysics*`, `ftCommonWaitSetStatus`, `ftCommonFallSetStatus`.

## Uncertainties

- `absorb_struct` (asm 973-981): the first word is `1` (asm comment "not sure"). It is ported as `FTSpecialColl.kind = 1`.
  The Remix struct is 0x20 bytes and ends at size.z, so `damage_resist` is set to 0. This is an unverified assumption.
- `absorb_setup_` (asm 754-767) writes the absorb hitbox pointer at 0x0850 and sets the 0x018D bit 0x80 (`is_absorb`).
  The port follows that.
- `air_end_initial_` (asm 451-466): the comment says `DSP_Ground_End`, but the code loads `DSP_Air_End`. The port
  uses `DSP_Air_End` (0xEC), which the ROM confirms.
- `ground_collision_` (asm 618-628): the comment says `air_to_ground_`, but the code passes `ground_to_air_`. The
  port follows the code.
- `cmd_throw_ground_collision_` (asm 860-870) calls `0x800DDDDC` (`mpCommonProcFighterOnFloor`), not the
  `0x800DDE84` used by `ground_collision_`. The port follows the call.
- Transition order: `ground_to_air_` sets the aerial state, changes the action, clamps the air x velocity, then
  applies the absorb bit and the command grab. Air-to-ground skips the clamp. The port keeps this order.
- The table flags (`0x0092`, `0x0097`, `0x0804`, `0x0004`, `0x0024`) are kept as literals next to the table, as in
  the Marina port.
- `status_vars` is read through a cast overlay (`FTRemixPianoDSPStatusVars`, min_frame_timer 0x0B18 and b_not_held
  0x0B1C), which follows the Marina port. It is not the `ftremixfields.h` accessor.
- Mad Piano's wait and absorb routines use `motion_vars.flags` (temp variables 1-3) as in the asm. Temp variable 1 is
  `flag0` (0x017C), 2 is `flag1` (0x0180) and 3 is `flag2` (0x0184).
