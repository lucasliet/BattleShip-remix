# Item routine GObjs on the effect link corrupt memory when a cloaked fighter lets go of the shield — 2026-10-29

Found by the level 9 CPU matrix of the Remix AI (ROADMAP T7.1): `vs.sh 0,33,34,38` with `SSB64_VS_CPU_LEVEL=9` crashed at about frame 1850 in 3 of 3 runs.

## Symptom

SIGSEGV in `ftParamStopEffect` (fault address 0x90002090900c0) called from `ftCommonGuardUpdateShieldVars` -> `ftParamProcStopEffect`
-> `ftParamRunProcEffect`. The log showed a held Cloaking Device (`ip->kind=45`) destroyed a few frames earlier.

## Cause

`ftParamRunProcEffect` walks `gGCCommonLinks[nGCCommonLinkIDEffect]` and treats the `user_data` of every GObj as an `EFStruct`, calling the
stop proc for each one whose `fighter_gobj` matches the fighter. The Remix item ports (`itremixcloakingdevice.c`, `itremixsupermushroom.c`)
created their per-frame routine GObj on that link and stored an `FTStruct` (cloak) or a routine struct (mushroom) in `user_data`. For a
fighter with an active cloak, the field read as `fighter_gobj` matched, so the stop proc received an `FTStruct` as an `EFStruct`: it read a
garbage `xf`, and would have pushed the `FTStruct` into the effect pool. Any shield release, or any other stop of the fighter's effects, while
cloaked triggered it. The old item sweeps never held a shield with a cloak on, so it stayed hidden until the CPU of level 9 started
shielding after pickups.

## Fix

The routine GObjs of the Cloaking Device, Super Mushroom and Stopwatch (its `user_data` is NULL, so it was only safe by the NULL check) are
created on the interface link, which already hosts the other Remix per-frame routines. The processes run the same way (the cloak decloaks,
`consume` events are still logged).

## Verified

`vs.sh 0,33,34,38` at level 9: 3 of 3 runs finish (7200 frames) where 3 of 3 crashed before; the 16 run level 9 and 10 matrix has no crash;
`item_cpu_sweep.sh 45 46 57` (Cloaking Device, Super Mushroom, Stopwatch) still spawns, touches and consumes the items.
