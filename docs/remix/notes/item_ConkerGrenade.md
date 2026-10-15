# Item: Conker's grenade (ConkerGrenade)

Port: `decomp/src/it/itremix/itremixconkergrenade.c` and `itremixconkergrenade.h`.
Source: `.toolchain/pkgs/smashremix-src/src/Conker/ConkerSpecial.asm` lines 1077-1899 (`main`, `grenade_*`, `grenade_attributes`, `grenade_item_info_array`), plus `grenade_item_states`.

## Id and descriptor

- Item id: `GRENADE_ID` = `0x15` (ConkerSpecial.asm line 1898). No `add_item` entry exists for it in `Item.asm`, so it is a vanilla id.
- No `ITRemixItemDesc` is exported, and `itremixitems.c` is untouched. The spawn uses a file-local `ITDesc` (`sITRemixConkerGrenadeItemDesc`) copied per spawn.
- Item file: `Character.CONKER_file_7_ptr` is Conker's file 7, mapped to `fp->data->p_file_special2`.
- Item footer offset `0x40` goes into `o_attributes`.

## Spawn interface

```c
GObj *itRemixConkerGrenadeMakeItem(GObj *parent_gobj, Vec3f *pos, s32 power);
```

- `parent_gobj`: the Conker fighter GObj (owner, team, port, facing come from it).
- `pos`: the throw origin. The asm takes this from the right-hand part (`0x800EDF24` on part 0xC).
- `power`: Conker's grenade power (asm `0x0B20`, the DSP's `grenade_power`).
- The velocity is zero (asm passes the zeroed `sp+0x40` vector through as `vel`). The flags are 0.

## Routines ported (asm to C)

| asm | C |
|---|---|
| `grenade_stage_setting_` (+ spawn args from `main`) | `itRemixConkerGrenadeMakeItem`, `...SetupRender`, `...SetupLaunch`, `...SetupState` |
| `grenade_main_` | `itRemixConkerGrenadeProcUpdate` (+ DragGround, DecaySpeedAir, UpdateRotation*, UpdateHitRecords, UpdateDamage, SpawnSmoke) |
| `grenade_collision_` | `itRemixConkerGrenadeProcMap` |
| `grenade_resting_collision_` | `itRemixConkerGrenadeProcMapResting` |
| `grenade_exploding_main_` | `itRemixConkerGrenadeProcExploding` |
| `grenade_explosion_hitboxes_` | `itRemixConkerGrenadeExplosionAttackEvent` |
| `grenade_begin_main_` | `itRemixConkerGrenadeSetAir` |
| `grenade_begin_resting_` | `itRemixConkerGrenadeSetResting` |
| `grenade_explosion_` | `itRemixConkerGrenadeExplode` |
| `grenade_begin_explosion_` | `itRemixConkerGrenadeSetExplosion` |
| `grenade_detect_collision_` (copy_segment fragments) | `itRemixConkerGrenadeDetectCollision` |
| `grenade_hurtbox_collision_` | `itRemixConkerGrenadeProcHurtbox` |
| `grenade_hitbox_collision_` | `itRemixConkerGrenadeProcDamage` |
| `grenade_blast_zone_` | `itRemixConkerGrenadeProcDead` |
| `grenade_item_states` | `sITRemixConkerGrenadeStatusDescs[3]` |
| `grenade_item_info_array` | `sITRemixConkerGrenadeItemDesc` |
| `grenade_attributes` | `ITREMIX_CONKER_GRENADE_*` defines |

Not in this item file: `scope main` (ConkerSpecial.asm 1080), the fighter-side `ground_*`/`air_*` wrappers and `ground_collision`/`air_collision` (ConkerSpecial.asm 1800-1900). They belong to ConkerDSP, and `ftremixconkerdsp.c` already has the ones it needs.

## copy_segment fragments

- `grenade_detect_collision_` copies two fragments from vanilla. Their RAM is `itMapProcAllCheckCollisionFlag` (decomp `it/itmap.c:108`, vanilla RAM `0x801736B4`), not `0x801737B8`. Under the item-segment delta, `0x800855C0`, that is ROM `0xEE0F4`.
- Fragment 1 is ROM `0xEE0F4`, size `0x88`. It is the wall/ceiling/floor collision of that proc, ending in `mpProcessRunFloorCollisionAdjNewNULL`.
- Fragment 2 is ROM `0xEE1CC`, size `0x2C`. It is the tail that returns `(mask_curr & flags) != 0`.
- The asm's `beql v0,r0` then `mpProcessSetLandingFloor`, and the omitted `mask_stat & FLOOR` block (edge adjust, FGM, spin reset, `is_coll_end`), come out as the decomp function minus that block. The port does exactly that.
- `grenade_explosion_hitboxes_` tail-jumps into vanilla `0x801863B8`, which is inside `itLinkBombExplodeUpdateAttackEvent` (`itfighter/itlinkbomb.c:528`). Its body (ROM `0x100DEC`, same delta) reads the attack events at `*p_file + 0x88`, indexed by `event_id`, and compares `multi` with `timer`. The port uses the grenade's own file pointer.

## Vanilla calls and their decomp names

`0x8016E174` itManagerMakeItem, `0x800EDF24` (not used here), `0x800DF058` mpCommonRunItemCollisionDefault, `0x800DA034` mpProcessUpdateMain, `0x801737EC` itMapCheckCollideAllRebound, `0x80173F78` itMapSetAir, `0x80173F54` itMapSetGround, `0x80172EC8` itMainSetStatus, `0x8017279C` itMainClearOwnerStats, `0x8017275C` itMainRefreshAttackColl, `0x80172558` itMainApplyGravityClampTVel, `0x80185CD4` itLinkBombCommonSetHitStatusNormal, `0x801735A0` itMapCheckLRWallProcNoFloor, `0x801733E4` itMainCommonProcHop, `0x80173434` itMainCommonProcReflector, `0x801005C8` efManagerSparkleWhiteMultiExplodeMakeEffect, `0x801008F4` efManagerQuakeMakeEffect(1), `0x800FE9B4` efManagerFlameLRMakeEffect, `0x800269C0` func_800269C0_275C0 (FGM), `0x801409BC` ftCommonDamageGetKnockbackAngle, `0x80008CC0` gcAddXObjForDObjFixed (kind 0x2E), `0x80035CD0` / `0x800303F0` `__cosf` / `__sinf`.

## Vanilla patches

None applied. The port calls vanilla functions as-is, and needs no edit to vanilla files. It does need the item manager to use the descriptor passed to `itManagerMakeItem` (it does, since the grenade passes its own desc).

Two things the integration must handle:

1. ConkerDSP must call `itRemixConkerGrenadeMakeItem` where `REMIX-TODO(Item)` sits (`ftremixconkerdsp.c` line 100), and set the active slot (see REMIX-TODO 1).
2. Spawn does not call `itMainSetStatus`. The initial procs are the aerial status (ITDesc procs), matching the asm's inline state 0.

## REMIX-TODOs

1. **Active grenade slot (ConkerDSP).** The asm writes the grenade pointer to the owner's `0x0ADC` on spawn (`grenade_stage_setting_` line 1213), clears it when the explosion ends (`grenade_exploding_main_` line 1564), and clears it on blast zone (`grenade_blast_zone_` line 1248). The DSP keeps that slot in `passive_vars`. The item cannot write it from C without the DSP's overlay struct. **Until this is wired, a grenade that explodes leaves the slot set, so Conker cannot toss another.** The item stores the owner in `vars->fp` for this. The DSP side must store the value returned by `itRemixConkerGrenadeMakeItem` on spawn and clear it when the grenade ends.
2. **DObj byte 0x54 (first joint).** asm `sb 2, 0x54(joint0)` in `grenade_explosion_`. The field is not named in `sys/objtypes.h`, so it is skipped.
3. **Kinetic/shared helpers.** None beyond the above.

## Named-field mapping and uncertain offsets

The Remix asm offsets for ITStruct differ from `build/remixasm/refs/struct_offsets.txt` by +4 from around `0x0248`/`0x0294` on (N64 layout). Each field is mapped by meaning, and the decomp field names are used:

- `0x02C0` duration: `lifetime` (the struct table has it at `0x02BC`).
- `0x0298` damage: `damage_queue`. `0x029C` angle: `damage_angle`. `0x02A4` lr: `damage_lr`. `0x02A8` owner: `damage_gobj`.
- `0x033E` explosion timer: `multi` (the struct table has it at `0x033A`). `0x0340` event nibble: `event_id`.
- `0x02CE` bits: `0x0080` set on halfword at spawn is `is_damage_all` (asm `0x02CF` bit 0x80 = struct `0x02CB` bit 0x80, owner hitboxes hit the grenade). `0x02CE` `andi 0xCF` on rest is `times_landed = 0` (struct `0x02CA` bits 0x30).
- `0x0158` `ori 0x10` at spawn is `attack_coll.can_rehit_shield`. Offsets from `0x010C` to `0x0156` match the struct table exactly.
- `0x0398` blast zone routine: `proc_dead`.
- `0x0100` at spawn (commented as a Bomb character ID reference): written as `coll_data.cliff_id = 0`. This is uncertain.
- `0x0108` ground/air: `ga`, where `nMPKineticsGround` is 0, so the asm test `beq at, r0` is the ground branch.

The Remix-only fields (`0x01C4`..`0x01D4`, `0x0354`) live in `ITRemixConkerGrenadeVars`, inside `item_vars`. `0x01C4` is `fp`, `0x01C8` is `max_speed`, `0x01CC` is `rotation_direction`, `0x01D0` is `hitbox_refresh_timer`, `0x01D4` is `is_hit_collide`, and `0x0354` is `unk_0x354`.

## Uncertainties

1. Descriptor transform types. `0x0C` of the asm info array is `0x1B000000` (commented "either 0x1B000000 or 0x1C000000 - possible argument"). It is not a valid `nGCMatrixKind`, so the port uses the tomato defaults (`nGCMatrixKindTraRotRpyR`, `nGCMatrixKindNull`, 0). The asm states at `0x14`, and the decomp `ITDesc` layout, do not match. Check how Remix's item manager reads `0x0C` before trusting the defaults.
2. `ftCommonDamageGetKnockbackAngle(angle, ga, knockback)`. The asm passes whatever `a1` holds, which is undefined. The port passes `ip->ga`.
3. `efManagerFlameLRMakeEffect(pos, lr)`. The asm leaves `a1` undefined. The port passes `ip->lr`.
4. Rotation direction. The asm tests the x-speed bits (`beqz t0`), so `-0.0F` is treated as nonzero there. The port uses `x == 0.0F`. The only difference is `-0.0F`.
5. Comment vs value mismatches in the asm (the values were taken from the bits, not the comments):
   - Ground drag `0x3F700000` is `0.9375F` (comment says 0.875).
   - Base rotation `0x3C900000` is `0.017578125F` (comment says 0.086).
   - Y launch velocity uses `sin` (the comment says cos).
   - The hit-refresh speed threshold is `0x420C0000` = 35.0F (comment says 20).
   - Damage scale `0x3D900000` = 0.0703125F.
   - The asm line 993 immediate in the DSP is `0x14` (not the port's concern).
6. The asm's `andi 0xFF7F` in `grenade_begin_main_` is commented out, so it is not ported. Its disabled `ori 0x80` in `begin_resting` is not ported either.
7. Kind `0x15` is a vanilla id shared with whatever vanilla item uses it. The grenade is only spawned through its own descriptor, never by kind. Any vanilla code that looks up items by kind 0x15 would apply that vanilla item's behavior. The enum value of that vanilla item was not checked.
8. `func_800269C0_275C0` is declared locally in the `.c` file, the same way `itmap.c` and `itlinkbomb.c` do it. `efManagerQuakeMakeEffect` is declared by its own header.

## Verification

- `scripts/remix/syntax-check.sh decomp/src/it/itremix/itremixconkergrenade.c` printed `syntax-check OK` with no warnings. The flags include `-DPORT=1`.
- Not built, not run, not committed, per the brief.
