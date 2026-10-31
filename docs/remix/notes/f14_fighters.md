# F14 notes: fighters scope (non-AI REMIX-TODO lines)

Scope: `decomp/src/ft/**` (ftremix/**, ftcommon/ftcommonitemshoot.c, ftcommon/ftcommonthrow.c, ftchar/ftness/ftnessspecialn.c,
ftmain.c, ftdisplaymain.c) and `decomp/src/ef/efmanager.c`. Source: `.toolchain/pkgs/smashremix-src`.

## Implemented

| Location | Asm | What changed |
|---|---|---|
| ftchar/ftness/ftnessspecialn.c `ftNessSpecialNProcAccessory` | Size.asm ness.nsp.adjust_pkfire_position_ (0x801539B4) | PK Fire spawn X offset is `100 * lr * mult`, Y is `180 * mult` (REGION_REMIX only). |
| ftremix/ftremixchargesmash.c, ftremixsizebody.c, ftmain.c `FTMotionEventSetAttackCollDamage` | Size.asm fix_damage_ charged smash block (asm 477-527) | New `ftRemixChargeSmashScaleDamage`: damage times `1 + charge_time * 0.4 / 60` (0x3BDA740E, verified), rounded, applied before the Size factor as in the asm. Rounding moved to shared `ftRemixRoundDamage`. |
| ftremix/ftremixzcancel.c `ftRemixZCancelPunishMode` (BURY) | ZCancel.asm `_bury` calls Damage.bury_initial_ with a1 = 1 | Calls `ftRemixDamageBuryInitial` (asm `bury_initial_` ignores a1; port matches it). |
| ftremix/ftremixzcancel.c `ftRemixZCancelTrip`, ftremix/ftremixtripping.c | ZCancel.asm `_trip` (`sb at` = player index into Tripping.asm player_trip_flag) | New `ftRemixTrippingStoreZCancelFlag(player)` stores the port index, not TRUE, as the asm does. Ports 1 to 3 count as tripped, port 0 is cleared. |
| ftremix/ftremixstartitem.c hammer pickup | Item.asm item pickup (0x800E7AFC after 0x800F3938) | Calls `ftParamTryPlayItemMusic(0x2D)` after `ftHammerSetStatusHammerWait`. |

## Skipped (tag kept, reason given)

| Location | Subsystem missing or unverified |
|---|---|
| ftdisplaymain.c `ftDisplayMainDrawAll` (Skeleton) | Skeleton.asm register meaning not decoded (`skeleton_mode_1_` / `_2_`). |
| ftremix/ftremixsizemove.c `ftRemixSizeAerialJumpHeight` (Pwing) | Needs `Pwing.pwing_jump_flag` accessor from `it/itremix/itremixpwing.c` (`jump_state != None`). That file is outside this scope. |
| ftremix/ftremixsizefx.c header (Size) | Yoshi egg, Samus gun, Kirby USP and Falcon kick descriptors are not mapped to port descriptors. |
| ftremix/ftremixentry.c:455 (Size) | Size.asm entry gfx_ scope not ported. |
| ftremix/ftremixshield.c (Shield) | Costume colour needs `Character.costume_shield_color`; egg colour not ported. |
| ftremix/banjo/ftremixbanjousp.c:140 (Size) | Wall bounce GFX (save_player_struct_, atan2 0x8001863C) not ported. |
| ftremix/crash/ftremixcrashdsp.c:76 (Size) | Dig GFX Size.multiplier_table scale not verified against asm in this pass. |
| ftremix/crash/ftremixcrashdsp.c:330, 347, 371 (animation) | track_dig_wait/turn/end: 0x8000BD1C has no decomp name; not verified. |
| ftremix/crash/ftremixcrashnsp.c:86 (GFX) | create_spin_gfx_ / setup_spin_gfx_: GFX subsystem not ported. |
| ftremix/crash/ftremixcrashnsp.c:93 (Item) | spin_detect_item_ and item fling: item subsystem not ported. |
| ftremix/goemon/ftremixgoemonusp.c:93 (Size) | Size.item.render_routine_ on the cloud: item render subsystem not ported. |
| ftremix/lanky/ftremixlankydsp.c:594 (gobj-store) | Store to GObj 0x1BE is outside the port's fighter state. |
| ftremix/marina/ftremixmarinansp.c:302 (throw_turn) | RAM words 0x8018C230..0x8018C23C not in baserom linear map, cannot verify. |
| ftremix/sheik/ftremixsheiknsp.c:352 (GFXRoutine) | GFXRoutine.SHEIK_CHARGE write: GFX subsystem not ported. |
| ftremix/dragonking/ftremixdragonkingusp.c:203 (Item) | Item.CloakingDevice cloaked_players: reader exists in it/itremix/itremixcloakingdevice.c but no accessor; outside scope. |
| ftremix/dragonking/ftremixdragonkingnsp.c:281 (lag) | Stores to 0x800F9C8C and 0x800F9CC4 have no decomp name. |
| ftremix/younglink/ftremixyounglink.c:78, 121, 464 (ITStruct) | Bombchu FGM handle at item special struct 0x1D0/0x1D4 overlays `attack_coll.attack_pos`; `ITFighterItemVarsLinkBomb` has no field. Needs `decomp/src/it/ittypes.h` (outside scope). |
| ftremix/ftremixhitbox.c:18 (singleplayer) | Remix 1P mode flag (`SinglePlayerModes.singleplayer_mode_flag`) not ported. |
| ftremix/ftremixhitbox.c:22 (hitbox display) | Debug display patches (projectile_, grab_immunity_and_armor_frames_, ...) not ported. |
| ftremix/ftremixstaminaaction.c:148 (stamina-gigabowser) | 1P Giga Bowser HP branch belongs to ROADMAP T6.2. |
| ftremix/ftremixdpad.c:137 (DpadControl) | Partial stick values need the dpad_move_timer state (DpadFunctions.asm); not ported. |
| ftremix/ftremixzcancel.c:63 (1P final boss) | Needs the "match off" byte at 0x80131580, which the port maps to `gIFCommonPlayerInterface` (a struct); not confirmed. |
| ftremix/ftremixzcancel.c:74 (ZCancel.setup_) | Yoshi egg file 0x153 is not ported. |
| ftremix/ftremixzcancel.c:140 (HowToPlay) | Current screen id 3 check not verified against the port's screen global. |
| ftremix/ftremixzcancel.c:157 (VsStats) | `mnremixvsstats` has special-move counters only, no zcancel success/miss counters. Outside scope (`mn/`). |
| ftremix/ftremixwalltech.c:39, 48 (VsStats) | Same: no wall tech tracker counters in `mnremixvsstats`. Outside scope (`mn/`). |
| ftcommon/ftcommonthrow.c:107 (PokemonAnnouncer) | Bowser and Giga Bowser forward throw: seismic announcement and action 0xE5. Not ported: the asm's Kirby branch `at` register (vanilla or Remix id) and the numeric value of `nFTKirbyStatusThrowF` are not verified. The announcer call exists (`nIFRemixAnnouncerCallSeismicToss`). |
| ftcommon/ftcommonitemshoot.c:64 (projectile) | GoldenGun.asm projectile patches blocked by the projectile subsystem. |
| ef/efmanager.c:4315 (Size, Pikachu DSP) | `efManagerThunderAmpMakeEffect(Vec3f *pos)` takes no player; needs a signature change plus the caller. |
| ef/efmanager.c:5102 (Size, Ness DSP) | Per-frame update wrapper: the descriptor has no per-frame hook in the port. |
| ef/efmanager.c:5260 (Size, Ness USP) | Same as above. |
| ftremix/ftremixdamage.c header (Damage) | Stun, bury and flashbang action changes: the bury part is now wired through ZCancel; other stun paths not re-verified in this pass. |

## Verification

- `scripts/remix/syntax-check.sh` returned `syntax-check OK` for every touched `.c` file (ftnessspecialn.c, ftremixsizebody.c,
  ftremixchargesmash.c, ftmain.c, ftremixzcancel.c, ftremixtripping.c, ftremixstartitem.c). Warnings in ftmain.c are pre-existing.
- Not built, not run in game (lead to wire and test).

## In-game checks for the lead

- Ness NSP PK Fire spawn with a Size multiplier above 1 (the spawn should move out with size).
- Charged smash hit with Charge Smashes on: damage must rise with charge time, then the Size factor applies.
- Z-cancel trip punishment on port 0 (no trip) and port 1 (trip).
- Hammer pickup plays the hammer midi.
