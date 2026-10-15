# 0x41 GOEMON, USP (Magic Cloud): port notes

Port file: `decomp/src/ft/ftremix/goemon/ftremixgoemonusp.c`
Table: `dFTRemixGoemonUSPProcs` / `dFTRemixGoemonUSPProcsNum` (10 entries, all `GoemonUSP.*` routines in the worklist).
Source: `.toolchain/pkgs/smashremix-src/src/Goemon/GoemonSpecial.asm` lines 1273-1891.

Not wired: `ftremixported.c` must add `extern` declarations and the table to its list. That file is outside this worker's scope.

## 1. Vanilla patches

- None in the GoemonUSP scope. No `OS.patch_start` or `j`/`jal` hooks into vanilla code.
- `Goemon.asm` lines 492-497 are `Character.table_patch_start(ground_usp | air_usp, ...)`. They point the action table at the routines and are handled by the Remix binder (`ftremixbind.c`). Nothing needs to be inserted in vanilla C.
- Vanilla calls used by the port, with their decomp names:

| RAM | Decomp name | Used by |
|---|---|---|
| 0x800E6F24 | ftMainSetStatus | air_initial_, shared_initial_ |
| 0x800E0830 | ftMainPlayAnimEventsAll | air_initial_, shared_initial_ |
| 0x800DEEC8 | mpCommonSetFighterAir | ground_initial_ |
| 0x801438F0 | ftCommonFallSpecialSetStatus | attack_main_, jump_main_, escape_main_ |
| 0x800D9160 | ftPhysicsApplyAirVelDriftFastFall | jump_physics_ |
| 0x801728D4 | itMainDestroyItem | destroy_attached_cloud_ |
| 0x800269C0 | func_800269C0_275C0 (FGM play) | air_initial_ (sfx 0x500) |

- Unnamed in decomp, declared locally: 0x8001863C is `extern f32 atan2f(f32 y, f32 x)`.
- `fcos` (0x80035CD0) and `__sinf` (0x800303F0) are used as the decomp names `fcos` and `__sinf`. The address-to-name match is inferred from `libultra/gu/cosf.c` and `sinf.c`, not checked in `vanilla_funcs.txt`.

## 2. Shared Remix subsystems (REMIX-TODO)

- REMIX-TODO(Item): `attach_cloud_` (asm 1851-1854). `Item.Cloud.SPAWN_ITEM` is a custom Remix item and is not ported. No cloud is spawned, so the attached cloud stays NULL. Anything that depends on the cloud does nothing: the on-hit destroy path and the Magic Cloud visuals.
- Skipped with the spawn: `Size.item.render_routine_` is stored into the cloud's GObj at 0x2C (asm 1857-1858). It goes with the Item port.
- The cloud spawn position (`ftSamusSpecialNGetChargeShotPosition`, 0x8015D35C, asm 1839) is not called. Its result only feeds the spawn.

## 3. Copied code (`collision_`)

- `GoemonUSP.collision_` (asm 1789-1797) splices Mario's up special landing check, `ftMarioSpecialHiProcMap` (0x80156358), from ROM 0xD0D98 (0x78 bytes) and ROM 0xD0E14 (0x44 bytes). The bytes were decoded with `tools/remix_disasm_rom.py`. Its default vram mapping does not apply here, but the decoded code matches the decomp's `ftMarioSpecialHiProcMap` line for line. The port is a copy of that function with Goemon's landing lag.
- The replaced instruction is the `lui a2, 0x3E8F` at ROM 0xD0E10. The next instruction, `ori a2, a2, 0x5C29` at ROM 0xD0E34, is kept. So the lag is `0x3E805C29` (about 0.2507031), not the exact `0x3E800000` (0.25). The port uses `GOEMON_USP_LANDING_LAG = 0.2507031261920929F`.

## 4. Uncertainties

1. **air_initial_ 0x18D flag byte** (asm 1333-1335). `ori t6, r0, 7; and v1, v1, t6; sb v1, 0x18D(a0)` masks a register (`v1`) that the routine never loads, so the stored low three bits are whatever `v1` held on entry. The port clears the four named bits (`is_absorb`, `absorb_lr`, `is_goto_attack100`, `is_fastfall`) and leaves `is_magnify_show`, `is_limit_map_bounds`, `is_invisible` alone. This is the same choice as the Falco port.
2. **Cloud pointer cleared after attach** (asm 1316, 1319, 1804). `attach_cloud_` runs first, then `goemon_on_hit_subroutine_establishment_` sets `0x0B20` (the attached cloud) to 0. When the cloud is ported, the cloud will be created but not referenced, so the on-hit destroy will never find it. The port keeps this order as written. Check it once the Item is ported; it may be a bug in the Remix source.
3. **Frame timer write order** (asm 1402-1406, 1493-1497). `sw t6, 0x0B18(v1)` sits in the delay slot of `bnez`, so the timer is incremented on every frame. The port increments first, then compares the old value.
4. **Slot names.** The decomp has no Goemon name for these FTStruct slots, so the port reuses decomp names for the same offsets via the macros at the top of the file:
   - `0x0B18` = `status_vars.common.dead.wait` (frame timer).
   - `0x0B20` = `status_vars.common.tarucann.tarucann_gobj` (attached cloud).
   - `0x0AE4` = `passive_vars.kirby.copysamus_charge_recoil` (charge level, reset in the on-hit routine). The name is misleading, but the slot is only written here.
5. **Shield bitmask** (asm 1626-1627). `0x01B8` is read as `input.button_mask_z`, the decomp name for that offset. It is AND-ed with `button_tap` (`0x1BE`), as the asm does. Confirm the bit's meaning if escape behaves oddly.
6. **atan2 operand order** (asm 1729-1731). The asm loads `f12 = Y_DIFF` and `f14 = X_DIFF` before `jal 0x8001863C`. The port calls `atan2f(diff_y, diff_x)`, following the asm's `atan2(f12,f14)` comment. The function name is an assumption, and the implementation at 0x8001863C is not checked.
7. **Special fall arguments** (asm 1586-1594 and similar). Read from the O32 argument slots: `a1` = drift 0.9375, `a2` = `unknown` = FALSE, `a3` = `is_fall_accelerate` = TRUE, stack `0x10` = `is_goto_landing` = FALSE, stack `0x14` = `landing_lag` = 0.25, stack `0x18` = `is_allow_interrupt` = FALSE. The decomp parameter names come from `ftcommonfallspecial.c` line 72.
8. **Stick deadzone** (asm 1668-1677). Neutral means `|stick_x| < 11` and `|stick_y| < 11`. The neutral branch stops only when speed is at most `MIN_SPEED` (3.0); otherwise it steers toward zero. The asm's `bc1fl` delay-slot behaviour was traced, and the port matches it.
9. **Turn check** (asm 1378-1389). `stick_x * DIRECTION < -39` flips DIRECTION. The multiply is done in signed 32-bit, which matches the asm's `multu` plus `slti`.

## 5. Helpers (static, not in the table)

`ftRemixGoemonUSPDestroyAttachedCloud`, `ftRemixGoemonUSPOnHit` (installed as `proc_damage`), `ftRemixGoemonUSPAttachCloud`, `ftRemixGoemonUSPOnHitEstablishment`, `ftRemixGoemonUSPInitStatusVars`, `ftRemixGoemonUSPSharedInitial`, `ftRemixGoemonUSPCheckTurn`, `ftRemixGoemonUSPBeginFall`, `ftRemixGoemonUSPBeginFallAtAnimEnd`, `ftRemixGoemonUSPIsStickNeutral`, `ftRemixGoemonUSPStopBelowMinSpeed`, `ftRemixGoemonUSPGetStickTarget`, `ftRemixGoemonUSPAccelerateToward`.

`goemon_on_hit_subroutine_` is not in the worklist. It is still ported as a static C function, since the asm installs it through `proc_damage`.
