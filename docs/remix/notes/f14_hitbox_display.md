# F14 hitbox display: Hitbox.asm

Asm: `.toolchain/pkgs/smashremix-src/src/Hitbox.asm`. Port: `decomp/src/ft/ftremix/ftremixhitbox.c`, `decomp/src/ft/ftremix/ftremixhitboxdisplay.c`.

## Done

| Patch | Asm | State | Where |
|-------|-----|-------|-------|
| force_grab_immunity_ Giga Bowser | `force_grab_immunity_` (0x800E6064), `_giga` branch | implemented | `ftRemixForceGrabImmunity`, `sc1PRemixModeIs(SC1PREMIX_MODE_REMIX_1P)` |
| hitbox_mode_ fighter part, toggle 1 and 3 | `hitbox_mode_` (0x800F2BFC) | implemented, existing | `ftRemixHitboxDisplayApply` (state 1 HitCollisionFill, 3 MapCollision) |
| hitbox_mode_ fighter part, toggle 2 | same | changed | was HitAttackOutline (hides the model in `ftDisplayMainProcDisplay`). Now Master, as the asm writes state 0. Overlay is a DEVIATION (see below) |

Giga Bowser: `Character.id.GBOWSER` = 0x35 = `REMIX_ID_GBOWSER`. The asm forces non-grabbable for any Giga Bowser while `singleplayer_mode_flag == REMIX_1P_ID (4)`. The CPU check is commented out in the asm, so the port applies it to any controller. The caller is `decomp/src/ft/ftmain.c:4205`, already wired.

## Deviation

- `DEVIATION(render_hitbox_plus_model_ not ported)` in `ftremixhitboxdisplay.c`: toggle 2 (hitbox+model) keeps the model and should draw the hitbox over it through `render_hitbox_plus_model_` (Hitbox.asm 0x800F2FD0, which jumps to 0x800F330C on hitbox mode). The overlay is not ported, so only the model shows.

## Not ported (subsystem missing or unverified)

- Advanced Hurtbox color, `grab_immunity_and_armor_frames_` (Hitbox.asm 0x800F279C, hook at 0x6DF9C). The asm (1) turns the color cyan (`0x00FFFFFF`) when `FTStruct` 0x018C mask 0x0480 is set (`is_reflect` bit 0x04 at 0x018C, `is_absorb` bit 0x80 at 0x018D), (2) turns it gray (`0x606060FF`) when the word at 0x07E8 is non-zero, and (3) shifts it right by 8 when the hurtbox grab flag (struct +0x10) is 0. The source of t5 (the vanilla color) and the owner of the 0x07E8 word are not verified. The offset table names 0x07E8 as `knockback_resist_status` (f32), which does not fit an armor-frames flag. Needs the vanilla hurtbox color code in `ftdisplaymain.c` (~1060 to 1110, hurtbox status switch) mapped first.
- Kirby cube state 4 in `hitbox_mode_` (`kirby_cube_enabled`, Kirby and J Kirby) and the ECB path in `render_ecb_over_hurtboxes_` (0x800F3648). Needs the Kirby cube subsystem.
- Item display state loop in `hitbox_mode_` (writes 0x374 of item structs, value 2 for hitbox+model). Needs the item display port (`ITStruct`, `it/itdisplay.c`).
- `projectile_` (0x80167538), `render_hitbox_plus_projectile_` (0x80167578, 0x801675AC), `render_hitbox_plus_item_` (9 hooks), `make_hurtboxes_transparent_` (0x800F2830, 0x8017193C), `make_hitboxes_transparent_` (0x800F33B0), `add_reflect_absorb_hitboxes` (0x800F3358 and dw buffers 0x8013B9C8, 0x80138CC8, 0x80135B70, 0x80135B80), `fix_hitbox_position_` (0x800F3310), star KO fix (0x800F29A0), `skip_rendering_hitboxes_` (0x800F336C). All are debug display code that edits display-list builders, not ported.

## Lead wiring

None new. `ftRemixForceGrabImmunity` and `ftRemixHitboxDisplayApply` are already called from `ftmain.c:4205` and `ftdisplaymain.c:1161`.

## Test in game

- Remix 1P (singleplayer_mode_flag 4) with Giga Bowser: try to grab it with any fighter, expect no grab.
- Special Model Display = hitbox+model: the model stays visible, no hitbox overlay yet.
- Special Model Display = hitbox: hitboxes drawn, model hidden.
