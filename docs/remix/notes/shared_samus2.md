# samusshared.asm (scope SamusShared), second group

Source: `.toolchain/pkgs/smashremix-src/src/samusshared.asm`.
Already done before this pass (not touched): `get_bombinstructions_struct_`,
`get_chargeinstructions_struct_` (ftremixsamusshared.c), and all DSamus NSP/DSP
patches (dsamus/ftremixdsamushooks.c).

Helpers: `decomp/src/ft/ftremix/ftremixsamusshared2.c`, declared in
`decomp/src/ft/ftremix/ftremixhooks.h` (block "Samus clones, ftremixsamusshared2.c").

## Patches

| # | asm scope | RAM | Vanilla function (vanilla_funcs.txt) | Status |
|---|-----------|-----|--------------------------------------|--------|
| 1 | `fully_charged_check_` | 0x800EAC18 | ftParamMakeEffect (ftparam.c) | applied |
| 2 | `kirby_power_check_flash_` | 0x800E99C4 | ftParamResetStatUpdateColAnim (ftparam.c) | applied, colour anim is REMIX-TODO(GFXRoutine) |
| 3 | `kirby_power_change_` | 0x80161EC4 | ftKirbySpecialNInitPassiveVars (ftkirbyspecialn.c) | applied |
| 4a | `throw_ball_graphic` | 0x80149C14 | ftCommonCatchSetStatus (ftcommoncatch1.c) | applied |
| 4b | `throw_ball_graphic_2` | 0x8014A2DC | ftCommonThrowSetStatus (ftcommonthrow.c) | applied |
| 5a | `get_entry_anim_gfx_struct` | 0x8010347C | efManagerSamusEntryPointMakeEffect (efmanager.c) | REMIX-TODO(GFX), signature lacks the fighter |
| 5b | `get_grapple_anim_gfx_struct` | 0x80101E88 | efManagerSamusGrappleBeamGlowMakeEffect (efmanager.c) | REMIX-TODO(GFX), see below |

No `cpu_*`, `recovery_logic` or `ledge_check` patches in this file (no REMIX-TODO(AI)).

## Vanilla edits (all under `#ifdef REGION_REMIX`, vanilla branch kept)

1. `ftparam.c`, ftParamMakeEffect, `case nEFKindChargeSparkle`:
   `switch (ftRemixSamusSharedChargeSparkleFkind(fp->fkind))`, else `switch (fp->fkind)`.
   Clones and Mewtwo map to nFTKindSamus.
2. `ftparam.c`, ftParamResetStatUpdateColAnim, `case nFTKindKirby`, before the
   donkey copy check: `ftRemixSamusSharedKirbyCopyChargeFlash(fighter_gobj);`.
   Clones: copysamus_charge_level == FTKIRBY_COPYSAMUS_CHARGE_MAX (7) gives
   nGMColAnimFighterCommonSpecialNCharge. Mewtwo (0x0AE0 == 7), Sheik and Peppy
   (0x0AE0 == 6), Dedede (0x0AE4 == 2) call `ftParamCheckSetFighterColAnimID`
   with GFXRoutine ids 0x64 (KIRBY_MTWO_CHARGE), 0x6D (SHEIK_CHARGE), 0x74 (DEDEDE_CHARGE).
3. `ftkirbyspecialn.c`, ftKirbySpecialNInitPassiveVars: `switch
   (ftRemixSamusSharedKirbyCopyPowerFkind(copy_id))`. Clones, Mewtwo, Marth, Sheik,
   Peppy map to nFTKindSamus. Include `ft/ftremix/ftremixhooks.h` added.
4. `ftcommoncatch1.c`, ftCommonCatchSetStatus, and `ftcommonthrow.c`,
   ftCommonThrowSetStatus: the Samus/NSamus check that creates the grapple glow
   becomes `ftRemixSamusSharedHasGrappleBeam(fkind)` (Samus, NSamus, DSamus, JSamus, ESamus).
5. `efmanager.c`: one REMIX-TODO(GFX) comment above efManagerSamusEntryPointMakeEffect.
   No behaviour change in the grapple function (see the TODO below).

## REMIX-TODO

- **REMIX-TODO(GFXRoutine), kirby_power_check_flash_ colour anim** (ftremixsamusshared2.c,
  `ftRemixSamusSharedKirbyCopyChargeFlash`). The call to `ftParamCheckSetFighterColAnimID`
  is made, but `ftParamCheckSetColAnimID` rejects ids >= dGMColScriptsDescsNum (ftparam.c
  about line 1386), so for ids 0x64, 0x6D and 0x74 it does nothing until the Remix GFX
  routine table is ported. Caveat: `docs/remix/notes/40_DEDEDE_DSP.md` says
  0x800E9814 (the call the Samus branch makes, `jal func_800E9814` at vanilla 0x800E99EC, reached from
  the patch's `j 0x800E99E8`) is not the Remix GFX begin and does not call it. The asm
  does call it with the GFX id, so the port follows the asm. Reconcile with the DSP
  note when the GFX table is ported.
- **REMIX-TODO(GFX), get_grapple_anim_gfx_struct (Dark Samus grapple glow)**, not applied.
  The Remix struct (asm lines 372-383) is `0x060F0000`, file 6, `0x4F00002E`, proc
  `0x8000DF34`, render `0x80014768`, and DObj/MObj/Anim/MatAnim offsets
  `0x380/0x210/0x410/0x480 + 0x2970`. Vanilla `dEFManagerSamusGrappleBeamEffectDesc`
  (efmanager.c ~739) has the same flags, dl link and transforms, but the proc is
  `gcPlayAnimAll` and the offsets are `llSamusSpecial2GrappleBeam*`. The proc address
  0x8000DF34 is not in vanilla_funcs.txt or the decomp, so the file-only swap would pair
  Samus's offsets with Dark Samus file 6 and was not applied. Port the full descriptor
  once 0x8000DF34 is identified.
- **REMIX-TODO(GFX), get_entry_anim_gfx_struct (Dark Samus file 6 and Crash file 8 entry glow)**,
  not applied. `efManagerSamusEntryPointMakeEffect(Vec3f *pos)` gets no fighter, so it
  cannot choose the owner's `p_file_special1` (DSamus) or `p_file_special3` (Crash). Callers
  `ftport.c:99` and `ftcommonentry.c:235` have `fp`. Changing the signature to take
  `FTStruct *` is outside this pass. Once it does, copy `dEFManagerSamusEntryPointEffectDesc`
  (flags 0x4, dl 10, same transforms, `efManagerHaveStructProcUpdate`, render 0x80014768)
  and swap only `file_head`; the Remix offsets are in asm lines 360-370 (DSamus) and 418-428 (Crash).

## Uncertainties (asm line numbers in samusshared.asm)

- L104-120 (fully_charged_check_): the vanilla `beq` compares v0 with fkind. The port
  assumes the C `switch (fp->fkind)` is the same test, so the mapping sits in the switch.
- L127-204 (kirby_power_check_flash_): Dedede reads `0x0AE4` of the Kirby struct, which
  the Kirby header names `copysamus_charge_recoil`. The port reads that field, and the
  name is only a label for the same word. Peppy uses the Sheik GFX id (L157-165), not
  its own, as the asm does.
- L208-238 (kirby_power_change_): the port zeroes `copysamus_charge_level` and
  `copysamus_charge_recoil` for Mewtwo, Marth, Sheik and Peppy, as the asm routes them
  through the Samus branch (L224-229). No separate branch was found for Marth.
- L240-288 (throw_ball_graphic and _2): the asm's `at` values (L248-257, L274-281) are
  flow-only. The port keeps the vanilla glow call and only widens the fighter test. It
  does not reproduce the asm's `sb` of the andi result at vanilla 0x192 (the vanilla flag
  write is kept as-is).

## Verification

`scripts/remix/syntax-check.sh` prints `syntax-check OK` for: ftremixsamusshared2.c,
ftparam.c (two warnings at lines 1145 and 1759, not from this change), ftkirbyspecialn.c,
ftcommoncatch1.c, ftcommonthrow.c and efmanager.c. The reference flags define PORT and
REGION_REMIX. No build or game run.
