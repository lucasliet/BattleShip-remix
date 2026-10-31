# F14 notes: Training leftovers (scope training_left, ROADMAP T6.3)

Source: `.toolchain/pkgs/smashremix-src/src/Training.asm`, `Item.asm`, `css/StartWith.asm`, `Render.asm:2406-2420`.
Port: `decomp/src/sc/sc1pmode/sc1ptrainingremix.{c,h}`, `decomp/src/ft/ftremix/ftremixstartitem.c`.
Not built, not run, no git. Syntax check OK on `sc1ptrainingremix.h`.

## Status per task

| Task | State | Where | Why |
|---|---|---|---|
| Steps 10 to 13, Z menu (pages, entries, per-port chain, CPU options, music, BG, hold A to exit) | REMIX-TODO(Training), not implemented | `sc1ptrainingremix.h` | Needs `Training.toggle_menu` (RAM 0x80190979, shared with the vanilla pause menu), the Remix text path in the training scene, and the per-port struct. Nothing of it exists in the port (`grep` for shield break, spam, DI practice, Remix menu finds nothing in `decomp/src/sc/sc1pmode`, `ft/ftremix`, `ai`). Too large to write and verify without a game run in this pass. |
| Step 8, item in hand name in the Training HUD | REMIX-TODO(Training), not implemented | `sc1ptrainingremix.h` | The asm draws the name from file 0xFE `TRAINING_HUD_IMAGE_TABLE` (`Item.asm:311-365`). The port has no held-item name draw for kinds 0x2D+ and no `REMIX_SYM_` for file 0xFE. `sc1PTrainingRemixDrawItemName` draws the Item row of the pause menu, not the held item. |
| Start With | Already ported | `ft/ftremix/ftremixstartitem.c` | Applies at GO (`match_begin_with_item_`, Item.asm:1737) in every mode whose CSS panel value is set. Training is in `APPLIES_TO(0b111111)` (`css/StartWith.asm:12`). The asm training path `Item.start_with_item_` (`Render.asm:2410`) is the same logic. Not seen in game in this pass. |
| Step 14, guards for a one-fighter Training match | Not checked | none | `fix_sector_z_crashes_` (Training.asm:2645), `fix_pokemon_crashes_` (:2666), `skip_crowd_cheer_` (:2730). Decomp equivalents must be read before any change. Only fix if a crash is seen. |

## Notes on the brief

- The brief says `Toggles/Training.asm` for Start With. That file does not exist. Start With is `css/StartWith.asm` (entry "Start With", applies to training), with the table in `Item.asm:1698`.
- `docs/remix/f11_mode_training.md` does not exist. The research note is `docs/remix/notes/f11_mode_training.md`.

## Input script the lead must capture in game (once the Z menu exists)

1. Enter training: main menu, Training, CSS, stage select, START.
2. Pause check (already in port): press START, then B to close.
3. Z menu (not yet): press Z, expect the custom menu with "Port:" first entry; press R and Z for next and previous page; press B to close.
4. Start With: at the training CSS set the Start With panel to "Beam Sword", start, and check the item at GO.
5. Held item name (step 8): pick up a custom item, check the HUD name. Not yet drawn.
6. Guards (step 14): one fighter only in Sector Z, Hitmonlee, Starmie and Blastoise with no opponent, and training start with the crowd cheer toggle on.

## Files edited

- `decomp/src/sc/sc1pmode/sc1ptrainingremix.h`: two `REMIX-TODO(Training)` blocks (steps 8, and 10 to 13 with 14 unchecked). Comment only, no code change.

## Memory

- ai-memory project briefing for `JRickey/BattleShip` failed with "workspace 'JRickey' not found" under both name forms. The global `_rules/` (3 pages) were read in full. The project rules were not loaded.
