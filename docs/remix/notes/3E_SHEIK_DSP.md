# 0x3E SHEIK, SheikDSP notes

Port: `decomp/src/ft/ftremix/sheik/ftremixsheikdsp.c`, table `dFTRemixSheikDSPProcs` (7 entries, `dFTRemixSheikDSPProcsNum`).
Source: `.toolchain/pkgs/smashremix-src/src/Sheik/SheikSpecial.asm` lines 2025-2614 (scope `SheikDSP`).

## 1. Vanilla patches

No `OS.patch_start` and no `j`/`jal` hook in the DSP scope edits vanilla code. The bindings are generated:

- `Sheik.asm:481-485`: `Character.table_patch_start(ground_dsp / air_dsp, SHEIK, 0x4)` points the special entry at `SheikDSP.initial_`. The generated binding handles it.
- `Sheik.asm:464-467` and `441`: `Character.add_new_action` and `edit_action` bind DSP_BEGIN/ATTACK/LANDING/RECOIL. Some of the bound routines are vanilla addresses (`0x800D94E8`, `0x800D94C4`, `0x800D8CCC`, `0x800DDEE8`, `0x800DE978`). They belong to the action table and are not in this scope.

## 2. Shared Remix subsystems

None. The DSP routines call only decomp functions (`ftMainSetStatus`, `ftMainPlayAnimEventsAll`, `ftAnimEndCheckSetStatus`, `ftCommonFallSetStatus`, `ftCommonCliffCatchSetStatus`, `mpCommon*`, `ftPhysics*`) and no custom items, GFX or announcer code.

## 3. Uncertainties

- `SheikDSP.check_recoil_` (asm 2394-2434). `sw a1, 0x0010(sp)` at line 2399 stores attack_collision_'s `_end` address, and that slot is the 5th argument (`flags`) of the `jal 0x800E6F24` at line 2421. The asm passes a code address as the flags word. The port passes `FTSTATUS_PRESERVE_NONE`, because the flag bits of a code address are not meaningful. Check in game whether any preserve bit matters.
- `SheikDSP.air_to_ground_` (asm 2321-2341). The 5th argument of `ftMainSetStatus` (`0x10(sp)`) is never written. The port passes `FTSTATUS_PRESERVE_NONE`.
- `SheikDSP.recoil_physics_` horizontal clamp (asm 2476-2553). The stick is subtracted, `neg.s f2, f14` gives `+40`, and the comparisons are inverted relative to `physics_`. The port is literal:
  - facing right: `vel > -40` gives `-40`; otherwise `vel < -20` gives `-20`.
  - facing left: `vel < 40` gives `40`; otherwise `vel > 20` gives `20`.
  Those results are non-monotone (for example, facing left, `vel = 45` gives `20`). Verify in game.
- `SheikDSP.recoil_main_` frame window (asm 2576-2590). The comment says "frame 10", but the constants are 40.0 (`0x4220`) and 15.0 (`0x4170`). The port proceeds only when `15 < anim_frame <= 40`.
- `SheikDSP.recoil_main_` facing flip (asm 2597-2602). The asm sets facing to `1` when it is not already `1`, and to `-1` otherwise. The port writes `fp->lr = (fp->lr == 1) ? -1 : 1`.
- The DSP flag at 0x0ADC (asm 2063-2070, 2079-2082, and `dsp_on_hit_` at 2093-2097). The FTStruct passive union has no Sheik member. The port uses `fp->passive_vars.purin.unk_0x0`, the generic u32 at that offset. Behaviour is the same, but the name is only a placeholder.
- The button buffer (asm 2106-2114, 2561-2569). The asm stores the high byte of `button_tap` (`lbu 0x01BE`) and tests `0x40`. The port keeps a 16-bit buffer and tests `B_BUTTON` (`0x4000`). This tests the same bit.
- `SheikDSP.attack_collision_` wall check (asm 2350-2360). The asm reads `lhu 0x00CC` (`coll_data.mask_prev`) against `0x21` (`MAP_FLAG_LWALL | MAP_FLAG_RWALL`). The ledge check (asm 2375-2378) reads `coll_data.mask_stat` (`0x00D2`) against `0x3000` (`MAP_FLAG_CLIFF_MASK`). The port follows the offsets. Verify that `mask_prev` is the intended word.
- `SheikDSP.physics_` / `recoil_physics_` gravity (asm 2202-2211). The asm loads `flag variable 2` from `0x0180` (`motion_vars.flags.flag1`, the brief's "temp variable 2"). The port uses `flag1` too. Gravity is `attr->gravity * 0.23F` (`0x3E6B851F`) while flag1 is 0, and `attr->gravity` otherwise.
- `SheikDSP.initial_` (asm 2042-2046). The ground-to-air step runs when `ga == nMPKineticsGround`. The asm comment says "skip if kinetic state !grounded", which matches that reading.

## 4. Integration note (not changed here)

- `decomp/src/ft/ftremix/ftremixported.c` must list `dFTRemixSheikDSPProcs` in `dFTRemixPortedProcTables`, or the DSP routines fall back to Captain's routines. This worker did not edit that file.
- The file-local enums (`nFTRemixSheikStatusDSPBegin` = 0xF0 to `DSPRecoil` = 0xF3) match `Sheik.asm:126-129`. They are local to this file.
- Helpers `ftRemixSheikDSPOnHit`, `ftRemixSheikDSPAttackTransition`, `ftRemixSheikDSPAirToGround`, `ftRemixSheikDSPCheckRecoil` and the physics helpers are `static` and not in the table. The worklist does not list them, and `dsp_on_hit_` is reached only through `proc_damage`.

## 5. REMIX-TODO markers

None.
