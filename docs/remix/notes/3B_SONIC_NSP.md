# 0x3B SONIC NSP (Homing Attack): worker notes

Port: `decomp/src/ft/ftremix/sonic/ftremixsonicnsp.c`, table `dFTRemixSonicNSPProcs` (9 entries) and `dFTRemixSonicNSPProcsNum`. Worklist: `build/remixasm/worklists/3B_SONIC.md` (SonicNSP entries only). Source: `.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm`, scope `SonicNSP`.

## Status

- Worklist routines ported: 9 of 9 (`begin_initial_`, `begin_main_`, `move_main_`, `move_physics_`, `move_collision_`, `ground_end_collision_`, `air_end_collision_`, `ground_recoil_collision_`, `air_recoil_collision_`).
- Helpers ported as `static` (not in table): `is_super_sonic`, `check_for_targets_`, `check_target_`, `locked_move_initial_`, `move_initial_`, `bounce_initial_`, `ground_end_initial_`, `air_end_initial_`, `air_recoil_initial_`, the four `*_transition_` routines, and `Character.get_hitbox_collision_flags_` (shared Remix helper, small and self-contained, ported inline as `ftRemixSonicNSPHitboxCollided`).
- Not ported (out of scope): `SonicNSP.ssonic_begin_initial_`, `ssonic_begin_main_`, `ssonic_move_initial_`, `ssonic_move_main_`, `ssonic_move_collision_`. Only reachable through Kirby with the Super Sonic hat (status ids 0x1C7-0x1C9) and SONIC array entries 0x272-0x275, which reference them. Those statuses will fall back to the parent until an SSonic worker ports them.
- No `REMIX-TODO` markers in the file.

## Action ids (derived from the built ROM, not from source names)

The NSP names come from `Character.add_new_action` (`Character.asm:623`), which assigns ids sequentially. I took them from `build/remixasm/remix.z64`, scanning each character's action array (`Character.<X>_action_array` RAM = ROM `RAM - 0x80400000 + 0x2C00000`) for the SonicNSP routine addresses. The mapping was cross-checked: Sonic's `NSP_Begin` lands at 0xFD, which matches `Sonic.asm:422` `HomingStart(0xFD)`.

| Status | SONIC | KIRBY / JKIRBY |
|---|---|---|
| Begin | 0xFD | 0x15E |
| Move | 0xFE | 0x15F |
| Locked_Move | 0xFF | 0x160 |
| Ground_End | 0x100 | 0x161 |
| Air_End | 0x101 | 0x162 |
| Ground_Recoil | 0x102 | 0x163 |
| Air_Recoil | 0x103 | 0x164 |
| Bounce | 0x104 | 0x165 |
| SSonic Move / Locked (Kirby hat 0x2F) | - | 0x1C8 / 0x1C9 |

## Vanilla patches the scope relies on

1. **`SonicNSP.destroyed_target_fix_`** (`SonicSpecial.asm:1137-1179`). Patch at RAM `0x80009A98` (ROM `0xA698`). It clears the Sonic target when the destroyed object is that target. The target lives in the port's per-player static, so the hook must call the port function. The patch's `s0` is the object being destroyed. Insert this under `#ifdef REGION_REMIX` where the patch's original first line (`lui t6, 0x8004`) sits, in the vanilla routine that contains RAM `0x80009A98`:

   ```c
   #ifdef REGION_REMIX
   void ftRemixSonicNSPForgetTarget(GObj *destroyed_gobj);
   ftRemixSonicNSPForgetTarget(gobj);
   #endif
   ```

   The prototype belongs in the vanilla file or a header. Its definition is non-static in `ftremixsonicnsp.c`. Caveat: `vanilla_funcs.txt` lists `gcMakeGObjBefore` at `0x80009A34` as the last entry before `0x80009A98`. I did not confirm which decomp function owns `0x80009A98`, so verify in `decomp/src/sys/objman.c` before inserting. The patch only acts when the object type is 0x3E8 (player) or 0x3F5 (item). The port's pointer match is equivalent, because only fighters and items can be targets.

No other vanilla patches are used by the scope.

## Port-side state decisions

- **Target.** Remix stores it at `0x0B18` as a 32-bit pointer. The port keeps `sFTRemixSonicNSPTarget[MAXCONTROLLERS]` instead. `SonicDSP` reuses word `0x0B18` for a charge level or movement flag, so the NSP target must not share that word on the port.
- **X_DIFF, angle, timer** (`0x0B1C`, `0x0B20`, `0x0B28`). Kept in struct memory and read through `SONIC_NSP_OFF_*`, as Marth's port does. `SonicDSP` also uses `0x0B1C`; it is re-initialised on NSP begin.
- **Kirby hat** byte `0x0980` (`s8`, Super Sonic hat id `0x2F`), same overlap with `modelpart_status` as Marth's port.
- **Hitbox collision** (`Character.get_hitbox_collision_flags_`, `Character.asm:3322`). The asm reads the first byte of `attack_colls[k].attack_records[r].victim_flags` (offsets `0x60/0x68/0x70/0x78` relative to each 0xC4 hitbox) and masks `0xF0`. That is the four `is_interact_*` bits of `GMHitFlags`. The port tests those four bits by name on each record of each attack coll with `attack_state != 0`.
- **Item hurtbox.** Asm reads word `0x0248` of the item special struct (`ITStruct.damage_coll.hitstatus`) and masks `0x1`. The port uses `hitstatus & 0x1`.
- **Team and player lists.** `0x800466FC` and `0x80046700` are taken to be `gGCCommonLinks[nGCCommonLinkIDFighter]` and `gGCCommonLinks[nGCCommonLinkIDItem]`, as used by `ftpublic.c` and `itmanager.c`. I did not verify that `gGCCommonLinks[3]` and `[4]` sit at those addresses; check it with the decomp init code before relying on it.
- **Team battle.** `gSCManagerBattleState` (`Global.match_info`, `0x800A50E8`) with `is_team_battle` and `is_team_attack`.

## Approximations and uncertainties (asm line numbers in `SonicSpecial.asm`)

1. **Math helper addresses.** `0x8001863C` (atan2) and `0x800303F0` / `0x80035CD0` (sin/cos) are not in `vanilla_funcs.txt`. The port uses `syUtilsArcTan2(y, x)`, `__sinf`, `__cosf`, as Mewtwo's port does for atan2. Lines 442, 692, 747, 753.
2. **`0x800D8EB8`** is named `ftPhysicsClampAirVelXMax` in `vanilla_funcs.txt`. The asm comment says "momentum capture?". The port calls the decomp name. Lines 1001, 1128.
3. **`check_target_` X_DIFF lower bound.** The asm comment says "end if X_DIFF =< 0", but `c.le.s f0, f10` plus `bc1fl` accepts X_DIFF = 0. The port follows the instructions (`0.0F <= x_diff`). Lines 344-347.
4. **`check_target_` Y range.** Uses `|Y_DIFF| <= MIN_Y + 0.5 * X_DIFF`, with X_DIFF already multiplied by DIRECTION. Lines 360-383.
5. **`begin_initial_` kinetic test.** Sets aerial state when `ga == 0`, i.e., `bnez t0, _continue` skips. Lines 61-64. `nMPKineticsGround` is 0.
6. **`move_main_` kinetic test.** Air end when `ga != nMPKineticsGround`, from `bnez t6, _aerial`. Lines 592-603.
7. **`move_physics_` angle wrap.** Remix wraps the target-relative difference and the new angle only from below (`-pi < x`, add 2pi). The port does the same in `ftRemixSonicNSPWrapAngle`. Lines 709-737.
8. **`ground_end_transition_` / `ground_recoil_transition_`.** The mpCommonSetFighterGround call is `0x800DEE98` (decomp `mpCommonSetFighterGround(FTStruct *)`). Lines 951, 1078.
9. **Start frame of transitions.** `lw a2, 0x0078(a0)` with `a0` = the GObj reads `GObj.anim_frame`, not the player struct. Lines 965, 995, 1092, 1122.
10. **`begin_initial_` B-button mask.** `Joypad.B` is `0x4000` (`Joypad.asm:12`; the comment at line 167 says 0x0020, which is stale). The port uses `B_BUTTON` = `CONT_B` = 0x4000 on `input.pl.button_hold`. Lines 166-168.
11. **`move_initial_` and `locked_move_initial_` `s0`.** The asm never sets `s0`. It uses the caller's `s0`, which is always the player struct (`begin_main_`, `ssonic_move_initial_`). The port passes the GObj and derives the struct.
12. **Kirby SSonic-hat ids.** `0x1C8` / `0x1C9` are the Kirby copies of `SSonic.NSP_Move` / `NSP_Locked_Move`. Those routines are not in this worker's scope. The ids are used as written.
13. **Sonic action array** entries `0x272`-`0x275` reference SSonic NSP routines. Outside this worklist. Flagged for the SSonic worker.
