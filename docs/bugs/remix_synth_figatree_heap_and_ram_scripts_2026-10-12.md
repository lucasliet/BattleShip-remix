# Remix synth fighters: zero-size figatree heap and raw RAM motion scripts

**Status:** FIXED (decomp PORT blocks + port + yamls), 2026-10-12.

## Symptom

The first match with a Smash Remix synth fighter (attract demo forced to Falco 29 vs Wario 33 with
`SSB64_START_SCENE=61 SSB64_DEMO_FKINDS=29,33`) crashed before the first frame:

1. `SIGSEGV` in `lbCommonAddFighterPartsFigatree` from `ftMainSetStatus`. Under lldb the fighter's
   TopN `DObj` was filled with s16 triples (`0xfebb000b0001ffa9`, ...): animation data written over
   the skeleton.
2. After fix 1, `SIGSEGV` at `0x80545b28` in `ftMainUpdateMotionEventsAll` for Wario.
3. After fix 2, `SIGSEGV` in `portFixupSprite` from `scAutoDemoInitSObjs`.

## Root causes

1. `ftManagerSetupFileSize` and `ftManagerAllocFighter` compute `file_main_size` and
   `file_anim_size` (the per-fighter figatree heap) only for the 27 vanilla rows of
   `gSCManagerFighterFileSizes`. Synth rows kept the ROM value 0, so
   `ftManagerAllocFigatreeHeapKind` returned a 0-byte heap and `lbRelocGetForceExternHeapFile`
   wrote the whole anim file over whatever followed it.
2. Remix motion rows for its new fighters store `command_offset` as a raw RAM address into its
   code image (ROM `0x02C00000` is DMA'd to RAM `0x80400000` by `Boot.asm`). The port treated it as
   a host pointer (`> 0x100000` branch in `ftMainSetStatus`). Goto/subroutine operands inside those
   scripts are raw RAM addresses too.
3. `dSCAutoDemoFighterNameSpriteOffsets` has 12 entries and was indexed by a synth fkind.

## Fixes

1. `decomp/src/ft/ftmanager.c`: PORT-only `sFTManagerPortFileSizes` (indexed by fkind, sized by the
   new `port_fighter_kind_limit()`), filled by `ftManagerPortSetupSynthFileSizes` with the same rules
   as the vanilla loop and applied in `ftManagerAllocFighter` (which also grows
   `gFTManagerFigatreeHeapSize`).
2. New o2r blob `remix/ram_image` (`yamls/remix/remix.yml`, `SSB64:REMIXBLOB`, ROM 0x2C00000,
   0x400000 bytes). `port/remix/remix_ram_image.cpp` keeps a native-u32 copy (same layout as
   reloc files after their blanket swap). `ftPortCopyMotionArray` maps `command_offset` through it,
   and `portRelocSetFallbackResolver` lets `PORT_RESOLVE` map goto/subroutine operands that do not
   decode as tokens.
3. `scautodemo.c`: names use `port_fighter_parent_fkind`, clamped to the table.

## Audit hooks

- Any per-fkind size or table computed in a `for (i = 0; i < nFTKindEnumCount; i++)` loop silently
  skips synth fighters; the failure shows up far away as memory corruption.
- Remix data reached through its code image uses raw `0x804xxxxx` addresses, not reloc tokens.
