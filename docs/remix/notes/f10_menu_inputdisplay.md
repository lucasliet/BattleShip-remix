# F10 notes: Input Display (panel option InputDisplay, ROADMAP T5.3)

Scope: `src/InputDisplay.asm` (984 lines), the value from the CSS panel (`REMIX_PANEL_INPUT_DISPLAY`, 0 Disabled, 1 Top,
2 Bottom). Written by the F10 worker. The lead builds and runs the game; the worker did not build, run cmake or commit.
The earlier skip (F8 notes, `notes/f8_inputdisplay_hud.md`) was because the setting had no route to the port. The setting
now comes from the port panel registry (`port/remix/remix_player_panel.cpp`), so the feature is ported as a port overlay.

## Approach

The Remix HUD sprites and the vanilla in-game HUD path are not reused. The overlay is a port ImGui draw: an N64 controller
outline per active port, drawn on the foreground draw list, so it sits above the game image and the menus.

- Feed: `ftMainProcUpdateInterrupt` sends the sample each fighter reads, once per frame, for human fighters (after the
  input delay and D-pad remap, `controller->button_hold` and `stick_range`) and for CPU or key fighters (`cp->button_inputs`
  and `cp->stick_range`). The sample is sent only when the port panel value is not 0.
- Draw: `remix_input_display_update` stores the last sample per port. The overlay window draws each active port, anchored
  to the top or the bottom of the window by its panel value, side by side and centred. Port tint: P1 red, P2 blue,
  P3 yellow, P4 green. Outline scale follows the window height (0.6 to 2.0 of 720 px).
- Shapes: body, L and R triggers, Z, analogue stick (circle with a dot at the stick position, clamped to the 80 range), D-pad
  (four arms), A, B, Start (labelled S), C-up, C-down, C-left, C-right. A shape is white while its button is held.
- Clear: `ftRemixInputDisplayClear` runs in `scManagerRunLoop` at every scene change, next to `itRemixClearActiveItems`, so
  nothing lingers into CSS, results or the next match.

## Edited and new files

| File | What changed |
|---|---|
| `port/remix/remix_input_display.h` | New. C API `remix_input_display_update`, `remix_input_display_clear`, and `remix_input_display_make_window` (C++). |
| `port/remix/remix_input_display.cpp` | New. Drawing, the per-port sample store, and `RemixInputDisplayWindow` (a `GuiWindow` whose `Draw` draws on the foreground list instead of opening an ImGui window). |
| `port/port.cpp` | Include, and under `#ifdef REGION_REMIX` `gui->AddGuiWindow(remix_input_display_make_window())` right after `SetMenu`. |
| `decomp/src/ft/ftremix/ftremixinputdisplay.c` | New (decomp submodule). `ftRemixInputDisplayUpdate` (panel check, then the port call) and `ftRemixInputDisplayClear`. |
| `decomp/src/ft/ftremix/ftremixhooks.h` | Appended declarations for the two functions. |
| `decomp/src/ft/ftmain.c` | `ftMainProcUpdateInterrupt`: one call in the `nFTPlayerKindMan` case (after `ftRemixDpadControlController`), one call in the `next:` path after `cp = &this_fp->input.cp;`. Both under `#if defined(PORT) && defined(REGION_REMIX)`. |
| `decomp/src/sc/scmanager.c` | `scManagerRunLoop`, one call `ftRemixInputDisplayClear();` after `ftRemixKirbyHatsClearCache();`. |

The decomp submodule also has other workers' pending changes (40 files at the time of writing, `ftmain.c` included). Only
the lines above are this feature.

## Asm correspondence

- `src/InputDisplay.asm` `update_input_display_` (asm line 757): the per-frame update. The port draws from the samples, so
  this routine is not ported as a C routine; the sample it reads is the hook above.
- `create_input_display_objects_` (asm line 63) and `setup_` (asm line 23): the N64 HUD objects. Not ported (the overlay
  replaces them).
- `adjust_timer_position_` (asm line 939): moves the match timer for the HUD. Not ported. The port overlay does not move
  the vanilla timer, so with the overlay on the timer stays where it is.

## Differences from the N64 HUD

- Position: the overlay is anchored to the window edge, not to the N64 HUD layout. It does not sit over the stock icons.
- Sample: the outline shows the fighter's input (after input delay and D-pad remap), not the raw pad. R is shown as R only;
  the fighter's R-to-A/Z expansion is not shown.
- Layout is a new drawing, not the Remix sprite layout. Colours are the port colours, not the Remix palette.

## What is left

- REMIX-TODO(InputDisplay): the vanilla timer move (`adjust_timer_position_`) is not applied. Only needed if the overlay
  overlaps the timer, which it does not do at the default size.
- The overlay does not draw while the game is paused (the foreground list is drawn, but the samples are not updated during
  pause, so the last sample stays on screen until the scene changes). Decide with the lead whether to clear on pause.

## What the lead must check on screen

Start a VS match with `SSB64_REMIX_PANEL='InputDisplay.1=1;InputDisplay.2=2'` (use `SSB64_START_SCENE` set to the VS battle
scene id, the case that calls `scVSBattleStartScene` in `decomp/src/sc/scmanager.c` around line 1391).

- P1 outline at the top, P2 outline at the bottom, each in its port colour, both visible during the match.
- Stick dot moves with the stick; A, B, Z, Start, the D-pad arms and the C buttons light up while held; L and R light up on
  the triggers.
- Input Delay on P1 (for example `InputDelay.1=3`): the P1 outline lags the pad by the delay.
- After the match ends (results, CSS) and at the next match start, no outline stays on screen.
- `InputDisplay.1=0` and no override: no outline appears for that port.
