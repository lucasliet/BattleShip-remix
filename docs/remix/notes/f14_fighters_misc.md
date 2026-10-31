# F14 fighters_misc: REMIX-TODO closure (ROADMAP T2.11)

Scope files: `decomp/src/ft/ftremix/ftremix{zcancel,dpad,damage,shield,sizemove,sizefx,handicap,staminaaction}.c`,
`ftremix/{banjo/ftremixbanjousp,lanky/ftremixlankydsp,dragonking/ftremixdragonkingnsp,dragonking/ftremixdragonkingusp,sheik/ftremixsheiknsp,goemon/ftremixgoemonusp}.c`,
`ftcommon/ftcommonthrow.c`.

## TODO state

| File:line | TODO | State | Why |
|---|---|---|---|
| ftremixzcancel.c (instant_ko) | 1P final boss | skipped | `ZCancel.asm` 151-167 needs `Global.current_screen == 1` (decomp scene 1 is `nSCKindTitle`, not a 1P screen; 1P battles run as `nSCKind1PGame`, so the check is never true there), `STAGE_FLAG == 0xD` (`nSC1PGameStageBoss`) and the match-off byte at 0x80131580. That byte is `gIFCommonPlayerInterface.is_magnify_display` in the decomp and the Remix writer is not in the asm, so its meaning is unverified. Port needs the owner's decision. |
| ftremixzcancel.c (setup_ egg) | Yoshi egg file 0x153 | skipped | Needs the Remix file loader for the egg model/data. Not a fighter-routine change. |
| ftremixzcancel.c (cruel z cancel) | HowToPlay | implemented | `ZCancel.asm` 1-60 (`_cruel_z_cancel`): reads the first byte of `*(0x800A50E8)` (= `gSCManagerBattleState->game_type`) and skips the punishment, including the RANDOM roll, when it is 3 (`nSCBattleGameTypeExplain`, set in `scexplain.c`). |
| ftremixdpad.c:137 | DpadControl partial stick (+-40 tap, +-80 held) | skipped | `css/DpadFunctions.asm` 200-520+ is a state machine: per-port `dpad_move_timer` (double tap window 10, Walk2/Walk3/Turn frames), per-character action id tables (Spacies, Pikachu, Sheik, M2, Mystical Ninja), air/ground and platform checks, a clip flag write (0xF6) and writes to the controller stick word 0x1C2. The port's Stick Swap path does not hold that state. Needs its own task. |
| ftremixshield.c:7 | Costume (17) | skipped | `Shield.asm` 252-290 reads `Character.costume_shield_color.table[fkind]` then the costume byte, then `table_custom[index]`. The table symbol `Character.costume_shield_color.table` (0x8049D1C0 in `build/remixasm/remix.sym`) is not in `port/remix/remix_symbols.h`. `tools/gen_remix_actions.py` `EXTRA_SYMBOLS` does not list it, so it needs that generator edit and a regeneration. Those files are outside this scope. Sonic classic (+6 costume index) also needs the fighter in the caller (`efmanager.c:4217` passes only the port). |
| ftremixshield.c:7 | Yoshi egg colour | skipped | `Shield.asm` `alter_yoshi_egg_color_` uses `Render.MODEL_PART_IMAGE_INIT_`, which the port does not have. |
| ftremixdamage.c:5 | stun, bury, flashbang, wind push | resolved (stale TODO) | Already ported: `ftremixdamagestun.c` (`ftRemixDamageGotoStunAction`, `ftRemixDamageUsesElectricStatus`) and `ftparam.c:630` (`prevent_wind_push_while_buried`). The header comment was updated. |
| ftremixsizemove.c:76 | Pwing 0.8 aerial jump scale | skipped | `Size.asm` 782-788 reads `Item.Pwing.pwing_jump_flag` per port. The port's `itremixpwing.c` keeps no such flag and exposes no accessor. Needs an accessor from the Pwing item (`it/itremix`, not in scope). |
| ftremixsizefx.c:8 | Size egg lay, Samus gun, Kirby USP render, Falcon kick | skipped | Size subsystem (`Size.asm` 1362, 1541, 1611-1619) and Kirby USP tables. Other scope. |
| ftremixhandicap.c:14 | AI handicap byte | skipped | The AI (M7) is not ported. |
| ftremixstaminaaction.c:148 | 1P Giga Bowser HP | implemented | `Stamina.asm` 655-680 (`_remix` branch): CPU Giga Bowser in 1P Remix, KO when percent > HP; HP 200 below Normal, 250 for Normal and Hard, 300 for Very Hard, from the 1P difficulty (`gSCManagerBackupData.spgame_difficulty`, the asm byte 0x800A493A). |
| banjo/ftremixbanjousp.c:140 | Size wall bounce GFX | skipped | `Size.ground_gfx.save_player_struct_._wall_bounce` and atan2 0x8001863C. Size GFX subsystem, not in scope. |
| lanky/ftremixlankydsp.c:594 | merged buttons store | implemented | `LankySpecial.asm` 1954-1985 (`cancel_main_`, `_change_action`): the delay slot stores `t6 = buttons_pressed | buffered` to the player struct at 0x1BE (`input.pl.button_tap`), not to the GObj as the TODO said. The port stores `vars->buffered_buttons` (already OR-ed this frame). |
| dragonking/ftremixdragonkingnsp.c:281 | Kirby lag procs | skipped | The asm stores 0x800F9C8C and 0x800F9CC4 as lag procs. Neither is in `vanilla_funcs.txt` or the decomp, so the target function is unknown. |
| dragonking/ftremixdragonkingusp.c:203 | cloaking device damage skip | implemented | `DragonKingSpecial.asm` 217-223: the drain skips the 1-damage hit when `Item.CloakingDevice.cloaked_players[captured port]` is set. Port: `itRemixCloakingDeviceIsCloaked(captured_fp->player)`. |
| goemon/ftremixgoemonusp.c:93 | Size render routine on the cloud | skipped | Size item render routine (`Size.item.render_routine_`). Size subsystem, not in scope. |
| sheik/ftremixsheiknsp.c:352 | GFXRoutine.SHEIK_CHARGE spark level | skipped | The spark GObj (`user_data + 0x2A4`) does not exist in the port, as the TODO says. |
| ftcommon/ftcommonthrow.c:107 | PokemonAnnouncer seismic toss + action 0xE5 for Bowser/GBowser | skipped | `jigglypuffkirbyshared.asm` 272-300 (`kirby_fthrow_1_fix`): Bowser and GBowser set `t3 = 0xE5` (Bowser's own forward throw status) and play the seismic announcement, then jump into the Kirby branch body at 0x8014A24C. Not verified: whether that body also calls `mpCommonSetFighterAir` for Bowser (the Kirby branch does). Left open. |

## Files edited

- `decomp/src/ft/ftremix/ftremixzcancel.c`: `ftRemixZCancelIsHowToPlay`, check in `ftRemixZCancelPunish`, includes `sc/scmanager.h` and `sc/sctypes.h`.
- `decomp/src/ft/ftremix/ftremixstaminaaction.c`: Giga Bowser HP helpers and the 1P branch in `ftRemixStaminaKOStatus`, includes `sc/scmanager.h` and `sc/sc1pmode/sc1premix1p.h`.
- `decomp/src/ft/ftremix/ftremixdamage.c`: header comment (stale TODO replaced).
- `decomp/src/ft/ftremix/lanky/ftremixlankydsp.c`: button_tap store before `mpCommonSetFighterWaitOrFall`.
- `decomp/src/ft/ftremix/dragonking/ftremixdragonkingusp.c`: cloak check, include `it/itremix/itremixcloakingdevice.h`.

## Uncertainties

- Giga Bowser 1P check uses `sc1PRemix1PIsActive()`. The asm checks `singleplayer_mode_flag == REMIX_1P_ID (4)`. The two agree while the Remix 1P path is built; the lead should confirm the state is the same during the 1P Giga Bowser fight.
- Giga Bowser threshold uses `nSC1PGameDifficulty*` (VeryEasy=0 ... VeryHard=4). The asm thresholds `< 2`, `< 4` map to Normal and Hard.
- The HowToPlay check reads `game_type` (the asm comment calls it a screen id). The value 3 is `nSCBattleGameTypeExplain`, not a screen.

## Lead wiring and tests

- Giga Bowser: 1P Remix, Giga Bowser as CPU, at Very Easy, Normal and Very Hard. Percent over 200, 250 or 300 should trigger the stamina KO (DownBounceD or Tumble).
- HowToPlay: enable the Cruel Z-cancel punishment and fail a Z-cancel on the how-to-play screen. No punishment should happen.
- Cloaking device: DK USP drain on a cloaked captured player should heal DK but not damage the victim.
- Lanky DSP: the cancel after landing should keep buffered buttons, as before.
