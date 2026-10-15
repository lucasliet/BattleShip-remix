# Item Basketball (0x40): port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/Basketball.asm` (1495 lines), the ball of Smashketball.
Mode source (context only): `.toolchain/pkgs/smashremix-src/src/Smashketball.asm`.
Port: `decomp/src/it/itremix/itremixbasketball.c`, `decomp/src/it/itremix/itremixbasketball.h`.
Descriptor: `const ITRemixItemDesc dITRemixBasketballItemDesc`, kind 0x40. Not yet listed in `itremixitems.c` (lead regenerates).

## Interface

- SPAWN_ITEM: `GObj* itRemixBasketballMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.
  The asm's `stage_setting_` takes (a1 info, a2 pos, a3 flags), which maps onto this signature like the other Remix items.
- SHOW_GFX_WHEN_SPAWNED: TRUE. PICKUP_ITEM_INIT, PICKUP_ITEM_MAIN, DROP_ITEM, THROW_ITEM and PLAYER_COLLISION are all NULL, as in the asm.
- `ITDesc`: `kind` 0x40, `p_file = &gITManagerCommonData` (file 0xFB, `0x8018D040`), `o_attributes = 0x1170` (FILE_OFFSET).
  `transform_types` is `{0x1B, 0x00, 0x00}`, the raw asm word `0x1B000000`; its meaning is not confirmed.
  `attack_state` is `nGMAttackStateOff`. The asm `item_info_array` has no proc entries, so all `proc_*` are NULL.
- `ITStatusDesc` table (3 states, `item_state_table`): 0 main, 1 barrel, 2 respawn. The asm labels the 2nd block "STATE 1" and the 3rd "STATE 1", but the 3rd is state 2 (`set respawn` uses 2).

## Routines: ported versus asm

Ported:
- `stage_setting_` -> `itRemixBasketballMakeItem`, `itRemixBasketballSetup`, `itRemixBasketballSetupVars`, `itRemixBasketballSetupImage`, `itRemixBasketballSetupAttack`.
- `main_` -> `itRemixBasketballMainProcUpdate` with the grounded, aerial, animation, timer, hitbox refresh, fire, invincibility, fall and score steps.
- `set_grounded_`, `set_aerial_` -> `itRemixBasketballSetGrounded`, `itRemixBasketballSetAerial`.
- `fire_state_manager_` -> `itRemixBasketballFireStateManager`, with `FireBegin`, `FireEndState`, `FireGfxStep`, `SmokeGfxStep`, `AdjustHitbox`.
- `collide_` -> `itRemixBasketballCollide` (hit, shield and hop procs).
- `collision_` -> `itRemixBasketballMainProcMap`, with `itRemixBasketballDetectCollision` (see Deviations).
- `collide_hitbox_` -> `itRemixBasketballCollideHitbox`, with `SetHitOwner`, `AddHitPercent`, `GetHitKnockback`, `Launch`, `SetScoreable`.
- `barrel_main_` -> `itRemixBasketballBarrelProcUpdate`, with `BarrelSetup` and `BarrelShoot` (display toggles are skipped, see REMIX-TODO).
- `check_basketball_fall_` -> `itRemixBasketballCheckFall`.
- `check_basketball_scored_` -> `itRemixBasketballCheckScored`.
- `respawn_main_` -> `itRemixBasketballRespawnProcUpdate`.

Not ported:
- `check_basketball_barrel_collision_`: REMIX-TODO(barrel). The ball never enters a barrel in this port.
- `set_golden_` (stored at 0x398): REMIX-TODO(blast wall and spawn_golden).
- Vanilla hooks: see "Vanilla patches" below.
- `allow_owner_damage_`: commented out in the asm, not ported.

## Vanilla patches (not applied; record only)

Each hook is an `OS.patch_start` in the asm. The vanilla function is the decomp function that contains the address (from `build/remixasm/refs/vanilla_funcs.txt`).

1. `prevent_fighter_enter_barrel_`: patch at RAM 0x8010A0E0 (ROM 0x858E0), inside `grJungleTaruCannCheckGetDamageKind` (`decomp/src/gr/grcommon/grjungle.c:162`). Under `#ifdef REGION_REMIX`, before the fighter enters the barrel, return FALSE while `VsRemixMenu.vs_mode_flag == SMASHKETBALL` and a basketball is in its barrel status. Needs an accessor `sb32 itRemixBasketballIsInBarrel(void)` exported from the item file (not added yet).
2. `bounce_patch_`: four hooks inside `itMapCheckCollideAllRebound` (`decomp/src/it/itmap.c:159`) at RAM 0x80173874 (left wall), 0x801738EC (right wall), 0x8017395C (ceiling), 0x801739BC (floor). Each replaces the call with a jump to one of `_lwall`, `_rwall`, `_ceiling`, `_floor`, which set the bounce joint scale and offset, and then run the original bounce `lbCommonReflect2D` (0x800C7B08). Values:
   - left wall: x_scale 0.625, y_scale 1.75, x_offset +40, y_offset 0.
   - right wall: x_scale 0.625, y_scale 1.75, x_offset -40, y_offset 0.
   - ceiling: x_scale 1.75, y_scale 0.625, x_offset 0, y_offset +40.
   - floor: x_scale 1.75, y_scale 0.625, x_offset 0, y_offset -40.
   The helper also applies only for item id 0x40 with speed >= 40. It sets the joint scale, adds the offset to the joint translate, sets `is_bounce_scaled`, and sets hitlag to `(s32)(speed * 0.0300293F) + 1` in `ITStruct.hitlag_tics`. It plays FGM 0x42F (fire) or 0x42E (normal). The port keeps `is_bounce_scaled` and the offsets in `ITRemixBasketballVars` and undoes them in `itRemixBasketballRestoreBounce`. The helper itself needs an export.
3. `prevent_bounce_fgm_`: three hooks in `itMapCheckCollideAllRebound` at RAM 0x801738A4 (left), 0x80173914 (right), 0x801739DC (floor). For a basketball with `is_bounce_scaled` set, skip the default bounce FGM call `func_800269C0_275C0`.
4. `multiply_player_hitlag_`: RAM 0x800E26C8 in `ftMainSetHitInteractStats` (`decomp/src/ft/ftmain.c:2078`). When the victim item is a basketball with `is_fire`, set the attacker `hitlag_multiplier` (FTStruct 0x7A4) to 2.0F.
5. `multiply_ball_hitlag_hurtbox_`: RAM 0x80171368 in `itProcessProcHitCollisions` (`decomp/src/it/itprocess.c:1015`). For a basketball whose `is_fire` is set, the hitlag call uses a2 = 2.0F (`bnezl` executes the delay slot only when taken). Otherwise a2 is unchanged.
6. `add_ball_hitlag_hitbox_`: RAM 0x800E39D0 in `ftMainUpdateDamageStatItem` (`decomp/src/ft/ftmain.c:2642`). For a fire basketball: set the attacker hitlag multiplier to 2.0F, call `ftParamGetHitLag` (0x800EA1C0) with the damage, and store the result in the item `hitlag_tics`.

## REMIX-TODO

- `REMIX-TODO(Smashketball.type)`: `itRemixBasketballIsSoccer()` always returns FALSE. The mode global is not in `remix.sym`.
- `REMIX-TODO(Smashketball.spawn_golden)`: `itRemixBasketballIsGolden()` always returns FALSE, and `set_golden_` is not ported.
- `REMIX-TODO(blast wall)`: `set_golden_` at ITStruct 0x398 is not stored. Other Remix items (BulletBill, Cloud) use the same slot.
- `REMIX-TODO(barrel)`: `check_basketball_barrel_collision_` is not ported. It reads the left barrel list at RAM 0x80131180 (`sFTMainGroundObstacles` in `ftmain.c`, static) and jumps into the middle of `grJungleTaruCannCheckGetDamageKind` (0x8010A018), which is not callable.
- `REMIX-TODO(display)`: the asm hides the ball in the barrel and while respawning by writing DObj byte 0x54. The name of that field is not known here.
- `REMIX-TODO(palette)`: soccer and golden palettes are a data-pointer swap in the item display list (asm lines 139 to 145). Not applied.
- `REMIX-TODO(GFXRoutine.id.BALL_FIRE)` and `REMIX-TODO(GFXRoutine.id.BALL_SMOKE)`: fire and smoke colour animations are skipped. The ids are not in the port.
- `REMIX-TODO(smoke gfx)`: `0x800FE9B4` (smoke, every 4 frames). Not mapped. The fire flame `0x800FE7B4` is mapped to `efManagerFlameLRMakeEffect` (same arguments as the signature).
- `REMIX-TODO(mode hooks)`: Smashketball writes the item fields `is_scored` (GObj 0x40, line 796), `is_above_rim` (0x50, line 723) and `is_scoreable` (0x68, lines 762 and 765), and reads them back (line 673 and 770). The port keeps these in `ITRemixBasketballVars`, which is file-local. The mode port needs exported accessors.
- `REMIX-TODO(hit fields)`: none beyond the layout note below.

## Hit data layout

The asm reads the hit data at offsets that are exactly +4 from the decomp field table names in `build/remixasm/refs/struct_offsets.txt`:
- damage: asm 0x298, decomp `damage_queue` (0x294).
- angle: asm 0x29C, decomp `damage_angle` (0x298).
- direction (lr): asm 0x2A4, decomp `damage_lr` (0x2A0).
- owner gobj: asm 0x2A8, decomp `damage_gobj` (0x2A4).

The port uses the decomp names, which match what `itprocess.c` writes. This is a reading of the table against the asm, not verified against the ROM. Check it before relying on the knockback values.

## Deviations

- State: the asm keeps its Remix state in GObj slots 0x40 to 0x68 (which overlap `buffer_mask` and `gobjscripts`), in ITStruct slots 0x1C8 to 0x1F4, and in 0x2C0. The port keeps it in one static `ITRemixBasketballVars`. This assumes one basketball at a time, which Smashketball guarantees (`basketball_main_` spawns one only when none exists). A second ball would share state.
- `itRemixBasketballDetectCollision` is a copy of `itRemixConkerGrenadeDetectCollision` (`itremixconkergrenade.c`, static). Lead can share it.
- Rounding: `cvt.w.s` rounds to nearest in the asm. The port casts (truncates) in the fire damage, angle, percent halving and the anim timer `trunc.w.s`. Check the fire damage against the ROM.
- Constants with `lui` only: the values are the exact floats of the high halfword. The asm comments are approximate, so the port uses the exact values and notes the hex: the speed divisor is 3.0 (the comment says 4), MAX_SPEED is 270 (the comment says 280), and the fire bounce is 0.94921875.
- `ftCommonDamageGetKnockbackAngle(angle, ga, knockback)`: the asm passes a1 that it does not set. The port passes `ip->ga`. Check this register.
- Trig and math: `cosf`, `sinf`, `atan2f`, `sqrtf` and `fabsf` replace vanilla 0x80035CD0, 0x800303F0, 0x8001863C and the asm `sqrt.s`/`abs.s`.
- `PI` for the fire angle is `3.1415927F`, not verified against RAM 0x8013FE60.
- `itMainClearOwnerStats` is used for the asm call at 0x8017279C (table name). The asm comment says "used with bob-omb and bumper". Check it.
- `itVisualsUpdateSpin` is used for both calls to 0x801713F4 (the asm comments name them "apply rotation" and "apply movement").
- The render routine 0x2E is applied with `gcAddXObjForDObjFixed(joint, 0x2E, 0)`, as in the Gem port. The asm copies and restores three translate words around that call; the port skips that copy.
- `ITStruct` byte 0x2D3 (bit 0x04, asm line 221) is not ported: the decomp field table has no name for it.
- `vel_scale` (0x2C0) is set to 0 at spawn, as the asm timer write.

## Uncertainties (asm line numbers)

- Line 41: transform word `0x1B000000` (asm comment says "either 0x1B000000 or 0x1C000000").
- Lines 113 to 124: the spawn velocity and flags pass-through is inferred from the MakeItem stack use.
- Lines 135 to 137 and 142 to 145: the image and palette selection. Image is ported; palette is REMIX-TODO.
- Lines 162 and 226: the vanilla calls at 0x8017279C and 0x80111EC0 (arrow).
- Lines 270 to 275: the fall of the collision flag (`mask_stat & 0x0800`), with `y < 5.0`.
- Lines 299 and 321: both calls to 0x801713F4.
- Lines 428 to 434: the invincibility timer sequence (85 flash, 1 end).
- Lines 594 and 608: the fire and smoke `a0` positions (`coll_data.p_translate`).
- Lines 620 to 626: the atan2 argument order (`y` first).
- Lines 853 to 866: the direction sign (`lr = -DIRECTION`).

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Hooks applied 2026-10-14 (this session)

- bounce_patch_ (4 sites) + prevent_bounce_fgm_ (3 sites) applied in itMapCheckCollideAllRebound (itmap.c): itRemixBasketballApplyBounce replaces each lbCommonReflect2D with squash/offset/hitlag/FGM then reflects; the default bounce FGM is skipped when is_bounce_scaled.
- multiply_player_hitlag_ applied at ftMainSetHitInteractStats entry (ftmain.c): fire basketball victim -> attacker hitlag_mul = 2.0F.
- multiply_ball_hitlag_hurtbox_ applied in itProcessProcHitCollisions (itprocess.c): hitlag call multiplier 2.0F for the fire ball.
- add_ball_hitlag_hitbox_ applied in ftMainUpdateDamageStatItem (ftmain.c): hitlag_mul 2.0F + ip->hitlag_tics = ftParamGetHitLag(damage_temp, nFTCommonStatusWait, 2.0F) on the raw itMainGetDamageOutput value.
- New exports in itremixbasketball.h: itRemixBasketballApplyBounce/IsFire/IsBounceScaled (kind-checked internally; vars are the asm-faithful single-ball state).
- Deviation vs this note's wording, per asm (bc1fl skips only the squash): bounce hitlag applies to EVERY basketball bounce, scaled or not; squash/offset only at speed >= 40.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
