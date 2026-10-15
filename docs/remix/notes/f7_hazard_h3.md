# F7 hazard group h3 (JapesPirate): notes

Status: NOT PORTED. `decomp/src/gr/grremix/grremixhazards_japespirate.c` is unchanged (returns FALSE), so the
stages below still run without hazards, as they do today. No C file was edited in this pass.

## Stage ids (grremixids.h)
- Jungle Japes: `function_table` entry `Hazards.jungle_japes_setup` (Stages.asm:3322), nGRRemixStage_JAPES = 0x86, stage file `stages/japes.asm`.
- Corneria: `Hazards.corneria_setup` (Stages.asm:3258, nGRRemixStage_CORNERIA2 = 0x46). No `stages/corneria*.asm` exists.
- Game Boy Land: `Hazards.gbland_setup` (Stages.asm:3324, nGRRemixStage_GB_LAND = 0x88). No stage file.
- Pirate Land: `Hazards.pirate_land_setup` (Stages.asm:3346, nGRRemixStage_PIRATE = 0x9E). No stage file.

## Routines and source ranges (Hazards.asm)
- 1581-1641 jungle_japes_setup: CREATE_OBJECT 0x03F2 group 1, rushing_water routine via 0x80008188, hazard struct at
  0x801313F0 (klaptrap header, stage header), spawns KlapTrap via 0x8016EA78 (item 0x3A) at klaptrap_coordinates.
- 1643-1712 klaptrap_main_: timer at 0x801313F0+8 (484 frames), random spawn (1 in 160), animation tracks 0x0908 / 0x0E20
  via 0x8000BD1C / 0x8000BD54.
- 1719-1796 klaptrap_stage_setting_: item create 0x8016E174, raw item-struct offsets (0x033E, 0x0350-0x035C, 0x0398,
  0x0110-0x0148, 0x0156, 0x01D0-0x01D4, 0x0248, 0x02D3, 0x0158) to be mapped to named ITStruct/hitbox fields.
- 1798-1812 klaptrap_blast_zone_; 1815-1860 klaptrap_room_ (room ordering fix, Mewtwo/RTTF checks).
- 1870-1980 rushing_water: y threshold 0xC47A0000 (-1000.0F), water effect.
- 1985-2025 corneria_setup; 2026-2175 great_fox_ (projectile struct, spawn locations/rotations at 2140-2175).
- 2176-2208 gbland_setup; 2209-2355 gb_music_.
- 3700-3825 pirate_land_setup; 3826-3845 pirate_land_main_; 3846-3974 pirate_land_water_;
  3976-4147 cannon_main_ (Cannon.* constants at 3672-3698); 4149-4219 cannonball_stage_setting_;
  4221-4265 cannonball_main_; 4268-4316 cannonball_explosion_; 4317-4354 cannonball data tables.

## Blockers for a faithful port
1. KlapTrap item (`src/items/KlapTrap.asm`, item 0x3A) is not ported; the item special struct is accessed by raw offsets
   (0x0084 item special struct, 0x0074 location struct). Needs named fields from `build/remixasm/refs/struct_offsets.txt`
   and an `itremixklaptrap.c/.h` with `dITRemixKlapTrapItemDesc` (not created).
2. Helpers with unverified signatures: `0x80008188` (rushing water routine registration), `0x8000BD1C` / `0x8000BD54`
   (animation track setup), `0x8016E174`, `0x8016EA78`, `Render.CREATE_OBJECT_`, `Global.get_random_int_`. Look each
   up in `build/remixasm/refs/vanilla_funcs.txt` before use.
3. Hazard state at 0x801313F0 is a fixed Remix RAM scratch area; it must become a file-local static struct (per brief),
   and the stage header pointer at 0x80131300 (+0x14) must be replaced by the stage file reference.
4. The pirate cannon and cannonball use projectile structs (`Projectile.id.PIRATELAND_CANNONBALL`) and the
   `wp*`/projectile manager; no port of that subsystem exists in decomp/src/wp for this path.
5. Corneria (Great Fox projectiles), Game Boy Land (gb_music_) and Pirate Land water have no stage asm in
   `src/stages/`; their geometry comes from the map file, verify with the stage file ids first.

## Suggested split for the lead
- Phase 1: KlapTrap item + jungle_japes_setup/rushing_water (serves the Jungle Japes stage id, see grremixids.h).
- Phase 2: Pirate Land cannon + cannonball (needs projectile port).
- Phase 3: Corneria great_fox_, Game Boy Land gb_music_.
