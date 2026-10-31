# F14: StockMode Last (1) in gmremixstockmode.c

Source: `.toolchain/pkgs/smashremix-src/src/StockMode.asm` set_initial_stock_count_ (lines 46-109), `css/StockMode.asm` onchange_handler (lines 13-100), `GameEnd.asm` 136-141 (salty runback).

## Asm semantics ported

- Last is read at match start only. The per-port `previous_stock_count_table` is stored once, when it is unset (-1): the match count becomes the stored count.
- Later matches use the stored count, lowered to the match count (`sltu`), except Tug of War where the stored count is used as is.
- `initial_previous_stock_count_table` is written on every Last match start (the value before the match). Salty runback copies it back.
- The CSS Stock option reset (`_check_reset`) sets the stored count back to -1.
- The CSS counter display (`CharacterSelect.asm` 5627) only reads the stored count; it is not a separate source. The brief said the CSS stock counter feeds Last; in the asm the source is the stored table above.

## Port

- `gmRemixStockModeCount` dispatches Last to the file-local `gmRemixStockModeLastCount` (per-port `sStockModeLastCount`, `sStockModeInitialLastCount`).
- `void gmRemixStockModeResetLast(void)` and `void gmRemixStockModeSaltyRunbackLast(void)` are non-static and declared at the top of the .c file. They are not declared in `gmremixstockmode.h` (not in scope to edit).

## Hooks the lead must wire (not edited, out of scope)

1. CSS reset: call `gmRemixStockModeResetLast()` where the port handles the CSS reset button (the equivalent of `CharacterSelectDebugMenu.handle_reset_button_press_._reset_return`). Shared file, owner: `mn/mnplayers/mnplayersvs.c` or the CSS reset handler. Prototype: `void gmRemixStockModeResetLast(void);`.
2. Salty runback: call `gmRemixStockModeSaltyRunbackLast()` where the salty runback is accepted (GameEnd.asm `_success`). Prototype: `void gmRemixStockModeSaltyRunbackLast(void);`.
3. Add both prototypes to `decomp/src/gm/gmremixstockmode.h` so callers outside this file compile without implicit declarations.

Without hook 1 the stored count is never cleared across CSS visits. Without hook 2 a salty runback after a Last match keeps the stored count.

## Uncertainties

- Tug of War Last: asm (StockMode.asm 91-95) returns the stored count without clamping. The port does the same via `gmRemixModeIs(MNREMIX_VSMODE_TUG_OF_WAR)`. Not tested in game.
- The asm onchange handler also runs on value change, not only on reset; the port only models the reset path.
- Not built or run (per brief). Syntax check OK.
