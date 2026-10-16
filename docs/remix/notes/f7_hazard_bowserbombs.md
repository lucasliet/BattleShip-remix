# F7 hazard notes: Bowser's Stadium bombs (BOWSERB)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 1309-1580 (replenish_bowser_bombs_, bowser_bomb_flash_main,
bowser_bomb_explosion_, bowser_bomb_air_collision_, bomb_hitstun_, track_hit_object_), plus the spawn code of
peach_bumper_ (lines 537-600) and add_item_ (line 938).

## Files

- `decomp/src/gr/grremix/grremixbowserbombs.c/.h` (PORT): respawn process, flash status, explosion, hit tracking, hitstun.
- Vanilla edits under `#ifdef REGION_REMIX`:
  - `decomp/src/it/itground/ittarubomb.c`: itTaruBombCommonProcHit (0x80184DC4) explosion patch; itTaruBombFallProcMap
    (0x80184EDC) landing FGM patch.
  - `decomp/src/ft/ftcommon/ftcommondamage.c`: ftCommonDamageInitDamageVars (0x80140EE4) hitstun patch (asm 0x80140F44).
  - `decomp/src/ft/ftmain.c`: item hit branch of ftMainProcessHitCollisionStatsMain (asm track_hit_object_ 0x800E3F14).

## Patches

| asm | vanilla function | status |
|-----|------------------|--------|
| replenish_bowser_bombs_ | new process (grRemixBowserBombsRespawnProcUpdate) | applied, started by grRemixBowserBombsStart |
| bowser_bomb_flash_main | status 0 proc update of the bomb (grRemixBowserBombsFlashProcUpdate) | applied |
| bowser_bomb_explosion_ (0xFF810 / 0x80184DD0) | itTaruBombCommonProcHit | applied: FGM 1, respawn flag, skips shrapnel |
| bowser_bomb_air_collision_ (0xFF9C4 / 0x80184F84) | itTaruBombFallProcMap | applied: FGM 0xC2 on BOWSERB |
| bomb_hitstun_ (0xBB984 / 0x80140F44) | ftCommonDamageInitDamageVars | applied: stun * 9 / (gravity + 4) |
| track_hit_object_ (0x5F714 / 0x800E3F14) | ftMainProcessHitCollisionStatsMain | applied in the item branch only |
| peach_bumper_ bomb spawn (lines 575-592) | grCastleMakeGround (other worker) | NOT wired here: call grRemixBowserBombsStart() on BOWSERB with hazards on |

## Uncertainties

- Shrapnel skip: the asm jumps to 0x80184DE8 and the port assumes that is the `itTaruBombExplodeMakeEffectGotoSetStatus` call.
- track_hit_object_ reads `sFTMainHitLogs[0].attacker_gobj` literally in the asm (line 1559-1565). The port records the
  item that hit the player in the item branch instead; fighter hits do not clear it.
- The asm hitbox/flash is the item main routine (0x378). The port uses a status desc copy of status 0 with only the update
  replaced; the map, hit and damage procs of the status still run as in vanilla.
- A bomb that is absorbed (Marina, asm checks position struct 0x74 == 0) is not detected: the port only uses the explosion flag.
- add_item_ failure: the asm ends the respawn object; the port retries after RESPAWN_TIME.

## REMIX-TODO

- `REMIX-TODO(items)` in grremixbowserbombs.c spawn: hover bottom ECB (-416.0F) and BOWSER_BOMB_HEADER file load of add_item_
  are not ported.

## Test

Stage id `nGRRemixStage_BOWSERB` = 0x43 (`SSB64_VS_STAGE=67`). Hazards must be on (Toggles.entry_hazard_mode).
