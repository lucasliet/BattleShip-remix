# Peppy's down special crashed: the 8-byte flashbang pointer overlapped the revolver ammo word

Code: `decomp/src/it/itremix/itremixflashbang.c` (`itRemixFlashbangSetActive`, `itRemixFlashbangGetActive`),
`decomp/src/ft/ftremix/peppy/ftremixpeppydsp.c` (`ftRemixPeppyDSPActiveGrenade`). Test: `lldb_count.sh` runs with a CPU
Peppy (`.claude/skills/remix-smoke-tests/SKILL.md`).

## Symptom

Found by the Phase 6 acceptance probes (2026-11-06): two CPU-only matches with Peppy (fkind 66), `p6q_chase` (Falco, Wolf,
Slippy, Peppy) and `p6q_charge` (Sheik, Peppy, Mewtwo, J Samus), stopped on `EXC_BAD_ACCESS` in
`ftRemixPeppyDSPDetonateCheck` (`ftremixpeppydsp.c:145-146`), reading the item struct of the grenade. The faulting
addresses were `0x700000334` and `0x412cddda60`: 64-bit values whose upper half is a small number.

## Root cause

The asm keeps the flashbang in flight in the owner's player-struct word 0x0ADC (`items/Flashbang.asm:158`, cleared at
:508 and :854). The port stored the 64-bit `GObj *` at the start of `passive_vars`, which is the port location of N64
word 0x0ADC, so the pointer also covered word 0x0AE0. For Peppy that word is the revolver (neutral special) ammo, which the
charge code writes as a 32-bit value. Once Peppy charged the revolver with a grenade in flight, the upper half of the pointer
held the ammo, and the next detonate request dereferenced it. Same class as the Z-cancel egg pointer cleared by a 32-bit
store (`remix_zcancel_egg_lp64_2026-11-04.md`).

## Fix

Word 0x0ADC keeps the asm meaning, an in-flight flag (1 or 0, written with `ftRemixFighterWord`), and the `GObj *` lives in
a table indexed by the owner's port. `itRemixFlashbangGetActive` returns the grenade only while the flag is set and the
pointer names a live flashbang of this owner on the item link; otherwise it clears both. The check also covers an item that
leaves by a path other than the explosion or the blast zone, which keeps the word set in the asm too: on the N64 that stale
read is harmless, here the GObj may be reused.

## Verification (2026-11-06)

`p6r_chase` and `p6r_charge` (the same matches, 9000 frames): no crash; a CPU Peppy grenade is made in each.
`p6s_peppy_dsp`: a human Peppy uses the down special every 150 frames (`inputs/dsp_tap.txt`) and throws a new grenade
after each one is gone.
