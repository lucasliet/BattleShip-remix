# F14 notes: Remix AI leftovers (scope `ai_left`, ROADMAP T8.2 "Sobras do M7")

Source of truth: `.toolchain/pkgs/smashremix-src/src` (AI.asm, Sheik/AI/Attacks.asm, Bowser, Banjo). Nothing was built or run.
ai-memory: the project scope could not be resolved (`workspace 'JRickey' not found`), so project rules were not loaded.
Global rules were read.

## Status per task

| # | Item | State | Where |
|---|------|-------|-------|
| 1 | `cpu_post_process` per-fighter table (AI.asm 3867-3878) | Table in place, Sheik only | `ftremixaipostprocess.c` (`ftRemixAiPostProcessFighter`), `ftremixaipostsheik.*`, `ftremixaipostsheikrules.*` |
| 2 | `auto_jab_grab_` (AI.asm 3584-3631) | REMIX-TODO(AI), not ported | `ftcommon/ftcommoncatch1.c:200` |
| 3 | `PUFF_DSP` (AI.asm 3180-3200) | Not ported: unreachable in Remix (see below) | none |
| 4 | `weight_attack_options` at levels below 10 | Checked: nothing to port | `ftremix/ftremixaiweight.c:20` |
| 5 | P11 for Sheik, Peppy, Bowser, GBowser; P06 Banjo | Bowser, GBowser and Banjo ported; Sheik and Peppy REMIX-TODO(AI) | `ftremixaispecials.*`, `ftremixairecovery.*`, `ftcomputer.c` (ChargeSpecialN) |

## Task 1: per-fighter post process table

Asm: `AI.asm` 3867-3878 reads `Character.cpu_post_process.table[fkind]` and calls the entry with a0 = player struct.
Entries are installed in `Sheik/AI/Attacks.asm` 150, `Bowser/AI/Attacks.asm` 99, and the shared files listed in the REMIX-TODO
of `ftremixaipostprocess.c`.

Ported: **Sheik** (`ftremixaipostsheik.c`, asm `Sheik/AI/Attacks.asm` 55-149):
- `teleport_down`: for the USP actions 0xE4, 0xE5, 0xE7, 0xE8 with ground below (`coll_data.floor_line_id != -1`), set the NULL
  command (`FTREMIX_AI_LV10_ROUTINE_NULL`), `stick_range.y = -80`, `stick_range.x = 0`.
- `dsp_control`: for DSP_BEGIN (0xF0) with a target (`computer.target_gobj`), press B (`button_inputs |= B_BUTTON`) when the
  distance to the target is at most 600.0F (sqrt of the squared x and y differences, as the asm). Otherwise the NULL command.
  No target: no change.

Not ported (REMIX-TODO(AI) in `ftremixaipostprocess.c`): Falco and Fox/JFox, Captain Falcon/JFalcon, Link family, DK/JDK,
Bowser, GBowser, Wario, Marth, Dedede, Wolf, Ganondorf, Mewtwo, Roy, Marina, DragonKing, Jigglypuff and Kirby families,
Pikachu family (`pikashared.asm:144`), Yoshi/JYoshi. Each is a separate asm routine (100 to 160 lines in most cases); not done
in this slice.

## Task 2: auto_jab_grab_

Asm hook `0xC48BC` (`0x80149E7C`) in `ftCommonCatchCheckInterruptAttack11` (vanilla 0x80149E24..0x80149EC0,
`ftcommoncatch1.c:181`). Level 10 CPUs grab during the jab from frame 3 unless Single Button Mode is `A` or `B`. The asm
then jumps to `0x80149E84`, which runs the vanilla polygon check before the grab.

Missing symbol: the vanilla polygon check at `0x80149E84`. The decomp C of `ftCommonCatchCheckInterruptAttack11` has no
polygon check, and the Remix asm has no copy of it. The Single Button Mode toggle (`Toggles.entry_single_button_mode`) is also
not ported (see `ftremixaiteching.c:34`). Left as REMIX-TODO(AI) in the code.

## Task 3: PUFF_DSP

The routine (`AI.asm` 3180) is only installed by the `Puff_DSP_` scope (`AI.asm` 6452-6465), which is entirely commented out.
The DSP edit of the Puff attack table below it (`edit_attack_behavior` at `CPU_ATTACKS_ORIGIN 0x1026A4`) is commented out too.
No other file names `PREVENT_ATTACK.ROUTINE.PUFF_DSP`. So in Remix the routine is never installed, and nothing is ported.
DEVIATION(unreachable routine): not ported because the asm installs it only in commented code. The lead can confirm by
grep in `.toolchain/pkgs/smashremix-src/src`.

## Task 4: weight_attack_options below level 10

Asm `AI.asm` 5496-5499: `addiu t0, t0, -10; bnez t0, _end`. Any level other than 10 goes to `_end` (the vanilla logic). The
port gate `ftRemixAiWeightAttackOptionsApplies` is `fp->level == FTREMIX_AI_WEIGHT_LEVEL` (10). Same behaviour, nothing changed.

## Task 5: P11 and P06

- **Bowser and GBowser** (asm `AI.asm` 923-1025, `_bowser`): if the action is 0xDE (Bowser USP ground), the stick X is set to zero
  (byte 0x1C8 = `input.cp.stick_range.x`), and the result is FALSE (`v0 = 0`). Ported: `ftRemixAiSpecialsBowserUspStickReset`
  (`ftremixaispecials.c`), called from the `REMIX_ID_BOWSER` and `REMIX_ID_GBOWSER` cases of `ftComputerCheckTryChargeSpecialN`.
- **Sheik and Peppy** (`_check_needle`, `_check_revolver`): REMIX-TODO(AI). The asm jumps to the vanilla exits
  `0x80136BF8` (charge states) and `0x80136B3C` (the Samus B press for needle ammo below 6). Neither address maps to a line of
  `ftComputerCheckTryChargeSpecialN` in the decomp (`vanilla_funcs.txt` only has the function start). Also not mapped: the
  Peppy action ids (`Peppy.Action.NSPG_BEGIN`, ...) and the needle ammo field (0xAE0, the `passive_vars` name for Sheik).
- **Banjo** (P06, asm `AI.asm` 326-357, `_banjo_flight`): in Banjo Flight (0xEF) at `status_total_tics == 21` (the asm comment
  says 22, the compare says 21), OR B into `input.pl.button_tap` (asm 0x1BE; `struct_offsets.txt` names it `button_tap`). If
  `floor_line_id == -1`, set the PRESS B command (0x09) and skip the custom recovery. Ported: `ftRemixAiRecoveryBanjoFlap` in
  `ftremixairecovery.c`, called first in `ftRemixAiRecoverySpecialCase`.
  Risk for the lead: the asm writes the human tap field, not the CPU field `input.cp.button_inputs` (0x1C6). The port copies the
  write as the asm has it. Check in game that Banjo presses B in Flight frame 21 (if not, the write is invisible for CPUs).

## Deviations from the asm

- PUFF_DSP not ported (task 3, unreachable in Remix).
- Banjo frame comment says 22, code compares 21; the port follows the code (21).
- Banjo writes the human tap field (0x1BE). Kept as in the asm; see the risk above.

## Files

Created:
- `decomp/src/ft/ftremix/ftremixaipostsheik.c`, `.h`
- `decomp/src/ft/ftremix/ftremixaipostsheikrules.c`, `.h`

Edited:
- `decomp/src/ft/ftremix/ftremixaipostprocess.c`: `ftRemixAiPostProcessFighter` switch, include of `remix/remix_roster_data.h`.
- `decomp/src/ft/ftremix/ftremixaipostprocess.h`: comment only.
- `decomp/src/ft/ftremix/ftremixaispecials.c`, `.h`: `ftRemixAiSpecialsBowserUspStickReset`.
- `decomp/src/ft/ftremix/ftremixaispecialsrules.c`, `.h`: `ftRemixAiSpecialsIsBowserUspGround`.
- `decomp/src/ft/ftremix/ftremixairecovery.c`: `ftRemixAiRecoveryBanjoFlap`, PRESS B define, Banjo case in `ftRemixAiRecoverySpecialCase`.
- `decomp/src/ft/ftremix/ftremixairecoveryrules.c`, `.h`: `ftRemixAiRecoveryIsBanjoFlapFrame`, `ftRemixAiRecoveryBanjoNeedsPressB`.
- `decomp/src/ft/ftcomputer.c`: Bowser and GBowser cases in `ftComputerCheckTryChargeSpecialN`; the REMIX-TODO comment there
  now names only Sheik and Peppy and the two vanilla exits.

No new `REMIX_SYM_*` symbols are needed (no Remix RAM reads added).

## Pure functions for unit tests (lead adds the tests, `tests/remix`)

- `ftRemixAiSheikIsTeleportAction`: 0xE4, 0xE5, 0xE7, 0xE8 TRUE; 0xE6, 0xF0 FALSE.
- `ftRemixAiSheikTeleportHasGroundBelow`: -1 FALSE; 0 and 5 TRUE.
- `ftRemixAiSheikIsDspAction`: 0xF0 TRUE; 0xE4 FALSE.
- `ftRemixAiSheikDspIsInRange`: 0.0F TRUE; 600.0F TRUE; 600.001F FALSE.
- `ftRemixAiSpecialsIsBowserUspGround`: 0xDE TRUE; 0xDD and 0xDF FALSE.
- `ftRemixAiRecoveryIsBanjoFlapFrame`: (0xEF, 21) TRUE; (0xEF, 22) FALSE; (0xEE, 21) FALSE.
- `ftRemixAiRecoveryBanjoNeedsPressB`: -1 TRUE; 0 FALSE.

## In game (lead)

- Sheik (any CPU level, the table is not level gated in the asm): USP in the air or on the ground with ground below: the stick
  goes down and the USP drops to the ground. DSP at a CPU target: B is pressed within 600 units, the NULL command otherwise.
- Bowser or GBowser: grounded USP with a CPU; the stick X must be zero during the move.
- Banjo (any CPU level): in Banjo Flight with no ground below (`floor_line_id == -1`), B is pressed on frame 21.
