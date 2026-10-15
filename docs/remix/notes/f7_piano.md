# F7 Piano leftovers: vanilla patches from PianoSpecial.asm

Sources: `.toolchain/pkgs/smashremix-src/src/Piano/PianoSpecial.asm` (patches at lines 985-1148) and
`Piano.asm` (patch list at lines 327-499). Vanilla map: `build/remixasm/refs/vanilla_funcs.txt`.

## Patches

| # | ROM / RAM | Vanilla function | Status | Where |
|---|-----------|------------------|--------|-------|
| 1 | 0xCFCB8 / 0x80155278 (ground absorb initial) | ftNessSpecialLwProcAbsorb (0x8015521C, `ftchar/ftness/ftnessspeciallw.c:41`) | Already applied by an earlier change, verified | `ftnessspeciallw.c` line 57 calls `ftRemixPianoDSPAbsorbRedirect`, which picks the ground or air absorb status by `fp->ga` |
| 2 | 0xCFCC8 / 0x80155278 (air absorb initial) | same as 1 | Already applied, verified | same redirect call, the air branch |
| 3 | 0xC733C / 0x8014C8FC (command grab action fix) | ftCommonCaptureYoshiProcCapture (0x8014C83C, `ftcommon/ftcommoncaptureyoshi.c:87`) | Applied | see below |
| 4 | 0x5EBAC / 0x800E33AC (absorb behaviour) | ftMainUpdateAbsorbStatWeapon (0x800E3308, `ftmain.c:2537`) | Applied | see below |

### 3. Command grab action (0x8014C8FC)

The vanilla instruction at 0x8014C8FC is `addiu a1, zero, 0xB1` (Action.EggLayPulled), the action argument of
`ftMainSetStatus(fighter_gobj, nFTCommonStatusCaptureYoshi, 0.0F, 1.0F, ...)` (`ftcommoncaptureyoshi.c:117`).
The patch swaps that action for the captured fighter from the grabber's fkind:

- Piano: `Action.ThrownDK` (0xB8)
- Dragon King, Marina, and Kirby or J Kirby copying Marina: `Action.Thrown2` (0xBA)
- others: `Action.EggLayPulled` (0xB1), unchanged

The patch covers all four grabbers in one block, so it is applied in full, not only for Piano. The grabber is read
from `this_fp->capture_gobj`, which line 110 sets before the status call.

C: `ftRemixCommandGrabCaptureAction(capture_gobj)` in `ftremixpianoshared.c`, called under `#ifdef REGION_REMIX` in
`ftcommoncaptureyoshi.c`. The `#else` branch keeps the original status call.

### 4. Absorb behaviour (0x800E33AC)

The hook is the `mtc1 t8, f8` at 0x800E33AC, inside the `!can_not_heal` block of `ftMainUpdateAbsorbStatWeapon`. The
vanilla healing is `(s32)(damage * 2.0F)` (asm 0x800E33B4-0x800E33C4). For Mad Piano the patch:

- heals `(s32)(damage * 0.75F)` instead, and the vanilla `percent_damage` update (0x800E33C8 onward) runs unchanged;
- adds `damage * (1/3)` to the float at FTStruct N64 0x0ADC (passive_vars) and clamps it to 5.0.

C: `ftRemixAbsorbHealAmount` and `ftRemixAbsorbPianoBonusAmmo` in `ftremixpianoshared.c`, called under
`#ifdef REGION_REMIX` in `ftmain.c`. The bonus ammo helper is a no-op for other fighters.

## Helpers and declarations

- New file: `decomp/src/ft/ftremix/ftremixpianoshared.c` (wrapped in `#if defined(PORT) && defined(REGION_REMIX)`).
  CMake picks it up through `GLOB_RECURSE`, so a re-configure is needed.
- `decomp/src/ft/ftremix/ftremixhooks.h`: block appended before the final `#endif` (three prototypes).
- Vanilla edits: `ftcommon/ftcommoncaptureyoshi.c` (include plus the status call), `ftmain.c` (the healing lines).

## REMIX-TODO

None added. The Remix bonus ammo is stored but not consumed here: the Piano neutral special (no port yet in this
scope) must read FTStruct N64 0x0ADC as a float.

## Not applied, outside the brief's list

- `Piano.asm` line 499: `OS.patch_start(0x62A20, 0x800E7220)`, the Piano tongue and held-item visibility
  (`extra_part_visibility_`). It is not in `36_PIANO_DSP.md` or `36_PIANO_USP.md`, and it calls the model part helper
  `0x800091F4` with a loop, so it was left alone. The lead should decide whether it belongs to this wave.
- `Piano.asm` lines 388-403: `capture_action_fix_`, a Remix action-table routine (ThrownDK interrupt) that makes the
  grabbed fighter invisible and returns 1 to skip the vanilla ThrownDK break-out. It is table data, not a vanilla
  patch, and the port has no routine for it yet (`custom_capture_dk_interrupt`).
- `Piano.asm` lines 383-387: `custom_capture_action` (`dw Action.ThrownDK`) is a Remix table entry; the
  `ftRemixCommandGrabCaptureAction` helper uses the same value, but the table itself is not in this port.

## Map discrepancies (vanilla_funcs.txt)

- `0x800E3308 ftMainUpdateAbsorbStatWeapon ... ftmain.c:2460` gives a stale line. The function starts at
  `ftmain.c:2538` (source comment `// 0x800E3308`). The address mapping is correct; the line number is not.
- `0x80155278` has no entry, and no entry between 0x8014DBE0 and 0x80155CF4. It falls inside ftNessSpecialLwProcAbsorb
  (source comment `// 0x8015521C`). The file has the redirect already, so no edit was needed.

## Uncertainties

- The Remix patch 4 leaves `t1` for the vanilla line at 0x800E33C8 (`subu v0, t9, t1`). The Piano path sets `t1` to the
  0.75x heal and loads `t9` (percent_damage) before the branch, so the vanilla subtraction uses both. This follows the
  asm; it was not run in a game.
- `ftRemixCommandGrabCaptureAction` returns `nFTRemixCommandGrabActionEggLayPulled` (0xB1) for other fighters. In
  vanilla this equals `nFTCommonStatusCaptureYoshi`, which the disassembly confirms (`addiu a1, zero, 0xB1`).
- The Thrown2 and ThrownDK ids (0xBA, 0xB8) come from `Action.asm` lines 189 and 191. They do not match the vanilla
  `nFTCommonStatusThrown*` names, so the enum uses Remix ids.
- Kirby copy check: the copied power is read from `passive_vars.kirby.copy_id` (N64 0x0ADC), the same as
  `ftremixlinkshared.c`, and compared with `REMIX_ID_MARINA`.
