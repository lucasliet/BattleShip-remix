# F8 Move Staling (StaleMoves.asm)

Source: `.toolchain/pkgs/smashremix-src/src/StaleMoves.asm`. Toggle: `entry_move_staling` in `Toggles.asm` (line 2448).

## Toggle

- Title: `Move Staling` (test with `SSB64_REMIX_TOGGLES="Move Staling=<value>"`, case-insensitive).
- Values: 0 Default, 1 Disabled, 2 Lenient, 3 Strict, 4 Wait For It, 5 Reverse, 6 Cheap Shot.
- Read with `gmRemixToggle(move_staling)` (no `single_player_guard`; the asm reads `entry + 4` directly).

## Patches

| Asm address | Vanilla function | Status | What it does in the port |
|---|---|---|---|
| 0x800EA4DC (`OS.patch_start 0x65CDC`, `j check_toggle`) | `ftParamGetStale`, `decomp/src/ft/ftparam.c` | Applied | Hook at the top of the `attack_id != nFTMotionAttackIDNone` block: `ftRemixStaleMovesIsCustom()` then `ftRemixStaleMovesGetMultiplier()` for modes 1, 2, 5, 6. Modes 0, 3, 4 fall through to the vanilla loop. |
| 0x800EA4F8 (`lui at, 0x8013` to `nop`) and 0x800EA504 (`lwc1 f0, 0xB820(at)` to `lwc1 f0, 0(at)`) | `ftParamGetStale` | Applied as table choice | The vanilla loop returns `ftRemixStaleMovesTable(dFTParamStaleTable)[i]`: strict table for mode 3, wait-for-it table for mode 4, vanilla table otherwise. |
| `_end` / `preserved_stale_value` | same | Applied | Per-port static `sFTRemixStalePreserved[GMCOMMON_PLAYERS_MAX]`, initial -1, written with the run count after each custom-mode call. Read back when the newest queue entry has the same motion_count. |
| `_original_strict`, `_original_wfi` | same | Applied | Vanilla loop with the other table (see above). The preserved value is not written on this path, as in the asm. |

No `REMIX-TODO` needed: no subsystem without a port is involved.

## Files

- `decomp/src/ft/ftremix/ftremixstalemoves.c` (new, owned): the four tables, `sFTRemixStalePreserved`, `ftRemixStaleMovesIsCustom`, `ftRemixStaleMovesTable`, `ftRemixStaleMovesGetMultiplier` and the queue walk (`ftRemixStaleMovesCountRun`, `ftRemixStaleMovesPrevIndex`, `ftRemixStaleMovesFreshMultiplier`).
- `decomp/src/ft/ftremix/ftremixhooks.h`: declarations appended at the end (block "Move Staling").
- `decomp/src/ft/ftparam.c`: two `#ifdef REGION_REMIX` hunks inside `ftParamGetStale`. Vanilla code under `#else` is unchanged.

## Uncertainties

- Strict table first entry: the asm word is `0x3C000000`, which is 2^-7 = 0.0078125, not 0.01 as its comment says. The port keeps the asm value `0.0078125F`. A strict repeat therefore deals about 1 damage, the same as the asm.
- Queue walk: the asm loop reads at most 4 entries (table size) out of 5 `stale_info` slots, and wraps to slot 4 (`ARRAY_COUNT - 1`). The port does the same.
- Walk semantics differ from the vanilla loop on purpose: the asm counts every matching entry in a row and does not return on the first motion_count difference. The port follows the asm.
- `sFTRemixStalePreserved` is never reset during a match, as in the asm. It starts at -1 at process start (asm: `db -1,-1,-1,-1` at ROM load).
- Mode 1 (Disabled): the asm skips the walk, stores 0 and returns 1.00x. The port does the same.

## What the lead should see (on)

- Reverse (`Move Staling=5`): the first consecutive hit of a move is 0.75x, then 0.82x, 0.89x, 0.96x, 1.00x when the same move is repeated. A different move in between resets it.
- Cheap Shot (`Move Staling=6`): the first hit is 1.00x, then 1.25x, 1.36x, 1.50x, 1.75x on repeats.
- Lenient (`Move Staling=2`): the first repeat is 0.75x, and a single different move restores 1.00x.
- Strict (`Move Staling=3`): the repeat of the same move deals about 1 damage. Other hits use the vanilla table with 0.82x, 0.89x, 0.96x.
- Wait For It (`Move Staling=4`): levels 0.75x, 0.82x, 0.89x, then 1.75x on the fourth repeat.
- Disabled (`Move Staling=1`): every hit 1.00x.
- Default (`Move Staling=0`): vanilla behaviour, no change.

Verification: `scripts/remix/syntax-check.sh` on `decomp/src/ft/ftremix/ftremixstalemoves.c` and `decomp/src/ft/ftparam.c` returns `syntax-check OK`. The two warnings in `ftparam.c` (lines 1160 and 1814) were already there. Build and game runs not done (lead).
