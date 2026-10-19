# F8 notes: Input Display and Disable HUD (ALL)

Scope: `src/InputDisplay.asm` (984 lines), `src/css/InputDisplay.asm` (55 lines), Disable HUD toggle.
Written by the F8 worker. The lead builds and tests, the worker did not build or run the game.

## Input Display (not ported)

- Value source: `css/InputDisplay.asm` declares the menu item "Input HUD" (`string_table`: Off / Top / Bottom, `MAX_VALUE(2)`,
  `APPLIES_TO(0b111111)`, `VALUE_ARRAY_POINTER(InputDisplay.state_table)`). It is a CharacterSelectDebugMenu item
  (`CharacterSelectDebugMenu.asm` `add_menu_item(InputDisplay)`), not a `Toggles.asm` entry. `Toggles.asm` has no
  input entry (`grep -n input Toggles.asm` only hits menu-input code).
- Consequence: there is no `REMIX_SYM_Toggles__entry_*` symbol for it, and `gmRemixToggle` cannot read it. Port toggles
  come only from the `Toggles.asm` entries (`port/remix/remix_toggles.cpp`, `sToggles`). `port/remix` has no CSS debug menu.
- Runtime: `src/InputDisplay.asm` `setup_` (asm line 23, called from Render.asm 2250/2311/2412/2440 and
  SinglePlayerModes.asm 2385) creates the display objects (`create_input_display_objects_`, asm line 63) and
  `update_input_display_` (asm line 757) runs after character actions. `adjust_timer_position_` (asm line 939) moves the timer.
  None of these has a C port yet, and no `ifremixinputdisplay.c` was written.
- Decision: not ported in this pass. The setting has no route to the port (no toggle, no menu), so ported code would be
  dead. Needs a lead decision first: (a) add an "Input HUD" port setting backed by a CVar (like `remix_display_settings.cpp`
  does for FPS), or (b) change `Toggles.asm` (read-only source, so out of scope here). After that, port the 984 lines in
  `ifremixinputdisplay.c`, one routine per commit.

## Disable HUD (ALL, value 2)

Toggle: "Disable HUD" (`Toggles.asm` line 2399, `entry_disable_hud`, `string_table_disable_hud`: OFF / PAUSE / ALL,
default 0, range 0-2, no single-player guard). Bit `0x0002` means ALL.

Values to test: `SSB64_REMIX_TOGGLES="Disable HUD=2"` (ALL), `=1` (PAUSE), `=0` (OFF).

Shared helper: `ifRemixHudDisabledAll()` in `decomp/src/if/ifremixhud.c` (header `decomp/src/if/ifremixhud.h`). It reads
`gmRemixToggle(disable_hud) & 0x0002`. `gmremixcamerapause.c` now calls it instead of its own static copy.

| Patch | Asm (ROM/RAM) | Vanilla function (C) | Status | What changed |
|---|---|---|---|---|
| Camera `disable_hud_` (after GO) | `Camera.asm` 0x8DA28 -> 0x80112228 | `ifCommonAnnounceGoSetStatus` (`ifcommon.c` ~2240) | Already ported before this pass, now uses the shared helper | `gmRemixCameraHudHiddenOnGo` hides all interface GObjs and clears the magnifying glass |
| Camera `unpause_camera_` | `Camera.asm` (unpause) | `ifCommonBattlePauseRestoreInterfaceAll` (`ifcommon.c` ~3391) | Already ported, now uses the shared helper | `gmRemixCameraRestorePauseHud` hides the interface when ALL, or when the camera is Scene |
| Training init | `Training.asm` 0x116D70 -> 0x80190550 | `sc1PTrainingModeFuncStart` (`sc1ptrainingmode.c` ~1959) | Applied, placement approximate | After `sc1PTrainingModeMakeMenuAll()`, `ifCommonInterfaceSetGObjFlagsAll(GOBJ_FLAG_HIDDEN)` when ALL |
| Training leave menu | `Training.asm` 0x113A4C -> 0x8018D22C | `sc1PTrainingModeCheckLeaveMenu` (`sc1ptrainingmode.c` ~314) | Applied | The show call `ifCommonInterfaceSetGObjFlagsAll(GOBJ_FLAG_NONE)` is skipped when ALL |
| Pause groups 0xE to 0x10 | `Pause.asm` 0x8F738 -> 0x80113F38 (`disable_hud_`, Pause.asm line 75) | `ifCommonBattlePauseMakeInterface` (`ifcommon.c` ~3094) | Skipped: `REMIX-TODO(Render)` kept | Hides the pause HUD groups through `Render.toggle_group_display_` (`Render.asm` 2087). Render group lists (`GROUP_TABLE`, 0x80046728) have no C port. Same blocker as `gmremixspecialzoom.c` 126/159. |
| Pause D-pad `check_toggle_hud_press_` | `Pause.asm` 195 (called from the pause update, `Pause.asm` ~199) | `ifCommonBattlePauseUpdateInterface` (`ifcommon.c` ~3284) | Skipped | Only active when the toggle is OFF (`bnez v0, _end`), so it is not part of ALL. It also needs the Render groups. There is no separate toggle entry for it. |

Notes on the asm:
- Pause.asm 0x80113F38 hook calls `Render.toggle_group_display_` with `a1 = (toggle > 0)`, so PAUSE and ALL both hide the
  pause HUD groups. This means the brief's "PAUSE mode is already ported" holds only for the camera side.
- The Training init hook (asm 0x80190550) starts from `lui t6, 0x8019` inside `sc1PTrainingModeFuncStart`; the C
  statement it maps to was not identified exactly, so the hide is placed at the end of the interface setup, after
  `sc1PTrainingModeMakeMenuAll()`. `ifCommonInterfaceSetGObjFlagsAll` only touches the interface link group
  (`gGCCommonLinks[nGCCommonLinkIDInterface]`), so the training menu, which uses its own link, stays visible.

## What to see (test with `SSB64_REMIX_TOGGLES="Disable HUD=2"`)

- VS match: after "3, 2, 1, GO!" the damage panels, timer and stock icons are hidden. The magnifying glass is off.
- Pause during the match: the restore hides the interface again (no panels after unpause). The pause HUD groups 0xE to
  0x10 are still shown, because of the REMIX-TODO(Render) skip. That is the known gap.
- Training: the HUD is hidden from the start. Opening the menu (START) hides the menu and keeps it reachable. Leaving the
  menu (B or START) does not bring the interface back.
- `Disable HUD=0` (OFF) and `=1` (PAUSE): the match HUD must be visible, so the vanilla behavior is unchanged.

## Files

- New: `decomp/src/if/ifremixhud.c`, `decomp/src/if/ifremixhud.h`
- Edited: `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` (include, init hide, leave-menu guard, all under `#ifdef REGION_REMIX`)
- Edited: `decomp/src/gm/gmremixcamerapause.c` (static helper replaced by `ifRemixHudDisabledAll`, include `if/ifremixhud.h`)
- Not edited: `ftremixhooks.h` (the new helper is declared in its own header), `ifcommon.c` (the REMIX-TODO(Render) stays)
