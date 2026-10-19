# F9 Damage.asm bury and plunge (worker B)

Source: `.toolchain/pkgs/smashremix-src/src/Damage.asm` (bury 370 to 471, bury and plunge routines 577 to 910, the
`prevent_wind_push_while_buried` patch at 887). The stun side (stun_damage_action_ dispatch, flashbang, stun main) is
worker S (`ftremixdamagestun.c`), not in this note.

Port: `decomp/src/ft/ftremix/ftremixbury.c` and `ftremixbury.h` (new), one hook in `decomp/src/ft/ftparam.c`.

## Ported routines

| Asm (Damage.asm) | Line | RAM | Port | Status |
|---|---|---|---|---|
| `bury_or_plunge_initial_` | 376 | none | `ftRemixDamageBuryOrPlungeInitial` | applied. Caller (S) passes `is_start`, FALSE when already buried |
| `bury_initial_` | 426 | none | `ftRemixDamageBuryInitial` | applied. Called by ZCancel.asm (`Damage.bury_initial_`) |
| `bury_player_` | 577 | none | `ftRemixBuryPlayer` (+ `ftRemixBuryOnPlatform`, `ftRemixBurySoftPlatform`) | applied |
| `buried_main_` | 666 | none | `ftRemixBuryMain` | applied, the timer lives in `sFTRemixBuryTimer` |
| `buried_interrupt_` | 705 | none | `ftRemixBuryIgnoreProc` (proc_accessory) | applied as a no-op (the asm body is empty) |
| `buried_collision_` | 712 | none | `ftRemixBuryCollision` | applied |
| `unbury_player_` | 756 | none | `ftRemixBuryUnbury` | applied |
| `plunge_player_` | 783 | none | `ftRemixPlungePlayer` | applied |
| `plunge_ground_transition` | 822 | none | `ftRemixPlungeGroundTransition` | applied |
| `plunge_main_` | 832 | none | `ftRemixPlungeMain` | applied |
| `plunge_interrupt_` | 856 | none | `ftRemixBuryIgnoreProc` | applied as a no-op (empty body) |
| `plunge_collision_` | 874 | none | `ftRemixPlungeCollision` | applied |
| `prevent_wind_push_while_buried` | 887 | 0x800E86B4 | `ftParamSetVelPush` (ftparam.c), `#ifdef REGION_REMIX` | applied |

The vanilla function at 0x800E86B4 is `ftParamSetVelPush` (`build/remixasm/refs/vanilla_funcs.txt`, ftparam.c). The asm
skips the x and y stores of the push and jumps to the z store at 0x800E86CC, so a buried fighter only gets `vel_push.z`.

## Mapping decisions

1. Buried state: `ftRemixDamageIsBuried(fp)` is `fp->proc_update == ftRemixBuryMain`. This is the C form of the asm
   comparison of 0x09D4 with `buried_main_` (Damage.asm line 268 and 896). The stun dispatch and the wind push both use it.
2. Process offsets. The asm offsets are N64 offsets; the port's FTStruct uses LP64 pointers, so the field names are used:
   0x09D4 `proc_update`, 0x09D8 `proc_accessory`, 0x09DC `proc_interrupt`, 0x09E0 `proc_physics`, 0x09E4 `proc_map`.
   The asm comments call 0x09D8 "interrupt" and 0x09E0 "collision" in plunge_player_; the names above follow the struct
   order (confirmed by peppy's `0x09EC = proc_damage` note). The plunge writes its collision to `proc_physics` and the
   ground transition to `proc_map`, as the asm offsets say.
3. Stun timer (asm 0x026C): no FTStruct field has this name, and adding one touches a shared header. It is a static
   per-player array `sFTRemixBuryTimer[GMCOMMON_PLAYERS_MAX]`, the same pattern as `ftremixairdodge.c`. It is set in
   `ftRemixBurySetTimer` (99 + 0.7 * percent, truncated) and counted down in `ftRemixBuryMain`, which adds 3 times the
   mash delta, as the asm does (`tics + 3 * (tics - previous)`).
4. Kinetic state (0x14C): `fp->ga` (`nMPKineticsAir`).
5. Percent (0x2C): `fp->percent_damage`.
6. Action ids: `nFTCommonStatusDownBounceU` (buried action), `nFTCommonStatusShieldBreakFall` (plunge action).
7. Jump of the unbury (0x0B18 to 0x0B24): the words are the consecutive `ftCommonKneeBendStatusVars` fields `jump_force`
   (0x0B18, 0.0F), `anim_frame` (0x0B1C), `input_source` (0x0B20, 2 = BUTTON) and `is_shorthop` (0x0B24, 1 = TRUE).
   The port sets the three fields and calls `ftCommonJumpSetStatus`, as the asm does with 0x8013F880.
8. Velocities (asm 0x48 to 0x60 are `physics.vel_air`, `vel_damage_air`, `vel_ground`, the layout of AirDodge.asm).
9. Surface and soft platform (0xF6, lh): `coll_data.floor_flags`. `MAP_VERTEX_COLL_PASS` (0x4000) is the soft platform,
   the low byte is the surface checked for the hazard.
10. No-clipping ids (0xEC): `coll_data.floor_line_id`, with -1 and -2 as "no platform".
11. Wall flag (0xDC, "moving into a wall"): `coll_data.mask_curr & (MAP_FLAG_LWALL | MAP_FLAG_RWALL)`. The asm field
    could not be matched to a name with certainty. This is the least certain mapping; check it in game.
12. Joint for the 48 unit drop (0x08F8, "player joint"): `fp->joints[nFTPartsJointTopN]`. Assumed to be the top joint,
    the one `mpCommonSetFighterAir` also uses.
13. Hazard mode (`Toggles.entry_hazard_mode`): `gmRemixToggle(hazard_mode) & 0x0001`, as `gmremixcameramode.c`.
14. Sound 0x11F: `func_800269C0_275C0`, as the footstool and zcancel ports.
15. Plunge fall cap (0x90, "y velocity", -128): the port caps `physics.vel_air.y`. The asm offset 0x90 is outside the
    physics struct and could not be matched to a field. If the cap does nothing in game, look at this first.

## Deviations

- `prevent_wind_push_while_buried` keeps the asm rule: the push z is stored, the x and y are not.
- Buried-collision wall check uses `mask_curr` instead of the 0xDC word (mapping 11).
- The static timer (3) is not in the save or reset path. It is set at every bury start, so a stale value is never read.
- The no-op interrupt routines are installed as `proc_accessory`, so the process table matches the asm.

## Not ported

- ZCancel.asm `Damage.bury_initial_` call: `ftRemixDamageBuryInitial` exists now, but the REMIX-TODO in
  `ftremixzcancel.c` (FTREMIX_ZCANCEL_PUNISH_BURY) is not wired. That file is outside this worker's scope.
- The stun dispatch that calls `ftRemixDamageBuryOrPlungeInitial` (`stun_damage_action_` BURY branch): worker S.

## For the lead

- Build with the normal flags; `scripts/remix/syntax-check.sh` reports OK for `ftremixbury.c` and `ftparam.c`.
- Game check (Remix damage type 0xB, BURY): a grounded hit buries the fighter for 99 + 0.7 * percent tics; mashing
  shortens it; at zero the fighter gets a short hop (button jump). An aerial hit plunges at -50, capped at -128.
- Check mapping 11 (walk into a wall while buried, unbury expected) and mapping 15 (fall speed cap).
- Whispy's wind on a buried fighter should not move it in x or y.
