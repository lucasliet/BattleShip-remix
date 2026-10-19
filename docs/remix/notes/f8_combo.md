# F8 Combo Meter (ComboMeter.asm, Combo.asm)

Settings: `Combo Meter`, `Improved Combo Meter`, `Tech Chase Combo Meter`, `1v1 Combo Meter Swap`. All four are bool
toggles (`0` off, `1` on), read with plain `gmRemixToggle` because the asm uses `Toggles.guard`. Titles and values in
`Toggles.asm` lines 2390-2393. Overrides for a run: `SSB64_REMIX_TOGGLES="Combo Meter=1;Improved Combo Meter=1;Tech Chase Combo Meter=1;1v1 Combo Meter Swap=1"`.
Dragon King HUD (`entry_dragon_king_hud`, line 2557) is read as well: `DRAGON KING` (0, default) puts the meter low on
the Dragon King stages, `ALL STAGES` (1) always, `OFF` (2) never.

## Files

- `decomp/src/if/ifremixcombometer.h` (new): `IFRemixComboMeter`, one per port.
- `decomp/src/if/ifremixcombohits.c` (new): hit state per frame (`ifRemixComboHitsUpdate`, asm `draw_hit_count_`).
- `decomp/src/if/ifremixcombometer.c` (new): setup (`ifRemixComboMeterSetup`), GObj and SObj creation, team and port
  colours, 1v1 swap, per-frame draw and `ifRemixComboMeterHideForScore`.
- `decomp/src/ft/ftremix/ftremixcombo.c` (new): `ftRemixComboKeepsCount` (asm `Combo.improve_meter_`).
- `decomp/src/ft/ftremix/ftremixhooks.h`: block appended at the end (before the closing `#endif`).
- Hooks in vanilla files, all under `#ifdef REGION_REMIX`:
  - `decomp/src/sys/taskman.c` `syTaskmanLoadScene`: `ifRemixComboMeterSetup()` before `func_start`. Include added.
  - `decomp/src/if/ifcommon.c` `ifCommonPlayerScoreMakeEffect`: `ifRemixComboMeterHideForScore(fp->player)`.
  - `decomp/src/ft/ftmain.c` (the `combo_count_foe` reset, `if (!(this_fp->is_hitstun))`): `ftRemixComboKeepsCount(this_fp)`.

## Patches

| Asm (file, label) | RAM | Vanilla function (vanilla_funcs.txt) | Status | Notes |
|---|---|---|---|---|
| `ComboMeter.setup_` (ComboMeter.asm), called from `Render.setup_` (Render.asm 2256, 2274, 2281); the patch is at Render.asm line 2116 | 0x800067B4 | `syTaskmanLoadScene` (sys/taskman.c, 0x80006548) | applied, approximated | Hook placed right before `func_start`. The asm calls it only for the 1P, multiman and VS screens; the port gates on `scene_curr` (VS battle or 1P game). Multiman (screen 0x77) is not mapped. |
| `ComboMeter.hide_for_score_sprite_` (ComboMeter.asm, OS.patch_start) | 0x801149D8 | `ifCommonPlayerScoreMakeEffect` (if/ifcommon.c, 0x801149CC) | applied | Clears the frame buffer of the slot of `fp->player` in a VS battle when the setting is on. |
| `Combo.improve_meter_` (Combo.asm, OS.patch_start) | 0x800E19CC | `ftMainUpdate` loop (ft/ftmain.c, around the `combo_count_foe` reset) | applied | Port of the action table (21 entries, plus the 5 Tech Chase entries when that setting is on), and the knockback check. |
| `Combo.j_begin_loop` | none | n/a | applied (folded) | The Tech Chase guard is a plain condition in `ftRemixComboIsKeepingAction`. |
| `ComboMeter.run_` (linked from setup as the GObj process) | none | `gcMakeGObjSPAfter` process | applied | Only the first meter GObj gets `run_`, as the asm does with s2. |
| `ComboMeter.draw_hit_count_` | none | `ifremixcombohits.c`, `ifremixcombometer.c` | applied | Hit count and damage from `gSCManagerBattleState->players[port].combo_count_foe / combo_damage_foe`. |
| `ComboMeter.initialize_combo_structs_` | none | `ifRemixComboInitMeters` | applied | Colour and attribution fields are kept between matches, as in the asm. |
| `ComboMeter.set_color_by_team` / team map | none | `ifRemixComboSetColorSets` | applied | Team colours when `is_team_battle` in a VS battle. |
| `ComboMeter.port_check` and the 1v1 table | none | `ifRemixComboAttachPlayers`, `ifRemixComboSwapSingles` | applied | Player count from `pkind` (0 man, 1 CPU). The swap exchanges the two x positions and the slots, as in `_swap_for_singles`. |
| `PokemonAnnouncer.pokemon_combo_` | none | n/a | REMIX-TODO(PokemonAnnouncer) | Called in draw_hit_count_ before the frame buffer is read. Skipped. |
| `FGM.announcer.misc.INCREDIBLE` (466) | none | `func_800269C0_275C0` | applied | Plays at 20 hits, once per combo. |
| Pause check (`0x8004671C` HUD list flags 0x7C) | none | n/a | skipped | Not replicated. The GObj uses link 0x0B (the paused queue), which the asm comment says keeps the meter off while paused. Lead should check the pause in game. |

## Port decisions

- Hit state: the asm counts in the match info. The port keeps the same numbers in `SCPlayerData`: `combo_count_foe`
  (hit count, the damage is the field right before it, which is the asm `-4` read) and `combo_damage_foe`. Those are
  reset by the vanilla code when hitstun ends, so the meter keeps the vanilla counting.
- Attribution: `FTStruct.damage_player` (`0x080C`), with `-1` = none and `>= 4` = silver, as in the asm.
- Current action: low halfword of `status_id` (`0x0026`). Hitstun counter: low halfword of
  `status_vars.common.damage.hitstun_tics` (`0x0B1A`). Knockback: `damage_knockback_stack` (`0x07EC`).
- Textures: Remix file `0x9A6` (`File.VS_COMBO_METER_TEXTURES`) loaded with `lbRelocGetExternHeapFile`, the same way
  as the raw Remix ids of `ftremixkirbyhats.c`. The bitmap of a colour set is `file + offset - 0x10` (the 16-byte
  bitmap header before the texels). Each texture is registered once per setup with `portRelocRegisterPointer` and the
  tokens are kept in arrays, so no token is registered per frame.
- Hidden digits use a null bitmap token, which `lbCommonPrepSObjDraw` skips.

## Deviations and uncertainties

1. Stats reset: the asm keeps the max hits, max damage and highest combo in Smashketball sudden death. The port has no
   `VsRemixMenu.vs_mode_flag`, so the stats are always reset. Only Smashketball sudden death differs.
2. X position: a port with HUD position 0 gets `0.0F`. The asm keeps the previous `f0` value there.
3. Dragon King stage check uses `gSCManagerSceneData.gkind` (`0x35` and `0xAD`), the asm reads match info stage id.
4. `lbCommonMakeSObjForGObj` decodes 4-bit (`G_IM_SIZ_4c`) sprites into a new buffer. The digit and COMBO textures are
   not decoded by the port's bind (only the sprite template is). If the Remix texture file is 4c, the digits will show
   wrong colours. Check in game.
5. Byte-order fixups of the digit and text bitmaps use `portFixupBitmapArray` and `portFixupSpriteBitmapData` with the
   template sprite (the port's own helpers, not verified against the file).
6. `VsStats.asm` lines 1002-1006 read `ComboMeter.combo_struct_p1` (max hits, max damage, highest combo). That scope is
   not ported here. The VS stats worker needs an accessor for `sIFRemixComboMeters[port]` (static in
   `ifremixcombometer.c`), or the stats move to a getter in `ifremixcombometer.h`.
7. Each setup loads the texture file again (the asm does the same), so the tokens are re-registered each match.
8. The scene gate reads `gSCManagerSceneData.scene_curr` inside `syTaskmanLoadScene`. If the scene is set after that
   point on some path, the meter is not set up for that match. Check that a VS match shows the meter at all.

## What the lead should see

- `Combo Meter=1` (other three at defaults): in a VS match, after the first hit, a COMBO meter with the hit count
  appears over each fighter. Colour is the attacker's port colour (red P1, blue P2, yellow P3, green P4), silver for an
  unattributed hit. The meter fades after about 30 frames without a hit. At 20 hits the INCREDIBLE sound plays.
- `1v1 Combo Meter Swap=1` in a 1v1: each meter sits where the other player's meter would be.
- `Teams`: team colours on the meters.
- `Dragon King HUD`: DK stages move the meter lower with `DRAGON KING` or `ALL STAGES`.
- `Improved Combo Meter=1`: a combo does not reset when the victim is in a listed action (captured, thrown, stunned,
  tornado, barrel, egg lay) or in knockback hitstun. `Tech Chase Combo Meter=1` adds tech and down bounce.
- Off (all `0`): no meter, no change to the vanilla combo reset.
- Not verified in game: texture decode (deviation 4), pause behaviour (skipped check), multiman screen (not mapped).
