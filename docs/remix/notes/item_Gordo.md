# Item Gordo (Item.Gordo.id 0x48): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Gordo.asm` (400 lines, `Item.asm` line 1463 scope, `add_item(Gordo)` at line 1545).
Port: `decomp/src/it/itremix/itremixgordo.c` and `itremixgordo.h`.
Descriptor: `dITRemixGordoItemDesc` (kind 0x48, `proc_make = itRemixGordoMakeItem`, `is_show_spawn_gfx = FALSE`, hold/consume/dropped/thrown/touch all NULL, as in the asm: SHOW_GFX_WHEN_SPAWNED = FALSE, PICKUP_ITEM_INIT/MAIN, DROP, THROW, PLAYER_COLLISION all 0).
Spawn function for fighter call sites: `GObj* itRemixGordoMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.
Item file: `Character.DEDEDE_file_6_ptr`, so `desc.p_file = fp->data->p_file_special1` (N = 6 is special1). The item footer (attributes) is at file offset 0x120, stored in `o_attributes`.

## Routines

| Remix label | Port | Kind |
|---|---|---|
| `Gordo.stage_setting_` | `itRemixGordoMakeItem` (exported) | SPAWN_ITEM |
| `Gordo.gordo_null_main_` | `itRemixGordoNullProcUpdate` (static) | state 0 main |
| `Gordo.gordo_throw_initial_` | `itRemixGordoThrowInitial` (static) | helper |
| `Gordo.begin_throw_` | `itRemixGordoBeginThrow` (static) | helper |
| `Gordo.begin_resting_` | `itRemixGordoBeginResting` (static) | helper |
| `Gordo.stick_to_surface` | `itRemixGordoStickToSurface` (static) | helper |
| (pin branch of null_main) | `itRemixGordoPinToHand` (static) | helper |
| `Gordo.gordo_aerial_main_` | `itRemixGordoAerialProcUpdate` (+ `itRemixGordoAerialStep`) | state 1 main |
| `Gordo.gordo_collision_` | `itRemixGordoAerialProcCollision` | state 1 map |
| `Gordo.gordo_hurtbox_collision_` | `itRemixGordoAerialHurtboxCollision` | state 1 hit and shield |
| `Gordo.gordo_hitbox_collision_` | `itRemixGordoAerialProcDamage` | state 1 damage |
| `Gordo.gordo_resting_main_` | `itRemixGordoRestingProcUpdate` (+ `itRemixGordoRestingStep`) | state 2 main |
| `Gordo.resting_collision_` | `itRemixGordoRestingProcMap` | state 2 map (returns FALSE; asm call disabled, it crashes) |
| `Gordo.resting_hurtbox_collision_` | `itRemixGordoRestingHurtboxCollision` | state 2 hit |
| `Gordo.detect_collision_` | NOT PORTED | dead code, see below |

Status table `sITRemixGordoStatusDescs` (static ITStatusDesc[3]): 0 = null, 1 = aerial, 2 = resting. Mapping from `item_state_table`: proc_update = main, proc_map = collision, proc_hit = hurtbox collision, proc_shield = hurtbox collision (0x20), proc_hop = `itMainCommonProcHop` (0x801733E4, the asm's "shield edge" slot), proc_reflector = `itMainCommonProcReflector` (0x80173434), proc_damage = hitbox collision (the asm's "hurtbox collision w/ hitbox" slot).

21 of 22 routines are ported. `detect_collision_` is not ported: the only reference is `li a1, detect_collision_` in `gordo_collision_`, and the `jal 0x801737B8` delay slot overwrites a1 with 0x0C21 before the call. The routine is never called. Its OS.copy_segment fragments (ROM 0xEE0F4 0x88 and 0xEE1CC 0x2C) are a modified copy of `itMapTestAllCollisionFlag` (0x801737B8).

## Remix item fields stored in ITRemixGordoVars (item_vars)

The vanilla ITStruct does not name these, so they live in a file-local struct cast over `ip->item_vars` (with `_Static_assert` that it fits):

| Asm offset | Field | Meaning |
|---|---|---|
| 0x01C4 | `player_gobj` | fighter that owns Gordo (set at spawn) |
| 0x01C8 | `max_speed` | speed cap; overwritten with the knockback on hit |
| 0x01CC | `rotation_direction` | sign of x speed |
| 0x01D0 | `hitbox_refresh_timer` | frames until hit objects are cleared |
| 0x01D4 | `is_hit_collide` | set when Gordo was hit by an attack |
| 0x033E | `thrown_flag` | 0 pinned to hand, -1 destroy, otherwise thrown |
| 0x0354 | `unk_0x354` | written as 4 at spawn and throw; meaning unknown |
| 0x0398 | `blast_zone_proc` | blast wall destruction routine |

## Vanilla patches and shared subsystems (NOT applied)

1. **Blast wall routine slot (REQUIRED).** Asm stores `Item.WaddleDee.minion_blast_zone_` at item struct 0x0398 (line 150-151). The vanilla blast wall code that reads that slot must, for Gordo, read `ITRemixGordoVars.blast_zone_proc` instead. The reader is not identified here. Wiring is outside this file. Remix item blast-zone dispatch in vanilla needs something like:
   ```c
   #ifdef REGION_REMIX
   ((ITRemixGordoVars*)&ip->item_vars)->blast_zone_proc(item_gobj);
   #endif
   ```
   `ITRemixGordoVars` must be visible to that file, so it is probably better as a header in `itremixgordo.h` if the vanilla caller needs it.
2. **Fighter rumble (REMIX-TODO(rumble)).** `stick_to_surface` (lines 708-726) looks up a fighter GObj by `ip->player` from the table at `0x800A50E8` (`lw t4, 0x50E8(t4)`, stride 116 per port, gobj at +0x78), then calls `ftParamMakeRumble(fp, 6, 0)` via `0x800E806C`. Skipped in the port: the decomp global at `0x800A50E8` is `gSCManagerBattleState` (scmanager.c:57), which is not a fighter table, and the port-to-fighter lookup is unresolved. The `ip->player == 4` (no combo port) check and the dead `0xFFFF` compare (a byte is never 0xFFFF) are also not ported. Needed C once the table is known: `ftParamMakeRumble(fp, 6, 0)` for each port that is not 4.
3. **Render routine (REMIX-TODO(gfx)).** `stage_setting_` calls `0x80008CC0` with a1 = 0x2E, a2 = 0 (line 109-111). No decomp name found (not in vanilla_funcs.txt, no `func_80008CC0` in decomp/src). Skipped. Needed: the decomp equivalent of `0x80008CC0`, then the same call in `itRemixGordoMakeItem`.
4. **Vanilla patches the item depends on.** None are applied by this file, and no vanilla file is edited. The dispatch in `itmanager.c`/`itmain.c` to ids >= 0x2D is assumed to exist as described in `itremix.h`.

## Shared WaddleDee routines (extern, to be exported by the WaddleDee worker)

Declared in `itremixgordo.c` as:
- `extern void itRemixWaddleDeeMinionFree(GObj *item_gobj);`: the same declaration as in `itremixwaddledoo.c`, reused. Called from aerial and resting main when the lifetime runs out (asm `Item.WaddleDee.minion_free_`, lines 397 and 516). Return value unused.
- `extern void itRemixWaddleDeeRemoveMinionCheck(GObj *item_gobj);`: new. Called from the null main throw branch (asm `Item.WaddleDee.remove_minion_check_`, line 193). Return value unused. Asm passes `a0 = s0` (see uncertainty 6), the port passes the item.
- `extern void itRemixWaddleDeeMinionBlastZone(GObj *item_gobj);`: new. Stored as a function pointer (asm `Item.WaddleDee.minion_blast_zone_`, line 150). The signature is an assumption; the asm only takes the address.

## Uncertainties (asm line numbers)

1. **Struct offset shift (+4 from the decomp field table).** `build/remixasm/refs/struct_offsets.txt` lists ITStruct damage fields at 0x294 to 0x2A8, but Gordo's code reads 0x298 (damage), 0x29C (angle), 0x2A4 (lr), 0x2A8 (gobj). Those fit the table at offset - 4 (damage_queue, damage_angle, damage_lr, damage_gobj), and the duration at 0x2C0 fits lifetime at 0x2BC. The pickup bits at 0x2CE/0x2CF fit 0x2CA/0x2CB, and is_attach_surface matches exactly. The port uses the decomp names on this mapping, so `ip->damage_queue` is the damage, `ip->damage_angle` the angle. Verify in-game.
2. **Attack damage at 0x298 vs damage_queue.** Same issue as 1. Line 302 (`lwc1 f0,0x0298`) is read as an integer, `cvt.s.w`.
3. **Handicap write at 0x0012 (line 129-131).** The asm stores the fighter byte at 0x12, which lands inside ITStruct.type in the table. The port maps it to `ip->handicap` by meaning (FTStruct.handicap at 0x12). Uncertain.
4. **Bit 0x10 at 0x0158 (lines 115-117, 262-263, 153-157 comment).** The port sets `ip->attack_coll.can_rehit_shield = TRUE`, the table name for bit 0x10 of byte 0x158. Meaning unknown.
5. **Bit 0x40 at 0x02CF (line 701-709 region in stick_to_surface).** Port sets `ip->is_attach_surface = TRUE`, the table bit under the +4 mapping. Meaning fits "sticking to a surface".
6. **`s0` in null_main (lines 194, 212, 233).** The asm uses `s0` as the item object in `_throw_minion` and `_pin_to_hand` without setting it in `gordo_null_main_`. The port uses the item GObj. This is an asm bug or relies on a caller's s0. Note that null_main also clobbers nothing else.
7. **`s0` in gordo_throw_initial_/begin_throw_.** Not an issue: begin_throw sets nothing from s0.
8. **Knockback angle `ga` argument (line 905).** `ftCommonDamageGetKnockbackAngle(angle, ga, knockback)`: the asm does not set a1 before the call, so the port passes `ip->ga`. The value is a guess.
9. **`(s32)speed` truncation (line 828).** The asm uses `cvt.w.s` (round to nearest in the default mode). The port truncates. A one-unit difference in the lifetime loss is possible.
10. **Transform types and attack state in the descriptor.** Gordo's info array has `dw 0x1B000000` (or 0x1C000000) at offset 0x0C and 0 at 0x10. Neither is a matrix kind, so the port copies the Tomato `DObjTransformTypes` values as a guess and uses `nGMAttackStateOff`. Verify.
11. **Hand joint index 10.** `0x0910 - 0x08E8 = 0x28`, so `FTStruct.joints[10]` (`ITREMIX_GORDO_FIGHTER_JOINT_LEFT_HAND`). This assumes `joints` is the array at 0x08E8 (struct_offsets.txt, FTStruct section) and that index 10 is the left hand. No named enum exists in the decomp.
12. **DObj translate offset for the hand position (line 201, `addiu a1, v0, 0x1C`).** The port uses `&joint->translate.vec.f`, assuming the asm's pin coordinates are the item joint's translate vector.
13. **Dedede file.** `Character.DEDEDE_file_6_ptr` is used through `fp->data->p_file_special1`, as the brief specifies. Not verified that the file is loaded when Gordo is created outside Dedede's DSP.
14. **Dead flag in rumble (line 697-ish).** `addiu at, r0, 0xFFFF` compared with a byte from `lbu` never matches. Not ported (see REMIX-TODO(rumble)).

## Verification

- `scripts/remix/syntax-check.sh decomp/src/it/itremix/itremixgordo.c`: `syntax-check OK`, no warnings from the file.
- No build, no cmake, no game run, no commit (per brief).
