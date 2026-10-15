# 4A ROY DSP: notes for the port in decomp/src/ft/ftremix/roy/ftremixroydsp.c

Source: .toolchain/pkgs/smashremix-src/src/Roy/RoySpecial.asm (scope RoyDSP, lines 7-425),
with Roy.asm lines 120-147 (Action ids), 464-481 (actions) and 495-500 (special table patches).

Ported (9 of 9 worklist routines, all in `dFTRemixRoyDSPProcs`, `dFTRemixRoyDSPProcsNum`):
ground_begin_initial_, air_begin_initial_, ground_begin_main_, air_begin_main_,
ground_wait_main_, air_wait_main_, end_main_, ground_collision_, air_collision_.

Status ids 0xEF-0xF6 (DSP_Ground_Begin .. DSP_Air_Strong_End) follow Roy.asm's enum and the
add_new_action order. The enum's names (DSP_G_Begin ...) differ from the names used in the
asm (DSP_Ground_Begin ...), so the numbers were checked by order, not by a built ROM.

Non-worklist helpers in RoySpecial.asm, ported as static functions and kept out of the table:
- ground_wait_initial_ / air_wait_initial_ (lines 61, 79): static `ftRemixRoyDSPGroundWaitInitial`
  and `ftRemixRoyDSPAirWaitInitial`. They are passed as proc to ftAnimEndCheckSetStatus.
- ground_end_initial_ / air_end_initial_ (lines 97, 114): not separate functions. Callers use
  `ftRemixRoyDSPStartStatus(gobj, status)` with the status id. The asm sets a1 in the caller.
- ground_to_air_ / air_to_ground_ (lines 381, 406): static `ftRemixRoyDSPGroundToAir` and
  `ftRemixRoyDSPAirToGround`, passed to mpCommonProcFighterOnEdge / Landing.

## 1. Vanilla patches and hooks

- **Damage hook (ftParamUpdateDamage, ft/ftparam.c:1727).** Already present:
  `#ifdef REGION_REMIX damage = ftRemixMarthDSPCounterDamage(fp, damage); #endif`.
  Roy's end_main_ calls `ftParamUpdateDamage(fp, 10)` (asm line 280, 0x800EA248). The Marth
  hook returns `damage` unchanged unless `fp->fkind == REMIX_ID_MARTH`, so Roy is not affected.
  **Nothing to add for Roy.** RoySpecial.asm has no counter-hit logic: no status checks, no
  flag set, no zero damage. This is a charge-up move. If Roy's move is meant to be a counter,
  its counter logic must come from another source (for example the moveset). Confirm this
  before writing a Roy hook. Do not copy the Marth hook.
- **Action table patches (Roy.asm 495-500).** `dw RoyDSP.air_begin_initial_` and
  `dw RoyDSP.ground_begin_initial_` patch the special tables for Roy's id. These go through the
  generated bind (ftremixbind.c), so no C is needed in a vanilla file.
- **Physics and collision addresses (Roy.asm 474-481).** The `0x800D8BB4`, `0x800D90E0` and
  `0x800D91EC` values are vanilla physics ASM addresses in the action records. They map through
  the generated vanilla table. No C is needed.
- **Vanilla calls used by these routines** (for the reviewer, from vanilla_funcs.txt):
  ftMainSetStatus (0x800E6F24), ftMainPlayAnimEventsAll (0x800E0830), mpCommonSetFighterAir
  (0x800DEEC8), mpCommonSetFighterGround (0x800DEE98), ftPhysicsClampAirVelXMax (0x800D8EB8),
  ftAnimEndCheckSetStatus (0x800D9480), mpCommonSetFighterWaitOrFall (0x800DEE54),
  ftParamUpdateDamage (0x800EA248), mpCommonProcFighterOnEdge (0x800DDE84),
  mpCommonProcFighterLanding (0x800DE6E4).

## 2. Shared Remix subsystems

None. The file calls no PokemonAnnouncer, GFX, item or Size routine, so there is no
REMIX-TODO marker in the port.

## 3. Uncertainties (asm line numbers in RoySpecial.asm)

1. **Release path of begin_main_ (lines 145 and 177).** `ground_end_initial_` and
   `air_end_initial_` do not set a1 (the action id). The asm relies on the caller's a1, but
   begin_main_ does not set it before the jal. The asm comments say DSP_Ground_End and DSP_Air_End.
   The port uses those ids. If the real game passes a different id, this is the first place to check.
2. **ground_to_air_ / air_to_ground_ offset (lines 391 and 416).** The comments say id +/- 3, but
   the code uses +/- 4. The port uses +/- 4, which matches the Roy ids (0xEF to 0xF3, 0xF1 to 0xF5).
3. **Hitbox "just active" check (line 294 and the next three checks).** The asm tests
   `attack_state == 1`, and its comment says "just became active". The port tests the same
   value on every frame it holds, as the asm does. The t6 loads in the branch delay slots
   were traced: each hitbox is tested against its own state.
4. **Shield damage halving (lines 290, 303, 313, 323, 333).** The asm halves `t8` after each active
   hitbox, so the k-th active hitbox (0-based) gets `(8 + charge) >> k`. The port does the same
   through `shield_damage >>= 1` inside the loop body.
5. **Hitbox stride.** The asm reads hitboxes at 0x294, 0x358, 0x41C and 0x4E0, a stride of
   0xC4. The port uses `fp->attack_colls[i]`, so the stride is handled by the port struct.
   The damage and shield offsets (+0xC and +0x38) match FTAttackColl fields `damage` and
   `shield_damage`.
6. **Temp variable 1 clear (lines 201 and 236).** The `sw r0, 0x017C` sits in the delay slot of
   `beqz`, so it runs on both paths. The port clears flag0 in ftRemixRoyDSPConsumeChargePulse on
   both paths. This is equivalent.
7. **Charge word.** 0x0B18 is the charge level here. In Marth's port the same word is the counter
   hit flag. No conflict, because each fighter uses its own routines.
8. **End idle check (lines 340-344).** `anim_frame <= 0.0F` triggers mpCommonSetFighterWaitOrFall.
   The port matches this, the same as Marth's ftRemixMarthAnimEndToIdle.
