# 3B SONIC DSP (Spin Dash): port notes

Source: `.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm`, scope `SonicDSP` (lines 2031-2898).
Port: `decomp/src/ft/ftremix/sonic/ftremixsonicdsp.c`, table `dFTRemixSonicDSPProcs` (15 entries, all of the worklist).

## 1. Vanilla patches

### 1.1 `spin_dash_fgm_patch_` (Sonic.asm:703-737), required for the port to match

- RAM: `0x800E81A8` (`j` plus delay slot `andi`) inside vanilla `ftParamPlayLoopSFX`, `0x800E8190`.
- Decomp: `decomp/src/ft/ftparam.c:341` (`ftParamPlayLoopSFX`).
- Effect: when the FGM id is `0x3D8` (SPINDASH_CHARGE) or `0x3DE` (CLASSIC_SPINDASH_CHARGE), the stored loop SFX is stopped first (`ftParamStopLoopSFX`, `0x800E81E4`), so each charge press restarts the sound. Other ids take the original path.
- Our `SonicDSP` routines call `ftParamPlayLoopSFX` with those ids, so they depend on this patch.
- Insert in `ftparam.c` under `#ifdef REGION_REMIX`, at the top of the function:

```c
void ftParamPlayLoopSFX(FTStruct *fp, u16 sfx_id)
{
#ifdef REGION_REMIX
    /* Remix spin_dash_fgm_patch_ (Sonic.asm:703): Spin Dash FGMs restart on every call. */
    if ((sfx_id == 0x3D8) || (sfx_id == 0x3DE))
    {
        ftParamStopLoopSFX(fp);
    }
#endif
    if (fp->p_loop_sfx == NULL)
    {
        fp->p_loop_sfx = func_800269C0_275C0(sfx_id);

        fp->loop_sfx_id = (fp->p_loop_sfx != NULL) ? fp->p_loop_sfx->sfx_id : 0;
    }
}
```

`ftParamStopLoopSFX` is declared in `decomp/src/ft/ftparam.h:30`.

No other vanilla patch is needed for this scope. The `Sonic.asm` classic patches (`classic_fgm_patch_*`, `classic_voice_patch_*`, `save/load/reset_classic_*`) belong to other routines. The `classic_table` writers are listed in section 2.

## 2. Shared Remix state and subsystems

- `Sonic.classic_table` (RAM `0x80563BC0`), per-port is_classic. Read by the charge FGM choice (asm 2133-2135, 2219-2222). Written by `Sonic.asm` `save_classic_flags_*`, `load_classic_flag*`, `reset_classic_flag_` (asm 739-870, 1001-1032, outside this scope). The port keeps a file-local `sFTRemixSonicDSPClassicTable[MAXCONTROLLERS]` that is never written, marked `REMIX-TODO(classic)`. Classic Sonic therefore plays SPINDASH_CHARGE. Lead decision needed: either a shared port of the classic-flag state (one owner for all Sonic routines), or make this table read from that owner.
- No other `REMIX-TODO` markers. Nothing skipped in this scope.

## 3. Status ids

The DSP ids are not defined in any `scope Action` text. They come from `Character.add_new_action` (Character.asm:681-695), which numbers new actions after the parent array. I read them from the assembled `build/remixasm/remix.z64`, at `Character.SONIC_action_array` (`0x804C02F0`, ROM `0x2C00000 + 0x3C02F0`):

| Id | Name | main routine |
|----|------|--------------|
| 0xF6 | DSP_Ground_Charge | `0x80560BF8` |
| 0xF7 | DSP_Ground_Move | `0x80560F48` |
| 0xF8 | DSP_Ground_End | `0x800D94C4` |
| 0xF9 | DSP_Air_Charge | `0x80560CC8` |
| 0xFA | DSP_Air_Move | `0x80561044` |
| 0xFB | DSP_Air_Jump | `0x80561044` |
| 0xFC | DSP_Air_End | `0x800D94E8` |

These coincide with the `Sonic.Action.SpinDash*` names at 0xF6-0xFC in the `scope Action` of `Sonic.asm:410`. The DSP names are the ones the asm uses, so the enum follows the DSP names.

## 4. Vanilla helpers used

Calls map to decomp functions as follows (`build/remixasm/refs/vanilla_funcs.txt`):

- `0x800E6F24` `ftMainSetStatus`, `0x800E0830` `ftMainPlayAnimEventsAll`, `0x800E8190` `ftParamPlayLoopSFX`
- `0x800DEE98` / `0x800DEEC8` `mpCommonSetFighterGround` / `mpCommonSetFighterAir`
- `0x800DDE84` `mpCommonProcFighterOnEdge`, `0x800DE6E4` `mpCommonProcFighterLanding`, `0x800DDDDC` `mpCommonProcFighterOnFloor`, `0x800DE978` `mpCommonProcFighterCliffWaitOrLanding`
- `0x800D94E8` `ftAnimEndSetFall`, `0x800D8EB8` `ftPhysicsClampAirVelXMax`, `0x800D8D68` `ftPhysicsApplyGravityClampTVel`, `0x800D9074` `ftPhysicsApplyAirVelXFriction`, `0x800D90E0` `ftPhysicsApplyAirVelDrift`
- `0x800D8BB4` `ftPhysicsApplyGroundVelFriction` (copied in `ground_move_physics_`), `0x800D91EC` `ftPhysicsApplyAirVelFriction` (copied in `air_move_physics_`)
- `0x80161478` `ftKirbySpecialLwGetGroundAxisYaw` (slope angle), `0x800303F0` sin (`__sinf` in `sys/utils.h`)
- `0x8013F474` `ftCommonKneeBendGetInputTypeCommon` (jump check), `0x8013F660` `ftCommonJumpProcInterrupt`
- `0x800FF3F4` `efManagerDustHeavyDoubleMakeEffect` (jump dust)

## 5. Decoded fragments

- `ground_move_physics_`: `OS.copy_segment(0x543B4, 0x28)` and `(0x543EC, 0x28)`, decoded with `tools/remix_disasm_rom.py`. The copy is the vanilla `ftPhysicsApplyGroundVelFriction` with the traction load (`lwc1 f6, 0x0024(v0)`) replaced by `GROUND_TRACTION`. The port is `table[floor_flags & MAP_VERTEX_MAT_MASK] * 0.25F`, then `ftPhysicsSetGroundVelFriction`, then `ftPhysicsSetGroundVelTransferAir`. This matches the decoded `mul.s $f8, $f4, $f6` followed by the two jal calls.
- `air_move_physics_`: a modified `0x800D91EC`. The asm passes a stack struct to `0x800D9074` whose offset 0x54 holds AIR_FRICTION. The port copies `*fp->attr` into a local `FTAttributes`, sets `air_friction` to 3.0F, and calls `ftPhysicsApplyAirVelXFriction`. That function reads only `air_friction` (`decomp/src/ft/ftphysics.c:339`), so the behaviour is the same.

## 6. Uncertainties (asm line numbers)

1. asm 2123-2131 comment says "increase charge level by 2", but the code adds `0x0003` (3). Ported as 3 (`SONIC_DSP_CHARGE_PER_PRESS`). Air charge (asm 2217) also adds 3.
2. asm 2459-2464: the FSM scale is `lui 0x3C22` with a zero low half, so the value is `0.0098876953125` (bits `0x3C220000`), not 0.01 as the comment says. Ported as the exact bits.
3. asm 2102-2105: the `andi 0x07` on byte 0x18D clears is_absorb, absorb_lr, is_goto_attack100 and is_fastfall. Ported as four field assignments, per the Falco pattern.
4. asm 2097-2101: the air charge Y scale is `0x3EA00000` (exactly 0.3125), while the comment says 0.31. Ported as 0.3125F.
5. asm 2391-2394 and 2479-2483: the `beql` plus delay-slot pattern for the Super Sonic speed choice is read as a plain conditional on `fkind == REMIX_ID_SSONIC`.
6. asm 2684-2686: `bgezl` with the delay-slot `andi` on the L mask. The port reads lr >= 0 as the L mask, and lr < 0 as the R mask.
7. asm 2723-2725: `air_move_collision_` calls `0x800DE978` without setting a1. The vanilla `mpCommonProcFighterCliffWaitOrLanding(GObj*)` takes one argument, so the port passes only the GObj.
8. asm 2499-2501: `0x8013F474` returns 0 for no jump, per the asm comment. The port treats nonzero as a jump.
9. asm 2443: the slope angle comes from the vanilla Kirby function `0x80161478` (`ftKirbySpecialLwGetGroundAxisYaw`), used by address for any fighter. Semantics assumed as the asm comment describes.
10. Charge is stored as a raw s32 at `status_vars` (0x0B18) through an overlay struct, the same way Conker's DSP does (`ftremixconkerdsp.c`). The field name is ours, not a decomp field.
11. `GObj` 0x78 (`fighter_gobj->anim_frame`) is used as the "current animation frame" for the transition routines (asm 2341, 2365, 2744, 2867, 2888).
12. The `ftParamPlayLoopSFX` name says "loop", but the asm comment says "play FGM once". Ported as a call to that function, which matches the asm.

## 7. Helpers kept static (not in the table, not in the worklist)

Transitions: `ground_charge_transition_`, `air_charge_transition_`, `ground_move_transition_`, `ground_end_transition_`, `air_end_transition_`.
Set-status routines: `ground_move_initial_`, `air_move_initial_`, `air_jump_initial_`, `ground_end_initial_`, `air_end_initial_`.
Physics helper: `air_move_physics_`.
Charge helpers: `ftRemixSonicDSPChargeAccumulate` (shared body of the two charges), `ftRemixSonicDSPChargeReleaseAllowed`.

## 8. Not done in this file

- `ftremixported.c` and `ftremixsonic.c` are untouched, as required. The lead must add `dFTRemixSonicDSPProcs` / `dFTRemixSonicDSPProcsNum` to the Sonic entry in `dFTRemixPortedProcTables` (that file is outside this worker's scope).
- SonicNSP and SonicUSP are not ported here.
- The `spin_dash_fgm_patch_` C in section 1.1 must be inserted by whoever owns `ftparam.c`.
