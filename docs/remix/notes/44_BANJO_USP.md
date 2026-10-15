# 44 BANJO USP (Shock Spring Jump) notes

Port: `decomp/src/ft/ftremix/banjo/ftremixbanjousp.c`, table `dFTRemixBanjoUSPProcs` (14 entries, `dFTRemixBanjoUSPProcsNum`).
Source: `.toolchain/pkgs/smashremix-src/src/Banjo/BanjoSpecial.asm`, `scope BanjoUSP` (lines 1024-1937).
Registration: `Banjo/Banjo.asm` lines 511-523 (`add_new_action`), special entries lines 544 and 547 (`dw BanjoUSP.initial_`).

## Action ids

Taken from the assembled Remix ROM, not from the macro arithmetic. `Character.BANJO_action_array` (RAM 0x804C87A0) was scanned for the routine addresses:

- USPBegin 0xEF, USPAttack 0xF0, USPAttackEnd 0xF1, USPRecoil 0xF2, USPWallSplat 0xF3.

## Routines ported (14 of 14)

Ported as table entries: initial_, begin_main_, begin_physics_, begin_collision_, attack_main_, attack_interupt_, attack_physics_, attack_collision_, attack_end_main_, collision_, recoil_move_, recoil_physics_, splat_physics_, wall_splat_main_.

Static helpers (not in the table): usp_transition_attack_, attack_end_initial_, begin_recoil_, wall_splat_initial_, check_recoil_.

## 1. Vanilla patches

None. The USP scope has no `OS.patch_start` and no `j`/`jal` hook into vanilla code.

The routines are reached only through `Character.add_new_action` (Banjo.asm 519-523). Those registrations use vanilla addresses in the non-Remix slots, which the vanilla table handles and the port does not touch:

- USPAttackEnd physics: `0x800D91EC` (ftPhysicsApplyAirVelFriction).
- USPRecoil main: `0x800D94E8` (ftAnimEndSetFall).
- USPRecoil collision: `0x800DE978`.

Vanilla functions called by the port that are not patches (the port calls them directly):

- `ftCaptainSpecialHiProcStatus` (0x801605FC) is stored in `fp->proc_status` by initial_ (asm line 1063).
- `ftCaptainSpecialHiProcInterrupt` (0x80160370) is called by attack_interupt_ when flag1 == 2 (asm line 1413, "turn subroutine").
- `ftFoxSpecialHiHoldInitStatusVars` (0x8015BFBC) is called by usp_transition_attack_ (asm line 1111).
- `ftMarioSpecialHiProcPass` (0x80156320) is the pass callback of the copied collision.

## 2. Shared Remix subsystems (REMIX-TODO)

- `REMIX-TODO(Size)`: wall_splat_initial_ (asm lines 1200-1209) calls `Size.ground_gfx.save_player_struct_._wall_bounce` with a0 = the x/y/z coordinates, a1 = 4, and a2 = the result of `jal 0x8001863C`. The port skips both calls, so the wall bounce graphic is missing. The quake call (`efManagerQuakeMakeEffect(2)`) is kept.
- `0x8001863C`: not named in `vanilla_funcs.txt` or in decomp. Its inputs are f12 = -wall_angle.x and f14 = wall_angle.y, and it returns f0 into a2. Its only consumer is the skipped Size call, so the port drops it with the Size hook. Identify it before porting the Size hook.

## 3. Uncertainties

1. **v0 branch in begin_physics_ and attack_physics_** (asm lines 1568 and 1682, `bnez v0, _check_begin`). v0 is read after `jal 0x800D8DA0` (ftPhysicsApplyFastFall) or `jal 0x800D8E50` (ftPhysicsApplyGravityDefault). Both are void, and their disassembly (ROM 0x545A0 and 0x54650) never writes v0, so the branch tests the caller's leftover v0. The port assumes v0 == 0, which takes the run path: friction in begin_physics_, and air control (`0x800D8FA8`) plus friction in attack_physics_. If the real game takes the other path, the air control and friction in attack_physics_ and the friction in begin_physics_ are skipped.
2. **ftMainPlayAnimEventsAll argument in initial_** (asm line 1079-1080). The asm passes `lw a0, 0x000(sp)`, which is never written in this frame. The port passes the fighter GObj, which is the evident intent (the other calls in the routine use 0x20(sp)). Confirm against a real run.
3. **Collision mask in attack_collision_** (asm line 1862). The asm reads `lhu 0xCC(t6)` and calls it "collision flags". The decomp names 0xCC `coll_data.mask_prev`, and `coll_data.mask_curr` is at 0xCE. The port uses mask_prev. If the value should be the current frame's mask, switch to mask_curr.
4. **recoil_move_ takes the player struct from a2** (asm line 1484-1510). The routine is the interrupt slot of USPRecoil (Banjo.asm 522), and it reads `0x17C(a2)` without loading a2. The port uses `ftGetStruct(gobj)`, which assumes the interrupt dispatcher sets a2 to the player struct.
5. **Button buffer is sticky** (asm lines 1233-1236). The asm stores the OR of the new tap with the old buffer, so once B is tapped, the bit stays set until the next initial_. The port keeps that behaviour, not a one-frame buffer. `lbu 0x1BE` reads the high byte of `button_tap`, and B is `0x40` there, which equals `B_BUTTON` (0x4000) on the u16. The port tests `B_BUTTON` on the u16.
6. **Scratch words in the status-variable union.** The asm keeps the button buffer at 0xB18 and the grounded flag at 0xB2C, which overlap `status_vars`. The port holds both in static per-player arrays (`sFTRemixBanjoUSPButtonPressBuffer`, `sFTRemixBanjoUSPIsGrounded`), reset in initial_. This is not byte-identical, but the USP never uses the union for vanilla state.
7. **Comment versus value mismatches.** The port uses the value, not the comment:
   - attack_end_initial_ (asm line 1136-1137): comment says "0.25", the value is `lui 0x3F00` = 0.5. The port uses 0.5 (`BANJO_USP_ATTACK_END_X_SCALE`).
   - X_SPEED_BACK (asm line 1027): comment says "-30.0", the value 0xC1A0 = -20.0. The port uses -20.0.
   - begin_main_ (asm line 1244): comment says "frame 8", the constant is 10.0 (`lui 0x4120`). The port uses 10.0.
   - recoil_move_ (asm line 1509-1511): comment says "1.25/0.55", the value is 1.25 (`lui 0x3FA0`). The port uses 1.25.
8. **Landing lag of the copied collisions.** The asm copies Mario's collision (`0xD0D98` 0x78 bytes and `0xD0E14` 0x44 bytes) and replaces only the first `lui` of the lag. The copied `ori a2, a2, 0x5C29` stays, so the lag is `0x3F805C29` for collision_ and `0x40005C29` for begin_collision_, not `1.0F` or `2.0F`. The port uses those exact values (1.00281250476837158203125F and 2.00562500953674316406250F). The landing lag of attack_end_main_ and wall_splat_main_ is a plain `lui`, so 1.0F and 2.0F.
9. **recoil_physics_ jump targets.** The asm loads `lui t8, 0x800E` and then `addiu` with 0x90E0 and 0x91EC. Those immediates are sign-extended, so the targets are 0x800D90E0 (ftPhysicsApplyAirVelDrift) and 0x800D91EC (ftPhysicsApplyAirVelFriction). Verified against the assembled ROM at 0x2D78CAC. The port calls the two functions directly.
10. **Begin and attack air control.** begin_physics_ and attack_physics_ end with the physics the asm shows. begin_physics_ always ends with `ftPhysicsClampAirVelXStickDefault` (0x800D9044); attack_physics_ never calls it.

## 4. Routine map

| Remix label | Port routine | Kind |
|---|---|---|
| BanjoUSP.initial_ | ftRemixBanjoUSPInitialSetStatus | special entry |
| BanjoUSP.begin_main_ | ftRemixBanjoUSPBeginProcUpdate | ProcUpdate |
| BanjoUSP.begin_physics_ | ftRemixBanjoUSPBeginProcPhysics | ProcPhysics |
| BanjoUSP.begin_collision_ | ftRemixBanjoUSPBeginProcMap | ProcMap |
| BanjoUSP.attack_main_ | ftRemixBanjoUSPAttackProcUpdate | ProcUpdate |
| BanjoUSP.attack_interupt_ | ftRemixBanjoUSPAttackProcInterrupt | ProcInterrupt |
| BanjoUSP.attack_physics_ | ftRemixBanjoUSPAttackProcPhysics | ProcPhysics |
| BanjoUSP.attack_collision_ | ftRemixBanjoUSPAttackProcMap | ProcMap |
| BanjoUSP.attack_end_main_ | ftRemixBanjoUSPAttackEndProcUpdate | ProcUpdate |
| BanjoUSP.collision_ | ftRemixBanjoUSPProcMap | ProcMap |
| BanjoUSP.recoil_move_ | ftRemixBanjoUSPRecoilProcInterrupt | ProcInterrupt |
| BanjoUSP.recoil_physics_ | ftRemixBanjoUSPRecoilProcPhysics | ProcPhysics |
| BanjoUSP.splat_physics_ | ftRemixBanjoUSPSplatProcPhysics | ProcPhysics |
| BanjoUSP.wall_splat_main_ | ftRemixBanjoUSPWallSplatProcUpdate | ProcUpdate |

Helpers: usp_transition_attack_ = ftRemixBanjoUSPTransitionAttack; attack_end_initial_ = ftRemixBanjoUSPAttackEndSetStatus; begin_recoil_ = ftRemixBanjoUSPRecoilSetStatus; wall_splat_initial_ = ftRemixBanjoUSPWallSplatSetStatus; check_recoil_ = ftRemixBanjoUSPCheckRecoil.

Port verification: `scripts/remix/syntax-check.sh` passes with no warnings. Each float constant was checked against its hex bit pattern.
