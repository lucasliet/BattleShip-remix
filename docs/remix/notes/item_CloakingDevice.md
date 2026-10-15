# Item CloakingDevice (0x2D): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/CloakingDevice.asm`.
Port: `decomp/src/it/itremix/itremixcloakingdevice.c`, `decomp/src/it/itremix/itremixcloakingdevice.h`.
Descriptor: `const ITRemixItemDesc dITRemixCloakingDeviceItemDesc`, kind 0x2D.
Spawn function for fighter call sites: `GObj* itRemixCloakingDeviceMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.
Extra export: `void itRemixCloakingDeviceClearStates(void)`, for the scene-end hook (see Vanilla patches).

## Item file and attributes

- `ITDesc.p_file = &gITManagerCommonData` (file 0xFB, the `dw 0x8018D040` of the asm). Remix patched file 0xFB in place, so the item is not owned by a fighter and there is no owner file.
- `o_attributes = 0x0D40`, the asm `FILE_OFFSET`.
- The vanilla tomato spawn code (`spawn_custom_item_based_on_tomato_`, copy_segment 0xEF064 and 0xEF08C) only swaps `a1` for the item info array. The port calls `itManagerMakeItem` with the static desc directly, the same as MrSaturn.
- The asm has no `item_state_table`. The item runs the vanilla tomato status table, so the desc has no status table of its own.

## Routine map

| Remix label | C | Role |
|---|---|---|
| stage_setting (spawn_custom_item_based_on_tomato_) | itRemixCloakingDeviceMakeItem | SPAWN_ITEM |
| pickup_cloaking_device_ | itRemixCloakingDevicePickupMain | PICKUP_ITEM_MAIN |
| (inline in pickup_cloaking_device_) | itRemixCloakingDeviceStart | starts the cloak, creates the routine GObj |
| handle_active_cloaking_device_ | itRemixCloakingDeviceProcUpdate | per-frame routine, timer and decloak |
| (inline in handle_active_cloaking_device_) | itRemixCloakingDeviceGetState | helper, state by `fp->player` |
| clear_active_cloaking_devices_ | itRemixCloakingDeviceClearStates | reset of the state table |
| (drop) 0x801745FC | itTomatoDroppedSetStatus | DROP_ITEM, vanilla routine |
| prevent_damage_when_cloaked_ | not ported | vanilla patch, see below |
| prevent_shadow_when_cloaked_ | not ported | vanilla patch, see below |

Scopes ported: 3 of 5 (`pickup_cloaking_device_`, `handle_active_cloaking_device_`, `clear_active_cloaking_devices_`), plus the spawn (`spawn_custom_item_based_on_tomato_`). The two `OS.patch_start` scopes are not ported. The `ITDesc` hit, shield, hop, set-off, reflector and damage procs are NULL, as in the asm.

## Per-player state

- `sITRemixCloakingDeviceStates[ITREMIX_CLOAKINGDEVICE_PORTS]`, indexed by `fp->player`. Fields: `is_active` (the asm `cloaked_players[port]` flag plus the routine presence) and `timer` (asm `0x4C` of the handler).
- The routine GObj is created with `gcMakeGObjSPAfter(nGCCommonKindEffect, NULL, nGCCommonLinkIDEffect, GOBJ_PRIORITY_DEFAULT)`, its `user_data.p` holds the fighter, and it runs as `gcAddGObjProcess(..., nGCProcessKindFunc, 3)`.
- Asm deviation: the asm adds a new handler on every pickup. The port keeps one routine per port: a pickup while cloaked restarts the timer. The visible result is the same, since the asm's second handler only resets the shared override.

## Vanilla functions used

| RAM | Decomp name |
|---|---|
| 0x8016E174 | itManagerMakeItem |
| 0x801745FC | itTomatoDroppedSetStatus |
| 0x801744C0 | itTomatoFallProcUpdate |
| 0x80174524 | itTomatoFallProcMap |
| 0x80145C4C | itRemixConsumeTomatoTail (consume tail, shared with other Remix items) |
| 0x800269C0 | func_800269C0_275C0 (FGM play; 0x529 cloak, 0x52A decloak) |
| 0x80008188 | gcAddGObjProcess |
| 0x80009A84 | Render.DESTROY_OBJECT_ in the asm, port uses gcEjectGObj |

## Vanilla patches (not applied, record only)

1. **prevent_damage_when_cloaked_**: `OS.patch_start(0x65A68, 0x800EA268)`. RAM 0x800EA268 is 0x20 bytes into `ftParamUpdateDamage` (`decomp/src/ft/ftparam.c:1699`, start 0x800EA248). Behaviour: if the port's cloak flag is set, `new_damage = current_damage` and `move_damage = 0`. Shape of the C to add, under `#ifdef REGION_REMIX`: a call to `itRemixCloakingDeviceIsCloaked(fp->player)` (not yet written, it would read `sITRemixCloakingDeviceStates[port].is_active`), which then zeroes the damage and the move damage in `ftParamUpdateDamage`. The exact insertion point needs a read of `ftparam.c` lines 1699 onward; not done here.
2. **prevent_shadow_when_cloaked_**: `OS.patch_start(0xB5A14, 0x8013AFD4)`. RAM 0x8013AFD4 is inside the fighter shadow code. `build/remixasm/refs/vanilla_funcs.txt` has no entry covering that address, so the enclosing decomp function is not confirmed. The closest site is `decomp/src/ft/ftshadow.c:111` (`if (!(fp->is_invisible) && !(fp->is_shadow_hide))`), where the port already hides shadows for invisible fighters. Behaviour: when cloaked, the shadow is not drawn, so `is_invisible` covers this case in the port.
3. **clear_active_cloaking_devices_**: the asm has no hook in this file. The caller is a Remix scene-end path that is not in this file. The C to call is `itRemixCloakingDeviceClearStates();` at scene end, under `#ifdef REGION_REMIX`. Caller not identified.
4. **pickup dispatch**: none. The `j 0x80145C4C` tail is covered by `itRemixConsumeTomatoTail`.

## REMIX-TODO

- `REMIX-TODO(CharEnvColor)`: the env color override fade (32 frames), the speed-based opacity and the end-of-cloak flicker are not ported. The port uses `fp->is_invisible` (binary).
- `REMIX-TODO(CharEnvColor)`: the asm forces the full-charge sparkle off (`0x0A30`/`0x0A34` of the fighter). Not ported.
- `REMIX-TODO(vanilla patch)`: the damage immunity (patch 1) and the scene-end clear (patch 3) need the vanilla edits above before the cloak is complete.

## Uncertainties

- Process order `3` for the routine GObj (asm `Render.register_routine` uses group 0x0001, order 0x8000). The render group and order have no port equivalent that was checked.
- `gcEjectGObj(routine_gobj)` is called from inside its own process. Vanilla item code does the same for arrows, but this path is not verified at runtime.
- The asm handler runs its decloak check after the timer update (asm lines 106 to 110), then the death check (asm lines 112 to 114, `lhu` of action at 0x26, `sltiu 4`). The port uses `fp->status_id <= nFTCommonStatusDeadUpFall`, the same low-16-bit range, because `status_id` is the 32-bit field at 0x24 of the fighter struct.
- The asm sets the cloak flag at pickup time (asm line 97 to 98), before the first handler frame. The port sets `is_invisible` on the first routine frame, so the cloak appears one frame later.

## Hooks applied 2026-10-14 (this session)

- Vanilla patch 1 (prevent_damage_when_cloaked_): applied — itRemixCloakingDeviceIsCloaked(fp->player) zeroes damage at the top of ftParamUpdateDamage (ftparam.c), after the Marth DSP hook.
- Vanilla patch 3 (scene-end clear): applied — itRemixClearActiveItems() aggregator (itremixspawn.c) called at the scManagerRunLoop scene boundary (scmanager.c, after the gGRCommonStruct memset).
- Patch 2 (shadow): stays covered by fp->is_invisible as recorded below.
- CharEnvColor fade/flicker and sparkle-off: still open REMIX-TODOs.

## In-match verification 2026-10-14

- 4-CPU FFA, forced cloak, 2400 frames: 'cloak immunity — damage 14 blocked on port 3 (percent stays 0)' in ssb64.log — a cloaked port took a real hit and the ftParamUpdateDamage hook blocked it. Invisibility also vision-verified earlier (players vanish from live scenes; both cloaked simultaneously in one run).
- Harness note: 1v1 with an idle/patrol P1 produces no CPU aggression; use 4-CPU FFA (SSB64_VS_FKINDS=0,0,0,0) when in-match damage behavior needs exercising.
- A port_log line fires on each blocked hit (ftparam.c, PORT-only) — rare event, useful for future sweeps.
