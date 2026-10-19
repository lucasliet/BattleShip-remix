# F8 toggle worker brief: port one Remix gameplay setting (ROADMAP T4.2)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. Remix source:
`.toolchain/pkgs/smashremix-src/src` (read-only).

Read in this order before touching code:

1. `docs/remix/f7_leftover_worker_brief.md` (rules, final report) and the two briefs it names (`f5_worker_brief.md`,
   `f5_shared_patch_brief.md`): translation rules, LP64 rules, how you apply vanilla patches yourself with
   `#ifdef REGION_REMIX`, helpers in a new file you own declared by appending a block to
   `decomp/src/ft/ftremix/ftremixhooks.h` (or the header of the module you touch).
2. Your scope file(s) named in the task, and `Toggles.asm` for the toggle entries it reads (`grep -n "entry_<name>" Toggles.asm`:
   range, string table, defaults per profile CE/TE/NE/JP).

## The setting API (already in the tree)

```c
#include "gm/gmremixtoggle.h"
gmRemixToggle(hitstun)      /* value of Toggles.entry_hitstun (label without the entry_ prefix) */
gmRemixToggleVS(hitstun)    /* same, but 0 in Bonus and 1P Game (Toggles.single_player_guard) */
```

Pick `gmRemixToggleVS` when the asm uses `Toggles.single_player_guard` and `gmRemixToggle` when it uses `Toggles.guard`
or reads `Toggles.entry_x + 4` directly. The symbol is `REMIX_SYM_Toggles__entry_<name>` in `port/remix/remix_symbols.h`
(if the name is missing, tell the lead; do not edit that generated file). A toggle that is off must leave the vanilla
behavior exactly as it is. The lead tests each toggle by turning it on for a run with
`SSB64_REMIX_TOGGLES="Hitstun=1"` (the title of the entry in `Toggles.asm`, case-insensitive; values are the integer the
entry stores), so state in your notes the title, the values and what to look for in a short match.

## Your deliverable

- The patches of your scope file(s) that exist because of the setting (the setting's own routines and the hooks into
  vanilla functions), ported to C under `#ifdef REGION_REMIX`, reading the setting through the API above.
- Patches that need a subsystem that has no port (AI, PokemonAnnouncer) stay `REMIX-TODO(<subsystem>)`; everything else
  is ported. `Size`, `GFX`, `GFXRoutine`, `CharEnvColor`, `Reflect`, `Kirby` already have ports: use their helpers
  (`grep -rn "<name>" decomp/src/ft/ftremix/*.h`).
- Notes in `docs/remix/notes/f8_<scope>.md`: each patch (asm address, vanilla function, applied or skipped and why),
  the toggle title and values, what the lead should see with it on.
