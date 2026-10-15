# 26 LUCAS NSP: port notes

Output: `decomp/src/ft/ftremix/lucas/ftremixlucasnsp.c`, table `dFTRemixLucasNSPProcs` / `dFTRemixLucasNSPProcsNum`.
Ported: 1 of 1 routine (`LucasNSP.air_move_`, `REMIX_SYM_LucasNSP__air_move_`, 0x80549FA0).

Source: `.toolchain/pkgs/smashremix-src/src/Lucas/LucasSpecial.asm` lines 14-54.
Referenced by `Lucas.asm:122` (`edit_action(LUCAS, 0xE3, ..., LucasNSP.air_move_, ...)`) and by the Kirby/JKirby
`LUCAS_NSP_Air` actions (`Kirby/Kirby.asm:345`, `JKirby/JKirby.asm:221`). Neither Kirby site adds a Kirby branch to
this routine, so none is ported.

## Vanilla patches

None. `LucasSpecial.asm` contains no `OS.patch_start` or `j`/`jal` hooks.

## Shared subsystems

None. The routine uses only FTStruct fields.

## Uncertainties

- Line 26-27, 37: the asm reads temp variable 1 (`0x17C`, `motion_vars.flags.flag0`) and the facing (`0x44`, `lr`)
  from `a2`, which the asm header calls "player struct". The port takes the player struct from `fighter_gobj`
  via `ftGetStruct`. This assumes `a2 == ftGetStruct(a0)` at every call site, which holds for action-table
  calls from the fighter's status proc.
- Line 38: `cvt.s.w` converts `lr` (int) to float, and the port does the same through the implicit
  `s32 -> f32` conversion in `LUCAS_NSP_AIR_X_SPEED * fp->lr`.
- Line 42: the port keeps the asm's form `vel_x - (X_SPEED * facing)` in that order. Float evaluation order
  matches for single operations.
