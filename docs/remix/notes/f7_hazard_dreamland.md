# F7 hazard notes: Dream Land wind and downtime (Hazards.asm)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 75-85, 2979-3125. Vanilla: `decomp/src/gr/grcommon/grpupupu.c`.
Files: `decomp/src/gr/grremix/grremixdreamland.c/.h` (new, `#ifdef PORT`), `decomp/src/gr/grcommon/grpupupu.c` (guarded edits).

## Patches

| Asm address (RAM) | Asm routine | Vanilla function | Status |
|---|---|---|---|
| 0x80105F34 (ROM 0x81734) | dream_land_wind_ (hazard_toggle of 0x80105BE8) | grPupupuUpdateWhispyStatus | Already applied before this wave (`grRemixHazardToggleIsOn()`), semantics re-checked: vanilla wait runs when hazards are on or outside VS/Training |
| 0x8010666C (ROM 0x81E6C) | dreamland_downtime_initially_ | grPupupuInitAll | Applied: `grRemixDreamLandWhispyInitialWait` after the vanilla random wait |
| 0x80105E10 (ROM 0x81610) | dreamland_downtime_ | grPupupuWhispyUpdateStop | Applied: `grRemixDreamLandWhispyStopWait` after the vanilla random wait |
| 0x80105A74 (ROM 0x81274) | dreamland_wind_speed_multiplier_ | grPupupuWhispySetWindPush | Applied: push decay from `grRemixDreamLandWindDistDecay()` |
| 0x80105A50 (ROM 0x81250) | dreamland_wind_speed_max_ | grPupupuWhispySetWindPush | Applied: push base from `grRemixDreamLandWindSpeedMax()` |

Not in this scope: `dream_land_wind_parameters` (Hazards.asm 3129, RAM 0x801059A0, Dream Greens; the grpupupu Dream Greens block from the earlier wave covers that stage).

## Values (decoded from the asm bits)

- Japanese decay 0x399D4952 = 0.0003F (asm 3071). Super and Hyper decay 0x3A1D4952 = 0.0006F (asm 3066, 3076), equal to the vanilla US decay macro.
- Max speed: Normal 0x40C0 = 6.0 (vanilla base), Hyper 0x41F0 = 30.0. Hyper initial wait 0x100 = 256 frames, Hyper stop wait 0x25 = 37 frames.
- Screen rule: 1P keeps vanilla values, other screens follow the mode (the hazard toggle itself stays VS/Training only).

## asm quirk (kept as the asm behaves)

Hazards.asm 3109-3117: the Super branch compares `t2` (still the Japanese id) with the mode, so it never branches. Super falls through to `lui at, 0x41A0` (20.0) and then `lui at, 0x41F0` (30.0) overwrites it. Super max speed is therefore 30.0 in the asm. The port returns 30.0 for Super. Confirm with the Remix author before changing it.

## REMIX-TODO

- `REMIX-TODO(toggles)`: `Toggles.entry_whispy_mode` has no port accessor. `grRemixDreamLandWhispyMode()` returns Normal, so every Whispy patch is vanilla until the accessor exists. Wire it in `grremixdreamland.c`.
- `REMIX-TODO(screen id)`: `Global.current_screen == 1P` is mapped to `nSCKind1PGame`. Not verified in the game.

## Test

Stage ids (decimal, `SSB64_VS_STAGE`): 6 (Dream Land, `nGRRemixStage_DREAM_LAND`), 120 (Dream Land O), 9 and 10 (Beta). Check which of them use the grPupupu maker. The wait and wind changes only show with the whispy mode wired (above), so until then the lead sees vanilla behavior.
