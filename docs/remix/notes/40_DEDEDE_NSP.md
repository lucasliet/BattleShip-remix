# 0x40 DEDEDE NSP (Inhale): port notes

Source: `.toolchain/pkgs/smashremix-src/src/Dedede/DededeSpecial.asm`, scope `DededeNSP` (lines 584 to about 2446).
Port: `decomp/src/ft/ftremix/dedede/ftremixdededensp.c`, table `dFTRemixDededeNSPProcs` (12 entries) and `dFTRemixDededeNSPProcsNum`.
Scope: the 12 `DededeNSP.*` routines in `build/remixasm/worklists/40_DEDEDE.md`. All 12 are ported. `ftremixdedede.c` and the USP and DSP files are not touched.
Build: not built, per the brief. `scripts/remix/syntax-check.sh` prints `syntax-check OK` with no warnings.

## Ported routines (12 of 12)

| Remix label | Port | Table role |
|---|---|---|
| `ground_begin_initial_` | `ftRemixDededeNSPGroundBeginInitial` | special entry |
| `air_begin_initial_` | `ftRemixDededeNSPAirBeginInitial` | special entry |
| `ground_begin_main_` | `ftRemixDededeNSPGroundBeginProcUpdate` | proc_update |
| `air_begin_main_` | `ftRemixDededeNSPAirBeginProcUpdate` | proc_update |
| `ground_idle_interrupt_` | `ftRemixDededeNSPGroundIdleProcInterrupt` | proc_interrupt |
| `ground_walk_interrupt_` | `ftRemixDededeNSPGroundWalkProcInterrupt` | proc_interrupt |
| `air_fall_interrupt_` | `ftRemixDededeNSPAirFallProcInterrupt` | proc_interrupt |
| `ground_walk_collision_` | `ftRemixDededeNSPGroundWalkProcMap` | proc_map |
| `inhale_loop_ground_to_air_check_` | `ftRemixDededeNSPInhaleLoopGroundProcMap` | proc_map |
| `inhale_loop_air_to_ground_check_` | `ftRemixDededeNSPInhaleLoopAirProcMap` | proc_map |
| `ground_spit_main_` | `ftRemixDededeNSPSpitProcUpdate` | proc_update |
| `air_spit_main_` | `ftRemixDededeNSPSpitProcUpdate` | proc_update |

Static helpers (not in the table): `ftRemixDededeNSPBeginInitial`, `ftRemixDededeNSPLoopInitial`, `ftRemixDededeNSPGroundLoopInitial` (`ground_loop_initial_`, a pointer target of `ground_begin_main_`), `ftRemixDededeNSPAirLoopInitial` (`air_loop_initial_`), `ftRemixDededeNSPInhaleLoopGroundToAirInitial` and `ftRemixDededeNSPInhaleLoopAirToGroundInitial` (pointer targets of the check routines), `ftRemixDededeNSPGroundWalkInitial` (`ground_walk_initial_2_`), `ftRemixDededeNSPGroundWalkTransitionCheck`, `ftRemixDededeNSPGroundWalkToIdleInitial` and `...TransitionCheck` (`ground_walk_to_idle_*`), `ftRemixDededeNSPGroundWalkFallInitial` (`ground_walk_fall_initial_`, pointer target), `ftRemixDededeNSPCheckForSpit` (`check_for_spit_`), `ftRemixDededeNSPSpitTransition` with `ftRemixDededeNSPGroundSpitTransition` and `ftRemixDededeNSPAirSpitTransition` (`ground_spit_transition_`, `air_spit_transition_`, pointer targets), `ftRemixDededeNSPSpitShootCheck` (`spit_shoot_check_`), `ftRemixDededeNSPSpitPlayer`, `ftRemixDededeNSPSpatDeceleration` (`spat_deceleration_`, stored in the held player's proc_physics).

Routines that are in `DededeNSP` but not in the worklist and not reached by the ported routines are not ported (see "Not ported" below).

## Kirby status ids

Kirby ids are `0xDC + i`, where `i` is the entry index in `Character.KIRBY_action_array` (RAM 0x804A1BA0, read from `build/remixasm/remix.z64`). The entries whose routines are DededeNSP's sit at indices 144 to 162. The method was checked against the Dedede array: entry 11 (`ground_begin_main_`) is id 0xE7, and entry 17 (`spit_main_`) is id 0xED.

| Kirby action | Id | Dedede action | Id |
|---|---|---|---|
| DEDEDE_NSP_BEGIN_GROUND | 0x16C | NSP_BEGIN_GROUND | 0xE7 |
| DEDEDE_NSP_LOOP_GROUND | 0x16D | NSP_LOOP_GROUND | 0xE8 |
| DEDEDE_NSP_PULL_GROUND | 0x16E | NSP_PULL_GROUND | 0xEA |
| DEDEDE_NSP_SWALLOW_GROUND | 0x16F | NSP_SWALLOW_GROUND | 0xEB |
| DEDEDE_NSP_IDLE_GROUND | 0x170 | NSP_IDLE_GROUND | 0xEC |
| DEDEDE_NSP_SPIT_GROUND | 0x171 | NSP_SPIT_GROUND | 0xED |
| DEDEDE_NSP_TURN_GROUND | 0x172 | NSP_TURN_GROUND | 0xEE |
| DEDEDE_NSP_END_GROUND | 0x173 | NSP_END_GROUND | 0xEF |
| DEDEDE_NSP_BEGIN_AIR | 0x174 | NSP_BEGIN_AIR | 0xF0 |
| DEDEDE_NSP_LOOP_AIR | 0x175 | NSP_LOOP_AIR | 0xF1 |
| DEDEDE_NSP_PULL_AIR | 0x176 | NSP_PULL_AIR | 0xF2 |
| DEDEDE_NSP_SWALLOW_AIR | 0x177 | NSP_SWALLOW_AIR | 0xF3 |
| DEDEDE_NSP_FALL | 0x178 | NSP_FALL | 0xF4 |
| DEDEDE_NSP_SPIT_AIR | 0x179 | NSP_SPIT_AIR | 0xF5 |
| DEDEDE_NSP_TURN_AIR | 0x17A | NSP_TURN_AIR | 0xF6 |
| DEDEDE_NSP_END_AIR | 0x17B | NSP_END_AIR | 0xF7 |
| DEDEDE_NSP_WALK_1..3 | 0x17C..0x17E | NSP_WALK_1..3 | 0xFF..0x101 |

Indices 144, 145, 148, 149, 152, 153, 156, 157, 160, 161 and 162 were read from the ROM and match the expected routines. The other ids follow the `add_new_action` order in `Kirby.asm` (lines 398 to 410).

Kirby branch: the asm takes the Dedede side when `fp->fkind == REMIX_ID_DEDEDE` (0x40) and the Kirby side otherwise. The port's `ftRemixDededeNSPStatus` does the same.

## Vanilla patches (asm `OS.patch_start` hooks in scope)

These are not ported as procs. They patch vanilla Kirby code, so the C must go into those functions under `#ifdef REGION_REMIX`. All 27 hooks are in `decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c`. Each hook sets the action id for Kirby's status change, and the ids come from the table above.

Hook logic, from each `scope` in `DededeSpecial.asm`:
- If `fp->fkind == REMIX_ID_DEDEDE`, the action is the Dedede id.
- Otherwise, if the current hat byte (asm `lb 0x0980(fp)`) is below 0x20 as unsigned, the action is the vanilla Kirby id (the hook's "original line 2").
- Otherwise, the action is the Kirby id.

Suggested C (per hook, replacing the action argument of the vanilla status change in that function):

```c
#ifdef REGION_REMIX
s32 kirbyAction = (fp->fkind == REMIX_ID_DEDEDE) ? DEDEDE_ID : (((u32) (s8) hat_id < 0x20) ? VANILLA_ID : KIRBY_ID);
#endif
```

`hat_id` is the byte at FTStruct 0x980. The decomp lists 0x97C as `modelpart_status[33]`, and the field name for the hat is not confirmed. Check which `FTStruct` member holds the current hat before using `hat_id`.

| Hook (asm line) | RAM | Containing decomp function (file, line) | Offset | Dedede | Vanilla (kirby normal) | Kirby hat |
|---|---|---|---|---|---|---|
| inhale_begin_ground_to_air_ (862) | 0x80162AD8 | ftKirbySpecialNStartSwitchStatusAir (ftkirbyspecialn.c:590) | +0x1C | NSP_BEGIN_AIR 0xF0 | 0x116 | 0x174 |
| inhale_begin_air_to_ground_ (893) | 0x801629F4 | ftKirbySpecialAirNStartSwitchStatusGround (ftkirbyspecialn.c:562) | +0x1C | NSP_BEGIN_GROUND 0xE7 | 0x10D | 0x16C |
| ground_end_transition_ (1124) | 0x80163378 | ftKirbySpecialNEndSetStatus (ftkirbyspecialn.c:875) | +0x14 | NSP_END_GROUND 0xEF | 0x10F | 0x173 |
| air_end_transition_ (1154) | 0x801635C4 | ftKirbySpecialAirNEndSetStatus (ftkirbyspecialn.c:951) | +0x14 | NSP_END_AIR 0xF7 | 0x118 | 0x17B |
| inhale_end_air_to_ground_ (1184) | 0x80162A8C | ftKirbySpecialAirNEndSwitchStatusGround (ftkirbyspecialn.c:580) | +0x20 | NSP_END_GROUND 0xEF | 0x10F | 0x173 |
| inhale_end_ground_to_air_ (1213) | 0x80162B70 | ftKirbySpecialNEndSwitchStatusAir (ftkirbyspecialn.c:608) | +0x20 | NSP_END_AIR 0xF7 | 0x118 | 0x17B |
| ground_pull_transition_ (1244) | 0x801631EC | ftKirbySpecialNLoopSetStatus (ftkirbyspecialn.c:815) | +0x4C | NSP_PULL_GROUND 0xEA | 0x110 | 0x16E |
| air_pull_transition_ (1278) | 0x80163438 | ftKirbySpecialAirNCatchProcCatch (ftkirbyspecialn.c:899) | +0x08 | NSP_PULL_AIR 0xF2 | 0x119 | 0x176 |
| inhale_pull_ground_to_air_ (1311) | 0x80162C10 | ftKirbySpecialNCatchSwitchStatusAir (ftkirbyspecialn.c:628) | +0x20 | NSP_PULL_AIR 0xF2 | 0x119 | 0x176 |
| inhale_pull_air_to_ground_ (1341) | 0x80162C60 | ftKirbySpecialAirNCatchSwitchStatusGround (ftkirbyspecialn.c:638) | +0x20 | NSP_PULL_GROUND 0xEA | 0x110 | 0x16E |
| ground_swallow_transition_ (1371) | 0x8016320C | ftKirbySpecialNEatSetStatus (ftkirbyspecialn.c:829) | +0x08 | NSP_SWALLOW_GROUND 0xEB | 0x111 | 0x16F |
| air_swallow_transition_ (1404) | 0x80163458 | ftKirbySpecialAirNEatSetStatus (ftkirbyspecialn.c:905) | +0x08 | NSP_SWALLOW_AIR 0xF3 | 0x11A | 0x177 |
| inhale_swallow_air_to_ground_ (1437) | 0x80162CB0 | ftKirbySpecialAirNEatSwitchStatusGround (ftkirbyspecialn.c:648) | +0x20 | NSP_SWALLOW_GROUND 0xEB | 0x111 | 0x16F |
| inhale_swallow_ground_to_air_ (1467) | 0x80162BC0 | ftKirbySpecialNEatSwitchStatusAir (ftkirbyspecialn.c:618) | +0x20 | NSP_SWALLOW_AIR 0xF3 | 0x11A | 0x177 |
| ground_idle_transition_ (1497) | 0x8016328C | ftKirbySpecialNWaitSetStatusFromEat (ftkirbyspecialn.c:845) | +0x18 | NSP_IDLE_GROUND 0xEC | 0x113 | 0x170 |
| inhale_idle_ground_to_air_ (1561) | 0x80162DA0 | ftKirbySpecialNWaitSwitchStatusAir (ftkirbyspecialn.c:678) | +0x20 | NSP_FALL 0xF4 | 0x11C | 0x178 |
| inhale_idle_air_to_ground_ (1592) | 0x80162DF0 | ftKirbySpecialAirNWaitSwitchStatusGround (ftkirbyspecialn.c:688) | +0x20 | NSP_IDLE_GROUND 0xEC | 0x113 | 0x170 |
| air_fall_transition_ (1622) | 0x801634D8 | ftKirbySpecialAirNWaitSetStatusFromEat (ftkirbyspecialn.c:921) | +0x18 | NSP_FALL 0xF4 | 0x11C | 0x178 |
| inhale_spit_ground_to_air_ (2229) | 0x80162D00 | ftKirbySpecialNThrowSwitchStatusAir (ftkirbyspecialn.c:658) | +0x20 | NSP_SPIT_AIR 0xF5 | 0x11B | 0x179 |
| inhale_turn_to_fall_air_ (2259) | 0x80162FF4 | ftKirbySpecialAirNWaitSetStatusFromTurn (ftkirbyspecialn.c:759) | +0x14 | NSP_FALL 0xF4 | 0x11C | 0x178 |
| inhale_spit_air_to_ground_ (2289) | 0x80162D50 | ftKirbySpecialAirNThrowSwitchStatusGround (ftkirbyspecialn.c:668) | +0x20 | NSP_SPIT_GROUND 0xED | 0x112 | 0x171 |
| ground_idle_to_turn_transition_ (2319) | 0x801632DC | ftKirbySpecialNTurnSetStatus (ftkirbyspecialn.c:855) | +0x18 | NSP_TURN_GROUND 0xEE | 0x114 | 0x172 |
| ground_turn_to_idle_transition_ (2349) | 0x80162FBC | ftKirbySpecialNWaitSetStatusFromTurn (ftkirbyspecialn.c:752) | +0x14 | NSP_IDLE_GROUND 0xEC | 0x113 | 0x170 |
| inhale_turn_ground_to_air_ (2379) | 0x80162E40 | ftKirbySpecialNTurnSwitchStatusAir (ftkirbyspecialn.c:698) | +0x20 | NSP_TURN_AIR 0xF6 | 0x11D | 0x17A |
| inhale_turn_air_to_ground_ (2409) | 0x80162E90 | ftKirbySpecialAirNTurnSwitchStatusGround (ftkirbyspecialn.c:708) | +0x20 | NSP_TURN_GROUND 0xEE | 0x114 | 0x172 |

Two details to check when inserting:
- The `inhale_turn_to_fall_air_` hook reads the player struct from `a2`, not `s1`. The other hooks use `s1`, `v0` or `t6`.
- The `ground_turn_to_idle_transition_` hook reads the player struct from `v0`.

Also, `ground_idle_transition_2_` has its `OS.patch_start` commented out, so it has no hook.

## REMIX-TODO markers (shared Reflect subsystem)

1. `absorb_setup_` (asm line 2448) is not called. It sets the reflect bit (`lbu/ori/sb` on FTStruct 0x18C, value 0x04) and stores `absorb_struct` (Remix data at RAM 0x8056DB94, custom reflect table index 2) at 0x850. Affects `ftRemixDededeNSPLoopInitial` and `ftRemixDededeNSPInhaleLoopSetStatus`. Result: Dedede does not absorb or reflect projectiles while inhaling.
2. Projectile spit-back. `spit_shoot_check_` first looks up `last_reflected_object_table[player]` and calls `unsuspend_projectile_` on the stored projectile (asm lines 2141 to 2162). `suspend_projectile_` (asm line 606, not ported, not in the worklist) is the only writer of that table, so this port always spits a Star Rod star. The PK Thunder branch is also dropped, since it only fires from table entries 0x0E or 0x0F, which are never written here.

## Uncertainties

1. `ground_walk_initial_2_` (asm 1764 to 1803): the "already walking" check does `lw at, 0x0024(at)` on the speed-table entry, which reads code after `walk_speed_table`. The check never matches, so the port always sets the walk status. The port passes the raw stick x to `ftCommonWalkGetWalkStatus`, as the asm does (`lb a0, 0x01C2`). `ground_walk_interrupt_` passes the absolute value, as its asm does.
2. `itStarRodWeaponStarMakeWeapon` is passed `is_smash = FALSE`. The asm leaves a2 unset on that path (asm 2186). Check whether a2 matters.
3. Spit position: `spit_shoot_check_` passes the joint world position from `gmCollisionGetFighterPartsWorldPosition(joints[16])`. The asm's joint offset 0x0928 is taken as `joints[16]` by analogy with Conker's `joints[17]` at 0x092C. Verify it.
4. `ftRemixDededeNSPSpitPlayer` writes `physics.vel_air` as `-lr * 240.0F`, with y and z zero. The asm stores the held player's x velocity from `-lr` times `0x43700000` (240.0F). The y and z velocities are zeroed by the asm too (0x4C and 0x50).
5. The begin routines zero `physics.vel_ground.x` and `.y` (N64 0x60 and 0x64). The asm stores to those offsets, and the decomp names them as in `struct_offsets.txt`.
6. The `ground_walk_fall_initial_` status call passes speed 0.0F and flags `FTSTATUS_PRESERVE_FASTFALL` (8). The asm uses `mfc1` of an f0 that is zero on both paths, and flags 8.
7. `FTStruct.ga` is used as the ground/air kinetic state (N64 0x14C). The asm compares it with the raw values 1 and 0, and the port uses `nMPKineticsGround` and `nMPKineticsAir`, which are 0 and 1 in `mpdef.h`.
8. The spit-transition capture immune mask is the asm literal 0x3F, not `FTCATCHKIND_MASK_ALL`, since the bit meaning of that macro was not confirmed.

## Not ported

- `DededeNSP` routines outside the worklist: the `*_transition_` pulls, swallows, idles and ends (asm 1124 to 1650), `inhale_*_initial_` hooks not referenced here, `absorb_initial_` (asm 733), `suspend_projectile_`, `unsuspend_projectile_` (see REMIX-TODO 2), `make_pk_thunder_` (calls a vanilla function that is not referenced anywhere in scope). The vanilla hooks above reach some of them, so check the hook list before assuming any given routine is covered.
- Remix data `absorb_struct` (see REMIX-TODO 1).
- Custom Remix items and GFX: none used by these routines.
