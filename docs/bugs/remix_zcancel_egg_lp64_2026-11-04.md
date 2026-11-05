# Z-cancel Egg punishment crashed in ftCommonYoshiEggProcUpdate

Code: `decomp/src/ft/ftremix/ftremixzcancel.c`, called from `ftManagerAllocFighter` (`decomp/src/ft/ftmanager.c`). Test:
`vs.sh` with `SSB64_REMIX_TOGGLES="Punish Failed Z-Cancel=8"` and an aerial A spam input script, once with Mario and once with
Kirby (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

With the setting "Punish Failed Z-Cancel" set to Egg (or Random rolling Egg), a failed Z-cancel on an aerial landing of a
fighter that is not Yoshi crashed with SIGSEGV in `ftCommonYoshiEggProcUpdate`. The fault address was `0x40400000000000e0`: the
low 32 bits looked valid and the high 32 bits were garbage.

## Root cause

Two separate gaps:

1. `ZCancel.asm setup_` loads the Yoshi egg file (file 339, `gFTDataYoshiSpecial3`) in battle scenes when the punishment is Egg
   or Random. The port did not load it, so a match without Yoshi had no egg model.
2. `ZCancel.asm _egg` clears the capture status variable with `sw r0, 0x0B18(player)`. On N64 that word is
   `status_vars.common.captureyoshi.effect_gobj`, a 4-byte pointer. The port wrote `status_vars.common.dead.wait = 0`, a 32-bit
   field at the same offset. On LP64 the pointer is 8 bytes, so only its low half was cleared and the egg update dereferenced
   the leftover high half.

## Fix

- `ftRemixZCancelSetup` (`ftremixzcancel.c`), called from `ftManagerAllocFighter` when the main motion data loads, loads the egg
  file for the Egg and Random modes.
- `ftRemixZCancelEgg` clears `status_vars.common.captureyoshi.effect_gobj = NULL`, the whole pointer.

## Verification

The two runs (Mario, Kirby) with the setting above: the egg shows on the landing, the fighter breaks out, and the run ends with
rc 0.

## Lesson

When porting an asm `sw r0, <offset>` into a union of status variables, pick the member the asm means, not one that happens to
share the offset. A 32-bit member over an N64 pointer leaves half of the LP64 pointer behind.
