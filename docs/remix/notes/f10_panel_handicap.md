# F10 panel: HANDICAP (CPU Handicap, css/Handicap.asm)

Panel value 0..40 per port (`REMIX_PANEL_HANDICAP`). Asm: `src/Handicap.asm` use_handicap_ (patch 0x800D816C, ROM 0x5396C,
line 21), override_table (line 13); labels `css/Handicap.asm` lines 18 (string_table) to 103.

## Behaviour ported

- The asm writes the panel value into the handicap byte of the fighter description (`sb t9, 0x0012(v0)`, the `handicap` field
  of FTDesc), only when the player type is CPU (`lbu t5, 0x0023(v0)`, value 1) and the value is not 0. The port does the same
  on the fighter struct: `fp->handicap` is set from the panel value when `fp->pkind == nFTPlayerKindCom` and the value is not 0.
- Value 0 (Default): vanilla `desc->handicap`, nothing changes.
- The asm does not touch `desc->level` (CPU level), so the port does not touch `fp->level` either.
- Human ports keep their vanilla handicap.

## Value map (css/Handicap.asm string_table)

| Value | Label |
|---|---|
| 0 | Default |
| 1..9 | 1..9 |
| 10..14 | Yoshi V. Easy, Easy, Normal, Hard, V. Hard |
| 15..19 | Kirby V. Easy .. V. Hard |
| 20..24 | Polygon V. Easy .. V. Hard |
| 25..29 | G. DK V. Easy .. V. Hard |
| 30..34 | M. Mario V. Easy .. V. Hard |
| 35..39 | MH V. Easy .. V. Hard |
| 40 | Samus |

The values 10..39 are per-character difficulty levels that the Remix AI reads, not vanilla CPU levels. The asm names the
character only in the label, so the value alone does not select a character in the port.

## Edited

- `decomp/src/ft/ftremix/ftremixhandicap.c` (new): `ftRemixHandicapGetFighterValue(FTStruct *fp, u8 vanilla_handicap)`,
  with a `REMIX-TODO(AI)` comment.
- `decomp/src/ft/ftremix/ftremixhooks.h`: declaration appended at the end.
- `decomp/src/ft/ftmanager.c`, `ftManagerInitFighter`, line near 1052: `#ifdef REGION_REMIX` branch around
  `fp->handicap = ...`. The vanilla line is unchanged in the `#else` branch.
- `port/remix/remix_player_panel.cpp`: title "CPU Handicap" (was "CPU Handicap (0 = default)"), new `kHandicapNames`
  list of 41 labels, and the HANDICAP entry uses it. Range 0..40 is unchanged.

Syntax check OK for `ftremixhandicap.c` and `ftmanager.c`. `remix_player_panel.cpp` is C++ and was not syntax-checked here.

## Not done (REMIX-TODO(AI))

- The Remix AI reads the handicap byte for its per-character handicap (`src/AI.asm` line 6896: `beqz at, _handicap`, the
  index `handicap - 0x0A` into `handicap.table`, and the indicator setup until line 7002). The AI is not ported (M7), so the
  value has no AI effect in the port.
- The vanilla knockback code also reads `fp->handicap` as an index into `dFTCommonDataHandicapTable` (`ftmain.c` 3053,
  `ftParamGetCommonKnockback`). A CPU with a panel value of 10..40 now uses that table with the Remix value. This matches the
  asm (it writes the same byte), but it was not checked in game, and it is the one place where the port's behaviour can change
  without the AI being present.
- Not built or run (lead builds and tests).
