# F8 notes: Pause.asm toggles (Hold To Pause, Camera Mode, Disable HUD, Play Music)

Source: `.toolchain/pkgs/smashremix-src/src/Pause.asm` (607 lines). Vanilla RAM = ROM + 0x80084800 for these addresses
(`tools/remix_disasm_rom.py`). Vanilla functions from `build/remixasm/refs/vanilla_funcs.txt`. Remix `REMIX_SYM_Toggles__entry_*`
for hold_to_pause, disable_hud, camera_mode and play_music exist in `port/remix/remix_symbols.h`.

Toggles read by Pause.asm (`grep -n "entry_" Pause.asm`): `entry_hold_to_pause` (guard), `entry_disable_hud` (raw `+0x4`),
`entry_camera_mode` (raw `+0x4`), `entry_play_music` (raw `+0x4`).

## Patches

| asm address (ROM) | Pause.asm scope | vanilla function (decomp file:line) | status | notes |
|---|---|---|---|---|
| 0x8011408C (0x8F88C) | `hold_` | ifCommonBattleGoUpdateInterface (`if/ifcommon.c`) | applied | START test of the player loop becomes `ftRemixPauseStartRequested` |
| 0x80113F38 (0x8F738) | `disable_hud_` | ifCommonBattlePauseMakeInterface (`if/ifcommon.c`) | skipped, REMIX-TODO(Render) | the asm hides pause groups 0xE, 0xF, 0x10 through `Render.toggle_group_display_` (Render.asm:2087), a Remix group list at GROUP_TABLE with no port |
| 0x80114320 (0x8FB20) | `extra_controls_` | ifCommonBattlePauseUpdateInterface (`if/ifcommon.c`) | partly applied | camera keep part applied; `set_alternate_music_` and `check_toggle_hud_press_` and `check_legend_toggle_press_` marked REMIX-TODO (see below) |
| 0x80114328 (0x8FB28) | `extra_camera_controls_` | ifCommonBattlePauseUpdateInterface | skipped | calls `Camera.extended_movement_` (Camera.asm), no toggle read in Pause.asm; outside this scope |

Not toggle-gated and not ported (other scopes): `show_legend_helper_` (0x80113EA0), `check_legend_toggle_press_`
(Pause.asm:235), `render_live_music_string_` (Pause.asm:370). They are Render-based HUD code.

### Port details

- `hold_` (Pause.asm:21-71). Toggle off: `ftRemixPauseStartRequested` returns `button_tap & START_BUTTON`, the vanilla test. Toggle on:
  START not held resets the counter of the port to 0 and returns FALSE. START held increments the counter; above
  `FTREMIX_PAUSE_HOLD_FRAMES` (30, NUM_FRAMES) it resets to 0 and returns TRUE. The counter array is file-local, one entry per
  port (the asm `input_table`). The tap bit is not used while the toggle is on, so a short START press does not pause (this is
  the feature: hold to pause).
- `extra_controls_` camera part (Pause.asm:139-150). The asm runs only when the pause kind is the default one (`bne a3, at` with
  `at = 1` = `nIFPauseKindDefault`, checked by the disassembly at 0x80114318). It copies gGMCameraPauseCameraEyeX/Y to
  sIFCommonBattlePauseCameraEyeXOrigin/YOrigin before the stick code, when Camera Mode is SCENE (3). The port puts the copy at the
  top of the default-kind block, before the stick code, same order. `ftRemixPauseKeepsSceneCamera` reads
  `gmRemixToggle(camera_mode)`.

## Toggles

- Hold To Pause: title `Hold To Pause`, bool 0-1, default 0 (off). Run with `SSB64_REMIX_TOGGLES="Hold To Pause=1"`.
- Camera Mode: title `Camera Mode`, range 0-3, default 0 NORMAL. Values: 0 NORMAL, 2 FIXED, 3 SCENE (Camera.asm:23-27). Use
  `SSB64_REMIX_TOGGLES="Camera Mode=3"`.
- Disable HUD: title `Disable HUD`, values 0 OFF, 1 PAUSE, 2 ALL. Not ported (REMIX-TODO(Render)).
- Play Music: title `Play Music`, bool, default on. Not ported (REMIX-TODO(BGM)).

## REMIX-TODO markers added

- `decomp/src/if/ifcommon.c`, camera block of ifCommonBattlePauseUpdateInterface: `REMIX-TODO(BGM)` (set_alternate_music_,
  entry_play_music) and `REMIX-TODO(Render)` (check_toggle_hud_press_, entry_disable_hud; check_legend_toggle_press_ not toggle-related).
- `decomp/src/if/ifcommon.c`, ifCommonBattlePauseMakeInterface before the decal call: `REMIX-TODO(Render)` (disable_hud_).

## Files

- New: `decomp/src/ft/ftremix/ftremixpause.c` (helpers `ftRemixPauseStartRequested`, `ftRemixPauseKeepsSceneCamera`).
- Appended block at the end of `decomp/src/ft/ftremix/ftremixhooks.h` (after the efRemixQuakeSeverity block).
- Edited: `decomp/src/if/ifcommon.c` (3 sites, all under `#ifdef REGION_REMIX`).
- Syntax check: `syntax-check OK` for both files (REGION_REMIX=1 is in the flags). The warning at ifcommon.c:2386 (self-assign) predates this change.

## Uncertainties

- Disable HUD off-case: the asm always calls `toggle_group_display_` (show) on groups 0xE-0x10 even when the setting is off. Not
  ported at all for now, so vanilla behaviour is kept. Revisit with the Render port.
- Hold To Pause counter is not reset when the pause opens. The asm does not reset it either. Its value is kept per port.
- The asm `input_table` is a byte (`lb`, signed). The port uses `s32`; the values stay in 0..31, so there is no difference.

## What the lead should see

- Hold To Pause on, VS match: holding START for about half a second (31 frames) opens the pause. A short START tap does nothing.
  Off: START tap pauses at once, as before.
- Camera Mode SCENE on, VS match: open the pause, rotate the camera with the left stick, unpause, pause again. The restore
  target (sIFCommonBattlePauseCameraEye*Origin) is the pause camera eye of the last default pause frame, as the asm copies it.
  Off or NORMAL: vanilla behaviour. Value 1 of the Camera Mode range is not used by this patch.
- Disable HUD and Play Music (D-pad) have no effect yet. They are REMIX-TODO markers.
