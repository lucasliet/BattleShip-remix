# F14: SpecialZoom stage groups 1 and 2 (REMIX-TODO(Render))

Status: NOT MAPPED. `decomp/src/gm/gmremixspecialzoom.c` is unchanged; both TODO markers remain (lines 127 and 160).

## Remix source

- `SpecialZoom.asm:1591` `temp_toggle_group_display_(a0 = group, a1 = hide)`: walks the Render object list at
  `Render.GROUP_TABLE` (0x800466F0) + `group * 4`. For each object it shifts the word at object `+0x7C` left by 16 bits on hide
  (saves it) and right by 16 on show (restores it). It does not touch GObjs.
- `SpecialZoom.asm:917-922` (apply_zoom, KO flag 1): hides groups 1 and 2, then `FGM.play(152)`.
- `SpecialZoom.asm:1125-1130` (end_zoom): shows groups 1 and 2 again.
- `Render.asm:2087` `toggle_group_display_` is the plain version (writes a1 to `+0x7C` of every object in the list).

## Why there is no port display link to hide

1. Render groups are Remix render-object lists (`Render.draw_*` and `REGISTER_OBJECT_ROUTINE_`), not GObj display list
   links (`gcAddGObjToDLLink` / `dl_link_id`). The port has no equivalent of `GROUP_TABLE` or `ROOM_TABLE`: the only Render
   references in `decomp/src` are TODO comments (`ifcommon.c:3200`, `gmremixcamerapause.c:34`, `gmremixspecialzoom.c`).
2. Group ids are used by menu and screen code, not by stage geometry: group 1 (`Toggles.asm:350`, `Boot.asm:160`), group 2
   (`Stages.asm:2878`, stage select options), groups 0xC to 0xF (`Stages.asm` stage select, `Pause.asm:92` HUD).
   The `.asm` does not say which objects sit in groups 1 and 2 while a battle runs.
3. The stage geometry is GObj display lists; the zoom's KO stage hide is not a Render group hide there, so the group id
   cannot be matched to a stage DL link by name.

## Next step (lead)

Port the Render object list (or document it as a subsystem) before these two lines can be closed. Then re-read which objects
`temp_toggle_group_display_` affects in a battle and add the matching display state save/restore in `gmremixspecialzoom.c`.

## Verification

- `scripts/remix/syntax-check.sh decomp/src/gm/gmremixspecialzoom.c`: see the result in the worker report.
- No build, game or git was run.
