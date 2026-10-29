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
