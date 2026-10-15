# F7 leftover worker brief: close one REMIX-TODO group of the Remix fighter port

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`.

Read in this order before touching code:

1. `docs/remix/f5_worker_brief.md` (references, translation rules, LP64 rules, `OS.copy_segment` decoding).
2. `docs/remix/f5_shared_patch_brief.md` (you apply vanilla patches yourself, guarded by `#ifdef REGION_REMIX`, with helpers in
   a new file you own, declared by appending a block to `decomp/src/ft/ftremix/ftremixhooks.h`).
3. The notes of your scope in `docs/remix/notes/` (they list what was skipped and why) and the `REMIX-TODO` comments in
   the files named by your scope (`grep -rn "REMIX-TODO" <file>`).

State of the port: 78 Remix fighters run, standard items and the VS item switch exist. This wave closes the leftovers.
Everything is built and tested by the lead, not by you.

## Rules

- Do not run cmake, the build, the game or git. Syntax-check every file you touch with
  `scripts/remix/syntax-check.sh <file>` until `syntax-check OK`, after your last edit to it.
- Other workers edit vanilla files in parallel: re-`Read` a vanilla file right before each `Edit`, use small unique
  `old_string`s, never `Write` over an existing file, never reorder existing lines of `ftremixhooks.h` (append only).
- Only touch files your scope needs, plus new files you own. Do not edit another scope's files.
- No ROM-derived bytes in versioned files. No `//` comments that restate code; a short `/* */` comment is allowed when a line exists
  because of an asm quirk (cite the asm file and line).
- A patch for a subsystem with no port yet (Size, GFX, GFXRoutine, PokemonAnnouncer, CharEnvColor, Reflect, AI) stays a
  `REMIX-TODO(<subsystem>)`; apply the logic around it. Never leave a half-applied patch that changes vanilla behavior.
- If an address does not map to a decomp function (`build/remixasm/refs/vanilla_funcs.txt`), say so in the notes and skip it.

## Final report (at most 20 lines)

Files written or edited, each patch (asm address, vanilla function, applied or skipped and why), the new `REMIX-TODO`s, the
notes path `docs/remix/notes/f7_<scope>.md`, and anything the lead must wire or test.
