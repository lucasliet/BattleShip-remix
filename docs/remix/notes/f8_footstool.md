# F8 notes: Footstool Jumping (FootStool/FootStool.asm)

Scope file: `.toolchain/pkgs/smashremix-src/src/FootStool.asm` (629 lines).
Port: `decomp/src/ft/ftremix/ftremixfootstool.c` (new), hooks in `ftremixhooks.h` (appended block).

## Toggle

- Title: `Footstool Jumping` (`Toggles.asm` line 2431, `entry_footstool`, `entry_bool`, default OFF in CE, TE, NE, JP).
- Values: 0 = off (vanilla behaviour), 1 = on.
- Symbol: `REMIX_SYM_Toggles__entry_footstool` (present in `port/remix/remix_symbols.h`).
- API: `gmRemixToggle(footstool)`. The asm reads `Toggles.entry_footstool + 0x4` directly, with no
  `single_player_guard`, so the setting also applies in Bonus and 1P Game modes.
- Lead test: `SSB64_REMIX_TOGGLES="Footstool Jumping=1"` (title match is case-insensitive).
- What to see with it on: in a 2P VS match, have fighter A jump from above fighter B (the target must be within its
  scaled width and below A's height band). B goes into Tumble (or LandingHeavy when grounded) and falls at -25.0 vertical
  speed. A's jump becomes a JumpSquat and plays the footstool FGM (0x04B0). With the toggle off, the same press is a
  normal aerial jump.

## Vanilla patches

ROM to RAM check: the function `ftCommonJumpAerialCheckInterruptCommon` starts at RAM 0x8014019C, which is file offset
0xBABDC in the base ROM. The patch ROM offsets of the asm (0xBAC60, 0xBACCC, 0xBAD10, 0xBAD48) keep that delta, and the
Remix ROM (`build/remixasm/remix.z64`) shows the `jal` hooks at those sites.

| Asm patch | RAM | Vanilla function (decomp) | Status |
|---|---|---|---|
| `num_jumps_no_reset` (ROM 0x5A6E4) | 0x800DEEE4 | `mpCommonSetFighterAir` (`mp/mpcommon.c`) | applied: `#ifdef REGION_REMIX`, skips `jumps_used = 1` when `ftRemixFootStoolPreservesJumps` |
| `kirby_jump_aerial_1_check` (ROM 0xBAC60) | 0x80140220 | `ftCommonJumpAerialCheckInterruptCommon` (`ftcommon/ftcommonjumpaerial.c`), `jumps_used == 1` knee-bend branch | applied: `ftRemixFootStoolTryAerialJump` before `ftCommonJumpAerialMultiSetStatus` |
| `kirby_jump_aerial_2_check` (ROM 0xBACCC) | 0x8014028C | same, Kirby case with `jumps_used != 1` | applied, same pattern |
| `puff_jump_aerial_2_check` (ROM 0xBAD10) | 0x801402D0 | same, Purin/Puff case | applied, same pattern |
| `aerial_jump_check` (ROM 0xBAD48) | 0x80140308 | same, generic (non-Kirby) branch | applied: check before `ftCommonJumpAerialSetStatus` |

Notes on the four jump sites:
- The asm hook sits on the call that starts the jump. The input type is already computed by the time the hook runs
  (kneebend or `ftCommonJumpAerialMultiGetJumpInputType`), so the port checks after the input type. This matches the
  asm's `at = valid jump` gate and the `FootStool` comment "if here, then player pressed JUMP while aerial".
- A footstool jump returns TRUE without the normal jump, as the asm `_no_jump` path does (v0 = 1 from the Remix code that
  follows the hook).
- The generic site (0x80140310) is also used by the Marina scope: Remix replaces its `jal ftCommonJumpAerialSetStatus`
  with `j Marina.jump_timing_patch_` (0x8056C468, `Marina/Marina.asm` line 832, RAM 0x80140310). The port keeps the
  vanilla call on that line, so the Marina patch is not applied here. **The lead must reconcile the two patches at
  0x80140310**: the footstool check stays before whatever call the Marina patch makes.
- The air dodge hook (`ftRemixAirDodgeCheckInterrupt`) runs at the top of the same function and is unchanged.

## Ported routines (FootStool.asm to ftremixfootstool.c)

- `foot_stool_check_` to `ftRemixFootStoolTryAerialJump` (search loop, `check_target_`, victim and jumper init).
- `check_target_` to `ftRemixFootStoolIsInRangeX` / `IsInRangeY` / `IsVictimStateValid` / `IsTeamBlocked`.
- `foot_stool_initial_` to `ftRemixFootStoolVictimInit` (Kirby family check uses REMIX_ID_JKIRBY, JPUFF, EPUFF, DEDEDE).
- `foot_stool_jump_initial_` to `ftRemixFootStoolJumperInit` (kneebend fields copied from the vanilla kneebend
  setup, `ftCommonKneeBendSetStatusParam`).
- `footstool_air_collision_` / `footstool_air_to_ground_` (proc_map of the jumper, via `mpCommonProcFighterLanding`).
- `footstooled_main` / `footstooled_main_2` (proc_map and proc_interrupt of the victim).
- `num_jumps_no_reset` (`ftRemixFootStoolPreservesJumps`).

No REMIX-TODO markers: the Size helper (`ftRemixSizeGetMultiplier`) exists, and the SFX is `func_800269C0_275C0`.

## Uncertainties and deviations (lead to decide)

1. Team check, `FootStool.asm` line 116: `lbu t1, 0x000C(a1)` reads the jumper's GObj byte 0x0C (`link_id`), not its
   FTStruct team. Ported literally (`target_fp->team == jumper_gobj->link_id`). This is probably a Remix bug, since the
   intent is the jumper's team. Only affects team battles without team attack.
2. Victim grounded test, `FootStool.asm` lines 412 and 426: `lw at, 0x148(...)` reads the word at 0x148, which
   starts with `jumps_used`, and its comment says "kinetic state" (`ga` is at 0x14C). Ported as `jumps_used != 0`.
   If in-game victims land in the wrong state, switch to `fp->ga` (0x14C).
3. `check_target_` line 248: the `c.le.s f0, f10` test (X_DIFF >= 0) is dropped, since X_DIFF is an absolute value.
4. Ledge bits: `footstooled_main` ignores the victim when `coll_data.mask_stat & 0x3000` is set. The ledge grab call is
   commented out in the asm, so this branch does nothing (ported the same way).
5. `footstool_air_to_ground_` passes `GObj` offset 0x78 (`anim_frame`) as the starting frame, as in the asm.
6. `NDEDEDE` (polygon Dedede) is commented out in the asm (line 391), so it is not in the Kirby family list.
7. Tie-break of the closest target: `x_diff <= best`, so the later fighter in the link list wins ties (asm `beq`
   order is equivalent).

## Checks

- `scripts/remix/syntax-check.sh` on `decomp/src/ft/ftremix/ftremixfootstool.c`, `decomp/src/ft/ftcommon/ftcommonjumpaerial.c`
  and `decomp/src/mp/mpcommon.c`: all `syntax-check OK` (the two `mpcommon.c` warnings at lines 124 and 141 predate this
  change). Build and game runs were not done, per the brief.
