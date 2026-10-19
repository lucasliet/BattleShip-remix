# F8 notes: Accessibility, Anti-Aliasing and FPS settings (ROADMAP T4.2)

Scope: Accessibility.asm (Flash Guard, Screenshake), AA.asm (Disable Anti-Aliasing), FPS.asm (FPS Display *BETA).
Not built or run by the worker. Lead to verify in the game.

## Toggles

| Title (Toggles.asm) | Entry | Values | Default | Read by |
|---|---|---|---|---|
| Flash Guard | entry_flash_guard | 0 off, 1 on | 0 | `gmRemixToggle` (asm reads the entry directly) |
| Screenshake | entry_screenshake | 0 DEFAULT, 1 LIGHT, 2 OFF | 0 | `gmRemixToggle` |
| Disable Anti-Aliasing | entry_disable_aa | 0 off, 1 on | 0 | port registry (`remix_toggle_get`) |
| FPS Display *BETA | entry_fps | 0 OFF, 1 normal, 2 OVERCLOCKED | 0 | port registry (`remix_toggle_get`) |

Override for a run: `SSB64_REMIX_TOGGLES="Flash Guard=1"`, or several with `;`, for example
`"Flash Guard=1;Screenshake=2;Disable Anti-Aliasing=1;FPS Display *BETA=1"`.

What to look for:
- Flash Guard = 1: no white screen flash on a hard hit, on Flashbang, Deku Nut, the Zebes acid or barrel flashes.
- Screenshake = 1 (LIGHT): every screen shake uses the lightest intensity. Screenshake = 2 (OFF): no screen shake at
  all (a heavy smash or a Deku Nut blast).
- Disable Anti-Aliasing = 1: the MSAA slider still shows its value, but rendered geometry has no MSAA (aliased edges).
  Setting it back to 0 restores the slider value on the next frame.
- FPS Display = 1 or 2: the port Stats window opens (ms/frame and FPS). Closing it stays closed until the toggle goes
  off and on again. The value 2 does not change the port measurement.

## Patches (asm address, vanilla function, status)

Accessibility.asm
- `0x80115E10` flash_guard, in `ifScreenFlashMakeInterface` (`decomp/src/if/ifscreenflash.c`, 0x80115DE8). Applied:
  the flash interface is created only when `is_allow_screenflash` is set and `!gmRemixToggle(flash_guard)`.
  Assumption: the asm's 0x800A4930 byte is `gSCManagerBackupData.is_allow_screenflash` (vanilla value 1). Confirm it
  with `build/remixasm/refs/struct_offsets.txt` if the guard does not hit the hard-hit flash.
- flash_guard in Flashbang.asm and DekuNut.asm: applied through the existing `ITREMIX_FLASH_GUARD_ON` in
  `decomp/src/it/itremix/itremixscreenflash.h` (was the placeholder FALSE, now `gmRemixToggle(flash_guard)`).
- Lighting.asm flash checks: not in this scope, not touched.
- `0x800EABF4` / `0x800EB388` ParamMakeEffect save/clear (in `ftParamMakeEffect`, 0x800EABDC, `decomp/src/ft/ftparam.c`):
  skipped. They store the struct that the screenshake patch reads for the Remix RTTF offscreen-CPU skip. The stored
  pointer and the meaning of its flag at 0x018C are not verified, so this part is not applied. Effect: on Remix RTTF,
  offscreen CPUs still shake the screen (vanilla behaviour, not toggle-dependent).
- `0x80100964` screenshake_toggle, in `efManagerQuakeMakeEffect` (`decomp/src/ef/efmanager.c`, 0x801008F4). Applied:
  the switch on the severity is `switch (efRemixQuakeSeverity(magnitude))` under `REGION_REMIX`. LIGHT returns 0,
  OFF returns -1 (no case, takes no shake branch), DEFAULT returns the severity. The priority
  (`3 - magnitude`) keeps the vanilla severity. Uncertainty: the asm sets v1 only, the switch input; I read that as the
  switch value.
  Helper: `decomp/src/ef/efremixscreenshake.c` (`efRemixQuakeSeverity`), declared in `ftremixhooks.h`.

AA.asm
- `0x800010CC`, `0x800011EC` (in `sySchedulerAddPausedQueue`, 0x80000D44) and `0x800027F0` (in `sySchedulerSpTaskDone`,
  0x800020D0): skipped. They are VI antialias register writes; the port has no VI. Disable Anti-Aliasing maps to the port
  MSAA instead (below).

FPS.asm
- `setup_` (overlay objects, Render calls) and `run_` at `0x800053D0` (in `func_800053CC`, `decomp/src/sys/taskman.c`):
  skipped. They are the N64 frame counter and the overlay. The port shows its own frame rate in the Stats window.

## Port mapping (chosen by the worker)

- Disable Anti-Aliasing: port MSAA level. `port/remix/remix_display_settings.cpp` sets the window MSAA level to 1 while
  the toggle is on, else the `gMSAAValue` CVar (the Anti-aliasing (MSAA) slider). Applied every frame from
  `PortPushFrame` (`port/gameloop.cpp`, under `REGION_REMIX`), so the slider cannot override it.
- FPS Display: opens the Stats window (CVar `gStatsEnabled`, `CVAR_STATS_WINDOW_OPEN`) on the off-to-on edge of the
  toggle. The Stats window already shows ms/frame and FPS (`libultraship/src/ship/window/gui/StatsWindow.cpp`).

## Files

- New: `decomp/src/ef/efremixscreenshake.c`, `port/remix/remix_display_settings.h`, `port/remix/remix_display_settings.cpp`.
- Edited: `decomp/src/ef/efmanager.c` (switch), `decomp/src/if/ifscreenflash.c` (flash guard), `decomp/src/it/itremix/itremixscreenflash.h`
  (flash guard accessor), `decomp/src/ft/ftremix/ftremixhooks.h` (block appended before the closing guard, for
  `efRemixQuakeSeverity`), `port/gameloop.cpp` (include and per-frame call).
- The new `decomp/src/ef/*.c` file needs a CMake reconfigure (the `ef` source list is a GLOB without CONFIGURE_DEPENDS).

## Checks

- `scripts/remix/syntax-check.sh` OK for `efremixscreenshake.c`, `efmanager.c`, `ifscreenflash.c`,
  `itremixscreenflash.c` (includes the header).
- The port C++ files were syntax-checked with the flags of `port/gameloop.cpp` from `build/compile_commands.json`
  (`remix_display_settings.cpp` is not in that file yet, so it used those flags). No errors.
