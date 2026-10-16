# Remix GFX and GFXRoutine (ROADMAP T2.1, 2026-10-16)

Smash Remix extends the particle system (`GFX.asm`) and the colour animation system (`GFXRoutine.asm`). Both read their
data from the Remix RAM image, so no new pipeline was needed.

## GFX: new effect ids 0x5C..0x7C

- `decomp/src/lb/lbremixgfx.c`: when `lbParticleSetupBankID` finishes the common effect bank (119 scripts, 47 texture
  blocks), 33 scripts and 9 texture blocks of the image are appended to the bank arrays (`GFX.extended_gfx_instructions_map`,
  `GFX.extended_gfx_texture_block_map`). The image keeps its words in host order, so script bytecode and pixel data are copied
  back to big endian byte order (`lbRemixGfxCopyBytes`); script and texture headers are swapped like
  `particle_bank_bridge.cpp` does for vanilla banks.
- `ftParamMakeEffect`: an id >= 0x5C runs the handler of its vanilla base id (table `sLBRemixGfxBaseIDs`, from the
  `add_gfx` base ids of the asm) with the next script substituted (`lbRemixGfxTakeScriptOverride`, consumed by
  `lbParticleMakeScriptID` / `lbParticleMakeCommon`, the two hooks of the asm). `lbRemixGfxSetNextEffect(id)` is the C form of
  `GFX.current_gfx_id = id` for fighter code.
- Deviations: 0x64 (ground effect clone) has no handler yet; 0x72 (Orange Blast) borrows the Orange Spark handler because the
  asm base id 0x62 has no vanilla handler.
- Harness: `SSB64_REMIX_GFX_TEST=<decimal id>` spawns that effect on the first fighter every 45 frames.
  All 32 working ids were seen on screen.

## GFXRoutine: colour animation ids 0x56..0x87

- `decomp/src/gm/gmremixcolscripts.c`: `dGMColScriptsDescs` grows to 0x56 + 64 entries under `REGION_REMIX`;
  `gmRemixColScriptsEnsureLoaded` (called by `ftParamCheckSetColAnimID` for ids >= 0x56) copies the 50 routines of
  `GFXRoutine.gfx_routine_table_extended` as one block of words and turns the address operands of goto, subroutine and parallel
  commands into registered pointers (the same trick as `gmColScriptsLinkRelocTargets`).
- `port_override`: `gmRemixColScriptsSetOverride(port, id)`; `ftParamResetStatUpdateColAnim` reapplies it, the overrides are
  cleared when the effect actors are created (`efdisplay.c`).
- Harness: `SSB64_REMIX_COLANIM_TEST=<decimal id>` applies that routine to the first fighter every 45 frames.

## Sites closed with workers

Mewtwo shadow ball and dark cross, Goemon / Dedede charge, Sheik charge start, Pwing and Stopwatch overrides, Basketball
fire and smoke, Franklin Badge override, Flashbang and Deku Nut screen flash (`itremixscreenflash.c`, written to the IF
screen flash colour animation; the guard `ITREMIX_FLASH_GUARD_ON` is FALSE until the settings menu, M4), Samus-shared Kirby
copy flash.

## Still open

Lightning camera colour (needs a camera draw colour field), Crash spin GFX (an attached effect with DObj plus Size),
Sheik charge level write, Franklin `reflect_gfx_` (Reflect T2.3), Dr. Mario/Luigi hit GFX swap and shadow damage GFX
(`Damage.asm`, T2.7), DOOM_ACID while poisoned (Poison, T2.7), ground effect clone 0x64.
