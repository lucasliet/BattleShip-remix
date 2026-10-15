# 43 SLIPPY notes

Port: `decomp/src/ft/ftremix/slippy/ftremixslippy.c`, table `dFTRemixSlippyProcs` (4 of 4 worklist routines).

## 1. Vanilla patches the Slippy scope relies on

These are `OS.patch_start` hooks in `Slippy/Slippy.asm`, outside the 43_SLIPPY worklist, and not ported.
They are needed for the up special to behave as Remix intends, so they must be applied for the port to match the N64 moveset.

### 1a. Up special duration (`up_special_duration_`, RAM 0x8015C4C8)

- Decomp function: `ftFoxSpecialHiInitStatusVars` (`decomp/src/ft/ftchar/ftfox/ftfoxspecialhi.c:315`).
- Effect: sets the duration word 0x0B24 to `SlippyUSP.DURATION` (64) for Slippy, 30 for everyone else.
- The ported `ftRemixSlippyUSPSpeed` reads 0x0B24 as the frames-left counter, so this patch is required.
- C to insert in `ftFoxSpecialHiInitStatusVars` (after `pass_timer` is reset), under `#ifdef REGION_REMIX`:

```c
#ifdef REGION_REMIX
    *ftRemixFighterWord(fp, 0x0B24) = (fp->fkind == REMIX_ID_SLIPPY) ? SLIPPY_USP_DURATION : 30;
#endif
```

  `SLIPPY_USP_DURATION` is 64 (`SlippyUSP.DURATION`). It must be defined in a shared header, or the literal 64 used with a comment, since the constant is file-local to `ftremixslippy.c`.
  `ftremixfields.h` provides `ftRemixFighterWord`. The vanilla statement is `sw t6, 0x0B24(a0)`, where `a0` is the `FTStruct` in this function.

### 1b. Up special stick_y forced to 0x12C (`up_special_angle_patch_1/2/3_`)

- Patch 1 (RAM 0x8015C550), patch 2 (RAM 0x8015C63C), patch 3 (RAM 0x8015C688): all three force the stick_y used by the up special angle to 300 (`0x12C`) for Slippy, so only x steers the angle.
- Containing decomp functions:
  - 0x8015C550 and 0x8015C63C: inside `ftFoxSpecialHiDecideSetStatus` (`ftfoxspecialhi.c:323`).
  - 0x8015C688: inside `ftFoxSpecialAirHiSetStatusFromGround` (`ftfoxspecialhi.c:364`).
  - Patch 1 and 3 read `0x01C3(s0)` (stick_y); patch 2 branches on the same value.
- C to insert (in each function, where it reads `fp->input.pl.stick_range.y` for the up special angle):

```c
#ifdef REGION_REMIX
    stick_y = (fp->fkind == REMIX_ID_SLIPPY) ? 300 : fp->input.pl.stick_range.y;
#endif
```

  Verify the exact local names against the vanilla code at each site. The vanilla `patch 2` (0x8015C63C) takes the `_branch` path (0x8015C650) for Slippy whatever the stick.

### 1c. Up special collision for ledge grabs (`usp_collision_patch_`, RAM 0x8015C2C0)

- Decomp function: `ftFoxSpecialAirHiProcMap` (`ftfoxspecialhi.c:250`). The vanilla call there is `mpCommonCheckFighterPass(fighter_gobj, ftFoxSpecialHiProcPass)`.
- For Slippy only, the patch skips that vanilla call and instead calls `mpCommonCheckFighterPassCliff` (0x800DE798). If that returns non-zero and `fp->coll_data.mask_stat & 0x3000` is set, it calls `ftCommonCliffCatchSetStatus` (0x80144C24) and returns. Otherwise it returns without the vanilla pass check.
- Field: `0x00D2` is `coll_data.mask_stat` (u16), per `struct_offsets.txt`.
- C to insert, under `#ifdef REGION_REMIX` (check that the callback argument matches the one the vanilla call passes at that point; the asm does not set `a1`, so it reuses the vanilla value):

```c
#ifdef REGION_REMIX
    if (fp->fkind == REMIX_ID_SLIPPY)
    {
        if ((mpCommonCheckFighterPassCliff(fighter_gobj, ftFoxSpecialHiProcPass) != FALSE)
            && (fp->coll_data.mask_stat & 0x3000))
        {
            ftCommonCliffCatchSetStatus(fighter_gobj);
        }
        return;
    }
#endif
```

  The asm jumps to `_end`, which returns to the vanilla code after the patch point, so the Slippy path skips the vanilla `mpCommonCheckFighterPass` call. The `return` above is a placeholder: check it against the tail of `ftFoxSpecialAirHiProcMap` before inserting. Other fighters run the vanilla path unchanged.

## 2. Shared Remix subsystems

None in this scope. The laser uses vanilla `wpManagerMakeWeapon`, `efManagerFoxBlasterGlowMakeEffect` and `syUtilsArcTan2`. No `REMIX-TODO` markers were needed.

## 3. Uncertainties

- Kirby action ids `nFTRemixSlippyKirbyStatusNSPGround = 0x193`. Derived, not read from the ROM. `Kirby/Kirby.asm:437` is the 101st `add_new_action(KIRBY, ...)` (index 100). Base 0x12F comes from `Action.KIRBY.string_0x12F` in `build/remixasm/remix.sym`, which is the first new action after the vanilla 0xDC..0x12E block. Confirm in-game (Kirby copies Slippy NSP and lands in the right status).
- Kirby begin handlers `SlippyNSP.kirby_ground_begin_initial` (0x80575F84) and `SlippyNSP.kirby_air_begin_initial` (0x80575FC0) are not in the worklist and have no `REMIX_SYM_*`, so they are not ported. They are special-entry handlers for Kirby's copy (`ftMainSetStatus` + `ftMainPlayAnimEventsAll` + `ftFoxSpecialNInitStatusVars`), so Kirby's worker should port them if Kirby's table references them.
- Laser descriptor: decoded from the vanilla bytes at ROM 0x10391C (0x28). It matches `dWPFoxBlasterWeaponDesc` except `p_weapon` (Slippy file 6, set per call) and the spawn flags (`WEAPON_FLAG_COLLPROJECT`, bit 31 = Remix 0x80000000). Transform kind 0x1C = `nGCMatrixKindTraRotRpyRSca`.
- DSP air physics: vanilla Fox uses `FTFOX_REFLECTOR_GRAVITY` (0.8F, 0x3F4CCCCD). Remix uses `lui a1, 0x4020` (2.5F). Ported as `SLIPPY_DSP_AIR_GRAVITY`.
- Stick threshold: asm uses `c.le.s` (10 <= magnitude) and branches on false. The port uses `magnitude < 10.0F`. Identical for finite inputs.
- Turn wrap: the asm wraps `stick - prev` into `(-pi, ...)` before the turn step, and wraps the result again after it. Ported as written, not normalized to `(-pi, pi]`.
- Math: `__cosf`/`__sinf` (0x80035CD0, 0x800303F0) have no decomp function name in `vanilla_funcs.txt`; they use the same `__cosf`/`__sinf` helpers as `ftremixwolf.c`. `syUtilsArcTan2` is 0x8001863C.
- Weapon file: `WPDesc.p_weapon` = `fp->data->p_file_special1` (Character.SLIPPY_file_6_ptr), filled at each shot.
- Joint for the laser spawn: N64 0x092C is `fp->joints[17]` (joints at 0x08E8, 4 bytes each). Index 17 is derived arithmetically; verify with the model when it fires.
- Registration: `ftremixported.c` must reference `dFTRemixSlippyProcs` / `dFTRemixSlippyProcsNum` (not edited here).
- Project memory: the ai-memory project scope for this repository could not be resolved (`JRickey/BattleShip` is not a valid project name, and `JRickey` is not a workspace). Only the `default/_global` rules were read.
