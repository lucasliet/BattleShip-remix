# Item 0x38 MrSaturn notes

Port: `decomp/src/it/itremix/itremixmrsaturn.c`, header `decomp/src/it/itremix/itremixmrsaturn.h`.
Descriptor: `dITRemixMrSaturnItemDesc` (`ITRemixItemDesc`, kind 0x38 = `Item.MrSaturn.id`, 0x2D + 11 in `Item.asm` `add_item` order).
Source: `.toolchain/pkgs/smashremix-src/src/items/MrSaturn.asm` (705 lines).

## Spawn signature for fighter code

`GObj* itRemixMrSaturnMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

Mapping from the Remix `stage_setting_` register use: a0 = parent (unchanged), a1 = pos, a2 = vel,
a3 = flags (s0). This is the same ABI as `itManagerMakeItem`, so no wrapper is needed.
Fighter call sites are wired by another worker.

## Item file

`item_info_array` word 0x04 is `0x8018D040`, documented in `Item.asm` line 2076 as "hard-coded
pointer to file 0xFB". The decomp declares that address as `gITManagerCommonData` (`it/item.h`,
`itmanager.c`). The port sets `ITDesc.p_file = &gITManagerCommonData`, the same as the Tomato item.
The owner-fighter `p_file_special*` override from the brief does not apply: this item does not use
a character file.

Attributes: `FILE_OFFSET` 0x10B0 (word 0x08) goes into `o_attributes` as a raw offset.
REMIX-TODO(file): the footer at 0x10B0 is inside file 0xFB, which is shared with vanilla
item data. The Remix build must add Mr. Saturn data to that file, and the port's file 0xFB
must contain it at 0x10B0. Not verified in this pass (no build run, per brief). Check with
`lbRelocGetFileData` at runtime, or add a reloc entry if the port resolves 0xFB through relocs.

## Descriptor fields

| Field | Value | Source |
|---|---|---|
| `ITDesc.transform_types` | `{nGCMatrixKindTraRotRpyR, nGCMatrixKindNull, 0}` | word 0x0C = 0x1B000000; 0x1B = `nGCMatrixKindTraRotRpyR` (objdef.h) |
| `attack_state` | `nGMAttackStateOff` | word 0x10 = 0 |
| `proc_update` | `itTomatoFallProcUpdate` (0x801744C0) | word 0x14 |
| `proc_map` | `itRemixMrSaturnGroundProcMap` (`ground_collision`) | word 0x18 |
| `is_show_spawn_gfx` | TRUE | `SHOW_GFX_WHEN_SPAWNED(OS.TRUE)` |
| `proc_hold` | `itRemixMrSaturnHoldProc` (`prepickup_`) | `PICKUP_ITEM_INIT` |
| `proc_consume` | NULL | `PICKUP_ITEM_MAIN(pickup_)` is a no-op, see below |
| `proc_dropped` | `itRemixMrSaturnFallSetStatus` (`falling_initial_`) | `DROP_ITEM` |
| `proc_thrown` | `itRemixMrSaturnThrowSetStatus` (`throw_initial_`) | `THROW_ITEM` |
| `proc_touch` | NULL | `PLAYER_COLLISION(0)` |

## Item state table (ITStatusDesc order = ASM word order)

The eight words per state map one-to-one to `ITStatusDesc`: update, map, hit, shield, hop, setoff,
reflector, damage. The state indexes match the Remix `addiu a2, r0, N` calls:

| State | Name in port | update | map | hit | shield | hop | setoff | reflector | damage |
|---|---|---|---|---|---|---|---|---|---|
| 0 | GroundWait | `wait_main` | `grounded_wait_collision` | | | | | | |
| 1 | AerialWait (never entered by Remix) | 0x801744C0 | `ground_collision` | | | | | | |
| 2 | Held | `held_main` | | | | | | | |
| 3 | Thrown | `thrown_main_` | `ground_collision` | `throw_collide_` | `throw_collide_` | 0x801733E4 (`itMainCommonProcHop`) | `throw_collide_` | `reflect_` | `throw_collide_hitbox_` |
| 4 | Fall | 0x801744C0 | `ground_collision` | | | | | | |
| 5 | Walk | `walking_main` | `grounded_check_collision_` | | | | | | |

## Routines ported (asm scope, C routine)

Scopes in the asm: 22. Functions ported: 20. `pickup_` is NULL in the descriptor. `detect_collision_`
is skipped.

| Asm scope | Line | C routine |
|---|---|---|
| `grounded_check_collision_` | 110 | `itRemixMrSaturnGroundedCheckProcMap` |
| `grounded_wait_collision` | 146 | `itRemixMrSaturnGroundWaitProcMap` |
| `stage_setting_` | 160 | `itRemixMrSaturnMakeItem` (exported) + `itRemixMrSaturnSetup*` |
| `ground_collision` | 274 | `itRemixMrSaturnGroundProcMap` |
| `wait_initial` | 289 | `itRemixMrSaturnWaitSetStatus` |
| `wait_main` | 307 | `itRemixMrSaturnWaitProcUpdate` |
| `walking_main` | 332 | `itRemixMrSaturnWalkProcUpdate` |
| `_update_x_scale` | 384 | `itRemixMrSaturnUpdateXScale` |
| `_update_held_x_scale` | 399 | `itRemixMrSaturnUpdateHeldXScale` |
| `_animate_saturn` | 415 | `itRemixMrSaturnAnimateWalk` |
| `held_main` | 436 | `itRemixMrSaturnHeldProcUpdate` |
| `_animate_held_saturn` | 465 | `itRemixMrSaturnAnimateHeld` |
| `throw_initial_` | 487 | `itRemixMrSaturnThrowSetStatus` |
| `reflected_initial_` | 510 | `itRemixMrSaturnReflectedSetStatus` |
| `falling_initial_` | 530 | `itRemixMrSaturnFallSetStatus` |
| `thrown_main_` | 555 | `itRemixMrSaturnThrownProcUpdate` |
| `throw_collide_` | 589 | `itRemixMrSaturnThrowCollide` (+ `itRemixMrSaturnBounceOff`, `itRemixMrSaturnPlayHitSound`) |
| `throw_collide_hitbox_` | 618 | `itRemixMrSaturnThrowCollideHitbox` |
| `detect_collision_` | 646 | not ported: `OS.copy_segment` of raw vanilla bytes (0xEE0F4, 0xEE1CC), and no table or call references it |
| `reflect_` | 663 | `itRemixMrSaturnReflectProc` |
| `prepickup_` | 680 | `itRemixMrSaturnHoldProc` |
| `pickup_` | 699 | NULL in descriptor (see below) |

## Vanilla calls and external symbols

| RAM | Port call | Note |
|---|---|---|
| 0x8016E174 | `itManagerMakeItem` | spawn |
| 0x801735A0 | `itMapCheckLRWallProcNoFloor` | |
| 0x80172508 | `itMainSetSpinVelLR` | |
| 0x8017279C | `itMainClearOwnerStats` | |
| 0x80172E74 | `itMainSetGroundAllowPickup` | |
| 0x80172EC8 | `itMainSetStatus` | |
| 0x80173B24 | `itMapCheckDestroyDropped` | rebound 0.3F (0x3E99999A), ground 0.5F (0x3F000000) |
| 0x801733E4 | `itMainCommonProcHop` | state 3 hop |
| 0x80173434 | `itMainCommonProcReflector` | |
| 0x8017AC84 | `itRShellSpinCheckCollisionEdge` | prototype added locally; the decomp returns void, see uncertainties |
| 0x801713F4 | `itVisualsUpdateSpin` | used for spin on both thrown calls |
| 0x80172558 | `itMainApplyGravityClampTVel` | gravity 1.7546875F (0x3FE0999A), thrown speed 100.0F (0x42C80000) |
| 0x80177218 | `itBombHeiCommonSetHitStatusNone` | in `prepickup_` |
| 0x80111EC0 | `ifCommonItemArrowMakeInterface` | arrow indicator |
| 0x800269C0 | `func_800269C0_275C0` | FGM play, id 0x5FD + random |
| Global.get_random_int_ (0x80018994) | `syUtilsRandIntRange(3)` | assumed to map to the same `[0, N)` range; verify |
| 0x80008CC0 | not called | see REMIX-TODO(GFX) |

## pickup_ (consume) and the hook

`pickup_` (asm lines 699-705) saves a2/a3 to the stack and jumps to 0x80145C4C, which is the
`Crash.crash_eat_sfx._return` label in `remix.sym`, the return point of the Remix pickup dispatch
(`Item.asm` `item_pickup_check_`, hook at 0xC0654). It does nothing else. `ftCommonLightGetProcDamage`
(`decomp/src/ft/ftcommon/ftcommonget.c`, `#ifdef REGION_REMIX` branch) calls `proc_consume` only if it is
non-NULL and then `break`s, which is the same place. So `proc_consume = NULL` reproduces the behaviour.
No C function is written for `pickup_`.

## Patches the item relies on (none required)

- No per-item vanilla patch is needed. The shared Remix dispatch in `Item.asm` (`extend_item_spawn_`,
  `item_pickup_check_`, the pre-pickup and drop/throw tables) already routes ids >= 0x2D to
  `ITRemixItemDesc` through the port hooks.
- `Item.asm` line 2076 (file 0xFB pointer) is the same address as `gITManagerCommonData`. Nothing to add.

## REMIX-TODO markers

- **REMIX-TODO(file)**: item footer at 0x10B0 in file 0xFB. The Remix build must place Mr. Saturn data
  there. Verify the port's reloc table for 0xFB (see Item file section).
- **REMIX-TODO(GFX)**: asm lines 179-216. The image index is set (done: `mobj->texture_id_curr = 6`).
  The render save/restore around `jal 0x80008CC0` (apply render routine 0x2E, the `DObj.translate.vec` triple
  at joint+0x1C..0x24) is skipped. It nets to nothing except the render routine's side effects, which
  are not ported.
- **REMIX-TODO(struct)**: asm lines 219-224. `0x01D0` hitbox refresh timer = 0, `0x01D4` hitbox collision
  flag = FALSE, `0x0398` blast-wall routine = 0, `0x033E` bomb flag = 0. No named field in the decomp
  table; the item is zero-initialised by `itManagerMakeItem`, so these are no-ops on a fresh item, but
  they are not written.
- **REMIX-TODO(struct)**: asm line 250-252. `0x02D3 |= 0x04` (`lbu`/`ori`/`sb`). The struct table places
  0x02D3 inside the `attr` pointer (0x02D0), so the mapping is unclear. Not written.
- **REMIX-TODO(struct)**: asm lines 533-542 (`falling_initial_`). `0x02CE` bit `0x02` (clears "don't
  randomly despawn") and `0x02D2` ground timer = 0x5674 (quick despawn). Both fall in the low pickup_wait /
  attr region of the struct table, so no named field is safe. Not written. This means falling Mr. Saturn
  may not despawn quickly. Needs a field check before enabling.

## Uncertainties (asm line numbers)

1. **Arrow pointer offset** (asm 257-258). Remix stores `ifCommonItemArrowMakeInterface` at `0x0348`.
   The decomp `ITStruct.arrow_gobj` is at `0x0344` and `0x0348` is `arrow_timer` (u8). The port uses
   `ip->arrow_gobj` (named field). If the Remix offset is right, this pointer is written to a different
   slot than vanilla reads.
2. **Shell velocity offset** (asm 134, 135, 262, 360). Remix uses `0x0358`. The struct table has
   `item_vars.shell.vel_x` at `0x0354`. The port maps by meaning to `item_vars.shell.vel_x`, which is what
   `itRShellSpinEdgeInvertVelLR` reads and flips. Confirm the offset against the Remix build.
3. **Walk direction block** (asm 351-359). The `beql` with `addiu at, r0, 0xFFFF` (taken case) and the
   fall-through `addiu at, r0, 1` set `lr` to -1 when it is already -1, and to +1 otherwise. For lr in
   {-1, +1} it is a no-op. The `v0` flag it reads comes from `itRShellSpinCheckCollisionEdge`, which the
   decomp declares void. The port calls the vanilla edge routine (which does the actual flip through
   `itRShellSpinEdgeInvertVelLR`) and omits the no-op block. Verify the flip at runtime on a ledge.
4. **Timers moved into item_vars** (asm 0x02C0 wait/walk timer, 0x01CC throw timer). The Remix offsets are
   `vel_scale` and an unnamed slot in the decomp table. The port keeps them in
   `ITRemixMrSaturnVars { wait_timer, throw_timer }` at the start of `item_vars` (0..7). These overlap
   shell bytes 0..7 (`damage_all_delay`, `dust_effect_int`, `health`, `is_damage`, `is_setup_vars`,
   `interact`), which `itRShellSpinCheckCollisionEdge` path does not read. Behaviour-preserving for this item only.
5. **Image index** (asm 182, 244). Remix writes 5 and then 6 in the same routine. The port writes 6 once.
6. **Base damage** (asm 237-238). The `BASE_DAMAGE(2)` constant is unused. The code writes 6. The port uses 6.
7. **Gravity constant**. `GRAVITY(0x3FE0999A)` decodes to 1.7546875. The float literal round-trips exactly.
8. **Random helper**. `Global.get_random_int_` is assumed to match `syUtilsRandIntRange`. Verify.
9. **Hit sound id**. `0x5FD + random(3)` is passed to `func_800269C0_275C0` (the decomp FGM play name, used
   by `sc1pintro.c`).
10. **Walking timer destroy** (asm 340-342). The `beqzl` delay slot stores 1 as the return, and the stored
    timer keeps its previous value (1). The port returns TRUE without storing 0. The item is destroyed either way.
