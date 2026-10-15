# F7 notes: Kirby magic hat / copy routines and Piano leftovers (kirby_piano2)

Sources: `.toolchain/pkgs/smashremix-src/src/Kirby/Kirby.asm` (lines 651-1135), `KirbyHats.asm` (magic hat state),
`Piano/Piano.asm` (lines 388-403 and 499-560), `Piano/PianoSpecial.asm` (NSP), `CustomGrabAction.asm`.
Action ids for the clone swap were read from the assembled ROM `build/remixasm/remix.z64` (disassembled with
`tools/remix_disasm_rom.py 0x2D8E4F0 0x400 --vram 0x8058E4F0 --remix`), because the Remix clone actions are numbered
at assembly time and do not appear in the source.

## Patches

| # | Asm / RAM | Vanilla function (vanilla_funcs.txt) | Status | Where |
|---|-----------|--------------------------------------|--------|-------|
| K1 | Kirby.asm `clone_power_action_swap_`, RAM 0x800E6F34 | ftMainSetStatus (ftmain.c:4717, containing function) | Applied | `ftremix/kirby/ftremixkirbyclone.c` `ftRemixKirbyCloneActionSwap`, hook in `ftmain.c` under `REGION_REMIX` after the Dedede hook |
| K2 | Kirby.asm `restore_magic_hat_`, RAM 0x800E6F44 | ftMainSetStatus | Skipped | see "Not ported" |
| K3 | Kirby.asm `kirby_magic_hat`, RAM 0x80151070 and 0x80150EE0 | ftKirbySpecialNSetStatusSelect (0x80151060, ftcommonspecialn.c:84) and ftKirbySpecialAirNSetStatusSelect (0x80150ED0, ftcommonspecialair.c:147) | Skipped | see "Not ported" |
| K4 | Kirby.asm `clear_magic_hat_charge` | no vanilla patch; entered from KirbyHats.asm:593 (`j Kirby.clear_magic_hat_charge`) | Skipped | see "Not ported" |
| P1 | Piano.asm `capture_action_fix_`, custom_capture_dk_interrupt of Piano | ftCommonCaptureShoulderedProcInterrupt (0x8014E4D4, ftcommon/ftcommoncapture.c:115), via the registry call that already exists | Applied, not registered | `ftremixpianoshared.c` `ftRemixPianoCaptureDKInterrupt`, see "Registration" |
| P2 | Piano.asm `extra_part_visibility_`, RAM 0x800E7220 | ftMainSetStatus (containing function, unverified) | Skipped | see "Not ported" |
| P3 | Piano bonus ammo word (N64 0x0ADC) read by the neutral special | none (no C routine) | Nothing to add | see "Piano neutral special" |

## Applied details

- K1: the id pairs (copy power, parent action, clone action) from the ROM disassembly. First match wins per status call,
  as in the asm `beql ... _end`. Only Kirby (fkind 0x08) and J Kirby (0x30) are checked, and the copy power is
  `passive_vars.kirby.copy_id` (N64 0x0ADC).
  - Ganondorf (0x1E): 0x127 to 0x12F, 0x128 to 0x130.
  - Dr. Mario (0x20): 0xE9 to 0x131, 0xEA to 0x132.
  - Dr. Luigi (0x4B): 0xE9 to 0x1C3, 0xEA to 0x1C4.
  - J Mario (0x2A): 0xE9 to 0xE7, 0xEA to 0xE8.
  - Dark Samus (0x22): 0xEE to 0x133.
  - Young Link (0x1F): 0x11F to 0x134, 0x122 to 0x135.
  - Lucas (0x26): 0xFE to 0x136, 0xFF to 0x137.
  - Falco (0x1D): 0xEB to 0x13C, 0xEC to 0x13D.
  - Piano (0x36): 0xE9 to 0x140, 0xEA to 0x141.
  - Wolf (0x37): 0xEB to 0x142, 0xEC to 0x143.
  - Lanky (0x4C): 0xE9 to 0x1C5, 0xEA to 0x1C6.
- P1: the asm returns 1 unconditionally (`lli v0, 0x1` in the delay slot of `jr ra`). It sets the captured fighter's
  `is_invisible` (N64 0x018D bit 0, struct_offsets.txt `0x018D:7-7`) only while the grabber's status is GrabWait
  (0xA8, Action.asm:173). The port gets the grabber fp, so the captured fighter is `grabber_fp->catch_gobj`.

## Not ported, with reasons

- K2 `restore_magic_hat_`, K3 `kirby_magic_hat`, K4 `clear_magic_hat_charge`: all three read and write the
  magic hat state (`KirbyHats.magic_hat_on` per port, `magic_kirby_hat_history`, `magic_kirby_hat_charge` table in
  Kirby.asm:858-872). `magic_hat_on` is written by routines in KirbyHats.asm that are not ported
  (`set_absorbed_hat_` at KirbyHats.asm:525, `set_absorbed_` at 854, `hat_loss_prevent_` at 790, and Size.asm:370).
  Porting them now would leave a state that nothing can set. They need the KirbyHats port first, which is outside
  this scope. K3 also needs `KirbyHats.spawn_with_table_` (KirbyHats.asm:464), the Kirby inhale struct table
  (`Character.kirby_inhale_struct`), and the part helpers at 0x800E8EAC, 0x800E8ECC, 0x800EDF24, 0x8010066C.
  Helper names: 0x800E8EAC is `ftParamSetModelPartDefaultID`, 0x800E8ECC is `ftParamResetModelPartAll`,
  0x800EDF24 is `gmCollisionGetFighterPartsWorldPosition`, and 0x8010066C is `efManagerSparkleWhiteScaleMakeEffect`.
  The hat head part is `fp->joints[6]` (N64 0x0900). The sparkle scale is 0x3FA00000 (1.25F). The Y offset is
  0x43000000 (128.0F).
  - REMIX-TODO(KirbyHats): magic hat state and the hat swap are not ported.
- P2 `extra_part_visibility_`: the asm hides the tongue and held item for Piano outside Grab, GrabPull and
  LightItemPickup through HammerLanding. It sets `is_item_show` (N64 0x0190 bit 1, mask 0x02) and sets the parts'
  special id to -1 (N64 0x0975 per part). It calls 0x800091F4 in a loop over parts 0xE to 0x10 and sets the revert
  flag (0x018C mask 0x10). vanilla_funcs.txt names 0x800091F4 `gcRemoveMObjAll`, but the Piano comment says "push
  model part". That name conflicts with the comment and needs checking before any port. The vanilla map has no entry
  at 0x800E7220. The containing function is not confirmed, so the hook site is unknown.

## Piano neutral special (bonus ammo, P3)

`PianoSpecial.asm` has only the `PianoUSP` and `PianoDSP` scopes. The neutral special is moveset data
(`moveset/PIANO_NSP.bin`, `Piano.asm:69`, edit_action at `Piano.asm:281-282`), so there is no C routine under
`decomp/src/ft/ftremix/piano/` to add the read to. The bonus ammo is already read as an f32 from N64 0x0ADC in
`decomp/src/wp/wpmario/wpmarioremixfireball.c:138` (`ftRemixFighterFloat`, `WP_REMIX_PIANO_BONUS_AMMO`). The absorb
side writes it in `ftremixpianoshared.c` (`ftRemixAbsorbPianoBonusAmmo`). Nothing was added, to avoid duplicating
that read.

## Registration (lead must wire)

- `ftRemixPianoCaptureDKInterrupt` is reachable only through `port_fighter_custom_capture_dk_interrupt` (called in
  `ftcommoncapture.c:130`). The Piano descriptor must set `custom_capture_dk_interrupt` to it. The generated synth
  descriptor (`port/remix/remix_roster.cpp`) does not set this field for any fighter, including Piano and Wario.
- Piano's `custom_capture_action` (ThrownDK, 0xB8) is applied by `ftRemixCommandGrabCaptureAction`, not the registry.

## Uncertainties

1. P1 assumes `grabber_fp->catch_gobj` is the captured fighter. The decomp break-out clears `capture_fp->catch_gobj`
   for the same pair, so the assumption matches the vanilla code. A NULL `catch_gobj` is not guarded. The asm would
   fault too.
2. K1 is not run in a game. The table is the disassembly of the built ROM at the time of this pass. If the Remix
   assembler renumbers the clone actions, the table must be regenerated.
3. `vanilla_funcs.txt` gives `0x800E6F24 ftMainSetStatus` with the line 4580 comment. The current source is at
   ftmain.c:4717. The line number is stale.

## Memory and rules

- ai-memory global rules loaded: `_rules/no-agent-commit-attribution.md`, `_rules/no-live-provider-tests.md`,
  `_rules/ai-memory-operational-recall.md`.
- Project rules could not be loaded. The repo has no `.ai-memory.toml`, and `memory_briefing` rejected both
  `JRickey/BattleShip` and `JRickey`/`BattleShip`.
- This file was not in the repo before this pass. The brief named it, so it was created here.
