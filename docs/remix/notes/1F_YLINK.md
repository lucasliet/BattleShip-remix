# 0x1F YLINK (Young Link): notes for the F5 port

Source: `.toolchain/pkgs/smashremix-src/src/YoungLink/YoungLinkSpecial.asm`,
`YoungLink/YoungLink.asm`, `YoungLink/AI/Attacks.asm`, plus the shared hooks in
`linkshared.asm`, `Reflect.asm` and the Link routines that YoungLink patches.
Port file: `decomp/src/ft/ftremix/younglink/ftremixyounglink.c`.

## What the file ports

- Table `dFTRemixYoungLinkProcs` (1 entry, `dFTRemixYoungLinkProcsNum` = 1):
  `REMIX_SYM_YoungLinkUSP` (0x8054216C, the `direction_` routine, asm lines 5-41)
  maps to `ftRemixYoungLinkUSPDirectionProcInterrupt`. Its action slot is 0xE4
  (`edit_action`, YoungLink.asm:87).
- Bombchu item (YoungLinkDSP, asm lines 43-662), ported as file-local statics
  except where noted:
  - `ftRemixYoungLinkBombchuUpdateDirection` (update_direction_)
  - `ftRemixYoungLinkBombchuEndFgm` (end_fgm_, no-op, see REMIX-TODO 1)
  - `ftRemixYoungLinkBombchuBeginFalling` (begin_falling_)
  - `ftRemixYoungLinkBombchuBeginExplosion` (begin_explosion_)
  - `ftRemixYoungLinkBombchuBeginMoving` (begin_moving_)
  - `ftRemixYoungLinkBombchuRestingProcUpdate` (resting_main_)
  - `ftRemixYoungLinkBombchuThrownProcUpdate` (thrown_main_)
  - `ftRemixYoungLinkBombchuDroppedProcUpdate` (dropped_main_)
  - `ftRemixYoungLinkBombchuHeldProcUpdate` (held_main_)
  - `ftRemixYoungLinkBombchuExplodingProcUpdate` (exploding_main_)
  - `ftRemixYoungLinkBombchuMovingProcUpdate` (moving_main_)
  - `ftRemixYoungLinkBombchuThrownProcMap` (thrown_collision_)
  - `ftRemixYoungLinkBombchuMovingProcMap` (moving_collision_)
  - `ftRemixYoungLinkBombchuProcDamage` (hurtbox_collision_)
  - `ftRemixYoungLinkBombchuDroppedProcDamage` (dropped_hurtbox_collision_)
  - `ftRemixYoungLinkBombchuProcReflector` (reflect_, see U2)
- Exported (not in the table, needed by the hooks below):
  ```c
  sb32 ftRemixYoungLinkBombchuBlastZone(GObj *item_gobj);   /* bombchu_blast_zone_, returns TRUE = destroy */
  extern ITStatusDesc dFTRemixYoungLinkBombchuStatusDescs[]; /* item_state_table, 8 slots x 7 states */
  ```
- Not exported, no hook needed: `item_info_array` (data, see H10), `state_change_`
  (vanilla hook H1), `boomerang_speed_` (H9), `up_special_landing_` (H7).

## Vanilla hooks (owners apply them under `#ifdef REGION_REMIX`)

Each hook lists the RAM address, the decomp function (from
`build/remixasm/refs/vanilla_funcs.txt`, or the function that contains the
address when it is not listed), and the C to insert. Do not edit vanilla files
from the port file.

### H1. Bombchu state table swap (YoungLinkSpecial.asm:626-649, `state_change_`)

- Hook: `OS.patch_start(0xED908, 0x80172EC8)`.
- Function: `itMainSetStatus`, `decomp/src/it/itmain.c:577`.
- Behaviour: for item kind 0x15 (bomb) created by YLINK, the status table becomes the Bombchu table.
- Insert (before the original status lookup):
  ```c
  #ifdef REGION_REMIX
  if ((ip->kind == 0x15) && (ip->coll_data.cliff_id == REMIX_ID_YLINK))
  {
      status_desc = dFTRemixYoungLinkBombchuStatusDescs;
  }
  #endif
  ```
  `ip->coll_data.cliff_id` is the N64 0x100 slot that H2 fills with the owner's
  character id. Add `extern ITStatusDesc dFTRemixYoungLinkBombchuStatusDescs[];`
  in `itmain.c` under the same `#ifdef`.

### H2. Bomb creation: owner id and blast-zone slot (linkshared.asm:287-378)

- `create_bomb_` hook `OS.patch_start(0x100FE0, 0x801865A0)`: function `itLinkBombMakeItem`, `decomp/src/it/itfighter/itlinkbomb.c:613`. Selects the bomb descriptor per owner. For YLINK it is `YoungLinkDSP.item_info_array` (H10). Hook `0x801865D4` (`save_character_id_`) is inside the same function.
- `save_character_id_` (linkshared.asm:340-365): stores the owner fkind at item+0x100, and for YLINK sets the FGM pointer at item+0x1D0 to 0 and the blast-zone routine at item+0x398 to `YoungLinkDSP.bombchu_blast_zone_`.
- Insert (inside `itLinkBombMakeItem` after `ip` is created, owner is `fp`):
  ```c
  #ifdef REGION_REMIX
  ip->coll_data.cliff_id = fp->fkind;
  if (fp->fkind == REMIX_ID_YLINK)
  {
      /* REMIX-TODO(ITStruct): item+0x1D0 (FGM pointer) = NULL and item+0x398 = ftRemixYoungLinkBombchuBlastZone are not in the decomp ITStruct. Add fields before enabling. */
  }
  #endif
  ```
  The render routine patches (0x801865DC, 0x80186658) only set render id 0x48 for YLINK; they are presentation, not specials.

### H3. Bomb flash (linkshared.asm:381-410, `bomb_flash_`)

- Hook `0x801859DC`, function `itLinkBombExplodeWaitUpdateScale` (`itlinkbomb.c:149`). It selects the file-1 pointer for the flash by owner id (YLINK: `YoungLinkDSP.item_info_array`).
- It also reads u16 at item+0x354 (`lhu a2, 0x0354(v1)`), the value the Bombchu writes in H-moving (REMIX-TODO 2). No C insert until ITStruct has that field.

### H4. Bomb explosion data (linkshared.asm:413-440, `bomb_explosion_`)

- Hook `0x801863B0`, function `itLinkBombExplodeUpdateAttackEvent` (`itlinkbomb.c:528`, address comment 0x801863AC). Same file-pointer selection as H3 for the explosion.

### H5. Reflect with the Franklin Badge runs the Bombchu blast zone (Reflect.asm:672-707)

- Hook `OS.patch_start(0xEBCDC, 0x8017129C)`, reflect path of the item hit processing. The vanilla function containing 0x8017129C is `itProcessProcHitCollisions` (`decomp/src/it/itprocess.c:1015`, nearest-entry match, not verified). Not YLINK-only: the custom reflect type 1 calls `item+0x398` (`lw a1, 0x0398(v1)`), so YLINK's Bombchu is destroyed through `ftRemixYoungLinkBombchuBlastZone`.
- Insert: owner of Reflect.asm. Needs the item+0x398 field (REMIX-TODO 3).

### H6. Up-special spin velocity (linkshared.asm:675-711, `up_special_velocity_`)

- Hook `OS.patch_start(0xDEDC8, 0x80164388)`. Function `ftLinkSpecialAirHiSetStatus`, `decomp/src/ft/ftchar/ftlink/ftlinkspecialhi.c:354`. The vanilla line is `fp->physics.vel_air.y = FTLINK_SPINATTACK_AIR_VEL_Y;`.
- Remix values: YLINK 52.0F (0x42500000), JLINK 71.0F (0x428E...), ELINK 72.0F (0x4290...), default 69.0F (0x428A...).
- Insert:
  ```c
  #ifdef REGION_REMIX
  fp->physics.vel_air.y = (fp->fkind == REMIX_ID_YLINK) ? 52.0F
      : (fp->fkind == REMIX_ID_JLINK) ? 71.0F
      : (fp->fkind == REMIX_ID_ELINK) ? 72.0F
      : FTLINK_SPINATTACK_AIR_VEL_Y;
  #else
  fp->physics.vel_air.y = FTLINK_SPINATTACK_AIR_VEL_Y;
  #endif
  ```
  Check that `REMIX_ID_ELINK` exists in `port/remix/remix_roster_data.h` before use.

### H7. Up-special landing lag (YoungLink.asm:176-290, `up_special_landing_`)

- Hook `OS.patch_start(0xDEA30, 0x80163FF0)`, returns to 0x80163FF8. Function `ftLinkSpecialAirHiProcUpdate`, `ftlinkspecialhi.c:223`. The landing lag passed to `ftCommonFallSpecialSetStatus` (macro `FTLINK_SPINATTACK_LANDING_LAG`, `ftlink.h:21`) is replaced.
- Remix values (float32 data, YoungLink.asm:177-180): YLINK 0.33F (0x3EA8F5C3, "25 frames"), JLINK 0.75F, LINK keeps the vanilla value (`0x8018CA28`, expected 0.65F; verify).
- Insert:
  ```c
  #ifdef REGION_REMIX
  landing_lag = (fp->fkind == REMIX_ID_YLINK) ? 0.33F
      : (fp->fkind == REMIX_ID_JLINK) ? 0.75F
      : FTLINK_SPINATTACK_LANDING_LAG;
  #else
  landing_lag = FTLINK_SPINATTACK_LANDING_LAG;
  #endif
  ```
  The `up_special_aerial_drift_` hook (linkshared.asm:713-738, 0x80163FF8) only changes J Link (0.75 vs 0.6). YLINK keeps 0.6, so it is not a YLINK special change.

### H8. Up-special weapon struct (linkshared.asm:586-625, `up_special_fix_`)

- Hook `OS.patch_start(0xE7594, 0x8016CB54)`. Function `wpLinkSpinAttackMakeWeapon`, `decomp/src/wp/wplink/wplinkspinattack.c:115`. Selects `YoungLink.up_special_struct` for YLINK (linkshared.asm:10-14, data: `dw 0x03000000`, `dw 0x00000008`, not a C port).
- REMIX-TODO(data): needs the YLINK up-special struct as a data port before the insert can use it. The C insert would pick that struct when `fp->fkind == REMIX_ID_YLINK`.

### H9. Boomerang: weapon struct and speed (linkshared.asm:628-672, YoungLink.asm:185-254)

- `boomerang_fix_` `OS.patch_start(0xE8500, 0x8016DAC0)`: selects `YoungLink.boomerang_struct` (linkshared.asm:17) for YLINK. Data, REMIX-TODO(data) as H8.
- `boomerang_speed_` (YoungLink.asm:185-254), hooks `0x8016DB38` (slow) and `0x8016DB5C` (fast). Function `wpLinkBoomerangMakeWeapon`, `decomp/src/wp/wplink/wplinkboomerang.c:552`. The `slow` path is the TILT call (`WPBOOMERANG_VEL_TILT` 85.0F), the `fast` path is SMASH (`WPBOOMERANG_VEL_SMASH` 114.0F).
  - Slow, YLINK: 70.0F (`0x428C0000`). The asm comment says 65, but the hex is 70.0F. Verify (U11).
  - Fast, YLINK: 99.0F (`0x42C60000`).
  - Kirby or J Kirby copying Young Link use the copied power's id from the N64 0x0ADC field of the FTStruct (raw offset, no decomp field name confirmed).
- Insert (replaces the vel_mul argument of the two `wpLinkBoomerangGetAngleSetVel` calls):
  ```c
  #ifdef REGION_REMIX
  tilt_vel = (owner_kind == REMIX_ID_YLINK) ? 70.0F : WPBOOMERANG_VEL_TILT;
  smash_vel = (owner_kind == REMIX_ID_YLINK) ? 99.0F : WPBOOMERANG_VEL_SMASH;
  #endif
  ```
  `owner_kind` is `fp->fkind`, or the copied power for Kirby and J Kirby.
- `boomerang_crash_fix_` (linkshared.asm:788-820, hooks 0x8016D364, 0x8016D3F4, 0x8016D58C): not YLINK-specific. It fixes a double-destroy crash for all Link-family boomerangs. Owner: `wpLinkBoomerangCheckOwnerCatch` (`wplinkboomerang.c:301`) and `wpLinkBoomerangProcUpdate` (`wplinkboomerang.c:354`). Recorded, no port.

### H10. Bombchu item descriptor (YoungLinkDSP.item_info_array, YoungLinkSpecial.asm:50-56)

- Data at RAM 0x805421DC: `dw 0x15` (BOMBCHU_ID), `dw Character.YLINK_file_1_ptr`, `dw 0x40` (footer offset), `dw 0x12000000`, `dw 0`, then the state-0 "unused" proc list whose main is `held_main_` (`ftRemixYoungLinkBombchuHeldProcUpdate`).
- REMIX-TODO(data): the Remix item subsystem (`Item.*.SPAWN_ITEM`, `item_info_array_table` in `Item.asm`) must build the descriptor. Not ported.

### H11. CPU pull-out bombs (linkshared.asm:740-786, `cpu_pull_out_bombs_`)

- Hook `OS.patch_start(0xB3444, 0x80138A04)`. The containing function is not in `vanilla_funcs.txt` (nearest-entry lookup gives `mnPlayersVSPuckProcUpdate`, which is wrong for this code). Locate by the call site before inserting.
- Behaviour: YLINK is added to the bomb-pulling set with Link and ELink, so the CPU pulls bombs like Link.
- Insert: `if (fkind == REMIX_ID_YLINK) goto pull_bomb;` (equivalent to the asm `bne a0, at` branch).

### H12. CPU post process for YLINK (linkshared.asm:822-878, `cpu_post_process`)

- Remix routine registered as the YLINK (and Link, JLINK, ELINK) `cpu_post_process` table entry. It checks the CPU's NSP command and skips it when a boomerang already exists (`lw at, 0xADC(a0)`), then runs AI command `0x80132758` with `AI.ROUTINE.NULL`.
- Not ported: the AI command table layout and the function at 0x80132758 have no decomp name in `vanilla_funcs.txt`. REMIX-TODO(AI): port with the AI owner.

### H13. CPU AI attack tables (YoungLink/AI/Attacks.asm, all 63 lines)

- Data only: `add_attack_behaviour` / `add_custom_attack_behaviour` lists (USPG, NSPG, NSPA, grab, jab, smashes, aerials) and table patches `ai_long_range = AI.LONG_RANGE.ROUTINE.NSP_SHOOT`, `nsp_shoot_custom_move = AI.ROUTINE.DSP`, and `ai_behaviour = CPU_ATTACKS`. Consumed by the Remix AI tables, not C.

### H14. Action parameters and movesets (YoungLink.asm:45-78)

- Moveset `.bin` data for USP (0xE2, 0xE3, 0xE4), NSP (0xE5, 0xE8, Boomerang and BoomerangAir use the NSP moveset), and Bomb (0xEB, 0xEC) is loaded by the Remix moveset loader. No C.
- Only `edit_action(YLINK, 0xE4, ..., YoungLinkUSP.direction_)` (YoungLink.asm:87) is a routine, and it is ported.

### H15. Non-special table patches (YoungLink.asm:88-174)

- Menu zoom, crowd chant FGM, default costumes, team and shield colours, action strings, Remix 1P ending BGM. None change specials; no port.

## REMIX-TODO markers in the file

1. ITStruct FGM handle (N64 0x1D0 pointer, 0x1D4 id, asm lines 166-236). `ftRemixYoungLinkBombchuEndFgm` is a no-op and the start call does not store the handle. The rolling FGM can play without being ended until the field exists.
2. ITStruct u16 at N64 0x354 (asm line 395, `sh t8, 0x0354(a3)`), read by `bomb_flash_` (H3). Not written until the field exists.
3. ITStruct blast-zone routine at N64 0x398 (H2, H5), past the decomp ITStruct end (`proc_dead` is at 0x394). Needs a field.

## Uncertainties (asm line numbers)

- U1. `begin_explosion_` (line 250) and `moving_main_` (line 389) call 0x80186368. The vanilla address is not in the decomp by an address comment. The port uses `itLinkBombExplodeInitVars` (`itlinkbomb.c`, unaddressed function between `itLinkBombDroppedSetStatus` 0x80186314 and `0x801863AC`). Confirm the address.
- U2. `reflect_` (lines 605-621): Remix reads float `f6` left by `itMainCommonProcReflector` (0x80173434, `itmain.c:840`) and stores it as the new `lr`. The C version assumes that `f6` holds the owner's `lr` (the vanilla reflector multiplies `vel_air.x` by `fp->lr`). Verify in the asm of 0x80173434.
- U3. `update_direction_` (lines 147-153) and `held_main_` (lines 349-352) use `0x34` of the DObj joint, named `rotate.vec.f.y` in the decomp. The Remix comment says "z rotation". The port writes offset 0x34, matching the store, not the comment.
- U4. The moving-Bombchu timer (N64 0x2C0) is `vel_scale` (f32) in the decomp but an integer in Remix (lines 384-431). The port reads it through an `s32 *` cast (`ftRemixYoungLinkBombchuTimer`). Aliasing risk: build with `-fno-strict-aliasing`, or add a proper s32 field.
- U5. Bit masks in `begin_moving_` (lines 206-209, 222-224) and `begin_falling_` (lines 271-273): `andi 0xCF` on 0x2CE and `ori 0x20/0x80` on 0x2CF are mapped to `pickup_wait` (12-bit, MSB first from 0x2CE): 0x300, 0x002, 0x008. Confirm the decomp bitfield order against the N64 layout.
- U6. `begin_falling_` `sh r0, 0x0352` (line 270) is written to `item_vars.linkbomb.scale_int` (decomp name). The Remix comment says "invincibility timer".
- U7. Moving timer compares (lines 402-420): the float compares against 96.0 and the integer port are equivalent. The colanim 0x4F and length 0x60 (lines 399-401) are literals, not decomp names.
- U8. `thrown_collision_` stores the `mpProcessUpdateMain` result (line 470) and never reads it. The port drops the store.
- U9. `begin_falling_` is passed to `itMapCheckLRWallProcNoFloor` as a `void` proc (line 543). The port keeps it `void`.
- U10. `held_main_` (line 344): `lw t1, 0x0010(t1)` (child joint) runs in the delay slot of `beqz`, so it executes even when there is no owner. The port reads the child only with an owner. Equivalent, no side effect.
- U11. YLINK slow boomerang value: asm line comment says "float: 65", the `lui` value 0x428C decodes to 70.0F. The port uses 70.0F (hex authoritative). Confirm with the original Remix build.
- U12. `ftCaptainSpecialHiProcInterrupt` is called as "turn subroutine (copied from captain falcon)" (line 17). Confirm that this decomp function is the same turn routine.
- U13. Stick: `lb` (line 21) gives a signed 8-bit x, read as `fp->input.pl.stick_range.x` and taken as an absolute value. The port uses integer abs, same result as float abs.

## Summary of coverage

- Worklist scope: 1 of 1 routine ported (YoungLinkUSP / `direction_`).
- YoungLinkSpecial.asm: all routines ported (`direction_`, the 16 Bombchu routines and `bombchu_blast_zone_`). `item_info_array` is data (H10) and `state_change_` is a hook (H1).
- Vanilla hooks in YoungLink.asm and linkshared.asm that change his specials: H1-H12 recorded. H13-H15 are data or non-special.
- Unported Remix routine: H12 `cpu_post_process` (REMIX-TODO(AI)).

## Hooks applied (F5 pass)

Applied under `#ifdef REGION_REMIX` with the `#else` branch keeping the vanilla line. Each helper in
`ftremixyounglink.c` is a no-op for other owners (returns the vanilla value, or returns without writing).

- H6 `ftlinkspecialhi.c:ftLinkSpecialAirHiSetStatus` (0x80164348, hook 0x80164388 inside it) calls
  `ftRemixYoungLinkUpSpecialAirVelY(fp, FTLINK_SPINATTACK_AIR_VEL_Y)`. YLINK = 52.0F.
- H7 `ftlinkspecialhi.c:ftLinkSpecialAirHiProcUpdate` (0x80163FB0, hook 0x80163FF0 inside it) passes
  `ftRemixYoungLinkUpSpecialLandingLag(fp, FTLINK_SPINATTACK_LANDING_LAG)` to `ftCommonFallSpecialSetStatus`.
  YLINK = 0.33F. The asm uses the owner's character id, the port uses `fp->fkind`.
- H9 (speed only) `wplinkboomerang.c:wpLinkBoomerangMakeWeapon` (0x8016DA78, hooks 0x8016DB38 and 0x8016DB5C
  inside it): the TILT call uses `ftRemixYoungLinkBoomerangTiltVel` (70.0F, U11) and the SMASH call uses
  `ftRemixYoungLinkBoomerangSmashVel` (99.0F). Kirby and J Kirby use the copied power from N64 0x0ADC through
  `ftRemixFighterWord`, the same way the asm does it.
- H2 (owner id only) `itlinkbomb.c:itLinkBombMakeItem` (0x801865A0, save_character_id_ 0x801865D4 inside it)
  calls `ftRemixYoungLinkBombSetOwner(item_gobj, ftGetStruct(fighter_gobj))`. It stores
  `coll_data.cliff_id` (N64 0x0100) only for YLINK owners. Deviation: Remix writes the owner id for every
  owner. The port skips other owners to keep vanilla behaviour unchanged. The FGM handle (0x1D0) and blast-zone
  routine (0x398) writes are REMIX-TODO(ITStruct) inside the helper.

### Not applied

- H1 (Bombchu state table, `itMainSetStatus`, `it/itmain.c:577`): needs `it/itmain.c`, which is outside this
  worker's file list. Lead insert: under `#ifdef REGION_REMIX`, if `ip->kind == 0x15` and
  `ip->coll_data.cliff_id == REMIX_ID_YLINK`, set the status table to
  `dFTRemixYoungLinkBombchuStatusDescs` (already non-static in `ftremixyounglink.c`, declared extern there).
- H5 (Franklin Badge reflect runs the blast zone, `itProcessProcHitCollisions`): needs `it/itprocess.c`, which
  is forbidden to this worker. Also blocked on the ITStruct field at 0x398.
- H3 and H4 (bomb flash and explosion file pointer): the owner-based file selection needs the H10 descriptor.
  Not started. The 0x354 u16 read is REMIX-TODO 2.
- H8 (`up_special_struct`, linkshared.asm:10-14) and H9 struct (`boomerang_struct`, linkshared.asm:17): both are
  0x34-byte blobs copied from ROM (0x103DAC and 0x103DEC), not dw tables, so they cannot be ported as C
  without dumping ROM bytes. REMIX-TODO(data). The boomerang vanilla struct pointer swap in
  `boomerang_fix_` (0x8016DAC0) and `up_special_fix_` (0x8016CB54) is not applied.
- H10 (bomb item descriptor): REMIX-TODO(data), see above.
- H11, H12, H13 (AI pull-out, cpu_post_process, AI tables): REMIX-TODO(AI). Nothing applied.
- H14, H15: moveset and non-special table patches, no port.

### Verification

- `scripts/remix/syntax-check.sh` passes on `ftremixyounglink.c`, `ftlinkspecialhi.c`, `wplinkboomerang.c` and
  `itlinkbomb.c`. The `wplinkboomerang.c:181` warning (`sqrtf` result unused) is in vanilla code and existed
  before this pass.

## Step 2 wave 2026-10-14 (partial — kimi quota window)

- LANDED (W1, complete): H1 Bombchu status-table swap (itmain.c itMainSetStatus, owner via coll_data.cliff_id); H2 create_bomb_ owner-clone descriptor + save_character_id_ for ALL owners (item+0x398 mapped onto ITStruct proc_dead — REMIX-TODO 3 resolved); H3 bomb_flash_ bloat from owner clone file 1 + scale_id write (REMIX-TODO 2 resolved via ROM disasm: 0x0354 = scale_id); H4 bomb_explosion_ events from owner file; H11 cpu_pull_out_bombs_ (ftcomputer.c func_ovl3_8013877C: ELink/JLink/Peach share Link's case, YLink same body, Marina gated on 0x0ADC); U2 reflect verified against baserom; U6 0x352 = drop_update_wait (note misnamed scale_int).
- STILL OPEN: H5 reflect-path half (Reflect subsystem wave); REMIX-TODO 1 FGM handle 0x1D0 (overlays attack_pos — kept as deviation); H12/H13 AI; U5 bitfield audit.
- Boomerang destroy + shield/dair/drift: verified already ported, no change.
