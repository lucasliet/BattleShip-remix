# F16 notes: the per-port panel menu inside the character select screens

Source: `.toolchain/pkgs/smashremix-src/src/CharacterSelectDebugMenu.asm` (2745 lines) and `css/<Item>.asm`. The menu that f10 left as a port
menu (Remix > Player Panel) is now also drawn in the CSS. The registry `port/remix/remix_player_panel.*` stays the single store: the menu reads
and writes it (`remix_panel_get/set`), the ImGui menu and `SSB64_REMIX_PANEL` keep working. Not built, not run (worker rules): the lead tests it.

## What is ported

| Asm | Port |
|---|---|
| `init_debug_menu_` 27, `update_debug_menu_` 79: gold button (File.CSS_IMAGES `0x1208`, 16x16, scale 0.75, tint and palette 0) at panel x + 4, y 130; reset button (`0x1828`) at panel x + 20, y 130, hidden until open | `mnremixcsspanelmenu.c` (`MakeSprites`, `PlaceButtons`), sprites through `ifRemixSpriteMakeFromFile` (links 22 and 28 like the panel card). Sprite headers checked in the `.o2r` (`reloc_misc/file_2566`): both 16x16, `width_img` 16 (already 8 aligned) |
| `setup_menu_page_` 554, `render_menu_page_` 644: 3 items per page, row height 24, label at x + 4 / y + 18 (colour `D0D0D0FF`, scale 0.875, max width W - 8), value centred at x + W/2 / y + 30 (white, max width W - 24) | `mnremixcsspanelmenutext.c` through `remix_hud_text_put_fit`, slots 56..83 (7 per panel, in the slot map comment of `remix_hud_text.h`) |
| `create_item_arrows_` 929, `arrow_state_routine_` 1040, `render_pagination_` 1073: arrows (`0xECE8`/`0xEDC8` of MNPlayersCommon, scale 0.625, tint `FF0000FF`, palette `303030FF`) at x + 2 and x + W - 5, y + 32 + 24 * row; page arrows at x + W - 20 and x + W - 5, y + 6; page number centred at x + W - 10.5, y + 4, shown only with more than one page; blink 0..19 shown below 11 | `mnremixcsspanelmenu.c` (`PlaceItemArrows`, `PlacePageArrows`); one blink timer per panel (the asm keeps one per arrow, started in step with the page arrows) |
| `press_a_handler_` 1272 order: panel button, reset button, item arrows, page arrows; each hit takes the cursor point (+20, +3) against the sprite size (16, or 13 with padding left 0 / right 8 for the item arrows, 0 / 3 and 4 / 0 for the page arrows) | `mnremixcsspanelmenuinput.c` `mnRemixPanelMenuPress`; the CSS calls it in its A-tap chain after the checks the asm also runs first (token grab, fighter select) |
| `hold_a_handler_` 1432: after 15 frames of A, an arrow repeats every 6th frame, from 90 frames on every frame; a page arrow press restarts the wait; sound at most every 3 frames | `mnRemixPanelMenuRulesHoldAdvance`, `mnRemixPanelMenuHoldPress`; the CSS calls it every frame per cursor |
| `handle_item_arrow_press_` 1761: value wraps over min..max; a blank Player Tag name is passed over | `mnRemixPanelMenuStepItem` (tag names come from `remix_panel_tag_name`; with no name typed every tag is blank and the arrows stay on Default) |
| `handle_reset_button_press_` 1621: every item of the human and CPU lists of the mode goes to its default | `ListResetItems` (union of both lists), `mnRemixPanelMenuResetPanel` |
| `add_menu_item` 2477, `write_menu_items` 2575, item order 2720-2739, `APPLIES_TO` and `APPLIES_TO_HUMAN_CPU` of each `css/X.asm`: which items each CSS lists (VS human 18, VS CPU 15, 1P 18, Training human 15 and CPU 12, Bonus 17, All-Star 16); the human/CPU split only applies to VS and Training | `mnremixcsspanelmenurules.c` (unit tested with the numbers above) |
| `set_panel_display_` 317, `prevent_player_model_display_` 2135, `prevent_series_logo_and_name_display_` 2190, `prevent_white_circle_display_` 2224, `disable_panel_buttons_` 2082, 1P/Bonus panel image width 0 | the CSS hands its GObjs to `mnRemixPanelMenuSetHideTargets` every frame; the menu controller (GObj link 32, the last, so its process runs after the CSS ones) forces them hidden while open and gives the flags back at close. The spotlight proc, the HMN/CPU/NA and Team button hit tests, the handicap arrows hit tests, the variant D-pad/flag/bonus hint and the 12CB stock indicator ask `mnRemixPanelMenuIsOpen` |
| Gate `Toggles.entry_css_panel_menu` (default on) | `gmRemixToggle(css_panel_menu)`: off hides the button and ignores presses |
| `check_high_scores_enabled` 2366 | `mnRemixPanelMenuAreHighScoresEnabled` (rule in `mnRemixPanelMenuRulesHighScoresEnabled`), see the deviations |
| `css/StockMode.asm` onchange: outside the default, 12CB, Tug of War and Tag Team modes Stock stays Default | `mnRemixPanelMenuApplyChangeRules` (VS only) |
| `css/Damage.asm` onchange: in Stamina above `Stamina.TOTAL_HP` a right press goes to 0, a left press to the HP | same function, `gGMRemixStaminaRule` and `gmRemixStaminaGetTotalHP` |

Per CSS (all in `decomp/src/mn/mnplayers/`): `mnplayersvs.c` (mode VS, 4 panels, enabled when `door_offset` is 0, present when panel and puck exist, none of
panels 3 and 4 in the 12-Character Battle), `mnplayers1ptraining.c` (mode Training, the human and the CPU panel only, which is what the port's Training CSS has;
the asm loop of 4 is the Remix struct array), `mnplayers1pgame.c` (mode 1P, All-Star when `sc1PRemixModeIs(SC1PREMIX_MODE_ALLSTAR)`, port = `sMNPlayers1PGameManPlayer`),
`mnplayers1pbonus.c` (mode Bonus). The 1P, Training and Bonus CSS now also load `REMIX_CSS_IMAGES_FILE_ID` (VS already did, slot 11).

Registry addition: `remix_panel_value_name(option, value)` (the css/X.asm strings are the `kOptions` strings; Player Tag, Input Delay and Damage are built in the menu:
`Default`/tag name, `None`/`1 frame`/`N frames`, the number).

## Also done here (coordinator message): I7 extra costumes in the CSS

- `mnRemixCostumeApplyButton`: C right and left use `ftRemixPartImageGetNextCostume/GetPrevCostume` (skipped ids, extra costumes, wrap).
- `mnRemixCostumeGetRandomLegal`: rolls over `ftRemixPartImageGetMaxCostumeID` and rerolls until `ftRemixPartImageIsLegalRandomCostume` (bounded to 64 tries, then costume 0).
- `mnRemixCostumeApplyPartImage(fighter_gobj)`: `ftRemixPartImageApplyFighter` on the panel fighter, called every frame from the frame update of each of the four CSS (it is
  idempotent and returns after three reads for a vanilla costume), so it also covers every `ftParamInitAllParts` call (select, costume change, recall) without touching each call site.

## Deviations and open items

| What | Why |
|---|---|
| `SinglePlayer.high_score_enabled` | The port has no such flag and no reader: nothing in `sc/sc1pmode` checks it before it writes a record (only `sc1PRemixBonus3CanSaveRecord` checks the enemy port). The CSS keeps `mnRemixPanelMenuAreHighScoresEnabled()` up to date every frame for the 1P, Bonus and All-Star panel (asm 1729-1740, 1874-1879) so a record writer can read it. The Gameplay and Pokemon toggle checks and the enemy port check of the asm (2396-2417) are not in it |
| `onchange_handler` of Model (`0x800E9198`), Kirby Hat (`0x800E8EAC`/`0x800E8ECC`), Visibility, Size | They redo the CSS fighter model (hi/lo poly, Kirby hat) or fill match tables; the port reads the registry when the match starts, so the value takes effect there, but the CSS model does not change when the arrow is pressed |
| Button shown only when the panel is enabled | The asm creates the button once and always shows it when the toggle is on; in the VS CSS the closed doors cover the panel. Not checked against a closed-door capture, so the port hides it with the doors (and ignores presses, as the asm does) |
| Hide and restore by flags | The asm sets a display flag once (at toggle) for most objects and every frame for the model, the logo and the circle. The port forces every target hidden each frame and restores the flags seen before the hide; if a CSS legitimately hides a target while the menu is open (puck recalled with the menu open) the model can reappear at close until the CSS sets it again |
| Item arrow blink | One timer per panel instead of one per arrow |
| Text over sprites | The text is the ImGui overlay (always above the game), the sprites are game objects; the asm draws both in the game display list. The menu has no text under a sprite, so the order does not show |
| Press order | VS puts the menu press in front of the 12-Character Battle press inside the A-tap block; 1P, Training and Bonus after the puck grab, like the asm hook (`0x801384E8` and the others run after the built-in checks) |

## Test recipe

VS CSS: `SSB64_START_SCENE=16`, `.claude/skills/remix-smoke-tests/scripts/css.sh <input> <name> [frames]`, input lines `<from> <to> <port> <buttons|-> [stick_x stick_y]`
(stick y positive is up). The panel card of VS panel i sits at x = 22 + 69 * i, y = 127 (the log line `css panel menu: port N open, card x= y= width=` prints the real values).
The cursor hot spot is the cursor sprite position + (20, 3); the P2 hand (port 1) starts near (113, 177) and moves about 4 per frame with stick 80.

| Target | Hit box (hot spot) | Cursor sprite position to aim for |
|---|---|---|
| Button of panel i | x in [x+4, x+20], y in [130, 146] | x in [x-16, x], y in [127, 143] |
| Reset (menu open) | x in [x+20, x+36], y in [130, 146] | x in [x, x+16], y in [127, 143] |
| Left arrow of row r | x in [x+2, x+23], y in [159+24r, 172+24r] | x in [x-18, x+3], y in [156+24r, 169+24r] |
| Right arrow of row r | x in [x+W-13, x+W+8], same y | add W - 5 to the left arrow x range |
| Page arrows | y in [133, 146]: left x in [x+W-20, x+W-4], right x in [x+W-9, x+W+8] | subtract 20 from x, y in [130, 143] |

Port 1 (P2) on panel index 1 (x = 91): `60 68 1 - -80 80` (cursor to about (77, 141)), `70 71 1 A` (open: button, panel shows rows Shield, Player Tag, Visibility with a page number 1),
screenshot 76; `76 81 1 - 0 -80` (down to about (77, 161), row 0 left arrow), `82 83 1 A` (Shield 0 goes to 17, text `Costume`), screenshot 88; a long A (`90 160 1 A`) repeats the arrow
(about every 6 frames from frame 105, the value cycles, the log prints `css panel menu: port 1 item 0 = N`); page 2: aim at the right page arrow (use W from the log) and press A, screenshot shows
Skeleton, Model, Input HUD and page number 2; reset: aim at the reset button and press A, the log prints `reset` and the values return to Default; press the button again to close (model, logo and
circle come back). With `SSB64_REMIX_PANEL="Size.2=1"` the Size row of P2 must read `Giant` on page 3.

Costume cycling (I7): `css_pick_mario.txt` plus `170 172 0 CR`, `180 182 0 CR`, ... every 10 frames up to 240: the portrait costume goes 1, 2, 3, 4, then 5, 6, 7 (Mario has vanilla ids 0 to 4
and extras 5 to 7), then wraps to 0; shots at 175, 215, 225, 235, 245 must show 5, 6 and 7 different from 0 and 4. Repeat with Pikachu (the part table has 4 extras and 2 skipped ids, so C right must jump the skipped ids and land on the first extra).
