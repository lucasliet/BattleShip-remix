# F14 worker brief: close the REMIX-TODO lines of one scope (ROADMAP T2.11)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Always use absolute paths and `builtin cd`. Remix source (read only):
`.toolchain/pkgs/smashremix-src`. Offset maps: `build/remixasm/refs/struct_offsets.txt` (N64 offsets of `ITStruct`, `WPStruct`, `FTStruct`,
`DObj`, ...; it is 4 bytes low from about 0x248 on `ITStruct` and 0x25C on `WPStruct`), `build/remixasm/refs/vanilla_funcs.txt`
(RAM address to decomp function). Read `docs/remix/f5_worker_brief.md` (translation and LP64 rules) first.

Goal: every `REMIX-TODO` line of your files ends in one of three states. The grep must not find a `REMIX-TODO` there afterwards.

1. **Implemented.** The asm effect is visible in the game or changes state another system reads (a blast-zone callback, a hitbox flag
   that decides a hit, a sound, a GFX, a hidden model, a camera or Render effect, a Size scale). Port it with the real asm values.
   Read the asm of the patch; the TODO text is a summary and can be wrong. If the decomp has a named field, use it.
2. **Mapped.** The asm writes a raw N64 offset (0x158 bit, 0x2D3, 0x33E, 0x1D0, 0x1D4, 0x354, 0x35C, 0x2CE, 0x2CF, 0x224..0x243, 0x398).
   Find the offset in `struct_offsets.txt` (add 4 from 0x248 on for `ITStruct`). Examples: 0x398 is the item `proc_dead` blast-zone
   callback; 0x158 is the attack_coll bitfield word (IDO packs bits from the most significant bit: byte mask 0x10 of the first byte is
   bit index 3, `can_rehit_shield`); 0x224 is `attack_coll.attack_records`. If the offset is a named field, write that field.
   If it is a Remix scratch slot that the Remix code itself keeps (for instance an FGM handle at 0x1D0/0x1D4) and the port already keeps
   that state in a file-local struct or `item_vars`, the TODO is resolved by saying so.
3. **Documented deviation.** Only when the asm effect is provably invisible (a store whose only reader is vanilla code that never runs for this
   item, a duplicate of state the port keeps elsewhere) or needs a subsystem that is a separate ROADMAP task (named in the TODO list
   the lead gave you). Rewrite the comment as `/* DEVIATION(<short reason>): ... asm file:line ... */` and add one row to the
   notes file. Never use this for something you did not read in the asm. Never use it for a visible effect just because it is hard.

Do not touch lines whose behaviour you cannot verify against the asm; report them instead.

## Rules

- Do not run cmake, the build, the game or git. Syntax-check every file you touch with `scripts/remix/syntax-check.sh <file>` until
  `syntax-check OK`, after your last edit.
- Edit only the files of your scope (plus new files you own and an append-only block in a hooks header). Other workers edit other
  directories in parallel: re-`Read` a file right before each `Edit`, small unique `old_string`s, never `Write` over an existing file.
- Native Read/Edit/Write for files. Bash only for grep/find and the syntax check.
- Code in English. No `//` comments restating code. Comments only for intent, provenance (asm file and line) or a quirk. TSDoc on new functions.
  Keep existing comments unless provably stale (a resolved TODO comment is replaced, not deleted silently).
- No ROM bytes in versioned files; Remix data tables become `static const` C data read from the Remix RAM image only through
  `remix_ram_image_read_*` and the `REMIX_SYM_*` symbols of `port/remix/remix_symbols.h`.
- Pure logic with numbers goes into a `*rules.{c,h}` file with ints and floats only so the lead can add a unit test.
- If a task needs a subsystem that does not exist in the port, stop for that line and list it in the report.

## Final report (at most 25 lines)

One line per original TODO: file:line, state (implemented, mapped, deviation, skipped) and why. Then files created or edited, the notes path
`docs/remix/notes/f14_<scope>.md`, and what the lead must wire or test in game (item, stage, input).
