# F10 worker brief: one Remix menu or screen feature (ROADMAP T5.3)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. Read
`docs/remix/f7_leftover_worker_brief.md` first (rules, final report) and `docs/remix/menus.md` (the Remix menu side, with the
asm file and line of every screen; the address-to-decomp matches marked **collision** are wrong overlay, find the real function
by name and by what the asm does). You only write code and one notes file. The lead builds and runs the game; do not build, do
not run cmake, do not commit, no attribution trailers anywhere.

## Design rules

- The Remix menu framework (`Menu.asm`, the settings screen, the CSS panel) is not redrawn: those settings live in the port
  menu (Remix section, `remix_toggles`, `remix_player_panel`). Everything else a player sees (a button on a vanilla menu,
  a page, a screen, a data table) is ported as real vanilla-menu code under `#ifdef REGION_REMIX`.
- Vanilla behaviour without `REGION_REMIX` must stay identical.
- Data comes from the Remix source (`.toolchain/pkgs/smashremix-src/src/`) or is read at run time from the Remix image
  (`port/remix/remix_ram_image.h`, symbols in `port/remix/remix_symbols.h`; add a symbol to `EXTRA_SYMBOLS` in
  `tools/gen_remix_actions.py` and tell the lead to regenerate when you need one). Sprite and texture bytes of the Remix files
  are read from the loaded file, never copied into versioned files. Finished examples of the pattern:
  `decomp/src/mn/mnmaps/mnremixemblem.c`, `mnremixstagevariant.c`, `mn/mnplayers/mnremixcss.c`, `mnremixvariants.c`.
- Put new logic in files you own (`decomp/src/mn/<screen>/mnremix*.c` or the module of the feature) and hook the vanilla
  function with small `#ifdef REGION_REMIX` blocks; declare helpers in a header you own. Port rules of the earlier briefs apply
  (LP64, no restating comments, syntax check with `scripts/remix/syntax-check.sh <file>` until `syntax-check OK`).
- If a feature needs an asset or a function the port lacks, port what works, leave a `REMIX-TODO(<feature>)` with the reason
  and say it in the report. Never leave a half-applied hook that changes vanilla behaviour or can crash.
- Write `docs/remix/notes/f10_menu_<feature>.md`: edited functions, asm lines, what differs, what is left, and what the lead must
  look at on screen (scene id to start with `SSB64_START_SCENE`). Final report at most 20 lines.
