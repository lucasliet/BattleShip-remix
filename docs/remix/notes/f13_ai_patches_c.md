# F13 / T7.1 - AI.asm patch inventory, part B (`OS.patch_start` at line >= 5158)

Status: inventory only. No code was ported or edited. Source read: `.toolchain/pkgs/smashremix-src/src/AI.asm` lines 5150-7337.
Decomp target: `decomp/src/ft/ftcomputer.c` (plus VS CSS: `decomp/src/mn/mnplayers/mnplayersvs.c`).

Counts: 25 active `OS.patch_start` calls in 23 scopes (two scopes contain two hooks each: `stop_roll_spam_`, `render_10_`). Plus 1 table patch (`Fox_USP_`, `Character.table_patch_start`, not an `OS.patch_start`). Commented-out hooks ignored: `shorthop_approach` (AI.asm:6397, ROM 0xB04FC), the `stop_roll_spam_` table entries (6309-6360), `Puff_DSP_` and `Pika_NSP_` (6448-6494).

---

## 0. How the vanilla address was mapped to a decomp function

1. Greatest `// 0x8013xxxx` comment <= patch address, in the file that owns that overlay.
2. AI.asm's own comments and internal jump targets confirm the ftcomputer mapping for most hooks (for example `80132EC8+2BC`, `80134E98+6B8`, `8013877C+250`, `801397F4+AC`). Those are listed as "confirmed" below.
3. Overlay split by ROM offset (used for the rest):
   - ROM 0xAD000-0xB5000 (all `ft` hooks): ftcomputer overlay.
   - ROM 0x133000-0x135300 (the 5 VS CSS hooks, 0x80135848 to 0x80136FF0): VS CSS overlay, `mnplayersvs.c`. Confidence medium. The handicap render function name (`mnPlayersVSMakeHandicapLevelValue`) matches the hook content (handicap icons, CPU level 10 "1"/"0" digits), but I did not verify the overlay load table.
4. Internal jump targets inside the hooks (`j 0x801392B8`, `j 0x80136B5C`, `jal 0x80132564`, ...) agree with the ftcomputer mapping for every ft hook.

Note: `grep` for the raw address text in decomp/src gave only false positives (`0x801398A0` etc. match unrelated constants). Use the comment-based search instead.

---

## 1. Toggles and gating (important for T7.1)

| Toggle | Label used by macro | Title in ROM | Where AI.asm reads it (this share) | Notes |
|---|---|---|---|---|
| Improved AI | `gmRemixToggle(improved_ai)` -> `REMIX_SYM_Toggles__entry_improved_ai` (remix_symbols.h:4068) | `"Improved AI"` (Toggles.asm:2428, default TRUE) | only `cpu_di_` (P11), read at +4 of the entry | Gates DI for non-lv10 CPUs only. lv10 CPUs always DI. |
| Default CPU LVL (V.S.) | `gmRemixToggle(default_cpu_level)` (decomp gm/gmremixspawnsetup.c:255) | `"Default CPU LVL (V.S.)"` (Toggles.asm:2403) | not read in this share | Sets the CPU level byte (fighter +0x13), which is what all lv10 hooks test. |
| Remix 1P mode flag | not a toggle: `SinglePlayerModes.singleplayer_mode_flag == 4` | n/a | `cpu_di_` | Forces advanced DI in Remix 1P regardless of Improved AI. |

Correction to the brief: `gmRemixToggle` takes the Toggles **label** (for example `improved_ai`), not the title string. `gmremixtoggle.h:16` expands it to `REMIX_SYM_Toggles__entry_##name`. The title strings are only used by the menu.

Gating summary for the 25 hooks:
- No toggle gate: P1-P10, P12-P25. The gate is the fighter CPU level byte (fighter +0x13) equal to 10 (or Giga Bowser for P8/P9, see below). Level 10 is only reachable through Default CPU LVL or the CSS. So they are dormant unless a CPU is at level 10.
- VS CSS hooks (P12-P16) are unconditional when the VS CSS is shown.
- Improved AI gates only the non-lv10 part of P11.
- P8/P9 (`stop_roll_spam_`) also run for **Giga Bowser at any CPU level** (`beq at, GBOWSER` skips the level check). This is not an lv10-only patch.

Unpatched readers of CPU level listed by AI.asm (AI.asm:7275-7310, potential future clamp sites, not in scope of any hook): 80131EEC, 80133830, 80133A38, 80133A6C, 80133A78, 80133A94, 801355E8, 80135684, 8013569C, 80135734, 80135744, 80135784, 80135E38, 801360DC, 80136E30, 80136E98, 80136EC0, 801370B0, 80137364, 801373B0, 801373FC, 801375E8, 80137610, 80137900, 801381B8, 801389C4, 80138BE4, 8013908C, 801390B4, 80139138, 80139160, 8013940C, 80139934, 80139EB0, 8013A964.

Constant to keep in mind: `FTCOMPUTER_LEVEL_MAX` = 9 (decomp/src/ft/ftdef.h:4). The C code already computes `FTCOMPUTER_LEVEL_MAX - fp->level` (ftcomputer.c:3693, 3695, 3716, 3718, 3979, 5997, 6020, 6026), which is -1 at level 10. The clamp hooks exist for that reason. The C port must clamp these reads too, or the new level 10 will produce negative waits.

---

## 2. Inventory summary

| ID | AI.asm lines | Scope | ROM | RAM (vanilla) | Decomp function (file:line) | Confidence | Diff |
|---|---|---|---|---|---|---|---|
| P1 | 5157-5176 | extend_attack_check_range_2 | 0xB42E0 | 0x801398A0 | func_ovl3_801397F4 (ft/ftcomputer.c:7331) | confirmed (`801397F4+AC`) | S |
| P2 | 5182-5440 | fix_movement_prediction | 0xADBC4 | 0x80133184 | ftComputerCheckDetectTarget (ftcomputer.c:3903) | confirmed (`80132EC8+2BC`) | L |
| P3 | 5447-5483 | fix_multi_doublejump_usage | 0xAFF90 | 0x80135550 | ftComputerFollowObjectiveWalk (ftcomputer.c:5006) | confirmed (`80134E98+6B8`) | S |
| P4 | 5486-5938 | weight_attack_options | 0xAE1C4 | 0x80133784 | ftComputerCheckDetectTarget (ftcomputer.c:3903) | confirmed (`80132EC8+8BC`) | L |
| P5 | 5943-5973 | skip_repeated_input_check | 0xAE590 | 0x80133B50 | ftComputerCheckDetectTarget (ftcomputer.c:3903) | confirmed (`80132EC8+C88`) | S |
| P6 | 5979-6009 | skip_repeated_input_skip | 0xAE8D8 | 0x80133E98 | ftComputerCheckDetectTarget (ftcomputer.c:3903) | confirmed (`80132EC8+FD0`) | S |
| P7 | 6015-6034 | no_attack_when_all_odds_eq_zero | 0xAEA04 | 0x80133FC4 | ftComputerCheckDetectTarget (ftcomputer.c:3903) | confirmed (`80132EC8+10FC`) | S |
| P8 | 6039-6043 | stop_roll_spam_ (hook 1) | 0xB3CD4 | 0x80139294 | func_ovl3_80138EE4 (ftcomputer.c:7074) | by comment | L (shared with P9) |
| P9 | 6044-6047 | stop_roll_spam_ (hook 2) | 0xB3CE4 | 0x801392A4 | func_ovl3_80138EE4 (ftcomputer.c:7074) | by comment | (see P8) |
| P10 | 6365-6391 | shield_drop_ | 0xB057C | 0x80135B3C | ftComputerFollowObjectiveWalk (ftcomputer.c:5006) | by comment, no AI label | S |
| P11 | 6533-6812 | cpu_di_ | 0xB4E08 | 0x8013A3C8 | ftComputerProcessObjective (ftcomputer.c:7667) | by comment | L |
| P12 | 6822-6839 | handle_right_arrow_press_ | 0x133AC8 | 0x80135848 | mnPlayersVSCheckHandicapArrowInRangeAll (mn/mnplayers/mnplayersvs.c:2495) | medium (overlay) | S |
| P13 | 6842-6844 | VS_CSS left-arrow redirect | 0x134D44 | 0x80136AC4 | mnPlayersVSArrowThreadUpdate (mnplayersvs.c:3192) | medium | S |
| P14 | 6848-6872 | render_right_arrow_ | 0x134DAC | 0x80136B2C | mnPlayersVSArrowThreadUpdate (mnplayersvs.c:3192) | medium | S |
| P15 | 6877-6880 | render_10_._render_1 | 0x135210 | 0x80136F90 | mnPlayersVSMakeHandicapLevelValue (mnplayersvs.c:3311) | medium | L |
| P16 | 6883-6889 | render_10_._render_0 | 0x135270 | 0x80136FF0 | mnPlayersVSMakeHandicapLevelValue (mnplayersvs.c:3311) | medium | M |
| P17 | 7104-7118 | clamp_level_9_1 | 0xACFD0 | 0x80132590 | ftComputerSetCommandWaitShort (ftcomputer.c:3686) | confirmed (`80132564+2C`) | S |
| P18 | 7121-7135 | clamp_level_9_2 | 0xB2ED0 | 0x80138490 | func_ovl3_8013837C (ftcomputer.c:6757) | by comment | S |
| P19 | 7139-7153 | clamp_level_9_3 | 0xAD0A4 | 0x80132664 | ftComputerSetCommandWaitShort (ftcomputer.c:3686) | confirmed (`80132564+100`) | S |
| P20 | 7156-7170 | clamp_level_9_4 | 0xB3568 | 0x80138B28 | func_ovl3_80138AA8 (ftcomputer.c:6963) | by comment | S |
| P21 | 7173-7187 | clamp_level_9_5 | 0xAD1E4 | 0x801327A4 | ftComputerSetCommandWaitLong (ftcomputer.c:3709) | by comment | S |
| P22 | 7190-7204 | clamp_level_9_6 | 0xAD2C4 | 0x80132884 | ftComputerSetCommandWaitLong (ftcomputer.c:3709) | by comment | S |
| P23 | 7207-7226 | clamp_level_9_7 | 0xADAA0 | 0x80133060 | ftComputerCheckDetectTarget (ftcomputer.c:3903) | by comment | S |
| P24 | 7229-7250 | clamp_level_9_8 | 0xB17AC | 0x80136D6C | ftComputerGetObjectiveStatus (ftcomputer.c:5976) | by comment | S |
| P25 | 7255-7273 | clamp_level_9_9 | 0xB340C | 0x801389CC | func_ovl3_8013877C (ftcomputer.c:6825) | confirmed (`8013877C+250`) | S |
| T1 (table) | 6499-6520 | Fox_USP_ (`ai_attack_prevent` entries FOX, JFOX, SLIPPY, FALCO -> PREVENT_ATTACK.ROUTINE.FOX_USP) | n/a | n/a | n/a (data table) | n/a | S |

Difficulty count (25 entries, T1 excluded): S = 18 (P1, P3, P5, P6, P7, P10, P12, P13, P14, P17-P25), M = 1 (P16), L = 6 (P2, P4, P8, P9, P11, P15). The 5 hardest: P4, P2, P11, P8/P9, P15.

---

## 3. Per-patch entries

### P1. extend_attack_check_range_2 (AI.asm:5157-5176)
- Scope: `func_ovl3_801397F4` attack check range.
- Vanilla: 0x801398A0 (ftcomputer overlay). Decomp: func_ovl3_801397F4, ftcomputer.c:7331 (comment at 7331 is `0x801397F4`).
- Change: for CPU level 10, the base of the view range (f10) is 2000.0 instead of 1200.0. The random part (`f8`, random*300) is unchanged. Hook re-executes `lwc1 f4, 0x68(v1)` (distance to target) at entry, and `add.s f16, f8, f10` runs as the final instruction (delay slot of `j _return`).
- Data: fighter +0x13 (cpu level byte). No tables.
- Cross-patch deps: none in this share.
- Proposed C hook: `f32 ftRemixAiAttackCheckRangeBase(FTStruct *fp)` returns `2000.0F` if `fp->level == 10`, else `1200.0F`. Call at the `add.s f16, f8, f10` site in func_ovl3_801397F4, under `#if defined(PORT) && defined(REGION_REMIX)`.
- Difficulty: S.
- Risks: none found. Clamp is not needed here because the level is compared, not indexed.

### P2. fix_movement_prediction (AI.asm:5178-5440)
- Scope: `ftComputerCheckDetectTarget` (vanilla 0x80132EC8 region), patched at 0x80133184 (AI comment `80132EC8+2BC`). Decomp: ftcomputer.c:3903 (`0x80132EC8`).
- Change for lv10 (other levels run the original `lwc1 f10, 0x18c(sp)` path):
  1. If the current action is Dash, Run or RunBrake, only the input commands GRAB, USMASH, USPG, NSPG and DASH_ATTACK are allowed. Anything else jumps to `0x80132EC8+0xB50` (skip this move).
  2. Predicts own X position over `hit_frame` frames, with ground friction (friction table lookup) when grounded and 0 when airborne. Predicts own Y with gravity, max fall speed and ground clamp (`fp+0xF0` distance to ground, `fp+0xEC` clipping below).
  3. Predicts target X without friction (vanilla behaviour), and target Y with gravity and ground clamp.
  4. Stores the predicted values to stack `0x170(sp)` (X) and `0x16C(sp)` (Y), then jumps to `0x80132EC8+0x534` with `is_attempt_cliffcatch = FALSE`.
- Data read: `Surface.friction_table` (Remix version of dMPCollisionMaterialFrictions); fighter +0x14C (grounded), +0xF4 (ground attribute, masked with 0xFFFF00FF, shifted by 2), +0x9C8 (attributes pointer, +0x24 character friction), +0xF0, +0xEC; target struct at `0x1B0(sp)` / `0x1FC(a0)`; stack locals 0x180-0x1AC (target and own pos/vel/gravity/max fall speed); Action.Dash/Run/RunBrake; AI.ATTACK_TABLE.GRAB.INPUT, USMASH.INPUT, USPG.INPUT, NSPG.INPUT; AI.ROUTINE.DASH_ATTACK.
- Cross-patch deps: P4 (weight_attack_options) and P23 use the same locals family (0xBC chance array). P2 output (0x170/0x16C) is consumed by vanilla code after `0x80132EC8+0x534`, so the C version must produce the same two stack values in the same struct.
- Proposed C hook (split by responsibility):
  - `Vec2f ftRemixAiPredictSelfPos(FTStruct *fp, s32 hit_frame)` (X with friction, Y with gravity).
  - `Vec2f ftRemixAiPredictTargetPos(FTStruct *target, s32 hit_frame)` (X linear, Y with gravity).
  - `sb32 ftRemixAiCanUseMoveWhileRunning(s32 input_command)` (the running whitelist).
  - Call sites inside ftComputerCheckDetectTarget at the point where vanilla computes the predicted position.
- Difficulty: L (long loops, two friction sources, stack-based outputs).
- Risks:
  - Ground friction index mask `0xFFFF00FF` keeps bits 16-31. If those bits are non-zero for some ground types the table read goes out of range. Verify against vanilla before porting.
  - Loop for X runs `hit_frame` times with sign-dependent friction. Keep the exact crossing rule (`c.le.s` against zero) or results differ at low speeds.

### P3. fix_multi_doublejump_usage (AI.asm:5446-5483)
- Scope: `ftComputerFollowObjectiveWalk`, patched at 0x80135550 (AI comment `80134E98+6B8`). Decomp: ftcomputer.c:5006.
- Change: for lv10, a CPU below the ledge may use another jump when the current `proc_update` (fighter +0x9D4) is `ftCommonJumpAerialProcUpdate` (0x8013FB00, fighter overlay), not only when the status is `ftStatus_Common_FallSpecial` (0x3A). Result jumps to `+0x6C0` (allow) or `+0x6D4` (skip). If the proc check fails, it runs the original status check.
- Data: fighter +0x9D4 (proc_update), status id from v0 (vanilla), constant 0x8013FB00 (function pointer), constant 0x3A.
- Deps: none.
- Proposed C hook: `sb32 ftRemixAiCanJumpWhileFalling(FTStruct *fp, s32 status_id)` returns TRUE when `fp->level == 10 && fp->proc_update == ftCommonJumpAerialProcUpdate`, or when `status_id == ftStatus_Common_FallSpecial`. Call at the vanilla decision point.
- Difficulty: S.
- Risks: the function pointer compare needs the decomp symbol for ftCommonJumpAerialProcUpdate. The hook writes `at` before reading it (`lli at, 0x3A` in the delay slot of `beq`). Behaviour is intended; no bug found.

### P4. weight_attack_options (AI.asm:5485-5938)
- Scope: `ftComputerCheckDetectTarget` (AI comment `80132EC8+8BC`), patched at 0x80133784. Decomp: ftcomputer.c:3903. This is the largest hook in the share (about 450 lines of asm).
- Change for lv10:
  1. Sets the chance for this move to 1.0 (stored in `0xBC(sp+a0)`) and saves the input type to `0x10C(sp+a0)`.
  2. Landing discard: if the move starts after the CPU would land (own predicted landing, using the same gravity loop as P2), chance = 0.
  3. Opponent-landing discard: if the opponent is airborne and would land (tech window: `DamageFlyHigh` .. `Tumble`) before the hitstun ends, the move is discarded when it would otherwise combo.
  4. Combo logic: if the opponent is in hitstun and the move would hit in hitstun (start frame <= hitstun - 2), chance is multiplied by 1000. A random roll `rand(hitstun_diff)` can discard combo moves when much hitstun remains.
  5. True block strings: if the opponent is in ShieldStun and the move would hit before they can release, a combo bonus applies. Grab (input 28, `AI.ROUTINE.DASH_GRAB`) gets x1000 when the opponent is Shield, ShieldStun or ShieldOn.
  6. Non-combo moves: start frame >= 30 / 20 / 15 is discarded with probability 1 - 1/64, 1 - 1/32, 1 - 1/8 respectively, using `Global.get_random_float`.
  7. Slow-move weighting by opponent state: "advantage" states (stun, sleep, tumble down, egg, capture) multiply by start frame^2; opponent offstage multiplies by start frame^2; rolls/tech multiply by start frame; otherwise chance / start frame^2. Opponent at >150% always normal; >90% uses the small-advantage branch.
  8. Calls `_custom_weight_table` (see risk R4) which calls the per-character `Character.cpu_attack_weight.table[char_id]` function with the chance in f2, then stores the result back.
  9. Returns to `0x80132EC8+0xB4C` (lv10) with `s1 += 1` in the delay slot.
- Data read: fighter +0x8 (character id), +0x13, +0x9C8 (not used here), +0x14C (grounded), +0xEC (clipping below), +0xF0 (ground distance), +0x1CC+0x6C = +0x238 (opponent struct pointer); opponent +0x18C (bit 0 = hitstun), +0x40 and +0xB18 (hitstun timers), +0x24 (action), +0x2C (damage %), +0xB34 (shield stun float), +0xEC, +0x14C; Action.DamageFlyHigh, Tumble, ShieldStun, Shield, ShieldOn, ShieldBreak, ShieldBreakFall, DownBounceD/U, StunLandD/U, StunStartD/U, Stun, Sleep, FallSpecial, EggLay, EggLayPulled, CapturePulled, RollF, RollB, Tech, TechF, TechB, Action DamageFlyRoll and DamageHigh1 (range); input command ids 28 and AI.ROUTINE.DASH_GRAB; Global.get_random_int_ and Global.get_random_float; `Character.cpu_attack_weight.table` (Character.asm:2949; Falcon custom function in captainshared.asm:613).
- Stack slots: chance array at `sp + a0 + 0xBC` (a0 = `s1 << 2`), input type at `sp + a0 + 0x10C`.
- Cross-patch deps: P5, P6, P7 (same function, same chance array, zeroing discards the move). P2 for own landing prediction semantics. Custom weight table is consumed by Character.asm and captainshared.asm.
- Proposed C hook (one function per rule set, all `static`, all under `#if defined(PORT) && defined(REGION_REMIX)`):
  - `f32 ftRemixAiLandingDiscardChance(FTStruct *fp, s32 start_frame, f32 chance)`
  - `f32 ftRemixAiOpponentLandingDiscardChance(...)`
  - `f32 ftRemixAiComboChance(...)`
  - `f32 ftRemixAiSlowMoveChance(...)`
  - `f32 ftRemixAiShieldChance(...)`
  - `f32 ftRemixAiWeightAttackOption(FTStruct *fp, s32 index, f32 chance)` as the single entry that calls the others in the same order as the asm.
  - Keep the RNG call order identical to the asm if replay determinism matters.
- Difficulty: L.
- Risks:
  - R4a. Custom weight table is applied for **all** CPU levels (`_end` path also calls `_custom_weight_table`). Only the lv10 rule set is gated. Confirm if intentional.
  - R4b. Loop counter: lv10 path does `addiu s1, s1, 1` then jumps to `+0xB4C`, while the non-lv10 path returns to `_return` and lets vanilla increment. Verify the vanilla code at `+0xB4C` does not increment again.
  - R4c. Two different hitstun fields are read: `0x40(t4)` in the combo check and `0xB18(t4)` in the opponent landing check. Both are described as "hitstun timer" in comments. Confirm the field semantics in the decomp struct before porting.
  - R4d. `Global.get_random_int_(hitstun_diff)` is only reached when `hitstun_diff >= 2`, so no divide-by-zero. OK.
  - R4e. Uses `jal` inside the middle of vanilla function. ra is clobbered; this is safe only if the vanilla prologue saves ra to its stack frame (the no_attack hook restores `0x54(sp)`, so it does).

### P5. skip_repeated_input_check (AI.asm:5942-5973)
- Scope: `ftComputerCheckDetectTarget`, patched at 0x80133B50 (AI comment `80132EC8+C88`).
- Change: for lv10, the switch on the "recently used input" table is skipped and execution goes to the default branch at `+0xF60`. Non-lv10 runs the original `beqz at, +0xF60` and `sll t5, t5, 2`.
- Data: t5 (switch index), at (set by vanilla before the hook).
- Deps: P6 and P7 (same repeated-input mechanism).
- Proposed C hook: `sb32 ftRemixAiSkipRepeatedInputCheck(FTStruct *fp)` returns TRUE for lv10. The C version jumps to the default case.
- Difficulty: S.
- Suspected asm bug (B1): when the original `beqz at, +0xF60` is taken, its delay slot `sll t5, t5, 2` executes once in `_original` and again in `_b_f60` (`j +0xF60; sll t5, t5, 2`). t5 ends up shifted by 4 instead of 2 on that path. Impact is probably nil if `+0xF60` does not read t5. Verify in vanilla, then decide whether the port should fix it.

### P6. skip_repeated_input_skip (AI.asm:5979-6009)
- Scope: `ftComputerCheckDetectTarget`, 0x80133E98 (AI comment `80132EC8+FD0`).
- Change: lv10 never performs the "same move used 4 times in a row, jump instead" check. It always runs `sb r0, 0x37(v1)` and jumps to `+0x1008`. Non-lv10 keeps `bnel t5, t6, +0x1008`.
- Data: t5, t6 (input comparison), v1 (repeated-input struct), offset 0x37.
- Deps: P5, P4.
- Proposed C hook: `sb32 ftRemixAiSkipFourInARowCheck(FTStruct *fp)` returns TRUE for lv10, plus the same store to the repeated-input counter when skipping.
- Difficulty: S.
- Risks: the `sb r0` is executed twice on the taken path (idempotent, harmless).

### P7. no_attack_when_all_odds_eq_zero (AI.asm:6015-6034)
- Scope: `ftComputerCheckDetectTarget`, 0x80133FC4 (AI comment `80132EC8+10FC`).
- Change: lv10 returns FALSE (`v0 = 0`) and jumps to `+0x1108`. Non-lv10 jumps to `+0x1108` with `lw ra, 0x54(sp)` only. The `lw ra` in the patch's delay slot runs in both paths.
- Deps: P4 (chance zeroing), P5, P6.
- Proposed C hook: `sb32 ftRemixAiAllOddsZeroResult(FTStruct *fp, sb32 vanilla_result)` returns FALSE for lv10.
- Difficulty: S.
- Risk: the name suggests the return value means "no attack possible". The lv10 forcing to FALSE therefore means "always consider attacks". Confirm the vanilla return semantic at `+0x1108` before naming the C function.

### P8 and P9. stop_roll_spam_ (AI.asm:6039-6047 patches, body at 6049-6307)
- Scope: `func_ovl3_80138EE4` (ftcomputer.c:7074), two patch points at 0x80139294 and 0x801392A4 that jump into one hook body. Both keep the delay slot "line 2". The hook's end jumps back to `0x801392B8` with `v0 = 1` (command issued) or `v0 = 0` (skip).
- Change (only when the CPU is at lv10, or the fighter is Giga Bowser at any level):
  - Gating: if the current action is above `TurnRun`, issue `AI.ROUTINE.RESET_STICK` (stop stick input). Otherwise look up `Character.close_quarter_combat.table[char_id]` and jump to the per-character handler:
    - Ness: `NESS_DJC_NAIR`.
    - Yoshi: `SHIELD_DROP`.
    - Lucas: 1/2 chance to shine (`MULTI_SHINE` family) decided with `Global.get_random_int_`.
    - Fox, Falco, Wolf, Slippy: shine when the opponent shields or is shield-broken, else F-smash if the opponent is shield-broken; direction handled with `MULTI_SHINE` / `MULTI_SHINE_TURNAROUND`.
    - Kirby (and copies): UTILT if the opponent is behind within 260.0 units; otherwise short-hop tries.
    - Puff (Jigglypuff and copies): rest-based logic (skip if opponent below 30%), DSP or PUFF_SHORT_HOP_DAIR within 260.0.
    - Table entry NULL: falls through to a random short-hop/pivot table (advantage: 2/15 short hop; not advantage: 1/15 roll, 3/15 short hop in place/towards/away, 4/15 pivot away).
  - Non-lv10 non-Gbowser: no change (returns through `_end` with the vanilla command).
- Data: fighter +0x8 (character id), +0x13 (cpu level), +0x24 (action), +0x44 (facing), +0x46 (current direction, lh), +0x48(sp) (saved fighter pointer), +0x1FC (target pointer), target +0x84 (struct), target +0x24 (action), +0x2C (damage), +0x5BA (tangibility); Action.TurnRun, Idle, Shield, ShieldStun, ShieldOn, ShieldBreak, Sleep; AI.ROUTINE: RESET_STICK, NESS_DJC_NAIR, SHIELD_DROP, SMASH_FORWARD, MULTI_SHINE, MULTI_SHINE_TURNAROUND, UTILT, DSP, PUFF_SHORT_HOP_DAIR, SHORT_HOP_IN_PLACE, SHORT_HOP_TOWARDS, SHORT_HOP_AWAY, PIVOT_AWAY, DASH_ATTACK-related; command ids 0x2D, 0x2E (rolls), 0x2E for rolling forward; `Character.close_quarter_combat.table` (Character.asm:2912); constant UTILT_DISTANCE 0x4382 (= 260.0F). The "character table patch" lines in 6309-6360 are commented out, so the table values come from Character.asm.
- Deps: calls `0x80132564` (ftComputerSetCommandWaitShort, patched by P17 and P19). Overlaps with P16/P17 only through that call. The Fox_USP_ table patch (T1) changes which attacks Fox can choose.
- Proposed C hook: `s32 ftRemixAiCloseQuarterCommand(FTStruct *fp, s32 default_command)` returning the command or `FTREMIX_AI_NO_COMMAND`. Then a per-character `static s32 ftRemixAiCloseQuarter<Name>(...)` function selected through a `switch` on `fp->fkind`. Keep the RNG order and fighter-specific branches identical.
- Difficulty: L (about 230 lines of asm, 10+ character branches, mixed float and integer checks).
- Risks:
  - R8a. Giga Bowser runs this at every CPU level (`beq at, Character.id.GBOWSER` skips the level check). Keep that behaviour when porting.
  - R8b. Fox branch: the direction register `t6` is set in a delay slot and used in `bnel`. Read carefully when porting.
  - R8c. `jal Global.get_random_int_` clobbers ra and a0; the code restores a0 from `0x48(sp)`. C version should not depend on those.

### P10. shield_drop_ (AI.asm:6365-6391)
- Scope: `ftComputerFollowObjectiveWalk` region (0x80135B3C). Not labelled by AI.asm. Decomp: ftcomputer.c:5006. Vanilla continuation: `0x80135B68` with `lw ra, 0x24(sp)`.
- Change: lv10 CPUs issue `AI.ROUTINE.SHIELD_DROP` only when their action is `Idle`. Otherwise (and for non-lv10) they issue `AI.ROUTINE.STICK_DOWN`. Both go through `0x80132564` (ftComputerSetCommandWaitShort).
- Data: fighter +0x13, +0x24 (action), Action.Idle, AI.ROUTINE.SHIELD_DROP, AI.ROUTINE.STICK_DOWN.
- Deps: calls P17 and P19 (ftComputerSetCommandWaitShort clamp hooks).
- Proposed C hook: `s32 ftRemixAiDownCommand(FTStruct *fp)` returns SHIELD_DROP for lv10 + Idle, else STICK_DOWN. Call site: the vanilla `ftComputerSetCommandWaitShort(fp, command)` call in the shield-drop path.
- Difficulty: S.
- Risk: the hook uses `jal` and restores ra from `0x24(sp)`. The vanilla prologue must save ra at that slot. Verify when porting.

### P11. cpu_di_ (AI.asm:6533-6812)
- Scope: `ftComputerProcessObjective` (ftcomputer.c:7667, comment 0x8013A38C). The patch is at 0x8013A3C8 (`lui at, 0x8019` original line 1 as the `jal` delay slot). Vanilla continuation at 0x8013A49C is used to skip the vanilla AI jump table when DI is set.
- Change: adds DI (directional influence) for CPUs in hitstun. Behaviour:
  1. Per-port combo memory in `cpu_di_parameters` (4 ports x [type, strength, direction, advanced index]). Reset to -1 when the hit count is 0, otherwise kept for the whole combo.
  2. Hit-count gate: in a combo, DI runs when the hit count is >= 2 (normal) or on first hit only after 10 frames of hitstun (normal); training uses `Training.entry_di_first_hit`.
  3. Training screen (screen id 0x36): uses the Training DI menu values (`entry_di_type`, `entry_di_strength`, `entry_di_direction`), with random selections rolled once per combo.
  4. Non-training, lv10: always DI with a random advanced index 0..8 (0..3 slide-high, 4..7 smash-high path, 0 = none), independent of Improved AI.
  5. Non-training, other CPUs: DI only if `Toggles.entry_improved_ai` is non-zero **and** match gamemode is not CLASSIC, or if the Remix 1P mode flag is 4 (automatic advanced AI).
  6. When DI applies: writes the routine pointer into the CPU struct (+0x8), sets the controller command wait timer (+0x7) to 1, and sets return address to 0x8013A49C. When not, returns normally.
- Data read:
  - `Global.match_info` (+0x74 block per port, hit count address), `Global.match_info` gamemode byte at +0x0, `Global.GAMEMODE.CLASSIC`.
  - `Global.current_screen` (training screen 0x36).
  - `Training.entry_di_first_hit`, `entry_di_type`, `entry_di_strength`, `entry_di_direction` (+0x4 values).
  - `SinglePlayerModes.singleplayer_mode_flag` == 4.
  - `Toggles.entry_improved_ai` (+0x4).
  - CPU struct a1: +0xD (port), +0x13 (level), +0x1C (frame count), +0x24 (action), +0x40 (hitstun), +0x44 (facing: 1 right, -1 left).
  - Tables `smash_di_table` (12 routines, `DI_SMASH_HIGH/MEDIUM/LOW_{LEFT,RIGHT,UP,DOWN}`) and `slide_di_table` (`DI_SLIDE_*`). Defined in this file (AI.asm:6785-6811), the DI_* constants are defined elsewhere (not checked).
- Cross-patch deps: none inside the share. Shares the ProcessObjective jump-table return address.
- Proposed C hook (the largest logic; split it):
  - `sb32 ftRemixAiDiEnabled(FTStruct *cpu)` (gate rules 4 and 5 above).
  - `FTRemixAiDiPlan ftRemixAiPlanDi(FTRemixAiDiCombo *combo, s32 hit_count, s32 level, sb32 is_training)`.
  - `FTRemixAiDiCombo` replaces `cpu_di_parameters` (one per port), reset when `hit_count == 0`. Avoid a free global; pass it in or put it on the port struct.
  - `void ftRemixAiApplyDi(FTStruct *cpu, FTRemixAiDiPlan plan)` writes the routine and wait timer.
- Difficulty: L.
- Risks:
  - R11a. Intent check: lv10 CPUs DI regardless of Improved AI. Turning the toggle off does **not** remove DI from lv10. Confirm if that is the desired UX.
  - R11b. RNG: up to 4 `Global.get_random_int_` calls per combo, with a fixed order (advanced index, strength, direction). Keep the order for determinism.
  - R11c. `cpu_di_parameters` persists across matches. A combo that ends without hit count reset (for example training exit) keeps stale values until the next hit count of 0. Preserve or deliberately fix.
  - R11d. Training path reads `Training.entry_di_*` values and uses `+0x4` word offsets. Check the menu entry layout in the port before reusing.

### P12. handle_right_arrow_press_ (AI.asm:6822-6839)
- Scope: VS CSS, right-arrow press handler. Vanilla 0x80135848, comment in decomp `mnPlayersVSCheckHandicapArrowInRangeAll` (mnplayersvs.c:2495, comment 0x801357A4). Medium confidence on overlay (see section 0).
- Change: the upper limit for the value under the arrow: Human panel (type 0) -> 40 (handicap max, 0x28). CPU panel (type 1) -> 10 (CPU level max). Original `slti at, t8, X` becomes `slt at, t8, t0` with the limit in t0.
- Data: CSS panel struct s0 (+0x84 type: 0 = HMN, 1 = CPU), current value t8 = `s2` value (`lw t8, 0x0(s2)`).
- Deps: same limit as P14 (duplicate in the asm). P13/P14/P15/P16 are the same CSS feature.
- Proposed C hook: `s32 mnRemixVSArrowLimit(const MNPlayersVSPanel *panel)` returns 40 for human, 10 for CPU. Name uses `mn`, not `ftRemixAi`, because it is CSS code. Replace both P12 and P14 with it.
- Difficulty: S.
- Risks: none significant. Input value must stay in t8 for the `slt`.

### P13. VS_CSS left-arrow redirect (AI.asm:6841-6844)
- Scope: VS CSS, 0x80136AC4, comment in decomp `mnPlayersVSArrowThreadUpdate` (mnplayersvs.c:3192, comment 0x801369E4).
- Change: replaces the single instruction at 0x80136AC4 with `b 0x80136B2C`, which points at the render hook (P14). Patch covers only that one word. The delay slot is the vanilla instruction at 0x80136AC8, which is not restated.
- Deps: P14.
- Proposed C hook: none. In C, call `mnRemixRenderRightArrow(...)` from the same place unconditionally.
- Difficulty: S.
- Risk: the delay slot of the new `b` is whatever was at 0x80136AC8 in vanilla. Confirm it is harmless.

### P14. render_right_arrow_ (AI.asm:6848-6872)
- Scope: VS CSS, 0x80136B2C, comment in decomp `mnPlayersVSArrowThreadUpdate` (mnplayersvs.c:3192).
- Change: computes the upper limit `at` for the panel at `s8 + port * 0xBC` (port in s3): type 1 -> 10, type 0 -> 40. Returns via `jr ra`, then the vanilla `bne s1, at, 0x80136B5C` at +8 uses that `at`. Note the `port*0xBC` computed as `(port * 0x2F) * 4` through shifts.
- Data: s8 (P1 CSS panel base), s3 (port), s1 (current index), panel +0x84 (type).
- Deps: P12, P13.
- Proposed C hook: `sb32 mnRemixVSArrowIsVisible(...)` or reuse `mnRemixVSArrowLimit` from P12.
- Difficulty: S.
- Risk: the `at` value must survive the hook for the `bne`. It is a return contract, not an ABI detail.

### P15. render_10_._render_1 (AI.asm:6875-6899 hook, body at 6891-7043)
- Scope: VS CSS handicap/CPU-level renderer. Patch at 0x80136F90 (the "1" digit or the handicap icon). Decomp: `mnPlayersVSMakeHandicapLevelValue` (mnplayersvs.c:3311, comment 0x80136E90).
- Change:
  - Human handicap >= 10: draws an icon from `handicap.table` (7 icon kinds: YOSHI, KIRBY, POLYGON_TEAM, DK, METAL, MASTER_HAND, SAMUS, each with 5 levels VERY_EASY..VERY_HARD; SAMUS is NONE). Icon from the character's stock icon footer (attr +0x340) or from the CSS images file (`File.CSS_IMAGES`) for polygon/metal/master hand, with `CharacterSelect.VARIANT_ICON_OFFSET`. Adds a coloured rectangle for the 1P difficulty (`level_colors`: 5 RGBA values). Then jumps to `0x80136FF8` to skip the number.
  - CPU level 10: uses the "10" texture footer offset `t9` (set at 0x3C(sp)) for the "1" digit.
  - Human handicap < 10 and CPU < 10: returns and lets vanilla draw the number.
- Data read: `0x80116E10` (main character struct table, hard-coded RAM address), `0x8014C4A4` (file pointer 0 base, hard-coded), `Render.file_pointer_2`, `Render.load_file`, `Render.TEXTURE_INIT_`, `File.CSS_IMAGES`, `CharacterSelect.VARIANT_ICON_OFFSET.{POLYGON,METAL,MASTER_HAND}`, CSS panel (stack 0x28(sp)), panel +0x84 (type), handicap.table (`db`), level_colors.
- Deps: P16 (sibling hook, same routine, writes the "0" digit), P12/P14 (limits).
- Proposed C hook: `void mnRemixVSRenderLevelOne(MNPlayersVSPanel *panel, s32 level_value)` and `void mnRemixVSRenderHandicapIcon(...)`. Keep the texture creation order.
- Difficulty: L (texture lookups, stack-based render setup, runtime file load).
- Risks:
  - R15a. Hard-coded RAM addresses `0x80116E10` and `0x8014C4A4` must be replaced by decomp symbols.
  - R15b. `Render.load_file` runs inside the render path. In the port, file access must not block the render loop differently than the N64 code. Keep the same load order.
  - R15c. The stack trick in `_draw_icon` (`addiu sp, sp, -0x30` in the `jal` delay slot) is a real frame reserve; the C version should use a normal call.

### P16. render_10_._render_0 (AI.asm:6882-6889 hook, body at 7014-7043)
- Scope: VS CSS, 0x80136FF0 (the "0" digit when level is 10). Decomp: `mnPlayersVSMakeHandicapLevelValue` (mnplayersvs.c:3311).
- Change: for CPU level 10 only: creates a texture for the "0" digit from file 0 (`0xC4A4` offset from `0x8014`) with the footer offset `t9` (0x38(sp)), copies y position and image flags from the "1" object, and sets x = "1".x + 8.0 (0x41000000). Then returns with the vanilla `lw ra, 0x1C(sp); jr ra; addiu sp, sp, 0x60` epilogue (its own `jr ra`).
- Data: `0x8014C4A4` (hard-coded), `Render.TEXTURE_INIT_`, stack 0x2C(sp) (CPU level), 0x34(sp) (CPU level object), 0x38(sp) (footer offset), "1" image object (+0xC of the texture struct, +0x5C, +0x58, +0x24).
- Deps: P15 (creates the "1" digit object this hook reads).
- Proposed C hook: `void mnRemixVSRenderLevelZero(MNPlayersVSPanel *panel, ...)`. Name follows the decomp CSS naming.
- Difficulty: M.
- Risks: saves ra at 0x5C(sp) inside the caller frame. The C version does not need it. Depends on the "1" object existing (P15 must have run first in the same frame).

### P17. clamp_level_9_1 (AI.asm:7104-7118)
- Scope: `ftComputerSetCommandWaitShort`, vanilla 0x80132590 (AI comment `80132564+2C`). Decomp ftcomputer.c:3686.
- Change: for lv10, the level used by the rest of the function is 9 (t7 = 9). Original line 3 `addiu v1, a0, 0x1CC` is executed in the hook.
- Data: fighter +0x13 (level), +0x1CC (offset used later).
- Deps: none. Affects P19/P21/P22 through the same function (independently).
- Proposed C hook: `s32 ftRemixAiClampedLevel(const FTStruct *fp)` returns `fp->level < FTCOMPUTER_LEVEL_MAX + 1 ? fp->level : FTCOMPUTER_LEVEL_MAX`. Use it in the four `FTCOMPUTER_LEVEL_MAX - fp->level` expressions in this function (ftcomputer.c:3693, 3695, 3716, 3718).
- Difficulty: S.
- Risks: none.

### P18. clamp_level_9_2 (AI.asm:7121-7135)
- Scope: `func_ovl3_8013837C` (ftcomputer.c:6757, comment 0x8013837C), vanilla 0x80138490. AI.asm comment: "occurs often".
- Change: same clamp (t7 = 9 for lv10) and original line 3 `addiu t0, r0, 1`.
- Deps: none.
- Proposed C hook: same `ftRemixAiClampedLevel`.
- Difficulty: S.
- Risks: none.

### P19. clamp_level_9_3 (AI.asm:7139-7153)
- Scope: `ftComputerSetCommandWaitShort`, vanilla 0x80132664 (AI comment `80132564+100`).
- Change: clamp t2 to 9 for lv10 before `subu v0, t3, t2` (original line 3). The subtraction uses the clamped value.
- Deps: P17.
- Proposed C hook: same `ftRemixAiClampedLevel`.
- Difficulty: S.
- Risks: the clamp applies to the subtraction operand, so the result differs from unclamped lv10 arithmetic. Intended.

### P20. clamp_level_9_4 (AI.asm:7156-7170)
- Scope: `func_ovl3_80138AA8` (ftcomputer.c:6963), vanilla 0x80138B28.
- Change: same clamp on t8 (level), then original line 3 `addiu t2, r0, 1`.
- Deps: none.
- Proposed C hook: same `ftRemixAiClampedLevel`.
- Difficulty: S.
- Risks: none.

### P21. clamp_level_9_5 (AI.asm:7173-7187)
- Scope: `ftComputerSetCommandWaitLong`, vanilla 0x801327A4 (ftcomputer.c:3709; no AI comment, "?" in the asm).
- Change: same clamp on t7, then original line 3 `lui at, 0x3F80` (1.0F constant).
- Deps: P22.
- Proposed C hook: same `ftRemixAiClampedLevel`.
- Difficulty: S.
- Risks: the `at` set here is the 1.0F constant. The C version must not assume `at` is a level.

### P22. clamp_level_9_6 (AI.asm:7190-7204)
- Scope: `ftComputerSetCommandWaitLong`, vanilla 0x80132884.
- Change: same clamp on t2, then original line 3 `addiu t6, r0, 1`.
- Deps: P21.
- Proposed C hook: same `ftRemixAiClampedLevel`.
- Difficulty: S.
- Risks: none.

### P23. clamp_level_9_7 (AI.asm:7207-7226)
- Scope: `ftComputerCheckDetectTarget`, vanilla 0x80133060 (AI comment: "while near an opponent").
- Change: for lv10, the level is 9 (t8 = 9) and the collision size constant changes from 1.0F (0x3F80) to 0.875F (0x3F60), "underestimate collision size" per the asm comment.
- Data: fighter +0x13, the scaling constant used by the vanilla code (not restated).
- Deps: none in this share.
- Proposed C hook: `f32 ftRemixAiNearCollisionScale(const FTStruct *fp)` returns 0.875F for lv10, 1.0F otherwise. Plus `ftRemixAiClampedLevel`.
- Difficulty: S.
- Risk: the vanilla use of the constant is not restated in this hook. Check the vanilla use before naming.

### P24. clamp_level_9_8 (AI.asm:7229-7250)
- Scope: `ftComputerGetObjectiveStatus`, vanilla 0x80136D6C (ftcomputer.c:5976, comment 0x80136D0C). AI.asm: "while hanging on cliff".
- Change: for lv10, t9 is clamped to 9 and `a1` is set to 9 (original line 3). Also, a `bc1fl` on the FP condition from the vanilla compare before the hook: `sll v1, v1, 1` runs when the FP condition is **true** (branch-likely nullifies the delay slot). Preserved as-is in the asm.
- Data: fighter a2 (+0x13), FP condition code set by vanilla.
- Deps: none.
- Proposed C hook: `s32 ftRemixAiClampedLevel` for the level. Keep the FP-dependent `v1 <<= 1` as a plain `if (vanilla_fp_condition) v1 <<= 1;` in the C version. The condition must come from the same compare.
- Difficulty: S.
- Risks:
  - R24a. The `bc1fl` depends on FP condition code from a compare **before** the hook. The C port must keep that compare. Verify in vanilla.
  - R24b. The hook reads `lbu t9` twice (delay slot and again after `slti a1`). Behaviour is the same as one read.

### P25. clamp_level_9_9 (AI.asm:7255-7273)
- Scope: `func_ovl3_8013877C` (ftcomputer.c:6825, comment 0x8013877C). AI comment: "compares some timer to ((-this_fp->level * 2) + 18)). At lv9 this results in 0, but at lv10 it's -2. Clamp."
- Change: `t3 = (-level * 2) + 18`, clamped to 0 if negative (`bgezl t3` / `lli t3, 0`).
- Deps: none.
- Proposed C hook: `s32 ftRemixAiTimerThreshold(const FTStruct *fp)` returns `max(0, 18 - 2 * fp->level)`. Use at the vanilla comparison.
- Difficulty: S.
- Risks: none.

### T1. Fox_USP_ (AI.asm:6499-6520, table patch, not `OS.patch_start`)
- Change: `ai_attack_prevent` table entries for FOX, JFOX, SLIPPY and FALCO set to `PREVENT_ATTACK.ROUTINE.FOX_USP`. This stops LV10 Fox-family CPUs from spamming up special. Table data, not code.
- Not gated by toggle or level in the asm (the table is per character). Confirm that the table entry is used only for CPU attacks, not for human players.
- Proposed C: data change in the Remix attack table, not a function hook. Difficulty: S.

---

## 4. Not in this share (for the other half of AI.asm)
- Earlier hooks, lines 1-5157 (fine, not inventoried here).
- Commented: `shorthop_approach` (AI.asm:6397 ROM 0xB04FC, RAM 0x80135ABC) - disabled.
- Commented: `Puff_DSP_` table edits (6449-6472), `Pika_NSP_` (6475-6494), roll-spam jump table entries (6309-6360). The roll-spam table is still in use by P8/P9 through `Character.close_quarter_combat.table` (Character.asm:2912). Those commented entries are the old way to populate it.

---

## 5. Conflicts with the existing doc

`docs/remix/gameplay_a_l.md` lines 127-153 already map these AI.asm rows. Most of its decomp names are non-ftcomputer functions for addresses that AI.asm itself ties to ftcomputer:

- Agree with this inventory: 5158 (P1), 7122 (P18), 7157 (P20).
- Disagree (doc names mn/sc functions, AI.asm comment or internal jumps tie the address to ftcomputer): 5183 (scStaffroll), 5448 (mnPlayers1PGame), 5487 (mnTitle), 5944 (sc1PStageClear), 5980 (mnVSResults), 6016 (mnPlayers1PTraining, the doc's own text says 80132EC8+10FC), 6040/6044 (mnPlayersVSMakePuck), 6366 (mnPlayers1PGameAnnounce), 6534 (mnPlayersVSSetPlayerNot), 7105 (mvEnding), 7140 (mnVSModeAnimateRuleArrows), 7174 (scStaffroll), 7191, 7208, 7230, 7256 (doc says mnPlayersVSPuckProcUpdate, its own text says 8013877C+250).
- Doc's CSS rows 6823/6842/6849/6877/6883 (mnPlayers1PTraining*, mnVSResults*, mnPlayers1PBonus*) disagree with this inventory (mnplayersvs.c). The ROM-offset argument (section 0) favours mnplayersvs.c, but this is medium confidence.

Recommendation: fix or regenerate these rows before T7.1 uses them as the source of truth. Otherwise porting code will be placed into the wrong functions.

---

## 6. Open questions and suspected asm bugs (consolidated)

1. B1 (P5): double `sll t5, t5, 2` on the taken branch of the repeated-input default path. Check whether `+0xF60` reads t5. Impact probably nil.
2. R4a (P4): custom weight table is applied at all CPU levels. Confirm intent.
3. R4b (P4): loop counter `s1` handling differs between the lv10 path (`addiu s1, s1, 1`) and the non-lv10 path. Confirm against vanilla at `+0xB4C`.
4. R4c (P4): two hitstun fields (`0x40` and `0xB18`). Confirm semantics.
5. R11a (P11): lv10 DI ignores Improved AI. Confirm intent.
6. R11c (P11): `cpu_di_parameters` state persists across matches. Confirm reset rules.
7. R24a (P24): depends on an FP condition from before the hook. Verify in vanilla.
8. R7 (P7): meaning of the vanilla return value at `+0x1108`. Confirm.
9. R8a (P8/P9): Giga Bowser always uses the advanced roll-spam path. Keep it.
10. Overlay question (section 0): VS CSS hooks mapped to `mnplayersvs.c` with medium confidence. Confirm the overlay load address.
11. Ground friction mask `0xFFFF00FF` (P2). Check that bits 16-31 are zero for every ground type.
12. Load-delay slots (for example `lw at` followed by `beq at`) appear many times in this share. I did **not** flag them as bugs, because the VR4300 interlocks load-use hazards in hardware.
13. Port-specific: hard-coded RAM addresses in P15/P16 (0x80116E10, 0x8014C4A4) need decomp symbols.
