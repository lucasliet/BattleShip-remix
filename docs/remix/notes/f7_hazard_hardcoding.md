# F7 hazard hardcoding: Hazards.asm dream_greens_hardcoding_animation_4 and mk_remix_hardcoding_1..9

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 3372-3670. Helper: `decomp/src/gr/grremix/grremixhardcoding.c/.h`
(`GRREMIX_MK_OFFSET(vanilla, remix)` selects the Remix offset when `gkind == nGRRemixStage_MK_REMIX`, 0x9B).
Only the PORT branch of each vanilla line is patched; the non-PORT `&symbol` branches are left as vanilla.

| Patch | RAM | Vanilla function | Offset (vanilla -> Remix) | Status |
|---|---|---|---|---|
| dream_greens_hardcoding_animation_4 | 0x80106460 | grPupupuUpdateGObjAnims (grpupupu.c) | Whispy eyes textures table 0x78 | Already done in grpupupu.c (`GRPUPUPU_DREAM_GREENS_EYES_TEXTURES`); not duplicated |
| mk_remix_hardcoding_1 | 0x80109514 | grInishieMakeScale | llGRInishieMapScaleDObjDesc 0x380 -> 0x628 | Applied; unreachable for MK_REMIX (scale gated off by grRemixIsMushroomKingdomClone) |
| mk_remix_hardcoding_2 | 0x80109574 | grInishieMakeScale | llGRInishieMapMapHead 0x5F0 -> 0x830 (platform dobj) | Applied; unreachable for MK_REMIX (same gate) |
| mk_remix_hardcoding_3 | 0x80109250 | grInishieScaleUpdateStep | llGRInishieMapScaleRetractAnimJoint 0x734 -> 0x968 | Applied; unreachable for MK_REMIX |
| mk_remix_hardcoding_4 | 0x80109BEC | grInishieInitHeaders | llGRInishieMapMapHead 0x5F0 -> 0x830 (map_head) | Applied; reachable |
| mk_remix_hardcoding_5 | 0x8017D10C | itPakkunWaitProcUpdate | llGRInishieMapPakkunAppearAnimJoint 0xCC8 -> 0xBC8 | Applied; unreachable for MK_REMIX (no piranha plants) |
| mk_remix_hardcoding_6 | 0x8017D130 | itPakkunWaitProcUpdate | llGRInishieMapPakkunAppearMatAnimJoint 0xCF8 -> 0xBEC | Applied; unreachable for MK_REMIX |
| mk_remix_hardcoding_7 | 0x8017D400 | itPakkunAppearProcDamage | llGRInishieMapPakkunDamagedMatAnimJoint 0xE04 -> 0xCE4 | Applied; unreachable for MK_REMIX |
| mk_remix_hardcoding_8 | 0x8017C180 | itPowerBlockWaitProcDamage | llGRInishieMapPowerBlockDataStart 0x11F8 -> 0xF80; AnimJoint 0x1288 -> 0x1060 | Applied; reachable |
| mk_remix_hardcoding_9 | 0x80137464 | none in decomp (CPU AI platform/height check, `jal 0x800FCA18`) | n/a | Skipped: REMIX-TODO(AI). vanilla_funcs.txt maps this address to mnVSResultsSetIsPresent, which is a wrong mapping |

Notes:
- The Pakkun and scale patches (1, 2, 3, 5, 6, 7) only matter if the gating is lifted; they stay so the vanilla tables match the Remix file layout.
- Stage id to test: MK_REMIX = `SSB64_VS_STAGE=155` (0x9B). Power block (patch 8) and Inishie map head (patch 4) are the reachable ones.
- The ai-memory project scope for BattleShip was not found on the server, so only the `_global` rules were loaded.
