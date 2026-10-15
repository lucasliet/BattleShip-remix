# 44 BANJO DSP (Beak Bomb / Rear Egg, down special)

Port file: `decomp/src/ft/ftremix/banjo/ftremixbanjodsp.c`, table `dFTRemixBanjoDSPProcs` / `dFTRemixBanjoDSPProcsNum`.
Source: `.toolchain/pkgs/smashremix-src/src/Banjo/BanjoSpecial.asm`, scope `BanjoDSP` (lines 1944 to 2380).

## Coverage

- Worklist routines ported and in the table (7): `aerial_main_`, `air_initial_`, `air_move_`, `collision_`,
  `ground_initial_`, `grounded_collision_`, `physics_`.
- Helpers ported as `static` and kept out of the table: `loop_initial_` (passed to `ftAnimEndCheckSetStatus`),
  `begin_landing_` (passed to `mpCommonProcFighterCliff`), `status_check_`, `action_change_`.
- Not ported: `ground_move_` (asm line 2026). No routine in Banjo.asm or BanjoSpecial.asm references it, and it is
  not in the worklist.
- Weapons: none. No DSP routine creates a weapon, so no `WPDesc` is built.
- Items, GFX and other shared Remix subsystems: none used. No `REMIX-TODO` markers.

## Vanilla patches

None. The DSP scope has no `OS.patch_start` blocks or `j`/`jal` hooks into vanilla code. It only calls vanilla
functions and passes addresses that `Character.edit_action` (Banjo.asm, outside this scope) binds.

Vanilla functions called, with the decomp name used in the port:

| RAM | Decomp function (file) | Used by |
|-----|------------------------|---------|
| 0x800E6F24 | ftMainSetStatus (ftmain.c) | ground/air/loop initial, action_change_, begin_landing_ |
| 0x800E0830 | ftMainPlayAnimEventsAll (ftmain.c) | ground/air/loop initial |
| 0x800D9480 | ftAnimEndCheckSetStatus (ftanimend.c) | aerial_main_ |
| 0x800D90E0 | ftPhysicsApplyAirVelDrift (ftphysics.c) | physics_ |
| 0x800D91EC | ftPhysicsApplyAirVelFriction (ftphysics.c) | physics_ |
| 0x800DDF44 | mpCommonSetFighterFallOnEdgeBreak (mpcommon.c) | collision_ (grounded) |
| 0x800DE80C | mpCommonProcFighterCliff (mpcommon.c), inferred, see below | collision_ (aerial, dive) |
| 0x800DE978 | mpCommonProcFighterCliffWaitOrLanding (mpcommon.c) | collision_ (aerial, other) |
| 0x800DEEC8 | mpCommonSetFighterAir (mpcommon.c) | action_change_ |
| 0x800DEE98 | mpCommonSetFighterGround (mpcommon.c) | begin_landing_ |
| 0x8015FCE8 | ftCaptainSpecialLwDecideMapCollide (ftcaptainspeciallw.c) | grounded_collision_ |
| 0x8015FEB4 | ftCaptainSpecialLwBoundCheckGoto (ftcaptainspeciallw.c) | grounded_collision_ |

## Uncertainties

1. **0x800DE80C is inferred.** `vanilla_funcs.txt` and the decomp have no label for it. mpcommon.c has
   `mpCommonCheckFighterCliff` at 0x800DE7D8 and `mpCommonCheckFighterCeilHeavyCliff` at 0x800DE87C. The gap between
   them is the body of `mpCommonProcFighterCliff`, which takes `proc_map`, allows the cliff catch, and matches the
   asm comment "transition on landing, allow ledge grab". Confidence is high but not verified byte-for-byte.
   Asm lines 2216 and 2223.
2. **Proc addresses in action_change_ (asm lines 2370 to 2372).** The `lui 0x800f` plus `addiu 0x9c8c` pair sign-extends
   the low half. The real values are 0x800E9C8C and 0x800E9CC4.
   - 0x800E9CC4 is `ftParamProcResumeEffect`, which matches the decomp.
   - 0x800E9C8C is treated as `ftParamProcPauseEffect`. The decomp labels `ftParamProcPauseEffect` at 0x800E9C78,
     which is also the label for `ftParamPauseEffect`. That is probably a stale comment, since the resume pair sits
     0x14 bytes apart (0x9CB0 to 0x9CC4). Check the comment in `decomp/src/ft/ftparam.c` lines 1585 to 1600.
   The port uses `ftParamProcPauseEffect` / `ftParamProcResumeEffect`, the same pair the Falcon kick sets in
   `ftCaptainSpecialLwAirSetStatus` (ftcaptainspeciallw.c line 210).
3. **begin_landing_** copies vanilla 0x801600EC. The copied bytes were disassembled with
   `tools/remix_disasm_rom.py 0xDAB2C 0x38 --vram 0x801600EC`. They are `mpCommonSetFighterGround(fp)`, then
   `ftMainSetStatus(gobj, 0xE8, 0.0F, 1.0F, 0)`, which matches the port. The port's landing status id 0xE8 is the
   same value as `Action.DSPLand` (Banjo.asm).
4. **action_change_** is only loosely a copy of Falcon's 0x80160060. The asm saves and restores the root joint's
   `translate.vec.f.y` (joints[0] at N64 0x08E8, DObj offset 0x38), not the `rotate.z` that Falcon's version saves.
   The port follows the Banjo asm. The DObj 0x38 offset comes from the asm and was not cross-checked against the
   struct table.
5. **status_check_** returns `v0` (0 or 1), but `grounded_collision_` ignores it. The port returns void.
6. **physics_ `_check_fall` block** (asm lines 2156 to 2162) is a no-op because both branches fall through. It is not
   ported as code.
7. **Y speed in physics_** (asm lines 2164 to 2175). Y = -110.0F (`Y_SPEED`) when the action is DSPALoop or temp
   variable 3 is MOVE, otherwise 12.0F (`AERIAL_INITIAL_Y_SPEED`). This is derived from the branch order, where the
   `lui` in the `bne` delay slot is overwritten on the fall-through path.
8. **air_initial_ fast-fall clear** (asm lines 1993 to 1997). Mask 0x07 clears bits 0 to 4 of byte 0x018D
   (`is_absorb`, `absorb_lr`, `is_goto_attack100`, `is_fastfall`), using the bit layout in `struct_offsets.txt`.
   The other bits are untouched.
9. **loop_initial_ status flags** are `FTSTATUS_PRESERVE_HIT` (asm value 1), not NONE.
10. **Temp variable 2 value for status_check_.** The asm tests for exactly 0x1
    (`BANJO_DSP_TEMP2_ACTION_CHANGE`). The moveset meaning of that value is not documented in the Remix source.
