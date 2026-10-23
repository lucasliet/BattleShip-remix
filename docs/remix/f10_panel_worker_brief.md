# F10 worker brief: one Remix CSS panel option in the match (ROADMAP T5.2)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. Read
`docs/remix/f7_leftover_worker_brief.md` first (rules, final report). You only write code and one notes file. The lead builds
and runs the game; do not build, do not run cmake, do not commit, no attribution trailers anywhere.

## What exists

The Remix CSS panel (20 options per port, drawn by Remix inside the CSS) is not redrawn. The port keeps the values in a
registry edited in the port menu (Remix > Player Panel P1..P4). Reference of every option (range, default, where the asm reads
it): `docs/remix/notes/f10_css_port_panel.md`. Remix source (read only): `.toolchain/pkgs/smashremix-src/src/`, option menu in
`css/<Option>.asm`, match hook in `src/<Option>.asm` (paths in the notes).

Read a value in the match with (include `<gm/gmremixpanel.h>`):

    gmRemixPanel(SHIELD, fp->player)

`name` is the `RemixPanelOption` without `REMIX_PANEL_` (SHIELD, PLAYER_TAG, VISIBILITY, SKELETON, MODEL_DISPLAY, INPUT_DISPLAY,
SIZE, STOCK_MODE, KNOCKBACK, INPUT_DELAY, HANDICAP, START_WITH, TAUNT_ITEM, TAUNT_BUTTON, KIRBY_HAT, DPAD_FUNCTIONS,
DPAD_CONTROL, DAMAGE, POISON_DAMAGE, PRACTICE_1P). Values are exactly the asm value index (same order as the strings in the
notes and in `port/remix/remix_player_panel.cpp`). The port is 0-based (`fp->player` 0..3). In the Bonus and 1P Game modes the
macro already returns 0 for non-human ports.

Finished examples to copy: `ft/ftremix/ftremixknockback.c` (read in a hook), `ft/ftremix/ftremixpanel.c` plus the call in
`ftManagerInitFighter` (value at creation), `ft/ftremix/ftremixsize.c`.

## Rules

- Hook the matching vanilla function with a `#ifdef REGION_REMIX` block (vanilla unchanged without it); put logic in a file
  under `decomp/src/ft/ftremix/` (or `it/itremix/`, `gm/`) that you own, and declare its functions by appending a block to
  `decomp/src/ft/ftremix/ftremixhooks.h`. Find the vanilla function of an asm address with
  `build/remixasm/refs/vanilla_funcs.txt`; if there is none, say so in the notes and skip.
- Default value 0 must keep the vanilla behaviour byte for byte.
- Port rules of the earlier briefs apply (LP64, no ROM-derived bytes, no restating comments, syntax check with
  `scripts/remix/syntax-check.sh <file>` until `syntax-check OK`).
- Delete the `REMIX-TODO` comment of your option when you close it. Leave the ones you cannot close and say why.
- Write `docs/remix/notes/f10_panel_<option>.md`: edited functions, asm addresses, what is left. Final report at most 20 lines.
