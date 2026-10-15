# F7 notes: JKirby (Remix id 0x30, fkind 48) copy and swallow routines

Source: `.toolchain/pkgs/smashremix-src/src/JKirby/JKirby.asm` (401 lines), `JKirby/Moveset/*.bin` (data only), and the Kirby-side hooks in `Kirby/Kirby.asm`.

## Enumeration

### 1. JKirby.asm itself

JKirby.asm defines no MIPS routine of its own. Its code references are:

- 150 unique `Scope.label` Remix routine tokens in the Add Actions block (lines 213-367) and the `edit_action` at line 371 (`DSamusNSP.aerial_b_button_buffer_kirby`).
- Every one has a `REMIX_SYM_<Scope>__<label>` in `port/remix/remix_symbols.h` and a `{ REMIX_SYM_..., ... }` entry in some `dFTRemix*Procs[]` table under `decomp/src/ft/`. Result: 0 unported routines among these references. This is a text match on table entries, not a built-ROM check.
- Vanilla addresses (`0x800D8BB4`, `0x800D91EC`, `0x8015D464`, ...) map to decomp functions and need no port.
- `Kirby.*` labels in the Add Actions and `Character.add_new_action_params` lines (for example `Kirby.GND_NSP_GROUND`, `Kirby.ROY_NSP_1`) are moveset `insert` labels defined in `Kirby/Kirby.asm`. They are data, not routines, and need no port.
- `File.KIRBY_*` names are animation or file ids, not routines.
- Moveset `.bin` files are inserted data run by the moveset interpreter. They are not C routines.

### 2. Code routines in Kirby/Kirby.asm that JKirby depends on

| Label (Kirby.asm) | Lines | Role | Status |
|---|---|---|---|
| `clear_marth_flag_` | 1120-1135 | grounded_script for JKIRBY (`JKirby.asm:384-386`) | PORTED |
| `clone_power_action_swap_` | 973-1108 | copy-power action swap (Kirby-wide hook, not referenced from JKirby.asm) | NOT PORTED |
| `kirby_magic_hat` | 651-858 | hat selection / charge (Kirby-wide, with sub-labels) | NOT PORTED |
| `clear_magic_hat_charge` | 858-872 | hat charge reset | NOT PORTED |
| `restore_magic_hat_` | 893-972 | hat restore on copy | NOT PORTED |

Sub-labels such as `_check_hat`, `_loop`, `_kirby`, `_gnd_actions` are local branch targets inside those five routines, not separate routines.

Count: the brief says 14 unported routines. This pass finds 0 among JKirby.asm's own references and 5 code routines in Kirby.asm (1 ported). The 14 figure could not be reproduced from the sources, so it may cover data or other scopes.

## Ported in this pass

- `decomp/src/ft/ftremix/jkirby/ftremixjkirby.c` (new): `ftRemixJKirbyGroundedScript(FTStruct *fp)`. When `passive_vars.kirby.copy_id` is `REMIX_ID_MARTH` or `REMIX_ID_ROY`, it writes 0 to `passive_vars.kirby.copysamus_charge_level` (N64 `0x0AE0`). Matches `Kirby.asm:1120-1135`.
- `decomp/src/ft/ftremix/ftremixhooks.h`: appended the declaration under a scope comment (append only).
- `decomp/src/ft/ftremix/ftremixchartables.c`: `ftRemixRunGroundedScript` dispatches JKIRBY to the new hook before the per-fkind table. Before this, JKirby had no grounded_script case.

Vanilla patches: none. The hook runs through the existing REGION_REMIX call in `decomp/src/mp/mpcommon.c` (`ftRemixRunGroundedScript`).

Syntax check: `syntax-check OK` for `ftremixjkirby.c` and `ftremixchartables.c`. The header is covered by the `.c` compiles.

## Not ported, with reasons

- `clone_power_action_swap_`, `kirby_magic_hat`, `clear_magic_hat_charge`, `restore_magic_hat_`: Kirby-wide copy and hat hooks that run for every Kirby copy, not only JKirby. Each needs its vanilla call site under `REGION_REMIX` and the Kirby action tables. They were out of this pass's budget and are better done in a Kirby-scope worker.

## Uncertainties

1. `0x0AE0` is named `passive_vars.kirby.copysamus_charge_level` in `struct_offsets.txt`, and the asm comment calls it the pseudo-jump flag. The port writes the same word. Whether it is only a Marth or Roy landing flag, or shared with the Samus ammo, is not confirmed in game.
2. The AI-only `cpu_*` routines were not searched for in JKirby.asm.

## Memory and rules

- ai-memory global rules were loaded (`_rules/no-agent-commit-attribution.md`, `_rules/no-live-provider-tests.md`, `_rules/ai-memory-operational-recall.md`).
- Project rules could not be loaded: no `.ai-memory.toml` in the repo, and project `JRickey/BattleShip` returned "not found" from `memory_briefing`.
