# F8 notes: Japan.asm toggles (Hitlag, Japanese Sounds, Japanese Stun/Sleep, Momentum Slide, Shield Stun)

Source: `.toolchain/pkgs/smashremix-src/src/Japan.asm`. DI and Hitstun are ported by other workers and skipped here.

## Toggles (`grep -n entry_ Toggles.asm`, lines 2420-2425)

| Toggle | Entry | Title (run override) | Values | Guard | Profile defaults CE/TE/NE/JP |
|---|---|---|---|---|---|
| Hitlag | `entry_hitlag` | `Hitlag` | 0 normal, 1 japanese, 2 melee, 3 ultimate, 4 none (0-4) | single_player_guard (VS) | 0/0/0/1 |
| Japanese Sounds | `entry_japanese_sounds` | `Japanese Sounds` | 0 default (J fighters only), 1 always J, 2 never J (0-2) | none, read directly | 0/0/0/1 |
| Japanese Stun/Sleep | `entry_j_stun_sleep` | `Japanese Stun/Sleep` | bool | single_player_guard (VS) | 0/0/0/1 |
| Momentum Slide | `entry_momentum_slide` | `Momentum Slide` | bool | single_player_guard (VS) | 0/0/0/1 |
| Shield Stun | `entry_shieldstun` | `Shield Stun` | 0 default, 1 japanese, 2 melee, 3 brawl, 4 ultimate (0-4) | single_player_guard (VS) | 0/0/0/1 |

Run one toggle with `SSB64_REMIX_TOGGLES="<title>=<value>"`, for example `SSB64_REMIX_TOGGLES="Hitlag=1"`. Bool toggles use `=1`.

## Patches

| asm address (ROM) | vanilla function | status | notes |
|---|---|---|---|
| 0x800EA1D0 (0x659D0) hitlag_ | ftParamGetHitLag (ftparam.c) | applied | `ftRemixHitlagTics` replaces the tics formula. Constant and damage multiplier per value; 0 and out-of-range keep the vanilla formula. |
| 0x80149138 (0xC3B78) japanese_shieldstun_ | ftCommonGuardSetOffSetStatus (ftcommon/ftcommonguard2.c) | applied | `ftRemixShieldStunFrames(fp, vanilla)` replaces `shield_damage * MUL + ADD` for values 1-4. |
| 0x8013F134 (0xB9B74) momentum_slide_ | ftCommonRunBrakeCheckInterruptTurnRun (ftcommon/ftcommonrunbrake.c) | applied | Inline: the US momentum slide clamp is skipped when the toggle is on (the J glitch). |
| 0x8014990C (0xC434C) japanese_stun_mash | ftCommonFuraFuraSetStatus (ftcommon/ftcommonfurafura.c) | applied | `ftRemixFuraFuraBreakoutWait`: breakout wait minus 30 (60 instead of 90). |
| 0x801499E8 (0xC4428) japanese_sleep_mash | ftCommonFuraSleepSetStatus (ftcommon/ftcommonfurasleep.c) | applied | `ftRemixFuraSleepBreakoutWait`: breakout wait minus 35 (40 instead of 75). |
| 0x800DF86C (0x5B06C) japanese_sfx_ | ftMainParseMotionEvent, nFTMotionEventPlayFGM case (ftmain.c) | applied | `ftRemixJapaneseMotionSoundFgm(fp, value)` maps the fgm id before `func_800269C0_275C0`. |
| 0x800E359C (0x5ED9C) japanese_sfx_projectile_ | ftMainUpdateDamageStatWeapon (ftmain.c) | applied | `ftRemixJapaneseProjectileSoundFgm(wp->owner_gobj, wp_attack_coll->fgm_id)`. Hit ids only, as in the asm. |
| 0x8016E548 (0xE8F88) japanese_sfx_link_bomb_collide_ | not mapped | skipped | Writes `ITStruct.attack_coll.fgm_id` (0x156). `build/remixasm/refs/vanilla_funcs.txt` maps 0x8016E548 to itManagerMakeItem (range 0x8016E174-0x8016EA78), which is not the collision code, and no other entry covers it. Locate the Link bomb collision function, then port with `ftRemixJapaneseItemSoundFgm`-style helper. |
| 0x80165A1C (0xE045C) japanese_sfx_link_boomerang_collide_ | not mapped | skipped | Writes `WPStruct.attack_coll.fgm_id` (0x146). Same problem: 0x80165A1C falls between wpManagerMakeWeapon (0x801655C8) and wpProcessUpdateHitOffsets (0x80165ED0), with no entry for the function. It calls the link bomb routine above, so both are skipped together. |
| 0x80166DD4 (0xE1814) reflect_ | not a toggle | skipped | Not a setting: the J reflect multiplier (1.5F) applies to J characters with no toggle. Outside this scope; decide whether to port it as an always-on Remix behaviour. |

## Port files

- `decomp/src/ft/ftremix/ftremixhitlag.c` (new): `ftRemixHitlagTics`.
- `decomp/src/ft/ftremix/ftremixshieldstun.c` (new): `ftRemixShieldStunFrames`.
- `decomp/src/ft/ftremix/ftremixjstunsleep.c` (new): `ftRemixFuraFuraBreakoutWait`, `ftRemixFuraSleepBreakoutWait`.
- `decomp/src/ft/ftremix/ftremixjapanesesounds.c` (new): `ftRemixJapaneseMotionSoundFgm`, `ftRemixJapaneseProjectileSoundFgm`, plus file-local helpers for the hit and beam sword diffs and `ftRemixJSfxIsJ`.
- `decomp/src/ft/ftremix/ftremixhooks.h`: block appended after the Perfect Shielding block (declarations for the six functions above).
- Vanilla edits, each under `#ifdef REGION_REMIX` with the original line in `#else` (or the original kept after the hook): `ftparam.c` (ftParamGetHitLag), `ftcommon/ftcommonguard2.c`, `ftcommon/ftcommonrunbrake.c`, `ftcommon/ftcommonfurafura.c`, `ftcommon/ftcommonfurasleep.c`, `ftmain.c` (two sites).

## Approximations and uncertainties

- Japanese Sounds, default value: the asm reads the character id byte at player struct 0x0B. `FTStruct` has no named field there (struct_offsets.txt has no entry at 0x0B), so the port uses `fp->fkind`. The sound type comes from `kRemixRoster[fkind].sound_type` (remix_roster_data.h), not from the RAM table `Character.sound_type.table` (0x8049C890 in remix.sym). The symbol is not in `port/remix/remix_symbols.h`, so the port does not use it.
- Projectile owner check: the asm walks the player list (`Global.p_struct_head`) to confirm the owner is a fighter. The port checks `owner_gobj->link_id == nGCCommonLinkIDFighter`, which is the same set (fighter gobjs are the players).
- The Hitlag ultimate damage multiplier is 0.65234375 (0x3F270000), exact. All shieldstun constants are exact decimals of the asm hex (checked with struct.unpack).
- The hitlag `none` value zeroes both the damage multiplier and the constant, as the asm does with `mtc1 r0, f8` and `lui at, 0`.
- Hitlag and shieldstun use gmRemixToggleVS, as Toggles.single_player_guard does. Japanese Sounds uses gmRemixToggle, as the asm reads the entry directly (it has no guard).
- The asm's shieldstun `default` value (0) and the Hitlag `normal` value (0) jump to the vanilla exit, so they are the vanilla formula.

## What the lead should see

- Hitlag=1 (Japanese): hits freeze the target and attacker for less time (constant 4 instead of 5). Hitlag=2 (Melee): constant 3. Hitlag=3 (Ultimate): damage multiplier 0.65 and constant 6, so long hitlag on big hits. Hitlag=4 (None): no hitlag at all, hits connect with no freeze. Bonus and 1P Game ignore the toggle.
- Shield Stun=1: the shield break stun is 1.75 * damage + 3 frames (vanilla 1.62 * damage + 4). Shield Stun=2 (Melee): 0.45 * damage + 1.99. Shield Stun=3 (Brawl): 0.35 * damage. Shield Stun=4 (Ultimate): 0.73 * damage + 2.
- Japanese Stun/Sleep=1: the Fura Fura breakout wait is 60 + (400 - percent) instead of 90 + ..., and the Fura Sleep wait is 40 + (300 - percent) instead of 75 + ..., so the player breaks out sooner.
- Momentum Slide=1: after a run stops, the velocity above run speed is kept (the J glitch). Off: vanilla US clamp.
- Japanese Sounds=1: all fighters play the J hit sounds (punch and kick S/M/L, beam sword). Japanese Sounds=2: no J sounds. Default: only fighters whose Remix sound type is J (roster `sound_type`) use them. Projectile hits use the same rule with the owner's sound type.
- Not built and not run, as the brief requires. Syntax checks: all touched files print `syntax-check OK`.
