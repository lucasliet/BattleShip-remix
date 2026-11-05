# Remix GFX and GFXRoutine

Smash Remix extends the particle system (`GFX.asm`) and the colour animation system (`GFXRoutine.asm`). Both read their data
from the Remix RAM image, so no new asset pipeline was needed. Code: `decomp/src/lb/lbremixgfx.c` (particles),
`decomp/src/gm/gmremixcolscripts.c` (colour animations), `decomp/src/ft/ftparam.c` (`ftParamMakeEffect`,
`ftParamCheckSetColAnimID`). Test: `SSB64_REMIX_GFX_TEST=<decimal id>` and `SSB64_REMIX_COLANIM_TEST=<decimal id>` spawn or
apply the effect on the first fighter every 45 frames (`.claude/skills/remix-smoke-tests/SKILL.md`).

## GFX: new effect ids 0x5C..0x7C

- `lbremixgfx.c`: when `lbParticleSetupBankID` finishes the common effect bank (119 scripts, 47 texture blocks), 33 scripts and 9
  texture blocks of the image are appended to the bank arrays (`GFX.extended_gfx_instructions_map`,
  `GFX.extended_gfx_texture_block_map`). The image keeps its words in host order, so script bytecode and pixel data are copied
  back to big endian byte order (`lbRemixGfxCopyBytes`); script and texture headers are swapped like
  `particle_bank_bridge.cpp` does for vanilla banks.
- `ftParamMakeEffect`: an id >= 0x5C runs the handler of its vanilla base id (table `sLBRemixGfxBaseIDs`, from the `add_gfx`
  base ids of the asm) with the next script substituted (`lbRemixGfxTakeScriptOverride`, consumed by
  `lbParticleMakeScriptID` / `lbParticleMakeCommon`, the two hooks of the asm). `lbRemixGfxSetNextEffect(id)` is the C form of
  `GFX.current_gfx_id = id` for fighter code.
- Deviations: id 0x64 (ground effect clone) has base id 0 in the table, so `ftParamMakeEffect` returns NULL for it; the asm
  clones a Falcon-punch-style ground effect with a palette offset. Id 0x72 (Orange Blast) borrows the Orange Spark handler
  because the asm base id 0x62 has no vanilla handler.

## GFXRoutine: colour animation ids 0x56..0x87

- `gmremixcolscripts.c`: `dGMColScriptsDescs` grows to 0x56 + 64 entries under `REGION_REMIX`;
  `gmRemixColScriptsEnsureLoaded` (called by `ftParamCheckSetColAnimID` for ids >= 0x56) copies the 50 routines of
  `GFXRoutine.gfx_routine_table_extended` as one block of words and turns the address operands of goto, subroutine and
  parallel commands into registered pointers (the same trick as `gmColScriptsLinkRelocTargets`). `dGMColScriptsDescsNum` is
  the live bound.
- Per-port override: `gmRemixColScriptsSetOverride(port, id)`; `ftParamResetStatUpdateColAnim` reapplies it, and the overrides
  are cleared when the effect actors are created (`efdisplay.c`, `gmRemixColScriptsClearOverrides`). Used by the poison
  (`DOOM_ACID`) and King of the Hill glow ports.
- Screen flashes of the Flashbang and the Deku Nut go through `itremixscreenflash.c` into the IF screen flash colour
  animation, guarded by the `flash_guard` setting (`ITREMIX_FLASH_GUARD_ON`).

Open gaps: `docs/remix/patch_audit.md`, group g5 (ground effect clone 0x64).
