# Remix synth fighters: CSS hover crash and parent-routine fallbacks

Seven crashes found by driving the VS character select headless and running every Remix fighter through the attract demo in
pairs. A "synth" fighter is a Remix fighter registered by the port on top of a vanilla parent (`port/remix/remix_roster.cpp`,
`decomp/src/ft/ftport.c`). Code: `decomp/src/mn/mnplayers/mnplayersvs.c`, `decomp/src/ft/ftmanager.c`,
`decomp/src/ft/ftport.c`, `decomp/src/ft/ftcommon/`, `port/bridge/lbreloc_bridge.cpp`. Test: `css.sh` and `vs.sh` with
`SSB64_INPUT_SCRIPT` (`.claude/skills/remix-smoke-tests/SKILL.md`); the attract demo pairs run with
`SSB64_START_SCENE=61 SSB64_DEMO_FKINDS=a,b`.

## Symptoms and root causes

1. **CSS hover SIGSEGV** in `portFixupFTAttributes` (fault `0x94c`). `mnPlayersVSFuncStart` preloads only the 12 vanilla
   playables; hovering a Remix portrait built a fighter whose `*p_file_main` was still NULL.
2. **Neutral-B SIGSEGV** in `wpManagerMakeWeapon` for Sonic, Conker and Mewtwo. Inherited specials run the parent's routines,
   which read the parent's own file globals (`gFTDataFoxMain` for the blaster), never loaded when only the synth was set up.
3. **Color-animation SIGSEGV** (fault `0x2740`) in `ftMainUpdateColAnim`. Remix scripts request extended gfx routines
   (ids `>= 0x56`, `GFXRoutine.asm`), which index past the vanilla `dGMColScriptsDescs` (86 rows).
4. **Animation parse watchdog spam** (thousands of `ftAnimParseDObjFigatree - watchdog` lines, `raw16=[0xffed 0x500e]`).
   Remix's new animation files are unnamed (`reloc_misc/file_NNNN`), so `portRelocIsFighterFigatreeFile` skipped their u16
   halfswap.
5. **Hidden-joint SIGSEGV** in `ftMarioSpecialHiProcInterrupt` (Wario, Goemon and Crash up-B). Remix rows for moves it
   reimplements omit the XRotN/TransN/YRotN flags, so `joints[TransN]` was never built while the parent's routine reads it.
6. **Grab SIGSEGV** in `ftCommonCapturePulledRotateScale` (Bowser). Bowser's ThrowF is a Remix custom action; the parent
   (Yoshi) path used `joint_itemheavy_id = 31`, a joint Bowser's 27-part model never builds.
7. **Release SIGSEGV** in `ftCommonThrownReleaseThrownUpdateStats` (Banjo up-B through Captain's Falcon Dive). The script
   never ran `SetThrow`, so `throw_desc` was NULL.

## Fixes

1. `mnplayersvs.c`: `mnPlayersVSMakeFighter` calls `ftManagerSetupFilesAllKind(fkind)` (idempotent), so Remix fighters load
   on first hover instead of all 30 sitting in the 16 MB arena.
2. `ftmanager.c`: `ftManagerSetupFilesAllKind` also loads the parent's files for synth fkinds.
3. `gmcolscripts.c` exports `dGMColScriptsDescsNum`; `ftParamCheckSetColAnimID` rejects ids out of range. The Remix GFX
   routines are now loaded (`gmremixcolscripts.c`), so the bound is a plain guard.
4. `lbreloc_bridge.cpp`: `portRelocMarkFighterFigatreeFile` (`bridge/lbreloc_figatree.h`); `ftPortCopyMotionArray` marks every
   synth `anim_file_id`.
5. `ftport.c`: synth main-motion rows inherit the parent row's `0xE0000000` hidden-joint flags
   (`ftPortParentHiddenJointFlags`).
6. `ftcommoncapturepulled.c`: a missing capture joint falls back to the grabber's TopN joint.
7. `ftcommonthrown2.c`: `ftCommonThrownGetThrowDesc` returns `dFTCommonThrownNoDamageKnockback` when `throw_desc` is NULL.

Fixes 6 and 7 are guards, not Remix behavior. Each logs once per run when it fires (`ftCommonCapturePulledRotateScale`,
`ftCommonThrownGetThrowDesc`). A special-move matrix of Bowser, Banjo, Crash, Goemon, Captain Falcon, Dedede and Kirby (NSP,
DSP, USP) does not trigger either log line, so the sweeps grep for them.

## Debug tooling

`SSB64_INPUT_SCRIPT=<file>` (`port/port_input_script.{h,cpp}`, applied in `syControllerReadDeviceData`) overrides controller
ports for frame ranges:

```
# <from> <to> <port> <buttons> [stick_x stick_y]
60 103 0 - 80 0
135 137 0 A
170 172 0 START
```

Frames use the `SSB64_MAX_FRAMES` numbering. The CSS cursor moves `stick / 20` pixels per frame.

## Audit rules that came out of it

- Synth fighters run parent status routines: anything those routines read from the parent's files, joints, or per-move script
  state (throw desc, hidden joints) must exist for the synth too.
- File-type detection by reloc path fails for Remix-added files; mark them by id.
