# F9 announcer group B (asm 590 to 1054)

Phase 2 hooks for the Pokemon Stadium announcer, group B. Public API only (`if/ifremixannouncer.h`). Each hook runs
`ifRemixAnnouncerPlayCall` only when `ifRemixAnnouncerIsActive()` is TRUE, so vanilla is unchanged when the toggle is off.
Not built or run by the worker.

## Applied

| Move | Asm routine (line) | RAM patch | Vanilla function (decomp file) | Enum | Site |
|---|---|---|---|---|---|
| Crash DSP burrow | burrow_announcement_ (889) | none (called by label) | ftRemixCrashDSPBeginInitial (ftremix/crash/ftremixcrashdsp.c) | Burrow | inside `dig_gfx != NULL` |
| Crash DSP dig | dig_announcement_ (964) | none (called by label) | ftRemixCrashDSPEndInitial (ftremix/crash/ftremixcrashdsp.c) | Dig | before DSPEnd SetStatus |
| Crash USP body slam | body_slam_announcement_ (1009) | none (called by label) | ftRemixCrashUSPProcUpdate (ftremix/crash/ftremixcrashusp.c) | BodySlam | after jumps reset |
| Mewtwo DSP disable (ground) | disable_announcement_ (799) | none (called by label) | ftRemixMewtwoDSPGroundInitial (ftremix/mewtwo/ftremixmewtwodsp.c) | Disable | before SetStatus |
| Mewtwo DSP disable (air) | disable_announcement_ (799) | none (called by label) | ftRemixMewtwoDSPAirInitial (ftremix/mewtwo/ftremixmewtwodsp.c) | Disable | before SetStatus |
| Mewtwo USP teleport | teleport_announcement_ (843) | none (called by label) | ftRemixMewtwoUSPMoveSetStatus (ftremix/mewtwo/ftremixmewtwousp.c) | Teleport | before stick check |
| Sonic USP teleport | teleport_announcement_ (843) | none (called by label) | ftRemixSSonicUSPMoveSetStatus (ftremix/ssonic/ftremixssonicusp.c) | Teleport | before stick check |

Each applied hook ends with the vanilla path unchanged. Sites use the marker positions from the REMIX-TODO lines, which were
removed.

## Skipped

| Move | Asm routine | RAM patch | Vanilla function | Reason |
|---|---|---|---|---|
| Pikachu thunder (ground, air) | thunder_announcement_ | 0x80151FC4, 0x80152004 | ftPikachuSpecialLwStartSetStatus (0x80151FBC), ftPikachuSpecialAirLwStartSetStatus (0x80151FFC) | The asm fallback plays the vanilla FGM held in v1, set by code before the hook. The C has no matching FGM call at this point and no REMIX-TODO marker. Needs the ROM disassembly to place the hook and the fallback. |
| Pikachu quick attack (hi, air hi) | quick_announcement_ | 0x801529B0, 0x801529FC | ftPikachuSpecialHiStartSetStatus (0x801529A0), ftPikachuSpecialAirHiStartSetStatus (0x801529EC) | Same as thunder: fallback FGM in v1, no C call site or marker. |
| Samus charge | charging_announcement_ | 0x80168E08 | wpSamusChargeShotMakeWeapon (0x80168DDC), mid-function hook | Hook is at +0x2C of the function. Placing it at the C function start would change when it fires. Needs the disassembly. |
| Jigglypuff sing | sing_announcement_ | 0x80102100 | efManagerPurinSingMakeEffect (0x801020F4), prologue hook | Hook is in the function prologue and the fallback FGM is in v1. No marker in the decomp. Needs the disassembly. |
| Burrow repeat (BurrowAgain) | burrow_announcement_ history | none | dig_match_history (asm byte, cleared by refresh_burrow_binary_) | The asm picks burrow_call (1 entry) on the first burrow of a port and the 3 burrow entries after that. The port uses Burrow every time. The per-port history and its reset are not ported. |
| refresh_burrow_binary_ | (clears dig_match_history) | 0x800D71F0 | ftManagerAllocFighter (ftmanager.c:254) | No announcer call. It only resets the burrow history, which is not ported. |

## Deviations to verify in game

- The asm fallback FGM (played when the announcer does not play) is not played in any applied hook, because the C sites do
  not hold that FGM. When the announcer is active and the pick fails (cooldown or timer), these moves are silent.
- Burrow always uses the first-use clip (`Burrow`). `BurrowAgain` is never played.

## Open for the lead

- Build and run the Crash, Mewtwo and Sonic matches with the announcer on (mode 1 and 0 on Pokemon Stadium).
- Decide on the Pikachu, Samus and Jigglypuff sites above, which need the disassembly from the Remix ROM.
