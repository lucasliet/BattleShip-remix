# F14 hitbox display: Hitbox.asm

Asm: `.toolchain/pkgs/smashremix-src/src/Hitbox.asm`. Port: `decomp/src/ft/ftremix/ftremixhitbox.c` (display-list builders, grab immunity), `ftremixhitboxdisplay.c` (toggle readers, fighter and item modes), `ftremixhitbox.h` (declarations).

Toggles (`Toggles.asm`): `Special Model Display` (0 off, 1 HITBOX, 2 HITBOX+, 3 ECB), `Advanced Hurtbox Display` (0 off, 1 on), `Model Display` (0 default, 1 high poly, 2 low poly). Vanilla address to port function, with the vanilla delta 0x80084800 for 0x800F xxxx and 0x800855C0 for 0x8016/0x8017/0x8018 addresses.

## Done

| Patch | Asm | Where in the port |
|-------|-----|-------------------|
| force_grab_immunity_ (Giga Bowser, Crash DSP) | 0x800E6064 | `ftRemixForceGrabImmunity`, called from `ftmain.c` |
| hitbox_mode_ fighter part | 0x800F2BFC | `ftRemixHitboxDisplayApply`: 1 HitCollisionFill, 2 Master, 3 MapCollision, 0 Master |
| hitbox_mode_ item display state loop | 0x800F2BFC (FIRST_ITEM_PTR 0x80046700, ITStruct 0x374) | `ftRemixHitboxDisplayApplyToItems`: writes `ip->display_mode` of every object of `nGCCommonLinkIDItem` |
| projectile_ | 0x80167538 (`wpDisplayMain`) | `wp->display_mode = ftRemixHitboxDisplayObjectMode()` before the HitboxView override |
| render_hitbox_plus_model_ | 0x800F2FD0 (`ftDisplayMainProcDisplay`) | the hitbox branch also runs after the model branch when the toggle is 2 and the mode is Master. Removes the old DEVIATION |
| render_hitbox_plus_projectile_ | 0x80167578, 0x801675AC (`wpDisplayMain`) | toggle 2 draws the projectile and then `wpDisplayHitCollisions` |
| render_hitbox_plus_item_ | 0x80171D20 (OPA), 0x80171DDC (XLU), 0x80171FF0 (ColAnim OPA), 0x801722F0 (ColAnim XLU), 0x8018346C (`itPippiCommonProcDisplay`), 0x801835C8 (`itPippiCommonMoveDLProcDisplay`), 0x8017E5CC (`itKabigonCommonProcDisplay`), 0x8017E228 (`itKabigonFallProcDisplay`) | `itDisplayRemixHitCollisions(item_gobj, model)` in `it/itdisplay.c`, called in place of `itDisplayHitCollisions` at the 8 sites. Toggle 2 draws the model first. The monster sites set the model render mode (tex edge for rising, XLU for falling) before the toggle check, as `_clefairy_snorlax_shared` does (0x00553078 and 0x005041C8 decode to `G_RM_AA_ZB_TEX_EDGE` and `G_RM_AA_XLU_SURF`) |
| make_hitboxes_transparent_ | 0x800F33B0 | the outline colours test `display_mode == HitAttackOutline \|\| toggle == 2` |
| make_hurtboxes_transparent_ | 0x800F2830 (fighters), 0x8017193C (items) | `ftRemixHitboxDisplayDrawHurtbox`: toggle 2 copies the env colour word into the prim colour command, adds blend colour 0x000000E0 and draws `dFTDisplayMainHitCollisionCubeDL` (0x8012C310) instead of the cuboid (0x8012C058), as the asm edits the display list |
| grab_immunity_and_armor_frames_ | 0x800F279C, Normal case of the hurtbox colour switch | `ftRemixHitboxDisplayNormalHurtboxColor`. Advanced Hurtbox on: cyan 00FFFF while `is_reflect \|\| is_absorb` (flag halfword 0x018C mask 0x0480), else gray 606060 while `fp->knockback_resist_status` (FTStruct 0x07E8, the Remix Armour command writes it) is non-zero, else 00D0D0 when `damage_coll->is_grabbable` (0x10) is 0, else D0D000 (vanilla colour, found in `ftDisplayMainDrawParts`). The Invincible and Intangible cases are not patched by the asm |
| add_reflect_absorb_hitboxes | 0x800F3358 | `ftRemixHitboxDisplayDrawSpecialColl`, called inside `ftDisplayMainDrawParts` when `dobj` is the joint of `fp->special_coll` and `is_reflect \|\| is_absorb`. The asm builds the joint matrix chain by hand after the parts (first-child walk from the top joint); the port draws it while the joint matrix is on the stack, like a hurtbox. Offset and `size / 15`, hitbox cube list, blend alpha E0, prim FF8040 (reflect) or FF4080 (absorb, also for the Reflect.asm custom ids Marina and Dedede, kind >> 16 != 0), env white |
| fix_hitbox_position_ | 0x800F3310 | `ftRemixHitboxDisplayFixesPosition`: toggle 1 or 2 on the CSS screens, VS results, 1P intro, opening room, running, ending room (and the Continue screen) draws the hitbox without the RotRpyR mooring |
| star KO display fix | 0x800F29A0 | an invisible fighter stops at the top of `ftDisplayMainProcDisplay` in every display mode (`addu t3, r0, r0` replaces the display mode load) |
| dw buffer sizes | 0x8013B9C8 (VS CSS graphics heap, 0x3A98 + 0x4000), 0x80138CC8 (1P CSS display list 0, 0x4A38 + 0x4000 N64 bytes), 0x80135B70 (1P intro display list 0, 0xC350 + 0x10000), 0x80135B80 (1P intro graphics heap, 0x10000 + 0x11000) | `mnplayersvs.c`, `mnplayers1pgame.c`, `sc1pintro.c`. The display list sizes are added as `Gfx` counts (bytes / 8) because the port's `Gfx` is 16 bytes. Offsets confirmed against `SYTaskmanSetup` (+0x1C display list 0, +0x2C graphics heap) and the original values in the ROM |

Giga Bowser: `Character.id.GBOWSER` = 0x35 = `REMIX_ID_GBOWSER`. The asm forces non-grabbable for any Giga Bowser while `singleplayer_mode_flag == REMIX_1P_ID (4)`. The CPU check is commented out in the asm, so the port applies it to any controller.

## Changes to older behavior

- The scene gate (HTP, Demo, intro scenes) on the fighter hitbox display was removed. Only `Costumes.asm force_hi_lo_poly_` (Model Display) checks screens; `hitbox_mode_` has no check, and `fix_hitbox_position_` even lists intro screens, so the display is meant to work there.
- Toggle 0 now writes `nDBDisplayModeMaster` to the fighter each frame, as the asm does (it writes state 0). Before, the mode stayed at its last value after the Training d-pad cycle returned to off. The HitboxView enhancement still applies after this call.

## Not ported

- Kirby Cube: `kirby_cube_enabled`, display state 4, `skip_rendering_hitboxes_` (0x800F336C), `render_ecb_over_hurtboxes_` (0x800F3648), the cube branches of `hitbox_mode_` (0x800F2BFC), `make_hurtboxes_transparent_` (`_kirby_cube`), `fix_hitbox_position_` and `setup_`. Evidence: `Hitbox.setup_` (called from Render.asm 2252..2470 and SinglePlayerModes.asm 2387) enables it only when the first word of `CharacterSelectDebugMenu.PlayerTag.string_table + 8*4` reads `cube` (0x63756265), the text of the user-editable 8th Player Tag (`Toggles.entry_player_tags_7`), and loads Remix file 0x124F (`File.KIRBY_CUBE`, display list at +0x980). The port's Player Tag panel is a number (`port/remix/remix_player_panel.cpp`, no tag text), so the flag is always FALSE. With it FALSE the four patches are no-ops: display state 4 is never written, and the ECB display state 3 never enters the hitbox branch (0x800F2C0C sends state 3 to the model path), so 0x800F336C and 0x800F3648 only run with state 0, 1 or 2 where they fall through to the original code.
- The `knockback_resist_status` test uses `!= 0.0F`; the asm tests the word, which differs only for -0.0.
- The Continue screen of `fix_hitbox_position_`: the asm identifies it by the 1P screen id plus first loaded file 0x4F. The port has no loaded-file list and has `nSCKind1PContinue` as its own scene, which is used instead. The TITLE_AND_1P screen (Title, 1P game over, 1P battle) is not in the fix list for that reason.

## Lead wiring

None new. `ftRemixForceGrabImmunity` and `ftRemixHitboxDisplayApply` are already called from `ftmain.c` and `ftdisplaymain.c`.

## Test in game

Run with `SSB64_START_SCENE=22` (VS) and `SSB64_VS_FKINDS=<a>,<b>`, `SSB64_SCREENSHOT_FRAMES=<frames>`. Toggle titles: `Special Model Display`, `Advanced Hurtbox Display`. Training: the d-pad down cycles the Special Model Display (off, hitbox, hitbox+, ECB), `SSB64_START_SCENE=18` is the Training CSS.

- `SSB64_REMIX_TOGGLES="Special Model Display=2"`: fighters show the full model with outlined hurtboxes (coloured outline, no fill) and red outlined hitboxes on attacks; projectiles (Fox blaster, Mario fireball) show the model plus red outlined hitbox; items with an active hitbox (Beam Sword swing, Bob-omb explosion, Pokeball monsters) show the model plus the hitbox.
- `Special Model Display=1`: no model, opaque textured hurtboxes (yellow, green when invincible, blue when intangible) and filled red hitboxes. Reflect (Fox shine `down-B`, Mario cape) and absorb (Ness down-B) show an orange or pink translucent sphere at the joint.
- `Special Model Display=1;Advanced Hurtbox Display=1`: hurtboxes cyan during Fox shine or Ness absorb; gray on a fighter in armor frames (a move with the Remix Armour command); dark cyan on hurtboxes whose `is_grabbable` is 0 (Giga Bowser in Remix 1P is only a grab-immunity, not a hurtbox flag); otherwise yellow.
- `Special Model Display=3`: model plus ECB, items show their map collision diamond.
- `Special Model Display=1` on the VS CSS (`SSB64_START_SCENE=16`), the 1P CSS (17) and the VS results: hitboxes sit on the fighters and the screen does not overflow its buffers.
- Star KO the last stock with the toggle on: the fighter does not stay on screen.
