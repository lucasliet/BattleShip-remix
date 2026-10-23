# F12 worker brief: one step of a Remix mode port (ROADMAP M6, T6.1 to T6.3)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. You port ONE step of a mode
plan. Your prompt names the note (`docs/remix/notes/f11_mode_<name>.md`) and the step (section 7 or the plan list of that note).
Read first: `docs/remix/f7_leftover_worker_brief.md` (rules, final report), `docs/remix/modes_base.md` (the common base: mode of the
match, Timed Stock, positioned HUD text), the note of your mode (sections 2 to 7 and 8) and the Remix source it cites
(`.toolchain/pkgs/smashremix-src/src/`, read only). You only write code and append to the notes file of your mode; the lead builds
and runs the game. Do not build, do not run cmake or git, no attribution trailers anywhere.

## Rules

- Vanilla behaviour without `REGION_REMIX` must stay identical. Hook the vanilla function with a small `#if defined(PORT) && defined(REGION_REMIX)`
  block; put the mode logic in files you own (`decomp/src/gm/gmremix<mode>*.c` and a header declaring them; for fighter hooks
  `decomp/src/ft/ftremix/`, items `it/itremix/`). New files need no CMake edit (globbed, the lead reconfigures).
- The mode is read with `gmRemixModeIs(MNREMIX_VSMODE_*)` / `gmRemixModeGet()` (`gm/gmremixmode.h`), Timed Stock with
  `gmRemixTimedStock*`. A rule-based mode (Stamina) keeps its own flag in its own module, set where the menu choice is made.
- Text on screen (scores, counters, labels) uses `remix_hud_text_put` (`port/remix/remix_hud_text.h`), one slot range per mode: Stamina
  0-3, Tug of War 4-7, KOTH 8-13, Tag Team 14-17, Smashketball 18-23, 12CB 24-27, 1P and Training 28-31. Sprite parts of the HUD
  that the Remix draws from vanilla sprites use the vanilla sprite code.
- Remix-only files (sprites, models) are read at run time from the loaded Remix image (`port/remix/remix_ram_image.h`, symbols in
  `port/remix/remix_symbols.h`, new ones in `EXTRA_SYMBOLS` of `tools/gen_remix_actions.py`, tell the lead to regenerate). No ROM
  bytes in versioned files.
- Check the asm against the note before copying it: the notes list suspected asm bugs (Tug of War shield break, Multiman Cruel,
  Smashketball `ai_ball_attack_range`, Tag Team `prevent_start_with_empty_slots_`, KOTH team double count, All-Star `_assign_stage`
  loop). Port the intended behaviour and say what you changed from the asm.
- LP64, no `//` comments restating code, no `<stdio.h>` before decomp headers (use `<port_log.h>` after `<ft/fighter.h>`),
  block comments of headers must not contain `/*`. Syntax check each C file with `scripts/remix/syntax-check.sh <file>` until
  `syntax-check OK`.
- A step that needs something another step has not delivered: stop, port what works, leave a `REMIX-TODO(<mode>)` with the reason
  and say it in the report. Never leave a half-applied hook that changes vanilla behaviour or can crash.

## Output

Append a section `## Step <id>: <title>` to the notes file of your mode (`docs/remix/notes/f12_<name>_port.md`, create it if
missing): files edited, hooks (asm line -> decomp function), deviations from the asm, what is left, and what the lead must see on
screen (and the env vars or input script to get there). Final report at most 15 lines.
