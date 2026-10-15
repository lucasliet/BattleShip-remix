# 0x3D SSONIC: SSonicUSP (Teleport) port notes

Port file: `decomp/src/ft/ftremix/ssonic/ftremixssonicusp.c`
Table: `dFTRemixSSonicUSPProcs` / `dFTRemixSSonicUSPProcsNum` (14 entries, all worklist routines).
Source: `.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm`, `scope SSonicUSP` (lines 2901-3593).
Syntax check: `scripts/remix/syntax-check.sh` prints `syntax-check OK`, no warnings. No build run.

Action ids: USP_Ground_Begin 0x100, USP_Ground_Move 0x101, USP_Ground_End 0x102, USP_Air_Begin 0x103,
USP_Air_Move 0x104, USP_Air_End 0x105. Checked against `build/remixasm/remix.z64`: the SSONIC action
array at 0x804C2020 (entries 36-41, status 0xDC + i) holds the USP mains and collisions at those indices
(begin_main_ at 36 and 39, move_main_ at 37 and 40, ground_end_main_ at 38, air_end_main_ at 41).

## Not wired yet

`ftremixported.c` has no entry for this table. Add the `extern` pair and a `dFTRemixPortedProcTables` row
(the lead owns that file), or the routines fall back to the parent fighter.

## 1. Vanilla patches

### 1a. Teleport cancel hook (SonicSpecial.asm:3595, `teleport_cancel_patch_`)

- RAM hook: `OS.patch_start(0x5D198, 0x800E1998)`, that is `jalr $v0` in `ftMainProcUpdateInterrupt`
  (`decomp/src/ft/ftmain.c:1378`). The instruction at 0x800E1998 is the call of `FTStruct.proc_interrupt`
  (FTStruct+0x9DC), guarded by a `beql` null check at 0x800E1990.
- The Remix stub is `ftRemixSSonicUSPInterruptPatch`, defined non-static in the port file. It takes the
  original interrupt pointer, so the vanilla edit is one line.
- Vanilla C to insert in `decomp/src/ft/ftmain.c` inside `ftMainProcUpdateInterrupt`, replacing the call at
  lines 1667-1670:

  ```c
          if (this_fp->proc_interrupt != NULL)
          {
  #ifdef REGION_REMIX
              ftRemixSSonicUSPInterruptPatch(fighter_gobj, this_fp->proc_interrupt);
  #else
              this_fp->proc_interrupt(fighter_gobj);
  #endif
          }
  ```

- Prototype (declare it in `ftmain.c` under `REGION_REMIX`, or in a header it includes):

  ```c
  void ftRemixSSonicUSPInterruptPatch(GObj *fighter_gobj, void (*proc_interrupt)(GObj *));
  ```

- Behaviour: when Super Sonic (`fkind == REMIX_ID_SSONIC`), `stick_range.y >= 40`, B is tapped
  (`input.pl.button_tap & B_BUTTON`), and a hitbox interacted (see 2c), it calls the USP begin state
  (`air_begin_initial_`) and skips the original interrupt. Otherwise it calls the original interrupt.
- Risk: the hook runs only while the fighter is in a status whose proc_interrupt is non-NULL, the same as Remix.

## 2. Shared Remix subsystems

a. **PokemonAnnouncer.teleport_announcement_** (asm line 3072, called in `move_initial_`). Skipped; marked
   `REMIX-TODO(PokemonAnnouncer)` in the port.

b. **CharEnvColor.moveset_table** (asm lines 3288-3293, 3330-3335, 3326-3335). Remix writes
   `0xFFFFFF00` (full transparency) for the player's port. `ftremixcommand.c` exposes only
   `ftRemixCommandGetEnvColor`, with no setter, so the write is skipped and marked
   `REMIX-TODO(CharEnvColor)`. `is_invisible` is set instead, as in the Mewtwo port. A setter would be
   needed in `ftremixcommand.c` to make this exact.

c. **Character.get_hitbox_collision_flags_** (Character.asm:3322). Used only by the teleport cancel
   (asm line 3621). The port reimplements it as static `ftRemixSSonicUSPHitboxCollided` over
   `fp->attack_colls`, with the same logic as the Sonic NSP helper (`ftremixsonicnsp.c`). The copy is
   duplicated because statics cannot be shared across files.

## 3. Vanilla calls and their decomp names

| RAM | Decomp name | File |
|-----|-------------|------|
| 0x800E6F24 | ftMainSetStatus | decomp/src/ft/ftmain.c:4580 |
| 0x800E0830 | ftMainPlayAnimEventsAll | decomp/src/ft/ftmain.c:1047 |
| 0x800D9480 | ftAnimEndCheckSetStatus | decomp/src/ft/ftanimend.c:4 |
| 0x800D94C4 | ftAnimEndSetWait | decomp/src/ft/ftanimend.c:16 |
| 0x800DDE84 | mpCommonProcFighterOnEdge | decomp/src/mp/mpcommon.c:261 |
| 0x800DDDDC | mpCommonProcFighterOnFloor | decomp/src/mp/mpcommon.c:233 |
| 0x800DE80C | mpCommonProcFighterCliff | decomp/src/mp/mpcommon.c:640 |
| 0x800DDEE8 | mpCommonProcFighterOnCliffEdge | decomp/src/mp/mpcommon.c:279 |
| 0x800DE724 | mpCommonCheckFighterProject | decomp/src/mp/mpcommon.c:604 |
| 0x800DE798 | mpCommonCheckFighterPassCliff | decomp/src/mp/mpcommon.c:622 |
| 0x800DEE54 | mpCommonSetFighterWaitOrFall | decomp/src/mp/mpcommon.c:873 |
| 0x800DEE98 | mpCommonSetFighterGround | decomp/src/mp/mpcommon.c:885 |
| 0x800DEEC8 | mpCommonSetFighterAir | decomp/src/mp/mpcommon.c:897 |
| 0x800D8EB8 | ftPhysicsClampAirVelXMax | decomp/src/ft/ftphysics.c:283 |
| 0x800E8044 | ftParamSetStickLR | decomp/src/ft/ftparam.c:276 |
| 0x80142E3C | ftCommonLandingFallSpecialSetStatus | decomp/src/ft/ftcommon/ftcommonlanding.c:89 |
| 0x80144C24 | ftCommonCliffCatchSetStatus | decomp/src/ft/ftcommon/ftcommoncliffcatchwait.c:39 |
| 0x80143394 | ftCommonSquatRvProcInterrupt | decomp/src/ft/ftcommon/ftcommonsquat.c:225 |
| 0x8013F660 | ftCommonJumpProcInterrupt | decomp/src/ft/ftcommon/ftcommonjump.c:10 |
| 0x8015DD20 | ftSamusSpecialHiProcPass | decomp/src/ft/ftchar/ftsamus/ftsamusspecialhi.c |
| 0x8001863C | atan2, port uses `syUtilsArcTan2` | decomp/src/sys/utils.c:81 |
| 0x80035CD0 / 0x800303F0 | cosf / sinf, port uses `__cosf` / `__sinf` | decomp/src/sys/utils.h |

## 4. Uncertainties and deviations

1. **end_collision_ (asm 3519-3540).** The asm copies 26 instructions from 0xD8798 (Samus' map, its
   `mpCommonCheckFighterProject` and `mpCommonCheckFighterPassCliff` path), then 7 instructions from
   0xD8808 (`MAP_FLAG_CLIFF_MASK` test and `ftCommonCliffCatchSetStatus`). The port uses
   `MAP_FLAG_CLIFF_MASK`; I assumed it equals the asm mask 0x3000. The grounded branch is
   `mpCommonProcFighterOnCliffEdge` (asm 3532).
2. **air_end_main_ (asm 3423).** Calls `mpCommonSetFighterWaitOrFall` (0x800DEE54), not a fall special
   as MewtwoUSP does. The port follows the SSonic asm.
3. **move_initial_ stick magnitude (asm 3092-3113).** The asm uses `cvt.w.s` (round to nearest). The
   port uses `(s32)(sqrtf(v) + 0.5F)`. For integer squares the two differ only on exact halves, which
   sqrt cannot produce, so they match.
4. **move_initial_ order (asm 3088-3190).** Turnaround (|stick_x| >= 11) runs first. The default angle is
   written before the atan2 branch, then `apply_movement_`, then the movement timer (12), visibility,
   intangibility, and the platform ID copy (`ignore_line_id = floor_line_id`) when the start state was grounded.
5. **Up special counter (asm 2942-2948).** The word at N64 0x0ADC is read and written through
   `ftRemixFighterWord`. It is the same word as SonicUSP's up special bool (`passive_vars`), so both
   ports share it, as Remix does. The air begin is skipped when the counter is already 2 or more.
6. **Temp variables.** Temp variable 1/2/3 is `motion_vars.flags.flag0/1/2` (N64 0x17C/0x180/0x184).
   `ground_end_interrupt_` and `air_end_interrupt_` check temp variable 2 (`flag1`, asm 3476, 3499).
7. **Remix transition pointers.** The asm passes transitions (`ground_begin_transition_`, `air_begin_transition_`,
   `ground_move_transition_`, `air_move_transition_`) as N64 addresses in `a1`. The port passes the C
   function pointers directly. These transitions are not in the worklist, so they stay static.
8. **B button bit.** Asm `andi t0, 0x0020` (Joypad.B) is ported as `B_BUTTON` on `input.pl.button_tap`
   (N64 0x01BE), as the Falco port does. This assumes the port's `B_BUTTON` matches the Remix bit.
9. **Vanilla SSonic helper not ported.** `SSonicUSP.apply_movement_` is ported as a static helper
   (`ftRemixSSonicUSPApplyMovement`) with speed 160.0F (asm 3548, `lui 0x4320`).
