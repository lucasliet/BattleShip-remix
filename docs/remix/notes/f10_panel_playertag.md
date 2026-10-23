# F10 panel: Player Tag (PLAYER_TAG)

Option 0..20, default 0 (Default: the vanilla 1P/2P/3P/4P/CP/Ally tag). Remix reference: `src/PlayerTag.asm`,
`css/PlayerTag.asm`, `src/Toggles.asm` (`head_player_tags`).

## What the asm does

- `use_custom_tag_image_` (src/PlayerTag.asm:18): for an enabled port the in-match indicator uses the arrow only sprite,
  offset `0x1158` in file `0x26` (llIFCommonPlayerTagsFileID), instead of the vanilla tag sprite. The value N is not
  used for the image: every N >= 1 gets the same arrow.
- `create_custom_tag_` (src/PlayerTag.asm:70): for an enabled port it draws the custom name string of the value,
  `CharacterSelectDebugMenu.PlayerTag.string_table[N]` = `Toggles.entry_player_tags_{N-1} + 0x28`, the text typed in the
  Toggles menu (Menu.entry_input, 20 chars). The text is not in the Remix image, so it is not in the port.
- `always_display_custom_tag_` (src/PlayerTag.asm:311): for an enabled port the indicator skips the countdown check
  (`playertag_wait`), so it stays shown.

## Port edits

- `decomp/src/if/ifremixplayertag.c` (new): `ifRemixPlayerTagSpriteOffset`, `ifRemixPlayerTagAlwaysShown`, both reading
  `gmRemixPanel(PLAYER_TAG, player)`.
- `decomp/src/if/ifcommon.c`, `ifCommonPlayerTagMakeInterface` (0x80111BE4): the sprite offset of
  `lbRelocGetFileData(gGMCommonFiles[6], ...)` goes through `ifRemixPlayerTagSpriteOffset` under `#ifdef REGION_REMIX`.
  Asm hook: `use_custom_tag_image_` at 0x80111C9C.
- `decomp/src/if/ifcommon.c`, `ifCommonPlayerTagProcDisplay` (0x80111A3C): the wait check gets
  `ifRemixPlayerTagAlwaysShown(player) ||` under `#ifdef REGION_REMIX`. Asm hook: `always_display_custom_tag_` at
  0x80111A8C. The Visibility alpha hook and the Remix size scale in that function are unchanged.
- `decomp/src/ft/ftremix/ftremixhooks.h`: declarations appended.
- Value 0 keeps the vanilla branch of both hooks.

## Not done, and why

- The custom name text (`create_custom_tag_`, `position_custom_tag_`, `string_table`): the names are typed by the user in
  Remix and are not stored anywhere the port reads. The port shows the arrow only, with no name. With the default empty
  names this matches the asm (the asm skips the text when the string is empty). A port menu text entry would be needed to
  draw the names; not done.
- The results screen tag (`use_custom_tag_image_._results_*`, `create_custom_tag_._results`, mnVSResults at 0x80133D50,
  0x80133DD8, 0x80133E60): out of the in-match scope of this brief. Not applied.
- `REMIX_PANEL_PLAYER_TAG` labels in `port/remix/remix_player_panel.cpp` stay `{}`: the Remix titles are user input, not
  literal strings in the source, so there is nothing to copy. The menu shows the number.
- Runtime check for the lead: the arrow sprite at file `0x26`, offset `0x1158`, must exist in the port's file 0x26. The
  vanilla sprite table ends at `0xEB8` (`include/reloc_data.us.h`); `0x1158` is a Remix addition in that file.
- No `REMIX-TODO` of Player Tag existed in the tree, so none was deleted.
