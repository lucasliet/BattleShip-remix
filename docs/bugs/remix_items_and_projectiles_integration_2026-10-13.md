# Remix items and projectiles: integration crashes and LP64 mappings

Failures found by driving each fighter in a scripted VS match once the custom items (`decomp/src/it/itremix/`) and the
projectile moves were wired into the fighters. Code: `decomp/src/ft/ftremix/`, `decomp/src/it/itremix/`,
`decomp/src/wp/`, `port/bridge/lbreloc_bridge.cpp`, `decomp/src/ft/ftport.c`. Test: `vs.sh <fkinds> <input-script> <name>` with
`SSB64_VS_FKINDS="a,b[,c,d]"` (scene 22, Dream Land, P1 human, others CPU level 1) and `SSB64_INPUT_SCRIPT`
(`.claude/skills/remix-smoke-tests/SKILL.md`). A VS fighter spawns with `lr = 0`, so a throw from the first frame has x velocity
0; that is game-accurate.

## 1. Extern relocation target past the end of the file

**Symptom:** Peach's Down Special crashed in `itRemixTurnipMakeItem` (`joint->mobj` NULL). The log had
`lbReloc bridge: file_id 5144 intern chain target OOB`.

**Cause:** Remix's `PEACH_TURNIP_GFX` (file 5144) lists two unused face images at words 0x930 and 0x9B2 of a 0x498-word file.
The bridge aborted the whole chain walk there, so every fixup after the bad word (including the texture the item model needs)
was left as a raw offset.

**Fix:** `lbreloc_bridge.cpp` nulls that slot, warns, and keeps walking. The step bound in the same loop still contains a
corrupt chain.

## 2. Unmatched `LoopEnd` in a Remix moveset

**Symptom:** a Kirby copy of Mewtwo's Teleport hung the game (watchdog) in `ftMainUpdateMotionEventsForwardEffect`.

**Cause:** `Mewtwo/moveset/USP_END.bin` has a `LoopEnd` with no `LoopBegin`. On the N64 it indexes `loop_count[-2]` and
`p_goto[-2]`, which alias `script_id` and `p_script`, and the script ends. The LP64 `FTMotionScript` layout reads other fields
and loops forever.

**Fix:** `ftMainParseMotionEvent` ends the script when `LoopEnd` runs with `script_id < 2` (`#ifdef PORT`).

## 3. Item descriptors run into the state table

Remix's `item_info_array` (the `ITDesc`) is immediately followed by the `item_state_table`, so the ITDesc procs at 0x14-0x30
are the procs of state 0. Items whose descriptor left them NULL never updated (frozen Waddle Dees, Gordo). Every Remix item
descriptor now sets them, and `ITRemixWaddleDoo`'s footer offset is 0xB0 (it was 0x08).

## 4. Minion layouts shared by three items

Waddle Dee, Waddle Doo and Gordo share one Remix minion state machine, so they must share one `item_vars` prefix
(`ITRemixWaddleDeeVars`). The `_Static_assert`s in `itremixwaddledee.h` and `itremixgordo.c` pin it. The thrown/attack flag the
asm keeps at N64 ITStruct 0x33E is `ITStruct.multi`.

## 5. Per-spring state in `weapon_vars`

Sonic's Spring kept its state in a table indexed by player; the old spring lost its entry when the next one spawned
(`ftRemixSonicUSPSpringSettle` SIGSEGV). The state now lives in the weapon (`FTRemixSonicUSPSpring` over `wp->weapon_vars`,
`ftremixsonicusp.c`).

## 6. Active-item slots that nothing wrote

Conker's grenade slot (N64 0x0ADC) was never set or cleared, and the grenade's `ProcDead` returned FALSE, so the grenade was
never destroyed at the blast zone. Peppy's Flashbang uses the same slot; the grenade sets it at spawn and clears it at
detonation (`itremixconkergrenade.c`, `itremixflashbang.c`).

## 7. `lifetime` is N64 WPStruct 0x268, `group_id` is 0x264

A MIPS32 struct layout dump generated for the asm port was 4 bytes low on WPStruct from 0x25C. Several ports followed it and
decided the asm's `sw duration, 0x268(wp)` hit `is_camera_follow`, so Conker's nut, Goemon's ryo and Lanky's grape never
expired, and Goemon's ryo overwrote `lifetime` with its group id. `0x264` is `group_id` and `0x268` is `lifetime`
(`wpMainDecLifeCheckExpire` reads 0x268). Ported code must use named fields, never offsets from that dump.

## 8. Vanilla patches applied

| Remix patch | Port site |
|---|---|
| DSamusDSP ground/air physics, jump velocity, air status, flags | `ftsamusspeciallw.c` -> `ftremixdsamushooks.c` |
| DSamusNSP charge level table, aerial charge, release flag | `ftsamusspecialn.c`, `wpsamuschargeshot.c` -> `ftremixdsamushooks.c` |
| SamusShared charge shot / bomb model file | `wpsamuschargeshot.c`, `wpsamusbomb.c` -> `ftremixsamusshared.c` |
| Slippy USP duration, forced stick_y, ledge catch | `ftfoxspecialhi.c` |
| Super Sonic teleport cancel | `ftMainProcUpdateInterrupt` -> `ftRemixSSonicUSPInterruptPatch` |
| Ebisumaru USP refresh | `ftRemixRunOnHit` (`ftparam.c`) |
| Lanky USP damage status, DSP taunt stance bonus | `ftCommonDamageInitDamageVars`, `sc1pgame.c` -> `ftremixlankyhooks.c` |
| Mad Piano absorb | `ftNessSpecialLwProcAbsorb` -> `ftRemixPianoDSPAbsorbRedirect` |

## 9. Shared patches for the clone fighters

Remix's `*shared.asm` files patch vanilla routines so that the clones (J/E/Dark Samus, J/E/Y Link, Marina, J Donkey Kong, the
Falcon family, Dr. Mario, J Mario, Piano, J Kirby, ...) use their own projectile files and values. Each patch is applied at the
vanilla function that contains its RAM address, under `#ifdef REGION_REMIX`, and calls a helper in `decomp/src/ft/ftremix/`;
`rg` the helper's prefix to find the sites.

| Scope | Helper file | Vanilla sites (examples) |
|---|---|---|
| Samus clones (charge sparkle, Kirby copy flash, grapple beam) | `ftremixsamusshared2.c` | `ftparam.c`, `ftkirbyspecialn.c`, `ftcommoncatch1.c`, `ftcommonthrow.c` |
| Link family (sword trail, up special, spin attack, boomerang) | `ftremixlinkshared.c` | `ftparam.c`, `ftcommonattackair.c`, `ftcommoncatch2.c`, `ftmain.c`, `ftlinkspecialhi.c`, `wplinkspinattack.c`, `wplinkboomerang.c`, `itlinkbomb.c` |
| DK cargo clones and Falcon-family kick/punch | `ftremixdkcaptainshared.c` | `ftcommonthrown1.c`, `ftcommonthrow.c`, `ftcommonitemthrow.c`, `ftcommondamage.c`, `ftparam.c` |
| Mario fireball clones (capsule, books, eggs) | `wp/wpmario/wpmarioremixfireball.c` | `wpmariofireball.c`, `ftmariospecialn.c`, `ftkirbycopymariospecialn.c` |
| Jigglypuff/Kirby clones (Dedede jump, pipe turn, copy ammo, cutter) | `ftremixpuffkirbyshared.c` | `ftcommonjumpaerial.c`, `ftcommonattack100.c`, `ftcommondokan.c`, `ftmanager.c`, `ftkirbyspeciallw.c`, `wpkirbycutter.c`, others |

`wpLinkBoomerangCheckOwnerCatch` returns `sb32` so the Link family can destroy the boomerang when the owner catches it.
Piano's Neutral Special goes through `ftMarioSpecialNProcAccessory`, which calls `wpRemixPianoBookMake` (one book per ammo count
at its own spread angle) instead of the single-fireball spawn.

Verification: `SSB64_VS_FKINDS="<fighter>,8"` with a scripted P1 used each of the 11 newest fighters' Neutral, Down and Up
Special for 1000 frames (33 runs). None crashed or tripped the watchdog, and each special produced its projectile or effect on
screen and removed it again.

## 10. Polygon fighters crashed at spawn (attributes offset)

**Symptom:** every VS match with a Remix fkind 13..25 (Metal Mario, the polygon fighters) crashed in
`lbCommonSetupFighterPartsDObjs` with a NULL `commonparts_container`; the log had `RelocPointerTable: invalid/stale token` at
`ftmanager.c` (`attr->translate_scales`).

**Cause:** Remix rebuilt those main files with the attributes block at a new offset (`docs/remix/fighter_table_extraction.md`,
e.g. NMario 0x298 -> 0x2B0), but the vanilla `FTData` that the port still uses for fkinds 0..26 kept the old `o_attributes`,
so the attributes were read from the wrong place.

**Fix:** `patchVanillaAttributeOffsets` (`port/remix/remix_roster.cpp`) patches `o_attributes` of fkinds 13..25 from the Remix
blob through `port_fighter_set_attributes_offset` (`ftport.c`).

## 11. Yoshi egg and star files (Dedede crash)

**Symptom:** Dedede's Super Dedede Jump crashed in `portFixupStructU16` from `wpYoshiStarMakeWeapon`.

**Cause:** `yoshishared.asm` replaces the hardcoded `gFTDataYoshiMain` of the egg and star descriptors with the clone's own file
(J Yoshi: main file; Dedede's stars: `DEDEDE_file_7`, the second special file). The patches were never applied, so the
descriptor read a file that is not loaded in the match.

**Fix:** `ftRemixYoshiEggWeaponDesc` and `ftRemixYoshiStarWeaponDesc` (`ftremixyoshishared.c`) return a copy of the vanilla
descriptor with `p_weapon` set to the clone's file.

## 12. Crash's Dig GFX descriptor

**Symptom:** Crash's Down Special crashed in `ftRemixCrashDSPCreateDigGfx` (effect without DObj).

**Cause:** the port built the effect descriptor with no render proc and every offset zeroed, believing the Remix data held
nothing else. `dig_gfx_struct` is Link's spin attack descriptor with its own proc and the offsets 0x230/0x340/0x378/0x394, and
word 0xCC0 of Crash's second special file points at the Dig GFX file (`CRASH_DIG_GFX`) those offsets live in.

**Fix:** the descriptor is filled from `dig_gfx_struct`, the file pointer is resolved from word 0xCC0, and `dig_gfx_main_` is
ported (update, smoke puff at the end) in `ftremixcrashdsp.c`.
