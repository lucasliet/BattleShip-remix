# Remix standard items: ports, spawn extension and generator fix (2026-10-14)

Step 3 of the Remix roadmap. Status: 13 of the 19 standard items are ported, spawn in a match and have their vanilla hooks plus the VS switch menu;
only the Cloaking Device was verified through pickup and effect.

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

- Cloaking Device (kind 45): spawn, fall, landing, pickup, translucency, and damage immunity in a 4-CPU FFA.
- All 14 standard kinds (45 to 55, 57, 64, 65) spawn and fall in a VS match under `SSB64_VS_ITEMS=5 SSB64_REMIX_FORCE_ITEM=<kind>`
  with no abort or assert in `ssb64.log` (the forced-item spawn wait is 60 ticks). Pickup and effect were exercised only
  for Cloaking Device; the others are spawn-verified.
- Hooks landed in decomp `5af34d19d` (all `#ifdef REGION_REMIX`, vanilla intact under `#else`): Cloaking immunity and
  scene-boundary clear (`itRemixClearActiveItems` in `scManagerRunLoop`), Stopwatch hitlag and Speed process hook,
  Golden Gun shoot path, shared Super/Poison Mushroom state (the Poison pickup SIGBUS is gone), Basketball bounce/FGM/hitlag,
  Franklin Badge keep-reflect and immunities.
- VS item switch menu: R pages, L toggle-all, icon strip as a Sprite, save/restore of `gITRemixExtendedToggles`, zeroing in
  the comp ruleset and Training, netplay (peer exchange, replay metadata, settings hash; old replays are rejected).

## Open

- Golden Gun projectile (patches 1 and 3), Franklin Badge reflect wiring in the Reflect subsystem.
- Pickup/effect verification of the 13 items other than Cloaking Device; Training item menu/HUD, taunt spawn,
  match-begin item, Pokemon selection, container toggles, Marina `prevent_item_despawn_`.
- Hazard items (0x3A to 0x3F) depend on the stage step. `Size`, GFX, CharEnvColor, Reflect and Speed subsystems are
  REMIX-TODO (map in `docs/remix/implementation_plan.md`).
- Metal `nextDrawable` SIGSEGV flakes in the headless harness (macOS, not game code); a retry passes.

## Generator bug: `tools/gen_remix_actions.py` wrote garbage into `ftremixvanillaprocs.c`

`vanilla_return_types` read each routine's return type from the line number in `vanilla_funcs.txt`. Those numbers
go stale when a decomp file is edited, so the "type" became half of a statement and the generated `extern`
lines did not compile (about 20 errors). It now checks the hinted line against a definition pattern, falls back to
searching the file for the definition, and stops with an error if none exists. The regenerated file is identical to the
committed one. The script also gained `EXTRA_SYMBOLS` (`Stages.custom_item_spawn_rate_table`, `Stages.series_logo_table`).
