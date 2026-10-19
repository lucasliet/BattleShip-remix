# F8 Item settings (Item.asm, items/Stopwatch.asm)

Settings in this scope. Override for a run with `SSB64_REMIX_TOGGLES="<title>=<value>"`.

| Title (Toggles.asm) | Entry | Values | Override example |
|---|---|---|---|
| Item Containers | `entry_item_containers` | 0 DEFAULT, 1 OFF, 2 NO_EXPLODE, 3 ALL_EXPLODE | `Item Containers=1` |
| All Items R Drop (Aerial) | `entry_item_dropping` | 0 Off, 1 On (bool) | `All Items R Drop (Aerial)=1` |
| Stopwatch Item | `entry_stopwatch_behaviour` | 0 DEFAULT, 1 SLOW ALWAYS, 2 FAST ALWAYS, 3 BACKFIRE ALWAYS, 4 BACKFIRE NEVER | `Stopwatch Item=3` |

Values for the Stopwatch come from the asm comments in `items/Stopwatch.asm` (lines 74 and 224). The item itself is
already ported (`decomp/src/it/itremix/itremixstopwatch.c`); only its toggle read changed.

## Patches

| Asm (Item.asm unless noted) | RAM | Vanilla function (vanilla_funcs.txt) | Status | Port |
|---|---|---|---|---|
| `item_containers_spawn_toggle` (line 2654) | 0x8016EF64 | `itManagerSetupContainerDrops` (itmanager.c) | applied | `itRemixContainersApplyToggle()` before the remix `if`. Rewrites the low halfword of `item_toggles` (`sh t3, 0xE(t2)`), VS only. |
| `item_containers_explosions` crate_hit (2724) | 0x801796AC | `itBoxCommonProcHit` (itbox.c) | applied | `itRemixContainerKeeps(spawn result)` |
| crate_throw (2739) | 0x80179850 | `itBoxThrownProcMap` (itbox.c) | applied | same |
| crate_throw_sfx (2759) | 0x801795F8 | `itBoxCommonCheckSpawnItems` (itbox.c) | applied | `itRemixContainerSmashFgm()` replaces the crate-open fgm (0x3C) with `nSYAudioFGMExplodeL` (1) on ALL_EXPLODE |
| capsule_hit (2775) | 0x80174004 | `itCapsuleCommonProcHit` (itcapsule.c) | applied | `itRemixContainerKeeps` |
| capsule_throw (2790) | 0x80174184 | `itCapsuleThrownProcMap` (itcapsule.c) | applied | same |
| barrel_roll (2806) | 0x80179C4C | `itTaruCommonProcHit` (ittaru.c) | applied | same |
| egg_hit (2821) | 0x80181650 | `itEggCommonProcHit` (itegg.c) | applied | same |
| egg_toss (2836) | 0x8018181C | `itEggThrownProcMap` (itegg.c) | applied | same |
| `item_drop` (2863) | 0x80150BAC | `ftCommonAttackAirCheckInterruptCommon` (ftcommonattackair.c) | applied | `ftRemixAerialItemIsThrowType(fp)` replaces the `nITTypeThrow` compare |
| Stopwatch.asm `pickup_stopwatch` (line 74) and `clock_out_` (line 224) | none | `itRemixStopwatchGetBehaviour` (itremixstopwatch.c, port of the item) | applied | `gmRemixToggle(stopwatch_behaviour)`, read by the backfire roll and the fast/slow roll |

Not in this scope: `Item.asm` line 883 (`mnVSItemSwitchCheckAllTogglesOff`) reads a toggle from the item switch menu.
The menu is owned by the lead, so it is not touched.

## Files

- New: `decomp/src/it/itremix/itremixcontainers.c`, `decomp/src/ft/ftremix/ftremixitemdrop.c`.
- Edited: `decomp/src/it/itmanager.c`, `decomp/src/it/itcommon/itbox.c`, `itcapsule.c`, `ittaru.c`, `itegg.c`,
  `decomp/src/ft/ftcommon/ftcommonattackair.c`, `decomp/src/it/itremix/itremixstopwatch.c`.
- Declarations appended to `decomp/src/ft/ftremix/ftremixhooks.h` (one block for the containers, one for the drop).

## Deviations and uncertainties

1. Yoshi Island check (OFF mode egg bit): the asm reads the stage byte of `Global.match_info` (line 2682). The port reads
   `gSCManagerBattleState->gkind` (the same value in VS, as used by `grRemixCameraAdjustZoomRange`). Not verified in game.
2. With ALL_EXPLODE the spawn check still runs first, as in the asm: `itBoxCommonCheckSpawnItems` spawns its items, then
   the container explodes. Expect items and an explosion together, not an explosion alone. Same for other hits.
3. The mask write (`andi t3, a2, 0xFFFF0`, line 2678) is truncated to 0xFFF0 by the assembler. The store is a halfword,
   so the result is the same as `& 0xFFF0`. Only the low 16 bits of `item_toggles` change; the high half is kept.
4. The egg and capsule `v0` the asm tests is the result of the spawn call (`itMainMakeContainerItem`). The port tests the
   same result through `itRemixContainerKeeps`.
5. Stopwatch: the port reads the toggle inside each roll helper, not once per pickup. Same value for the whole match.

## What the lead should see (manual, after build)

- `Item Containers=1` (OFF) in VS: no crates, barrels or capsules spawn. Eggs spawn except on the Yoshi Island variants.
- `Item Containers=2` (NO_EXPLODE): hit containers never explode.
- `Item Containers=3` (ALL_EXPLODE): hit containers explode (see uncertainty 2). A crate thrown plays the explosion SFX.
- `All Items R Drop (Aerial)=1`: holding R in the air with a throwable item (Bob-omb, Shell) drops it. Without R the item
  is thrown as in vanilla.
- `Stopwatch Item=1..4`: pickup behaviour as listed above. `=0` must match vanilla.

## Verification

- `scripts/remix/syntax-check.sh` reports `syntax-check OK` for every touched file (itmanager.c, itbox.c, itcapsule.c,
  ittaru.c, itegg.c, ftcommonattackair.c, itremixstopwatch.c, itremixcontainers.c, ftremixitemdrop.c).
- No build and no game run, as briefed.

## Pokemon block (worker report, details in the worker's scratchpad notes)

Scope: `whos_that_pokemon` (0x80173260, itMainMakeMonster), `whos_that_pokemon_setup` (0x8016F218,
itManagerInitMonsterVars), `pokemon_variant_sfx` (10 sites: itkabigon, ittosakinto, itkamex, itmlucky, itdogas,
itpippi, ithitokage, itfushigibana, itglucky).

- New: `decomp/src/it/itremix/itremixpokemonpicker.c`, `decomp/src/it/itremix/itremixpokemonvoices.c`.
- Toggles: `Onix` through `Clefairy` and `Mew` (0 or 1, default 1). `Pokemon SFX` (0 DEFAULT, 1 JAPANESE, 2 RANDOM;
  default 1 in JP, 0 otherwise). Override: `SSB64_REMIX_TOGGLES="Pokemon SFX=1"`.
- Psyduck, Shellder and Staryu are not ported (commented out in the asm).
- RANDOM mode reads `syUtilsRandSeed()` where the asm reads RAM 0x800A0578. This is unverified: confirm that address is
  the seed before trusting RANDOM.
- Item.asm comments on the two Saffron sites are swapped against FGM.asm. The port uses the values.
- With all Pokemon and Mew off, the spawn is a Poke Ball (kind 0x13). Play-test it.
- Mew is rolled without the newcomers unlock check of vanilla, as the asm does.
- CMake needs a reconfigure (GLOB_RECURSE) before the new files are compiled.

## Verification log (2026-10-19)

`SSB64_REMIX_ITEM_LOG=1` now also prints `monster kind=0x..` (Pokemon picker spawn) and `containers_mask kind=0x..` (mask written by `itRemixContainersApplyToggle`). Poke Ball forced with `SSB64_FORCE_ITEM_KIND=19`: only Snorlax ON gave 0x21 twice; all ON gave 0x25 and 0x2A. `Item Containers=1` gave mask 0xFFF8 on Dream Land, DEFAULT and `=3` gave 0xFFFF.
