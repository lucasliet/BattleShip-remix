# F14 scope `menus_left`: Remix menu-screen leftovers (ROADMAP T8.2 `**telas**`)

Status of the six tasks of the brief. Build, game and git were not run (lead's job).

## Done

### (4) Transition logos: `decomp/src/lb/lbtransition.c`, `decomp/src/lb/lbtransition.h`
- Asm: `Transitions.asm` table entries `dh File.TRANSITION_SMASH_LOGO, 0x3C80, 0x4138` and `dh File.TRANSITION_REMIX_LOGO, 0xAB60, 0xAF40` (`File.asm:2587` 0x1262, `File.asm:2847` 0x1366). `NUM_TRANSITIONS = 11 + 2 + 2`.
- Port: two Remix descriptors after `Flip`, same raw-value style as the existing Falling Shapes and Flip rows. `LB_TRANSITION_DESC_COUNT` 13 to 15 (REMIX only).
- Why it is safe: the random pick (`mnvsresults.c:3566`) and the size scan use `ARRAY_COUNT(dLBTransitionDescs)`, so the count follows the asm's 15. The Remix reloc table has 5455 entries (`RelocFileTable.remix.h`), so 0x1262 and 0x1366 are inside it. The old TODO said ids were above 2132, which is the US count.
- Syntax check: OK.
- Not verified in game. The lead must trigger a VS results transition that picks index 13 (Smash Logo) or 14 (Remix Logo) and check the logo draws without a crash or a black frame. Random picks alone may take a while, so forcing the index for one run is the fastest check.

## Not done (blocked or not studied)

### (5) Credits speed assist and glyphs: `decomp/src/sc/sccommon/scstaffroll.c:1475`, `:20`
- Speed assist: the asm (`Credits.asm:1310`) checks that the 2nd player tag is the text `"afk"` (`0x61666B00`), that the name id is `0x55`, and that the speed is not the normal one. The port stores player tags only as an index 0 to 20 (`port/remix/remix_player_panel.cpp:68`). Typed tag text does not exist in the port. This is a missing subsystem (T8.2 names: tags). Left as `REMIX-TODO(credits-speed-assist)`. Needed: the Player Tags text store.
- Glyphs 0x50 to 0x61: needs the Remix glyph textures from `Credits.asm add_new_character`, and the encoder in `tools/gen_remix_credits.py`. Not studied in this pass. Left as `REMIX-TODO(credits-glyphs)`.

### (3) VS results mode rows: `decomp/src/gm/gmremixgameend.c:150`
- Asm: `VsRemixMenu.asm:1007` `add_row_to_results_`, with hooks at `0x80136338`, `0x80136340`, `0x801356D8` (alt texture), `0x80135790` (times), `0x8013567C` (goals), `0x80134BE0` (tens digit), `0x8013670C` (teams), `0x80136540` (no contest row). The row is drawn in the VS results (`mnvsresults.c`). Not started: it changes the results layout for KOTH and Smashketball and needs the port's results drawing extended. Left as `REMIX-TODO(results-mode-rows)`.

### (1) Gallery button and 28 entries: `decomp/src/mn/mncommon/mnremixgallery.c:12`
- Needs the synth fighter kinds (`REMIX-TODO(gallery-remix-fighters)`) and the Gallery.asm entry table. Not studied in this pass. Not changed.

### (2) Character Data emblems and motion names: `decomp/src/mn/mndata/mncharacters.c:1698`, `:2371`
- The TODO text says the asm adds no emblem models and no special-move name sprites for the extended fighters. The brief expects them. That contradicts the TODO and needs a check of `CharacterDataScreen.asm` first. Not changed.

### (6) Item switch legend
- Remix draws the R and L legend in `item_switch_setup_` (`docs/remix/menus.md`, VS item switch section). The port has no legend text draw. No TODO marker exists in `mnvsitemswitch.c`. Not changed. Needs a text draw in the port.

## What the lead must capture in game

- Transition: a VS match end that uses results transition index 13 or 14 (see above). Screen: results transition. Input: normal match end, no R or L held.
- Nothing else changed in game-visible code.

## Memory

- ai-memory: the global briefing worked (`_global`, three rules read in full). No project-scope briefing was available: no `.ai-memory.toml` exists in the repo, and both `BattleShip` and `JRickey/BattleShip` (workspace `JRickey`) returned not found. Project rules were therefore not loaded.
