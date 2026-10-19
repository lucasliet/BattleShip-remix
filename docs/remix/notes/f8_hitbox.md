# F8 notes: Hitbox.asm display and Model Display (Costumes.asm)

Source: `.toolchain/pkgs/smashremix-src/src/Hitbox.asm` (871 lines), `Costumes.asm` 1188-1270 (force_hi_lo_poly_).
Port file: `decomp/src/ft/ftremix/ftremixhitboxdisplay.c`, declared in `decomp/src/ft/ftremix/ftremixhooks.h` (appended block).

## Toggles (Toggles.asm 2386-2388)

| Title | Symbol | Values | Defaults CE / TE / NE / JP | Read by |
|-------|--------|--------|----------------------------|---------|
| Model Display | `entry_model_display` | 0 default, 1 high poly, 2 low poly | 0 / 0 / 1 / 0 | Costumes.asm force_hi_lo_poly_ |
| Special Model Display | `entry_special_model` | 0 off, 1 hitbox, 2 hitbox+model, 3 ECB | 0 / 0 / 0 / 0 | Hitbox.asm hitbox_mode_ and others |
| Advanced Hurtbox Display | `entry_advanced_hurtbox` | off / on | off in all profiles | Hitbox.asm grab_immunity_and_armor_frames_ |

The asm reads the three toggles directly (no single-player guard), so the port uses `gmRemixToggle`.
Run override for the lead: `SSB64_REMIX_TOGGLES="Model Display=1"` or `SSB64_REMIX_TOGGLES="Special Model Display=2"`.

## Patches

| # | Asm address | Vanilla function (decomp) | Status | What the port does |
|---|-------------|---------------------------|--------|--------------------|
| 1 | 0x800D8050 (Costumes, load) | `ftManagerMakeFighter` (`ftmanager.c:1024`) | applied | `fp->detail_curr = fp->detail_base = ftRemixModelDisplayDetail(desc->detail);` under `REGION_REMIX` |
| 2 | 0x800E91C0 (Costumes, VS pause swap) | `ftParamSetModelPartDetailAll` (`ftparam.c:1111`) | applied | `detail = ftRemixModelDisplayDetail(detail);` at the top of the function (the asm patch sits at the function entry, so every caller gets it) |
| 3 | 0x800F2BFC (Hitbox, `hitbox_mode_`) | `ftDisplayMainProcDisplay` (`ftdisplaymain.c`) | applied, moved | `ftRemixHitboxDisplayApply(fp);` before the port `HitboxView` override, at the top of the function. The asm hook sits mid-function, so the timing is per frame, not identical |
| 4 | 0x800F2FD0, 0x800F3648 (render hitbox over model, ECB over hurtboxes) | `ftDisplayMainProcDisplay` | folded into 3 | the port's `nDBDisplayModeHitAttackOutline` (model + boxes) and `nDBDisplayModeMapCollision` (ECB) |
| 5 | 0x800F330C, 0x800F364C (jump targets) | `ftDisplayMainProcDisplay` | folded into 3 | not separate; the port draw path picks the mode |

### Skipped, with reason

- **Advanced Hurtbox Display** (`grab_immunity_and_armor_frames_`, 0x800F279C, in `ftDisplayMainDrawParts` `ftdisplaymain.c:984`).
  The asm recolors each joint hurtbox (gray in armor frames, cyan when reflecting, absorbing or grab-immune). The port
  draws no per-joint hurtboxes: `ftDisplayMain` only draws one cuboid (`dFTDisplayMainHurtCollisionCuboidDL`, magenta).
  REMIX-TODO(hurtbox display): needs the per-joint hurtbox draw first. Toggle has no effect yet.
- **Item display state** (the loop in `hitbox_mode_`, item struct offset 0x0374). The items' display modes live in
  `it/itdisplay.c` and `wp/wpdisplay.c`, outside this scope. REMIX-TODO(item display).
- **Projectile display** (`projectile_` 0x80167538, `render_hitbox_plus_projectile_` 0x80167578 / 0x801675AC). Same reason:
  `wpdisplay.c` is outside the scope. REMIX-TODO(projectile display).
- **Transparent hurtboxes and hitboxes** (`make_hurtboxes_transparent_` 0x800F2830, 0x8017193C; `make_hitboxes_transparent_`
  0x800F33B0; `add_reflect_absorb_hitboxes` 0x800F3358). These patch N64 display lists with blend commands. The port display
  has no equivalent yet. Mode 2 uses the outline mode instead (see uncertainties).
- **Hitbox over item and render-item patches** (`render_hitbox_plus_item_`, 0xEC760 to 0xFE008). Same as item display.
- **Kirby Cube** (`kirby_cube_enabled`, state 4 at 0x0B4C, `skip_rendering_hitboxes_` 0x800F336C, `setup_`). A debug
  feature that loads its own file. REMIX-TODO(kirby cube).
- **Screen mooring and star KO fixes** (`fix_hitbox_position_` 0x800F3310, `0x800F29A0`). These fix the N64 hitbox position on
  the CSS and results screens. The port draws the boxes at the fighter position, so they are not needed.
- **Display list buffer sizes** (`dw` at 0x8013B9C8, 0x80138CC8, 0x80135B70, 0x80135B80). N64 memory sizing, not ported.
- **Per-port Model Display** (`Costumes.model_display_for_port`, written by `css/ModelDisplay.asm`). The CSS is not ported.
  REMIX-TODO(CSS model display). The array is 0 (default) in the port, so the per-port step is a no-op.

## Behavior with the toggles

- Model Display 1 or 2: every fighter loads and swaps to hi-poly (`nFTPartsDetailHigh`) or lo-poly (`nFTPartsDetailLow`).
  Applies to every fighter spawn and to the VS pause swap, unless the screen is HTP, Demo or an intro scene.
- Special Model Display 1 (hitbox): fighter display mode `nDBDisplayModeHitCollisionFill`.
- Special Model Display 2 (hitbox+model): fighter display mode `nDBDisplayModeHitAttackOutline` (red outlines over the model).
- Special Model Display 3 (ECB): fighter display mode `nDBDisplayModeMapCollision` (ECB and model).
- Special Model Display 0: the mode is not written, so the vanilla mode and the `HitboxView` CVar still apply.
- Screens with no effect: scene ids `nSCKindStartup` to `nSCKindOpeningNewcomers` (intro) and `nSCKindExplain` (HTP),
  `nSCKindAutoDemo` (Demo). The asm uses the Remix screen ids, which match the port scene enum (see grmetallic note).

## What the lead should see (on)

- `Model Display=1` in VS: every fighter is high poly; `=2` low poly, visible on the VS pause swap too.
- `Special Model Display=1`: the fighter hitboxes show with the model, filled. Check that the model is still drawn.
- `Special Model Display=2`: red outlines on the attack hitboxes over the model.
- `Special Model Display=3`: the ECB shows on the fighter.
- Advanced Hurtbox: no visible change (skipped).

## Uncertainties

- Mode 1 keeps the model visible. The port draws the model in every non-Master mode (`ftDisplayMainDrawParts` runs for all of
  them), while the N64 may hide it in hitbox-only mode. Check in the game.
- Mode 2 uses the outline mode, not the N64 transparent-hitbox blend. Approximation.
- Mode 2 and mode 1 differ only by the outline colors in the port. Expect a close look.
- The toggle is not reset to Master when set back to 0 during the same fighter's life. The creation of the next
  fighter resets it (`ftmanager.c:1052`).
- `ftparam.c:1821` (a `-Wparentheses` warning in the cargo `ftRemixDKSharedIsCargoClone` change) is not in this scope. Left
  alone.
