# 0x3A MARTH: port notes

Source: `.toolchain/pkgs/smashremix-src/src/Marth/MarthSpecial.asm`.
Port: `decomp/src/ft/ftremix/marth/ftremixmarth.c`, table `dFTRemixMarthProcs` (18 entries, all of the worklist). Syntax check passes.

## 1. Vanilla patches (not applied; for the vanilla owner)

### 1a. Counter damage patch, RAM 0x800EA248

- Source: `MarthSpecial.asm` lines 1153-1197, `OS.patch_start(0x65A48, 0x800EA248)`.
- Vanilla function: `ftParamUpdateDamage(FTStruct *fp, s32 damage)` in `decomp/src/ft/ftparam.c:1699`.
- Effect: for Marth (fkind `REMIX_ID_MARTH`) in DSP or DSP air (`0xEF`, `0xF1`) with temp variable 1 (`motion_vars.flags.flag0`) set, damage becomes 0 and the counter hit flag (0x0B18) is set to 1.
- Helper: `s32 ftRemixMarthDSPCounterDamage(FTStruct *fp, s32 damage);` is non-static in `ftremixmarth.c`. Add this prototype to a header included by `ftparam.c`, or declare it at the top of `ftparam.c`.
- C to insert at the top of `ftParamUpdateDamage`, under `#ifdef REGION_REMIX`:

```c
#ifdef REGION_REMIX
    damage = ftRemixMarthDSPCounterDamage(fp, damage);
#endif
```

  Note: the port file is wrapped in `#ifdef PORT`. Use `#if defined(REGION_REMIX) && defined(PORT)` if the non-PORT build defines `REGION_REMIX`.

## 2. Shared subsystems not ported

- `PokemonAnnouncer.counter_announcement_` (`PokemonAnnouncer.asm` line 1219). Called from `MarthDSP.ground_attack_initial_` (`MarthSpecial.asm` line 952) and `MarthDSP.air_attack_initial_` (line 974). The port marks both with `REMIX-TODO(PokemonAnnouncer)` and skips the voice line. The status change still runs.
- `MarthNSP.kirby_ground_collision_` (`MarthSpecial.asm` line 811) is not in the Marth worklist and not ported. Kirby's action table (`Kirby/Kirby.asm` lines 370-374, Kirby actions `MARTH_NSPG_*`) references it. It calls `0x800DDDDC` (`mpCommonProcFighterOnFloor`) with `ground_to_air_`. It belongs to the Kirby worker.

## 3. Helpers ported as static (not in the table)

- USP: `air_control_` (`ftRemixMarthUSPAirControl`), the air drift copy (`ftRemixMarthUSPAirDrift`), and the BEGIN/BEGIN_MOVE/END_MOVE phases.
- NSP: `ground_shared_initial_`/`air_shared_initial_` (`ftRemixMarthNSPSharedInitial`, `ftRemixMarthNSPAirSharedInitial`), `ground_2/3_initial_` and `air_2/3_initial_`, `ground_to_air_` and `air_to_ground_`.
- DSP: `ground_to_air_` and `air_to_ground_`, `ground_attack_initial_` and `air_attack_initial_` (`ftRemixMarthDSPStartAttack`).

## 4. Other facts recorded for review

- Kirby action ids are not in any generated symbol file. They were derived from `Character.asm` line 681 (`Action = action_array_size.KIRBY / 0x14 + index + 0xDC`). KIRBY array size is `0x67C` (`Character.asm`, `scope action_array_size`), giving base `0x12F`. The index is the position among `add_new_action(KIRBY, ...)` lines in `Kirby/Kirby.asm` (MARTH_NSPG_1 is index 33). Values: NSPG_1 `0x150`, NSPG_2_Mid `0x152`, NSPG_3_Mid `0x155`, NSPA_1 `0x157`, NSPA_2_Mid `0x159`, NSPA_3_Mid `0x15C`. Verify against the assembled ROM if possible.
- Marth ids: `DSP_Ground` 0xEF, `DSP_Ground_Attack` 0xF0, `DSP_Air` 0xF1, `DSP_Air_Attack` 0xF2 (order of `Marth.asm` lines 462-465, matches the `DSPG*` constants).
- Raw word access (`ftRemixMarthWordAt`) is used for words the decomp union does not name cleanly: 0x0980 Kirby hat (a byte that overlaps `modelpart_status`), 0x0ADC/0x0AE0 pseudo-jump flags, 0x0B18 counter hit, 0x0B20 NSP stage.
- The USP aerial physics is the 16 instructions at ROM 0x548F0 (`OS.copy_segment(0x548F0, 0x40)`, `MarthSpecial.asm` line 167). Decoded from `baserom.us.z64` with a local disassembler. Semantics: `if (fp->is_fastfall) ftPhysicsApplyFastFall else ftPhysicsApplyGravityDefault`, then `if (ftPhysicsCheckClampAirVelXDecMax(fp, attr) == FALSE)` run the air control and `ftPhysicsApplyAirVelXFriction`. Air control is `air_control_` when temp variable 3 is END (5), otherwise a re-run of the check, which changes nothing.
- The USP main copy (`OS.copy_segment(0xD7190, 0x20)`, line 89) is 8 instructions of Fox's up special end (`0x8015C750`). It checks `anim_frame <= 0.0F`, then calls `ftCommonFallSpecialSetStatus(gobj, 1.0F, FALSE, TRUE, FALSE, 0.375F, FALSE)`. The 5th argument (asm stack 0x10, labelled "unknown") maps to `is_goto_landing`.
- The USP collision (`OS.copy_segment(0xD0D98, 0x78)` and `0xD0E14, 0x44`, lines 347-352) is `ftMarioSpecialHiProcMap` (`0x80156358`) with the landing lag changed to 0.375F. Ported in `ftRemixMarthUSPProcMap`.
- The USP change direction calls `ftCaptainSpecialHiProcInterrupt` (`0x80160370`).

## 5. Uncertainties

- `MarthSpecial.asm` line 251: the comment says 0.375, but `lui t0, 0x3F00` loads 0.5F. The port uses 0.5F (`MARTH_USP_STICK_Y_FACTOR`).
- Lines 236-238 and 259-264: when `stick_x * direction < 10`, the branch skips the stick update. `f2` still holds 10.0 and `_apply_movement` adds `X_SPEED` again, so the x velocity is `20 * direction`. The port keeps this.
- Lines 691-692 and 749-750: the comment says "temp variable 1" but the load is `0x0180`, which is temp variable 2 (`motion_vars.flags.flag1`). The port uses `0x0180`, which matches the code.
- Line 1016-1017: the rotation constant at RAM `0x8012FE90` was not located. The port uses `F_CST_DTOR32(90.0F)` (`MARTH_DSP_FACING_ROTATION`), which is an assumption. Verify against the RAM value.
- Lines 384 and 519: `lb t0, 0x0980(v1)` uses `v1` without setting it. In every caller, `v1` still holds the player struct, so the port reads the hat from `fp`.
- Lines 519-524 (air shared): the pseudo-jump compare uses the original action id (`sw a1, 0x0024(sp)` before the Kirby `+101`). The port passes the pre-adjust id.
- Lines 378-381 and 415-419: Kirby and J Kirby share one branch. Kirby and J Kirby both take the Kirby action ids.
- Armor (`0x07E8`) is written as `knockback_resist_status` (the decomp's name for this f32). Set to `131072.0F` when temp 1 is set, else `0.0F`.
