# F13 AI port notes (ROADMAP M7, T7.1)

## Slice A1: Remix attack tables from the RAM image

### Files created

- `decomp/src/ft/ftremix/ftremixaitables.c` and `.h`: `ftRemixAiGetAttackList(s32 fkind, s32 level)`, lazy per-fkind cache of
  the normal table (`Character.ai_behaviour.table`) and of the lv10 table (`Character.lv10_ai_behaviour.table`).
- `decomp/src/ft/ftremix/ftremixaitablesrules.c` and `.h`: pure rules (int/float only): terminator, custom id, walk to the second
  terminator, f32 bit-cast, lv10 level, table index bound.
- `decomp/src/ft/ftremix/ftremixaicommands.h`: stub with the prototype only, `sb32 ftRemixAiIsCustomInputSupported(s32 input_kind)`.
  The real definition belongs to slice A2 (`ftremixaicommands.c`). Until A2 is merged the build does not link.
- `tests/remix/test_ai_tables_rules.c`: entry `remixTestAiTablesRules`, 8 tests, 20 checks, all pass when compiled standalone with
  `-DPORT=1 -DREGION_REMIX=1`. The lead still registers it in `tests/remix/CMakeLists.txt` and `test_main.c`.

### Files edited

- `decomp/src/ft/ftcomputer.c`: include of `ft/ftremix/ftremixaitables.h` (REGION_REMIX block, after `ftremixfields.h`); attack list
  lookup at ~line 4000 (`ftRemixAiGetAttackList` first under `PORT && REGION_REMIX`, registry list when NULL; `elif PORT` and the
  vanilla `dFTComputerAttackList` branch unchanged). The grep found no other reader of `port_fighter_computer_attack_list` or
  `dFTComputerAttackList` in `ftcomputer.c`.

### Data sources

- Pointer table: `REMIX_SYM_Character__ai_behaviour__table` (0x8049A320) and `REMIX_SYM_Character__lv10_ai_behaviour__table`
  (0x8049D9C4), both already in `port/remix/remix_symbols.h`. No new `EXTRA_SYMBOLS` needed.
- Index = fkind. `REMIX_ID_TO_PORT_FKIND` is the identity, so the Remix id is the port fkind.
- Entry: 7 words (input, start, end, four f32 bounds), read with `remix_ram_image_read_word`, floats bit-cast.
- Walk: up to 128 entries (896 words) until the second `0xFFFFFFFF` input. The list keeps both terminators as `input_kind == -1`,
  so it ends with the sentinel and keeps the grounded and aerial groups as in the vanilla arrays.

### Selection rule (AI.asm 3547-3572, P01 custom_attacks_table)

- `level == 10` and a nonzero lv10 entry for the fkind: the lv10 list. This applies to any fkind that has an lv10 entry, as the asm does.
- Otherwise a fkind `>= nFTKindEnumCount` (Remix fighters): the normal table list.
- Otherwise NULL, so the vanilla registry list is used.
- Level is `fp->level` (u8). Level 10 is reachable: `default_cpu_level` toggle range is 0 to 10 (Toggles.asm:2403), and the spawn
  setup copies it to the player level (`gmremixspawnsetup.c:255-262`).

### Logs (port_log, one line per fkind, each failure once)

- `fkind N normal table at 0xADDR built: K entries kept, D dropped` (and `lv10`).
- Zero pointer for a Remix fighter, table outside the image, no second terminator within 128 entries, out of memory: explicit line,
  then the registry list. Failures are cached per fkind, so the line appears once.

### Deviations from the asm and brief

1. The Remix condition is `fkind >= nFTKindEnumCount` as the brief says. fkinds 27 (RANDOM) and 28 (NULL) never reach it with a table.
2. The lv10 branch is generic (any fkind with a nonzero lv10 entry), as in the asm, not restricted to the 12 vanilla fighters.
3. Custom input ids (`>= 0x31`) are dropped until `ftRemixAiIsCustomInputSupported` returns TRUE. This is a behaviour change for Remix
   fighters: until now they use the parent's vanilla list, after this change their Remix lists lack the custom routines. Examples:
   Banjo DASH_ATTACK / BANJO_BACKWARDS_NSP, Falco MULTI_SHINE, Marina NSP. Expect the picker to attack less with these fighters until A2.
4. Entries with `NULL` in the Remix sources (e.g. Falco `MULTI_SHINE, NULL, DASH`, Marth/Ganondorf/Sheik) are read as input 0 (vanilla).
   The meaning of `NULL` in `AI.asm` was NOT verified; decide in review.
5. `hit_end_frame` is read as written (0 for `add_attack_behaviour`).
6. Prevent routines and the long-range/aerial-chase routines are not in this slice (A3 / other slices).

### Ai-memory

- Global rules (`_rules/`: no agent attribution, no live provider tests, ai-memory recall) read in full.
- The project scope of this repository was NOT found: `BattleShip`, `JRickey/BattleShip` (workspace `JRickey`) all failed. No
  `.ai-memory.toml` exists. Project rules were not loaded; the BattleShip AGENTS.md and the brief were read instead.

### Lead checks in a match

- Build: `ftremixaicommands.c` (A2) must exist with the definition; without it the link fails.
- Register `test_ai_tables_rules.c` in the tests CMake; run `remixTestAiTablesRules`.
- `vs.sh 68,58` (Banjo vs Marth) at CPU level 9: the log must show `fkind 68 normal table ... built` and `fkind 58 normal table ...
  built` with kept and dropped counts, and no `lv10` line (level 9 does not use the lv10 table).
- Level 10 vs a vanilla fighter (e.g. Mario) with the default_cpu_level toggle at 10: an `fkind 0 lv10 table ... built` line appears
  only if the lv10 entry is nonzero. For Mario the lv10 entry should exist per the notes.
- Compare the attack picker of Banjo before and after: the custom DASH_ATTACK and FAIR/BAIR routines are missing until A2.

### REMIX-TODO(AI)

- Custom routine ids: `ftRemixAiIsCustomInputSupported` (slice A2). Until then custom entries are dropped (logged as dropped counts).
- The meaning of `NULL` entries in the Remix tables (deviation 4).

## Slice A4

Scope: AI.asm P02 to P17 (combat and recovery), with the hooks that are clear in the decomp. P01, P18 to P25 not touched.

Files created:
- `decomp/src/ft/ftremix/ftremixaicombat.{c,h}`: hooks for P03, P10, P15.
- `decomp/src/ft/ftremix/ftremixaicombatrules.{c,h}`: pure rules (int in, int out).
- `decomp/src/ft/ftremix/ftremixairecovery.{c,h}`: hook for P04.
- `decomp/src/ft/ftremix/ftremixairecoveryrules.{c,h}`: pure rules for P04 and the P05 clamp.
- `tests/remix/test_ai_combat_rules.c`, entry `remixTestAiCombatRules` (lead registers it in tests/remix/CMakeLists.txt and test_main.c). Covers both rule files.

Files edited:
- `decomp/src/ft/ftcomputer.c`: two includes after the remix includes, and four hooks:
  - P03 at the start of `ftComputerCheckEvadeDistance` (0x80132BC8): `ftRemixAiCombatEvadeIsSuppressed` returns FALSE.
  - P10 at the start of `ftComputerCheckTryChargeSpecialN` (0x80136A20): `ftRemixAiCombatChargeIsSkipped` returns FALSE.
  - P15 in the condition of `ftComputerProcDefault` before `func_ovl3_80135B78` (0x80137778): `!ftRemixAiCombatProjectileIgnored(fp)`.
  - P04 in the condition of the up-B block of `ftComputerFollowObjectiveWalk` (0x80134E98, `is_attempt_specialhi_recovery`): `|| ftRemixAiRecoveryAllowsUpBRepeat(fp)`.

Patch status:
- P03 done. Only the improved toggle is checked. The Remix 1P automatic improved AI is REMIX-TODO(AI): there is no live Remix 1P flag in the port (see ftremixhitbox.c:18). Timers: `star_invincible_tics` (asm 0x5B0) and `invincible_tics` (asm 0x5A4), inferred from the field order in fttypes.h. Skip = return FALSE from the whole check (asm jumps to 0x80132CE8, taken as the exit).
- P04 done, medium confidence. Hook placed at the decomp block that tests `is_attempt_specialhi_recovery`, which the asm `bnez t1` seems to match. Classic mode (vanilla 1P/RTTF) is mapped to `nSCBattleGameType1PGame`, which is an assumption (the asm reads Global.match_info, GAMEMODE.CLASSIC). The Remix 1P part is REMIX-TODO(AI).
- P05 partly: the clamp rule `ftRemixAiRecoveryClampTargetX` exists and is tested, but the hook is NOT placed. The asm inserts it mid-function in ftComputerFollowObjectiveWalk (writes computer target X at +0x60); the decomp point is not mapped. REMIX-TODO(AI).
- P10 done. Level 10 only. The roll uses `syUtilsRandIntRange(60)` and only runs for level 10, so lower levels do not consume RNG.
- P15 done. MLUIGI and GBOWSER always ignore projectiles. Other fighters ignore them with the Franklin Badge, via `itRemixFranklinBadgeIsActive(fp)` (per-fighter flag). The asm reads `Item.FranklinBadge.players_with_franklin_badge[port]`: equivalence not checked.
- P07 not re-checked in this slice (the brief asked to check it against the asm).
- P02, P06, P08a/b, P09, P11, P12, P13, P14, P16, P17: not ported in this slice (time and scope). P08 has two call sites, U-02 unresolved. P02 needs the Size multiplier table in the port.

Deviations and suspected bugs:
- P03 (AI.asm 102-104): the Remix 1P test compares `at` with `v1` after `t6 = 4`, so the automatic 1P improved AI never triggers. Not ported (the 1P part is absent), keep in mind when it is ported.
- P04: the asm fix forces the vanilla "not used yet" branch, so the CPU repeats up-B. Level 10 always repeats, even in classic mode (asm order kept).
- P10: the asm saves no ra around `jal Global.get_random_int_`. Not applicable to the C port.

EXTRA_SYMBOLS: none. `REMIX_SYM_Toggles__entry_improved_ai` already exists in port/remix/remix_symbols.h.

REMIX-TODO(AI) left: P03 Remix 1P automatic improved AI; P04 Remix 1P; P05 hook point; P02, P06, P08, P09, P11 to P14, P16, P17.

How to see it in a match (CPU level 9 unless noted):
- P03: enable "Improved AI" (toggle `improved_ai`). A CPU next to an opponent with Starman or right after respawn should not evade.
- P04: CPU at level 10 (any fighter) recovering with up-B twice; or toggle on at level 9 outside 1P/RTTF.
- P10: CPU at level 10 starts NSP charge rarely (about 1/60 per check).
- P15: CPU MLuigi or GBowser, or a CPU wearing the Franklin Badge, does not dodge or react to projectiles.

## Slice A2

Custom CPU input scripts (ids >= 0x31) and the extended script commands (P18-P22). Source: `AI.asm` 1555-1600 and 2197-2490, `ftcomputer.c` ftComputerUpdateInputs and the three ftComputerSetCommand* functions.

Files created:
- `decomp/src/ft/ftremix/ftremixaicommands.h` and `.c`: `ftRemixAiIsCustomInputSupported`, `ftRemixAiGetInputScript`, `ftRemixAiRunCustomCommand` (CUSTOM(n) 0xFE), `ftRemixAiStickXFromCode` (codes 0x81-0x84).
- `decomp/src/ft/ftremix/ftremixaicommandsrules.h` and `.c`: pure stick code and turn rules.
- `tests/remix/test_ai_commands_rules.c` (entry `remixTestAiCommandsRules`, not registered: the lead adds it to `test_main.c` and `CMakeLists.txt`).

Files edited (`decomp/src/ft/ftcomputer.c`, all under `PORT && REGION_REMIX`):
- include of `ftremixaicommands.h` at the top.
- `ftComputerGetInputScript(index)` (new static helper, vanilla path reads the same table entry) used by ftComputerSetCommandWaitShort, ftComputerSetCommandImmediate and ftComputerSetCommandWaitLong (P18-P20).
- `ftComputerUpdateInputs`, STICK_X_TILT switch: cases 0x81-0x84 (P21 -> `ftRemixAiStickXFromCode`).
- `ftComputerUpdateInputs`, `else switch (command)`: `case FTREMIX_AI_COMMAND_CUSTOM` (0xFE) (P22 -> `ftRemixAiRunCustomCommand`).

Hooks and asm mapping:
- P18/P19/P20 (0x80132734/0x80132938/0x80132760) -> `ftComputerSetCommandWaitShort` , `ftComputerSetCommandWaitLong`, `ftComputerSetCommandImmediate`. The command table is read through `remix_ram_image_read_word(REMIX_SYM_AI__command_table + 4*id)`, and the script bytes through `remix_ram_image_read_byte` (big-endian path). Each script is copied into a host buffer (0x80 bytes, stops at END 0xFF) at its first start. Ids past the table (>= 0xFF) get an END-only script.
- P21 (0x80131DA4) -> STICK_X_TILT. 0x81 AUTOFULL_AWAY and 0x82 AUTOHALF_AWAY: +/- 80 or 40 depending on player X >= target X. 0x83 FORWARDS and 0x84 BACKWARDS: +/- 80 along facing (`lr`, >= 0 is right).
- P22 (custom dispatch at the command byte 0xFE, ROM entry 0x1065BC / 0x8018BB7C): the sub-index 0..4 is JUMPSQUAT_WAIT, PRESS_C, UNPRESS_C, STICK_TO_TARGET, TURNAROUND_WAIT. A wait sets `input_wait = 1` and rewinds the script to the 0xFE byte. In the decomp, 0xFE has no case in the `else switch`, so it was a silent no-op before this slice.

Deviations from the asm:
1. TURNAROUND_WAIT (asm 2446-2457): the first `beql` annuls its delay slot (`addiu t6, TurnRun`) when Action.Turn does not match, so the second compare sees Turn and never detects TurnRun. Port waits on Turn OR TurnRun (the evident intent, comment "until out of turning animation"). Behaviour change, documented here.
2. STICK_TO_TARGET: the asm comment says "truncate" but `cvt.w.s` rounds to nearest. Port uses `lrintf` (nearest), which matches the instruction. A one-unit difference at most.
3. STICK_TO_TARGET location: the asm reads FTStruct +0x78 ("location vector"). Port uses `joints[nFTPartsJointTopN]->translate`, the same point the vanilla AUTOFULL code uses. Not verified against +0x78.
4. UNPRESS_C clears the four C-button bits (mask 0x000F), as the comment says. PRESS_C sets only 0x0001 as in the asm.

U-03 (0x8018BB7C): not resolved. The jump-table word is not in the decomp, and the port does not patch the ROM. The C switch in `ftRemixAiRunCustomKind` replaces it. REMIX-TODO(AI): none needed for the port path, but the ROM-level address stays unverified.

EXTRA_SYMBOLS: none. `REMIX_SYM_AI__command_table` already exists in `port/remix/remix_symbols.h`. The routine labels are reached through the table, so no per-routine symbols are needed.

How to see it in a match: the port log line `SSB64: Remix AI input 0x.. routine at 0x... loaded` is written the first time each custom id starts (once per boot per id). Run a CPU at level 9 of a fighter whose attack table uses a custom input (e.g. Banjo, BANJO_BACKWARDS_NSP) and grep the port log for `Remix AI input`. Unknown custom command indices are logged as `Remix AI custom command index`.

Not done: `ftremixhooks.h` untouched (the include lives in ftcomputer.c); tests not registered; no build run (lead builds). `scripts/remix/syntax-check.sh` reports OK for `ftremixaicommands.c`, `ftremixaicommandsrules.c` and `ftcomputer.c`; the standalone compile of the test file is clean.

## Slice A3

Arquivos criados:
- `decomp/src/ft/ftremix/ftremixaiprevent.{c,h}`: `sb32 ftRemixAiPreventAttack(FTStruct *fp, s32 input_kind)`, dispatch por `fp->fkind`.
- `decomp/src/ft/ftremix/ftremixaipreventrules.{c,h}`: regras puras (ints) de veto.
- `tests/remix/test_ai_prevent_rules.c`: entrada `remixTestAiPreventRules` (CMakeLists e test_main.c ficam com o lead). Rodado standalone com `PORT=1 REGION_REMIX=1`: 19 checks, 0 falhas.

Arquivo editado: `decomp/src/ft/ftcomputer.c`: include de `ft/ftremix/ftremixaiprevent.h` (linha 13) e bloco `#if defined(PORT) && defined(REGION_REMIX)` logo depois do bloco PORT de `ai_attack_prevent` (~linha 4537), que chama `ftRemixAiPreventAttack` e retorna FALSE. Syntax-check OK.

Rotinas PREVENT portadas (AI.asm 2973-3180):
- FALCO_NSP (Falco): NSP vetado no ar (`ga != nMPKineticsGround`, asm 0x14C); USP delega para FOX_USP.
- FOX_USP (FOX, JFOX, SLIPPY): USP vetado em lv10.
- BOWSER_USP_DSP (Bowser): DSP e USP.
- GBOWSER (GBowser): DSP. O comentario do asm fala em "facing ledge", mas o codigo nao testa direcao.
- USP (Wolf, Dedede): USP.
- SONIC_DSP (Sonic, DSamus): DSP.
- WARIO (Wario): DSP, 0x42, 0x43.
- MARINA_NSP (Marina): NSP.
- CONKER_GRENADE (Conker): DSP ou SMASH_DSP vetado com granada ativa (word N64 0x0ADC via `ftRemixFighterWord`, mapeado em passive_vars).

Pendente (REMIX-TODO(AI), sem veto por enquanto):
- CRASH (Crash/Crash.asm:716): veta DSP/USP em plataforma soft, que le o halfword de superficie N64 0x00F6 (flag 0x4000). Nao ha campo nem mapeamento em `ftremixfields`.
- PUFF_DSP: nao registrado (so em bloco comentado, AI.asm 6453-6464). Nao portado.
- MARIO (Banjo, DrLuigi, DrMario, Marth, Peach, Piano, Roy), YOSHI_FALCON (Ganondorf, DragonKing) e KIRBY: rotinas vanilla em 0x801334EC, 0x80133510, 0x80133500. O codigo nao esta em `smashremix/src`; precisa de desmontagem da ROM.
- NONE (Lanky, Mewtwo, NFalco, NMarina, Sheik, NSheik, NGanondorf): nunca veta, nada a portar.

LONG_RANGE e AERIAL_CHASE (P23, P24, P25): NAO portados.
- O decomp nao tem C que leia `Character.ai_long_range.table` / `ai_aerial_chase.table`. O trecho vanilla de 0x80133880 (AERIAL_CHASE) e 0x80138CB8 / 0x80138EA0 (LONG_RANGE e custom_move) so existe como asm. `func_ovl3_80138AA8` (ftcomputer.c:6964) decide por switch de fkind, sem a tabela. O ponto de hook precisa de mapeamento antes do port.
- A classificacao de poligono (P23/P24) depende de `NUM_CHARACTERS` e `NUM_POLYGONS` (Character.asm:27, 352), sem valor em C.

Pesos (P4, `ftRemixAiAttackWeight`) e P7: NAO portados. P4 tem ~450 linhas e le campos sem nome em C (+0x13, +0x40, +0xB18, +0x18C, +0xB34, +0x10C). P7 depende de P4/P5/P6.

Desvios e verificacoes pendentes:
- Registros duplicados: Falco fica com FALCO_NSP e DSamus com SONIC_DSP. A ordem de include (main.asm: AI.asm 128, Falco 151, DSamus 164, `DSamus/AI/Attacks.asm` dentro de DSamus.asm:150 antes de DSamus.asm:283) da essa resolucao. VERIFY no build.
- `SMASH_DSP` = 0x41 (0x31 + 16, contagem de `add_cpu_input_routine` em AI.asm 1603-1798). VERIFY contra o id atribuido pelo A2.
- Conker: a nota dizia "pula quando grenade == 0". O asm (AI.asm 3134-3160) tem `beqz at, _allow_attack // branch if grenade available`, entao veta quando o word != 0. Segui o asm.
- Semantica do veto: TRUE faz `ftComputerCheckDetectTarget` retornar FALSE (aborta a selecao do frame), igual ao bloco PORT existente. O asm so faz `t2 = 1` e salta para o commit (0x80133520) ou SKIP_ATTACK. VERIFY o contrato de `t2` antes de fechar.
- `docs/remix/notes/f13_ai_attack_tables.md` diz "Crash: LONG_RANGE NONE". Essa e outra tabela; o PREVENT do Crash e CRASH.

Como ver na partida: o primeiro veto de cada fkind escreve no port_log `ftRemixAiPreventAttack: fkind 0x.. level .. vetoed input 0x.. (first veto of this fkind)`. Para testar: CPU nivel 9 com Falco, Conker, Marina ou Sonic contra qualquer personagem, e procurar a linha no log.

EXTRA_SYMBOLS: nenhum simbolo REMIX_SYM novo.

## Slice C1

Scope: patches of `f13_ai_patches_c.md`: P3, P5, P6, P10, P17 to P25 (clamps). P8/P9 and P11 are NOT ported (see TODO).

Files created:
- `decomp/src/ft/ftremix/ftremixailevel.{c,h}`: `ftRemixAiClampedLevel`, `ftRemixAiWalkStopThreshold`, `ftRemixAiNearCollisionScale` (fighter wrappers).
- `decomp/src/ft/ftremix/ftremixailevelrules.{c,h}`: pure rules (int/float): clamp to 9, walk-stop threshold `max(0, 18 - 2*level)`, collision base 0.875 at level 10.
- `decomp/src/ft/ftremix/ftremixairoll.{c,h}`: `ftRemixAiCanJumpWhileFalling` (P3), `ftRemixAiSkipRepeatedInputCheck` (P5), `ftRemixAiInputRepeatLimitHit` (P6), `ftRemixAiDownCommand` (P10).
- `decomp/src/ft/ftremix/ftremixairollrules.{c,h}`: pure rules for level 10, aerial-jump proc, idle shield drop.
- `tests/remix/test_ai_level_rules.c` (entry `remixTestAiLevelRules`, 8 checks, pass standalone) and `tests/remix/test_ai_roll_rules.c` (entry `remixTestAiRollRules`, 9 checks, pass standalone). Lead registers both in `tests/remix/CMakeLists.txt` and `test_main.c`.

Files edited: `decomp/src/ft/ftcomputer.c`
- Includes `ftremixailevel.h` and `ftremixairoll.h` in the REMIX block.
- Macros after the includes: `FTCOMPUTER_LEVEL_CLAMPED(fp)`, `FTCOMPUTER_WALK_STOP_WAIT(fp)`, `FTCOMPUTER_COLL_SIZE_BASE(fp)`. Vanilla expansions are `((fp)->level)`, `((-(fp)->level * 2) + 18)` and `1.0F`, so the vanilla code is unchanged after preprocessing.

Hooks (AI.asm patch -> decomp function, line range; comments are the anchors):
- P17 `clamp_level_9_1` -> `ftComputerSetCommandWaitShort` (0x80132564), ground input_wait line.
- P19 `clamp_level_9_3` -> same function, aerial input_wait line (0x80132664). Both lines clamped.
- P21 `clamp_level_9_5` -> `ftComputerSetCommandWaitLong` (0x80132778), both lines.
- P22 `clamp_level_9_6` -> same function (0x80132884).
- P18 `clamp_level_9_2` -> `func_ovl3_8013837C` (0x8013837C), `fighter_follow_wait` line. MEDIUM: the only level read in that function.
- P20 `clamp_level_9_4` -> `func_ovl3_80138AA8` (0x80138AA8), `unk_ftcom_0x35` line (the `MAX - level` product). MEDIUM: only the product goes negative at level 10; the `(level - 1) / 9` ratio is left unclamped.
- P23 `clamp_level_9_7` -> `ftComputerCheckDetectTarget` (0x80132EC8), `damage_coll_size_mul` line: level clamp and base 0.875 at level 10, 1.0 otherwise. MEDIUM: the asm point is not mapped by a label.
- P24 `clamp_level_9_8` -> `ftComputerGetObjectiveStatus` (0x80136D0C), CliffWait `(MAX - level) * 15` line. MEDIUM. The bc1fl `sll v1,v1,1` is the vanilla `action_wait *= 2` before it, kept as is. The downwait line `* 25` (not in any patch) is NOT clamped.
- P25 `clamp_level_9_9` -> `func_ovl3_8013877C` (0x8013877C), walk_stop_wait comparison. Exact asm match (`8013877C+250`).
- P3 `fix_multi_doublejump_usage` -> `ftComputerFollowObjectiveWalk` (0x80134E98), the last term of the `jumps_used < jumps_max` status list: `ftRemixAiCanJumpWhileFalling(fp)` = status FallSpecial OR (level 10 AND `proc_update == ftCommonJumpAerialProcUpdate`).
- P5 `skip_repeated_input_check` -> `ftComputerCheckDetectTarget`, first `switch (input_kinds[i])` (the range adjustment on `*_button_*_count`): skipped at level 10 with `if (!ftRemixAiSkipRepeatedInputCheck(this_fp))`. The switch default is `break`, so this matches the asm jump to the default case.
- P6 `skip_repeated_input_skip` -> same function, `if (com->input_repeat_count >= 4)`: `ftRemixAiInputRepeatLimitHit(this_fp, com)`. At level 10 it returns FALSE and resets the counter (the asm `sb r0, 0x37`).
- P10 `shield_drop_` -> `ftComputerFollowObjectiveWalk`, platform drop branch (`MAP_VERTEX_COLL_PASS`, asm 0x80135B3C): `ftRemixAiDownCommand(fp)`. Level 10 and status `nFTCommonStatusWait` send the custom SHIELD_DROP (0x37); others send `nFTComputerInputStickND1MoveAutoSmashLw` (= AI.asm STICK_DOWN 0x27, checked against the enum order).

Deviations and assumptions:
1. Brief parenthetical "with level 10 allowed they must see 10" conflicts with the asm and the notes: the Remix patches CLAMP level 10 to 9 in these reads (`slti 10` / `lli 9`). `MAX - level` would be -1 at level 10 and produce negative waits. I ported the asm (clamp), not "see 10". Confirm with the lead.
2. P24 `FTCOMPUTER_LEVEL_MAX` stays for the downwait line (unpatched), so at level 10 that line still reads -1.
3. P10 "Idle" = `status_id == nFTCommonStatusWait` (AI.asm `Action.Idle` at +0x24 assumed to be the status id). Not verified in game.
4. P6 side effect: the predicate also resets `input_repeat_count` at level 10 (asm store). Name and TSDoc say so.
5. P3 `ftCommonJumpAerialProcUpdate` is compared directly (decomp function, not a REMIX symbol).
6. Asm suspected bug B1 (P5, double `sll t5`) does not apply to C.

EXTRA_SYMBOLS: none.

REMIX-TODO(AI) left in this slice:
- P8/P9 `stop_roll_spam_` (`func_ovl3_80138EE4`, AI.asm 6039-6307): per-character close-quarter branches, RNG order and the `Character.close_quarter_combat.table` jump table. Not ported (about 230 asm lines, needs its own slice). The header of `ftremixairoll.c` carries the marker.
- P11 `cpu_di_` (`ftComputerProcessObjective`, AI.asm 6533-6812): combo memory, training DI menu, DI tables, Improved AI toggle gate, Remix 1P flag. Not ported. No `ftremixaidi.*` or `test_ai_di_rules.c` created.
- Not in this slice: P1, P2, P4, P7 (A-slices), P12-P16 (CSS).

How to see it in a match (`SSB64_VS_CPU_LEVEL` with the vs.sh shortcut, CPU level 9 vs 10):
- Clamps (P17-P22, P25, P18, P20, P23, P24): CPU level 10 must not produce negative waits. At level 10 the CPU's input waits stay >= 1 frame. Level 9 is unchanged. P25: a CPU at level 10 on the ground walking near the stop line gets threshold 0.
- P3: level 10 CPU falling below the ledge with aerial jumps left should jump again without waiting in Fall.
- P5/P6: level 10 CPU repeating the same attack many times should not stop after four in a row; level 9 still does.
- P10: level 10 Idle CPU on a soft platform sends the SHIELD_DROP routine; level 9 sends the vanilla drop. Log the routine id if needed.

## Slice A3b

Arquivos criados:
- `decomp/src/ft/ftremix/ftremixailongrangerules.{c,h}`: regras puras (ints) do LONG_RANGE: usa tabela, polígono, rotina -> tipo, comando customizado.
- `decomp/src/ft/ftremix/ftremixailongrange.{c,h}`: `ftRemixAiLongRangeSwitchKind(fkind)` (lê `Character.ai_long_range.table` pela imagem), `ftRemixAiLongRangeCommand(fp, default)` (lê `Character.nsp_shoot_custom_move.table`) e `ftRemixAiLongRangeSetCommand(fp, is_delay)`.
- `tests/remix/test_ai_longrange_rules.c`: entrada `remixTestAiLongRangeRules` (CMakeLists e test_main.c com o lead).

Arquivos editados:
- `decomp/src/ft/ftremix/ftremixaipreventrules.{c,h}`: regra `ftRemixAiPreventRulesCrashSoftPlatformVeto`.
- `decomp/src/ft/ftremix/ftremixaiprevent.c`: `case REMIX_ID_CRASH` e `ftRemixAiPreventCrashSoftPlatform`. REMIX-TODO do Crash removido.
- `tests/remix/test_ai_prevent_rules.c`: teste do Crash (registrado no `remixTestAiPreventRules`).
- `decomp/src/ft/ftcomputer.c`: include de `ftremixailongrange.h` (linha 23); hook antes do `switch (fkind)` de `func_ovl3_80138AA8` (~7169); e a escolha do comando final (~7227) substituída por `ftRemixAiLongRangeSetCommand` sob `PORT && REGION_REMIX` (o `#else` é o bloco vanilla intacto).
- `tools/gen_remix_actions.py`: `EXTRA_SYMBOLS` ganhou `Character.nsp_shoot_custom_move.table`.

Rodado standalone (`cc -DPORT=1 -DREGION_REMIX=1` com um main temporário que define `gRemixTestChecks`/`gRemixTestFailures`): long range e prevent, 45 checks, 0 falhas. Syntax-check OK em `ftremixailongrangerules.c`, `ftremixaiprevent.c`, `ftremixaipreventrules.c` e `ftcomputer.c`. `ftremixailongrange.c` falha só por `REMIX_SYM_Character__nsp_shoot_custom_move__table` (indefinido até o lead regenerar `port/remix/remix_symbols.h`).

Hooks portados:
- P24 (AI.asm 2543-2565, 0x80138CB8) -> `func_ovl3_80138AA8` (ftcomputer.c). Polígono -> NONE. Para fighters da tabela, a rotina lida de `Character.ai_long_range.table` vira o case do switch: NSP_SHOOT -> `nFTKindMario` (mesmo corpo de NSP), PHYSICAL_SHOOT -> `nFTKindLink` (boomerang e depois NSP), NONE ou desconhecido -> sem case (`FTREMIX_AI_LONG_RANGE_NO_CASE`, retorna FALSE). Fighters vanilla (0x00-0x0D e 0x1A-0x1D) mantêm o switch vanilla.
- P25 (AI.asm 2567-2615, 0x80138EA0) -> mesmo fim da função: `ftRemixAiLongRangeSetCommand` troca o comando de `WaitLong` pelo `Character.nsp_shoot_custom_move.table[fp->fkind]` quando não é 0. O default é AutoX B (is_delay FALSE) ou Tilt AutoX (is_delay TRUE), como no vanilla.
- Crash PREVENT (Crash/Crash.asm:716, AI.asm 3034-3056): DSPG e USPG vetados com plataforma soft. Campo: `fp->coll_data.floor_flags & MAP_VERTEX_COLL_PASS` (0x4000).

Desvios do asm (decisão do lead):
- D1 Polígono: o asm usa faixas `id < FALCO` (0x1D) e `id >= NUM_CHARACTERS - NUM_POLYGONS`. Com o roster (97 ids, 0x00-0x60, 32 polígonos), a faixa `NUM_CH - NUM_POLY` começa em 0x41 e inclui 0x41-0x4C (Goemon, Peppy, Slippy, Banjo, MLuigi, Ebi, DragonKing, Crash, Peach, Roy, DrL, Lanky), que não são polígonos. A faixa `< FALCO` também inclui 0x1A-0x1C (GDonkey, Random, None). Porte usa a variante POLYGON do roster: 0x0E-0x19 e 0x4D-0x60. Efeito: Lanky, Banjo, Crash, Peach etc. usam a tabela (Lanky/Banjo NSP_SHOOT); o asm mandaria NONE. VERIFY `NUM_POLYGONS` no build.
- D2 `at` do vanilla: o asm depende do flag `at` do OG line 1 (não visível). Assumi o comentário do Remix, "at = 0 if character ID is >= POLY MARIO" (0x0E), ou seja, ids < 0x0E seguem o vanilla. Consistente com D1.
- D3 Crash: o offset N64 0x00F6 é inferido como o halfword baixo de `floor_flags` (word em 0x00F4). Não conferi no layout N64 do FTStruct. VERIFY.
- D4 P25 é aplicado a todos os fighters (leitura da tabela sem gate, como no asm). Entradas vanilla devem ser 0; só Dedede, YLink e Peach registram custom move no Attacks.asm.
- D5 Gating: as notas (f13_ai_patches_c.md, linha 34) dizem que os hooks P23-P25 exigem CPU nível 10. O asm de P24/P25 não testa nível: o hook roda em qualquer nível para fighters da tabela. Confirmar na partida.

Não portado (REMIX-TODO(AI)):
- P23 AERIAL_CHASE.extend_polygon_check_ (0x80133880): o decomp não tem ramo de chase aéreo (`grep chase` em ftcomputer.c sem resultado). A escolha NORMAL/CHASE vanilla não existe em C. Sem ponto de hook.
- P4 weight_attack_options (AI.asm 5485-5938, 0x80133784): não portado. Motivos: ~450 linhas de asm; campos sem nome (+0x13 level, +0x40, +0xB18, +0x18C, +0xB34, +0x10C); a tabela de chances é compartilhada com P5-P7, que não estão portados; R4a/R4b/R4c abertos em f13_ai_patches_c.md. Porte parcial quebraria o contrato da tabela de chances.
- P7 no_attack_when_all_odds_eq_zero (AI.asm 6015-6034, 0x80133FC4): depende de P4 (zeragem de chances). A semântica do retorno vanilla em +0x1108 não foi confirmada (R7). O decomp tem o análogo `input_repeat_count >= 4` (ftcomputer.c, ftComputerCheckDetectTarget), mas não o hook.

EXTRA_SYMBOLS (o lead regenera `port/remix/remix_symbols.h`):
- `Character.nsp_shoot_custom_move.table` (já adicionado em `tools/gen_remix_actions.py`).
- `Character.ai_long_range.table` já existe como `REMIX_SYM_Character__ai_long_range__table`.

Registro para o lead: `remixTestAiLongRangeRules` em tests/remix/CMakeLists.txt e test_main.c.

Como ver na partida (CPU nível 9, sem gate de nível no código):
- Long range NSP (AutoX B à distância): Lanky, Mewtwo, Conker, Dedede, Banjo (tabela NSP_SHOOT). Sem NSP à distância: Falco, Wario, Sonic, Crash (NONE). Vanilla Mario, Fox, Samus, Luigi, Pikachu, Metal Mario: inalterados.
- Custom move: Dedede, YLink, Peach trocam o comando do NSP à distância pela entrada da tabela. Confirmar o comando no `port_log` ou no input do CPU.
- Crash (0x48): DSP ou USP do CPU em plataforma soft (stage com plataforma que atravessa) é vetado. Log: `ftRemixAiPreventAttack: fkind 0x48 level 9 vetoed input 0x1B (first veto of this fkind)`.
- Tabela desconhecida (valor fora de 0x80138D24/0x80138CD4/0x80138ECC): `ftRemixAiLongRangeSwitchKind: fkind 0x.. has long range routine 0x..`, uma vez por fkind.

## Slice A4b

Scope: AI.asm P02, P05, P06, P16, P17 (the patches A4 left). P01, P07 (already ported), P08 to P15 and P18 to P25 not touched.

Files created (decomp submodule, `decomp/src/ft/ftremix/`):
- `ftremixaisize.{c,h}`: `ftRemixAiSizeDetectScale` (P02).
- `ftremixaicloud.{c,h}`: `ftRemixAiUspIsFoxFamily`, `ftRemixAiUspIsCloudAction` (P16), `ftRemixAiCloudRecoveryInput` (P17).
- `ftremixaicloudrules.{c,h}`: pure Magic Cloud action rules and the high-byte button write.

Files edited:
- `ftremixairecovery.{c,h}`: classic-mode helper extracted (P04 unchanged in behaviour); P06 hooks `ftRemixAiRecoveryVanillaPikachuApplies`, `ftRemixAiRecoveryWolfTarget`, `ftRemixAiRecoverySpecialCase`.
- `ftremixairecoveryrules.{c,h}`: `ftRemixAiRecoveryIsWolfUspAction`, `ftRemixAiRecoveryIsNspChargeAction`, `ftRemixAiRecoveryKeepsPikachuVanilla`.
- `decomp/src/ft/ftcomputer.c`: includes for `ftremixaisize.h` and `ftremixaicloud.h`; hooks listed below.
- `tests/remix/test_ai_cloud_rules.c` (new, entry `remixTestAiCloudRules`, 9 checks), `tests/remix/test_ai_combat_rules.c` (+7 recovery checks, 27 total). Lead registers the new file in tests/remix/CMakeLists.txt and test_main.c.

Hooks (asm patch -> decomp function, line in ftcomputer.c):
- P02 size_multiplier_check_ -> ftComputerCheckDetectTarget (~4154): the four detect ranges of each attack are multiplied by `ftRemixAiSizeDetectScale` (table value of the player for non-giant fighters, 1.0F for giant DK) before the vanilla giant DK 1.4F block. Multiplying by 1.0F is exact, so the non-scaled case is unchanged.
- P16 usp_check_ -> ftComputerUpdateInputs air branch (~3617, ~3632): the `fkind != Fox` clause is replaced by the Fox family (Fox, JFox, Peppy, Falco); the `dist_y < 0` zeroing is skipped for Goemon/Ebisumaru cloud actions (asm jumps past the zeroing block).
- P17 recovery_stick_y_fix -> ftComputerUpdateInputs after the command loop (~3729), inside the `input_wait == 0` block.
- P06 fix_remix_recovery -> ftComputerFollowObjectiveRecover (~6741, ~6761): Pikachu vanilla block gated by the classic/toggle rule; Wolf USP target; NSP charge cancel (Sheik, DSamus, Peppy, Mewtwo) returns before the walk and the custom recovery.

Deviations and inferences (medium confidence, check in match):
- P16 Goemon/Ebisumaru jump target (vanilla 0x801321A4) is inferred from the structure: the instruction after the vanilla zeroing block. No vanilla disassembly was available in this session (no ROM), so this is not verified byte by byte.
- P16 Peppy: the asm comment says "Goemon usp action check" but the code branches to the Fox path. The port follows the code.
- P16 and P17 disagree on Magic Cloud Jump (0xE1): P16 includes it for Goemon, P17 does not (asm). Kept as in the asm, both lists are in ftremixaicloudrules.c.
- P17 runs once per frame after the command loop. The asm runs at every return to 0x8013253C (after each command). Final stick and button state is the same when it is idempotent, which it is; it is skipped on COMMAND_END (the asm does not reach 0x8013253C there either).
- P17 byte writes (`sb` into the button halfword high byte) are emulated with `ftRemixAiCloudReplaceButtonHighByte`, which clears the other high-byte buttons as the asm does.
- Fighter field offsets from the asm (not checked with a layout dump, FTStruct did not dump): status_id = action 0x24, `input.cp.button_inputs` = 0x1C6, `input.cp.stick_range.y` = 0x1C9, `coll_data.floor_line_id` = 0xEC (-1 = no floor line above), `computer.target_pos` = 0x1CC+0x60 (x) and +0x64 (y).
- P06 special cases: the asm jumps to 0x80137FBC+4 for the NSP cancel, taken as "skip walk and custom recovery" (0x80137FBC is the walk call that P07 replaces).
- P06 command 0x09 is `nFTComputerInputStickSmashAutoXButtonB` (the NSP towards the target), not a plain "PRESS B" as the asm comment says. Only relevant for Banjo, which is not ported.

REMIX-TODO(AI) left:
- P05 fix_target_pos_x: rule `ftRemixAiRecoveryClampTargetX` exists. Hook point: ftComputerFollowObjectiveWalk, first branch, right after the `ftcom_flags_0x4A_b1` floor-edge clamp and before the final `ftComputerSetCommandImmediate(fp, nFTComputerInputMoveAuto)`. Not placed: the blastzone bounds (asm reads halfwords at +0x78 right and +0x7A left from the pointer at 0x80131300, the ground header) have no named field in the decomp or the port.
- P06 Banjo flight (action 0xEF, frame 21, B press into the halfword at +0x1BE, then command 0x09 when floor_line_id is -1): the field names for +0x1BE and the frame at +0x1C are not confirmed.
- P02 for the Remix 1P automatic improved AI is not involved (no toggle read in P02).

EXTRA_SYMBOLS: none (no new REMIX_SYM_ used).

How to see it in a match (CPU level 9 unless noted):
- P02: give the CPU a Super Mushroom (size multiplier > 1.0). Its attacks should be picked from a larger detect range than vanilla; a Giant DK CPU is unchanged.
- P16: CPU Goemon in Magic Cloud over the stage. The stick should stay up (no zeroing) while the cloud is active. CPU Fox, Falco, JFox or Peppy should take the Fox USP branch.
- P17: CPU Goemon or Ebisumaru recovering with the Magic Cloud should always hold the stick up. Ebisumaru presses A (nair) once above a ledge or floor line; Goemon presses Z.
- P06: CPU Sheik, DSamus, Peppy or Mewtwo in their NSP charge while recovering should press Z (cancel the charge). CPU Wolf in his USP should aim straight up. With "Improved AI" on, CPU Pikachu family should use the Remix recovery; with it off or in 1P/RTTF, the vanilla Pikachu recovery.

## Slice B1

Escopo: AI.asm lv10 P02 a P09 (patches_b). P01 fica com A1.

Arquivos criados:
- `decomp/src/ft/ftremix/ftremixailv10.{c,h}`: hooks P03, P04, P05, P07, P08/P09.
- `decomp/src/ft/ftremix/ftremixailv10rules.{c,h}`: regras puras (ints), constantes de rotina e de input.
- `decomp/src/ft/ftremix/ftremixaipostprocess.{c,h}`: P06 (sub-comportamentos genericos).
- `tests/remix/test_ai_lv10_rules.c`: entrada `remixTestAiLv10Rules`, 13 checks. Rodado standalone com `PORT=1 REGION_REMIX=1`: 13 checks, 0 falhas. Falta registrar no `tests/remix/CMakeLists.txt` e em `test_main.c` (o lead).

Arquivos editados:
- `decomp/src/ft/ftcomputer.c`: includes (REGION_REMIX); `ftComputerGetInputScript` deixou de ser `static` (P06 precisa comparar `p_command`); hooks:
  - P03 `ftComputerCheckEvadeDistance` (0x80132BC8): o primeiro operando do teste de star vira `ftRemixAiEvadeCountsEnemyInvulnerable`.
  - P08/P09 `ftComputerCheckSetTargetEdgeRight` (0x80134000) e `Left` (0x80134368): `vel_air.y >= 0` ganha `&& ftRemixAiSkipsUpwardDriftTarget(fp) == FALSE`.
  - P04 `ftComputerGetObjectiveStatus` (0x80136D0C), ramo CliffWait final `else`: `ftRemixAiCliffGetupCommand`.
  - P05 mesmo função, caso de taunt (`nFTComputerInputStickNButtonL`): pulado para lv10.
  - P07 `ftComputerProcessTrait` (0x8013A63C): `switch (ftRemixAiTraitRoll(fp))`.
  - P06 `ftComputerProcessAll` (0x8013A834): `ftRemixAiPostProcess(fp)` logo depois de `ftComputerProcessObjective`, dentro do bloco `input_wait == 0`. O asm substitui a chamada `jal 0x8013A38C`, que é `ftComputerProcessObjective`, não `UpdateInputs`.
- `decomp/src/ft/ftcomputer.h`: prototipo de `ftComputerGetInputScript`.
- `decomp/src/ft/ftcommon/ftcommoncatch1.c`: so comentario REMIX-TODO(AI) para P02.

Patch a patch:
- P02 auto_jab_grab_ (0x80149E7C, `ftCommonCatchCheckInterruptAttack11`): REMIX-TODO(AI). O asm faz verificacao de poligono (0x80149E84) antes do grab e esse codigo vanilla nao esta no decomp nem no asm Remix. Hook nao ligado.
- P03 check_invulnerable_: feito. Inimigo conta como Starman se tem star ou (lv10 e `invincible_tics != 0`). Assumido: o teste de distancia 1500 continua valendo para respawn (o asm pula para 0x80132C90, que presumo ser esse teste).
- P04 extend_cliff_attack_: feito, com desvio. O asm faz `get_random_int_(2)` seguido de `srl 1`, o que sempre da 0, entao lv10 sempre usa CLIFF_LET_GO. Portei o comportamento literal do asm. A intencao (50/50 entre normal e salto) nao esta clara no codigo, entao nao corrigi.
- P05 taunt_replace_: feito. lv10 nunca envia o taunt. O ramo USP de Ness/Lucas e codigo morto (o `b _no_input` anterior retorna antes) e nao foi portado.
- P06 cpu_post_process: parcial. Portados os sub-comportamentos genericos: START mash para morto (gateado por `improved_ai`, sem gate de nivel), cancelamento de dash attack (sem gate de nivel), jump-cancel de USMASH/USP correndo/dashando (lv10), USP para USP_TOWARDS (lv10). REMIX-TODO(AI) para a tabela `Character.cpu_post_process` por lutador. Lutadores com entrada: Falco, Fox/JFox (foxshared), Captain Falcon/JFalcon (captainshared), Link/JLink/ELink/YLink (linkshared), DK/JDK (dkshared), Bowser, GBowser, Wario, Marth, Dedede, Wolf, Ganondorf, Mewtwo, Roy, Marina, Sheik (teleport), DragonKing, Jigglypuff/JPuff/EPuff/Kirby/JKirby, Pikachu/EPika/JPika, Yoshi/JYoshi.
- P07 force_objective_attack: feito. lv10 forca comportamento 0 e nao chama o RNG.
- P08/P09 _fix_weird_air_drift_right/left: feito, com inferencia. Assumi que o bloco pulado pelo asm (0x80134000+0x98) e o `if (vel_air.y >= 0)` que seta o alvo para si mesmo +-500 em X. Confirmar no jogo.

Ids de rotina customizada (0x31 + indice de add_cpu_input_routine em AI.asm 1603-1913): CLIFF_LET_GO 0x41, DASH_ATTACK 0x48, USP_TOWARDS 0x49, JUMPCANCEL_UPSMASH 0x4B, JUMPCANCEL_USP 0x4C, NULL 0x4D. Divergencia: a nota A2/A3 diz SMASH_DSP=0x41, mas a contagem aqui da CLIFF_LET_GO=0x41 (17o add) e SMASH_DSP=0x42. Lead: conferir antes de fechar.

EXTRA_SYMBOLS: nenhum. Usa `REMIX_SYM_Toggles__entry_improved_ai` (ja existe) via `gmRemixToggle(improved_ai)`.

REMIX-TODO(AI) deixados: P02 (poligono); P06 por lutador (tabela cpu_post_process); lv10 CliffWait com srl (intencao, nao corrigida).

Como ver na partida (SSB64_VS_CPU_LEVEL=10 no vs.sh):
- P03: CPU lv10 perto de inimigo recem-respawnado (invencivel por ~20 tics), deve fugir dele.
- P04: CPU lv10 na borda: sempre tenta CLIFF_LET_GO (salto para fora), nunca o getup normal.
- P05: CPU lv10 nunca faz taunt (ex.: Ness, Lucas).
- P06: CPU lv10 correndo/dashando com USMASH ou USP: deve sair em jump-cancel; com improved_ai ligado, CPU morto deve apertar START (Start na tela de respawn).
- P07: CPU lv10 nao muda de comportamento (fica sempre em atacar).
- P08/P09: CPU lv10 subindo no ar nao deve ir para o alvo deslocado ao lado; deve buscar a borda.

## Slice B2

Scope: AI.asm P10 to P18 (part B, lv10 movement and attack range), P18 equal to c.P2. Vanilla addresses were checked with the
US ROM (`baserom.us.z64`, ROM = RAM - 0x800855C0 in this overlay) and rabbitizer, not only with the patch comments.

Files created:
- `decomp/src/ft/ftremix/ftremixaimovementrules.{c,h}`: pure rules (int/float): fastfall gate, shorthop input, neutral jump roll, X and Y clamp, offstage gate.
- `decomp/src/ft/ftremix/ftremixaipredictionrules.{c,h}`: pure rules: attack view base, running candidate filter, my X with friction, delta X, falling Y.
- `decomp/src/ft/ftremix/ftremixaimovement.{c,h}`: game side of P11, P12, P13, P14, P15.
- `decomp/src/ft/ftremix/ftremixaiprediction.{c,h}`: game side of P16, P17, P18.
- `tests/remix/test_ai_movement_rules.c` (entry `remixTestAiMovementRules`) and `tests/remix/test_ai_prediction_rules.c` (entry `remixTestAiPredictionRules`).
  The lead registers both in `tests/remix/CMakeLists.txt` and `test_main.c`. Standalone run with `-DPORT=1 -DREGION_REMIX=1`: 42 checks, 0 failures.

Files edited: `decomp/src/ft/ftcomputer.c` (all under `PORT && REGION_REMIX`, vanilla path unchanged when compiled out):
- includes of `ftremixaimovement.h` and `ftremixaiprediction.h` after `ftremixailongrange.h`.
- P12 `ftComputerCheckFindTarget` (0x8013295C): `ftRemixAiMovementTargetsOffstage(this_fp) ||` as the first operand of the on-stage/cliff group (~line 3821).
- P13 same function, store of the nearest target (0x80132AE4 store): `ftRemixAiMovementClampTarget(this_fp, &com->target_pos)` (~line 3853).
- P11 `ftComputerFollowObjectiveWalk` (0x80134E98, "Fast fall?" block, asm 0x80135704): the WaitShort(0x27) is taken only when `ftRemixAiMovementFastfallPermitted(fp)` (~line 5367). When denied the function continues to the switch, as the asm `_skip_fastfall` does.
- P14 same function, grounded branch `target_line_id != floor_line_id` / `y < target_pos.y` (asm 0x80135B18): the command passes through `ftRemixAiMovementShorthopInput` (~line 5494).
- P15 same function, grounded branch `dash_predict <= distance` and not below the target (asm 0x80135ABC): `ftRemixAiMovementNeutralInput(fp)` (~line 5480).
- P16 `ftComputerFollowObjectiveAttack` (0x801392C8), first state (asm 0x80139374 `lwc1 f4,0x68(v1)`, `add.s f16,f8,f10`): base from `ftRemixAiPredictionAttackViewBase` (~line 7451).
- P17 same function, second state (asm 0x801398A0): same change (~line 7587). Site choice by address order, medium confidence.
- P18 `ftComputerCheckDetectTarget` (0x80132EC8): candidate filter `else if (!ftRemixAiPredictionCandidateAllowed(...)) continue;` (~line 4119); after the vanilla predict_pos_x/y chain, `ftRemixAiPredictAttackPosition(...)` overrides the two outputs for level 10 (~line 4157).

Patch status:
- P10 `_improve_recovery_drift` (asm 4343-4791 with helpers 4063-4329): NOT PORTED. REMIX-TODO(AI). Reason: about 450 asm lines of ledge search over `gMapLineTypeGroups`, stack-slot hand-offs (`0x44(sp)`, `_find_closest_ledge` returns a stack address), and the f20 callee-saved register. A partial hook would change lv10 behaviour, so nothing is applied. Needs its own pass with the map line API.
- P11 prevent_fastfall: ported. Level 10 clears the flag, denies on used jumps, offstage opponent (target floor_line_id -1) or floor_dist >= -2000.
- P12 detect_offstage_opponents: ported as "skip the on-stage/cliff group for level 10". Deviation: the asm jumps to 0x8013295C+0x15C (0x80132AB8), which is the delay slot of the distance `bc1f`, so it skips the distance computation and would store a stale f12 (distance) and the stale `f24`. The port keeps the distance computation.
- P13 clamp_target_pos_to_stage: ported with two corrections (deviations 1-2 below). Y clamp as written.
- P14 shorthop_followup_attack: ported. Gap = target_pos.y + target physics.vel_air.y - CPU Y. Thresholds 300/600/900 (inclusive), above 900 keeps the vanilla command 4.
- P15 jump_in_neutral: ported with the port RNG (`syUtilsRandIntRange(200)`, called once per frame for level 10 in this state, as the asm does).
- P16 and P17 extend_attack_check_range(_2): ported, base 2000 at level 10, 1200 otherwise.
- P18 fix_movement_prediction (= c.P2): ported. Running filter, my X with ground friction, target X, delta X, falling Y for both bodies. Deviations 3-4 below.

Deviations and asm bugs (decide in review):
1. P13 bounds: the asm reads `0x28(s3)` and `0x2C(s3)` as "left/right ledge". With `s3 = gMPCollisionBounds` (MPAllBounds: start, stop, current, diff, 16 bytes each, MPBounds = top, bottom, right, left), `current` starts at 0x20, so 0x28 = current.right and 0x2C = current.left. The port uses current.left and current.right with the intent of the comments.
2. P13 sense: `bc1fl` after `c.le.s` clamps when the target is INSIDE (the mov.s runs when the condition is true). The port clamps only beyond the edges (target < left - 300, target > right + 300), as the comment "if not offstage, skip" says. Behaviour with 300 padding outward, as the asm padding sign.
3. P18 X: the asm does not multiply the predicted X by `this_fp->lr`, but the vanilla line does (4099) and the detect boxes are flipped by lr. The port keeps `* lr`. Check a left-facing CPU in game.
4. P18 friction: the asm masks the ground word with 0xFFFF00FF and uses it as a table index with no bound. The port uses `grRemixSurfaceFriction(floor_flags & MAP_VERTEX_MAT_MASK) * attr->traction`, the same expression as ftphysics.c:177. Target air Y speed is read from `physics.vel_air.y` (asm offset 0x4C, assumed).
5. P14: `f6` (my Y) comes from the joint translate (the asm expects the caller to set it).
6. P12 and P13 read the CPU level from `this_fp->level` (asm 0x13(s1)); equal.

ID note (conflict for the lead, not edited here): AI.asm has 28 `add_cpu_input_routine` calls (lines 1603 to 1913), ids = 0x31 + order. Order gives SHORT_HOP_TOWARDS 0x31, SHORT_HOP_IN_PLACE 0x33, FULL_C_JUMP_TOWARDS 0x34, SHIELD_DROP 0x3C, CLIFF_LET_GO 0x40, SMASH_DSP 0x41 (agrees with the A3 notes), DASH_ATTACK 0x47, USP_TOWARDS 0x48, NULL 0x4C.
`decomp/src/ft/ftremix/ftremixailv10rules.h` has DASH_ATTACK 0x48 and CLIFF_LET_GO 0x41, and `ftremixairollrules.h` has SHIELD_DROP 0x37. Those look off by one or more. The new code uses its own constants (`FTREMIX_AI_MOVEMENT_INPUT_*`, `FTREMIX_AI_PREDICTION_INPUT_DASH_ATTACK 0x47`) and the lead should verify with the built routine table.

EXTRA_SYMBOLS: none. Input ids are the same as the custom routine ids of A2 (`ftComputerGetInputScript`).

How to see it in a match (CPU level 10: `SSB64_VS_CPU_LEVEL=10` with the vs.sh shortcut, use any two fighters with the vs.sh shortcut; the fighter ids were not checked in this slice):
- P11: CPU falling below the stage with all jumps used should not fastfall; falling far above ground should.
- P12/P13: CPU should go after an opponent knocked offstage and stop short of the blastzone (edge - 300 at most).
- P14: CPU below a target that floats above should short hop more often.
- P15: about 1% short hop, 0.5% in place, 1% full C jump while walking to a target (grep the log if added).
- P16/P17: CPU attacks targets up to 2000 units away.
- P18: CPU running (Dash) only picks grab, upsmash, up special ground, neutral special ground and dash attack.

REMIX-TODO(AI): P10 (recovery drift). P12 and P13 are approximations as described in deviations 1-2; the P12 aerial exclusion of the asm is not reproduced (the decomp has no aerial test in this function).


## Slice A4c

Escopo: P08a/P08b (random_teching_), P09 (z_cancel_), P11 (improve_remix_specials), P12 (improve_remix_charged_NSP_1), P13 (improve_remix_charged_NSP_2). P14 ficou REMIX-TODO(AI). P01-P07, P10, P15 e P16 ou acima não tocados.

Achados de mapeamento (confirmados no decomp):
- 0x80140980 = `ftCommonDamageAirCommonProcMap`+0x44 e 0x801435F4 = `ftCommonDamageFallProcMap`+0x44 (docs/remix/gameplay_a_l.md:91-92). Os dois sites são a cadeia de tech mais o bounce: `tech_roll_og_` = `ftCommonPassiveStandCheckInterruptDamage`, `tech_in_place_og_` = `ftCommonPassiveCheckInterruptDamage`, `tech_fail_` (0x80144498) = `ftCommonDownBounceSetStatus`.
- Como `tech_fail_` é o bounce, o hook substitui a cadeia inteira e o bounce; após sucesso o caminho sai sem bounce (a `bnezl ... _end` do original). Não há bug de sobreposição.
- O ZCancel.asm `_cpu_z_cancel` consome o `at` do AI; o consumidor é `ftRemixZCancelResult` (ftremixzcancel.c).
- P11 e P12 são extensões do switch vanilla: os ids Remix entram por rótulos `case` (DK, Samus) ou por `default`, sem perder os casos vanilla.

Arquivos criados (decomp/src/ft/ftremix/):
- `ftremixaiteching.{c,h}`: `ftRemixAiTechIsHandled(GObj*)` e `ftRemixAiShouldZCancel(FTStruct*)`.
- `ftremixaiteachingrules.{c,h}`: regras puras (rolls 30/30/30 e 0-99, Single Button Mode, gates do z-cancel 95%).
- `ftremixaispecials.{c,h}`: `ftRemixAiSpecialsCancelChargeWithZ(FTStruct*)` e `ftRemixAiSamusNspIsBlocked(FTStruct*, FTStruct*)`.
- `ftremixaispecialsrules.{c,h}`: listas de ações de cancelamento por fighter e regra de bloqueio do oponente.
- `tests/remix/test_ai_teaching_rules.c` (entrada `remixTestAiTeachingRules`) e `tests/remix/test_ai_specials_rules.c` (entrada `remixTestAiSpecialsRules`). Rodados standalone: 47 checks, 0 falhas.

Arquivos editados:
- `ft/ftcommon/ftcommondamage.c`: include e P08a. A cadeia de tech em ftCommonDamageAirCommonProcMap virou `ftRemixAiTechIsHandled(...) == FALSE`.
- `ft/ftcommon/ftcommondamagefall.c`: include e P08b. O `else if` de ftCommonDamageFallProcMap usa o mesmo hook.
- `ft/ftremix/ftremixzcancel.c`: include e, no topo de `ftRemixZCancelResult`, a rolagem do CPU (P09). Sucesso do AI vira SUCCESS, ou GLIDE se a opção for Glide.
- `ft/ftcomputer.c`: include de `ftremixaispecials.h`; P11 (JDK no case DK de ftComputerCheckTryChargeSpecialN; MTWO, DSAMUS, ESAMUS, JSAMUS no case Samus) e REMIX-TODO(AI) do Sheik/Peppy/Bowser; P12 (JDK no case DK de ftComputerCheckTryCancelSpecialN e `default` com `ftRemixAiSpecialsCancelChargeWithZ`); P13 (em func_ovl3_80138AA8 o teste Ness/Fox vira `ftRemixAiSamusNspIsBlocked`).

Hooks e mapeamento asm -> decomp:
- P08a -> ftCommonDamageAirCommonProcMap (ftcommondamage.c, comentário 0x8014093C). P08b -> ftCommonDamageFallProcMap (ftcommondamagefall.c, comentário 0x801435B0).
- P09 -> ftRemixZCancelResult (ftremixzcancel.c), chamado por ftRemixZCancelAttackAirLanding a partir de ftCommonAttackAirProcMap (comentário 0x80150A08).
- P11 -> ftComputerCheckTryChargeSpecialN (ftcomputer.c, 0x80136A20).
- P12 -> ftComputerCheckTryCancelSpecialN (ftcomputer.c, 0x80136C0C).
- P13 -> func_ovl3_80138AA8 (ftcomputer.c, 0x80138AA8), dentro do teste aleatório existente.

Desvios do asm (todos reportados):
1. P08: Single Button Mode só bloqueia CPUs. O asm aplica a todos, mas o comentário diz "don't let CPU tech". Sandbag continua valendo para qualquer fighter.
2. P08: contador de techs falhas (VsStats.tech_miss_tracker) não portado.
3. P08: Training.entry_tech_behavior não portado; treino usa o roll padrão (índice 0).
4. P08: flag de off-stage (N64 0x18C bit 4) não mapeado; todo CPU é tratado como em palco.
5. P08 e P09: flag Remix 1P não existe no port; tratado como FALSE.
6. P09: no ftRemixZCancelResult, `DISABLED` para o AI vira SUCCESS (o asm deixa at=1 nessa opção). Para humanos o código existente retorna FAIL. Suspeita para o dono do ZCancel, não alterada.
7. P12: removido o segundo teste duplicado de DSamusNSPChargeAir. Os valores de Sheik (NSPG_CHARGE 0xEB, NSPA_CHARGE 0xEE) vêm dos comentários do P14, não de Sheik.asm: validar.
8. P13: Ness passa a ser liberado quando o próprio fighter é Sheik (o vanilla bloqueava sempre). Lucas e JNess seguem a mesma regra. Mudança de comportamento intencional do asm.
9. P12: o asm retorna 0 para todo id não listado, o que apagaria os casos vanilla. O port cai no switch vanilla (default com break), que é o comportamento de extensão.

REMIX-TODO(AI) restantes: P14 (Kirby com hat Samus/Sheik/Mewtwo/DSamus, a0 sobrescrito, mapeamento de 0x80138C44-0x80138CB8 não verificado); P11 Sheik/Peppy (inclui Samus B press com needle < 6) e Bowser/GBowser (USP zera stick X, offset 0x1C8 não confirmado); P08 contador, training menu, off-stage e Single Button Mode; P09 consumidor com Single Button Mode; P13 Kirby hat (P14).

EXTRA_SYMBOLS: nenhum (nenhuma leitura da imagem Remix).
Registro necessário pelo lead: `remixTestAiTeachingRules` e `remixTestAiSpecialsRules` em tests/remix/test_main.c e tests/remix/CMakeLists.txt.

Como ver na partida (CPU nível 9 salvo indicado, com "Improved AI" ligado):
- P08: CPU de qualquer fighter, derrubado no chão ou caindo de um hit, deve teclar com frequência (roll 30/30/30, falha em 10%). Sandbag nunca tecla. Nível 10 segue o mesmo caminho.
- P09: CPU nível 10 (ou nível 9 com o toggle) em landing de ataque aéreo: z-cancel de aproximadamente 95%. Não ocorre na tela How to Play nem na intro DK x Samus.
- P11: CPU DK (JDK) e Samus, DSamus, ESamus, JSamus, Mewtwo carregam NSP como o vanilla.
- P12: CPU DSamus, Sheik ou Mewtwo segurando NSP carregado: solta a carga com Z.
- P13: CPU Samus não dispara NSP contra Fox, Falco, Wolf, Slippy, Marina, Piano ou JFox; contra Ness, Lucas e JNess só se a CPU não for Sheik.

## Slice D3

Scope: AI.asm P05 fix_target_pos_x (A4b leftover), P06 Banjo flight (A4b leftover), P10 _improve_recovery_drift (B2 leftover).
Result: P05 ported. P06 Banjo and P10 NOT ported (REMIX-TODO(AI)), vanilla path kept for both.

Files edited:
- `decomp/src/ft/ftremix/ftremixairecovery.c`: includes `mp/mpdef.h`, `mp/map.h`; new `ftRemixAiRecoveryClampWalkTargetX(FTStruct *fp)`.
- `decomp/src/ft/ftremix/ftremixairecovery.h`: declaration of the above.
- `decomp/src/ft/ftcomputer.c`: P05 hook in `ftComputerFollowObjectiveWalk` (~line 5445), first branch, after the `ftcom_flags_0x4A_b1` floor-edge clamp and before `ftComputerSetCommandImmediate(fp, nFTComputerInputMoveAuto)`. Under `PORT && REGION_REMIX`.

P05 (asm 0xAFDBC / 0x8013537C -> ftComputerFollowObjectiveWalk, 0x80134E98+0x4E4):
- Bounds source: `gMPCollisionGroundData->map_bound_left` and `map_bound_right` (s16). Offset check: PORT `MPGroundData` layout places `map_bound_right` at 0x78 and `map_bound_left` at 0x7A, which is the asm `lh 0x78/0x7A`. Ground data pointer = word at 0x80131300 (consistent with grremixhazards_onett.c and grremixmetallic.c notes). Not verified in a running match.
- Clamp: `ftRemixAiRecoveryClampTargetX` (existing rule, covered by tests/remix/test_ai_combat_rules.c), padding 200.0F, left bound first.
- Gate: `gmRemixToggle(improved_ai)` only, no level gate, no Remix 1P (as the asm reads `Toggles.entry_improved_ai`).
- Deviation: the asm `lh` is a signed halfword, same as the s16 field. No other deviation.
- Not verified: that the stage's ground data pointer always equals the asm word at 0x80131300 at the time the walk runs.

Field offsets: the PORT FTStruct layout does NOT match the N64 asm offsets (`status_id` is 0x2C in the port, asm uses 0x24; `level` is 0x1B, asm uses 0x13). Raw asm offsets must be mapped by named field, never copied. Measured with `_Static_assert` probes on the decomp headers (scratchpad, not committed).

P06 Banjo flight (asm 0xB298C / 0x80137F4C, the `_banjo_flight` block):
- Not ported. Button write target is `fp->input.cp.button_inputs` (the B mask, 0x4000). Port offset measured: 0x1E2 (not 0x1BE, not 0x1C6).
- Frame: asm `lw 0x1C(a0)` compared to integer 21. No named integer frame field found in FTStruct; `fighter_gobj->anim_frame` is a float and its owner is unconfirmed. Porting on a guessed frame would press B at the wrong frame. REMIX-TODO(AI) with the frame field name needed.
- Command 0x09 (PRESS B) only when `coll_data.floor_line_id == -1`.

P10 _improve_recovery_drift (asm 0xAFE40 / 0x80135400, body 4343-4791):
- Not ported. Read so far (lines 4337-4440): gate (lv10, FallSpecial action, all jumps used, behavior == Recover 0x4 at ftcom+0, `floor_line_id` == -1 with LEDGE_PADDING 300.0 and gMapEdgeBounds 0x80131308). `_improved_logic` then calls `_find_closest_ledge` (left, then right) which returns the ledge position in a stack slot (`v0` points to sp+0x28) and stores it into `0x1CC+0x4C/0x50`. Not ported: the ledge search, the stack-slot hand-off, the f20 register (`_check_ground_below`) and the `0x44(sp)` caller slot are unresolved, so a partial hook would change lv10 recovery.

REMIX-TODO(AI) left: P06 Banjo (frame field and button offset), P10 (ledge search and handoffs, lines 4337-4791).

EXTRA_SYMBOLS: none.

How to see P05 in a match: CPU level any, improved AI toggle ON, a fighter off the stage edge; the recovery walk target should stop 200 units inside the blastzone-side ground bounds (no walk toward the blast line). Use `SSB64_VS_CPU_LEVEL=10` with the vs.sh shortcut and a stage with a short left/right map bound to see the clamp.

## Slice D4

Escopo: seletor de nivel CPU do VS CSS ate 10 (P12, P13, P14, P15, P16 de `f13_ai_patches_c.md`) e os dois consumidores
de CPU de configuracao (Default CPU LVL, Single Button Mode). Mapeamento do overlay confirmado por leitura: o trecho
0x80135848-0x80136FF0 e `mnplayersvs.c` (`mnPlayersVSCheckHandicapArrowInRangeAll`, `mnPlayersVSArrowThreadUpdate`,
`mnPlayersVSMakeHandicapLevelValue`), a leitura do asm confirma os mesmos digitos e limites.

Arquivos criados:
- `decomp/src/mn/mnplayers/mnremixcpulevelrules.{c,h}`: regras puras (`mnRemixCpuLevelCanRaise`,
  `mnRemixCpuLevelNeedsTens`, `mnRemixCpuLevelUnitsDigit`), so int.
- `decomp/src/mn/mnplayers/mnremixcpulevel.{c,h}`: CSS (`mnRemixVSCanRaiseArrowValue`, `mnRemixVSIsArrowRightLimit`,
  `mnRemixVSIsCPULevelTens`, `mnRemixVSMakeCPULevelTens`).
- `tests/remix/test_ai_cpulevel_rules.c` (entry `remixTestAiCpuLevelRules`). Registro em `tests/remix/CMakeLists.txt` e
  `test_main.c` pendente para o lead. Rodado no host com `cc -DPORT -DREGION_REMIX`: 8 checks, 0 falhas.

Arquivos editados:
- `decomp/src/mn/mnplayers/mnplayersvs.c`: include de `mnremixcpulevel.h`; quatro hooks, cada um em bloco
  `#if defined(PORT) && defined(REGION_REMIX)` com `#else` vanilla:
  - P12/P14 (limite da seta direita) em `mnPlayersVSCheckHandicapArrowInRangeAll` (antes `*value < 9`).
  - P13/P14 (visibilidade da seta direita) em `mnPlayersVSArrowThreadUpdate` (antes `value == 9`).
  - P15/P16 (digitos "10") em `mnPlayersVSMakeHandicapLevelValue`, antes do desenho do digito unico.
- `decomp/src/ft/ftcomputer.c`: include de `gm/gmremixtoggle.h` no bloco REGION_REMIX existente; em
  `ftComputerUpdateInputs`, os quatro casos de press (`FTCOMPUTER_COMMAND_BUTTON_A_PRESS`, `B_PRESS`, `Z_PRESS`,
  `L_PRESS`) passam por `ftRemixSingleButtonCpuAOrBPress` / `ftRemixSingleButtonCpuZOrLPress` com
  `gmRemixToggle(single_button_mode)`. Resolve a nota "not resolved" de `f8_speed_sbm.md` para os quatro patches
  `handle_cpu_inputs_` (AI.asm/SingleButtonMode.asm 0xAC73C, 0xAC764, 0xAC784, 0xAC7A4 = os casos de press acima).

Syntax check: `scripts/remix/syntax-check.sh` OK em `mnremixcpulevel.c`, `mnremixcpulevelrules.c`, `mnplayersvs.c`
e `ftcomputer.c`. Nenhum build do jogo feito (regra do brief).

Desvios do asm:
- O asm nao da volta (wrap): o seletor satura em 10 e em 1. Nao foram criadas funcoes next/previous; a seta esquerda
  continua a vanilla (`*value > 1`), que ja satura em 1.
- Digitos "10": o asm desenha o "1" e o "0" com sprites do arquivo 0 (offset `0xC4A4`) e o footer `t9`. A port usa os
  sprites `llMNCommonDigit1Sprite` / `llMNCommonDigit0Sprite` do mesmo `sMNPlayersVSFiles[1]` do digito vanilla, com
  posicoes x = player*69+67 e +75 e y = 200, como o asm (`0 = 1.x + 8.0`).
- P13 (redirect da seta esquerda, 0x80136AC4) nao foi portado como `b`: a funcao C atualiza a visibilidade no mesmo
  lugar, sem redirect.
- Human handicap: mantido o limite vanilla 9. O asm estende para 40 (P12/P14 ramo HMN, icones P15 para >= 10). Nao
  portado: `REMIX-TODO(AI)` em `mnremixcpulevel.c` (`MNREMIX_VS_HANDICAP_VANILLA_MAX`).

REMIX-TODO(AI) restantes:
- Default CPU LVL, refresh no A (`set_cpu_level`, Toggles.asm:1740). E um callback de entrada do menu de toggles
  (`entry_default_cpu_level`, Toggles.asm:2403), chamado pelo despacho do menu em asm. A port nao tem despacho C desse
  menu, entao nao ha ponto de hook. Efeito: trocar o toggle no menu so vale no proximo boot
  (`gmRemixDefaultCPULevelSetup`, `gmremixspawnsetup.c`).
- Handicap humano ate 40 (acima).

Interacao com a port: o enhancement `CpuLevel9` (`mnplayersvs.c`, perto de `port_enhancement_cpu_level_9()`) forca o
nivel 9 e sobrescreve o 10 quando ligado. Nao foi alterado.

Nenhum `EXTRA_SYMBOLS` novo: os sprites usados ja existem (`llMNCommonDigit0Sprite`, `llMNCommonDigit1Sprite`), e os
helpers de SBM ja estavam declarados em `ftremixhooks.h`.

Como ver no jogo (a cargo do lead, nao rodado aqui):
- Build: `cmake --build build --target BattleShip-Remix -j 4`, depois `.claude/skills/remix-smoke-tests/scripts/css.sh
  <input> <nome>` (sobe direto no VS CSS, cena 16). Falta um script de input que leve o cursor ao painel CPU e aperte a
  seta direita 10 vezes; os `css_*.txt` existentes nao fazem isso.
- Checagem: com o painel CPU em 10, o digito "10" aparece, a seta direita some; ao iniciar a partida, `fp->level` do CPU
  deve ser 10 (o valor vai por `gSCManagerTransferBattleState.players[].level`, `mnplayersvs.c` ~5233).
- Single Button Mode: com o toggle em 1 (A), os CPUs enviam so A; com 3 (R) enviam A+Z.

## Slice D1

Escopo: P8/P9 `stop_roll_spam_` (AI.asm 6039-6307) e P11 `cpu_di_` (AI.asm 6533-6812). Resultado: P8/P9 portado (parcial, ver desvios); P11 NÃO portado (REMIX-TODO(AI)).

Arquivos editados (decomp):
- `ft/ftremix/ftremixairollrules.{c,h}`: constantes de `stop_roll_spam_` e regras puras `ftRemixAiRollSpamApplies`, `ResetsStick`, `IsAdvantageAction`, `AdvantageCommand`, `NeutralCommand`.
- `ft/ftremix/ftremixairoll.{c,h}`: `ftRemixAiStopRollSpam(fp, roll_command)` (sb32, TRUE = comando emitido, FALSE = skip). Helpers estáticos: comando, sorteio, alvo. Marcador REMIX-TODO atualizado. TSDoc de `ftRemixAiDownCommand` corrigido: SHIELD_DROP é 0x3C (não 0x37).
- `ft/ftcomputer.c`: hook no fim de `func_ovl3_80138EE4` (bloco de roll `nFTComputerInputEscapeL/R`, por volta da linha 7438), dentro de `#if defined(PORT) && defined(REGION_REMIX)`; o `#else` é o bloco vanilla intacto. Ponto de hook confirmado pela ordem de endereços: 0x80138EE4 < 0x80139294 < 0x801392C8. Marcador REMIX-TODO(AI) para P11 acima de `ftComputerProcessObjective` (0x8013A38C).
- `tests/remix/test_ai_roll_rules.c`: +17 checagens de P8/P9 (total do arquivo: 33, 0 falhas standalone).

Hook P8/P9 (AI.asm -> decomp):
- 0x80139294 / 0x801392A4 (`stop_roll_spam_`) -> `func_ovl3_80138EE4`, roll L/R. Gate: Giga Bowser (REMIX_ID_GBOWSER) em qualquer nível, ou nível bruto >= 10. Abaixo disso: roll vanilla.
- Ação do CPU > TurnRun (0x13, comparação com sinal): RESET_STICK (0x00).
- Alvo com ação de vantagem (0x25 a 0x37, 0x43, 0x44, 0xA4 Stun, 0xA5 Sleep): sorteio de 16; 0 = SHORT_HOP_IN_PLACE (0x33), 1 = SHORT_HOP_TOWARDS (0x31), resto = skip.
- Sem vantagem (ou sem alvo): sorteio de 16; 0 = roll vanilla, 1 = 0x33, 2 = 0x31, 3 = SHORT_HOP_AWAY (0x32), 4 e 5 = PIVOT_AWAY (0x37), resto = skip.
- RNG: uma chamada `syUtilsRandIntRange(16)` por ramo que sorteia; o asm faz uma chamada `get_random_int_(16)` no mesmo ponto.

Desvios e decisões (lead confirmar):
1. Ramos por personagem NÃO portados. A tabela `Character.close_quarter_combat.table` (Character.asm 2912) tem todas as entradas = `stop_roll_spam_._continue`, e as entradas por personagem (AI.asm 6309-6360) estão comentadas. Nenhum outro código grava a tabela. Logo Ness, Yoshi, Lucas, Fox/Falco/Wolf/Slippy, Kirby e Puff são inalcançáveis nesta build. Portar isso seria código morto. Se o lead quiser reativar os patches, é preciso porter esses ramos (uns 150 linhas de asm) e decidir o mapeamento de endereços de rótulo para handlers em C.
2. Ids customizados: a ordem vem de `add_cpu_input_routine` (AI.asm 1603-1913), que bate com as âncoras do brief (SHIELD_DROP 0x3C, CLIFF_LET_GO 0x40, NULL 0x4C). Os comentários `// 0x34`, `// 0x35`, `// 0x37` do asm para NESS_DJC_NAIR, MULTI_SHINE e SHIELD_DROP estão defasados em relação a essa ordem (NESS_DJC_NAIR fica em 0x38, MULTI_SHINE 0x3A). Os comentários são suspeitos; o valor usado aqui vem da ordem.
3. Valores de ação: numéricos, do `Action.asm`. O decomp não tem nome para Stun (0xA4) e Sleep (0xA5); `nFTCommonStatusSleep` é outro valor (índice 4). Idle (0x0A) e TurnRun (0x13) batem com a ordem do enum, então `fp->status_id` é tratado como o id de ação do asm. Não confirmado em jogo.
4. Nível: o gate usa o nível bruto (`fp->level >= 10`), como o asm (`slti 10`), não o nível clampado de P17-P25.

EXTRA_SYMBOLS: nenhum para P8/P9.

P11 `cpu_di_` (não portado, REMIX-TODO(AI) em `ftComputerProcessObjective`). Hook localizado (0x8013A3C8, jal para cpu_di_, retorno em 0x8013A49C). Bloqueios:
- Símbolos ausentes em `port/remix/remix_symbols.h`: `Training.entry_di_first_hit`, `entry_di_type`, `entry_di_strength`, `entry_di_direction` (+0x4), `SinglePlayerModes.singleplayer_mode_flag`, `Global.match_info`, `Global.current_screen` (conferir no decomp, pode existir como global C).
- `smash_di_table` e `slide_di_table` guardam endereços de rotinas DI_* (AI.asm 2074-2137), cujos ids de input não estão mapeados em C.
- `cpu_di_parameters` é estado persistente da RAM do jogo (4 portas x 4 campos). Precisa de estado de porta em C, não de leitura da imagem.
- Decisão pendente R11a (lv10 ignora o toggle "Improved AI") e R11c (estado sobrevive entre partidas).

Como ver na partida (o lead roda):
- P8/P9: CPU lv10 (`SSB64_VS_CPU_LEVEL=10`, vs.sh) e Giga Bowser em lv9. Perto do oponente, em vez de só rolar, deve aparecer parada de analógico (RESET_STICK), short hop in place/towards/away e pivot. Lv9 sem Giga Bowser: rolagem vanilla.
- P11: não se aplica (não portado). Depois que o porte existir: `SSB64_REMIX_TOGGLES="Improved AI=1"` em lv9, e lv10 via `SSB64_VS_CPU_LEVEL=10`; DI depois de hit.

## Slice D2

Scope: P4 `weight_attack_options` (AI.asm 5485-5938, 0x80133784) and P7 `no_attack_when_all_odds_eq_zero` (AI.asm 6015-6034, 0x80133FC4). P5 and P6 were already ported in C1 (`ftRemixAiSkipRepeatedInputCheck`, `ftRemixAiInputRepeatLimitHit`), and the `_custom_weight_table` dispatch was already in the port (`port_fighter_ai_attack_weight_routine`, accumulate loop).

Arquivos criados:
- `decomp/src/ft/ftremix/ftremixaiweightrules.{c,h}`: regras puras (int/float). Landing, hitstun left after tech landing, combo, combo roll, not-combo threshold, slow move kind and chance, grab vs shield, trunc. Constants from Action.asm and the input ids (28, DASH_GRAB 0x45).
- `decomp/src/ft/ftremix/ftremixaiweight.{c,h}`: wrappers over FTStruct. `ftRemixAiWeightAttackOptionsApplies(fp)` (level == 10) and `ftRemixAiWeightAttackOption(this_fp, target_fp, comattack)`.
- `tests/remix/test_ai_weight_rules.c`: entry `remixTestAiWeightRules`, 45 checks, 0 failures standalone (`cc -std=c99 -DPORT=1 -DREGION_REMIX=1`). The lead registers it in tests/remix/CMakeLists.txt and test_main.c.

Arquivo editado: `decomp/src/ft/ftcomputer.c`
- Include `ft/ftremix/ftremixaiweight.h` in the REMIX block (line 22).
- Hook in `ftComputerCheckDetectTarget` (~4356), in the gather loop right after `input_kinds[attack_count] = comattack->input_kind;`. Level 10 sets `detect_ranges_x[attack_count++] = ftRemixAiWeightAttackOption(...)` and `continue`s, which skips the vanilla range switch (the asm jump to +0xB4C with `addiu s1,1`). Other levels run the vanilla code unchanged. Syntax-check OK.

Hook map (asm patch -> decomp):
- P4 -> gather loop, after the input store (ftcomputer.c ~4356). Stages in asm order: own landing (`_check_landing`), combo or not-combo with the opponent landing (`_check_combo`), slow moves (`_less_odds_slow_moves`), grab vs shield (`_check_shield`). The asm's `_custom_weight_table` call is NOT repeated: the existing accumulate-loop dispatch applies it once for level 10 as well.
- P7 -> no hook. The asm forces v0 = FALSE at +0x1108 (the function exit reached when no candidate is picked). The port already returns FALSE on that fall-through (ftcomputer.c, end of ftComputerCheckDetectTarget, `#ifdef PORT return FALSE`). Equivalent for all levels, so P7 is covered without new code.

Field mapping (N64 offset -> port field), by FTStruct layout count and cross-checks:
- 0x13 level, 0x24 status_id, 0x2C percent_damage, 0xEC coll_data.floor_line_id, 0xF0 coll_data.floor_dist, 0x14C ga (nMPKineticsGround = 0, so the check runs when airborne), 0x18C bit 0 is_hitstun.
- 0xB18 status_vars.common.damage.hitstun_tics (cross-checked in ftremixcombo.c).
- 0xB34 via `ftRemixFighterFloat(fp, 0x0B34)` (shieldstun frames, float). Mapped by the union rule of ftremixfields.h. VERIFY in match.
- 0x40 read as `target_fp->hitlag_tics` (see deviation 1).

Deviations and assumptions (the lead decides, all marked in the code or here):
1. D2-1 (asm bug suspected): the asm comment calls the combo read at 0x40 "opponent's hitstun timer", but by FTStruct layout 0x40 is hitlag_tics. The port reads hitlag_tics literally (the Remix build does this). The landing read at 0xB18 is hitstun_tics. If the intent was hitstun, the fix is `target_fp->status_vars.common.damage.hitstun_tics` in ftRemixAiWeightComboCommitChance. REMIX-TODO(AI) comment in ftremixaiweight.c.
2. D2-2 order: the asm applies `_custom_weight_table` in P4, before the accumulate loop's `*= 0.25F` repeat factor and the count bonus. The port applies the existing custom dispatch in the accumulate loop, after the 0.25 factor. Same result when the custom routine is linear in the chance (the Crash/Dedede routines are). Confirm.
3. D2-3 no-target branch: the asm `beqz t4, _b_b4c` (no opponent struct) is not ported. `target_fp` is dereferenced at the top of ftComputerCheckDetectTarget, so the branch cannot be reached.
4. D2-4 asm comments inverted: `_check_landing` says "if grounded, skip" but `beqz` on ga skips when ga == 0. The port follows the branch (checks run when airborne). Same for the fall cap comment (cap applies when vel <= max_fall, the code is correct).
5. D2-5 RNG order kept: combo roll only when span >= 2; not-combo always draws one float before the threshold, even for start frames < 15.
6. D2-6 landing discard does not short-circuit the later stages (as in the asm). The later stages run on a zero chance and consume their random numbers.

EXTRA_SYMBOLS: none (no REMIX_SYM_ used).

REMIX-TODO(AI) left: none inside P4 and P7 beyond deviation 1 (the field choice for 0x40).

How to see it in a match:
- CPU level 10 (the hook is level 10 only, no fighter gate). With a CPU at level 10 near a target in hitstun, combo moves are picked more often (chance x1000). Near a shielding target, grabs (input 28 and DASH_GRAB) are picked much more often (x1000).
- Target falling in a tech action (0x33-0x39) above a floor: the CPU stops picking combo moves that land after the target does.
- Slow moves (start frame >= 15) against a target at low percent in neutral are picked less often than at level 9.
- No port_log line is added for this hook (no per-frame log, to avoid flooding). To confirm, watch the CPU's input choice at level 10 against a Shield or stunned target.
