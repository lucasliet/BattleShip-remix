# Item WaddleDoo (0x47) port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/WaddleDoo.asm` (815 lines).
Port: `decomp/src/it/itremix/itremixwaddledoo.c` and `itremixwaddledoo.h`.
Descriptor: `dITRemixWaddleDooItemDesc` (kind 0x47, `Item.WaddleDoo.id`).
Status: partial. The attack state machine is ported. The beam star projectile is
not spawned yet (`itRemixWaddleDooCreateBeamStar` returns NULL, REMIX-TODO).

## Routines

Ported: 4 of the 6 routines in scope (`create_beam_star` is a stub, and the knockback fix is a vanilla patch, not ported):

| asm routine | C | Notes |
|---|---|---|
| `waddle_doo_attack_initial` | `itRemixWaddleDooAttackInitial` (exported) | Called from WaddleDee and ScuttleMinion ports |
| `waddle_doo_attack_main` | `itRemixWaddleDooAttackProcUpdate` (static) | Used by state table entries 4 and 5 |
| `star_stage_setting` | `itRemixWaddleDooStarStageSetting` (static) | Creates one star; asm has one call, the loop is commented out |
| `beam_main` | `itRemixWaddleDooBeamProcUpdate` (static) | Unused until the beam WPDesc is ported |
| `create_beam_star` | `itRemixWaddleDooCreateBeamStar` (static) | STUB, returns NULL |
| `beam_knockback_direction_fix` | not ported | Vanilla patch, see below |

Not in the C file: `beam_info_array` (WPDesc data), `item_info_array` (ported as
the static `ITDesc`), `item_state_table` (ported as `sITRemixWaddleDooStatusDescs`).

Not ported:
- `beam_stage_setting` is commented out in the asm (line 449). Not ported.
- `beam_info_array` (line 801). Needs the beam WPDesc.

## Descriptor

```c
const ITRemixItemDesc dITRemixWaddleDooItemDesc =
{
    0x47,                                /* Item.WaddleDoo.id */
    itRemixWaddleDeeStageSetting,        /* SPAWN_ITEM = Item.WaddleDee.stage_setting_ (asm line 6) */
    FALSE,                               /* SHOW_GFX_WHEN_SPAWNED = OS.FALSE (line 7) */
    NULL, NULL, NULL, NULL, NULL         /* PICKUP_INIT, PICKUP_MAIN, DROP, THROW, PLAYER_COLLISION = 0 (lines 8-12) */
};
```

Spawn signature for fighter code: `itRemixMakeItem(parent_gobj, 0x47, pos, vel, flags)`.
The spawn is the WaddleDee worker's `itRemixWaddleDeeStageSetting`, so this item is
spawned through WaddleDee's stage setting, not through `itRemixWaddleDooMakeItem`.
This file does not call `itManagerMakeItem`, so it does not set `desc.p_file`.
The WaddleDee worker must set `p_file` to the owner's `fp->data->p_file_special1`
(`Character.DEDEDE_file_6_ptr`, asm line 17) before calling `itManagerMakeItem`.

## Item file

`Character.DEDEDE_file_6_ptr` = owner fighter's `p_file_special1` (file 6).
Recorded here, not applied, because this file does not spawn.

## Cross-worker: WaddleDee externs

Declared as `extern` in `itremixwaddledoo.c`. Names follow the rule `itRemixWaddleDee<Routine>`,
with the asm label converted to CamelCase and the trailing `_` dropped.

| Asm label | Used as | Extern name | Signature assumed |
|---|---|---|---|
| `stage_setting_` | SPAWN_ITEM | `itRemixWaddleDeeStageSetting` | `GObj* (GObj*, Vec3f*, Vec3f*, u32)` |
| `minion_null_main_` | state 0 update | `itRemixWaddleDeeMinionNullMain` | `sb32 (GObj*)` |
| `waddle_dee_aerial_main_` | state 1 update | `itRemixWaddleDeeWaddleDeeAerialMain` | `sb32 (GObj*)` |
| `waddle_dee_collision_` | state 1 map | `itRemixWaddleDeeWaddleDeeCollision` | `sb32 (GObj*)` |
| `minion_hurtbox_collision_` | state 1/2 hit and shield | `itRemixWaddleDeeMinionHurtboxCollision` | `sb32 (GObj*)` |
| `minion_hitbox_collision_` | all states, damage | `itRemixWaddleDeeMinionHitboxCollision` | `sb32 (GObj*)` |
| `waddle_dee_landing_main_` | state 2 update | `itRemixWaddleDeeWaddleDeeLandingMain` | `sb32 (GObj*)` |
| `waddle_dee_resting_collision_` | state 2 map | `itRemixWaddleDeeWaddleDeeRestingCollision` | `sb32 (GObj*)` |
| `minion_walk_main` | state 3 update | `itRemixWaddleDeeMinionWalkMain` | `sb32 (GObj*)` |
| `waddle_ground_collision` | states 3-5 map | `itRemixWaddleDeeWaddleGroundCollision` | `sb32 (GObj*)` |
| `minion_walk_intitial` | called from attack_main | `itRemixWaddleDeeMinionWalkInitial` | `void (GObj*)` |
| `minion_free_` | called from attack_main | `itRemixWaddleDeeMinionFree` | `void (GObj*)` |

Shared state layout risk: the WaddleDee routines run on the same ITStruct.item_vars.
This file overlays `ITRemixWaddleDooVars` on it (fields below). The WaddleDee port
must not put different data in the same bytes, or the two items will corrupt each other.

## Shared struct: item_vars overlay

`ITRemixWaddleDooVars` (file-local), `_Static_assert`ed to fit in `ITStruct.item_vars`:

| Field | N64 offset (item special struct) | Meaning |
|---|---|---|
| `bitfield_0x02CE` (u8) | 0x02CE | Two bits cleared at attack start; meaning unknown |
| `attack_timer` (s16) | 0x033E | Attack frame counter |
| `last_star_gobj` (GObj*) | 0x0350 | Last beam star |

Mapped to named ITStruct fields, not overlay fields:
- 0x0008 parent: `owner_gobj`
- 0x000C kind: `kind`
- 0x0010 type: `type`
- 0x0024 direction: `lr`
- 0x02C0 remaining duration: `lifetime`
- 0x010C hitbox disable: `attack_coll.attack_state = 0`

## Beam projectile overlay

`ITRemixWaddleDooBeamVars`, `_Static_assert`ed to fit in `WPStruct.weapon_vars`:

| Field | N64 offset (projectile struct) | Meaning |
|---|---|---|
| `startup_timer` (s16) | 0x029A | Spark gfx timer |
| `draw_toggle` (s16) | 0x02A8 | Set to 1 at creation; toggling not ported |
| `angle` (f32) | 0x02A4 | Angle around parent, radians |
| `parent_gobj` (GObj*) | 0x02A0 | Waddle Doo that made the star |

Mapped to named WPStruct fields: 0x0008 `owner_gobj`, 0x0012 `team`, 0x0018 `lr`,
0x0020 `physics.vel_air.x`, 0x0104 `attack_coll.damage`, 0x0100 `attack_coll.attack_state`,
0x0154/0x0158 `coll_data.pos_prev`, 0x029C `lifetime`.

## Vanilla patches

### beam_knockback_direction_fix (asm lines 408-442)

- RAM `0x800E44EC`, ROM `0x5FCEC`, one instruction replaced by `j beam_knockback_direction_fix; nop`.
- Vanilla function: `ftMainProcessHitCollisionStatsMain` (`decomp/src/ft/ftmain.c`, starts at `// 0x800E3EBC`, line 2849).
- Behaviour: when the attacker projectile's id (`s4->0x000C`) is `Projectile.id.DEDEDE_WADDLEDOO_BEAM`:
  - use 0.5F as `f6` (the normal path uses 5.0F, `lui at, 0x40A0`);
  - copy `t7` and `s1->0x000C` into `s5->0x07F4` and `s5->0x07F8` (the original logic at `0x800E44F4`);
  - set `f0 = (f32) projectile->direction`;
  - jump to `0x800E4508`.
- REMIX-TODO: the exact C insertion (under `#ifdef REGION_REMIX`) is not written. It needs
  the `ftMainProcessHitCollisionStatsMain` variables (`s1`, `s4`, `s5`, `t7`), which were not mapped in this pass.
- `Projectile.id.DEDEDE_WADDLEDOO_BEAM` is defined in `.toolchain/pkgs/smashremix-src/src/Projectile.asm:29`.
  The port has no C constant for it yet.

## Shared subsystems: REMIX-TODO

1. Beam weapon WPDesc (`beam_info_array`, asm lines 801-815). Needs a WPDesc with:
   - kind `Projectile.id.DEDEDE_WADDLEDOO_BEAM`;
   - file 0xFB pointer `0x8018D040`, offset `0x000004D4` (a Remix ROM file, not loaded in this port);
   - render index `0x1C000000`;
   - proc update `beam_main`;
   - attributes from the star rod (`0x801784A8` is the star rod's `wpManagerMakeWeapon` call).
   Once it exists, `itRemixWaddleDooCreateBeamStar` calls `wpManagerMakeWeapon(item_gobj, &desc, pos, flags)` with the flags from `sp+0x28` (asm line 638, value 1).
2. Image struct frame writes (`sh at, 0x0080(a0)`, asm lines 98, 177, 121). The port has no matching API for the item's image frame.
3. Visibility and hitbox toggle every 2 frames in `beam_main` (asm lines 688-707). The draw flag at 0x007C and the hitbox interval at 0x0100 have no decomp field mapping.
4. Clearing the already-hit player (`sw r0, 0x0214`, line 788). Not mapped.
5. Hitbox collision flag at 0x01D4 (lines 226, 214). Not mapped.

## Constants copied from WaddleDee.asm minion_attributes

Copied as file-local `#define`s, not shared. The WaddleDee port should export or use one set.

| Name | Value | Source |
|---|---|---|
| BEAM_CHARGE_TIME | 20 | WaddleDee.asm line 28 (0x28) |
| BEAM_INITIAL_ANGLE | 1.125F (0x3F900000) | line 29 (0x2C) |
| BEAM_DISTANCE_FROM_PARENT | 256.0F | line 30 (0x30) |
| BEAM_ROTATION_AMOUNT | 0.26171875F (0x3E860000) | line 34 (0x34) |
| BEAM_DURATION | 10 | line 38 (0x38) |
| BEAM_ROTATION_OFFSET (child angle offset) | 0.40625F (0x3ED00000) | line 39 (0x3C) |
| BEAM_STAR_COUNT | 4 | WaddleDoo.asm line 235 |
| BEAM_STAR_DAMAGE | 2 | line 236 |
| BEAM_START_UP | 10 | line 239 |
| SHOOT_TIME (40 frames after charge) | 40 | line 199 |

## Uncertainties (asm line numbers)

1. Status state table block labels disagree (lines 22-81). The port reads the table by block index, so status 4 is the attack state (`a2 = 4` at line 128). The offsets written as 0xD4 (lines 54-71) are inconsistent with a contiguous 0x20-byte layout. Verify once against the game ROM if possible.
2. `star_stage_setting` (lines 246-405) has one `create_beam_star` call. The loop comment says "3 stars" but the loop was removed. The port creates one star per call.
3. `create_beam_star` (lines 634-643) jumps into the middle of `itStarRodWeaponStarMakeWeapon` at 0x801784A8 (`itstarrod.c`, `// 0x80178474`), not a function entry. The port does not reproduce that jump. It is the REMIX-TODO above.
4. Sine (`jal 0x800303F0`, line 380 and line 773) is assumed to be `sinf`. No decomp name was found for that address. The port uses `sinf`, which may differ in precision from the N64 table.
5. The cosine call (`jal 0x80035CD0`, lines 358 and 754) has its result overwritten before use (line 364, line 759). The x coordinate uses `direction * distance`, not cos. The port omits the call.
6. `lw at, 0x0108(s0)` (line 146) is a dead load. Not ported.
7. Hurtbox disable at 0x0248 (line 167) maps to `damage_coll.hitstatus = 0`. Uncertain.
8. Rotation reset at 0x0038 on the position struct (line 95) maps to `DObj->rotate.vec.f.z`. Uncertain.
9. `beam_info_array` file pointer is `0x8018D040` (line 803). That address is in the Remix ROM, not in vanilla.
10. `dITRemixWaddleDooItemDesc.proc_make` uses the WaddleDee spawn. Confirm with the WaddleDee worker that `itRemixWaddleDeeStageSetting` takes the item kind from `item_info_array` (static `sITRemixWaddleDooItemDesc`, not exported).
11. `sITRemixWaddleDooItemDesc` (ITDesc) is static and unused in this file. `o_attributes` is set to `0x08` (footer offset). `transform_types` copies `ittomato.c` (`nGCMatrixKindTraRotRpyR`, `nGCMatrixKindNull`), not verified from the asm, which has no transform fields. The asm `item_info_array` 0x0C word (`0x1B000000`) is unexplained.

## Step 2 wave 2026-10-14 (partial — kimi quota window)

- LANDED (W3): beam weapon (WPDesc sITRemixWaddleDooBeamWeaponDesc + BeamProcUpdate) and the star spawn via wpManagerMakeWeapon. Same itprocess.c caveat as item_Cloud (flash-flags preservation unwired).
