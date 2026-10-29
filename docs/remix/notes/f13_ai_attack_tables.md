# F13 research: Remix CPU attack tables (ROADMAP T7.1)

Status: read-only research. No source, build or ROM was changed. Source root below is
`.toolchain/pkgs/smashremix-src/src` (abbreviated `SRC`). Port paths are relative to the repo root.

## 1. Entry layout: Remix `Attacks.asm` vs vanilla `FTComputerAttack`

Remix entry (`AI.asm`, macro `add_attack_behaviour` / `add_custom_attack_behaviour`, ~line 2790-2815). Size is 0x1C (`copy_attack_behaviour` copies 0x1C, `SRC/AI.asm` ~2790):

| Offset | Type | Field |
|---|---|---|
| 0x00 | u32 | input kind (ATTACK_TABLE.*.INPUT, or `AI.ROUTINE.*` custom id) |
| 0x04 | u32 | hitbox start frame |
| 0x08 | u32 | hitbox end frame (unused for `add_attack_behaviour`, written as 0) |
| 0x0C | f32 | detect min X |
| 0x10 | f32 | detect max X |
| 0x14 | f32 | detect min Y |
| 0x18 | f32 | detect max Y |

Vanilla `struct FTComputerAttack` (`decomp/src/ft/fttypes.h:1190`): `s32 input_kind; s32 hit_start_frame; s32 hit_end_frame; f32 detect_near_x; f32 detect_far_x; f32 detect_near_y; f32 detect_far_y;`.

Verdict: same shape, same 28 bytes, same field order, no pointers. No LP64 pointer-size issue.

Group terminators: Remix `END_ATTACKS()` = `dw 0xFFFFFFFF, 0,0,0,0,0,0` (`SRC/AI.asm` ~2858). Each file has two groups: grounded, then `END_ATTACKS`, aerial, then `END_ATTACKS` (e.g. `SRC/Banjo/AI/Attacks.asm:25`, `:38`). Vanilla arrays use the same convention (`input_kind == -1`, `decomp/src/ft/ftcomputer.c:4000-4015`): when not on the ground, the picker skips the first group; the grounded group ends at its own `-1`. A single array with two `-1` sentinels therefore works unchanged.

Input kind numbering: `ATTACK_TABLE.JAB.INPUT = 0x13` (`SRC/AI.asm` ~2678). In `decomp/src/ft/ftdef.h` the `nFTComputerInput*` enum starts at line 1177 (`nFTComputerInputStickN` = 0), and `nFTComputerInputStickNButtonA` is at line 1196 = index 19 = 0x13. So vanilla input ids 0x00-0x30 match Remix. Custom ids start at `AI.NUM_VANILLA_CPU_INPUT_ROUTINES = 0x31` (`SRC/AI.asm:1569`), added by `add_cpu_input_routine` (`SRC/AI.asm:1572-1580`). Those custom ids are Remix input scripts (e.g. `BANJO_BACKWARDS_NSP`), not present in the vanilla decomp. `ENDG` / `ENDA` use input `0xFFFFFFFF` (the sentinel).

## 2. Table selection and RAM placement

- Per-character pointer table: `Character.ai_behaviour.table`, `move_table(ai_behaviour, 0x102B04, 0x4)` (`SRC/Character.asm:2237`). Index = `Character.id`, 4 bytes per entry, `NUM_CHARACTERS = 97` (`SRC/Character.asm:27` = 27 + 2 + 68).
- Pointer table RAM address: `0x8049A320` (`build/remixasm/remix.sym`, label `Character.ai_behaviour.table`).
- Vanilla entries 0-11 point at the vanilla C arrays (0x80186684 ... = `dFTComputerAttacksMario` etc., comments in `decomp/src/ft/ftcomputer.c:36`). Entry 12 (BOSS) and 28 (NONE) are 0.
- Remix add-on fighters copy their parent's pointer when they have no own file (`SRC/Character.asm:2871`, `add_to_table(ai_behaviour, ...)` at ~271).
- Address mapping (verified): ROM offset = RAM - 0x80400000 + 0x02C00000. Example: `Banjo.CPU_ATTACKS` = 0x80579424 lives at ROM 0x02D79424 in `build/remixasm/remix.z64`. Its first 0x1C bytes decode to input 0x13, start 4, end 0, X 61.0 / 399.0, Y 227.0 / 402.0, which matches `SRC/Banjo/AI/Attacks.asm:23` (JAB 4, 61, 399, 227, 402).

Pointer table dump (`build/remixasm/remix.z64`, index = Character.id, only entries that resolve to a `*.CPU_ATTACKS` label):

| id | fighter | RAM CPU_ATTACKS | | id | fighter | RAM CPU_ATTACKS |
|---|---|---|---|---|---|---|
| 29 (0x1D) | Falco | 0x80540360 | | 64 | Dedede | 0x8056F944 |
| 30 | Ganondorf | 0x80541B9C | | 65 | Goemon | 0x8057369C |
| 31 | YoungLink | 0x805436F0 | | 66 | Peppy | 0x80574FDC |
| 32 | DrMario | 0x805448CC | | 67 | Slippy | 0x8057750C |
| 33 | Wario | 0x80546EE0 | | 68 | Banjo | 0x80579424 |
| 34 | DSamus | 0x805495BC | | 70 | Ebi | 0x8057CE0C |
| 38 | Lucas | 0x8054B27C | | 71 | DragonKing | 0x8057F418 |
| 52 | Bowser | 0x8054FB68 | | 72 | Crash | 0x805814FC |
| 53 | GBowser | 0x80551418 | | 73 | Peach | 0x805856E8 |
| 54 | Piano | 0x80553780 | | 74 | Roy | 0x805878B4 |
| 55 | Wolf | 0x805559B4 | | 75 | DrLuigi | 0x80588BF4 |
| 56 | Conker | 0x80558E8C | | 76 | Lanky | 0x8058A9D4 |
| 57 | Mewtwo | 0x8055C1DC | | 59 | Sonic | 0x805632B8 |
| 58 | Marth | 0x8055EFB0 | | 61 | SSonic | 0x80564B54 |
| 62 | Sheik | 0x80567FEC | | 63 | Marina | 0x8056B9D8 |

Entries 77-97 repeat parent pointers (clones). The fighter-to-id mapping is by pointer value, which is authoritative for the ROM. Whether the port's `fkind` equals this Remix id (`REMIX_ID_TO_PORT_FKIND`, `port/remix/remix_roster.cpp`) was NOT verified here.

Lv10 table: `Character.lv10_ai_behaviour.table` at `0x8049D9C4` (sym file), fill = `NUM_CHARACTERS * 4`. See section 5.

Data reads: the host-order note is in `port/remix/remix_ram_image.h` (header comment) and `docs/bugs/remix_m2_subsystems_2026-10-16.md:23`. `remix_ram_image_read_word` (`port/remix/remix_ram_image.cpp:91`) returns a native-order u32, so pointers and inputs need no swap. Floats must be bit-cast from the returned u32 (memcpy/union), not converted. Halfword/byte reads use `read_half`/`read_byte` (`remix_ram_image.cpp:98-105`). Byte streams (not these tables) need the big-endian copy path.

## 3. Proposed design: point a Remix fighter's `computer_attack_list` at the Remix table

Current behaviour (the bug):
- `decomp/src/ft/ftport.c:209` sets `desc.computer_attack_list = dFTComputerAttackList[fkind]` for vanilla rows.
- `decomp/src/ft/ftcomputer.c:4000` reads `port_fighter_computer_attack_list(this_fp->fkind)` (registry, `port/fighter_registry.cpp:159`).
- Remix synth fighters copy the parent row: `port/remix/remix_roster.cpp:303` `synth->descriptor = *parent;`. Nothing in `remix_roster.cpp`, `remix_synth_ft.*` or `ftremix*` assigns `computer_attack_list` (grep: only `fighter_registry.*` and `ftport.c`). So the 30 Remix fighters use the parent's vanilla list. Their Remix attack rows are never read.

Proposed hook (no change to ftcomputer.c):
1. In `remix_roster.cpp` `buildSynth`-style function, after line 303-306, build `std::vector<FTComputerAttack>` from the RAM table and store `.data()` in `synth->descriptor.computer_attack_list`. Keep the vector in `SynthFighter` (same ownership as `ft_data`), so it lives as long as the registration.
2. Read path per Remix id `r`: `ptr = remix_ram_image_read_word(REMIX_SYM_Character__ai_behaviour__table + r*4)`; `host = remix_ram_image_resolve(ptr)`. Read 7 u32 words per entry with `remix_ram_image_read_word(addr + 4*k)`, bit-cast the four floats. Stop after the second `0xFFFFFFFF` input (the terminator) and append a final sentinel `input_kind = -1`. Keep the layout as the vanilla array: grounded group, `-1`, aerial group, `-1`. A plain copy of the 28-byte entries also works, but explicit field reads avoid relying on the struct layout of the image.
3. Build once at registration (`port_fighter_register` time), not per frame. A bounds failure (`resolve` returns NULL) should log with `spdlog::error` including the remix id and address, then fall back to the parent list (safe default, explicit log).

Risks:
- Custom input ids (`>= 0x31`, `AI.ROUTINE.*` from `add_cpu_input_routine`) have no port implementation. Affected Remix fighters: Banjo (BANJO_BACKWARDS_NSP, DASH_ATTACK, FAIR/BAIR routines), Bowser, Conker, Crash, DSamus, Dedede, DragonKing, Falco, Ganondorf, Goemon, Marina, Marth, Mewtwo, Roy, SSonic, Sheik, Sonic, Wario, Wolf, Wario... (grep `AI.ROUTINE.` per file, section 4). Attack entries whose id is custom will hit the picker with an unknown input; the vanilla input dispatcher must be extended or those entries filtered. Needed: a decision per routine.
- Attack picker also uses `input_repeat_count`/script ids; `ftcomputer.c` must accept the new ids. Verify the dispatch table size before wiring.
- Prevent routines (section 4) are Remix code addresses, not C functions. The current registry hook `ai_attack_prevent_routine` (`ftcomputer.c:4475-4487`) is never assigned (grep: only `fighter_registry.h:145`, `.cpp:263`), so it is a no-op. Remix `PREVENT_ATTACK.ROUTINE.*` values (`SRC/AI.asm`, NONE 0x80133520, MARIO 0x801334EC, YOSHI_FALCON 0x80133510, KIRBY 0x80133500, SKIP_ATTACK 0x80133A14, plus per-fighter ones like FALCO_NSP) need C ports before they can be honored.
- `ai_long_range` (`LONG_RANGE.ROUTINE.NSP_SHOOT` etc.) and `ai_aerial_chase` have no port hook either (grep found none).
- Regeneration: `tools/gen_remix_actions.py` writes `REMIX_SYM_*` only for `CSS_FIGHTER_DIRS` (18 fighters; `tools/gen_remix_actions.py`, constant near line 60). The sym file has 30 `CPU_ATTACKS`. Missing from `port/remix/remix_symbols.h`: DSamus, DragonKing, Ebi, GBowser, Lanky, Lucas, Peppy, Piano, Roy, SSonic, Slippy, DrLuigi (12). `Character.ai_behaviour.table` is not in `EXTRA_SYMBOLS` (not emitted). Both need adding (the generator, not a hand edit, since the header says "do not edit").
- Symbols needed for the table walk: `REMIX_SYM_Character__ai_behaviour__table` (0x8049A320, add to EXTRA_SYMBOLS) and per-fighter `REMIX_SYM_<F>__CPU_ATTACKS` (or just the pointer table, which removes the need for per-fighter symbols).

## 4. The 30 Remix fighters

Counts are `grep -c` of `add_attack_behaviour|add_custom_attack_behaviour` lines (approximate: lines, not entries). Each file has 2 `END_ATTACKS`.

| Fighter (dir) | Id | Lines | Custom input routines (`AI.ROUTINE.*`) | Prevent routine |
|---|---|---|---|---|
| Banjo | 68 | 22 | BANJO_BACKWARDS_NSP, DASH_ATTACK, FAIR, BAIR | LONG_RANGE NSP_SHOOT; PREVENT MARIO |
| Bowser | 52 | 33 | BOWSER_SHORTHOP_FIREBREATH, POINT_STICK_TO_TARGET, DASH, FAIR, BAIR | PREVENT BOWSER_USP_DSP |
| Conker | 56 | 22 | SMASH_DSP, DASH, FAIR, BAIR | PREVENT CONKER_GRENADE |
| Crash | 72 | 26 | CRASH_DASH_NSP, NSP_TOWARDS, DASH, FAIR, BAIR | LONG_RANGE NONE |
| DSamus | 34 | 22 | DSAMUS_DSP_FOLLOW, DASH, FAIR, BAIR | PREVENT YOSHI_FALCON |
| Dedede | 64 | 21 | DSP, DASH, FAIR, BAIR | PREVENT USP |
| DrLuigi | 75 | 23 | DASH, FAIR, BAIR | PREVENT MARIO |
| DrMario | 32 | 23 | DASH, FAIR, BAIR | PREVENT MARIO |
| DragonKing | 71 | 24 | POINT_STICK_TO_TARGET, DASH, FAIR, BAIR | PREVENT YOSHI_FALCON |
| Ebi | 70 | 22 | DASH, FAIR, BAIR | LONG_RANGE NONE |
| Falco | 29 | 21 | MULTI_SHINE, NULL, DASH, FAIR, BAIR | PREVENT FALCO_NSP |
| GBowser | 53 | 32 | POINT_STICK_TO_TARGET, DASH, FAIR, BAIR | PREVENT GBOWSER |
| Ganondorf | 30 | 29 | NULL, POINT_STICK_TO_TARGET, DASH, FAIR, BAIR | PREVENT YOSHI_FALCON |
| Goemon | 65 | 22 | GOEMON_HELD_NSP, DASH, FAIR, BAIR | LONG_RANGE NSP_SHOOT |
| Lanky | 76 | 18 | DASH, FAIR, BAIR | PREVENT NONE |
| Lucas | 38 | 18 | DASH, FAIR, BAIR | LONG_RANGE NSP_SHOOT |
| Marina | 63 | 22 | NSP, NSP_TOWARDS, DSP, MARINA_DOUBLE_DSP, NULL, DASH, FAIR, BAIR | PREVENT MARINA_NSP |
| Marth | 58 | 28 | DASH, FAIR, BAIR | PREVENT MARIO |
| Mewtwo | 57 | 21 | NULL, DASH, FAIR, BAIR | PREVENT NONE |
| Peach | 73 | 21 | DSP, DASH, FAIR, BAIR | PREVENT MARIO |
| Peppy | 66 | 20 | DASH, FAIR, BAIR | LONG_RANGE NSP_SHOOT |
| Piano | 54 | 20 | DASH, FAIR, BAIR | PREVENT MARIO |
| Roy | 74 | 25 | ROY_DSP_HELD_10F/20F, DASH, FAIR, BAIR | PREVENT MARIO |
| SSonic | 61 | 20 | NSP_TOWARDS, DASH, FAIR, BAIR | LONG_RANGE NONE |
| Sheik | 62 | 23 | NULL, DASH, FAIR, BAIR | PREVENT NONE |
| Slippy | 67 | 19 | DASH, FAIR | LONG_RANGE NSP_SHOOT |
| Sonic | 59 | 20 | SONIC_CHARGED_DSP, NSP_TOWARDS, DASH, FAIR, BAIR | PREVENT SONIC_DSP |
| Wario | 33 | 27 | WARIO_NSP_TOWARDS(+_JUMP), NULL, DASH, FAIR, BAIR | LONG_RANGE NONE |
| Wolf | 55 | 23 | MULTI_SHINE, POINT_STICK_TO_TARGET, NULL, DASH, FAIR, BAIR | PREVENT USP |
| YoungLink | 31 | 32 | DSP, DASH, FAIR, BAIR | LONG_RANGE NSP_SHOOT |

Special cases:
- Banjo, Sheik, Crash and others define their own input scripts (`BANJO_BACKWARDS_NSP:`, `CRASH_DASH_NSP:`, etc.) above `CPU_ATTACKS:`. These are code, so the port must reproduce the input sequence.
- Sheik has a `cpu_post_process` hook (`SRC/Sheik/AI/Attacks.asm:~80`, teleport/DSP steering): this is a separate table (`Character.table_patch_start(cpu_post_process, ...)`), not in the attack list.
- Roy has `ROY_DSP_HELD_10F/20F` labels; Falco/Marina/Sonic have multi-step custom routines.
- Entry-level ranges: a few entries use `DSPA` hitbox offsets (Banjo `26 + 8`, Sheik late hits `4+4`, `5+4`); these are plain numbers in the file and need no special handling.
- Remix prevent routines (`PREVENT_ATTACK.ROUTINE.*`) are `AI.asm` code addresses; each is a `dw` jump target. Note that Mario/Yoshi/Kirby prevent addresses are vanilla (`AI.asm` ROUTINE scope).

## 5. VanillaLv10Attacks.asm

- File: `SRC/VanillaLv10Attacks.asm` (544 lines). Guarded by `__VanillaLv10Attacks__`. Scopes: Mario, Link, Dk, Fox, Captain, Pikachu, Samus, Jigglypuff, Luigi, Yoshi, Ness, Kirby (12 vanilla fighters only).
- Content: the vanilla (level-10 tuned) attack list for each vanilla fighter, same 0x1C format (`AI.add_attack_behaviour`, `AI.END_ATTACKS`). Each scope writes `Character.table_patch_start(lv10_ai_behaviour, Character.id.<F>, 0x4)`; `dw <LABEL>`.
- Table: `Character.lv10_ai_behaviour.table` (`SRC/Character.asm:2959`, RAM `0x8049D9C4`), 4 bytes per character id.
- Selection: `SRC/AI.asm:3559-3572`. If `cpu level (0x13 of the cpu struct) == 10` and the lv10 entry for the character id is nonzero, `s2` is replaced by the lv10 pointer; otherwise the normal `ai_behaviour` pointer is used. So level-10 CPUs of the 12 vanilla fighters use the VanillaLv10 list; all other levels and all Remix fighters use the normal table.
- Port needs: the vanilla fighters' lv10 arrays (12 arrays, same struct as section 1) and a level check (`cpu level == 10`) in the port's picker path (`ftcomputer.c:~4000`). The current decomp has no lv10 branch. The lv10 list would replace `port_fighter_computer_attack_list(fkind)` only when `level == 10`. The level field must be confirmed in the decomp (`FTComputer` struct) before implementing; not verified here.
- Read path: the same RAM-image reader as section 3, but the 12 fighter entries are in `Character.lv10_ai_behaviour.table` (RAM 0x8049D9C4).

## 6. Open items / not verified

- Port fkind vs Remix id mapping (`REMIX_ID_TO_PORT_FKIND`) not checked.
- Dispatcher support for custom input ids (>= 0x31) not checked in `ftcomputer.c`.
- The entry counts (section 4) are grep line counts, not parsed entries.
- The image is not loaded in this session; all RAM reads were done on `build/remixasm/remix.z64` (the Remix build output), not on the running image.
- Nothing in `port/` or `decomp/` was modified.
