# Smash Remix Size system

Covers `Size.asm` (2724 lines): the per-player size multiplier that makes a fighter Giant (2.25) or Tiny (0.5) and everything that follows it
(body, physics, weight, camera, effects). Code: `decomp/src/ft/ftremix/ftremixsize*.c` (state and helpers), one-line hooks under
`#ifdef REGION_REMIX` in `ft/`, `gm/gmcamera.c`, `ef/efmanager.c`, `if/ifcommon.c`, `gr/grcommon/grinishie.c` and `sys/objanim.c`. Test:
`SSB64_REMIX_PANEL="Size.1=1"` (Giant) or `Size.1=2` (Tiny) for port 1 with `.claude/skills/remix-smoke-tests/scripts/vs.sh`, or a Super
Mushroom / Poison Mushroom with `SSB64_REMIX_FORCE_ITEM`.

## 1. State and sources

`ftremixsize.c` keeps four tables indexed by port (`fp->player`):

| Asm table | Port | Meaning |
|---|---|---|
| `multiplier_table` | `ftRemixSizeGetMultiplier` / `ftRemixSizeSetMultiplier` | current multiplier, 1.0 by default |
| `base_multiplier_table` | internal | the body size of the fighter, `attr->size`, that the top joint scale starts from |
| `match_state_table` | `ftRemixSizeGetMatchState` / `ftRemixSizeSetMatchState` | NORMAL (0), GIANT (1), TINY (2) |
| (mode override) | `ftRemixSizeForceMatchState` | state a mode forces on a port, `FTREMIX_SIZE_STATE_UNFORCED` (-1) gives the port back to the panel |

`ftRemixSizeInitFighter` (called from `ftManagerInitFighter`, asm `initialize_base_multiplier_table_`) stores the base size, reads the match
state from the forced state or the CSS panel `gmRemixPanel(SIZE, player)` (Default, Giant, Tiny) and sets the multiplier to the state's value
(`ftRemixSizeGetModeMultiplier`: 2.25, 0.5 or 1.0), except on the How to Play and Auto-Demo screens, where it stays 1.0.

What changes the multiplier at run time:

| Source | Code |
|---|---|
| CSS panel `Size` | `ftRemixSizeInitFighter` |
| Remix 1P giant (Donkey) and tiny (Samus) stages | `sc1PRemix1PSetupEnemy` (`sc/sc1pmode/sc1premix1p.c`) forces GIANT or TINY on the enemy; the Remix 1P setup clears it |
| Super Mushroom and Poison Mushroom | `itremixsupermushroom.c`: a size state machine grows to 2.25 or shrinks to 0.5 over its transition frames; a pickup that contradicts the match state (giant + super, tiny + poison) is cancelled on its first frame; on expiry the size returns to the match state |
| Lightning | `itremixlightning.c`: 0.5 on zapped players, restored to the match state when the timer ends |
| Metallic Madness laser | `grRemixMetallicShrinkPlayer`: 0.5 for 900 tics, restored to the match state (`docs/remix/hazards.md`) |

## 2. Formulas and constants

The attribute scalings share one form, `x = vanilla * (1 + coefficient * (multiplier - 1))`, implemented by `ftRemixSizeAttrMultiplier(player,
coefficient)`. The `FTREMIX_SIZE_COEF_*` constants of `ftremixhooks.h` are the `Size.asm` values:

| Quantity | Coefficient |
|---|---|
| walking speed | 0.45 |
| brake force | -0.04712 |
| initial dash speed | 0.4 |
| dash deceleration | 0.45172 |
| running speed | 0.36664 |
| jump height multiplier | 0.3234 |
| base jump height | 0.0727 |
| second jump height | 0.0395604 |
| weight | -0.24 |

Other constants: Giant 2.25, Tiny 0.5; camera zoom factor `1 + (m - 1) * 0.5` (half of the size change reaches the zoom); Yoshi shield
scale 1.5 times the multiplier; hitbox damage factors 0.69921875 below 1.0 and 1.296875 above (the `lui`-only constants of the asm, not 0.7
and 1.3), rounded to nearest with ties to even like `cvt.w.s` (`ftRemixRoundDamage`); passive knockback resist +48 above 1.0; jump squat +1
frame above 1.0; the second jump is 0.8 times lower while the fighter jumps with a P-Wing (`itRemixPwingIsJumping`).

## 3. Hooks

Addresses are the vanilla RAM addresses patched by `Size.asm`; the function is where the port applies the helper.

### 3.1 Body and combat

| Asm scope | Vanilla function (file) | Port helper |
|---|---|---|
| `adjust_top_joint_and_ecb_` (0x800E2068) | `ftMainProcPhysicsMap` (ftmain.c), after the TopN copy | `ftRemixSizeApplyTopJointAndEcb`: top joint scale `base * m`; ECB top, centre, bottom, width and the ledge grab box `attr * m` |
| `adjust_ecb_display_` (4 patches, 0x800F3004 ff.) | `ftDisplayMainProcDisplay` (ftdisplaymain.c) | `ftRemixSizeScaleValue` on the ECB display box (bottom, centre, width, top) |
| `fix_size_` (0x800DF394, 0x800DF6AC) | `ftMainParseMotionEvent` (ftmain.c), `MakeAttack2` and `SetAttackCollSize` | `ftRemixSizeScaleHitboxSize`: `trunc(size * m)`, halved as in vanilla; offsets are not scaled |
| `fix_damage_` (0x800DF34C) | `ftMainParseMotionEvent`, `SetAttackCollDamage` | `ftRemixSizeScaleDamage`, after `ftRemixChargeSmashScaleDamage` (charged smash factor `1 + time * 0.4 / 60`, rounded; applies when the Charged Smashes toggle is on and the fighter is charging). The create-hitbox damage store is not scaled |
| `passive_armor_` (0x800E6324) | `ftMainProcParams` knockback block | `ftRemixSizePassiveArmor`: +48 above 1.0; returns 0 for a Stamina fighter out of HP (`ftRemixStaminaArmorIsZeroed`, the asm calls `Stamina.stamina_armor_fix_` first) |
| `adjust_respawn_platform` (0x80104078) | `efManagerRebirthHaloMakeEffect` | `ftRemixSizeScaleValue` on the effect scale |
| `adjust_player_label_` (0x80111B04) | `ifCommonPlayerTagProcDisplay` | `pos.y += camera_zoom_base * m` |

### 3.2 Movement and weight

| Asm scope | Vanilla function | Helper |
|---|---|---|
| `adjust_player_walking_speed_` (0x800D8A74) | `ftPhysicsSetGroundVelAbsStickRange` | `ftRemixSizeWalkingSpeed` and `ftRemixSizeBrakeForce` (every caller, as the patch sits at the entry) |
| `adjust_player_initial_dash_speed_` (0x8013ED40) | `ftCommonDashSetStatus` | `ftRemixSizeInitialDashSpeed` |
| `adjust_player_dash_deceleration_` (0x800D897C) | `ftPhysicsSetGroundVelFriction` | `ftRemixSizeDashDecel` (every caller) |
| `adjust_player_running_speed_` (0x8013EF14) | `ftCommonRunSetStatus` | `ftRemixSizeRunSpeed` |
| `adjust_player_jump_squat_` (0x8013F308) | `ftCommonKneeBendProcUpdate` | `ftRemixSizeJumpSquatLength` |
| `adjust_jumping_height_multiplier_1` (0x8013F94C) | `ftCommonJumpSetStatus` | `ftRemixSizeJumpHeightMul`, `ftRemixSizeJumpHeightBase` |
| `adjust_jumping_height_multiplier_2` (0x8013FE64) | `ftCommonJumpAerialSetStatus` | `ftRemixSizeAerialJumpHeightSingle` (with the P-Wing 0.8) |
| `adjust_jumping_height_multiplier_kirby` (0x80140014) | `ftCommonJumpAerialMultiSetStatus`, the `jumps_used == 1` branch | `ftRemixSizeAerialJumpHeight` (not gated by fighter kind, as in the asm) |
| `adjust_player_weight_1..3` (0x800E3F88, 0x800E4120, 0x800E4244) | `ftMainProcessHitCollisionStatsMain`: fighter, weapon and item hits | `ftRemixSizeWeight(this_fp, attr->weight)` in the knockback formula |
| `adjust_player_weight_4` (0x80143D30) | `ftCommonTwisterShootFighter` | same |
| `adjust_player_weight_5`, `_6` (0x8014B084, 0x8014B3F0) | `ftCommonThrownReleaseThrownUpdateStats`, `ftCommonThrownSetStatusDamageRelease` | same |
| `adjust_player_weight_7` (0x8014E60C) | `ftCommonCaptureShoulderedSetStatus` | same |
| `adjust_player_weight_8` (0x80108DE8) | `grInishieScaleGetPressure` (grinishie.c) | scales `weight` inside `1 - weight + 1.4` |

The ground-hit branch of `ftMainProcessHitCollisionStatsMain` and `ftCommonThrownSetStatusNoDamageRelease` have no `Size.asm` patch and stay
vanilla.

### 3.3 Camera

| Asm scope | Vanilla function | Helper |
|---|---|---|
| `adjust_camera_zoom_` (0x8010BB9C) | `gmCameraCalcFighterZoomRange` (gmcamera.c) | `ftRemixSizeCameraZoomFactor` |
| `adjust_camera_center_point_` (5 patches: 0x8010BD78, 0x8010C9AC, 0x800F2A5C, 0x800F2A80, 0x800F2A9C) | `gmCameraUpdateInterests`, `gmCameraUpdatePlayerZoom`, `ftDisplayMainProcDisplay` (the interest offset, its compare and its scale argument, in both the AVOID_UB and N64 branches) | `ftRemixSizeScaleValue(fp, attr->cam_offset_y)` |

`gmCameraUpdatePlayerFollow` has no patch in the asm and stays unscaled.

### 3.4 Effects of the owner

All scale the effect by the multiplier of the fighter that owns it; at 1.0 they return before writing, except where noted.

| Asm scope | Vanilla function | Helper |
|---|---|---|
| `adjust_size_egg_shield_` (0x801011A4) | `efManagerYoshiShieldProcDisplay` | `ftRemixSizeScaleYoshiShield`: DObj scale `1.5 * m` (the Remix shield is 1.5 even at 1.0) |
| `adjust_size_egg_entry_` (0x80102F64) | `ftport.c` Yoshi entry, `ftRemixEntryMakeYoshiEgg` | `ftRemixSizeScaleEntryEgg` (accepts a NULL egg) |
| `adjust_size_egg_lay_` (0x8014C994) | `efManagerYoshiEggLayProcUpdate` and `efManagerYoshiEggLayMakeEffect` (efmanager.c) | `ftRemixSizeScaleYoshiEggLay`: egg scale = default egg size of the captured character (`effect_size` of the port egg descriptor) `* m` of the captured fighter; the z scale is replaced too |
| `save_player_struct_` (6 patches: 0x800EB0E8, 0x800EB0F8, 0x80141B40, 0x801551BC, 0x8014C40C) | impact wave in `ftParamMakeEffect`, `ftCommonWallDamageSetStatus`, `ftNessSpecialAirHiJibakuBoundSetStatus`, `ftCommonCaptureKirbyCollide` | `ftRemixSizeAttachGroundGfx` stores the owner in the effect |
| `update_routine_` (0x8012DFC4 + 0x10) | impact wave effect descriptor | `ftRemixSizeGroundGfxProcUpdate`: vanilla update, then the DObj scale (owner-less waves untouched) |
| `adjust_suck_gfx_` (4 patches, 0x8010429C) | `efManagerKirbyInhaleWindProcUpdate` | `ftRemixSizeKirbySuckTransform`: offsets `lr * 800 * m` and `230 * m`, transform scale `m` |
| Kirby USP gfx (`render_routine_` 0x8012E37C, 0x8012E3A4, `render_routine_descend_` 0x8012E3F4) | Kirby cutter effect descriptors | `ftRemixSizeKirbyCutterProcDisplay`, `ftRemixSizeKirbyCutterTrailProcDisplay` |
| `kirby_ausp_` (0x80160F34, 0x80160F60) | `ftKirbySpecialAirHiProcPhysics` | `ftRemixSizeKirbyAirHiScale`: the scale the move gives the top joint becomes the base size (the big-Kirby glitch persists after the move), times `m` |
| Samus gun blast `render_routine_` (0x8012E0B4 + 0x14) | fire spark effect descriptor | `ftRemixSizeFireSparkProcDisplay` (2nd joint scale and top-joint y 160 `* m`) |
| Falcon punch `render_routine_`, Falcon kick `update_routine_` (0x8012E2C4, also installed for the Falcon clones) | `dEFManagerCaptainFalconPunchEffectDesc`, kick effect | `ftRemixSizeFalconPunchProcDisplay`; `ftRemixSizeFalconKickProcUpdate` (2nd joint scale and z offset 324.00006 `* m`, skipped while the effect is paused) |
| `link.adjust_usp_projectile_size_`, `adjust_usp_gfx_size_` (0x80163E20, 0x80163DD8) | `ftLinkSpecialHiMakeWeapon` | `ftRemixSizeScaleLinkWeapon`, `ftRemixSizeScaleLinkEffect` (effect 2nd joint y = 300 `* m`; Crash's neutral special spin effect uses the same helper, and Crash's Dig effect update sets the owner's root DObj scale to the multiplier, `ftremixcrashdsp.c`) |
| `pikachu.adjust_gfx_size_` (0x8010146C) | `efManagerThunderAmpMakeEffect` | `ftRemixSizeScaleThunderAmp`: x and y scale `m` of the fighter it hits (Pikachu DSP) or zaps (Lightning) |
| `ness.nsp.adjust_pkfire_position_` (0x801539B4, 3 patches) | `ftNessSpecialNProcAccessory` (ftnessspecialn.c) | PK Fire spawn position scaled by `m` |
| `ness.usp` and `ness.dsp` `update_routine_`/`_initialize` (0x8012E494, 0x8012E41C, 0x80102A3C, 0x8010260C) | PK Thunder wave and Psychic Magnet effects | `ftRemixSizeNessUSPProcUpdate/Initialize`, `ftRemixSizeNessDSPProcUpdate/Initialize` (`ftremixsizeness.c`): vanilla animation update, then the joints scaled every frame |
| `dragonking.nsp.render_routine_` (0x8012E1D4 + 0x14) | Dragon King ball effect | `ftRemixDragonKingNSPApplyBallScale`: base 0.75 (0.5 for Kirby copies) `* m` |
| Arwing `update_routine_`/`_initialize` (0x8012E704, 0x801038F0) | `efManagerFoxEntryArwingProcUpdate`, `efManagerFoxEntryArwingMakeEffect` | `ftRemixSizeEntryArwingBind` (owner), `ftRemixSizeEntryArwingUpdate` (wing joint scale every frame) |
| entry `animation_` (0x8013DB2C) | `ftCommonAppearProcPhysics` | `ftRemixSizeEntryAnimationScale`: trans joint translate `* m` for Mario, Luigi, Dr. Mario, Dr. Luigi, J Mario, J Luigi, Metal Mario, Metal Luigi and Conker |
| entry `gfx_` x7 (0x80103768 pipe, 0x8010345C barrel, 0x801034B8 portal, 0x80102B28 and 0x80102B84 Link wave and beam, 0x80102C10 Kirby star, 0x80103670 Falcon car) | the seven `ef*Entry*MakeEffect` makers, called from `ftport.c` and `ftremixentry.c` | `ftremixsizeentrygfx.c`: joints of the entry effect scaled once, per effect; the Bowser clown car height base is 576 |
| `apply_size_to_animation_` (0x8003DE5C jump table) | animation command interpreter (`sys/objanim.c`) | `itRemixCloudScaleAnimValue` (`itremixcloud.c`): the Cloud item's animation scale commands follow the owner size |

### 3.5 CPU

`size_multiplier_check_` (`AI.asm`): `ftRemixAiSizeDetectScale` (`ftremixaisize.c`, used by `ftcomputer.c`) scales the CPU detect ranges by the
multiplier; Giant Donkey Kong returns 1.0 because its own 1.4 factor is applied by the caller.

## 4. Deviations

- Link's Up Special weapon and effect scales are written once at spawn; the asm rewrites them on every render. Same result while the
  multiplier does not change during the move.
- The Dragon King ball scale is applied after the animation update on every frame, not in a render wrapper.
- `adjust_player_weight_*` use the hit fighter (`this_fp`) as the owner of the multiplier at all eight sites; the asm reads the port byte from a
  register of the enclosing function.
- `adjust_hitbox` scales only the hitbox size (as the asm code does), although its comment says "size and location".
- The Arwing wing scale is applied in place each frame, like the asm; if the animation did not reset the joint it would compound away from 1.0.

Open gaps: `docs/remix/patch_audit.md`, group IT (g4).
