# Remix: Conker's tail detaches in the up special descent, and other specials break the same way (inherited hidden joints)

Code: `decomp/src/ft/ftmain.c` (`ftMainDropInheritedHiddenJoints`, called from `ftMainSetStatus`),
`decomp/src/ft/ftchar/ftmario/ftmariospecialhi.c` (`ftMarioSpecialHiProcInterrupt`),
`decomp/src/ft/ftchar/ftcaptain/ftcaptainspeciallw.c` (`ftCaptainSpecialLwAirSetStatus`).

## Symptom

Issue #7 (Android, Training): during Conker's up special the tail does not spin and is drawn apart from the body.
Reproduced on Linux with the up special held (status 0xE6, HelicopteryTailThingDescent):
`SHOTS=$(seq -s, 470 2 508) SSB64_VS_STAGE=16 vs.sh 56,8 <input: up+B at 420, B held to 620> conker_usp_desc 520`.
From frame 478 the body lies flat and the tail is a still cone above it; the rising part (0xE3/0xE4) is fine.

## Cause

Same mechanism as `remix_inherited_hidden_joint_sink_2026-11-06.md`: `ftPortCopyMotionArray` merges the parent's
hidden-joint flags into every synth row. Conker's parent is Fox, whose row 0xE6 carries hidden joints; Conker's
descent animation (file 0xC2E) has no track for them, so building them shifts the tail and body tracks onto the wrong
joints. The drop added on 2026-11-06 only ran when the update, interrupt and map routines were all Remix ports. Remix
wires 0xE6 with its own update and physics, no interrupt, and the map routine of Samus' up special (0x8015DD58,
`ftSamusSpecialHiProcMap`), which is vanilla, so the inherited flags were kept.

## Audit of the other Remix fighters

A temporary log listed every special status of every synth fighter whose row carries inherited hidden-joint flags:
219 statuses, 124 of them still kept by the 2026-11-08 rule because a vanilla routine was wired to them. Leaving out the
polygon entries (dropped elsewhere), rows with no animation and rows that reuse the parent's animation file left
Wario, Bowser, Giga Bowser, Mewtwo, Sandbag, Super Sonic, Marina, Dedede, Peppy, Banjo, Dragon King, Peach and Polygon
Ganondorf (the other polygons share these animations).

An A/B run of their NSP, DSP and USP (`inputs/<move>_tap.txt`, shots 430..900 every 10 frames, one game at a time:
parallel runs get different window sizes from the tiling compositor, and only serial runs are pixel-identical) with and
without the inherited flags changed the picture of ten moves. Without them, Marina's NSP no longer draws the head apart
from the body, Dragon King's DSP no longer sinks into the platform, and Banjo's DSP, Dedede's NSP and Peach's NSP lose a
wrong pose.

## Fix

- `ftMainSetStatus` always drops the inherited bits (`ftMainDropInheritedHiddenJoints`), as the Remix ROM builds only
  the hidden joints its own row asks for.
- The functions that read `nFTPartsJointTransN`, `XRotN` or `YRotN` are physics routines that already skip a missing
  joint (`ftPhysicsApplyGroundVelTransN`, `ftPhysicsSetAirVelTransN`, `ftPhysicsGetAirVelTransN`), the common guard,
  cliff, appear and shield-break code (common statuses, dropped since 2026-11-08), the boss, and two vanilla specials
  that read TransN without testing for it: `ftMarioSpecialHiProcInterrupt` and `ftCaptainSpecialLwAirSetStatus`.
  Both now skip the missing joint under `#ifdef PORT`; the N64 wrote to RDRAM page zero there.

## Verification

- `fix_conker_usp_hold` (same input, 424..640 every 8 frames): the tail stays on the body and turns through the descent.
- A/B set again with the fix: identical to the run without inherited flags in all 39 cases (diff 0.00 on every frame).
- `parallel.sh 6 matrix 29..96` (NSP/DSP/USP of every Remix fighter, Linux): 204 runs, `rc=0` with 6 screenshots and no
  error lines. Six runs first timed out while their windows were hidden (each frame took 1 s to present); run again
  alone, they passed.
- Held NSP (`inhale_walker.txt`, P2 human, stage 16) of fkinds 33, 52, 57, 63, 64, 73, held DSP
  (`dsp_hold_charge.txt`) of 33, 52, 57, 63, 64, 68, 71, 73 and aerial specials of 13 fighters: 27 runs, `rc=0`, no
  error lines, no detached or sunk part on the sheets.
- Grabs with a caught P2 (Kirby, human, stage 16, `SSB64_REMIX_FIGHTER_LOG=1` for the statuses): Marina and Dedede
  grab with R and throw forward and back (Catch 166, CatchPull 167, CatchWait 168, ThrowF 169 / ThrowB 170, Marina's
  forward throw goes on to 235); Marina's neutral special grab throws forward, back, up and down (225, 226, then 227,
  228, 229, P2 in 186); Dedede's inhale catches, swallows and spits P2 (231, 232, 234, 235, 236, 237, P2 in 173, 174,
  175). No error lines, and the sheets show no detached part on either fighter.
