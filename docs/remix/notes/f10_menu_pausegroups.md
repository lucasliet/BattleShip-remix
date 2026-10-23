# F10 notes: Disable HUD pause groups 0xE to 0x10

Scope: `Pause.asm` `disable_hud_` (asm line 75, RAM 0x80113F38) and `Render.asm` `toggle_group_display_` (line 2087).
Follow-up to the F8 notes (`f8_pause.md`, `f8_inputdisplay_hud.md`, "Pause groups 0xE to 0x10").

## What the asm does

- `Render.toggle_group_display_(group, hide)` walks the linked list at `GROUP_TABLE[group]` (0x800466F0 + 4*group) and writes
  `hide` to +0x7C of every node. `next` is at +0x04. These are Remix Render display objects, not `gGCCommonLinks` entries.
- `disable_hud_` runs the vanilla call `0x80113E04` first (= `ifCommonBattlePauseMakeSObjsAll`, the pause decals, vanilla
  `if/ifcommon.c` 3081), then hides groups 0xE, 0xF and 0x10 with `a1 = (Toggles.entry_disable_hud > 0)`. So PAUSE and ALL hide
  them, OFF shows them.
- Objects that go into the groups (all Pause.asm, room 0x18):
  - group 0xE: R legend (`show_legend_helper_`, Pause.asm 181-182).
  - group 0xF: camera legend pieces and the pause track title (`check_legend_toggle_press_` and the legend block, Pause.asm
    ~262-340; `render_live_music_string_` at Pause.asm 370).
  - group 0x10: camera legend pieces (Pause.asm 0x10 draws, ~18 of them).
- Groups 0xE to 0x10 are shared with other Remix code: Training.asm (menus, "Hold A" text), TwelveCharBattle.asm, VsStats.asm,
  Menu.asm (group 0x0E). The group id alone does not identify a feature, only the Pause.asm code does.
- `unpause_camera_` (Camera.asm 844) destroys all objects of groups 0xF and 0x10 (`0x800CB608`) on unpause, and clears the
  magnifying glass in Special Zoom. Group heads are read at 0x8004672C (0xF) and 0x80046730 (0x10).
- Combo meter: `ComboMeter.asm` draws through its own sprite GObjs, not through groups 0xE to 0x10. Its port GObj
  (`ifremixcombometer.c`, `IFREMIX_COMBO_LINK_ID` 0x0B = `nGCCommonLinkIDInterface`) is already covered by
  `ifCommonInterfaceSetGObjFlagsAll`. It is not a group 0xE to 0x10 item.
- `InputDisplay.asm`: no group 0xE to 0x10 draws found. Not ported (F8).

## Port mapping

| Remix piece in groups 0xE to 0x10 | Port state | Action |
|---|---|---|
| R legend, camera legend, pause track title (Render groups) | Not drawn by the port (no Render groups, no pause legend, no music title overlay) | Nothing to hide. REMIX-TODO(Render) kept in `ifCommonBattlePauseMakeInterface` |
| Pause decals (`ifCommonBattlePauseMakeSObjsAll`) | Port GObjs on link 14 (`nGCCommonLinkIDPauseMenu`), not Render groups | Not hidden. The asm does not hide them either |
| Combo meter | Port GObj on link 11 | Not a group item. Already hidden by the interface flags when Disable HUD is ALL |
| Input display, music title overlay, pause combo | Not ported | Nothing to add |

No C API call is needed: none of the Remix pieces in these groups is drawn in the port. Adding one now would hide nothing.

## Edits

- `decomp/src/gm/gmremixcamerapause.c`: removed `gmRemixCameraEjectLegendLink` and its two calls in
  `gmRemixCameraRestorePauseHud`. The defines `GM_REMIX_CAMERA_LEGEND_LINK_FIRST` and `..._SECOND` (0x0F, 0x10) and the
  `lb/lbcommon.h` include went with them. The comment now states the reason and keeps a REMIX-TODO(Render).
  - Reason: the earlier port called `lbCommonEjectGObjLinkedList` on `gGCCommonLinks[0x0F]` and `[0x10]`. Those are vanilla
    links 15 (PlayerSelect) and 16 (SceneCamera), not the Remix render groups. A battle unpause could destroy SceneCamera GObjs
    that Remix never touches. Remix's group 0xF/0x10 contents are not ported, so the correct port of the destroy is a no-op.
  - Default behaviour: the eject runs on every unpause, with the toggles at default. It is removed for all settings. This is
    a change to default behaviour in theory, and it is only observable if links 15 or 16 hold objects at unpause in a battle.
    Lead: check that a pause and unpause in VS, with Disable HUD OFF and Camera Mode NORMAL, shows no change.
- `decomp/src/if/ifcommon.c` (`ifCommonBattlePauseMakeInterface`): the REMIX-TODO(Render) comment now says the asm hides the
  groups after the decal call, and that the port has no GObj for them. The marker is kept. No code changed there.

Syntax check: `syntax-check OK` for `gmremixcamerapause.c` and `ifcommon.c`. The `-Wself-assign` warning at ifcommon.c:2417 predates this change.

## What is left

- Render port (REMIX-TODO(Render)): when the R legend and camera legend are ported, hide their GObjs in
  `ifCommonBattlePauseMakeInterface` after `ifCommonBattlePauseMakeSObjsAll` when `gmRemixToggle(disable_hud) > 0`, and
  destroy them in `gmRemixCameraRestorePauseHud` (the Remix unpause destroys groups 0xF and 0x10).
- Pause D-pad (`check_toggle_hud_press_`, Pause.asm 195): still skipped (needs Render groups, as F8 says).
- Pause track title (group 0xF, `render_live_music_string_`): not ported (REMIX-TODO(BGM)).

## What the lead should check on screen

- Disable HUD = 1 (PAUSE), VS match: pause, and nothing changes compared with OFF (the port draws no pause legend).
- Disable HUD = 2 (ALL): pause, unpause; the interface stays hidden (restore hides it again).
- Default settings (OFF, NORMAL): pause and unpause, the match HUD returns as before. This is the one place the removed eject
  could matter. If a crash or a missing object appears after unpause, the removal is the first suspect.
- Scene id to start with: a VS battle scene for `SSB64_START_SCENE` (id not checked in this pass).
