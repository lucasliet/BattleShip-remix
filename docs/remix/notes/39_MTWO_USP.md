# 0x39 MTWO: MewtwoUSP (Teleport) notes

Port file: `decomp/src/ft/ftremix/mewtwo/ftremixmewtwousp.c`
Table: `dFTRemixMewtwoUSPProcs` / `dFTRemixMewtwoUSPProcsNum` (12 entries, all the worklist's MewtwoUSP routines).
Source: `.toolchain/pkgs/smashremix-src/src/Mewtwo/MewtwoSpecial.asm`, `scope MewtwoUSP` (lines 1373-1967).
Not yet wired: `ftremixported.c` has no `dFTRemixMewtwoUSPProcs` entry. Add the `extern` pair and a `dFTRemixPortedProcTables` row, or the port falls back to Yoshi.

Action ids: USP_Ground_Begin 0xEA, USP_Ground_Move 0xEB, USP_Ground_End 0xEC, USP_Air_Begin 0xED, USP_Air_Move 0xEE, USP_Air_End 0xEF. Checked against `build/remixasm/remix.z64`: the MTWO action array at 0x804BE6F0 has the USP mains at those indices (begin_main_ at 0xEA and 0xED, move_main_ at 0xEB and 0xEE, ground_end_main_ at 0xEC, air_end_main_ at 0xEF, end_collision_ at 0xEC and 0xEF).

## 1. Vanilla patches

None. Nothing in this scope patches or hooks a vanilla function. `end_collision_` uses `OS.copy_segment(0xD8798, 0x68)` and `OS.copy_segment(0xD8808, 0x1C)`, which copy Samus' up special collision (0x8015DD58) into the Remix ROM. The port reproduces those bytes in C, so no vanilla file needs a change.

Vanilla functions called (for the reviewer's cross-check):

| RAM | Decomp name | File |
|-----|-------------|------|
| 0x800E6F24 | ftMainSetStatus | decomp/src/ft/ftmain.c:4580 |
| 0x800E0830 | ftMainPlayAnimEventsAll | decomp/src/ft/ftmain.c:1047 |
| 0x800D9480 | ftAnimEndCheckSetStatus | decomp/src/ft/ftanimend.c:4 |
| 0x800D94C4 | ftAnimEndSetWait | decomp/src/ft/ftanimend.c:16 |
| 0x800DDDDC | mpCommonProcFighterOnFloor | decomp/src/mp/mpcommon.c:233 |
| 0x800DDE84 | mpCommonProcFighterOnEdge | decomp/src/mp/mpcommon.c:261 |
| 0x800DDEE8 | mpCommonProcFighterOnCliffEdge | decomp/src/mp/mpcommon.c:279 |
| 0x800DE724 | mpCommonCheckFighterProject | decomp/src/mp/mpcommon.c:604 |
| 0x800DE798 | mpCommonCheckFighterPassCliff | decomp/src/mp/mpcommon.c:622 |
| 0x800DE80C | mpCommonProcFighterCliff (no address comment in decomp, see 3) | decomp/src/mp/mpcommon.c:640 |
| 0x800DEE98 | mpCommonSetFighterGround | decomp/src/mp/mpcommon.c:885 |
| 0x800DEEC8 | mpCommonSetFighterAir | decomp/src/mp/mpcommon.c:897 |
| 0x800D8EB8 | ftPhysicsClampAirVelXMax | decomp/src/ft/ftphysics.c:283 |
| 0x800E8044 | ftParamSetStickLR | decomp/src/ft/ftparam.c:276 |
| 0x80142E3C | ftCommonLandingFallSpecialSetStatus | decomp/src/ft/ftcommon/ftcommonlanding.c:89 |
| 0x801438F0 | ftCommonFallSpecialSetStatus | decomp/src/ft/ftcommon/ftcommonfallspecial.c:72 |
| 0x80144C24 | ftCommonCliffCatchSetStatus | decomp/src/ft/ftcommon/ftcommoncliffcatchwait.c:39 |
| 0x8015DD20 | ftSamusSpecialHiProcPass | decomp/src/ft/ftchar/ftsamus/ftsamusspecialhi.c |
| 0x80035CD0 | cosf, port uses `__cosf` | decomp/src/sys/utils.h |
| 0x800303F0 | sinf, port uses `__sinf` | decomp/src/sys/utils.h |
| 0x8001863C | atan2 (see 3), port uses `syUtilsArcTan2` | decomp/src/sys/utils.c:81 |

## 2. Shared Remix subsystems

- `PokemonAnnouncer.teleport_announcement_` (MewtwoSpecial.asm:1534, PokemonAnnouncer.asm:843). Skipped with `REMIX-TODO(PokemonAnnouncer)` in `ftRemixMewtwoUSPMoveSetStatus`. It plays the teleport announcer line, or FGM 0x800269C0 when the announcer is off.
- `CharEnvColor.moveset_table` (MewtwoSpecial.asm:1630, 1754, 1796). Remix writes 0xFFFFFF00 (full transparency) into `moveset_table[port]` on each teleport. Skipped with `REMIX-TODO(CharEnvColor)` in `ftRemixMewtwoUSPHideIntangible`. The port hides the fighter through `is_invisible`, which `ftdisplaymain.c:1112` honours. The env-color alpha is not reproduced.

## 3. Uncertainties and deviations

- **end_collision_ grounded branch (MewtwoSpecial.asm:1958-1961).** The copied Samus code branches when `ga != air` to `_grounded`, which calls 0x800DDEE8 (`mpCommonProcFighterOnCliffEdge`). The ROM bytes at 0x8055AB4C confirm this. The Samus decomp (`ftsamusspecialhi.c:49-66`) instead calls `mpCommonSetFighterFallOnEdgeBreak` (0x800DDF44) on the same branch. The port follows the ROM bytes, so the decomp may be wrong here, or it may describe a different build.
- **Mapping of 0x800DE80C.** It has no `// 0x` comment in the decomp. I identified it as `mpCommonProcFighterCliff` by address placement, between `mpCommonCheckFighterCliff` (0x800DE7D8) and `mpCommonCheckFighterCeilHeavyCliff` (0x800DE87C), and by the asm comment "transition on landing, allow ledge grab". Not verified against bytes.
- **Mapping of 0x8001863C (atan2) and 0x80035CD0 / 0x800303F0 (cos / sin).** Not listed in `vanilla_funcs.txt`. The port uses `syUtilsArcTan2`, `__cosf` and `__sinf`, which the decomp uses for the same purpose. Results may differ in the last float bit.
- **ftPhysicsClampAirVelXMax (0x800D8EB8).** The asm comment says "momentum capture?". The mapping comes from `vanilla_funcs.txt`, and the port follows that name.
- **Teleport timer and angle (status vars 0x0B18 and 0x0B20).** No typed field exists in `FTStatusVars`, so they live in `sFTRemixMewtwoUSPMoveTics[port]` and `sFTRemixMewtwoUSPMoveAngle[port]`. This is equivalent while each port holds one fighter.
- **Hurtbox state (0x05BB).** The asm stores a byte. The port writes `fp->hitstatus = nGMHitStatusIntangible` (3) to the s32 at 0x05B8. Equivalent, because hitstatus is always 0 to 3.
- **Stick magnitude (move_initial_).** `cvt.w.s` rounds to nearest. The port uses `(s32)(sqrtf(x) + 0.5F)`, which gives the same result because `sqrt` of an integer is never a .5 tie.
- **Landing lag constant.** 0x3F9D89D9 decodes to 1.2307692766189575. The port uses `1.2307693F`, which rounds to the same float.
- **Fast-fall clear in air_begin_initial_.** Mask 0x07 clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall, as in the Falco port. The asm comment does not name the fields; the mapping follows `struct_offsets.txt`.
- **Movement at move_initial_.** The asm calls `apply_movement_` there (MewtwoSpecial.asm:1623), so the port calls `ftRemixMewtwoUSPApplyMovement` too.
- **Jump refill.** The asm writes `attr->jumps_max` to `jumps_used` (0x148). The port does the same at `ftRemixMewtwoUSPMoveSetStatus`.
