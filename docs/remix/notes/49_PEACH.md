# 0x49 PEACH: notes for the Remix Peach port

Source: `.toolchain/pkgs/smashremix-src/src/Peach/PeachSpecial.asm` (routines),
`Peach/Peach.asm` (action table, Float hooks, ledge patches), `Kirby/Kirby.asm`
(Kirby reuses PeachNSP.* at lines 468-470).

Port: `decomp/src/ft/ftremix/peach/ftremixpeach.c`. Table `dFTRemixPeachProcs`
(22 entries, every routine in `worklists/49_PEACH.md`) and `dFTRemixPeachProcsNum`.
Syntax check passes with `-Wall`. Not built, per the brief.

Parent for the header include: FOX (`ftfox.h`, for `FTFOX_FIREFOX_AIR_DRIFT`).

## 1. Non-static functions the vanilla hooks call

Defined in `ftremixpeach.c`. Prototypes (add to the header the hooked vanilla file includes):

```c
sb32 ftRemixPeachFloatHoldsHeight(FTStruct *fp);                         /* A */
sb32 ftRemixPeachJumpAerialCheckOrFloat(GObj *fighter_gobj);             /* B */
sb32 ftRemixPeachFallOverride(GObj *fighter_gobj);                       /* C */
sb32 ftRemixPeachLightThrowCheckItemTypeThrow(FTStruct *fp);             /* D */
void ftRemixPeachFloatAutoCancelAerial(FTStruct *fp);                    /* E */
void ftRemixPeachEndFloatOnHit(FTStruct *fp);                            /* F */
s32 ftRemixPeachFallSpecialStatus(FTStruct *fp, s32 status_id);          /* G */
void ftRemixPeachLandingSetLedgeFlag(FTStruct *fp);                      /* H1 */
void ftRemixPeachGroundedScriptRestoreFloat(FTStruct *fp);               /* H2 */
void ftRemixPeachCliffRestoreFloat(FTStruct *fp);                        /* H3 */
void ftRemixPeachFloatInitial(GObj *fighter_gobj);   /* PeachFloat.initial_, used by B and C */
```

## 2. Vanilla hooks: RAM address, decomp location, C to insert

All inserts go under `#ifdef REGION_REMIX`. Vanilla files are not edited by this worker.

### A. Gravity suspended while floating (PeachSpecial.asm:87-127, `PeachFloat.handle_physics_`)

- RAM `0x800D8D68` = `ftPhysicsApplyGravityClampTVel`, `decomp/src/ft/ftphysics.c:231`.
- Asm: replaces the first instruction. While Float holds, sets `vel_air.y = 0` and returns, so gravity is skipped. Otherwise the timer decrements (150 resets to 149, 2..149 decrements, no C button sets it to 1).

```c
void ftPhysicsApplyGravityClampTVel(FTStruct *fp, f32 gravity, f32 tvel)
{
#ifdef REGION_REMIX
    if (ftRemixPeachFloatHoldsHeight(fp))
    {
        return;
    }
#endif
    fp->physics.vel_air.y -= gravity;
    /* ...rest unchanged */
```

### B. Float start check (PeachSpecial.asm:130-237, `PeachFloat.check_float_`)

Four vanilla sites replace their call to `ftCommonJumpAerialCheckInterruptCommon(fighter_gobj);` (the one that is `jal 0x8014019C`):

| RAM | Decomp file:line | Function |
|---|---|---|
| 0x8013F688 | `decomp/src/ft/ftcommon/ftcommonjump.c:14` | `ftCommonJumpProcInterrupt` |
| 0x8013FB54 | `decomp/src/ft/ftcommon/ftcommonjumpaerial.c:57` | `ftCommonJumpAerialProcInterrupt` |
| 0x8013F9C8 | `decomp/src/ft/ftcommon/ftcommonfall.c:14` | `ftCommonFallProcInterrupt` |
| 0x80141D88 | `decomp/src/ft/ftcommon/ftcommonpass.c:14` | `ftCommonPassProcInterrupt` |

At each site, with the indentation of that site:

```c
#ifdef REGION_REMIX
        ftRemixPeachJumpAerialCheckOrFloat(fighter_gobj);
#else
        ftCommonJumpAerialCheckInterruptCommon(fighter_gobj);
#endif
```

`ftRemixPeachJumpAerialCheckOrFloat` returns the vanilla result when Float does not start, so the call is equivalent otherwise.

### C. Fall starts Float instead of Fall (PeachSpecial.asm:240-280, `PeachFloat.fall_override_`)

- RAM `0x8013FA54`, inside `ftCommonFallSetStatus`, `decomp/src/ft/ftcommon/ftcommonfall.c:19` (ftMainSetStatus at line ~33).
- Asm: when Peach is floating (timer not 0 or 1) and C is held, it calls `initial_` and skips the vanilla `ftMainSetStatus`. The vanilla code continues (`ftPhysicsClampAirVelXMax`, `is_special_interrupt`).

Replace the `ftMainSetStatus(fighter_gobj, ((fp->jumps_used ...` statement with:

```c
#ifdef REGION_REMIX
        if (!ftRemixPeachFallOverride(fighter_gobj))
#endif
        ftMainSetStatus(fighter_gobj, ((fp->jumps_used >= fp->attr->jumps_max) ? nFTCommonStatusFallAerial : nFTCommonStatusFall), 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
```

### D. Item throw blocked while floating (PeachSpecial.asm:283-313, `prevent_item_throw_`)

- RAM `0x80150B4C`, inside `ftCommonAttackAirCheckInterruptCommon`, `decomp/src/ft/ftcommon/ftcommonattackair.c`. The call is `if (ftCommonLightThrowCheckItemTypeThrow(fp) != FALSE)`.
- Asm: returns 0 (no throw) while Float is in use.

```c
#ifdef REGION_REMIX
            if (ftRemixPeachLightThrowCheckItemTypeThrow(fp) != FALSE)
#else
            if (ftCommonLightThrowCheckItemTypeThrow(fp) != FALSE)
#endif
```

### E. Float cancels into an aerial (PeachSpecial.asm:315-345, `auto_cancel_aerials_`)

- RAM `0x80150E34`, inside `ftCommonAttackAirCheckInterruptCommon`, `ftcommonattackair.c`. Placement: in the `if (is_have_attackair)` block, right after the `ftMainSetStatus(fighter_gobj, status_id, ...)` call and before `if (status_id == nFTCommonStatusAttackAirLw)` (the asm compares against 0xD5 right after the patch).
- Asm: while Float is in use, sets `proc_map = mpCommonProcFighterWaitOrLanding` (0x800DE934).

```c
#ifdef REGION_REMIX
                ftRemixPeachFloatAutoCancelAerial(fp);
#endif
```

Placement is inferred from the asm (see uncertainty 6 below).

### F. Hit ends Float (PeachSpecial.asm:349-377, `end_float_on_hit_`)

- RAM `0x800E824C`, inside `ftParamStopVoiceRunProcDamage`, `decomp/src/ft/ftparam.c:403`. Patch point is the `ftParamStopVoice(fp);` call.
- Asm: sets timer to 1 when it is not 0, then runs the original call.

```c
#ifdef REGION_REMIX
    ftRemixPeachEndFloatOnHit(fp);
#endif
    ftParamStopVoice(fp);
```

### G. Fall Special becomes Fall Special Fall after the close (PeachSpecial.asm:1034-1058, `fall_special_patch_`)

- RAM `0x80143928`, inside `ftCommonFallSpecialSetStatus`, `decomp/src/ft/ftcommon/ftcommonfallspecial.c:77`.
- Asm: for PEACH only, when the current action is USPClose (0xE7), the status becomes USPFall (0xE8).

```c
#ifdef REGION_REMIX
    ftMainSetStatus(fighter_gobj, ftRemixPeachFallSpecialStatus(fp, nFTCommonStatusFallSpecial), 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
#else
    ftMainSetStatus(fighter_gobj, nFTCommonStatusFallSpecial, 0.0F, 1.0F, FTSTATUS_PRESERVE_FASTFALL);
#endif
```

### H1. Landing flags the ledge (Peach.asm:576-609, `ledge_patch_`)

- RAM `0x800DE680`, inside `mpCommonRunFighterSpecialCollisions`, `decomp/src/mp/mpcommon.c:475`. Patch point is the `mpCommonSetFighterLandingParams(fighter_gobj);` call (mpcommon.c:538).
- Asm: sets the Remix `ledge_flag` when fkind is PEACH or NPEACH, then calls the original.

```c
#ifdef REGION_REMIX
            ftRemixPeachLandingSetLedgeFlag(ftGetStruct(fighter_gobj));
#endif
            mpCommonSetFighterLandingParams(fighter_gobj);
```

### H2. Landing restores Float unless it was a ledge grab (Peach.asm:615-623, `grounded_script_`)

- Installed in the Peach `grounded_script` table entry (Peach.asm:507-508). The decomp caller was not located in this pass. The asm returns to `0x800DE44C`, the end of `mpCommonSetFighterLandingParams` (`mpcommon.c:425`), so the grounded script runs at landing.
- Asm: if `ledge_flag` is FALSE, the float timer is reset to 0. `ledge_flag` is then cleared.
- C: call `ftRemixPeachGroundedScriptRestoreFloat(fp);` once per Peach landing, before the landing params. Needs verification of the caller (uncertainty 7).

### H3. Ledge action resets Float (Peach.asm:628-650, `restore_float_flag_ledge_action`)

- RAM `0x8014506C`, the end of `ftCommonCliffQuickOrSlowSetStatus`, `decomp/src/ft/ftcommon/ftcommoncliffclimb.c:58`. The patch is at the second store, `cliffmotion.cliff_id` (0x0B1C).
- Asm: for PEACH or NPEACH, resets the float timer to 0.

```c
    fp->status_vars.common.cliffmotion.cliff_id = fp->coll_data.cliff_id;
#ifdef REGION_REMIX
    ftRemixPeachCliffRestoreFloat(fp);
#endif
```

## 3. Shared Remix subsystems (REMIX-TODO markers)

- `REMIX-TODO(items)` in `ftRemixPeachDSPCreateAndAssignItem` (PeachSpecial.asm:484-562, `PeachDSP.create_and_assign_item_`). Not ported:
  - `Item.skip_item_spawn_gfx_.flag` writes (Item.asm:1620, patches 0x8016EAD8 and 0x80172DB8).
  - `Item.Bobomb/MrSaturn/BeamSword/Turnip.SPAWN_ITEM` calls. Turnip is a custom Remix item with id 0x4B (see uncertainty 3).
  - `jal 0x80172CA4` (`itMainSetFighterHold`) pickup.
  - Kept: the two `Global.get_random_int_` draws (RNG parity). The DSP therefore never creates an item.

## 4. Other notes

- Kirby's `PEACH_NSP_Ground/Air/Recoil` use the same PeachNSP addresses (Kirby.asm:468-470) and reach this port through the same table. Only the status id differs, chosen by `fp->fkind == REMIX_ID_PEACH`.
- Float's timer uses `fp->passive_vars.purin.unk_0x0` (0x0ADC), through `PEACH_FLOAT_TIMER`. There is no Peach field in the decomp, so this is a named slot, not a semantic one.
- `is_smash` (0x0B18) is read through `fp->status_vars.link.specialn.is_smash`. That field is written by `ftLinkSpecialNProcStatus` (`ftlinkspecialn.c:90`), which the NSP initials call.
- `sFTRemixPeachLedgeFlag` is a file-static global, matching Remix's single `ledge_flag`.
- Peach's AI `recovery_logic` (Peach.asm:652-765) references Action.Float and is not in the worklist. It is not ported.

## 5. Uncertainties (asm line numbers, PeachSpecial.asm unless noted)

1. **Float y-scale constant.** Line 22 comment says 0.375, but `0x3E80` decodes to **0.25** (`0x3E800000`). The port uses 0.25 (`PEACH_FLOAT_INIT_Y_DECAY`). Check the other comments too: `AIR_ACCELERATION 0x3C24` is 0.010009765625 exactly (the comment rounds to 0.01).
2. **USP air physics copy (lines 893-894, 1004-1015).** The asm copies a 16-instruction segment from `0x548F0`, which corresponds to `ftPhysicsApplyAirVelDrift` (`ftphysics.c:362`). Its tail is the decay call whose `v0` gates `bnez` at line 894. The port reconstructs it: fast fall or gravity, then `ftPhysicsCheckClampAirVelXDecMax`, then air control only in MOVE, then `ftPhysicsApplyAirVelXFriction`. Verify against the ROM.
3. **Turnip id 0x4B.** From `Item.asm:1515-1548`: 31 `add_item` calls, Turnip last (index 30), base id 0x2D. Verify against a live `Item.Turnip.id`. Also `Item.asm` line 2902 compares a kind to it.
4. **Kirby action ids 0x1B2/0x1B3/0x1B4.** Formula in `Character.asm:681`: `(0x67C/0x14) + new_index + 0xDC`, where `KIRBY(0x67C)` is at `Character.asm:2377` and the new index is 131/132/133 (`Kirby.asm:468-470`). Not in `remix.sym`. Verify at runtime.
5. **Peach close (lines 777-793).** The copy of Fox's `ftFoxSpecialAirHiEndProcUpdate` (`ftfoxspecialhi.c:396`) is assumed to set args 1-3 to `FTFOX_FIREFOX_AIR_DRIFT`, FALSE, TRUE. The port keeps those three and sets arg 5 = FALSE, arg 6 = 0.375 (`sw t6, 0x14(sp)`, line 786) and arg 7 = FALSE (`sw r0, 0x18(sp)`, line 783 comment "interrupt flag = FALSE").
6. **E placement (auto-cancel).** The patch at `0x80150E34` sits right before the compare with 0xD5. The C insert after `ftMainSetStatus` in the attack-air block is an inference. The asm also writes `proc_map`, which is 0x9E4.
7. **H2 caller.** The `grounded_script` table caller is not located. Peach.asm:507-508 and 615-623 are the only evidence.
8. **Ground collision (lines 1386-1387).** `beqz v0` treats `mpCommonProcFighterOnFloor` returning TRUE as "still on floor". This matches the decomp (`mpcommon.c:233`), which returns TRUE when on the floor. The wall bounce runs only then.
9. **Wall side (lines 1394-1399).** `lr >= 0` uses `WALL_COLLISION_L` (0x0001), and `lr < 0` uses `_R` (0x0020). Both read `coll_data.mask_prev` (0xCC).
10. **NSP recoil (lines 1169-1174).** `ga == 0` (grounded) triggers `mpCommonSetFighterAir`. Assumes `nMPKineticsGround == 0`, which matches `mpdef.h:39`.
11. **DSP main (lines 442-473).** The idle check is reached only when `flag1 == 0`, because `beqzl` skips the rest. The flag0 path clears flag0 before the call (the delay slot).
12. **syUtilsRandIntRange vs get_random_int_ (Global.asm:26, 0x80018994).** The port uses `syUtilsRandIntRange`, assumed to be the same function (`sys/utils.c:213`, `(rand16 * range) / 65536`). Not verified against the ROM.
13. **ftPhysicsApplyAirVelDrift in NSP air (line 1304).** Vanilla `0x800D90E0` is used directly, as in the asm.
