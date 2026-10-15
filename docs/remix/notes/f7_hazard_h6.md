# F7 hazard group h6: BayDraculaTwilight

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 6887-8072.
Stage ids (grremixids.h): GREAT_BAY 0x36, DRACULAS_CASTLE 0xB0, TWILIGHT_CITY 0xBA, REAPERS 0xBE, SCUTTLE_TOWN 0xBF, TIME_TWISTER 0xD2.

## Files

- `decomp/src/gr/grremix/grremixwater.c` / `.h`: water process shared by Great Bay and Twilight City.
- `decomp/src/gr/grremix/grremixhazards_baydraculatwilight.c`: dispatch by setup address.

## Ported

| Remix routine | Stage | Status |
|---|---|---|
| great_bay_setup + great_bay_water_ | GREAT_BAY | ported, water_y -1128.0, splash_y -828.0 |
| twilight_city_setup + twilight_city_water_ | TWILIGHT_CITY | ported, water_y 0.0, splash_y 0.0 (asm constants are placeholders `WATER_Y(0)`) |

Water behaviour: per fighter, Fox/Jigglypuff-Fox/Falco in Fire Fox air are skipped (asm status 0xE8). Under water, the first frame spawns the splash (sparks at both sides, FGM 0x3D1 through `func_800269C0_275C0`). Each frame the asm drag is applied to `physics.vel_air` and `physics.vel_damage_air`.

## Skipped or partial

- `draculas_castle_setup_`: only sets the global env colour 0xFFA8C4FF (`set_player_env_colour_`, RAM 0x80131388). Returns TRUE, colour not applied. REMIX-TODO(CharEnvColor).
- `reapers_setup_`: sets env colour 0xFF0000FF when hazards are on. Returns TRUE, colour not applied. REMIX-TODO(CharEnvColor).
- `scuttle_town_setup_`, `scuttle_town_bg_fade_`: the asm has `b _end` before the hazard block (`// TODO: remove when minions stop crashing`), so only the night/day background fade is set up. The fade needs GFX (image object and palette) and is not ported. Returns FALSE, so the stage runs without hazards and logs `[remix-stage]`.
- `minion_factory_`, `minion_stage_setting_`, `minion_main_`, `minion_destroy_`, `minion_blast_zone_`: unreachable. `minion_factory_` is only attached by `scuttle_town_setup_` after the `b _end`, so the ScuttleMinion item (0x3E) is not ported in this group. Port it when the Scuttle Town fade and minions are re-enabled.
- `time_twister_setup_`, `time_twister_bg_fade_`, tables `time_twister_colors`, `time_twister_glass_colors`, `time_twister_bgms`, `time_twister_bgm_change`: fade with warp ball, camera zoom (`gmCameraAnimFuncCamera`, `gmCameraMapZoomFuncCamera`), BGM change and volume fades (`syAudioSetBGMVolume`, `syAudioSetBGMVolumeFade`). Not ported. Returns FALSE.

## REMIX-TODO

- `match-state` in `grRemixWaterProcUpdate`: the asm skips training mode (`Global.current_screen == 0x36`) and fighters before the match timer (`Global.match_info` time elapsed). No named port accessor. The water acts during the intro.
- `CharEnvColor` for Draculas Castle and Reapers (see above).

## Uncertainties

- `Render.CREATE_OBJECT_` (asm lines 6887-6935 and the twilight copy): object id 0x3F2 and group 1 are mapped to `gcMakeGObjSPAfter(nGCCommonKindGround)` plus `gcAddGObjProcess(..., 4)`, the same pattern as grpupupu.c. The exact link and priority of the original object are not verified.
- Clearing 0x18D with mask 0x07 also clears is_absorb, absorb_lr and is_goto_attack100. Ported as written; the asm comment only mentions fast fall.
- Remix Falco uses the same status id 0xE8 for FireBirdAir in the asm, ported as written.
- `fp->player >= 4` is skipped. The asm iterates ports 0 to 3 through `Character.port_to_struct_`.

## Not verified

- Build and run not done (worker rule). Syntax check OK for all three files.
- Stage ids to run with `SSB64_VS_STAGE`: Great Bay 54 (0x36), Twilight City 186 (0xBA).
- Project memory: `memory_briefing` for project scope failed (`JRickey/BattleShip` not found in workspace `default`, `JRickey` workspace not found). Only the global rules were read.
