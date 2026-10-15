# F7 hazard: Spawned Fear (stage id 0xC3, SSB64_VS_STAGE=195)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` 8449-9255, `src/items/Cacodemon.asm`.

## Files

- `decomp/src/gr/grremix/grremixhazards_spawnedfear.c`: dispatch, TRUE for `REMIX_SYM_Hazards__spawned_fear_acid_setup_`.
- `decomp/src/gr/grremix/grremixspawnedfear.c/.h`: setup (hazard check, acid routine, stage file tokens, factory), acid check.
- `decomp/src/gr/grremix/grremixcacodemonfactory.c/.h`: `cacodemon_factory_` (timer, roll, one-at-a-time lock).
- `decomp/src/gr/grremix/grremixthunderball.c/.h`: `thunderball_*`, `check_target_`, `dWPRemixCacodemonThunderballWeaponDesc`.
- `decomp/src/it/itremix/itremixcacodemon.c/.h`: `cacodemon_*` item, exports `dITRemixCacodemonItemDesc` (kind 0x3F).

Registration of the item in `itremixitems.c` is the lead's step (`tools/gen_remix_items.py`).

## Ported routines

- `spawned_fear_acid_setup_`: ported. Hazard check through `grRemixHazardsAreOn()`.
- `spawned_fear_acid_main_`: loop over fighters, KO at 666% when not starred. Poison and overlay parts are REMIX-TODO.
- `cacodemon_factory_`: ported. Roll 80 of 1500 per frame after frame 1500, forced at 3000.
- `cacodemon_stage_setting_`, `cacodemon_main_`, `cacodemon_destroy_`, `cacodemon_blast_zone_`: ported.
- `cacodemon_thunderball_`, `thunderball_stage_setting`, `check_target_`: ported.
- `thunderball_main_`, `thunderball_collision_`, `thunderball_hit_`: ported.

## Skipped or approximated

- REMIX-TODO(Poison): `Poison.poisoned_players` entry for the acid (60 frames, 3 damage). No port.
- REMIX-TODO(GFX): `GFXRoutine.DOOM_ACID` while poisoned.
- REMIX-TODO(overlay): clearing of the overlay routine fields before the KO.
- REMIX-TODO(anim): the two Cacodemon animation tracks (vanilla 0x8000BD1C, 0x8000BD54). No `gcAddAnim` mapping was done.
- REMIX-TODO(hit records): reset of the four attack-record slots (ITStruct 0x224..0x243) on the loop turns.
- REMIX-TODO(unnamed fields): item 0x158 clanking bits, 0x1D4, 0x35C, 0x2D3 bit 0x08, 0x11C knockback effect.
- REMIX-TODO(facing) in the Thunderball collision: smoke effect gets the facing as `lr`, as the Mario fireball does.
- REMIX-TODO(reflector/absorb): Thunderball reflector and absorb procs are NULL (vanilla 0x80168748, 0x80168964).

## Uncertainties for the lead

1. Item file layout. The asm reads the stage header at +0xE0 both as the Cacodemon file pointer and as the item footer offset. The port sets the item file top to `PORT_RESOLVE(header+0xE0) - 0x7F8` and reads attributes at `top + 0xE0`. Check the Cacodemon renders and its hitbox size is 125 on the first run.
2. Thunderball file: `PORT_RESOLVE(header+0x130)` is the WPDesc file base, with attributes at offset 0.
3. Hazard-mode sense: the asm branches to the end when `Toggles.entry_hazard_mode & 1` is set, which contradicts its own comment. The port follows the brief and returns when `grRemixHazardsAreOn()` is FALSE.
4. Projectile kind 0x2B for `Projectile.id.CACODEMON_THUNDERBALL`, derived from the add order in `Projectile.asm` (SONIC_SPRING = 0x20, 12th entry). Verify against the Remix symbol or constant output. The port WP kind tables may also need an entry for it.
5. Training screen: the asm compares `Global.current_screen` with 0x36. The port uses `nSCKind1PTrainingMode` from `sc/scdef.h`. Check that the value matches.
6. Match clock: `gSCManagerBattleState->time_passed` stands in for `Global.match_info` time elapsed (0x18).
7. The Thunderball never expires. The asm ignores the result of the lifetime routine, and its explosion-flag branch is never taken (the flag at 0x29C is cleared at spawn and never set).
8. RNG: the asm draws the direction twice and keeps the second value. The port draws once. The sequence of rolls differs from the asm but the distribution does not.
9. `grRemixCacodemonFactoryAllowSpawn` runs on destroy and blast zone, as the asm clears the lock in both.
