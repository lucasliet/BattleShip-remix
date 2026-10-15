# F7 hazard group h1: gating of the vanilla ground makers

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` (hazard_toggle, mk_clone_hazard_removal_, peach_bumper_,
yoshis_island_clouds_, planet_zebes_acid_, alt_acid_levels_, the Dreamland/Yamabuki/Hyrule/Sector/Inishie hooks).
Stage ids: `decomp/src/gr/grremix/grremixids.h`. Clone mapping: `Stages.function_table` (Stages.asm ~3185), index = stage id.

## New files (owned)

- `decomp/src/gr/grremix/grremixgating.c/.h`: `grRemixHazardToggleIsOn`, `grRemixMovementToggleIsOn`,
  `grRemixIsMushroomKingdomClone` (SMBBF 0x41, SMBO 0x42, MK_REMIX 0x9B), `grRemixStageHasPeachBumper`
  (PEACHS_CASTLE 0x00, PCASTLE_DL 0x6B, PCASTLE_O 0x6C, PCASTLE_BETA 0x96, LMAO_CASTLE 0xC9).
- `decomp/src/gr/grremix/grremixzebes.c/.h`: `grRemixGetAltAcidLevels` (Norfair 0x5C, Norfair Remix 0xA5, Zebes DL 0x74,
  Zebes O 0x79). Values are the f32 words of the asm tables, written as decimal literals.

## Vanilla patches (all under `#ifdef REGION_REMIX`)

| asm site | vanilla function (file) | applied |
|---|---|---|
| 0x80105F34 dream_land_wind_ | grPupupuUpdateWhispyStatus (grpupupu.c), Wait case | hazard toggle around grPupupuWhispyUpdateWait |
| 0x80107E4C sector_z_arwing_ | grSectorProcUpdate (grsector.c), Wait case | hazard toggle around grSectorArwingUpdateWait |
| 0x8010A960 hyrule_castle_tornadoes_ | grHyruleTwisterProcUpdate (grhyrule.c), Wait case | hazard toggle around grHyruleTwisterUpdateWait |
| 0x801099AC mushroom_kingdom_pow_ | grInishiePowerBlockProcUpdate (grinishie.c), Make case | hazard toggle around grInishiePowerBlockUpdateMake |
| 0x8010 9C1C mk_clone_hazard_removal_ | grInishieMakeGround (grinishie.c) | scale skipped for MK clones |
| 0x80109C24 mushroom_kingdom_piranha_plants_ | grInishieMakeGround | piranha skipped for MK clones and when hazards are off; pakkun_gobj cleared to NULL |
| (same) | grInishiePakkunSetWaitFighter | skips NULL handles (REMIX only); called from ftcommondokan.c on every dokan |
| 0x80108450 planet_zebes_acid_ (hazards) | grZebesMakeGround (grzebes.c) | returns NULL before any gobj when hazards are off (asm returns early before gcMakeGObjSPAfter) |
| 0x80108410 planet_zebes_acid_ (movement) | grZebesProcUpdate, Normal case | movement toggle around grZebesAcidUpdateNormal |
| 0x801081E0 alt_acid_levels_ | grZebesMakeAcid | grZebesRemixSetAltAcidLevels, inserted after gcPlayAnimAll, before acid_status is set |
| 0x8010B4B4 peach_bumper_ | grCastleMakeGround (grcastle.c) | grCastleInitAll only when stage has bumper and hazard toggle is on |
| 0x8010B174 saffron_city_pokemon_ | grYamabukiGateProcUpdate (gryamabuki.c), Wait case | hazard toggle around both wait-case calls |

## Skipped (not in this group or not gr-ground)

- Congo barrel 0x80109FBC: group h2, grjungle.c not touched.
- ai_extend_hyrule_tornado_check (0x80134D50), planet_zebes_acid_._ai_fix (0x801374E8), mk_remix_hardcoding_9 (0x80137464): AI.
- mk_remix_hardcoding_8 (0x8017C180), peach_bumper_hitbox_ (0x8017D710), mk_remix_hardcoding_5-7 (0x8017D...): outside gr.
- mk_remix_hardcoding_1-4 (0x80109514, 0x80109574, 0x80109250, 0x80109BEC): gr files, but not in this brief. Not done.
- dream_greens_hardcoding_animation_4 (0x80106460, grPupupuUpdateGObjAnims): needs DREAM_GREENS_OFFSETS
  (`stages/dream_greens_offsets.bin`), not ported.
- dreamland_downtime_*, dreamland_wind_speed_*, saffron_city_pokemon_rate*, yoshis_island_cloud_anims_: they read
  Toggles.entry_whispy_mode / entry_saffron_poke_rate / entry_yi_clouds (Remix settings menu, not wired). With the
  defaults they run the vanilla path, so nothing changed.
- yoshis_island_clouds_ (0x80108C88, grYosterMakeGround): hazards-off path disables clipping of three cloud structs and
  skips the rest of the function. Not ported (needs the cloud struct layout). Vanilla behavior with hazards off.
- platform_movement_ (0x801055A8): the vanilla function resolved by vanilla_funcs.txt is grDisplayMakeGeometryLayer,
  which does not look like a platform setup. Not ported; needs a check of 0x80008188 and the patch site.

## REMIX-TODO markers

- `REMIX-TODO(camera)` in grZebesMakeGround: the asm copies 0x8013EB1C to 0x801313FC before the hazard check. No decomp
  name for these addresses.
- `REMIX-TODO(items)` in grCastleMakeGround: Bowser's Stadium RTTF bombs (`add_item_`, BOWSER_BOMB, replenish_bowser_bombs_)
  are not spawned.

## Uncertainties

- `scene_curr` is taken as Global.current_screen. The enum index is 22 (nSCKindVSBattle = 0x16) and 54
  (nSCKind1PTrainingMode = 0x36), which matches the asm. Verify in the game.
- Hazards.asm does not list stage 155 (MK_REMIX) in mk_clone_hazard_removal_. It is gated because the lead reports a crash
  in grInishieMakeScale for it. Check that a real MK_REMIX map has no scale platforms.
- asm alt_acid_levels_ writes the table for the four stages and never restores it. The port does the same (faithful). A
  Zebes stage played after Norfair keeps the Norfair levels.
- Peach's Castle bumper gating also skips grCastleInitAll for the non-bumper Peach clones (Kitchen 0x47, Big Blue 0x48,
  Bowser's Stadium, World 1-1, Flat Zone II, Bowser's Keep, ...), which means no castle map joint animations for them. This
  follows peach_bumper_. Check those stages visually.
- The asm's Zebes early exit returns a stale v0. The port returns NULL; the caller ignores the value.
- Insertion point of alt_acid_levels_ is approximated (see table above).
