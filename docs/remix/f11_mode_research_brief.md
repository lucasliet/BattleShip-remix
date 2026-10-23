# F11 research brief: one Remix game mode, design note for the port (ROADMAP M6)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. You write ONE notes file
and no code. Do not build, run cmake or git, no attribution trailers. Remix source (read only): `.toolchain/pkgs/smashremix-src/src/`.
Decomp side: `decomp/src/` (`sc/sccommon/scvsbattle.c` is the VS match scene, `sc/sc1pmode/` the 1P modes, `mn/mnvsmode/` the VS
menus, `ft/`, `it/`, `gr/`, `if/` as named). Earlier work to read first: `docs/remix/menus.md` (sections 3.2, 3.3 and the Remix
menu side), `docs/remix/notes/f10_menu_vsmodes.md` (the VS Remix Modes page stores the chosen mode in `gMNVSModeRemixMode`,
header `mn/mnvsmode/mnremixvsmode.h`), `docs/remix/notes/f10_panel_stock_practice.md`. The port keeps Remix ids as fkind; the Remix
RAM image is read by symbol (`port/remix/remix_ram_image.h`, `remix_symbols.h`).

## Output: `docs/remix/notes/f11_mode_<name>.md` with these sections

1. **What the mode is** (rules in two sentences, taken from the asm: win condition, timer, stocks, scoring) and which toggles
   or flags select it (`VsRemixMenu.vs_mode_flag`, `Global.vs.game_mode`, `Global.match_info`, single player mode flag).
2. **Flow**: menu entry -> character select -> stage select -> match -> results/return, listing every vanilla screen the mode
   changes and how (the asm hooks of that mode, each as `asm file:line | patched RAM/ROM address | what it does`, plus the
   vanilla decomp function it corresponds to, matched by screen and function name, not by the ROM address comment; say
   `no match found` when so).
3. **State and data**: the mode's globals (names, sizes, who writes them), tables, strings, and every file id or sprite offset the
   mode loads (plain constants in the asm; say which are Remix-only files).
4. **In-match logic**: the per-frame/per-event logic in the order it runs (spawn, scoring, timers, respawn rules, camera, items,
   stage changes), written so a C port can follow it without the asm.
5. **HUD and text**: every on-screen element (what it shows, whether it needs a font/text the port cannot draw: the port draws
   text only through an ImGui overlay, `port/remix/remix_overlay.h`, and vanilla sprites).
6. **Dependencies**: which other Remix modes or shared systems this code reuses or breaks (StockMode tables, TimedStock scoring,
   Stamina, Neutral Spawns, the results screen).
7. **Port plan**: an ordered list of steps, each small enough for one Haiku worker in one session (a single file or one hook
   group), each naming the files to create or edit and what the lead can see on screen to accept it. Mark steps that depend on
   another mode's step.
8. **Risks and unknowns**, honestly.

Be concrete (line numbers, names). Do not invent: if something is unclear in the asm, say so. Final report under 15 lines
(file written, size of the port plan, the biggest risk).
