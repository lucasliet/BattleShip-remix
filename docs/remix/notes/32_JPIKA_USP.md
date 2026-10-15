# 32 JPIKA USP: port notes

Output: `decomp/src/ft/ftremix/jpika/ftremixjpikausp.c`, table `dFTRemixJPikaUSPProcs` / `dFTRemixJPikaUSPProcsNum`.
Ported: 1 of 1 routine (`JPikaUSP.JPika_SpecialHiProcMap_`, `REMIX_SYM_JPikaUSP__JPika_SpecialHiProcMap_`, 0x8054D558).

Source: `.toolchain/pkgs/smashremix-src/src/JPika/JPikaSpecial.asm` lines 9-62.

## Vanilla callees (all already decomp functions, no patches)

| RAM | Name in decomp | File | Role in the asm |
|-----|----------------|------|-----------------|
| 0x800DDDA8 | `mpCommonCheckFighterOnFloor` | `decomp/src/mp/mpcommon.c:225` | floor test, returns `v0` |
| 0x800DEEC8 | `mpCommonSetFighterAir` | `decomp/src/mp/mpcommon.c:897` | called with `fp` (asm `a0 = a1 = fp`) |
| 0x80152DD8 | `ftPikachuSpecialHiSwitchStatusAir` | `ftpikachuspecialhi.c:257` | air switch, not on floor and no wall |
| 0x80153654 | `ftPikachuSpecialAirHiEndSetStatus` | `ftpikachuspecialhi.c:546` | wall in the air |
| 0x801535F4 | `ftPikachuSpecialHiEndSetStatus` | `ftpikachuspecialhi.c:533` | end of move, when a wall flag is set |

The port's control flow follows the asm, which differs from the vanilla U routine
(`ftPikachuSpecialHiProcMap`, 0x80152B6C). In the asm, after the not-on-floor wall branch, execution falls through
to the second wall check and calls `ftPikachuSpecialHiEndSetStatus` again when the wall flag is still set. The
vanilla U routine returns after the wall branch and does not make that second call.

## Vanilla patches

None. `JPikaSpecial.asm` contains no `OS.patch_start` or hooks into vanilla code.

## Shared subsystems

None.

## Uncertainties

- Lines 13-19, 42-45: the asm loads `fp` from `0x84(gobj)` (`user_data`) and reads `coll_data.mask_curr`
  (`0xCE`) through it. The port uses `ftGetStruct(fighter_gobj)`, which is the same pointer.
- Line 22-25 and 45-48: the port re-reads `fp->coll_data.mask_curr` for the second check. The asm also re-reads
  it there, so the port keeps the re-read. The two reads are identical unless a callee changes the mask
  (none of the three callees writes `mask_curr` as far as the decomp shows).
- Lines 48-49 (`beqzl` branch-likely with `lw ra` in its delay slot): this only restores `ra` on both paths and
  has no effect on C control flow. The port omits it.
