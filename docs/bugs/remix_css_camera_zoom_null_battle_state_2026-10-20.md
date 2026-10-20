# Remix: any fighter model made on the character select crashed (NULL battle state)

**Status:** FIXED (decomp `grremixcamera.c`).

## Symptom

`SIGSEGV` in `grRemixCameraAdjustZoomRange` called from `ftMainSetStatus` while `mnPlayersVSMakeFighter` built the
fighter model of the character select. Every CSS run that picked a fighter crashed.

## Root cause

The Camera port (`Camera.asm camera_adjust_`) read `gSCManagerBattleState->gkind` at the top of the function and only then
checked whether the current scene is a stage scene. On the CSS the battle state pointer is NULL.

## Fix

The screen check runs first; the stage is read only on VS battle, training and 1P game. The same pattern (a battle state
read before the scene check) was searched for in `grremix/` and `ifremix` code; the other readers run only inside stage code.
