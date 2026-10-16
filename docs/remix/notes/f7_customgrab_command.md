# F7 notes: CustomGrabAction.asm and command.asm vanilla patches

## command.asm (commands 0xD0-0xDC already ported in ftremixcommand.c)

| asm | RAM | vanilla function | status |
|---|---|---|---|
| command.asm:150 end_hitbox_ | 0x800E852C | ftParamClearAttackCollAll | applied earlier (ftparam.c) |
| command.asm:190 create_hitbox_ | 0x800DF1F0 | ftMainParseMotionEvent (MakeAttackColl) | applied now: ftRemixCommandResetAttack in ftmain.c |
| command.asm:232 texture_form_fix_ | 0x800E962C | ftParamSetTexturePartID | applied earlier (ftparam.c, nFTKindMMario) |
| command.asm:521 apply_direction_ | 0x800E446C | ftMainProcessHitCollisionStatsMain (fighter branch) | applied now: ftRemixCommandApplyDamageDirection in ftmain.c |
| command.asm:611 apply_ground_z_ | 0x800D8C2C | ftPhysicsApplyGroundVelTransN | translation multiplier applied earlier; Size.multiplier_table factor NOT applied (REMIX-TODO(Size)) |
| command.asm:663 apply_ground_x_ | 0x800D8C68 | ftPhysicsApplyGroundVelTransN | same as apply_ground_z_ |
| command.asm:715 apply_air_z_ | 0x800D9294 | ftPhysicsGetAirVelTransN | translation multiplier applied earlier; Size factor NOT applied (REMIX-TODO(Size)) |
| command.asm:767 apply_air_y_ | 0x800D92B0 | ftPhysicsGetAirVelTransN | same as apply_air_z_ |
| command.asm:820 apply_air_x_ | 0x800D9364 | ftPhysicsGetAirVelTransN | same as apply_air_z_ |
| command.asm:920 apply_fgm_ | 0x800E2CA8 | ftMainPlayHitSFX | applied earlier (ftmain.c) |
| command.asm:93 change_action_ _reset_translation_multiplier, _reset_env_color | 0x800E6F2C | ftMainSetStatus | applied earlier |
| command.asm:118-136 change_action_ _reset_training_action_frame_count | 0x800E6F2C | ftMainSetStatus | SKIPPED: training mode subsystem not ported (see ftmain.c comment near ftRemixCommandResetStatus) |
| command.asm:137-147 change_action_ _respawn_with_item | 0x800E6F2C | ftMainSetStatus | SKIPPED: Item.respawn_with_item_ not ported |

Command table: every id 0xD0-0xDC has a C case in ftRemixCommandRun. 0xDD-0xFF are OS.NULL in the asm and advance one word in C.

## CustomGrabAction.asm

| asm | RAM | vanilla function | status |
|---|---|---|---|
| CustomGrabAction.asm:12 capture_action_override_ | 0x8014A90C | ftCommonCapturePulledProcCapture | already ported through port_fighter_custom_capture_action; registration was missing, now added for Wario and Piano in ftremixdeschooks.c |
| CustomGrabAction.asm:41 capture_position_fix_ | 0x8014A95C | ftCommonCapturePulledProcCapture | already ported in the same PORT block (ThrownProcPhysics path); not re-verified against the asm frame-1 call to 0x8014AB64 |
| CustomGrabAction.asm:79 capture_dk_break_fix_ | 0x8014E4D4 | ftCommonCaptureShoulderedProcInterrupt | already ported through port_fighter_custom_capture_dk_interrupt (Piano registration already present) |

No new ftremixcustomgrab.c was needed: the custom grab logic already lives in the port registry path.

## REMIX-TODOs and gaps
- REMIX-TODO(Size): the ground and air translation patches also multiply by Size.multiplier_table (asm lines 632-650, 736-754 and the matching blocks). The C port applies only the command 0xD3 multiplier.
- Not wired: ftRemixCommandGetEnvColor has no caller. CharEnvColor.asm (set_env_color_ readers) is a separate subsystem.
- Not wired: change_action_ training frame reset and Item.respawn_with_item_ (command.asm:118-147).
- Toggles: none in these patches, so no REMIX-TODO(toggles).
