# F7 hazard group Time Twister: notes

Status: PARTIALLY PORTED. The setup runs and returns TRUE, the fade object is created and drawn, and the transition
state machine is written. The transition never starts yet because the warp ball lookup is a REMIX-TODO, so the
screen shows no fade until that is ported.

## Stage id (grremixids.h)
- Time Twister: `nGRRemixStage_TIME_TWISTER = 0xD2`, `function_table` entry `Hazards.time_twister_setup_` (Stages.asm:3398).
- Time Twister SSS (`nGRRemixStage_TIME_TWISTER_SSS = 0xD3`) uses `function.CLONE`: no hazard, not touched.
- Stage file: no `stages/time_twister*.asm` exists; the geometry comes from the map file.

## Source ranges (Hazards.asm)
- 7759-7832 time_twister_setup_: fade object, background image addresses, colors, bgm change flag (-1).
- 7838-8068 time_twister_bg_fade_: warp ball wait, camera zoom, warp sounds, bgm fade and change, alpha steps.
- 7729-7757 data: time_twister_colors, time_twister_glass_colors (same values as the glass colors), time_twister_bgms,
  time_twister_bgm_change.

## Ported (grremixtimetwister.c/.h)
- Setup: state reset (static `sGRRemixTimeTwister`), one ground gobj with update (priority 3) and display (priority 6).
- Transition state machine: timer, alpha steps of 4, reversal at 0xFC, end at 0. The alpha only moves from frame 180.
- Warp sounds at frame 180 (FGM 0x0094) and 270 (FGM 0x059E); FGM 0x059D on start.
- Display: fill rectangle 10,10 to 310,230 with the color and alpha, as the asm rectangle (300 x 220).

## Skipped or TODO
- REMIX-TODO(warp ball): `grRemixTimeTwisterFindWarpBgIndex` always returns -1. The asm walks gobj group 0x2 (RAM
  0x800466F8, top joint, then 4 siblings deep) to the warp ball and reads its part image index as the next bg index.
  Until this is ported the transition never starts.
- REMIX-TODO(camera): the asm sets the fixed camera (0x8010CC74 = `gmCameraAnimFuncCamera`), calls `Camera.zoom_to_max_`
  (Remix routine, no decomp name found) and restores the saved routine at frame 270 (zoomed-max routine 0x8010CCC0 =
  `gmCameraMapZoomFuncCamera`). Camera routine field of `gGMCameraStruct` not named yet.
- REMIX-TODO(bgm): the asm fades the song out (`auSetBGMVolumeSmooth`, 90 frames) and starts the next track at frame
  300. The MIDI ids (CRASH3, NSANITYBEACH, SNOWGO, FUTUREFRENZY) have no numeric values in the port (`midi.asm`
  `insert_midi` order only), so they are not invented here.
- REMIX-TODO(stage header): the asm reads the four background image headers from the stage header (0x48, 0xCC, 0xD0,
  0xD4, then +0x34) and writes the glass color to stage header + 0x4C at the middle alpha. The `glass_color` state
  field is kept for that step and is not read yet.
- REMIX-TODO(bg image object): at the max alpha the asm writes the next background to the bg image object
  (RAM 0x80046800, +0x44). No port of that object exists.
- Training mode check of the asm (skip the fade when `Training.entry_bg + 4` is 0) is not ported: the fade object is
  created in every screen. Harmless while the transition never starts, but needed before the warp ball lookup is done.

## For the lead
- Stage id to run: `SSB64_VS_STAGE=210` (0xD2). The stage should run with the fade object present and no visible
  transition until the warp ball REMIX-TODO is done.
- Item descs: none exported.
- Syntax check OK for `grremixtimetwister.c`, `grremixtimetwister.h`, `grremixhazards_timetwister.c`.
