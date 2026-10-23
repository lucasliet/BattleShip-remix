# F10 menu: VS Game Mode page 2 "Remix Modes" (VsRemixMenu.asm, menus.md 3.2)

Status: code written and syntax-checked (`scripts/remix/syntax-check.sh` OK for both files). Not built, not run, not committed.

## What is in

- Page 1 gets a 5th button, "Remix Modes" (cursor index 4). Up and down wrap over 0..4 on both pages. The four vanilla buttons keep their behaviour.
- A on "Remix Modes" switches to page 2 in place. Page 2 has five buttons: 12-Character Battle, Tag Team, King of the Hill, Smashketball, Tug of War (cursor 0..4).
- B on page 2 returns to page 1 with the cursor on "Remix Modes". B on page 1 is unchanged (leaves to mode select).
- A (or START) on a page 2 button stores the mode in `gMNVSModeRemixMode` (DEFAULT 0, TWELVE_CB 1, TAG_TEAM 2, KOTH 3, SMASHKETBALL 4, TUG_OF_WAR 5), then leaves for the VS CSS exactly as VS Start does: `mnVSModeSaveSettings()`, `sMNVSModeExitInterrupt = TRUE`, `scene_curr = nSCKindPlayersVS`.
- `gMNVSModeRemixMode` is reset to DEFAULT on every entry to the menu (`mnVSModeFuncStartVars`), so VS Start, VS Options and the Remix page-1 button all leave it at DEFAULT.

## Files

- New `decomp/src/mn/mnvsmode/mnremixvsmode.c`: owns `gMNVSModeRemixMode`, the row table (box and text position, text offset in the MNVSMode file) and `mnRemixVSModeMakeButton(row, file)`. Rows 0..4 are the page 2 modes in asm order, row 5 is the page 1 "Remix Modes" button. Box positions are the `button_positions` table of the asm, text positions and offsets are `remix_menu_button_table` and `menu_button_table` (VsRemixMenu.asm around lines 84-100 and 147-157). Constants are copied from the asm text, nothing is read from the ROM image.
- New `decomp/src/mn/mnvsmode/mnremixvsmode.h`: mode enum, page and option constants (outside the guard, so vanilla code can compare the page), `gMNVSModeRemixMode` and `mnRemixVSModeMakeButton` (under the `PORT && REGION_REMIX` guard).
- Edited `decomp/src/mn/mnvsmode/mnvsmode.c`:
  - Statics `sMNVSModePage` and `sMNVSModeRemixButtonGObjs[5]` (not guarded: always 0 / NULL in vanilla).
  - New helpers (remix guard unless stated): `mnVSModeGetButtonSlot` (unguarded, replaces the local `buttons[]` array; same pointers in vanilla), `mnVSModeMakePage1Buttons` (unguarded, the six vanilla make calls moved out of `mnVSModeFuncStart`, same order), `mnVSModeClearRemixButtons`, `mnVSModeEjectGObjRef`, `mnVSModeEjectPageGObjs`, `mnVSModeMakeRemixButton`, `mnVSModeMakeRemixButtons`, `mnVSModeSetPage`, `mnVSModeSelectRemixMode`.
  - `mnVSModeMain`: A block (page 2 select first, then case 4 "Remix Modes"), B block (page 2 back to page 1), up and down wrap (`MNREMIX_VSMODE_OPTION_LAST` under the remix guard, Options in vanilla), highlight through `mnVSModeGetButtonSlot`, Rule and TimeStock arrow flags only on page 1 (the arrow GObjs are NULL on page 2), and an early return before the L/R switch on page 2.
  - `mnVSModeFuncStartVars`: page reset, remix mode reset, remix slots cleared.
  - `mnVSModeFuncStart`: calls `mnVSModeMakePage1Buttons()` and then `mnVSModeMakeRemixButtons()`.

## Differences from the asm

- The asm reloads screen 9 for each page change (`page_2_exit`, `create_button_generic_` screen jump). The port rebuilds the buttons in place. Visually this should match, but the scene is not reloaded.
- The asm on a page 2 mode also sets `TwelveCharBattle.twelve_cb_flag` and `vs_mode_flag`, and reads them in the CSS and match hooks. The port only stores `gMNVSModeRemixMode`, as the brief asked. The 12CB, Tag Team and other rules are not active yet (M6).
- The asm's cursor on page 2 is set by `set_cursor_index_`. The port starts page 2 on 12CB (index 0).
- START on page 2 selects the highlighted mode, as START does for VS Start on page 1.
- The brief named `gSCManagerBattleState->game_type/ruleset`. Those names do not exist in the decomp. The vanilla VS Start path writes `gSCManagerTransferBattleState.game_rules` through `mnVSModeSaveSettings()`, which the port reuses.
- The text of the new buttons is read as a `Sprite` at `sMNVSModeFiles[1] + offset`, like the vanilla texts. The asm draws them with `Render.TEXTURE_INIT_` (`.toolchain/pkgs/smashremix-src/src/Render.asm:1223-1283`, an image footer struct), so the port reuses the vanilla sprite path. If the text is garbled or missing, this is the first suspect: the Remix footer may need its own texture path.

## Scene and input for the lead

- Start the scene: `SSB64_START_SCENE=9` (VS_GAME_MODE_MENU). The menu opens with VS Start highlighted.
- Input script (format `<from> <to> <port> <buttons|-> [stick_x stick_y]`, stick down is `0 -80`). Example for Tug of War:

```
100 108 0 - 0 -80
130 138 0 - 0 -80
160 168 0 - 0 -80
190 198 0 - 0 -80
220 222 0 A
260 268 0 - 0 -80
290 298 0 - 0 -80
320 328 0 - 0 -80
350 358 0 - 0 -80
380 382 0 A
```

  Frames 100-190 move the cursor from VS Start to "Remix Modes" (index 4), 220 opens page 2, 260-350 move to Tug of War (index 4), 380 selects it and leaves for the VS CSS. Run it with `SSB64_INPUT_SCRIPT=<file>` and the usual remix-smoke-tests environment (`css.sh` or a custom scene 9 run).
- What to check on screen:
  - Page 1: five buttons, "Remix Modes" at the bottom-left under "VS Options", highlight wraps from VS Start up to "Remix Modes".
  - The text of "Remix Modes" and of the five page 2 buttons is readable and in the right place (see the Sprite-vs-texture risk above).
  - Page 2: no Rule, Time/Stock, Options or VS Start buttons, no arrows, no rule or time values.
  - B on page 2 returns to page 1 with "Remix Modes" highlighted. B on page 1 still leaves to mode select.
  - Choosing a page 2 mode goes to the VS CSS. `gMNVSModeRemixMode` should equal 1 + the button index (`p gMNVSModeRemixMode` at a break on `mnVSModeSelectRemixMode`, or after the CSS starts).
  - Vanilla VS menu: the four buttons, the wrap from VS Start up to VS Options, and VS Start / VS Options still behave as before.

## Left to do

- Remix modes themselves (M6): the CSS and match code must read `gMNVSModeRemixMode`. Nothing does yet, so every page 2 choice plays as a normal VS match.
- The asm's 12CB flag (`TwelveCharBattle.twelve_cb_flag`) is not set.
- Splitting `mnvsmode.c` further: it grew by about 200 lines. The page logic could move to its own file if the lead wants it, but it needs the statics.

## ai-memory

The project scope for BattleShip could not be resolved: `memory_briefing` with `project: "BattleShip"` returned "project not found", and there is no `.ai-memory.toml` at the repo root. Only the three `_global` rules pages were read, and the project rules were not loaded.
