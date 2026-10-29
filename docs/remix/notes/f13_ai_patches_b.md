# F13 AI.asm patch inventory, part B (T7.1 input)

Scope: `OS.patch_start` lines 3500-5200 of `.toolchain/pkgs/smashremix-src/src/AI.asm`.
Active patches in that range: 18 (lines 3547, 3585, 3636, 3663, 3690, 3733, 3892, 4007, 4033, 4337, 4799, 4869, 4903, 4987, 5065, 5136, 5158, 5183).
Commented-out patches in the range, ignored: 3922 (`debug_target_pos`, debug only).
Read range: AI.asm 2640-5440 (the body of the last patch, `fix_movement_prediction`, runs to 5440).

Status: inventory only. Nothing ported, nothing built. Every hook below is an
observation from the asm and the decomp, not a tested behaviour. Items marked
"VERIFY" need the vanilla ROM disassembly (rabbitizer, ROM offset in the table)
or an in-game check before the C port.

---

## 0. How the PREVENT_ATTACK / attack weight / recovery routines are dispatched

### 0.1 Three per-fkind tables, filled by each fighter's asm

Remix never patches the vanilla dispatch sites with per-character code. It
patches three generic call sites, each of which reads a Character table keyed by
`character id` (`lw t9, 8(s0)`, `sll 2`, `li at, <table>`):

| Call site (AI.asm) | Table | Vanilla site | Port call site |
|---|---|---|---|
| `PREVENT_ATTACK` routines, reached through the vanilla jump table at 0x801334E4 (AI.asm 2971-2973) | `ai_attack_prevent` (Character.asm:272, moved at Character.asm:2279) | candidate commit in ATTACK_CHECK (0x80132EC8) | `ftcomputer.c:4474-4487` |
| `_custom_weight_table` (AI.asm 5914-5937), called by `jal` at AI.asm 5898 and 5905 | `cpu_attack_weight` | per-candidate weight, ATTACK_CHECK (0x80132EC8) | `ftcomputer.c:4438-4451` |
| `custom_recovery_logic` (AI.asm 384-411), patch at 0x80137FBC | `recovery_logic` | after `ftComputerFollowObjectiveWalk` in Recover (0x80137F24) | `ftcomputer.c:6609-6620` |

Other per-character tables used by the scopes in this range: `lv10_ai_behaviour`
(read in custom_attacks_table, AI.asm 3561-3567; filled in src/VanillaLv10Attacks.asm)
and `cpu_post_process` (read at AI.asm 3868-3878; filled in jigglypuffkirbyshared,
pikashared, yoshishared). Both are used only by the lv10 hooks listed below.

Fighter files that fill the tables (grep of `table_patch_start` in `.toolchain/pkgs/smashremix-src/src`):
- `ai_attack_prevent`: Falco, Crash, Wario, GBowser, Marina, Sonic, DSamus (2 entries), Dedede, Wolf, Bowser, Conker, Piano, Banjo, DrLuigi, DrMario, Roy, Marth, Peach, Lanky, Mewtwo, NFalco, Sheik, NSheik, NGanondorf, Ganondorf, DragonKing, and FOX/JFOX/SLIPPY via AI.asm 6502-6518.
- `cpu_attack_weight` and `recovery_logic`: 52 registrations in total (mario, pika, yoshi, captain, fox, link, ness, puff/kirby, falco, banjo, crash, dedede, dragonking, ganondorf, marina, marth, peach, piano, sheik, sonic, wario and others).

### 0.2 PREVENT_ATTACK routines (AI.asm 2973-3258)

Routine labels, with the registrations that point at them:

| Routine (AI.asm line) | Registered by | Notes |
|---|---|---|
| `NONE` 0x80133520 (vanilla) | Lanky, Mewtwo, NFalco, NMarina, Sheik, NSheik, NGanondorf | no veto |
| `MARIO` 0x801334EC (vanilla) | Piano, Banjo, DrLuigi, Marth, Roy, DrMario, Peach | "avoid SD from USP" |
| `YOSHI_FALCON` 0x80133510 (vanilla) | DSamus (AI/Attacks.asm:72), Ganondorf, DragonKing | conflicts with SONIC_DSP for DSamus, see 0.4 |
| `KIRBY` 0x80133500 (vanilla) | none | |
| `FALCO_NSP` (2985) | Falco (Falco/AI/Attacks.asm:45) | veto NSP while airborne near ground-SD; USP check goes to FOX_USP |
| `BOWSER_USP_DSP` (3015) | Bowser (Bowser/AI/Attacks.asm:75) | veto grounded DSP and USP |
| `CRASH` (3034) | Crash (Crash/Crash.asm:716) | veto grounded DSP/USP on soft platform |
| `GBOWSER` (3060) | GBowser (GBowser/AI/Attacks.asm:56) | veto DSP facing ledge |
| `USP` (3079) | Dedede, Wolf | veto USP |
| `SONIC_DSP` (3096) | Sonic (Sonic/AI/Attacks.asm:71), DSamus (DSamus.asm:283) | veto DSP |
| `WARIO` (3113) | Wario (Wario/Wario.asm:311) | veto DSP and NSP toward/jump-toward |
| `CONKER_GRENADE` (3134) | Conker (Conker/AI/Attacks.asm:44) | skip grenade when `0x0ADC(s0)` (grenade count) is 0 |
| `MARINA_NSP` (3163) | Marina (Marina/AI/Attacks.asm:59) | veto NSP |
| `PUFF_DSP` (3180) | none active (only in commented block AI.asm 6453-6464) | level < 10 cannot DSP; lv10 checks targets < 30% |
| `FOX_USP` (3233) | FOX, JFOX, SLIPPY, FALCO (AI.asm 6502-6518) | lv10 Fox does not USP while lv10 |
| `SKIP_ATTACK` 0x80133A14 (2981) | none (debug) | target of `j` used by CONKER_GRENADE, PUFF_DSP, FOX_USP |

Mechanics, same for every routine: the candidate command is in `t1`. A routine
either exits to the vanilla commit at `0x80133520` (`j 0x80133520`), or vetoes by
setting `t2 = 1` in the delay slot before the same `j`, or jumps to SKIP_ATTACK.
The `t2` contract is only known from these routines; the code at 0x80133520 is
outside this range and was not read (VERIFY, see section 3).

### 0.3 Attack weight routines (AI.asm 5898-5937)

`_custom_weight_table` looks up `cpu_attack_weight.table[char id]`. If non-zero,
it loads the current weight from the stack-relative chance slot (`sp + a0`,
offset 0xBC+0x10), calls the entry with `jalr`, then stores `f2` back. The
signature is "f2 in, f2 out", no argument registers. No Remix fighter entry was
inspected in detail (outside this range).

### 0.4 Comparison with the port

- Port header: `port/fighter_registry.h:145` (`ai_attack_prevent_routine`), `:151`
  (`ai_recovery_routine`), `:158` (`ai_attack_weight_routine`), with the
  signatures documented in the struct comments. Getters at
  `port/fighter_registry.cpp:263-282`, declared at `port/fighter_registry.h:310-312`.
- Call sites: `decomp/src/ft/ftcomputer.c:4438-4451` (weight, per candidate in
  ftComputerCheckDetectTarget), `:4474-4487` (prevent, at commit time, returns FALSE),
  `:6609-6620` (recovery, after ftComputerFollowObjectiveWalk in ftComputerFollowObjectiveRecover).
- **No fighter registers any of the three.** A grep of `port/` and `decomp/src/` for
  `ai_attack_prevent_routine`, `ai_attack_weight_routine` and `ai_recovery_routine`
  finds only the declarations, the getters and the three call sites. No
  `ftremix/` directory has an AI file (`decomp/src/ft/ftremix` has only `ftremixairdodge.c`
  and `ftremixdkcaptainshared.c` among AI-adjacent names). Every Remix PREVENT,
  weight and recovery routine is therefore inactive in the port: the getters return
  NULL and the calls are skipped.
- The port has no lv10 hooks at all for this range. `Toggles.entry_improved_ai`
  (port symbol `REMIX_SYM_Toggles__entry_improved_ai`, `port/remix/remix_symbols.h:4068`)
  and `entry_single_button_mode` (`:5007`) exist, and `gmRemixToggle(name)` is available
  (`decomp/src/gm/gmremixtoggle.h`). `gmRemixToggle(single_button_mode)` is already used by
  ftremixsinglebutton.c, but nothing in `decomp/src` reads `improved_ai` yet.
- Semantic gap on veto: the port's prevent dispatch returns FALSE from
  `ftComputerCheckDetectTarget`, which aborts the whole candidate selection for the
  frame. The SR routines only set `t2 = 1` and jump to the vanilla commit (or to
  SKIP_ATTACK). VERIFY what 0x80133520 does with `t2` before choosing between
  "skip this candidate" and "abort the selection".
- Two fighters register two different routines in the same `ai_attack_prevent`
  slot, so only the last one loaded wins:
  - FALCO: `FALCO_NSP` (Falco/AI/Attacks.asm:44-45) and `FOX_USP` (AI.asm:6517-6518).
  - DSAMUS: `SONIC_DSP` (DSamus/DSamus.asm:282-283) and `YOSHI_FALCON` (DSamus/AI/Attacks.asm:71-72).
  Include order decides the result. VERIFY which one the build uses.
- `PUFF_DSP` is defined but only referenced from commented code, so it is dead today.

---

## 1. Inventory of the 18 patches

Vanilla function names come from the `// 0xADDR` comments in
`decomp/src/ft/ftcomputer.c`. For each patch the function is the one with the greatest
anchor address <= the patch address. Anchors and function lines were read from the file.

Difficulty: S = a few lines of C, M = several branches or a helper, L = multi-part
with its own helpers.

---

### P01. custom_attacks_table (lv10 attack table override)
- Lines: scope AI.asm 3546-3580; patch 3547-3551. ROM 0xADB7C, RAM 0x8013313C.
- Decomp: `ftcomputer.c:3904` `ftComputerCheckDetectTarget` (anchor 0x80132EC8 at 3903).
- Change: for a level-10 CPU whose character id has an entry in `lv10_ai_behaviour`,
  replaces `s2` (the attack table pointer) with that entry. Otherwise keeps the vanilla
  pointer. The `j` in the patch uses the delay slot to run the vanilla word at 0x80133140
  (see Dependencies), then the hook falls back to vanilla line 1 (`beqz t7`) or returns
  to 0x80133144.
- Reads: `Character` byte 0x13 (cpu level), `lw t9, 8(s0)` (char id), `lv10_ai_behaviour`
  table (`Character.lv10_ai_behaviour.table`).
- Dependencies: patches at 0x80133140 (Character.asm:1780, `lw s2, LOWER(s2)`, in the
  delay slot) and at 0x80133144 (Smashketball.asm:1211, `ai_ball_attack_range`, reached by
  `j _return+0x4`). Both are owned by other files. VERIFY that the `_original` path matches
  vanilla. Its `beqz t7` replays vanilla line 1, and the vanilla delay slot is the
  already-executed line 2, which does not write t7.
- C hook: `ftRemixAiLv10AttackTable(FTStruct *fp)` returns the table pointer or NULL. Called
  where ftComputerCheckDetectTarget loads the entry for the attack being checked.
- Difficulty: M (the table is a generic per-char table, and the vanilla word order must be preserved).
- Risks: the `j` delay-slot trick hides the ordering of the three owners of 0x8013313C..0x80133144.
  Port needs the same table indirection or a direct pointer lookup.

### P02. auto_jab_grab_ (lv10 CPU auto grab during jab)
- Lines: scope 3584-3631; patch 3585-3588. ROM 0xC48BC, RAM 0x80149E7C.
- Decomp: `ftcommon/ftcommoncatch1.c:187` `ftCommonCatchCheckInterruptAttack11`
  (anchor 0x80149E24 at 186). Not in ftcomputer.c.
- Change: during the jab grab interrupt, a level-10 CPU (player type 1) grabs when the
  current frame is >= 3 (see the frame note below) and when Single Button Mode is off or
  set to R or R+C. Otherwise the vanilla path runs. The patch's delay slot is vanilla line 1
  (`lb t7, 0x23(a0)`, player type).
- Reads: `Toggles.entry_single_button_mode` offset 0x4 (values 0 off, 1 A, 2 B, 3 R, 4 A+C,
  5 B+C, 6 R+C; Toggles.asm:2446), `0x1C(a0)` (current frame), `0x13(a0)` (level).
- Dependencies: none inside AI.asm. Shares Single Button Mode semantics with
  `ftremixsinglebutton.c` (port) and the T5 Single Button task.
- C hook: `ftRemixAiShouldAutoGrab(FTStruct *fp)` returning bool, called in
  ftCommonCatchCheckInterruptAttack11 before the vanilla grab test.
- Difficulty: S.
- Risks / suspected bugs:
  - The header says the check is for frames 1-2 of the jab, but the code does `slti t7, t7, 3`
    and `bnez t7, _normal`, so the auto grab runs at frame 3 and later. Decide the intent
    before porting.
  - `_normal` executes `beqz t9, _original_end`, but nothing in the patch writes t9. `t0` in
    `lw t1, 0x0100(t0)` is also not set here. Both come from the vanilla context. VERIFY
    against the vanilla disassembly at 0x80149E7C..0x80149EA8.
  - `beqzl t1, pc()+8` is a branch-likely that runs the delay slot only when taken. Checked
    and correct: values 4..6 become 1..3.

### P03. check_invulnerable_ (lv10 avoid respawning players)
- Lines: scope 3635-3658; patch 3636-3639. ROM 0xAD6C4, RAM 0x80132C84.
- Decomp: `ftcomputer.c:3811` `ftComputerCheckEvadeDistance` (anchor 0x80132BC8 at 3810).
- Change: in the evade loop over enemies, skips to avoid (0x80132C90) when `s3 == enemy
  invuln flag` (vanilla line 1: `lw t9, 0x5B4(s0)`). For level 10, also avoids when the
  enemy's invulnerability timer (`0x5A4(s0)`) is non-zero. Otherwise goes to 0x80132CAC.
- Reads: `0x5B4(s0)`, `0x5A4(s0)` (enemy player struct), `0x13(a0)` (CPU level), `s3`.
- Dependencies: EVADE_CHECK main (0x80132BC8). Shares the player loop with other evade code.
- C hook: `ftRemixAiShouldIgnoreRespawningEnemy(FTStruct *cpu, FTStruct *enemy)` returning bool.
- Difficulty: S.
- Risks: the hook uses `s0` for the enemy struct and `a0` for the CPU. VERIFY that `s0`
  is the enemy in this loop (the vanilla line 1 suggests so). `s3` is not defined in this
  scope; its value must be known from the caller. Jump targets 0x80132C90 and 0x80132CAC must
  map to the same loop continuation in C.

### P04. extend_cliff_attack_ (lv10 cliff getup options)
- Lines: scope 3662-3685; patch 3663-3666. ROM 0xB1844, RAM 0x80136E04.
- Decomp: `ftcomputer.c:5977` `ftComputerGetObjectiveStatus` (anchor 0x80136D0C at 5976).
  The address is in its range, not in a separate function.
- Change: while a CPU is at a ledge (CliffWait), a non-level-10 CPU gets input 4 (normal
  getup). A level-10 CPU picks between getup (4) and `AI.ROUTINE.CLIFF_LET_GO` using
  `Global.get_random_int_(2)` then `srl 1`.
- Reads: `0x13(a0)` (level), `Global.get_random_int_` (0x80018994, range 0..N-1).
- Dependencies: `AI.ROUTINE.CLIFF_LET_GO` (AI.asm routine table, outside range).
- C hook: `ftRemixAiCliffGetupCommand(FTStruct *fp)` returning the command id; called in
  the CliffWait branch of ftComputerGetObjectiveStatus.
- Difficulty: S.
- **Suspected bug (high):** `get_random_int_(2)` returns 0 or 1. `srl v0, v0, 1` then always
  gives 0, so `beqzl v0` always takes the branch and a level-10 CPU always uses
  `CLIFF_LET_GO`. The "attack or jump" 50/50 in the comment never happens. Confirm the
  intent (maybe the shift was meant for a larger range) before porting.
- Risk: consumes RNG only for level 10 CPUs, so the RNG stream differs from vanilla.

### P05. taunt_replace_ (lv10 no taunt)
- Lines: scope 3689-3725; patch 3690-3693. ROM 0xB2194, RAM 0x80137754.
- Decomp: no anchor at 0x80137754. Greatest anchor below it is 0x80136D0C
  (`ftcomputer.c:5977` `ftComputerGetObjectiveStatus`). VERIFY; the address sits before
  ftComputerProcDefault (0x80137778, ftcomputer.c:6321).
- Change: a non-level-10 CPU's taunt is replaced by `AI.ROUTINE.TAUNT`. A level-10 CPU never
  sends a command here: NESS, LUCAS and JNESS (Pk Thunder) and everyone else go to
  `_no_input` after the `j 0x80137768` return. The `b _set_input` for USP at 3714-3715 is dead
  code (the `b _no_input` at 3712 precedes it), even though its comment says "for Ness clones
  and Lucas".
- Reads: `0x13(a0)` (level), `0x8(s0)` (char id; s0 and a0 both used for the same struct,
  VERIFY).
- Dependencies: `AI.ROUTINE.TAUNT`, `AI.ROUTINE.USP`, `Character.id.NESS/LUCAS/JNESS`.
- C hook: `ftRemixAiTauntCommand(FTStruct *fp)` returning the command id or NULL. Called where
  the vanilla taunt command is set.
- Difficulty: S.
- Risks: if the dead USP branch was meant to be live, lv10 Ness/Lucas would USP instead of
  nothing. Decide the intent with the author before porting.

### P06. cpu_post_process (per-frame post process, several sub-behaviours)
- Lines: scope 3732-3886; patch 3733-3737 (two words: `j` and `nop`). ROM 0xB52C4, RAM 0x8013A884.
- Decomp: `ftcomputer.c:7895` `ftComputerProcessAll` (anchor 0x8013A834 at 7894).
- Change, in order, after the three vanilla subroutines, before ftComputerUpdateInputs:
  1. Replays the vanilla call `jal 0x8013A38C` with `a0 = s0`.
  2. `_check_respawn` (gated by `Toggles.entry_improved_ai`): when the CPU's action is 4
     (Sleep, i.e. dead), presses START every 16 frames by writing `0x1C6(s0)`. Comment says
     "team battles", but there is no mode check.
  3. `_check_if_dash_attack_input_is_possible`: if the command is DASH_ATTACK and the action
     is not Idle or Run, cancels with command NULL (jal 0x80132758). Not level-gated.
  4. `_dash_usmash_usp` (level 10 only): while Dash or Run, USMASH becomes JUMPCANCEL_UPSMASH
     and USPG becomes JUMPCANCEL_USP.
  5. `_custom_usp_command` (level 10 only): USPA becomes USP_TOWARDS when the CPU uses it.
  6. `_post_process_function`: jumps to the per-character `cpu_post_process` entry with `a0 = s0`.
- Reads: `Toggles.entry_improved_ai` (Toggles.asm:2428), `0x24(s0)` (action), `0x1D4(s0)`
  (`ft_com->p_command`), `AI.command_table`, `0x1C6(s0)` (button mask), `Global.current_screen_frame_count`.
- Dependencies: `AI.ROUTINE.DASH_ATTACK`, `JUMPCANCEL_UPSMASH`, `JUMPCANCEL_USP`, `USP_TOWARDS`,
  `NULL`, the per-char `cpu_post_process` table (filled in jigglypuffkirbyshared, pikashared, yoshishared).
- C hook: split into one function per sub-behaviour, all called from ftComputerProcessAll:
  `ftRemixAiMashStartWhenDead`, `ftRemixAiCancelUnreadyDashAttack`,
  `ftRemixAiJumpCancelDashCommand`, `ftRemixAiLv10CustomUsp`, plus the per-char hook. Give
  the per-char hook a registry field: none exists in `fighter_registry.h` today.
- Difficulty: L (six behaviours, two of them not level-gated, one table without a port field).
- Risks:
  - Behaviours 2 and 3 are not gated by level. If that was not intended, non-lv10 CPUs change.
  - The START mash writes all button bits through `andi at, 0`. It also drops other buttons
    that frame. Acceptable only for dead CPUs.
  - `_check_respawn` is gated by a toggle; the other subs are not.

### P07. force_objective_attack (lv10 objective always attack)
- Lines: scope 3891-3915; patch 3892-3896 (`j` plus original line 2 `sh t9, 0x14(v1)` in
  the delay slot). ROM 0xB5158, RAM 0x8013A718.
- Decomp: `ftcomputer.c:7808` `ftComputerProcessTrait` (anchor 0x8013A63C at 7807).
- Change: for level 10, forces the random objective roll to 0 (default, so the objective
  stays "attack"). Other levels run the vanilla `jal 0x80018910` (get random short). The
  delay slot's `sh` runs before the hook.
- Reads: `0x13(a0)` (level).
- Dependencies: none inside AI.asm. Objective 0 is "default" in the decomp enum.
- C hook: `ftRemixAiTraitObjectiveRoll(FTStruct *fp)` returning s32, call site in
  ftComputerProcessTrait replacing the random roll.
- Difficulty: S.
- Risks: the vanilla `jal` in the patch leaves `ra` pointing into the patch, not the
  vanilla return address. Harmless if the enclosing function restores `ra` from the stack
  (VERIFY its prologue). Level 10 also stops consuming RNG here.

### P08. _fix_weird_air_drift_right (lv10 no drift target to the right)
- Lines: scope 4006-4028; patch 4007-4011. ROM 0xAEA98, RAM 0x80134058.
- Decomp: `ftcomputer.c:4560` `ftComputerCheckSetTargetEdgeRight` (anchor 0x80134000 at 4559;
  the asm comment says "80134000+58").
- Change: a level-10 CPU with positive Y speed skips the block at `0x80134000+0x98` that sets
  the target to itself plus an X offset. Other levels run the vanilla path.
- Reads: `0x13(a0)` (level). Vanilla lines 1-3 are replayed (`addiu s4, s2, 0x1cc`,
  `lui at, 0x3f40`, `addiu s3, s3, 0x50E8`).
- Dependencies: P09 is the mirror image.
- C hook: `ftRemixAiSkipAirDriftTarget(FTStruct *fp, sb32 is_right)` returning bool, shared with P09.
- Difficulty: S.
- Risks: the skip jumps into the middle of the vanilla function. The C version must turn it
  into a condition around the block, with the same delay-slot effects (s3 update) on both paths.

### P09. _fix_weird_air_drift_left (lv10, mirror of P08)
- Lines: scope 4032-4054; patch 4033-4037. ROM 0xAEE00, RAM 0x801343C0.
- Decomp: `ftcomputer.c:4654` `ftComputerCheckSetTargetEdgeLeft` (anchor 0x80134368 at 4653).
- Change: same as P08 with target `0x80134368+0x98`.
- Reads, dependencies, risks: same as P08.
- C hook: same shared function as P08 (direction parameter).
- Difficulty: S.

### P10. _improve_recovery_drift (lv10 improved recovery target, largest hook)
- Lines: scope 4056-4329 (helpers `improved_recovery._find_closest_ledge`, 4063-4283,
  and `is_ledge_grabbed`, 4289-4329); patch 4337-4341 (`j` plus `nop`); body 4343-4791.
  ROM 0xAFE40, RAM 0x80135400.
- Decomp: `ftcomputer.c:5007` `ftComputerFollowObjectiveWalk` (anchor 0x80134E98 at 5006;
  asm comment "80134E98+568").
- Change, for a level-10 CPU, when in the Recover objective (behaviour 4) and not in
  ft-special-fall, all jumps used, or below ledge rules:
  1. Finds the nearest left and right ledges with `_find_closest_ledge`. It walks
     `gMapLineTypeGroups` (0x80131348), skips ledges outside the blastzone padded by 200
     (0x80131300), skips ledges with ground below (`0x800F8FFC`), biases the distance by 1.5 when
     the ledge is above the CPU, and checks grabbability (`Surface.get_clipping_flag_`) and
     whether someone already holds it (`is_ledge_grabbed`).
  2. Picks the nearer side, sets the recovery direction, and writes target X/Y (0x1CC+0x60/0x64).
  3. Applies padding: outward 300 in X when facing away, inward 300 when above the ledge, and
     ledge grab offsets (0xAC/0xB0 of the attributes).
  4. Handles the up-special flag (0x1CC+0x49), the fall-special action, and a list of characters
     (Puff, Kirby, Dedede) that always go for the ledge.
- Reads: `0x13(a0)`, `0x24(a0)` (action), `0x148(a0)` (jumps used), `0x9C8(a0)` (attributes,
  `0x64` max jumps, `0xAC/0xB0` ledge grab offsets), `0x78(a0)` (location), `0x44(a0)` (facing),
  `0xEC(a0)` (clipping id), `0x1CC+0x6C` (opponent), `0x1CC+0x49` (USP flag), `0x1CC+0x0` (behaviour),
  `Global.p_struct_head` (0x80131300 region), gMapEdgeBounds (0x80131308), gMapLineTypeGroups (0x80131348).
- Dependencies: `custom_recovery_logic` is a separate hook that runs after this walk; the
  up-special flag is written elsewhere (outside range); P11 (`prevent_fastfall`) reads the same
  opponent struct.
- C hook: a set of functions, not one: `ftRemixAiFindClosestLedge(x, y, direction, out)`,
  `ftRemixAiLedgeIsGrabbed(line_id, direction)`, `ftRemixAiLedgeHasGroundBelow(...)`,
  `ftRemixAiRecoverTarget(FTStruct *fp)` as the entry called from ftComputerFollowObjectiveWalk.
  Use the decomp's decoded map structures, not raw RDRAM offsets. The asm reads big-endian
  halfwords with `lhu 0(s0)`, and the port is little-endian.
- Difficulty: L.
- Risks / suspicious points:
  - `_find_closest_ledge` returns the address of a stack slot (`sp+0x28`) after `routine_end`,
    and the caller reads through it. Works only because nothing runs in between. Use a return
    value in C.
  - `_check_ground_below` writes `f20` (callee-saved in the o32 ABI) and does not save it. If
    the enclosing function keeps a value in f20, it is corrupted. VERIFY. The same holds for
    `_get_friction` in P18.
  - `_original` replays vanilla lines 1-2 (`lw t8, 0x8e8(s0)`, `lwc1 f14, 0x20(t8)`) and returns to
    0x80135408, so the vanilla lines at 0x80135408-0x80135410 run twice. The code is idempotent
    (loads and a compare), but VERIFY against the vanilla words.
  - Both `_end` and `_original` read `lw t0, 0x44(sp)`. This depends on the caller's stack
    layout at that point. VERIFY, or port this as a parameter.
  - The debug comments mention a 0x1CC+0x45 slot and a 0x1CC+0x49 slot. Only 0x49 is used here.

### P11. prevent_fastfall (lv10 fastfall gate)
- Lines: scope 4798-4855; patch 4799-4803. ROM 0xB0144, RAM 0x80135704.
- Decomp: `ftcomputer.c:5007` `ftComputerFollowObjectiveWalk` (anchor 0x80134E98 at 5006;
  asm comment "80134E98+86C").
- Change, for a level-10 CPU:
  1. Clears the "already tried fastfall" bit (0x80 at byte 0x4A of the ftcom struct), so the
     CPU can retry.
  2. Denies the fastfall when the CPU has used all jumps, when the opponent is offstage (clipping
     below = -1), or when the distance to the platform below (0xF0) is not less than -2000
     (within 2000 of the ground). Otherwise issues the fastfall command 0x27 with `jal 0x80132564`.
  Level 1-9 runs the fastfall command directly.
- Reads: `0x13(a0)`, `0x44(sp)` (ftcom pointer from the caller frame), `0x4A`, `0x9C8(a0)` (attributes
  max jumps), `0x148(a0)` (jumps used), `0x1CC+0x6C` (opponent), `0xEC` (clipping below), `0xF0` (distance to ground).
- Dependencies: vanilla fastfall attempt flag in byte 0x4A (set by vanilla fastfall code, outside range).
- C hook: `ftRemixAiShouldFastfall(FTStruct *fp)` returning bool, called where the vanilla fastfall
  command is issued in ftComputerFollowObjectiveWalk.
- Difficulty: M.
- Risks: the comment says "distance > 2000.0", but the compare is "< -2000" (negative is above
  ground, so the rule is "more than 2000 above"). Fix the C comment to say what the code does.
  `lw t0, 0x44(sp)` depends on the caller stack layout (same as P10).

### P12. detect_offstage_opponents (lv10 target offstage opponents)
- Lines: scope 4866-4899 (scope `hit_offstage_opponents`); patch 4869-4873. ROM 0xAD458, RAM 0x80132A18.
- Decomp: `ftcomputer.c:3724` `ftComputerCheckFindTarget` (anchor 0x8013295C at 3723;
  asm comment "8013295C+BC").
- Change: a level-10 CPU skips the on-stage test (jumps to 0x8013295C+0x15C) so it can target
  an opponent who is offstage. Other levels run the vanilla test `beqzl v0, branch_128` with
  `lw t2, 0x14c(s1)` in the delay slot.
- Reads: `0x13(s1)` (CPU level), `v0` (result of the vanilla IGNORE/clip check), `0x14c(s1)`.
- Dependencies: P13 (`clamp_target_pos_to_stage`) is the other half. Together they let lv10 chase
  an offstage opponent and clamp the target back onto the stage.
- C hook: `ftRemixAiTargetsOffstageOpponent(FTStruct *cpu)` returning bool, call in
  ftComputerCheckFindTarget where the on-stage test happens.
- Difficulty: S.
- Risks: for level 10 the vanilla `lw t2, 0x14c(s1)` (delay slot) is skipped. VERIFY that nothing
  after 0x80132AB8 reads t2. Level 10 CPUs now target offstage opponents, so P13 must work.

### P13. clamp_target_pos_to_stage (lv10 clamp target inside the stage)
- Lines: scope 4902-4977; patch 4903-4907. ROM 0xAD524, RAM 0x80132AE4.
- Decomp: `ftcomputer.c:3724` `ftComputerCheckFindTarget` (anchor 0x8013295C at 3723;
  asm comment "8013295C+188").
- Change, for a level-10 CPU, on target f20 (X) and f22 (Y):
  - Left bound: `if (leftLedgeX - 300 <= targetX) targetX = leftLedgeX - 300` (as written, see below).
  - Right bound: the same with `rightLedgeX + 300`.
  - Y: `targetY -= 200`, then if `targetY <= 0`, `targetY = 0`.
  - Ends with the vanilla `swc1 f20, 0x60(v1)` and `swc1 f22, 0x64(v1)`.
- Reads: `0x13(s1)` (level), `0x28(s3)` and `0x2C(s3)` (left and right ledge X), f20, f22.
- Dependencies: P12. Also the vanilla caller that set f20/f22 (outside range).
- C hook: `ftRemixAiClampTargetToStage(FTStruct *cpu, f32 *target_x, f32 *target_y)` with explicit
  bounds, so the caller's stage data is injected and nothing reads raw offsets.
- Difficulty: S (but see the bug).
- **Suspected bug (high):** `clamp_min_x` does `c.le.s f2, f20` (true when the target is inside
  the bound) followed by `bc1fl after_clamp_min_x` (branch-likely, taken when the compare is
  false). The branch is therefore taken when the target is offstage, which skips the clamp, and
  the clamp runs when the target is inside. `clamp_max_x` has the same inverted sense. Result,
  as written: an in-bounds target is replaced by `rightLedgeX + 300`. The Y clamp is correct.
  Compare with the comments ("if not offstage, skip") and test in game before choosing intent.
- Also: `f2`, `f4` are scratch FP registers. The comment says "do not touch f12 or f24".

### P14. shorthop_followup_attack (lv10 short hop instead of full hop when target is above)
- Lines: scope 4982-5056; patch 4987-4991. ROM 0xB0558, RAM 0x80135B18.
- Decomp: `ftcomputer.c:5007` `ftComputerFollowObjectiveWalk` (anchor 0x80134E98 at 5006;
  asm comment "80134E98+c80").
- Change: a level-10 grounded CPU whose target is above it computes `diff = targetY + targetVelY
  - myY`. If diff <= 300 it uses MOVE. If <= 600 it uses SHORT_HOP_TOWARDS. If <= 900 it uses
  FULL_C_JUMP_TOWARDS. Otherwise it keeps the vanilla full hop (command 4, the default in the
  delay slot). It always ends with `jal 0x80132564`.
- Reads: `t0` (target struct, set by the caller), `0x6C(t0)` (target fighter), `0x4C(t1)` (target
  Y velocity), `f4` (target Y), `f6` (my Y, both set by the caller, not loaded here).
- Dependencies: P15 (`jump_in_neutral`) picks the same commands from a random roll in the same
  function; both use AI.ROUTINE.SHORT_HOP_TOWARDS and FULL_C_JUMP_TOWARDS.
- C hook: `ftRemixAiShorthopFollowupCommand(FTStruct *cpu, FTStruct *target)` returning the
  command id, call site where the walk sets command 4.
- Difficulty: S.
- Risks / suspicious points:
  - The comment says "f4 = my Y - target Y", but the code computes `f4 - f6 = targetY' - myY`.
    The thresholds only make sense with the code's sign. Fix the C comment.
  - `lw at, 0x0078(a0)` loads the location pointer and never uses it. `f6` ("my Y") is not loaded
    in this scope, so the hook relies on the caller setting f6. Probably the caller's `lwc1 f6`
    was meant to be here. VERIFY the register contract at 0x80135B18.
  - `t0` must still be the target struct when the patch starts (it is set by the caller).

### P15. jump_in_neutral (lv10 random jump while walking to target)
- Lines: scope 5058-5132 (scope `jump_in_neutral.jump_in_neutral`, 5064-5131); patch 5065-5069
  (`j` plus `addiu a1, r0, 1` in the delay slot). ROM 0xB04FC, RAM 0x80135ABC.
- Decomp: `ftcomputer.c:5007` `ftComputerFollowObjectiveWalk` (anchor 0x80134E98 at 5006;
  asm comment "80134E98+c24").
- Change: a level-10 CPU, grounded and not below the target, rolls `Global.get_random_int_(200)`
  every time this code runs:
  - 0, 1, 2 -> SHORT_HOP_TOWARDS (1.5% total)
  - 3 -> SHORT_HOP_IN_PLACE (0.5%)
  - 4, 5 -> FULL_C_JUMP_TOWARDS (1%)
  - anything else -> the vanilla command 1 (point stick at target), about 95%
  When it picks a jump it also sets the CPU behaviour (0x1CC) to 2 (Attack objective).
- Reads: `0x13(a0)` (level), `Global.get_random_int_` (0x80018994), `v0`.
- Dependencies: P14 (same function, same commands).
- C hook: `ftRemixAiJumpInNeutralCommand(FTStruct *cpu, s32 *out_behaviour)` returning the command.
  Use the port's RNG, not the N64 call, and keep the 200 range.
- Difficulty: S.
- Risks:
  - The RNG is consumed every frame for level-10 CPUs in this state. That changes the vanilla RNG
    stream for all systems, and matters for smoke-test determinism.
  - The `lli a1` values sit in the `beq` delay slots, so the last one executed wins. The final
    default `addiu a1, r0, 1` covers the no-match path. This is correct but easy to break in C.
  - The `_modified` path sets the behaviour byte at 0x1CC(a0) to 2. The C field name must be confirmed.

### P16. extend_attack_check_range (lv10 wider attack view range)
- Lines: scope 5135-5154; patch 5136-5140 (`j` plus `lwc1 f4, 0x68(v1)` in the delay slot). ROM 0xB3DB4, RAM 0x80139374.
- Decomp: `ftcomputer.c:7200` `ftComputerFollowObjectiveAttack` (anchor 0x801392C8 at 7199;
  asm comment "801392C8+AC").
- Change: the attack view range is `random * 300 + base`. Vanilla base is 1200.0 (in f10, loaded
  before the patch). For level 10 the hook loads 2000.0 into f10. Then `add.s f16, f8, f10`.
- Reads: `0x13(a0)` (level), f8 (random * 300, set by vanilla), f10 (base, set by vanilla).
- Dependencies: P17 is the same change in the other attack state.
- C hook: `ftRemixAiAttackViewBase(FTStruct *fp)` returning 1200.0F or 2000.0F, used in place of the
  constant in ftComputerFollowObjectiveAttack.
- Difficulty: S.
- Risks: the non-lv10 path depends on f10 (1200) loaded by the vanilla code before the patch. The
  `at` register is clobbered.

### P17. extend_attack_check_range_2 (same, second attack state)
- Lines: scope 5157-5176; patch 5158-5162. ROM 0xB42E0, RAM 0x801398A0.
- Decomp: `ftcomputer.c:7200` `ftComputerFollowObjectiveAttack` (anchor 0x801397F4 at 7331
  inside that function; asm comment "801397F4+AC"). Same function as P16, different state
  (state 0x08, "JAL 0x801397F4" in the state list at AI.asm 3458).
- Change: identical to P16.
- Reads, dependencies, risks: same as P16.
- C hook: same as P16, one helper for both call sites.
- Difficulty: S.

### P18. fix_movement_prediction (lv10 replaces the attack prediction)
- Lines: scope 5182-5440 (scope `fix_movement_prediction`, 5182 to the closing brace at 5440); patch
  5183-5187 (`j` plus `mtc1 a0, f4` in the delay slot). ROM 0xADBC4, RAM 0x80133184.
- Decomp: `ftcomputer.c:3904` `ftComputerCheckDetectTarget` (anchor 0x80132EC8 at 3903; asm
  comment "80132EC8+2BC").
- Change, for a level-10 CPU:
  1. If the CPU is in Dash, Run or RunBrake, only candidates in {GRAB, USMASH, USPG, NSPG,
     DASH_ATTACK} are considered. Others jump to `0x80132EC8+0xB50` (skip candidate).
  2. Predicts the CPU's X at hit_frame, with ground friction from `Surface.friction_table`
     (ground friction times character friction, attributes 0x24), and the target's X at
     hit_frame (target X + target vel X * hit_frame). Saves predicted X at `0x170(sp)`.
  3. Predicts the CPU's Y: current Y when grounded, otherwise steps gravity, max fall speed and
     the ground check (clipping `0xEC`). Predicts the target's Y the same way with the target's
     attributes. Saves `predicted delta Y` at `0x16C(sp)`.
  4. Clears `t2` (is_attempt_cliffcatch = FALSE) and jumps to `0x80132EC8+0x534`.
  Non-lv10 runs the vanilla path: `lwc1 f10, 0x18c(sp)` (original line 2).
- Reads: `0x13(s0)`, `0x24(s0)`, `0x14c(s0)` (aerial flag), `0xF0(s0)` (distance to ground), `0xEC(s0)`
  (clipping below, -1 offstage), `0xF4(s0)` (ground friction word), `0x9C8(s0)` (attributes),
  `Surface.friction_table`, `0x19c(sp)`, `0x194(sp)`, `0x18c(sp)` (target vel and pos), `0x1a0`, `0x1a4`,
  `0x1a8`, `0x1ac`, `0x198` (CPU vel and pos), `0x1b0(sp)` (other struct), `0x180`-`0x190` (target
  gravity, fall, Y).
- Dependencies: P01 (same function, same attack loop). Writes `sp` slots that the vanilla code reads
  after `0x80132EC8+0x534`.
- C hook: `ftRemixAiPredictAttackPosition(FTStruct *cpu, FTStruct *target, s32 hit_frame, FTRemixAiPrediction *out)`
  plus a candidate filter `ftRemixAiRunningCandidateAllowed(s32 input)`. Return the prediction in a
  struct, do not write `sp` slots.
- Difficulty: L (long, with many stack slots).
- Risks / suspicious points:
  - `_get_friction` masks the ground-friction word with `0xFFFF00FF`, keeps the upper halfword
    and the low byte, and uses the result as a word index with no bounds check. VERIFY that the
    upper halfword is 0 for every surface, or the read leaves `Surface.friction_table`.
  - `_get_friction` and the Y prediction write f20 and f6/f8 without saving. f20 is callee-saved
    in o32. VERIFY the enclosing function does not use f20 across this block.
  - The comment at `lw at, 0x14c(s0) // at = grounded status` is wrong: 0x14C is the aerial flag
    (non-zero = aerial), as FALCO_NSP shows.
  - `predict_my_y` reloads `lwc1 f0, 0x1a8(sp)` twice. Harmless, but it hides which value is used.
  - The candidate filter uses `lw a0, 4(s2)` for the hit start frame. It must match vanilla's `a0`
    for the skip path.

---

## 2. Cross-patch dependencies (this range and its neighbours)

- P01 (0x8013313C) returns through the delay slot of Character.asm:1780 (0x80133140, `lw s2, LOWER(s2)`)
  and `j _return+0x4` to 0x80133144 (Smashketball.asm:1211, `ai_ball_attack_range`). Those
  files must stay in the build for P01 to work. Also, P01 and Character.asm both touch `s2`.
- P12 + P13 form one behaviour: the offstage-target allowance (P12) only makes sense with the
  clamp (P13). P13's inverted bound (see its bug note) changes the result for every lv10 target.
- P14 + P15 choose the same jump commands in the same function. A port should keep one helper
  for the command set (SHORT_HOP_TOWARDS, SHORT_HOP_IN_PLACE, FULL_C_JUMP_TOWARDS, MOVE).
- P16 + P17 are the same constant change in two attack states. One helper.
- P08 + P09 are mirror images. One helper with a direction argument.
- P10 uses the opponent struct (`0x1CC+0x6C`) that P11 also reads. The up-special flag
  (`0x1CC+0x49`) is written outside this range.
- P10, P11 and P14 read the caller's stack slot `0x44(sp)` or depend on caller-saved FP registers
  (f6 in P14). Porting them as C parameters removes the coupling.
- P06 (cpu_post_process) depends on the per-character table whose fighter entries are in
  jigglypuffkirbyshared/pikashared/yoshishared (outside range). Port needs a registry field for
  it, which `fighter_registry.h` does not have.
- Every lv10 hook depends on the level byte 0x13 of the fighter struct. The C field name for it
  must be confirmed in `decomp/src/ft/ftcomputer.c` and `decomp/include/ft/fighter.h` (not checked).

## 3. Open items / could not resolve

- Vanilla disassembly is needed for the words the hooks replay: 0x8013313C, 0x80133144 (P01),
  0x80149E7C..0x80149EA8 (P02, the t9 question), 0x80135400..0x80135410 (P10 `_original`),
  0x80132A18..0x80132AB8 (P12, the t2 question), 0x80133520 (the `t2` contract of PREVENT_ATTACK).
  Use the ROM offsets in the table above with rabbitizer.
- `get_random_int_(2)` range: confirmed as `0..N-1` by the comment in Global.asm:22-27. P04 is a real
  bug only if the code really is `srl 1` on a 0..1 value. Read the vanilla function to be sure.
- Which of FALCO_NSP / FOX_USP and SONIC_DSP / YOSHI_FALCON wins the table slot: depends on the
  include order of the fighter asm files. Check the build's order.
- Port C field names (cpu level, behaviour byte 0x1CC, fastfall flag byte 0x4A, objective enum) are
  not confirmed in this pass.
- No test was run. No code was changed. Only this file was created.
