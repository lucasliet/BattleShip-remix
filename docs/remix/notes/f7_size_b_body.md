# F7 Size phase (b): body patches (respawn platform, player label, hitbox size, hitbox damage, passive armor)

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm` lines 61-165 and 412-562.
Helpers: `decomp/src/ft/ftremix/ftremixsizebody.c` (new, `#if defined(PORT) && defined(REGION_REMIX)`).
Declarations: appended block at the end of `decomp/src/ft/ftremix/ftremixhooks.h`.
Every hook is under `#ifdef REGION_REMIX`; the vanilla line stays in `#else` where it is replaced.
With the multiplier at 1.0F every hook is a no-op.

## Patches

| RAM | Vanilla function (file) | Size.asm | Status |
|-----|-------------------------|----------|--------|
| 0x80104078 | efManagerRebirthHaloMakeEffect (ef/efmanager.c) | 61-83 adjust_respawn_platform | applied: `scale` scaled before the effect is made (`ftRemixSizeScaleValue`) |
| 0x80111B04 | ifCommonPlayerTagProcDisplay (if/ifcommon.c) | 87-102 adjust_player_label_ | applied: `pos.y += camera_zoom_base` scaled |
| 0x8013B144 | mnPlayersVSFuncStart (per vanilla_funcs.txt) | 106-123 adjust_player_shadow_ | skipped: the map gives a menu function, implausible for a fighter shadow. Owner not identified. Display, phase (c) |
| 0x800E6324 | ftMainProcParams (ft/ftmain.c) | 128-161 passive_armor_ | applied, partly: passive resist +48 above size 1 in the knockback block. Stamina.stamina_armor_fix_ call skipped (no port) |
| 0x800DF394 | ftMainParseMotionEvent (ft/ftmain.c) | 443-452 fix_size_._create_hitbox | applied: MakeAttack2 size, `trunc(size * m) * 0.5` |
| 0x800DF6AC | ftMainParseMotionEvent | 449-452 fix_size_._change_hitbox_size | applied: SetAttackCollSize, same formula |
| 0x800DF34C | ftMainParseMotionEvent | 470-557 fix_damage_ | applied, partly: damage scaled at SetAttackCollDamage. Charged smash block skipped (REMIX-TODO(ChargeSmash)) |

## Deviations

- D1. `adjust_hitbox` in the asm scales only the hitbox size, not its offset. The macro comment says "size and location" but the code changes only the size.
- D2. `fix_damage_` factors are lui-only constants: `0x3F330000` = 0.69921875F and `0x3FA60000` = 1.296875F, not 0.7 and 1.3 (asm lines 541-542). The port uses the exact values.
- D3. Rounding: `fix_damage_` uses `cvt.w.s` (round to nearest, ties to even, default FCSR). The port rounds the same way. Hitbox size uses `trunc.w.s` and is a C cast.
- D4. `passive_armor_` writes only the register f12 in the asm, so the port uses a local `passive_resist` and leaves `fp->knockback_resist_passive` unchanged.

## REMIX-TODO

- `REMIX-TODO(ChargeSmash)` in ftmain.c, SetAttackCollDamage case: Size.asm lines 477-527 (`Toggles.read(entry_charged_smashes)`, `ChargeSmashAttacks.charged_smash_fighter_array`). No port exists in the tree.
- `REMIX-TODO(Stamina)`: `Stamina.stamina_armor_fix_` call in passive_armor_ (Size.asm line 136). No port exists.

## Uncertainties

- U1. The create-hitbox damage store (ftmain.c MakeAttack1, around line 253) is not scaled. The asm has one damage hook, at 0x800DF34C (+0x25C from ftMainParseMotionEvent). That offset fits the SetAttackCollDamage case better than the create path (+0x100). Verify with `tools/remix_disasm_rom.py` once `baserom.us.z64` is present.
- U2. `vanilla_funcs.txt` line numbers are stale. Function names were checked by name.
- U3. The player shadow mapping (0x8013B144) lands in a menu function. Check the owner with the disassembly before porting.
- U4. The passive armor compare order in ftMainProcParams is inferred from the C (`status < passive`), since the asm does not show the matching vanilla instructions.

## Verification

`scripts/remix/syntax-check.sh` returned `syntax-check OK` for ftremixsizebody.c, ftremixhooks.h, efmanager.c, ifcommon.c and ftmain.c. No build, cmake, game or git run, per the brief.
