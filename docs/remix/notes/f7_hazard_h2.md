# F7 hazard group h2 notes (partial)

## Done

- `decomp/src/gr/grcommon/grjungle.c`: `grJungleMakeGround` spawns the taru cann only for the Congo Jungle family
  (`nGRRemixStage_CONGO_JUNGLE`, `SMASHKETBALL`, `FALLS`, `CONGOJ_DL`, `CONGOJ_O`), under `#ifdef REGION_REMIX`.
  Fixes the crash in `grJungleMakeTaruCann` for stages 0x72 (SAFFRON_DL) and 0x76 (SAFFRON_O), which use the
  Congo Jungle maker (Stages.asm function table) but have no taru cann map joints.

## Not done (remaining h2 scope)

- Hazards.asm 298-382 `congo_jungle_barrel_`: the SMASHKETBALL branch (basket move, two custom barrels via `add_barrel_`)
  and the hazard_mode toggle for the standard stages. Only the spawn gate was applied.
- Hazards.asm 720-1120 `add_barrel_`, `barrel_angle_fix_`, `many_barrels_fix_`, `add_item_` (standard/stage/pokemon).
  Blocked on the Banjo barrel array patch (`barrel_array`, collision object array 0x80131180) and item spawn helpers.
- Hazards.asm 1260-1580 `peach_bumper_hitbox_`, `replenish_bowser_bombs_`, `bowser_bomb_*`, `bomb_hitstun_`,
  `track_hit_object_`: not started (grcastle.c is shared with group h1).
- Hazards.asm 9619-9655 `congo_falls_hazard`: not started.

## Helper addresses verified (build/remixasm/refs/vanilla_funcs.txt)

- 0x80009968 gcMakeGObjSPAfter, 0x80009DF4 gcAddGObjDisplay, 0x80008188 gcAddGObjProcess,
  0x80105760 grModelSetupGroundDObjs, 0x800E1D48 ftMainCheckAddGroundObstacle.
- 0x80109FD8 grJungleTaruCannCheckGetDamageKind, 0x8012EB50 dGRJungleTaruCannTransformKinds (already in grjungle.c).

## Open question

- The asm hooks `congo_jungle_barrel_` into the delay slot at 0x80109FBC, after the `jal grJungleMakeTaruCann` call at
  0x80109FB8. The `hazard_toggle(0x80109E84)` in the standard path re-calls the maker. Whether the original vanilla call
  still runs for non-jungle stages in Remix has not been confirmed with a disassembly of the Remix ROM. The gate applied
  assumes it must not.
