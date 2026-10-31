# F14 audit: Japan.asm against the port

Source: `.toolchain/pkgs/smashremix-src/src/Japan.asm` (562 lines). Addresses are RAM (US, 0x8xxxxxxx) with the ROM offset in parentheses. Remix is built with REGION_US, so the US branches (WEAPON_REFLECT_MUL_DEFAULT 1.8F, FURAFURA MIN 90, FURASLEEP MIN 75, momentum fix) are the ones that apply. Earlier audit: `docs/remix/notes/f8_japan.md`; this table re-checks it.

## Scope table

| # | Asm scope | RAM (ROM) | Vanilla function (vanilla_funcs.txt) | Port file:function (hook) | Status | Constants check |
|---|---|---|---|---|---|---|
| 1 | hitlag_ | 0x800EA1D0 (0x659D0) | ftParamGetHitLag 0x800EA1C0 | ftremixhitlag.c:ftRemixHitlagTics (hook ftparam.c:1805) | MATCH | 4.0 / 3.0 / 6.0 / 0.65234375 (0x3F270000) / 0 and default 1/3, 5.0 |
| 2 | japanese_shieldstun_ | 0x80149138 (0xC3B78) | ftCommonGuardSetOffSetStatus 0x80149108 | ftremixshieldstun.c:ftRemixShieldStunFrames (hook ftcommonguard2.c:127) | MATCH | 1.75/3.0, 0.451171875/1.9921875, 0.345703125/0, 0.7265625/2.0 decoded from the hex; damage at 0x07C8 = shield_damage |
| 3 | momentum_slide_ | 0x8013F134 (0xB9B74) | ftCommonRunBrakeCheckInterruptTurnRun | inline in ftcommonrunbrake.c (`!gmRemixToggleVS(momentum_slide)`), no ftremix file | MATCH | Polarity checked in Toggles.asm: toggle on skips the clamp (J glitch), 1P/Bonus keep the fix |
| 4 | japanese_stun_mash | 0x8014990C (0xC434C) | ftCommonFuraFuraSetStatus 0x801498A4 | ftremixjstunsleep.c:ftRemixFuraFuraBreakoutWait (hook ftcommonfurafura.c:60) | MATCH | US MIN 90, minus 30 (asm `addiu a1,-30`) |
| 5 | japanese_sleep_mash | 0x801499E8 (0xC4428) | ftCommonFuraSleepSetStatus 0x801499A4 | ftremixjstunsleep.c:ftRemixFuraSleepBreakoutWait (hook ftcommonfurasleep.c:58) | MATCH | US MIN 75, minus 35 (asm `addiu a1,-35`) |
| 6 | japanese_sfx_ | 0x800DF86C (0x5B06C) | ftMainParseMotionEvent 0x800DF0F0 | ftremixjapanesesounds.c:ftRemixJapaneseMotionSoundFgm (hook ftmain.c:415) | MATCH | Hit diffs 0x6B/0x6C/0x6C/0x6E/0x6F/0x6F and beam diff 0x2DE match FGM.asm J_ minus U ids; beam range 0x43-0x45 |
| 7 | japanese_sfx_link_bomb_collide_ | 0x8016E548 (0xE8F88) | not in vanilla_funcs (0x8016E548 is listed under itManagerMakeItem, range mismatch) | ftremixjapanfix_linkbomb.c:ftRemixJapanFixLinkBombSoundFgm (new) | MISSING, hook site NOT located | Same hit diffs as row 6; toggle read without guard (asm has none) |
| 8 | japanese_sfx_link_boomerang_collide_ | 0x80165A1C (0xE045C) | not in vanilla_funcs | ftremixjapanfix_linkboomerang.c:ftRemixJapanFixLinkBoomerangSoundFgm (new, calls row 7) | MISSING, hook site NOT located | Owner is the boomerang owner; asm loads owner at 0x08 of a0, a0 meaning not verified |
| 9 | japanese_sfx_projectile_ | 0x800E359C (0x5ED9C) | inside ftMainUpdateDamageStatWeapon (ftmain.c:2674, next function at 0x800E35BC, line 2727) | ftremixjapanesesounds.c:ftRemixJapaneseProjectileSoundFgm (hook ftmain.c:2721) | MATCH, approximation | Owner check uses link_id fighter instead of the player list walk |
| 10 | reflect_ | 0x80166DD4 (0xE1814) | wpProcessProcHitCollisions (wpprocess.c:467) | ftremixjapanfix_reflect.c:ftRemixJapanFixReflectMul (new) | MISSING, hook site decided | J multiplier 1.5F (0x3FC0 upper halfword) replaces WEAPON_REFLECT_MUL_DEFAULT (1.8F US) for J owners; no toggle |

## Hook sites for the new files

- Row 10: `decomp/src/wp/wpprocess.c:583`, inside wpProcessProcHitCollisions. Replace `WEAPON_REFLECT_MUL_DEFAULT` with `ftRemixJapanFixReflectMul(wp->owner_gobj, WEAPON_REFLECT_MUL_DEFAULT)` under `#ifdef REGION_REMIX`, and include `ft/ftremix/ftremixjapanfix_reflect.h`. Uncertain: the asm loads the multiplier at the reflect entry; the consumer is inferred to be line 583 and not verified in MIPS.
- Row 7: vanilla item-vs-item store of `ITStruct.attack_coll.fgm_id` (0x156) not found in decomp. grep of `fgm_id` in `decomp/src/it` shows no matching store. Hook NOT located; the helper is ready.
- Row 8: vanilla weapon-vs-item store of `WPStruct.attack_coll.fgm_id` (0x146) not found in decomp. Hook NOT located; the helper is ready.

## Notes

- `vanilla_funcs.txt` lists 0x800E35BC as ftMainUpdateAttackStatItem (ftmain.c:2533), but ftmain.c comments place 0x800E35BC at line 2727. The table looks inconsistent for that range.
- Japanese Sounds default (rows 6, 7, 8, 9) uses `kRemixRoster[fkind].sound_type`, not the player struct byte at 0x0B (approximation, same as f8).
- Row 1 default uses `1.0F / 3.0F`; the vanilla US formula in ftparam.c uses the same expression.
