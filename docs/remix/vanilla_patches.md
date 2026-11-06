# Smash Remix patches of vanilla functions (damage, commands, surfaces, camera and fighter leftovers)

Covers the `OS.patch_start` hooks of Smash Remix that rewrite vanilla fighter, item, weapon and map code and are neither stage hazards
(`docs/remix/hazards.md`) nor Size (`docs/remix/size.md`): `Damage.asm`, `Poison.asm`, `command.asm`, `CustomGrabAction.asm`, `Knockback.asm`,
`Hitbox.asm`, `Surface.asm`, `Blastzone.asm`, `Camera.asm`, `Spawn.asm`, `SpecialZoom.asm` and the small patches of the `*shared.asm` and item files. Code: helpers
in `decomp/src/ft/ftremix/`, `gr/grremix/`, `it/itremix/`, `wp/*/wp*remix*.c`; the hooks are single lines or small blocks under
`#ifdef REGION_REMIX` in the vanilla function (the vanilla code stays in the `#else`). Test: the stage and match recipes of
`.claude/skills/remix-smoke-tests/`, with `SSB64_REMIX_TOGGLES="<title>=<n>"` and `SSB64_REMIX_PANEL="<Key>.<port>=<n>"`.

An asm patch address maps to the decomp function that contains it by the `// 0x...` comment above each function; the tables below give both.

## 1. Damage types, stun and bury (`Damage.asm`)

Remix adds damage elements 7 to 0xC to the vanilla 0 to 6 (`add_damage_type`): `FTREMIX_DAMAGE_SHADOW` 7, `STUN` 8, `LASER` 9, `DEKU_STUN` 0xA,
`BURY` 0xB, `FLASHBANG` 0xC (`ftremixhooks.h`).

| Asm scope | Vanilla function (file) | Port |
|---|---|---|
| `extend_on_hit_gfx_` (0x800E3FC4) | `ftMainProcessHitCollisionStatsMain` fighter-hit switch (ftmain.c) | SHADOW: `ftRemixDamageMakeShadowEffect` (next GFX id 0x6C, then the fire effect); STUN and FLASHBANG: the electric effect. LASER, DEKU_STUN and BURY keep the normal branch. Only the fighter-hit switch is routed; weapon and item hits keep the vanilla default |
| `extend_on_hit_routines_` (0x80140BCC) | `ftCommonDamageCheckElementSetColAnim` (ftcommondamage.c) | `ftRemixDamageSetColAnim` for ids 7 to 0xC: SHADOW colanim 0x59 + damage level, STUN and FLASHBANG the skeleton (electric) colanim, LASER 0x6B, DEKU_STUN 0x6C, BURY the common damage colanim |
| `stun_damage_action_` (0x8014158C) | `ftCommonDamageGotoDamageStatus` (ftcommondamage.c), before the sleep check | `ftRemixDamageGotoStunAction` (`ftremixdamagestun.c`); FALSE falls through to the vanilla status |
| `stun_electric_action_` (0x801412E8) | `ftCommonDamageInitDamageVars`, the electric status test | `ftRemixDamageUsesElectricStatus`: only ELECTRIC and FLASHBANG take the electric branch (STUN, DEKU_STUN and BURY take the vanilla one, although the asm comment says STUN acts like electric) |
| `prevent_wind_push_while_buried` (0x800E86B4) | `ftParamSetVelPush` (ftparam.c) | a buried fighter (`ftRemixDamageIsBuried`) stores only `vel_push.z` |

### 1.1 Stun, deku stun, flashbang

All three set `shield_health` to 30 and the breakout wait through `ftCommonCaptureTrappedInitBreakoutVars`; "default wait" below is 400 minus the
percent, not below 0 (`FTCOMMON_FURAFURA_BREAKOUT_WAIT_DEFAULT`), and the vanilla minimum is 90 on the US build.

| Type | Condition | Status | Breakout wait | Colanim |
|---|---|---|---|---|
| STUN | grounded, hit facing the fighter (`damage_lr == lr`), not already stunned | Fura Fura (0xA4) without preserved flags | default + minimum, through the Japanese Stun toggle (`ftRemixFuraFuraBreakoutWait`) | Fura Fura |
| STUN otherwise | | the vanilla damage status (the hook returns FALSE) | | |
| DEKU_STUN | not already stunned (else the hit does nothing) | Fura Fura on the ground, ShieldBreakFall in the air | default + 90, no Japanese toggle | 0x6C |
| FLASHBANG | not already stunned (else the hit does nothing) | Fura Fura on the ground; in the air ShieldBreakFall with `proc_update = ftRemixDamageStunMainAerial` and the air velocity halved | percent + 90 + (150 grounded, 0 aerial), no clamp | 0x25 |

`ftRemixDamageStunMainAerial` counts the wait down each frame, adds `3 *` the mash delta of `ftCommonCaptureTrappedUpdateBreakoutVars`, and
falls (`ftCommonFallSetStatus`) when the wait before the x3 scaling is not above 0.

### 1.2 Bury and plunge

`ftremixbury.c` ports `bury_or_plunge_initial_`, `bury_initial_` (called by `ftremixzcancel.c` for the Z-cancel punishment), `bury_player_`, `buried_main_`,
`buried_interrupt_`, `buried_collision_`, `unbury_player_`, `plunge_player_`, `plunge_ground_transition`, `plunge_main_`, `plunge_interrupt_`
and `plunge_collision_`. The buried state is `fp->proc_update == ftRemixBuryMain`.

- A grounded fighter is buried, an aerial one plunges: ShieldBreakFall at `vel_air.y = -50` (capped at -128), buried on landing through
  `mpCommonProcFighterLanding`. A grounded fighter on a soft platform (`MAP_VERTEX_COLL_PASS`) is plunged and dropped 10 units. On a floor with a
  nonzero material (the damage floors) the fighter is not buried while Hazard Mode keeps hazards on (`mpCommonSetFighterWaitOrFall`).
- Buried: status DownBounceU, FGM 0x11F, the processes `proc_update = ftRemixBuryMain`, `proc_accessory` empty, `proc_physics` NULL,
  `proc_map = ftRemixBuryCollision`. The timer is `99 + (s32)(0.7 * percent)` in `fp->breakout_wait` (the asm word 0x26C).
  Unbury is a short hop (`ftCommonJumpSetStatus` with the button source and `is_shorthop`); it also happens when the fighter moves into a wall or
  becomes aerial.
- Deviations: (1) the asm decrements the word 0x26C (`fp->breakout_wait`) on mashing; `ftRemixBuryMain` calls the mash helper but keeps its own
  timer, so mashing does not shorten the bury. (2) The asm lowers joint 0x08F8 (`joints[4]`, `nFTPartsJointCommonStart`) by 48 each tic; the port
  moves `joints[nFTPartsJointTopN]`. (3) The asm wall test reads a flag word that is mapped to `mask_curr & (MAP_FLAG_LWALL | MAP_FLAG_RWALL)`.
  (4) The plunge caps `physics.vel_air.y`, where the asm caps `coll_data.pos_diff.y`; the plunge never exceeds -50 so the result is the same.

## 2. Poison (`Poison.asm`)

`ftremixpoison.c`. `ftMainProcUpdateInterrupt` calls `ftRemixPoisonUpdate` when `damage_heal == 0` (asm `star_fix` branches to the heal check,
`poison_hook` at 0x800E1820), so a CSS Heal enters the heal block on the same frame. Per-port entries `{duration, damage, type}` are written by
the Spawned Fear acid (`ftRemixPoisonSet(port, 60, 3, 0)`); damage lands every 32 frames of the 16-bit poison timer
(`ftRemixPoisonAdvanceTimer`), not during invincibility or respawn (a status below Wait ends the poison), adds to the percent (capped at the
Stamina total HP in a Stamina match, skipping defeated fighters), plays FGM 0x1C, and starts the doom acid colanim 0x79 when no override is
active; a cloaked fighter (`itRemixCloakingDeviceIsCloaked`) is skipped. The tail (stock sync, colanim 9 reset) is `ftRemixPoisonSyncStock`.

The CSS `Poison Dmg` option (`css/PoisonDmg.asm apply_damage`) runs first in `ftRemixPoisonUpdate`: every 128 match tics (low bits below 3 at
1/3 and 2/3 game speed, never on the extra advance of a faster speed) Low, Medium and High add 1, 2 or 4%, capped at 999; Heal sets
`damage_heal = 2`. A Super Star or respawn blocks it; the Cloaking Device guards only the damage values, not the heal. `ftRemixPoisonClear`
is the port of `Poison.clear_poison_`; it is called from `itRemixClearActiveItems` (asm `Item.asm clear_active_custom_items_`) at the end of a match (IT-4).

## 3. Commands and custom grabs

### 3.1 `command.asm`

`ftremixcommand.c` implements the custom moveset commands 0xD0 to 0xDC (frame-speed, armour, hitbox direction, translation multiplier, y velocity,
fast fall, random SFX, kinetic state, hitbox FGM, env colour, switch direction, go to moveset file, L voice SFX; 0xDD to 0xFF advance one word)
through the three `load_command_` hooks in `ftmain.c`.

| Asm scope | Vanilla function | Port |
|---|---|---|
| `end_hitbox_` | `ftParamClearAttackCollAll` (ftparam.c) | hook in ftparam.c |
| `create_hitbox_` | `ftMainParseMotionEvent` (ftmain.c) | `ftRemixCommandResetAttack` |
| `texture_form_fix_` | `ftParamSetTexturePartID` | hook in ftparam.c (Metal Mario form) |
| `apply_direction_` (0x800E446C) | `ftMainProcessHitCollisionStatsMain`, fighter branch | `ftRemixCommandApplyDamageDirection` |
| `apply_fgm_` | `ftMainPlayHitSFX` | hook in ftmain.c |
| `change_action_` (0x800E6F2C) | `ftMainSetStatus` | `ftRemixCommandResetStatus`: translation multiplier and env colour reset (the Kirby clone swap of section 6 sits at the top of the same function) |
| `translation_multiplier_` `apply_ground_z_/x_`, `apply_air_z_/y_/x_` | `ftPhysicsApplyGroundVelTransN`, `ftPhysicsGetAirVelTransN` | `ftRemixCommandGetTranslationMultiplier` returns the command 0xD3 value times the Size factor of the asm (0.59765625 when the Size multiplier is above 1.0, 1.59375 when it is below 1.0, `ftRemixCommandGetSizeFactor` in ftremixcommandrules.c) |

Deviation: `respawn_with_item_` is hooked in `ftCommonRebirthStandSetStatus` (`ftRemixStartItemRespawn`); the asm tests `Action.Revive1`, which is
`nFTCommonStatusRebirthDown` (0x007), one status earlier.

### 3.2 `CustomGrabAction.asm`

`capture_action_override_`, `capture_position_fix_` and `capture_dk_break_fix_` (hooks in `ftCommonCapturePulledProcCapture` and
`ftCommonCaptureShoulderedProcInterrupt`) go through the fighter registry: `port_fighter_custom_capture_action` and
`port_fighter_custom_capture_dk_interrupt`, registered for Wario and Mad Piano in `ftremixdeschooks.c`. Mad Piano's DK interrupt
(`Piano.asm capture_action_fix_`) is `ftRemixPianoCaptureDKInterrupt`: while the grabber is in GrabWait (0xA8) the captured fighter is made
invisible, and it returns TRUE to skip the vanilla ThrownDK break-out.

## 4. Knockback and hitboxes

### 4.1 `Knockback.asm`

`ftremixknockback.c`. `randomize_angles_` (`ftManagerMakeFighter`, 0x800D8008) rolls a table of 0x200 angles per port when the scene changes
(60% an angle from 45 to 135, 15% semi-spike or spike 180 to 360, 15% fully random 0 to 360, otherwise the Sakurai angle 361); re-entering the
same scene (Training reset, salty runback) keeps the table. `get_angle_hitbox_` (`ftMainParseMotionEvent`, 0x800DF424) and `get_angle_throw_`
(`ftCommonThrownReleaseThrownUpdateStats`, 0x8014B16C) read it through `ftRemixKnockbackAngle(fp, vanilla_angle)` when the CSS `Knockback`
option of the port is not Default. The asm `randomize_angle_` is commented out in Remix.

### 4.2 `Hitbox.asm`

- `force_grab_immunity_` (0x800E6064, `ftMainSearchFighterCatch`): `ftRemixForceGrabImmunity(victim)` is TRUE for Crash in Dragon Shot Pursuit wait
  or turn (0xE6, 0xE7) and for Giga Bowser while the Remix 1P mode is active.
- Hitbox display (`ftremixhitbox.c`, `ftremixhitboxdisplay.c`, hooks in `ftDisplayMainProcDisplay`, `wpDisplayMain` and the item display
  functions): `Special Model Display` (`entry_special_model`: off, hitbox, hitbox + model, ECB) sets the display mode every frame for fighters, items and
  projectiles (`hitbox_mode_`, `projectile_`); `Advanced Hurtbox Display` selects the hurtbox colours (cyan while reflecting or absorbing, gray
  with knockback resist, dark cyan when not grabbable, yellow otherwise) and `add_reflect_absorb_hitboxes` draws reflect (orange, FF8040) and absorb
  (pink, FF4080) bubbles. The Kirby Cube branches (`kirby_cube_enabled`) need an editable 8th Player Tag reading "cube" and are not reachable.
  `Model Display` (`force_hi_lo_poly_` of `Costumes.asm`) is in the same file and ignores the How to Play, demo and intro scenes.

## 5. Surfaces, blast zones, camera and spawns

### 5.1 `Surface.asm`

`grremixsurface.c`: `grRemixSurfaceFriction(material)` reads the vanilla 16-entry table for materials 0 to 0xF and the Remix list for 0x10 to
0x24 (0.3 for Cool Cool Mountain 0x11, 4.0 for the rest; any other id gets 4.0 instead of reading past the table). Hooks:
the `ftPhysicsSetGroundVelFriction` call of `ftPhysicsApplyGroundVelFriction` (`get_friction_` 0x800D8BDC), the damage-ground branch of
`ftMainProcPhysicsMap` (0x800E21DC). The conveyor materials 0x1F to 0x22 push grounded fighters and items (`fighter_apply_conveyor_surface` 0x800E23D8 in
`ftMainProcPhysicsMap`; `item_apply_conveyor_surface` 0x8016F8E4 in `itProcessProcItemMain`) through `grRemixSurfaceConveyorPush`: speeds 20.5
and -24.0 (Remix RTTF) and 20.0 and -20.0 (Time Twister); outside Remix RTTF the push stops when Hazard Mode turns movement off.
`zebes_acid_fix_` only gives the Zebes acid knockback struct an extra FGM field (-1 = default); the port reads the stage-file struct.

### 5.2 `Blastzone.asm`

`BlastZone Warp (BETA)` (`entry_blastzone_warp`: bit 1 left/right, bit 2 top/bottom). `ftremixblastzone.c` replaces the KO call of
`ftCommonDeadCheckInterruptCommon` with `ftRemixBlastZoneWarpFromRight/Left/Top/Bottom` (`ftcommondead.c`): the fighter reappears at the opposite
map bound, grabs and ledges are released (`grab_release_check_`), a grounded fighter turns aerial (`set_aerial_from_grounded`), the hitbox
interpolation is reset, and the bottom warp handles the actions that would stick at the bottom bound (Yoshi ground pound 0xE3, Bowser forward
throw 0xE6, Kirby forward throw fall 0xE5, ShieldBreakFall 0x9F, with a 250-tic fall limit). `skip_bottom_blast_sfx` is
`grRemixBlastZoneSkipsBottomSfx` (bit 2 of the toggle): the altitude warning FGM 0x99 is not played when the bottom warp is on.

### 5.3 `Camera.asm`

| Asm scope | Port |
|---|---|
| `override_camera_type_` (0x8010DAA4, `gmCameraMakeDefaultCamera`) | `grRemixCameraDefaultStatus`: 19 stages (`MK_REMIX`, `SUBCON`, `SECTOR_Z_REMIX`, `ONETT`, `BLUE`, `MUTE`, `HTEMPLE`, `DRAGONKING_REMIX`, `DRAGONKING`, `PEACH2`, `MT_DEDEDE`, `FIRST_REMIX`, `MELRODE`, `SCUTTLE_TOWN`, `YOSHIS_ISLAND_MELEE`, `BIG_SNOWMAN`, `GYM_LEADER_CASTLE`, `N64_REMIX`, `TOADSTURNPIKE`) use the Mushroom Kingdom camera routine (`nGMCameraStatusInishie`) |
| `set_initial_coordinates_` | `gmRemixCameraSetupDefault` (`gmremixcameramode.c`); fixed-view tables match `Camera.asm` |
| `camera_adjust_` (0x800E7270, `ftMainSetStatus`) | `grRemixCameraAdjustZoomRange`: zoom range 1.75 on Venom and Game Boy Land in VS, Training and the 1P game, 1.0 elsewhere |
| `fixed_camera_boss_`, `fixed_camera_always_`, `frozen_camera_` x4, `cinematic_entry_` | `gmRemixCameraIsFixed`, `gmRemixCameraSetStatusIsFrozen`, `gmRemixCameraFreezeUpdate` (the four camera routines), `gmRemixCameraCinematicFocus` |
| `prevent_fixed_camera_pause_bonus_`, `pause_disable_control_flag_`, `unpause_camera_`, `disable_hud_` | `gmRemixCameraPauseBonusZoomAllowed`, `gmRemixCameraPauseControlEnabled`, `gmRemixCameraRestorePauseHud`, `gmRemixCameraHudHiddenOnGo` (`gmremixcamerapause.c`) |
| `zoom_to_max_` (Time Twister) | `gmCameraSetStatusMapZoom` with the stage zoom bounds |

### 5.4 `Spawn.asm`

`load_respawn_point_` (0x800FC8B0, inside `mpCollisionGetMapObjPositionID`): `grRemixSpawnRespawnPointOverride` puts the respawn of Dream Land
Beta 1 and 2 at (0, 2526) and Sector Z Remix at (6580, 3150). `load_spawn_` (Neutral Spawns, default on): `gmRemixNeutralSpawnApply` in
`gm/gmremixspawnsetup.c` (called by `port_comp_ruleset_get_spawn`, `scvsbattle.c`); the Training custom spawn is in
`sc1ptrainingremixspawn.c`.

### 5.5 `SpecialZoom.asm`

`Special Zoom (BETA)` (`entry_special_zoom`: 0 off, 1 and 2 KO settings): `gmremixspecialzoom.c`. The camera zooms in on a fighter whose hit is
a KO (settings 1 and 2) or whose attacker is in a special move (any setting), and the game runs in slow motion for 0x28 frames; the
zoom is skipped in Stamina matches. Hooks: `final_knockback_was_set` (0x801414E4, `ftCommonDamageInitDamageVars`) calls
`ftRemixSpecialZoomFinalKnockback`; `zoomed_in_camera_offset` (0x8010C9BC, `gmCameraUpdatePlayerZoom`) `ftRemixSpecialZoomCameraOffset`;
`create_zoom_camera` (0x8010DC10, `gmCameraMakeWallpaperCamera`) `ftRemixSpecialZoomMakeWallpaper`; `objects_update` (0x8000A5EC, `gcRunAll`)
`ftRemixSpecialZoomSkipsObjectsUpdate`. The KO zoom hides the Remix render groups 1 and 2 (`temp_toggle_group_display_`, which saves and
restores the display word of every object of the group): the Remix `Render.GROUP_TABLE` is `gGCCommonLinks` in the port, so
`gmRemixGroupDisplayTempSet(1, hide)` and `(2, hide)` (`gmremixgroupdisplay.c`) set and restore the GObj flags of those links.

## 6. Fighter and item leftovers

| Asm patch | Vanilla function | Port |
|---|---|---|
| `DragonKingSpecial.asm drain_flash_fix_` (0x8010E9E4) | `ifCommonPlayerDamageUpdateDigits` (ifcommon.c) | `ftRemixDragonKingDrainFlashSuppressed`: the damage digits keep their normal colour while the displayed fighter is grabbed by a Dragon King in USPGroundThrow (0xF0) or USPAirPull (0xF2) with its temp variable 1 set |
| `PianoSpecial.asm` ground and air absorb initial (0x80155278) | `ftNessSpecialLwProcAbsorb` | `ftRemixPianoDSPAbsorbRedirect` (ground or air absorb status by `fp->ga`) |
| `PianoSpecial.asm command_grab_action_fix_` (0x8014C8FC) | `ftCommonCaptureYoshiProcCapture` | `ftRemixCommandGrabCaptureAction`: captured fighter enters ThrownDK (0xB8) for Mad Piano, Thrown2 (0xBA) for Dragon King, Marina and a Kirby copying Marina, EggLayPulled (0xB1) otherwise |
| `PianoSpecial.asm absorb_behaviour_` (0x800E33AC) | `ftMainUpdateAbsorbStatWeapon` | Mad Piano heals `(s32)(damage * 0.75)` instead of x2 and adds `damage / 3` to its bonus ammo float (cap 5.0, `passive_vars` word 0x0ADC, read by the Mad Piano fireball in `wpmarioremixfireball.c`) |
| `Piano.asm extra_part_visibility_` (0x800E7220) | `ftMainSetStatus`, after `is_item_show = TRUE` | `ftRemixPianoExtraPartVisibility` (`ftremixleftovers.c`): outside Grab, GrabPull and LightItemPickup to HammerLanding (0x64 to 0x97) Mad Piano's joints 0x0E to 0x10 are hidden (`gcRemoveMObjAll`, `dl = NULL`, `modelpart_id_curr = -1`) |
| `Item.asm prevent_item_despawn_` (0x80173C04) | `itMapCheckDestroyDropped` (itmap.c) | `ftRemixLeftoverItemSkipsDespawn`: Turnip, Gem, Shuriken, Boomerang and Clan Bomb skip the despawn roll after landing |
| `Item.asm clear_items_in_1p_` | RTTF, BTT and BTP stage init | `itRemixClearActiveItems()` runs on every scene change in `scmanager.c` (a superset of those inits) |
| `samusshared.asm get_grapple_anim_gfx_struct` (0x80101E88) | `efManagerSamusGrappleBeamGlowMakeEffect` (efmanager.c) | `ftRemixSamusSharedGrappleBeamDesc` (`ftremixsamusshared2.c`): Dark Samus uses the vanilla descriptor with her own file 6 and the four offsets + 0x2970 |
| `Dedede.asm jump_multiplier_table` | `ftCommonJumpAerialMultiSetStatus` | `ftRemixDededeJumpAerialMultiplier`: 0, 0, 68, 58, 52, 0 by jumps used |
| `jigglypuffkirbyshared.asm kirby_special_struct_fix` (0x8016BEA0) | `wpKirbyCutterMakeWeapon` (wpkirbycutter.c) | `ftRemixKirbyCutterWeaponDesc` picks the weapon descriptor |
| `kirby_rock`, `kirby_rock_2`, `kirby_rock_3` (0x80161424, 0x80161374, 0x801614E4) | `ftKirbySpecialLw*` (ftkirbyspeciallw.c) | `ftRemixKirbyStoneHealthMax`, `ftRemixKirbyStoneHealthMid`, `ftRemixKirbyStoneReleaseDuration` |
| `WaddleDee.asm`, `ScuttleMinion.asm remove_bob_bomb_walk_fgm_` (0x8017798C) | `itBombHeiWalkInitVars` | the fuse FGM plays only for `nITKindBombHei`; minions that reuse the walk init stay silent |
| `linkshared.asm boomerang_crash_fix_` (0x8016D364, 0x8016D3F4, 0x8016D58C) and `kirby_boomerangfix_1/2` | `wpLinkBoomerangCheckOwnerCatch` | a caught boomerang is destroyed once, through the return value of the update; J Kirby joins the Kirby cases |
| `fireball.asm bounce_effect_`, `bounce_destroy_check`, `hit_effect_` | `wpMarioFireballProcMap`, `wpMarioFireballProcHit` | `wpRemixMarioFireballBounceEffect` (capsule and book: small smoke; egg: FGM 0x506), `wpRemixMarioFireballMinVel` (egg min velocity 20.0 when `mask_curr & 0x21 == 0`, else -1280.0), `wpRemixMarioFireballCapsuleHitGfx` (hit GFX 0x81 Dr. Mario, 0x97 Dr. Luigi) |
| `Flashbang.asm` flags | `itremixflashbang.c` | `attack_coll.can_rehit_shield = TRUE`, `coll_data.cliff_id = 0`, first joint `flags = 2` at the explosion |
| `Cloud.asm enable_flash` (0x8016F5B4) | `itProcessProcItemMain` | `itRemixKeepsFlashFlags(kind)`: Cloud (0x49) and the Bowser bomb (0x11A) keep their GObj flash flags |
| `Kirby.asm clone_power_action_swap_` (0x800E6F34) | `ftMainSetStatus`, top | `ftRemixKirbyCloneActionSwap` (`kirby/ftremixkirbyclone.c`): Kirby or J Kirby copying a Remix clone power (Ganondorf, Dr. Mario, Dr. Luigi, J Mario, Dark Samus, Young Link, Lucas, Falco, Mad Piano, Wolf, Lanky) maps the parent's action id to the clone's own; first match wins |
| `Kirby.asm clear_marth_flag_` | grounded script of J Kirby | `ftRemixJKirbyGroundedScript`: a Marth or Roy copy clears `passive_vars.kirby.copysamus_charge_level` on landing |
| `KirbyHats.asm` power flags | `ftKirbySpecialNLoseCopy`, `ftManagerInitFighter` | `ftremixkirbypower.c` (magic hat state, `taunt_loses_power`, stolen power) and `ftremixkirbyhats.c` (the hat model part); the panel hat is a model only |
| Charge GFX routines (`GFXRoutine.asm`) | | GOEMON_CHARGE 0x73 in `ftRemixGoemonNSPStartCharge`, DEDEDE_CHARGE 0x74 in `ftRemixDededeDSPAddCharge` (at the maximum level), SHEIK_CHARGE 0x6D in `ftRemixSheikNSPChargeProcUpdate` |

Roy's counter is disabled in Remix: `MarthSpecial.asm detection_patch_` has the Roy branch commented out, so
`ftRemixMarthDSPCounterDamage` only reduces damage for Marth and Roy keeps vanilla damage.

Audit ledger (every item closed): `docs/remix/patch_audit.md`, groups ST (g3), GP (g5) and SH (g6).
