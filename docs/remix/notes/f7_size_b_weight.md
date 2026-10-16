# F7 Size phase (b): fighter weight (Size.asm adjust_player_weight_1..8)

Source: `.toolchain/pkgs/smashremix-src/src/Size.asm` lines 843-1185. Each patch loads the attribute weight
(`0x0068(attr)`), multiplies it by `1 + FTREMIX_SIZE_COEF_WEIGHT * (size - 1)` (-0.24, `weight_multiplier`) and
hands the result to the vanilla knockback formula. The size is `multiplier_table[port]`, port = byte 0x0D.

## Files

- New: `decomp/src/ft/ftremix/ftremixsizeweight.c`, `ftRemixSizeWeight(FTStruct *fp, f32 vanilla_weight)`
  (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`).
- Appended block: `decomp/src/ft/ftremix/ftremixhooks.h` (before the closing `#endif`, nothing reordered).
- Vanilla edits (each `#ifdef REGION_REMIX` with the vanilla line kept in `#else`):
  `decomp/src/ft/ftmain.c`, `decomp/src/ft/ftcommon/ftcommontwister.c`,
  `decomp/src/ft/ftcommon/ftcommonthrown2.c`, `decomp/src/ft/ftcommon/ftcommoncapture.c`,
  `decomp/src/gr/grcommon/grinishie.c`.

## Patches

| # | RAM | Vanilla function (file) | Site | Status |
|---|-----|-------------------------|------|--------|
| 1 | 0x800E3F88 | ftMainProcessHitCollisionStatsMain (ft/ftmain.c) | fighter hit, `attr->weight` of this_fp | applied |
| 2 | 0x800E4120 | ftMainProcessHitCollisionStatsMain | weapon hit (projectiles), `attr->weight` | applied |
| 3 | 0x800E4244 | ftMainProcessHitCollisionStatsMain | item hit, `attr->weight` | applied |
| 4 | 0x80143D30 | ftCommonTwisterShootFighter (ft/ftcommon/ftcommontwister.c) | tornado knockback | applied |
| 5 | 0x8014B084 | ftCommonThrownReleaseThrownUpdateStats (ft/ftcommon/ftcommonthrown2.c) | back throw knockback | applied |
| 6 | 0x8014B3F0 | ftCommonThrownSetStatusDamageRelease (ft/ftcommon/ftcommonthrown2.c) | damage release knockback | applied |
| 7 | 0x8014E60C | ftCommonCaptureShoulderedSetStatus (ft/ftcommon/ftcommoncapture.c) | cargo hold damage knockback | applied |
| 8 | 0x80108DE8 | grInishieScaleGetPressure (gr/grcommon/grinishie.c) | `weight = 1 - attr->weight + 1.4` | applied |

All eight write the scaled weight into the knockback argument only. Vanilla is unchanged without `REGION_REMIX`,
and the multiplier is 1.0F by default, so the Remix build is a no-op until a source sets the size.

## Deviations and uncertainties

- U1. Owner of the multiplier. The asm reads the port byte from registers s5, s0, s1, s2 (not from `attr`). The C
  uses `this_fp` (the fighter whose weight is read) for every site. Checked against the vanilla C only, not the
  register contents. Verify once the ROM disassembly is available.
- U2. Patch 8 (asm line 1173-1174) multiplies `f4` and does not restore it, so the new value feeds the vanilla
  `1 - weight + 1.4` directly. The C does the same by scaling the weight inside the expression.
- U3. `ftMainProcessHitCollisionStatsMain` ground branch (`gr_attack_coll`, ftmain.c, `attr->weight`) has no
  Size.asm patch in this range (asm offsets stop at +0x388). Left vanilla.
- U4. `ftcommonthrown2.c` `ftCommonThrownSetStatusNoDamageRelease` (`fp->attr->weight, 9`) has no Size.asm patch.
  Left vanilla.
- U5. `vanilla_funcs.txt` line numbers are stale (see the phase (a) note U2). Sites were located by function name
  and by the offset from the function start in the asm mapping.
- U6. Only the eight weight patches are in this change. The other phase (b) physics patches (walking, dash, jump,
  `fix_size_`, `fix_damage_`) are not touched here.

## REMIX-TODO

None added. No subsystem gating is needed: the weight helper is pure and the multiplier table is in place.

## For the lead

- Wire: nothing to register. `ftremixsizeweight.c` sits in `ft/ftremix/` next to `ftremixsize.c` (phase a), so it
  is built the same way.
- Test: default 1.0F means no change. Expected only once a source sets the size (none yet).
- Not verified: no build, no game run, no git (per brief). Syntax check OK for all seven touched files.
