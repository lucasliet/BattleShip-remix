# F13 worker brief: one slice of the Remix AI port (ROADMAP M7, T7.1)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. Reply in Portuguese, code in English.
You port ONE slice of `AI.asm` (and the per-fighter `AI/Attacks.asm`) to C. Your prompt names the slice and the patches. You only write code and
append to the notes; the lead builds, runs the game and commits. Do not build, do not run cmake or git writes, no attribution trailers.

## Read first

1. The inventory notes of your slice: `docs/remix/notes/f13_ai_patches_a.md` (AI.asm up to line 2660), `f13_ai_patches_b.md` (lines 2660-5200,
   lv10), `f13_ai_patches_c.md` (lines 5158-7337), `f13_ai_attack_tables.md`, `f13_ai_other_hooks.md`. Each patch has the asm lines, the decomp
   function that contains the vanilla address, what it does, and a proposed hook shape. The notes were written by a Haiku researcher: they can
   be wrong, so read the asm lines yourself (`.toolchain/pkgs/smashremix-src/src/AI.asm`, read only) before porting a patch.
2. The vanilla AI in `decomp/src/ft/ftcomputer.c` (~8100 lines; read the functions you hook in chunks with offset/limit). Each function has
   a `// 0x8013xxxx` comment with its vanilla address.
3. An existing Remix hook in the same file for the style: `ftcomputer.c` ~6911 (Link clones pull bombs) and ~6609 (custom recovery logic).

## Architecture decisions (do not change them without telling the lead)

- Remix data comes from the loaded Remix RAM image at run time: `port/remix/remix_ram_image.h` (`remix_ram_image_read_word/half/byte`,
  `remix_ram_image_resolve`), symbols `REMIX_SYM_*` in `port/remix/remix_symbols.h` (generated; new symbols are added to `EXTRA_SYMBOLS` in
  `tools/gen_remix_actions.py`, you list them for the lead who regenerates). The image keeps host-order u32 words: word reads need no swap,
  floats are bit-cast from the word, byte streams use the big-endian path (see `remix_ram_image.h`). No ROM bytes in versioned files.
- A Remix id is also its port fkind (`remix/remix_roster_data.h`, `REMIX_ID_*`), so `this_fp->fkind` selects fighter-specific code.
- Your code goes in NEW files you own in `decomp/src/ft/ftremix/ftremixai<topic>.c` plus a header `ftremixai<topic>.h` (guard
  `#if defined(PORT) && defined(REGION_REMIX)`), declared in `ft/ftremix/ftremixhooks.h` only if the lead says so. Pure rules (numbers in,
  numbers out) go in `ftremixai<topic>rules.{c,h}` using only `int`/`float` so `tests/remix` can compile them standalone (see
  `decomp/src/gm/gmremixtugofwarrules.*`); write `tests/remix/test_ai_<topic>_rules.c` for them (entry `void remixTestAi<Topic>Rules(void)`,
  pattern in `tests/remix/test_tugofwar_rules.c`). Do NOT edit `tests/remix/CMakeLists.txt` or `test_main.c`: the lead registers them.
- Hooks into `ftcomputer.c` are small `#if defined(PORT) && defined(REGION_REMIX)` blocks, one Edit each: re-read the exact region right before
  the Edit (other workers edit other functions of the same file at the same time), keep the vanilla path byte-identical when the block is
  compiled out, and re-read after the Edit to check it applied.
- Level gating: the CPU level is `fp->level` (1 to 9 in vanilla, 10 is a Remix extra); most Remix hooks only act at some level or toggle. The
  toggle "Improved AI" is read with `gmRemixToggle(...)` (`gm/gmremixtoggle.h`, takes the label such as `improved_ai`, see the notes).
- Interfaces between slices (implement your side exactly):
  - Attack tables (slice A1) call `ftRemixAiIsCustomInputSupported(s32 input_kind)` (slice A2, `ftremixaicommands.h`) to decide whether an
    entry with a custom input id (>= 0x31) is kept; until A2 exists it returns FALSE.
  - Input scripts (slice A2): `u8* ftRemixAiGetInputScript(s32 input_kind)` returns the script of a custom id or NULL for a vanilla id; the
    three `ftComputerSetCommand*` functions in `ftcomputer.c` use it.
  - Attack list (slice A1): `FTComputerAttack* ftRemixAiGetAttackList(s32 fkind, s32 level)`, NULL when the fighter is not a Remix fighter
    (the vanilla path stays) and for level 10 the lv10 table.
  - Prevent routine (slice A3): `sb32 ftRemixAiPreventAttack(FTStruct *fp, s32 input_kind)` TRUE vetoes the attack.

## Rules

- Vanilla behaviour without `REGION_REMIX` must stay identical. No fallback to the parent fighter's behaviour as a hack: if a piece is
  missing say so and leave `REMIX-TODO(AI)` with the reason; never leave a half-applied hook that changes vanilla behaviour or can crash.
- Suspected asm bugs are listed in the notes (for example inverted `bc1fl`, `srl 1` after `get_random_int_(2)`): decide from the asm text,
  port the intended behaviour only when the evidence is clear, and report every deviation.
- LP64: pointers are 8 bytes; Remix code addresses are never called, only their behaviour is ported.
- Block comments only where they say intent, rationale or asm provenance (`Stamina.asm`-style `AI.asm <scope>` references); no `//` comments,
  no comments that restate code; TSDoc-style block on each new function; early returns, short functions, no generic names.
- Syntax check each C file with `scripts/remix/syntax-check.sh <file>` until it reports `syntax-check OK` (the lead does the real build).

## Output

Append a section `## Slice <id>` to `docs/remix/notes/f13_ai_port.md` (create it if missing; append, do not rewrite): files created and edited,
hooks (asm patch -> decomp function and line), deviations from the asm, `EXTRA_SYMBOLS` the lead must add, what is left (`REMIX-TODO(AI)`),
and how the lead can see it in a match (fighters, level, input script or env). Final report at most 15 lines.
