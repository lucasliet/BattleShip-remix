# F10 panel options: MODEL_DISPLAY and SKELETON

## MODEL_DISPLAY (closed)

Asm: `src/Costumes.asm` `force_hi_lo_poly_` (0x800D8050 character load, 0x800E91C0 `_swap` VS pause). Table
`model_display_for_port` (`src/Costumes.asm:1188`). Universal toggle `Toggles.entry_model_display` is checked first, then the
per-port value overrides it when non-zero. Values: 0 Default, 1 Hi Poly, 2 Lo Poly.

Port:
- `ftRemixModelDisplayDetail(u8 detail, s32 player)` in `decomp/src/ft/ftremix/ftremixhitboxdisplay.c`. Reads
  `gmRemixPanel(MODEL_DISPLAY, player)`; when 0 falls back to `gmRemixToggle(model_display)`. HTP, Demo and intro scenes are
  still ignored, as before. `player` is new (was only `detail`).
- Callers (vanilla files, one line each, under the existing `REGION_REMIX` guards):
  - `decomp/src/ft/ftmanager.c` (ftManagerMakeFighter, the detail of a new fighter, `fp->player` already set).
  - `decomp/src/ft/ftparam.c` (`ftParamSetModelPartDetailAll`, the detail of every model part update).
- Declaration updated in `decomp/src/ft/ftremix/ftremixhooks.h`.
- Default 0 on the panel and toggle 0 return the vanilla `detail`, so the vanilla path is unchanged.

Not ported: nothing from the asm semantics. The asm's `_swap` path (VS pause, `s3` port) maps to the same call as the
creation path in the port, because the port recomputes the detail from `ftParamSetModelPartDetailAll`.

Verification: `scripts/remix/syntax-check.sh` OK on `ftremixhitboxdisplay.c`, `ftmanager.c`, `ftparam.c`, `ftdisplaymain.c`.
Not built, not run (lead builds). A warning from `decomp/src/gm/gmremixpanel.h` (`/*` inside a block comment) belongs to
another worker's header and was not touched.

## SKELETON (open, REMIX-TODO(Skeleton))

Asm: `src/Skeleton.asm`. `enable_for_port` (`:15`), `apply_skeleton_mode` macro, hooks `skeleton_mode_1_` (0x800F24B0,
inside `ftDisplayMainDrawAll` 0x800F24A0) and `skeleton_mode_2_` (0x800F2228, inside `ftDisplayMainDrawSkeleton` 0x800F21B4).
When the port's flag is 1, the asm loads `0x0800` into the register that selects the skeleton draw path.

Why it stays open:
- The port selects the skeleton path from `fp->colanim.skeleton_id` (`ftDisplayMainDrawAll`), which only the electrified
  colanim script sets (`ftmain.c` `nGMColEventSetSkeletonID`). Forcing the path needs a valid skeleton id for every fighter,
  and the port has no such id for a normal fighter.
- The asm register meaning (`0x0800` and the `0x0A88` source field) was not decoded, so the exact port equivalent is not
  verified. Guessing it would risk changing the draw of vanilla fighters.
- The `REMIX-TODO(Skeleton)` is at `decomp/src/ft/ftdisplaymain.c` above `ftDisplayMainDrawAll`.

Next step to close it: decode the two asm hook sites at 0x800F24B0 and 0x800F2228, then decide whether the port can use a
fixed skeleton id or needs a separate draw path.
