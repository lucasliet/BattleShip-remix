# F7 hazard: Jungle Japes (nGRRemixStage_JAPES = 0x86) and KlapTrap item (0x3A)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` 1581-1985 (jungle_japes_setup, klaptrap_*, rushing_water),
`src/items/KlapTrap.asm`, `src/stages/japes.asm` (sprite data only, no hazard geometry).

## Files written
- `decomp/src/gr/grremix/grremixhazards_japes.c`: dispatch, returns TRUE for `REMIX_SYM_Hazards__jungle_japes_setup`.
- `decomp/src/gr/grremix/grremixjapes.c/.h`: `grRemixJapesMakeGround()` (setup) and rushing water process.
- `decomp/src/it/itremix/itremixklaptrap.c/.h`: `dITRemixKlapTrapItemDesc` (kind 0x3A), `itRemixKlapTrapMakeItem`,
  `itRemixKlapTrapSetStageFile`, `itRemixKlapTrapBlastZone` (not registered).
- Not touched: `itremixitems.c` (generated; the lead must run `tools/gen_remix_items.py` so it lists the desc).

## Ported
- jungle_japes_setup: hazard gate (`grRemixHazardsAreOn`), water process (`gcMakeGObjSPAfter` + `gcAddGObjProcess`,
  kind = nGCProcessKindFunc, priority 4), stage file binding, KlapTrap spawn via `itManagerMakeItemSetupCommon`.
- klaptrap_main_ (as the item proc_update): arms the hitbox the frame after a spawn; timer; 1-in-160 roll (`syUtilsRandIntRange`);
  forced spawn when the timer equals 600; wait of 484 frames.
- klaptrap_stage_setting_: item creation, spawn position, hitbox (damage 20, angle 0x270, slash element 3,
  FGM 0x105, bkb/kbs 0x100), hurtbox off (`damage_coll.hitstatus = 0`).
- rushing_water: per fighter in `gGCCommonLinks[nGCCommonLinkIDFighter]`; below y = -1000 applies the splash
  FGM 0x3D1 on entry (not during the match intro, not in training), then pull `x -= 90 - (x - base) * 0.0006`
  when `status_id >= nFTCommonStatusWait` and movement is on.
- klaptrap_blast_zone_: hitbox off, x back to the spawn point. Bomb-path tail not ported (see TODO).

## Skipped / REMIX-TODO
1. `REMIX-TODO(vanilla 0x8000BD1C, 0x8000BD54)`: the two animation track setups on spawn (track offsets 0x0908 and 0x0E20
   of the KlapTrap file, base at the stage file). Not in `vanilla_funcs.txt` (the nearest entry is unrelated).
2. `REMIX-TODO(vanilla 0x8016F8C0)`: blast-zone tail (bomb/grenade destroy path) and the GObj 0x38 hide. Not in the refs
   (nearest is itProcessProcItemMain at 0x8016F534, not the containing function). Consequence: `itRemixKlapTrapBlastZone` is not
   in the item state, so the item has no blast zone callback yet (ITStruct 0x398 slot not wired).
3. ITStruct raw fields with no name in `ittypes.h`: 0x33E (u16 0xB4, "pirhana" flag), 0x158 bit 0x10 (unknown bitflag),
   0x2D3 bit 0x08, 0x1D0 / 0x1D4 (hitbox refresh timer and collision flag), 0x398 (blast callback slot). Each is a REMIX-TODO
   in `itremixklaptrap.c`.
4. GObj 0x38 visibility (`sw r0/at, 0x0038(gobj)`), hidden at spawn and shown on the armed frame. REMIX-TODO in the item.
5. ITDesc word 0x0C = 0x12300000 (asm "possible argument"): not mapped; transform kind is a placeholder (REMIX-TODO).
6. KlapTrap explosion state (asm state 2, main 0x8017D298 = `itPakkunAppearProcUpdate`): the status table is not
   wired. The table is needed only by the unported blast-zone tail.
7. Map data + 0x08 timer clear (asm `sw r0, 0x0008(t1)` where t1 = 0x80131300 value): the item keeps its own timer, so the
   map-data slot is not written. Unverified whether anything reads it.

## Assumptions to verify
- Stage header: asm reads `*0x80131300 - 0x14` as the header. The port uses `gMPCollisionGroundData - 0x14`. The item footer
  is at header + 0xE0 (FILE_OFFSET). Verify with a Japes run that the KlapTrap attributes resolve.
- `f20` in rushing_water is never set in the routine. The port assumes 0 (`GRREMIX_JAPES_WATER_BASE_X`).
- Fighter position: asm reads the fighter location struct (player + 0x78); the port reads the fighter DObj translate, as
  `grJungle` does for fighter positions.
- Training check: asm compares `Global.current_screen` with 0x36; the port uses `nSCKind1PTrainingMode`. Unverified that 0x36 is that scene.
- Spawn object id: asm passes 0x3F5 as the parent of the KlapTrap (`a0`). The port passes NULL, as the asm comment of
  klaptrap_stage_setting_ says ("no associated object").
- klaptrap_stage_setting_ writes the initial x into the DObj y slot (asm `sw t8, 0x0020(t9)`, with t8 = x). The port writes
  the full position. Coordinates are zero (`klaptrap_coordinates`), so the difference has no effect today.
- Stage item spawn: `itManagerMakeItemSetupCommon` is called with a NULL parent, the item spawns from `itRemixMakeItem`
  (ids >= 0x2D).

## Vanilla patch not applied (lead must decide)
`klaptrap_room_` patches vanilla at 0xE8C1C (RAM 0x8016E1DC, inside `itManagerMakeItem` area of `decomp/src/it/itmanager.c`).
It picks the room for the Remix items KlapTrap, Car, RobotBee and Cacodemon. Not applied: the vanilla function is in the
item manager, which other workers edit. The asm sets a2 (room) to 0x0B by default, 0x04 for KlapTrap, RobotBee and Cacodemon,
0x12 for Car, and special-cases Marina's, Mewtwo's and RTTF stages (RTTF bomb ram 0x8013141C). Apply under `#ifdef REGION_REMIX`
if KlapTrap must be placed in its room.

## Test
- Stage id: `SSB64_VS_STAGE=134` (0x86). Expect the rushing water splash when a fighter falls below y = -1000, and a
  KlapTrap appearing after about 484 frames (forced spawn at 600 frames).
- Known gap: the KlapTrap does not destroy on a blast zone yet (see 2), and the room placement needs the patch above.
