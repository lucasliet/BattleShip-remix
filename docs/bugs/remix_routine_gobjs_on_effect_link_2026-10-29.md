# Item routine GObjs on the effect link corrupt memory when a cloaked fighter lets go of the shield

Code: `decomp/src/it/itremix/itremixcloakingdevice.c`, `itremixsupermushroom.c`, `itremixstopwatch.c`;
`ftParamRunProcEffect` in `decomp/src/ft/ftparam.c`. Test: `vs.sh 0,33,34,38` with `SSB64_VS_CPU_LEVEL=9` (a level 9 CPU
shields after pickups) and `item_cpu_sweep.sh 45 46 57` (Cloaking Device, Super Mushroom, Stopwatch)
(`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

SIGSEGV in `ftParamStopEffect` (fault address 0x90002090900c0) called from `ftCommonGuardUpdateShieldVars` ->
`ftParamProcStopEffect` -> `ftParamRunProcEffect`, about frame 1850 of the level 9 CPU run (3 of 3 runs). The log showed a
held Cloaking Device (`ip->kind=45`) destroyed a few frames earlier.

## Root cause

`ftParamRunProcEffect` walks `gGCCommonLinks[nGCCommonLinkIDEffect]` and treats the `user_data` of every GObj as an `EFStruct`,
calling the stop proc for each one whose `fighter_gobj` matches the fighter. The Remix item ports created their per-frame
routine GObj on that link and stored an `FTStruct` (cloak) or a routine struct (mushroom) in `user_data`. For a fighter with an
active cloak, the field read as `fighter_gobj` matched, so the stop proc received an `FTStruct` as an `EFStruct`: it read a
garbage `xf`, and would have pushed the `FTStruct` into the effect pool. Any shield release, or any other stop of the fighter's
effects, while cloaked triggered it. Item sweeps that never held a shield with a cloak on did not show it.

## Fix

The routine GObjs of the Cloaking Device, Super Mushroom and Stopwatch (its `user_data` is NULL, so it was only safe by the NULL
check) are created on the interface link, which already hosts the other Remix per-frame routines. The processes run the same
way (the cloak decloaks, `consume` events are still logged). Rule: a Remix per-frame GObj never goes on a link that vanilla code
walks and casts.

## Verification

`vs.sh 0,33,34,38` at level 9: 3 of 3 runs finish (7200 frames) where 3 of 3 crashed before. `item_cpu_sweep.sh 45 46 57` still
spawns, touches and consumes the items.
