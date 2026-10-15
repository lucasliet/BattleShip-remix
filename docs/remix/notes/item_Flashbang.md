# Item.Flashbang (Remix id 0x4A, owner Peppy)

Source: `.toolchain/pkgs/smashremix-src/src/items/Flashbang.asm` (855 lines). Spawned by
`Peppy/PeppySpecial.asm` (PeppyDSP.main, line ~1549). The fighter caller is already wired in
`decomp/src/ft/ftremix/peppy/ftremixpeppydsp.c` (`itRemixFlashbangMakeItem(fighter, &pos, &vel, 0x80000002)`).

Port: `decomp/src/it/itremix/itremixflashbang.c` and `itremixflashbang.h`.
Descriptor: `dITRemixFlashbangItemDesc`, kind 0x4A, `SHOW_GFX_WHEN_SPAWNED` FALSE, no hold, consume, drop,
throw or touch procs.
Spawn: `GObj* itRemixFlashbangMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`.
Item file: `Character.PEPPY_file_6_ptr` = `fp->data->p_file_special1`, set in the desc copy at spawn.

## Routines

Ported (asm label, C name):

- `flashbang_stage_setting_` -> `itRemixFlashbangMakeItem`
- `flashbang_main_` -> `itRemixFlashbangProcUpdate` (state 0 and 1 proc_update)
- `flashbang_collision_` -> `itRemixFlashbangProcMap` (state 0 proc_map)
- `flashbang_resting_collision_` -> `itRemixFlashbangProcMapResting` (state 1 proc_map)
- `flashbang_exploding_main_` -> `itRemixFlashbangProcExploding` (state 2)
- `flashbang_hurtbox_collision_` -> `itRemixFlashbangProcHurtbox` (proc_hit and proc_shield)
- `flashbang_hitbox_collision_` -> `itRemixFlashbangProcDamage` (proc_damage)
- `flashbang_blast_zone_` -> `itRemixFlashbangProcDead` (ITStruct proc_dead, Remix slot 0x0398)
- `flashbang_detect_collision_` -> `itRemixFlashbangDetectCollision` (static copy, see below)
- `flashbang_explosion_` -> `itRemixFlashbangExplode`
- `flashbang_begin_explosion_` -> `itRemixFlashbangSetExplosion`
- `flashbang_explosion_hitboxes_` -> `itRemixFlashbangExplosionAttackEvent`
- `flashbang_begin_main_` -> `itRemixFlashbangSetAir`
- `flashbang_begin_resting_` -> `itRemixFlashbangSetResting`

Helpers that are not asm labels: `itRemixFlashbangDragGround`, `itRemixFlashbangDecaySpeedAir`,
`itRemixFlashbangUpdateRotation*`, `itRemixFlashbangUpdateHitRecords`, `itRemixFlashbangSpawnSmoke`,
`itRemixFlashbangShortenFuseOnHurt`, `itRemixFlashbangSetOwnerFromAttacker`,
`itRemixFlashbangLaunchFromDamage`, `itRemixFlashbangSetActive`, `itRemixFlashbangSetupRender`,
`itRemixFlashbangSetupState`, `itRemixFlashbangSetupLaunch`.

Routines ported: 14 of 14 asm routines in `Flashbang.asm`, plus the `item_info_array` and
`item_state_table` (states 0 to 2).

Item-state table: the port's `ITStatusDesc` array has three states. State 0 is the ITDesc procs
(asm 0x14 to 0x30). State 1 is 0x34 to 0x50. State 2 is the exploding main, labelled 0xD4 in the
asm, but the port places it at base + 2 * 0x20 (the index used by `itMainSetStatus`).

## Vanilla patches and dependencies

- `flashbang_detect_collision_` (asm 686-699) copies 0x88 and 0x2C bytes from vanilla
  `itMapTestAllCollisionFlag` (0x801737B8, `decomp/src/it/itmap.c:153`) with a changed branch.
  The port replaces the copy with a static C function (same shape as the Conker grenade detect),
  so no vanilla change is needed. The notes' uncertain point is the landing-floor condition (below).
- `flashbang_explosion_hitboxes_` (asm 519-525) jumps into vanilla
  `itLinkBombExplodeUpdateAttackEvent` (0x801863AC, `decomp/src/it/itfighter/itlinkbomb.c:528`)
  at 0x801863B8, with the file pointer loaded from `item_info_array`. The port runs its own
  event code, so no vanilla patch is needed. None of the vanilla function's callers need changes.
- Vanilla calls the item relies on (ported directly): `itManagerMakeItem` (0x8016E174), `itMainSetStatus`
  (0x80172EC8), `itMainApplyGravityClampTVel` (0x80172558), `itMapSetAir` (0x80173F78),
  `itMapSetGround` (0x80173F54), `itLinkBombCommonSetHitStatusNormal` (0x80185CD4),
  `itMainClearOwnerStats` (0x8017279C), `itMainRefreshAttackColl` (0x8017275C),
  `itMapCheckLRWallProcNoFloor` (0x801735A0), `itMapCheckCollideAllRebound` (0x801737EC),
  `mpProcessUpdateMain` (0x800DA034), `mpCommonRunItemCollisionDefault` (0x800DF058),
  `ftCommonDamageGetKnockbackAngle` (0x801409BC), `efManagerQuakeMakeEffect` (0x801008F4),
  `func_800269C0_275C0` (FGM, 0x800269C0), `itMainCommonProcHop` (0x801733E4),
  `itMainCommonProcReflector` (0x80173434).
- Remix globals written by the asm: `Toggles.entry_flash_guard` and the screen blend-colour
  command at 0x80131A40 (`flash_array_`, asm 598-606). Both are REMIX-TODO (see below).

Required C for the vanilla side: none. The port is self-contained, and no vanilla function must
call a routine of this item.

## REMIX-TODO markers

- `REMIX-TODO(GFX)`: screen flash. Asm 598-606 writes `flash_array_` to 0x80131A40 unless
  `Toggles.entry_flash_guard` is on. The port does not do the screen flash.
- `REMIX-TODO(GFX)`: `Item.BlueShell.hit_gfx_` (asm 636), called after the FGM. No decomp port
  exists.
- `REMIX-TODO(DObj)`: asm 624 sets the first joint's byte 0x0054 to 2. The DObj field is not named.
- `REMIX-TODO(ITStruct)`: asm 184 clears the ITStruct field at 0x0100 (a Bomb character id reference).
  No named field.
- `REMIX-TODO(ITStruct)`: asm 127 sets bit 0x10 at 0x0158 ("unknown bitflag"). No named field.

## Mapping decisions (asm offset to named field)

- ITStruct 0x0008 owner, 0x0014 team, 0x0015 port (combo ownership), 0x002C/0x0030/0x0034 velocity,
  0x0108 kinetic state, 0x010C hitbox (`attack_coll.attack_state`), 0x0110 damage, 0x011C element,
  0x0138 size, 0x0140/0x0144/0x0148 knockback scale/weight/base, 0x0248 hurtbox
  (`damage_coll.hitstatus`), 0x02A4 damage lr, 0x0298 damage queue, 0x029C damage angle, 0x02A8
  damage gobj, 0x02C0 lifetime, 0x033E multi, 0x0398 proc_dead, 0x0092 collision flags
  (`coll_data.mask_stat`).
- ITStruct 0x0340 bits 0xF0 are cleared by `flashbang_begin_explosion_` (asm 653-655). This is `event_id`
  (4-bit bitfield), so the port sets `event_id = 0`.
- ITStruct 0x02CE, the bit 0x80 set at spawn (asm 171-173), maps to `is_allow_pickup`. See uncertainty 2.
- ITStruct 0x0156, the halfword set to 1, then to 0x2B7 (asm 622-672), maps to `attack_coll.fgm_id`.
  The offset follows from the attack-coll layout (`ITEM_ATKCOLL_NUM_MAX` = 2, so `size` is at 0x138
  and `fgm_id` at 0x156).
- FTStruct 0x0ADC (owner slot) is `fp->passive_vars`. The port writes the GObj of the flashbang, because
  `ftRemixPeppyDSPActiveGrenade` (ftremixpeppydsp.c) reads a GObj. The asm stores the ITStruct pointer
  (asm 158, `v1` = item special struct). The fighter only checks the slot for NULL, so the value matters
  only as non-NULL.
- Owner power: FTStruct 0x0B20 has no named field. The port reads it with `ftRemixFighterWord`
  (`ftremixfields.h`), as ftremixpeppydsp.c does.
- ITStruct 0x0354 is written as 4 at spawn into `ITRemixFlashbangVars.unk_0x354`.
- Damage id: `Damage.id.FLASHBANG` = 0x0C. Damage.asm line 960 adds it as the 6th `add_damage_type`
  (ids start at 0x6 + count: SHADOW 7, STUN 8, LASER 9, DEKU_STUN 10, BURY 11, FLASHBANG 12).
- ITDesc transform `0x1B000000` = `nGCMatrixKindTraRotRpyR` (27 = 0x1B), with the other bytes 0.
- Spawn flags: the 5th argument Peppy passes (0x80000002) is never read. `flashbang_stage_setting_`
  stores 0 into its own 5th argument slot (asm 104), so the port passes FALSE to `itManagerMakeItem`
  and ignores `flags`.
- Spawn velocity: the `vel` argument goes through to `itManagerMakeItem` (asm 110-111 keep a3), and
  the launch overwrites it.

## Uncertainties (asm line numbers)

1. Blast zone return (asm 850-855): `flashbang_blast_zone_` returns v0, which it never sets (garbage).
   The port returns TRUE, so the item is destroyed off-stage. Check this on hardware.
2. Asm 171-173 sets the halfword 0x2CE bit 0x80 with the comment "allows owner's hitboxes to collide
   with the hurtbox". The port maps it to `is_allow_pickup`, from the bit's position in the BE
   bitfield order (is_allow_pickup, is_hold, times_landed). Those two meanings disagree. Verify
   that a flashbang cannot be picked up in play. If pickup must stay off, remove the assignment.
3. Asm 144 and 163: the comment for the y velocity says cos, but f0 holds sin(ANGLE) at that point.
   The port uses sin.
4. Asm 246-250: the comment says the ground drag is 0.875, but the hex (0x3F70 0000) is 0.9375.
   The port uses 0.9375.
5. Asm 352-362: `cvt.w.s` rounds to nearest, not toward zero, so the port uses `lrintf`. Other items
   in this port truncate with a cast; that is a possible inconsistency.
6. Asm 375-378 (smoke): `efManagerFlameRandgcMakeEffect(&pos)` takes one argument, and that is what
   the asm passes (a1 is unset). `vanilla_funcs.txt` lists 0x800FE9B4 under both
   `efManagerFlameLRMakeEffect` and `efManagerFlameRandgcMakeEffect`. The one-argument form is
   chosen. Check the function at `decomp/src/ef/efmanager.c:2666`.
7. Asm 795-798 (hit angle): `ftCommonDamageGetKnockbackAngle(angle, a1, knockback)`. The asm does
   not set a1, so its value is whatever was in the register. The port passes `ip->ga`, following the
   Conker grenade port.
8. Asm 686-699, detect collision: the port's landing-floor condition is copied from the Conker
   grenade. The asm (`beql v0, r0`) only runs the landing-floor call when the first part of the copied
   function returned non-zero. Check `mpProcessSetLandingFloor` against 0x801737B8.
9. Attack event data: the explosion event table is read at owner file offset 0x88, the same offset
   the Conker grenade port uses for the vanilla link-bomb routine. The linker symbol
   `llLinkMainBombAttackEvents` was not found in the repo, so the value is unverified.
10. `flashbang_begin_explosion_` (asm 659-662): knockback scale 70 and base 50 are set before the
    first event, and the damage (7), size (350) and element (0x0C) are set after it. The port keeps that
    order. If the vanilla event also writes element, it is overwritten by the later assignment.
11. Explosion timing: the asm ends the explosion when `multi` reaches 1 (asm 502), so the explosion
    lasts one frame. The port keeps that.
