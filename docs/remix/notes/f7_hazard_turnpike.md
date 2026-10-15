# F7 hazard: Toad's Turnpike (stage 0xA6, nGRRemixStage_TOADSTURNPIKE)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 6421-6637 (toads_turnpike_setup, toads_turnpike_setup_2, turnpike_timeline, toads_turnpike_main).

## Ported
- `toads_turnpike_setup` (dispatch `REMIX_SYM_Hazards__toads_turnpike_setup`): `grRemixHazardTurnpike` in `decomp/src/gr/grremix/grremixhazards_turnpike.c`.
- `toads_turnpike_setup_2` + `toads_turnpike_main` + `turnpike_timeline`: `decomp/src/gr/grremix/grremixturnpike.c` / `.h`.
  - Per-frame light cycle: 720-frame timer, 6 timeline entries (start angle, speed, direction, end time), angle = offset * speed + start.
  - Each fighter (`gGCCommonLinks[nGCCommonLinkIDFighter]` link list): `colanim.is_use_light = TRUE` (asm `ori 0x0040` on byte 0x0A88, bit 1), `light_angle_x` = angle when `lr == 1`, else -angle, `light_angle_y = 30.0F`.

## Deviations
- The asm registers the routine on the camera object (`Render.register_routine`, camera +0x110). The port creates a ground GObj with `gcAddGObjProcess` at priority 4 instead. Execution order relative to fighter processes is not verified.
- The asm gate in `toads_turnpike_setup_2` is `Toggles.entry_hazard_mode > 1` (skip). The port uses `grRemixMovementIsOn()`, as the worker brief prescribes.
- `toads_turnpike_main` has a second `mul.s f4, f4, f6` inside `_loop` (asm line 6608). It writes f4, which is not read afterwards, so the port omits it.

## Skipped
- None. No Car or other item is spawned by this stage; the Car belongs to `onett_setup` (asm line 6641).

## Unverified
- Visual result of the light angle in-game. Test with `SSB64_VS_STAGE=166` (0xA6).
- The asm comment states "This routine works but is unfinished". The port reproduces the asm as written.

## REMIX-TODO
- None.
