# 36 PIANO: PianoUSP port notes

Port: `decomp/src/ft/ftremix/piano/ftremixpianousp.c` (table `dFTRemixPianoUSPProcs`, 5 of 5 routines).
Source: `.toolchain/pkgs/smashremix-src/src/Piano/PianoSpecial.asm`, scope `PianoUSP` (lines 7-268), Action ids from `Piano.asm` `scope Action` (`SuperJumpChomp = 0xE1`). Parent fighter: MARIO (`Character.asm` line 3727).

## 1. Vanilla patches

None. The Piano USP routines do not patch vanilla code. Everything they need is ported in C.

## 2. Spliced vanilla code (ported as C, no action needed)

- `initial_` (special entry): no splice.
- `main_`: `OS.copy_segment(0xD7190, 0x20)` is the first 8 instructions of `ftFoxSpecialAirHiEndProcUpdate` (0x8015C750). Decoded: `if (fighter_gobj->anim_frame <= 0.0F)` then the fall-special call. Ported as the explicit C condition.
- `physics_`: `OS.copy_segment(0x548F0, 0x40)` is the start of the vanilla air physics routine (0x800D90F0). Decoded: `is_fastfall ? ftPhysicsApplyFastFall : ftPhysicsApplyGravityDefault`, then `ftPhysicsCheckClampAirVelXDecMax` (returns v0). Ported directly.
- `collision_`: `OS.copy_segment(0xD0D98, 0x78)` and `OS.copy_segment(0xD0E14, 0x44)` are `ftMarioSpecialHiProcMap` (0x80156358) with the landing `lui a2` replaced by `LANDING_FSM`. Ported as a copy of the Mario routine with `PIANO_USP_LANDING_FSM`, the same structure the Falco pilot uses.

## 3. Shared helpers and TODO markers

- None. No `REMIX-TODO` markers were needed. `ftMarioSpecialHiProcPass` and `ftCaptainSpecialHiProcInterrupt` are called as vanilla decomp functions.
- Not in the worklist and not ported: `PianoUSP.air_control_` (asm line 228). It is ported as the `static` helper `ftRemixPianoUSPAirControl`, called only from `physics_`, as in the asm.

## 4. Uncertainties

- `air_control_` (asm 228-253): the first argument passed to `ftPhysicsClampAirVelXStickRange` is the literal `8` (asm 234, `addiu a1, r0, 0x8`, "original line"). Its meaning (stick range minimum) is inferred from the decomp name only.
- `initial_` (asm 21): the ground check is `bnez t7` on `ga` (asm 20-21). The port writes `if (fp->ga == nMPKineticsGround)`, which assumes ground is 0 and air is 1 for `nMPKineticsGround` and `nMPKineticsAir`. The asm only checks non-zero, so this is equivalent when `ga` is 0 or 1.
- `physics_` (asm 133-149): the vanilla air-control call is made through `t8` with `a0 = player struct`. The BEGIN case calls `ftPhysicsCheckClampAirVelXDecMax` a second time (asm 136), and the other cases call `air_control_`. The port keeps both calls as written.
- `physics_` BEGIN_MOVE (asm 170-208): `c.le.s f2, f0` gives `stick_dir >= 0`. On that branch `vel_y -= 0.75F * (stick_dir * 0.5F)` and `vel_x = stick_dir * 0.5F`. Otherwise `vel_x = 0`. Verified against the asm, not against a running build.
- `physics_` friction (asm 148): the vanilla air-friction call `0x800D9074` (`ftPhysicsApplyAirVelXFriction`) runs only when the copied clamp returned 0 (asm 133-134), as in the asm.
- `main_` (asm 64-70): the stack argument order follows the Fox call (`ftFoxSpecialAirHiEndProcUpdate`). Argument 1 is the `1.0F` value loaded by `lui a1, 0x3F80` in the copied bytes, named `PIANO_USP_AIR_DRIFT`. Its meaning in `ftCommonFallSpecialSetStatus` is inferred from the Fox and Peach uses.
- `collision_`: Mario's `ftMarioSpecialHiProcMap` body was checked against the disassembly of the copied bytes. Only the tail (17 lines) was not compared line by line, but it matches the Falco port's tail.
- Float constants: `0x3C750000` is written as the exact decimal `0.01495361328125F`. The asm comment shows `0.0149536`, which is a truncation of the same value.
