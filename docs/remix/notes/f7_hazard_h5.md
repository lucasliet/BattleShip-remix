# F7 hazard group h5 (RainbowTurnpikeOnett): notes

Status: not ported. `decomp/src/gr/grremix/grremixhazards_rainbowturnpikeonett.c` is unchanged (returns FALSE), so
Rainbow Road, Toad's Turnpike and Onett keep running without hazards, as before.

## Stage ids (grremixids.h)

- nGRRemixStage_ONETT = 0x49
- nGRRemixStage_RAINBOWROAD = 0xA3
- nGRRemixStage_TOADSTURNPIKE = 0xA6

Stages.asm constants (function_table is not in this group's grep): ONETT(0x49), RAINBOWROAD(0xA3), TOADSTURNPIKE(0xA6).
The setup routines are `rainbow_road_setup`, `toads_turnpike_setup`, `onett_setup` (REMIX_SYM_Hazards__* in
port/remix/remix_symbols.h: 0x8048EB30, 0x8048F340, 0x8048F674).

## Routine inventory (Hazards.asm 5653-6887)

| Routine | asm lines | Role | Blocker |
|---|---|---|---|
| rainbow_road_setup | 5653-5790ish | Chain Chomp object + sfx object | Render.CREATE_OBJECT_, DISPLAY_INIT_, STAGE_OBJECT_INIT_, REGISTER_OBJECT_ROUTINE_, 0x8000F8F4, 0x8000BED8, 0x8000DF34 |
| chain_chomp_main_ | ~5790-5990 | timed scenes, random chomp spawn | GObj routine, Global.get_random_int_, FGM 0x401 |
| sfx_main_ | ~5990-6030 | timed noise sfx | FGM play (0x800269C0) |
| chainchomp_hitbox_stage_setting_ | ~6030-6080 | projectile spawn | 0x801655C8 (projectile maker) |
| chainchomp_hitbox_main_/connect_ | ~6080-6160 | hitbox refresh, duration | projectile struct layout |
| chain_chomp_projectile_counter | ~6160-6210 | VANILLA PATCH 0xE10E0 -> 0x801666A0 | vanilla hook |
| blast_zone_destroy_prevent | ~6210-6240 | VANILLA PATCH 0xE0E90 -> 0x80166450 | vanilla hook |
| stoplight_1/2/3/go | ~6240-6380 | VANILLA PATCHES 0x8DBE8/0x8DBFC/0x8DC10/0x8DC3C -> 0x801123E8/0x801123FC/0x80112410/0x8011243C | vanilla hooks, PokemonAnnouncer.opening_comment (stoplight_2) |
| toads_turnpike_setup | ~6380-6420 | stores main routine in camera object at +0x110 | camera object (0x80131460) |
| toads_turnpike_setup_2 | ~6420-6470 | registers toads_turnpike_main (per-frame routine) | Render.register_routine, player list at 0x800466FC |
| turnpike_timeline | ~6470-6510 | 6-entry timeline data (float32 + dw) | none (data) |
| toads_turnpike_main | ~6510-6660 | light angle per player (flag 0x0040 at 0x0A88, angles 0x0A74/0x0A78) | player light struct offsets |
| onett_setup | ~6660-6700 | spawns Car item (0x3F5) with car_coordinates | 0x8016EA78 spawn stage item |
| car_main_ | ~6700-6790 | honk at random, spawn after timer | item special struct offsets (0x010C, 0x0158, 0x002C, 0x0038) |
| car_stage_setting_ | ~6790-6850 | Car item init (scale 0.75, hitbox) | 0x8016E174 create item, Item.Car.item_info_array |
| car_blast_zone_ | ~6850-6870 | hides car on blast zone | 0x8016F8C0 |

Shared hardcoded RAM used by the asm: 0x801313F0 (hazard state struct), 0x80131300 (stage header pointer, +14),
0x80131460 (global camera object), 0x800466FC (first player object), Global.match_info, Global.current_screen.

Car item source exists: `.toolchain/pkgs/smashremix-src/src/items/Car.asm`. No `decomp/src/it/itremix/itremixcar.*` yet.
Stage asm: no file named rainbow/turnpike/onett/road exists under `src/stages/`; the stage data is in the map file
addressed by the header pointer, so the setup code reads it at runtime (lead to confirm the stage file mapping).

## Phased plan (proposed)

1. Phase 1: Car item (`it/itremix/itremixcar.c/.h`, export `dITRemixCarItemDesc`), from Car.asm, plus onett_setup and
   car_main_/car_stage_setting_/car_blast_zone_. Verifiable with the item build only. Onett is served by this phase.
2. Phase 2: Toad's Turnpike timeline and light routine (self-contained math, needs Render routine registration API).
3. Phase 3: Rainbow Road Chain Chomp object, projectile and sfx; needs the projectile and render APIs and the three
   vanilla patches (projectile counter, blast zone, stoplights).
4. Wire `grRemixHazardsMakeGroundRainbowTurnpikeOnett` to return TRUE only for each phase's setup once it is ported.

## Open questions for the lead

- Which decomp helper matches `Render.register_routine` and `REGISTER_OBJECT_ROUTINE_` (gcAddGObjProcess-style)?
- The Car descriptor (`Item.Car.id`, `item_info_array`) must be registered through `tools/gen_remix_items.py`, lead only.
- ai-memory had no project entry for this repo (`default/BattleShip`, `JRickey/BattleShip`), so no project rules were loaded.
