# F7 hazard notes: Onett (stage 0x49, nGRRemixStage_ONETT)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 6641-6887 (onett_setup, car_main_, car_stage_setting_,
car_blast_zone_) and `src/items/Car.asm`.

## Files

- `decomp/src/gr/grremix/grremixhazards_onett.c`: `grRemixHazardOnett`. TRUE for `REMIX_SYM_Hazards__onett_setup`.
  Calls `itRemixCarHazardSetup(header)` when `grRemixHazardsAreOn()`. Header = `gMPCollisionGroundData - 0x14`.
- `decomp/src/it/itremix/itremixcar.c` / `.h`: Car item (kind 0x3C), `dITRemixCarItemDesc`, `itRemixCarMakeItem`,
  `itRemixCarHazardSetup`. Honk timer and state live in the file-local `sITRemixCarState`.

## Routines

- onett_setup: ported (hazard toggle, header, timer and honk flag clear, spawn).
- car_main_: ported (training-mode check, match-start check, honk roll, honk, 35-frame wait, drive out).
- car_stage_setting_: ported (scale 0.75, hitbox fields, blast zone pointer). Unnamed fields are REMIX-TODO.
- car_blast_zone_: ported except the final jump to 0x8016F8C0 (REMIX-TODO, see below).

## Port choices

- Item file: `ITDesc.p_file = &sITRemixCarState.stage_header`, `o_attributes = 0xE0`. The asm's `0x801313F4` slot holds the
  Onett header, so this is the same file base.
- Spawn: `itManagerMakeItemSetupCommon(NULL, 0x3C, ...)`, the stage path. `SHOW_GFX_WHEN_SPAWNED` is FALSE in Car.asm.
  The asm passes object id 0x3F5 as the parent; the port passes NULL (no owner).
- Position: the asm writes car_coordinates to the DObj translate and the item location. The port does the same.
- Visibility: GObj offset 0x38 has no named field in the decomp GObj struct, so hide and show use `DObj.dl`
  (saved at spawn, cleared on blast zone, restored on drive out).
- Timer: the asm clears `ground_data + 0x08` in onett_setup. That is not the timer car_main_ reads (0x801313F0 + 8),
  so the port clears its own timer instead. No behavior difference found.

## REMIX-TODO and deviations

- `0x8016F8C0` (blast zone jump, bomb/grenade routine, inside `itProcessProcItemMain`, not identified): not jumped to.
  `proc_dead` returns FALSE so the car stays alive and can drive again. Same deviation as KlapTrap.
- Unnamed fields: 0x33E halfword (0xB4), 0x158 bit 0x10, 0x2D3 bit 0x08, 0x1D0/0x1D4 (hitbox refresh vars),
  0x35C, and the y store at 0x354 in the blast zone.
- Clang: asm clears the halfword top bit at 0x158, which is `attack_coll.can_setoff`. Named in the port.
- Hitbox fields 0x10C..0x156 are named through `ITAttackColl` (attack_state, damage, element, offsets, size, angle,
  knockback, fgm_id). Offsets were checked against the struct layout.

## Uncertainties

- Screen id: asm compares `Global.current_screen` with 0x36 (Remix training mode). The port's `scene_curr` uses the decomp
  `nSCKind1PTrainingMode` (0x38). The two numberings differ. Confirm in game: training mode should skip the match-start check.
- Header base: `gMPCollisionGroundData - 0x14` follows the asm's `header + 0x14` comment. Not verified at runtime.
- States 1 and 2 of the Car item table (state 2 uses vanilla 0x8017D298) are not ported. The car never sets them in its asm.

## For the lead

- `tools/gen_remix_items.py` must register `dITRemixCarItemDesc` (kind 0x3C).
- Test stage: Onett, id 0x49, run with `SSB64_VS_STAGE=73`. Expect the honk after about 2700 frames (or earlier by the 1-in-200
  roll), then the car drives left at -150 speed.
- Syntax check: `scripts/remix/syntax-check.sh` returns OK for `itremixcar.c`, `itremixcar.h` and `grremixhazards_onett.c`.
  No build or game run was done.
