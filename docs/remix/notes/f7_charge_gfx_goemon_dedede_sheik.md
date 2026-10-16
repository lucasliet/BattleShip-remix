# F7 charge GFX routines: GOEMON_CHARGE, DEDEDE_CHARGE, SHEIK_CHARGE

Ids come from the `add_gfx_routine` list in `.toolchain/pkgs/smashremix-src/src/GFXRoutine.asm` (lines 357-406, the first
call is MEWTWO_NAIR = 0x56, so the id is 0x55 + position): GOEMON_CHARGE is #31 (0x73), DEDEDE_CHARGE #32 (0x74),
SHEIK_CHARGE #24 (0x6D).

## Wired

- GOEMON_CHARGE: `ftRemixGoemonNSPStartCharge` (ftremixgoemonnsp.c) calls
  `ftParamCheckSetFighterColAnimID(fighter_gobj, 0x73, 0)` where the asm calls `0x800E9814` with a2 = 0
  (GoemonSpecial.asm 281 and 325, both on the charge timer reaching CHARGE_TIME).
- DEDEDE_CHARGE: `ftRemixDededeDSPAddCharge` (ftremixdedededsp.c) starts 0x74 when the level reaches
  DEDEDE_DSP_CHARGE_MAX (DededeSpecial.asm 2879-2882, a2 = 0).
- SHEIK_CHARGE: already started at max in `ftRemixSheikNSPChargeProcUpdate` (0x6D, a2 = 0, SheikSpecial.asm 1214).

## Still open

- SHEIK_CHARGE level write (SheikSpecial.asm 1221-1225): each charge tick writes the level to user_data+0x2A4 of the
  spark GObj stored at FTStruct 0x0B20. The colour animation has no GObj or per-fighter data, so the write stays a
  `REMIX-TODO(GFXRoutine.SHEIK_CHARGE)`. Only the first 0x40 bytes of SHEIK_CHARGE.bin were inspected; they show a fixed script
  with no user-data read. The consumer of +0x2A4 is not identified.
- Dedede: the charge GFX redirect (Dedede.asm 623-625, `charge_gfx_routine_`) is outside this scope.
