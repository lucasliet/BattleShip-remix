# F7 hazard group Rttf (Remix 1P Race to the Finish, stage 0xBD): notes

Status: PORTED for remix_rttf_setup_ and the rttf_bumpers_ vanilla patch. rttf_bomb_2_ is skipped (unreferenced).

## Files

- `decomp/src/gr/grremix/grremixhazards_rttf.c`: `grRemixHazardRttf` returns TRUE after `grBonus3MakeGround()` when
  `function == REMIX_SYM_Hazards__remix_rttf_setup_` (0x8048D168).
- `decomp/src/gr/grremix/grremixrttf.c/.h` (new): `grRemixBonus3GetBumpersAnimJointOffset()`.
- `decomp/src/gr/grbonus/grbonus3.c` (vanilla, `#ifdef REGION_REMIX` only): the two `anim_joint` lines use
  `GRBONUS3_BUMPERS_ANIM_JOINT_OFFSET`, which expands to the vanilla `llGRBonus3MapBumpersAnimJoint` outside REGION_REMIX.

## remix_rttf_setup_ (Hazards.asm 2768-2801)

Calls, in order, 0x8010B4D0 (grBonus3InitHeaders), 0x8010B508 (grBonus3MakeBumpers), 0x8010B660 (grBonus3TaruBombMakeActor),
0x8010B784 (grBonus3FinishMakeActor). That is exactly the body of vanilla grBonus3MakeGround (0x8010B7C8), so the port
calls it. The `//jal remix_rttf_bombs_` line is commented out, so the taru bomb is the vanilla one.

## rttf_bumpers_ patch (Hazards.asm 2956-2978, jal at 2958, OS.patch_start(0x86D50, 0x8010B550))

- The patch replaces the `jal` at 0x8010B550 and re-emits the original line 1 (`lui t7, 0`) in its delay slot.
  The original line 2 at 0x8010B554 (`addiu t7, t7, 0x0110`) is replaced by the routine. Return is to 0x8010B558, so there
  is no double add.
- `t7` is the offset of the bumper anim joints (`llGRBonus3MapBumpersAnimJoint` = 0x110). The DObjDesc offset is 0 and is
  not patched.
- The routine computes `t7 = 0x1E8` when `gkind == nGRRemixStage_REMIX_RTTF` (`s2 == s3`, fall-through path), otherwise
  `0x110`. The port helper does the same.
- Uncertainty: the asm does not show whether `t7` feeds anything besides the anim joint address. The port changes only the
  anim joint line, which matches the t7 use in the vanilla code.

## rttf_bomb_2_ (Hazards.asm 2916-2953): SKIPPED

Its only references are commented out: `//jal remix_rttf_bombs_` (2779) and `// li a1, rttf_bomb_2_` (2825). The Remix
build never registers it, so the routine is dead code. A dead C routine would be an unused static, so it is not ported.
If `remix_rttf_bombs_` is re-enabled, port rttf_bomb_2_ at that time. It uses item kind 0x1A (RTTF_BOMB),
`itManagerMakeItemSetupCommon` (0x8016EA78) and the gGRCommonStruct timer at 0x3C.

## _load_rttf_files (Hazards.asm:952): NOT PATCHED (not a vanilla hook)

The site is inside Remix `add_item_` (Hazards.asm:938), a Remix routine, not a vanilla function. It loads file 0x127 for the
RTTF_BOMB item (hazard id 0x1A). Its callers in the active asm are Bowser bomb (`BOWSER_BOMB`) and Dream Land only. No active
call passes RTTF_BOMB, so the load is unreachable here. The RTTF bomb item is part of the skipped rttf_bomb_2_ and
remix_rttf_bombs_ path.

## Stage and runtime

- Stage id: 0xBD (nGRRemixStage_REMIX_RTTF). Function table entry: Hazards.remix_rttf_setup_ (Stages.asm:3377).
- Test: `SSB64_VS_STAGE=189` (0xBD).
- Check: the bumpers should appear and be placed from the RTTF map (offset 0x1E8, not 0x110). If they are misplaced, the
  offset is wrong.

## REMIX-TODO

None in the new code.
