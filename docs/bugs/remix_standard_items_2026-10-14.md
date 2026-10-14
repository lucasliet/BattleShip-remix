# Remix standard items: ports, spawn extension and generator fix (2026-10-14)

Step 3 of the Remix roadmap. Status: 13 of the 19 standard items are ported and compile; one (Cloaking Device) was
verified in a match; the other twelve, the hooks below and the item switch menu are still open.

## What exists

- `decomp/src/it/itremix/itremix<name>.c/.h` for Cloaking Device, Super Mushroom, Poison Mushroom, Blue Shell,
  Lightning, Deku Nut, Franklin Badge, Pitfall, Golden Gun, Dango, P-Wing, Stopwatch, Basketball and Psyduck. Each exports
  `dITRemix<Name>ItemDesc`; `tools/gen_remix_items.py` lists them in `itremixitems.c` (25 items with the fighter items).
  Standard items use `ITDesc.p_file = &gITManagerCommonData` (file 0xFB, patched in place by Remix).
- Spawn extension (`itremixspawn.c`): Remix item n (kind 0x2D+n) is on when bit `nITKindCommonEnd + 1 + n` of
  `item_toggles` is set (n < 12); the 13th (Stopwatch) uses `gITRemixExtendedToggles` bit 0. Weights are the per-stage
  bytes of `Stages.custom_item_spawn_rate_table` in the Remix RAM image. `itManagerMakeAppearActor` and
  `itManagerSetupContainerDrops` add the enabled, ported items after the vanilla entries.
- Debug: `SSB64_VS_ITEMS=<rate 1..5>` (0 is "None") turns every switch on in the `SSB64_VS_FKINDS` shortcut, and
  `SSB64_REMIX_FORCE_ITEM=<kind>` replaces every stage-spawned item with that kind. Rate 5 spawns the first item
  after about 10 s (~600 frames).

## Verified

Cloaking Device (kind 45), Mario vs Mario on Dream Land: the item spawns with the swirl, falls, lands on the platform,
the CPU picks it up and turns translucent. No crash.

## Open

- Test the other twelve items the same way (`SSB64_REMIX_FORCE_ITEM`), then Training/taunt/match-begin spawns.
- Vanilla hooks recorded by the workers in `build/remixasm/notes/item_<Name>.md` (not versioned): scene-end clear calls
  (`itRemix*Clear*`), Cloaking Device damage immunity and shadow hide, Franklin Badge keep/immunity, Golden Gun
  projectile, Super Mushroom touch continuation, Stopwatch hitlag and Speed check, Basketball bounce/hitlag hooks.
- Super Mushroom and Poison Mushroom keep separate state; Remix shares `spawning_shrooms`/`player_shrooms`.
- Item switch menu (pages, L toggle-all, save), container toggles, Pokémon selection, Marina `prevent_item_despawn_`.
- Hazard items (0x3A to 0x3F) depend on the stage step. `Size`, GFX and Speed subsystems are REMIX-TODO.

## Generator bug: `tools/gen_remix_actions.py` wrote garbage into `ftremixvanillaprocs.c`

`vanilla_return_types` read each routine's return type from the line number in `vanilla_funcs.txt`. Those numbers
go stale when a decomp file is edited, so the "type" became half of a statement and the generated `extern`
lines did not compile (about 20 errors). It now checks the hinted line against a definition pattern, falls back to
searching the file for the definition, and stops with an error if none exists. The regenerated file is identical to the
committed one. The script also gained `EXTRA_SYMBOLS` (`Stages.custom_item_spawn_rate_table`, `Stages.series_logo_table`).
