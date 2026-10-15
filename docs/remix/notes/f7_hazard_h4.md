# F7 hazard group h4 (CasinoMadness): status notes

Status: not ported. `decomp/src/gr/grremix/grremixhazards_casinomadness.c` is unchanged and still returns FALSE, so
Casino Night Zone and Metallic Madness (their Stages.asm `add_stage` entries) run without hazards, as they do today.

Source range: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 4355-5653.

## Routines and decision

| Remix label | Lines | Decision | Reason |
|---|---|---|---|
| sonic_btp_setup | 4355-4396 | skipped | 1P mode only (BTP Sonic board), not in this group's stage set |
| casino_night_setup | 4400-4432 | blocked | spawns Remix item 0x17 via add_item_ and rewrites the bumper display list through RAM globals 0x80131300 / 0x13F0 / 0x13F8 |
| casino_night_side_bumper_bounce | 4436-4476 | blocked | vanilla patch at 0x80141B88 (REGION_REMIX) gated on stage id; needs the Casino stage check and the bumper surface ids (surface 0x16+) |
| casino_night_side_bumper_knockback | 4480-4572 | blocked | vanilla patch at 0x800E5C40; same stage gate; needs the player collision bitfields (0x00CC/0x00CE) mapped to ftcommon fields |
| metallic_madness_setup | 4576-4641 | blocked | reads the stage header through RAM 0x80131300 (+0xE0 Robot Bee file, +0x130 laser hitbox file); the port has no accessor for stage header file pointers |
| robotbee_coordinates / bee_factory_ | 4643-4732 | blocked | spawns item 0x3B through spawn helper 0x8016EA78 and sets animation tracks 0x8000BD1C / 0x8000BD54 from the Robot Bee file; file pointer needs the stage header accessor above |
| shrink_timer_ | 4734-4806 | blocked | writes Size.multiplier_table / Size.match_state_table; the Size subsystem has no port (REMIX-TODO(Size)) |
| robot_bee_stage_setting_ | 4815-4886 | blocked | item 0x3B ITDesc needs the file pointer from the stage header (0x801313F4 "Japes header" in RAM) and the Robot Bee state table |
| robot_bee_main_ / destroy_ / blast_zone_ | 4890-4971 | blocked | item states (ITRemixItemDesc) depend on the file and on robot_bee_laser_ |
| robot_bee_laser_ / stage_setting_ / main_ / collision_ / hit_ / reflect_ | 4975-5277 | blocked | Remix projectile METALLICMADNESS_BEELASER (Projectile.asm) is not ported; hit routine needs Size (shrink) and Item.SuperMushroom.player_shrooms |
| projectile struct and hitbox pointer | 5279-5297 | blocked | same as above |
| acidrain block (commented out in Hazards.asm 5301-5649) | 5301-5649 | not ported | commented out in the source; not active in Remix |

## Item

Item 0x3B RobotBee (`items/RobotBee.asm`, 54 lines) is not ported. `decomp/src/it/itremix/` has no `itremixrobotbee.*`. The
item's file is the stage header file at RAM 0x801313F4 (`FILE_OFFSET` 0xE0), and its state routines are the blocked ones above.

## Blockers the lead must resolve before this group can be ported

1. A stage header accessor that returns the Robot Bee file, laser hitbox file and the Casino bumper file (replaces the
   hardcoded RAM 0x80131300 / 0x801313F0 / 0x801313F4 reads of the asm).
2. A port of the Size subsystem (`Size.multiplier_table`, `Size.match_state_table`, `Size.state.GIANT`) for shrink_timer_
   and robot_bee_laser_hit_.
3. A port of the Remix projectile METALLICMADNESS_BEELASER (`Projectile.asm`) on top of `wpManagerMakeWeapon` (0x801655C8).
4. Vanilla patches to apply under `#ifdef REGION_REMIX` once the above exist: 0x80141B88 (side bumper bounce),
   0x800E5C40 (side bumper knockback, returns to 0x800E5C48 / 0x800E5C64 / 0x800E5D18).

## Test

No test was run. The stage ids to try are the Casino Night Zone and Metallic Madness entries of `Stages.asm`
(`SSB64_VS_STAGE=<decimal>`), but they render and play without hazards until the above lands.
