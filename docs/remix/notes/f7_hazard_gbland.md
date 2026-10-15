# F7 hazard notes: gbland (Game Boy Land, nGRRemixStage_GB_LAND 0x88)

Source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm` lines 2176-2356 (gbland_setup, gb_music_).

## Ported

- `gbland_setup` -> `grRemixGBLandSetup` (grremixgbland.c). Skipped in training mode (`nSCKind1PTrainingMode`).
- `gb_music_` -> `grRemixGBLandMusicProcUpdate`, all branches, with the timer, transition-skip flag and cue table.
- Dispatch: `grRemixHazardGbLand` in grremixhazards_gbland.c, TRUE for `REMIX_SYM_Hazards__gbland_setup`.

## Skipped / approximated

- `Toggles.entry_random_music`: no port accessor. `grRemixGBLandRandomMusicIsOn` returns FALSE (Remix CE default off).
  `REMIX-TODO(toggles)`. With random music on, the port keeps the default-song branch (not the random one).
- No vanilla patch is needed: the setup is reached through `grRemixMakeGround` and the process is a normal GObj proc.
- No `grRemixHazardsAreOn` check: gbland_setup and gb_music_ do not read `Toggles.entry_hazard_mode`.

## Mappings used

- `Global.current_screen` (0x800A4AD0) -> `gSCManagerSceneData.scene_curr`; 0x36 = `nSCKind1PTrainingMode`.
- `Render.CREATE_OBJECT_` (0x80009968) -> `gcMakeGObjSPAfter(nGCCommonKindGround, NULL, nGCCommonLinkIDGround, GOBJ_PRIORITY_DEFAULT)`.
- `0x80008188` (REGISTER_OBJECT_ROUTINE_) -> `gcAddGObjProcess(..., nGCProcessKindFunc, 4)`.
- `0x8013139C` (current song) -> `gMPCollisionBGMCurrent`; `0x801313A0` (stage default song) -> `gMPCollisionBGMDefault`.
- `BGM.play_` (0x80020AB4, sngplayer 0) -> `syAudioPlayBGM(0, bgm)`; `BGM.stop_` (0x80020A74) -> `syAudioStopBGMAll()`.
- FGM 0x800269C0 -> `func_800269C0_275C0(0x03D2)` (Game Boy startup sound).
- `0x801313F0` + 0x60 (timer) and + 0x04 (transition flag) -> static `sGRRemixGBLand` (the brief's rule for Remix stage state).
- MIDI ids from `build/remixasm/bass.log`: MUDA 0x3C, GB_MEDLEY 0xA1, BUBBLY 0xA2, ROADTOCERULEANCITY 0xA3, LEVEL1_WARIO 0xA4, MABE 0xA5.
  These depend on the Remix MIDI bank order; re-check them if the bank changes.

## Uncertainties

- Hammer (0x2D) and star (0x2E) song ids are literals in the asm (`_play`); no named decomp constant was found.
- In the random branch the asm calls BGM.play_ and then `_play` with the same song, so the song starts twice. Ported as written.
- `_transitions` runs only when random music is off and the default song is not the Medley; the Medley path ends the state machine at timer >= 0x188.

## Verification

- `scripts/remix/syntax-check.sh` OK for decomp/src/gr/grremix/grremixgbland.c and grremixhazards_gbland.c.
- Not built or run (per brief). Lead test: `SSB64_VS_STAGE=136` (0x88) and check the Game Boy beep at about frame 180 and the Muda change at frame 392.
