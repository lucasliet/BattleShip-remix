# 0x37 WOLF worker notes (F5)

Port file: `decomp/src/ft/ftremix/wolf/ftremixwolf.c`, table `dFTRemixWolfProcs` (11 entries, `dFTRemixWolfProcsNum`).
Source: `.toolchain/pkgs/smashremix-src/src/Wolf/WolfSpecial.asm` (lines 1-957).
Syntax check: `syntax-check OK` with `-Wall`, no warnings.

## Routines ported

Worklist: 11 routines, all 11 in the table.

| Remix routine | Address | C routine | Role |
|---|---|---|---|
| WolfNSP.main | 0x80553CDC | ftRemixWolfSpecialNProcUpdate | ProcUpdate (blaster part only partly ported, see TODO) |
| WolfNSP.air_collision_ | 0x8055415C | ftRemixWolfSpecialNProcMap | ProcMap |
| WolfUSP.initial_ground | 0x805541E8 | ftRemixWolfSpecialUInitialGround | entry (status 0xE4) |
| WolfUSP.initial_air | 0x80554268 | ftRemixWolfSpecialUInitialAir | entry (status 0xE3) |
| WolfUSP.main_ground | 0x805542EC | ftRemixWolfSpecialUProcInterruptGround | ProcInterrupt |
| WolfUSP.main_air | 0x8055436C | ftRemixWolfSpecialUProcInterruptAir | ProcInterrupt |
| WolfUSP.change_direction_ | 0x80554474 | ftRemixWolfSpecialUProcUpdate | ProcUpdate (turn) |
| WolfUSP.physics_ | 0x805544B8 | ftRemixWolfSpecialUProcPhysics | ProcPhysics |
| WolfUSP.collision_ | 0x80554744 | ftRemixWolfSpecialUProcMap | ProcMap |
| WolfUSP.main_2 | 0x80554804 | ftRemixWolfSpecialUPartTwoProcUpdate | ProcUpdate (part two) |
| WolfDSP.physics_ | 0x805548E4 | ftRemixWolfSpecialDProcPhysics | ProcPhysics |

Helpers kept static: `ftRemixWolfUpdateButtonBuffer`, `ftRemixWolfSpecialNAirToGround` (WolfNSP.air_to_ground_),
`ftRemixWolfSpecialUInitStatus`, `ftRemixWolfSpecialUTransition*` (usp_2_transition_ground/air),
`ftRemixWolfSpecialUInterrupt`, `ftRemixWolfSpecialUAirAccel`, `ftRemixWolfSpecialUAirControl`,
`ftRemixWolfSpecialUPhase*`, `ftRemixWolfSpecialUApplyPhase`, `ftRemixWolfSpecialUSlashGraphics`.

Not ported (not in worklist, not referenced by the table): `WolfNSP.projectile_stage_setting`,
`WolfNSP.main__blaster_duration`, `WolfNSP.main__blaster_reflection`, the blaster weapon descriptor
(`_blaster_projectile_struct`), `_blaster_fireball_struct`, `WolfUSP.button_press_buffer` (ported as a file-local array).

Status ids: 0xE1 NSP ground, 0xE3 USP initial air, 0xE4 USP initial ground, 0xE6 USP part two air,
0xE8 USP part two ground, Kirby WOLF_NSP_Ground 0xEB (Kirby.asm line 356, `add_new_action`).

## Approximations and REMIX-TODO markers

1. `REMIX-TODO(weapon)` in `ftRemixWolfSpecialNProcUpdate`: the blaster is not spawned. The Remix code calls
   `wpManagerMakeWeapon` (0x801655C8) with `_blaster_projectile_struct` (asm 281-295). The flag0 clear is kept.
   Blaster spec for a follow-up: origin from `gmCollisionGetFighterPartsWorldPosition` (0x800EDF24) into a
   zeroed Vec3f; desc procs: update = blaster_duration (asm 130), map = itLGunWeaponAmmoProcMap (0x80175914),
   hit/shield/setoff/absorb = itLGunWeaponAmmoProcHit (0x80175958), hop = wpBossBulletProcHop (0x8016DD2C),
   reflector = blaster_reflection (asm 207); `o_attributes` 0 and `p_weapon` = the file-6 pointer
   (`fp->data->p_file_special1`); flags 0x80000000; the fireball struct (asm 298-311) has duration 100, max speed 200,
   min speed 22, initial speed 22. Uncertain: `OS.copy_segment(0xE3268, 0x2C)` (asm 96), the velocity code
   (asm 98-119, calls 0x800303F0 and 0x80167FA0), the 0x29C free-space multiplier and the 0x268 duration field.
2. `REMIX-TODO(PokemonAnnouncer)` in `ftRemixWolfSpecialUSlashGraphics`: `PokemonAnnouncer.slash_announcement_` (asm 906)
   is skipped.
3. Up special physics, tail (asm 793, `OS.copy_segment(0x548F4, 0x58)`): this copies the tail of vanilla
   `ftPhysicsApplyAirVelDrift` (0x800D90E0). The port calls `ftPhysicsApplyAirVelDrift` for the END_MOVE tail. The
   copy ranges (0x40 at the head, 0x58 at the tail) overlap in the source, so the exact tail is unverified. If the
   tail does not include the gravity call, END_MOVE gravity is applied twice here.
4. Up special physics, head (asm 671-672): the copy ranges match the Drift prefix (gravity, then
   `ftPhysicsCheckClampAirVelXDecMax`), and `bnez v0` (asm 672) is read as "skip air control and friction when
   clamped". This is inferred from the copied prefix, not verified against the bytes.
5. The velocity constants in the asm comments are stale. The hex values used are: X_SPEED 0x435C0000 = 220.0 (comment
   says 100.0, asm 367), Y_SPEED 0x42F00000 = 120.0 (comment 40.0, asm 368), Y_INPUT 0x3F4C0000 = 0.796875 (comment
   0.5, asm 370). The port uses the hex values.
6. DSP physics (asm 944-955): the gravity call (asm 948-951, `ftPhysicsApplyGravityClampTVel`) is never executed because
   of the unconditional `beq` at 946. The port drops it. The delay-slot store at 947 writes `gravity_delay` after the
   copied code; the port guards the decrement with `!= 0`. If the copied prefix computes `gravity_delay - 1` even when
   it is 0, the Remix code would store -1 there. Unverified.
7. Slash state (asm 867-916): the `is_allow_interrupt` argument at asm 882 is the stale t6 register. The port passes the
   flag0 value read on entry (`slash_state != 0`). t6 can be clobbered by the calls at 906 and 909, so this is unverified.
   flag0 is written with `sb` to 0x17C (asm 916, the first byte of the word); the port stores semantic 1. All readers
   test only 0 or 2, so this is equivalent.
8. Button buffer (asm 472-480 and 521-529): the asm loads the byte at 0x1BE (`lbu`) and tests 0x40. The port keeps a
   `u16` buffer from `fp->input.pl.button_tap` and tests `B_BUTTON`, as the Falco pilot does.
9. Crossed action table (Wolf.asm 233-234): 0xE4 uses `WolfUSP.main_air` and 0xE3 uses `WolfUSP.main_ground`, the
   reverse of the names. The port follows the asm, so the address mapping is unchanged.

## Vanilla patches

None of the Wolf files hook into this scope. The `OS.patch_start` blocks in Wolf.asm (lines 391-617) patch
wolf reflect/graphics code and do not reference WolfNSP, WolfUSP or WolfDSP. Nothing needs to be added under
`REGION_REMIX` for this worker.

## Vanilla calls used by the port

ftMainSetStatus (0x800E6F24), ftMainPlayAnimEventsAll (0x800E0830), ftFoxSpecialHiHoldInitStatusVars (0x8015BFBC),
ftCaptainSpecialHiProcInterrupt (0x80160370, Captain Falcon's turn), ftAnimEndCheckSetStatus (0x800D9480),
mpCommonProcFighterLanding (0x800DE6E4), mpCommonSetFighterGround/Air, mpCommonSetFighterWaitOrFall (0x800DEE54),
ftCommonFallSpecialSetStatus (0x801438F0), ftCommonLandingFallSpecialSetStatus, ftParamProcStopEffect (0x800E9C3C),
efManagerCaptainFalconPunchMakeEffect (0x80101F84), ftPhysicsCheckClampAirVelXDecMax (0x800D8FA8),
ftPhysicsClampAirVelXStickRange (0x800D8FC8), ftPhysicsApplyAirVelXFriction (0x800D9074),
ftPhysicsApplyAirVelDrift (0x800D90E0), ftPhysicsApplyGravityDefault/FastFall.

## Uncertainties with asm line numbers

Listed above under items 3, 4, 6 and 7. The remaining semantics follow the asm directly.

## Blaster

Resolves `REMIX-TODO(weapon)` in `ftRemixWolfSpecialNProcUpdate`, which now calls `ftRemixWolfSpecialNCreateBlaster`.
Item 1 of the approximations above is superseded by this section. Source: `WolfSpecial.asm` lines 1-330 (`WolfNSP.main`,
`projectile_stage_setting`, `blaster_duration`, `blaster_reflection`, the descriptor and fireball struct). The blaster
procs are static and are not in `dFTRemixWolfProcs`, since the worklist does not list them.

Ported pieces:

- Spawn: `FTStruct.joints[16]` (N64 0x0928) gives the origin through `gmCollisionGetFighterPartsWorldPosition`.
  `wpManagerMakeWeapon(fighter, &desc, &pos, WEAPON_FLAG_COLLPROJECT)` (flags 0x80000000, asm line 297 `lui a3, 0x8000`).
- Descriptor, filled on the stack per spawn: `p_weapon = fp->data->p_file_special1` (file 6, WOLF_PROJECTILE_HITBOX).
  Procs: update = `ftRemixWolfSpecialNBlasterProcUpdate` (blaster_duration), map = `itLGunWeaponAmmoProcMap` (0x80175914),
  hit, shield, setoff and absorb = `itLGunWeaponAmmoProcHit` (0x80175958), hop = `wpBossBulletProcHop` (0x8016DD2C),
  reflector = `ftRemixWolfSpecialNBlasterProcReflector` (blaster_reflection).
- Init: `lifetime` = 100 (N64 0x268, `WPStruct.lifetime` in the LP64 table at 0x264), speed multiplier = 1.0 (N64 0x29C),
  velocity from `ftRemixWolfBlasterSetVelocity`, `mobj->palette_id` = 0.0 (asm 0x88 of the MObj), then `wpMainVelSetModelPitch`
  (0x80167FA0).
- Velocity: `vel.x = cos(angle) * 22 * lr`, `vel.y = sin(angle) * 22`, `vel.z = 0`. `cos` is 0x80035CD0 and `sin` is
  0x800303F0 (both comments in `AirDodge.asm`, lines 426 and 432), mapped to `__cosf` and `__sinf`.
  The copy_segment fragment (0xE3268, 0x2C) selects the ground angle (fireball 0x18) or the air angle (0x1C) by
  `fp->ga == nMPKineticsAir` (asm `lw 0x14C`, `bnel`). Both angles are 0.0 in the fireball struct, so the choice has no effect.
- Update (blaster_duration): `vel.x *= 1.03125`, clamped to +-(200 * multiplier), then size from |vel.x|. `scale.x = (|v| + 66) * 0.25 / 22`,
  `scale.y = 1 / (scale.x * 0.5 + 0.5)` (DObj scale at 0x40 and 0x44).
- Reflection (blaster_reflection): multiplier = pi/2 (0x3FC90FDB) for Wolf, else 1.0. The multiplier is stored, lifetime
  is reset to 100, `vel.x *= multiplier`, then `wpMainReflectorSetLR(wp, fp)` (0x801680EC). Rotation Y is +pi/2 when
  `vel.x >= 0`, else -pi/2 (asm 0x34 on the DObj, `rotate.vec.f.y`). Returns FALSE.

Uncertainties with asm line numbers:

1. Reflection check (asm reflection, `lw t0, 0x0008(v0)` then `bnel t0, Character.id.WOLF`): `v0` is a register left over from
   the caller, so the asm compares an unknown value. The port uses `ftGetStruct(wp->owner_gobj)->fkind == REMIX_ID_WOLF`,
   taking the owner as the reflecting fighter, which matches how the asm reads `lr` through the owner afterwards. Unverified.
2. Speed multiplier storage: the asm writes a float at WPStruct 0x29C, which is the start of `weapon_vars`. No typed float
   member exists in `wpStatusVars`, so `ftRemixWolfBlasterSpeedMul` casts `&wp->weapon_vars` to `f32 *`. Only this blaster
   uses the slot, and the Fox blaster does not touch `weapon_vars`.
3. Kind: the second descriptor word (asm line 282, `dw 0x00000000`) is stored as raw kind 0 (`nWPKindFireball` value). The asm
   does not name a kind, so the port keeps 0 and does not use `nWPKindBlaster`. Whether the weapon manager branches on kind here is unverified.
4. Origin joint: the asm loads `0x0928(v0)`, which is `joints[16]` (the table base is `joints` at 0x08E8). Conker's
   `CONKER_NSP_SPAWN_JOINT` uses `joints[17]` at 0x092C, a different joint. Both are as the asm states them.
5. Data not ported (not code and not referenced by the descriptor): `OS.copy_segment(0x1038A0, 0x30)` (fireball
   struct tail), `OS.copy_segment(0x103904, 0x0C)` (descriptor tail), `OS.copy_segment(0xE396C, 0x38)` and
   `OS.copy_segment(0xE39A8, 0x30)` (`_hitbox_end`, no entry point references it). The descriptor's fields after
   slot 13 are not used by this port.
6. Fireball struct float at 0x0C (gravity), 0x10, 0x14 and 0x24 are zero and not read by the blaster procs, so they are omitted.
7. `PokemonAnnouncer.slash_announcement_` is unchanged. Its `REMIX-TODO(PokemonAnnouncer)` marker stays in place, as the brief asks.
