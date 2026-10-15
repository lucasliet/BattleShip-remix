# 0x40 DEDEDE: DededeDSP notes

Port: `decomp/src/ft/ftremix/dedede/ftremixdedededsp.c`, table `dFTRemixDededeDSPProcs` (13 entries, `dFTRemixDededeDSPProcsNum`).
Source: `.toolchain/pkgs/smashremix-src/src/Dedede/DededeSpecial.asm` lines 2479-3394 (scope `DededeDSP`). Table wiring is not done: `ftremixported.c` must add `{ dFTRemixDededeDSPProcs, &dFTRemixDededeDSPProcsNum }` to `dFTRemixPortedProcTables` (it currently lists only `dFTRemixDededeProcs`). That file was out of scope.

## Coverage

- Worklist routines ported: 13 of 13.
  `air_begin_collision_`, `air_begin_initial_`, `air_charge_collision_`, `air_charge_interrupt_`, `air_shoot_collision_`, `begin_main_`, `charge_main_`, `ground_begin_collision_`, `ground_begin_initial_`, `ground_charge_collision_`, `ground_charge_interrupt_`, `ground_shoot_collision_`, `shoot_main_`.
- Helpers, all `static`: begin/charge/shoot set-status bodies, the four transition routines (`ground_begin_transition_`, `air_begin_transition_`, `ground_charge_transition_`, `air_charge_transition_`), `ground_to_air`, `air_to_ground`, `attach_minion_`, `destroy_attached_minion_`, `dedede_on_hit_subroutine_` and its establishment (inlined in the begin body).
- No `OS.patch_start` or `j`/`jal` hooks are needed by the DSP scope, so no vanilla C insertions are required.

## Vanilla patches the scope relies on

- `Dedede.asm` 597-601: the special table entries for `ground_dsp` and `air_dsp` point to `DededeDSP.ground_begin_initial_` and `air_begin_initial_`. The binder maps them through the table. No C insertion needed.
- `Dedede.asm` 623-625: the `gfx_routine_end` patch redirects Dedede's charge GFX to Remix `charge_gfx_routine_` (line 707). That routine reads the charge level at FTStruct 0x0AE4 (`Dedede.asm` 709), so the charge word must stay at 0xAE4. It is GFX code outside this scope.
- `Dedede.asm` 545-574: action params and action entries, including the vanilla physics procs `0x800D8CCC` (`ftPhysicsApplyGroundFrictionOrTransN`) and `0x800D90E0` (`ftPhysicsApplyAirVelDrift`). Not ported here.

## Shared subsystems (REMIX-TODO)

1. `REMIX-TODO(Item)`: `attach_minion_`, `DededeDSP.attach_minion_` lines 3245-3297. Spawns `Item.WaddleDee` (charge 0), `Item.WaddleDoo` (charge 1) or `Item.Gordo` (charge 2) via `Item.*.SPAWN_ITEM`. The port keeps the attached slot NULL, so the minion never exists. Consequences: the throw branch and destroy branch never see a minion, and the item-side state is not set. Gordo and Waddle Doo are custom items.
2. `REMIX-TODO(Item)`: `shoot_main_` throw, asm 3209-3216. Sets the minion's throw state at ITStruct 0x033E to 1. This is dead in the port until item 1 is ported. Marked in the source.
3. `REMIX-TODO(GFXRoutine)`: `charge_main_` `_add_charge` at max charge, asm 2880-2882. Begins `GFXRoutine.id.DEDEDE_CHARGE` (white spark on the hand). The call target `0x800E9814` is the Remix wrapped player GFX begin (`GFXRoutine.asm` line 6). `build/remixasm/refs/vanilla_funcs.txt` names it `ftParamCheckSetFighterColAnimID`, which is not the right function, so it is not called.

## Dropped computations with no observable effect in the port

- `attach_minion_` asm 3258: `0x8015D35C` (`ftSamusSpecialNGetChargeShotPosition`) computes the spawn position at `sp+0x28`. The only consumer is the item spawn in item 1. Dropped with it.
- `shoot_main_` asm 3199: `0x800EDF24` (`gmCollisionGetFighterPartsWorldPosition`) fills a vector that is written to the stack and never read on the throw path. Dropped.
- `shoot_main_` asm 3180: `slti at, t6, 3` result is never used. Dropped.
- `ground_begin_transition_` asm 2738: load of 0x08E8 is overwritten by `lli t8, 2` at 2741. The asm comment itself says "useless?". Dropped.

## Mappings that need a check

- Remix temp variable 1 is `motion_vars.flags.flag0` (0x017C). Used by `shoot_main_` as the throw trigger, and cleared by the begin body.
- `0x0192` bit set by `ori 0x80` in `ground_shoot_initial_` (asm 3102-3104) and `air_shoot_initial_` (3142-3144). The asm comment says "enabled unknown bitflag". Bit 0 is the MSB, so this is `is_special_interrupt` in the decomp. Ported as `fp->is_special_interrupt = TRUE`. Verify against the game if a shoot can be interrupted by a special.
- `ftPhysicsClampAirVelXMax` (0x800D8EB8) is called in asm as "momentum capture?" (lines 2653, 3059, 3334). The decomp name is used as-is. Uncertain semantics.
- Kinetic state `0x014C` is `fp->ga` (ground 0, air 1), per struct_offsets.txt.
- Current status for ground/air conversion is `fp->status_id` (0x24). `ground_to_air` adds 3 (asm 3326-3327), `air_to_ground` subtracts 3 (3376-3377). DSP ids: 0xF8 to 0xFD.
- Flags: `2` is `FTSTATUS_PRESERVE_COLANIM`, `0x802` is `COLANIM | LOOPSFX`, `0` is `PRESERVE_NONE`.
- Charge end frame: `lui t1, 0x41A0` at asm 2844 is the float `20.0F` (0x41A00000), not an integer.
- Minion item kind written before destroy: `0x1C` (asm 2714-2716). Kept as `DEDEDE_DSP_MINION_FAKE_KIND`.

## Uncertainties (asm line numbers)

1. Raw N64 FTStruct offsets 0x0AE4, 0x0B18, 0x0B1C, 0x0B28 are accessed through `ftRemixDededeDSPWord`, as `ftremixbowser.c` does. This assumes the PC FTStruct places those words at the same offsets. Pointer fields are wider on LP64, so the layout may differ. Check with `offsetof` when a build is available.
2. The minion pointer (N64 0x0B20) is a per-player static `sFTRemixDededeDSPMinion[MAXCONTROLLERS]`. A 32-bit word cannot hold it on LP64. The static is per port, not per fighter struct, so Kirby copying Dedede's DSP shares the slot with the port's own Dedede. Unlikely to matter because the slot is always NULL until item 1 is ported.
3. Air charge interrupt, asm 2943 vs 2794 and 2855. `air_charge_interrupt_` reads the action timer at FTStruct 0x001C (`status_total_tics`), while `air_charge_initial_` and the charge main use 0x0B1C. Ported as written, so the air release timing comes from `status_total_tics`. Possibly a Remix bug. Kept faithful.
4. Comment mismatch in asm: `ground_begin_collision_` says "air_charge_transition_" (line 2620), but the symbol loaded is `air_begin_transition_`. The symbol is used.
5. `ground_charge_interrupt_` at asm 2899-2906: the `bnezl` delay slot loads buttons_pressed on both paths, so the cancel test always uses pressed buttons. Ported as-is.
6. `charge_main_` at asm 2844: `CHARGE_END` is loaded as a float bit pattern. See mapping above.
7. Transition flags in `ground_begin_transition_` and `air_begin_transition_` are `2` (asm 2740-2743, 2659-2663). `ground_charge_transition_` and `air_charge_transition_` use `0x802` (asm 3030, 3067). Preserved.
