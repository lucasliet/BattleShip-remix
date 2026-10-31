# F14 items_fx: REMIX-TODO closure (ROADMAP T2.11)

Scope: `decomp/src/it/itremix/itremix{blueshell,franklinbadge,flashbang,goldengun,cloud,basketball}.c` plus the new
`itremixblueshellrules.{c,h}`. Asm source: `.toolchain/pkgs/smashremix-src/src/items/{BlueShell,FranklinBadge,Flashbang,GoldenGun,Cloud,Basketball}.asm`.

## Status of each original TODO

| File:line (after edit) | Item | State | Why |
|---|---|---|---|
| itremixblueshell.c (grounded gfx) | pink smoke, `custom_smoke_` script 0x8A | deviation | See DEVIATION row below. |
| itremixblueshell.c (grounded_active_) | homing toward player in first (`target_player_`) | skipped | Caches the player in first in ITStruct 0x180 (asm lines 1367, 1373 for the homing, 429 and 432 for the throw seed). The port has no named field for that slot. |
| itremixblueshell.c (idle hurtbox) | `get_player_in_first_` call | closed, no effect | Asm line 1336 discards the result. |
| itremixfranklinbadge.c | reflect dispatch | closed, already wired | `ftmain.c:4434` calls `ftRemixReflectRunCustom` (`ftremixreflect.c`), which calls `itRemixFranklinBadgeReflectInitial`. |
| itremixfranklinbadge.c | `reflect_gfx_` | implemented | Particle script 5 of `gEFManagerParticleBankID`, position from the torso joint (`fp->joints[6]`, particle `pos`). |
| itremixflashbang.c | `hit_gfx_` | implemented | Calls the shared `itRemixBlueShellHitGfx` (exported from itremixblueshell.c), after the FGM as in Flashbang.asm line 636. |
| itremixgoldengun.c | tilt floats 0x8018CD00 / 0x8018CD04 | skipped | See Skipped section. |
| itremixcloud.c | Size | skipped | See Skipped section. |
| itremixcloud.c | blast-zone proc | closed, bare return | Cloud.asm line 282 is a bare return, so the empty body matches. |
| itremixcloud.c | uncertain DObj read | marked UNCERTAIN | Comment only. |
| itremixbasketball.c | palette swap | skipped | See Skipped section. |
| itremixbasketball.c | barrel entry check | skipped | See Skipped section. |

## DEVIATION rows

| Where | Asm | Port | Visible effect |
|---|---|---|---|
| itremixblueshell.c, `itRemixBlueShellGroundedGfx` | BlueShell.asm `custom_smoke_` (line 955). Grounded gfx (line 910) passes `a2 = 0x4F800000`, so the smoke branch always runs. It calls `lbParticleMakeScriptID` (0x800CE9E8) with script 0x8A and bank `gEFManagerParticleBankID \| 8`, then sets position and scale (`lbParticleAddTransformForStruct`). | The vanilla dust effect `efManagerDustLightMakeEffect` is used. | The smoke colour and look differ from the Remix pink smoke. Porting the particle call directly is the follow-up. |

## Skipped (with reasons)

- **Golden gun tilt** (`itremixgoldengun.c`): GoldenGun.asm line 83 loads `lwc1 f4, 0xcd00(at)` with `at = 0x8019`, which is RAM 0x8018CD00, and line 92 loads 0x8018CD04. These are vanilla item data, 0x340 bytes below `gITManagerCommonData` (0x8018D040). The Remix RAM image is based at 0x80400000 (`port/remix/remix_ram_image.cpp`), so `remix_ram_image_read_word` cannot reach them. The asm has no immediate for them, and the decomp does not name them. The store is not ported, and the RNG advance is kept.
- **Cloud Size** (`itremixcloud.c`): `apply_size_to_animation_` (0x8003DE5C) needs the animation command interpreter. `render_routine_` (ECB from holder) needs the item render wiring. The helpers in `ftremixsize.c` (`ftRemixSizeApplyTopJointAndEcb`, `ftRemixSizeScaleValue`) scale the fighter top joint and ECB, not an item, so they do not fit.
- **Basketball palette** (`itremixbasketball.c`): Basketball.asm lines 139-145 write a palette address into the display list of joint 0 (`dl + 0x5C`) for a soccer ball. The soccer palette is `dl - 0x150` and the golden soccer palette is `dl - 0x120`. The port has no map from those addresses to a `palette_id`, so the palette is not swapped.
- **Basketball barrel entry** (`itremixbasketball.c`): `check_basketball_barrel_collision_` (Basketball.asm line 909) jumps to 0x8010A018 inside the vanilla `grJungleTaruCannCheckGetDamageKind` (0x80109FD8, `grjungle.c`) with the ball as its fighter argument. The barrel list is `sFTMainGroundObstacles` (RAM 0x80131180, `ftmain.c`). The result for a ball is not verified, so the port is not written.

## Helpers added for the lead

- `itRemixBlueShellGetPlayerInFirst()` (exported from `itremixblueshell.h`): the port of `get_player_in_first_`. It reads `gSCManagerBattleState` (asm `match_info`): `game_rules` at offset 3, `players[i].score` (time, asm 0x14), `players[i].stock_damage_all` (Stamina value and tie percent, asm 0x4C), `players[i].stock_count` (stock, asm 0x0B), and `players[i].fighter_gobj` (asm 0x58). It returns the fighter of the first port, or NULL.
- `itremixblueshellrules.{c,h}`: pure ints-only rules `itRemixBlueShellRulesFirstPort` and `itRemixBlueShellRulesIsStamina`. These are ready for a headless unit test under `tests/remix` (the lead adds it).
- `itRemixBlueShellHitGfx` is no longer static and is declared in `itremixblueshell.h`.

## Lead must wire or test

1. Homing (`grounded_active_` / `target_player_`): add a named ITStruct slot for the asm 0x180 cache (N64 offset 0x180, no shift), then port `target_player_` and the seed call in `throw_initial_` (asm line 429). Call `itRemixBlueShellGetPlayerInFirst` for the seed and, in `target_player_` (asm lines 1363-1373), every 8 frames when the gfx timer equals 7. Then apply `itRShellSpinUpdateFollowPlayer(item, player)` when the player is non-NULL. The asm's squared-distance code is dead: its result is never read.
2. Test in game: Blue Shell with 3 CPUs in Time, Stock and Stamina rules. Check which player is chosen as first when homing is wired.
3. Test Franklin Badge reflect: the green circle must appear on the torso when the badge reflects.
4. Test Flashbang explosion: the white spark must appear at the bomb position.

## UNCERTAIN

- `get_player_in_first_` (BlueShell.asm 1433): in Stamina the lowest value wins (`bge` skip), which may be the opposite of the intent. The port keeps the asm behaviour.
- `game_rules` values: the asm uses 1 for time, 5 for Stamina, and anything else for stock. The decomp comment says 0x1 time and 0x2 stock. The port uses the asm values, so the mapping needs a check in game.
- The asm returns the last fighter it walked when the port is not found. The port returns NULL in that case.
- `hit_gfx_` stores a leftover register at the end (asm `_end`, line 1159). The port keeps the value 8 (existing UNCERTAIN comment).
- `cloud` DObj: the asm reads the item DObj from `s0`, which the routine never sets. The port uses the item's own DObj.
