# 0x41 GOEMON: GoemonNSP port notes (Ryo coin toss)

Source: `.toolchain/pkgs/smashremix-src/src/Goemon/GoemonSpecial.asm`, scope `GoemonNSP` (lines 5-669), plus the
Ryo projectile block it calls (lines 686-1270, `ryo_*`, `effect_projectile_main_`, the Ryo and effect structs).
Output: `decomp/src/ft/ftremix/goemon/ftremixgoemonnsp.c`, table `dFTRemixGoemonNSPProcs` / `dFTRemixGoemonNSPProcsNum`.
`ftremixgoemon.c` and `ftremixgoemonusp.c` are untouched. `scripts/remix/syntax-check.sh` prints `syntax-check OK`.

## Coverage

- Worklist routines ported: 13 of 13 (`air_begin_initial_`, `air_begin_main_`, `air_collision_`, `air_physics_`,
  `air_wait_main_`, `end_main_`, `ground_back_walk_physics_`, `ground_begin_initial_`, `ground_begin_main_`,
  `ground_collision_`, `ground_interrupt_`, `ground_wait_main_`, `ground_walk_physics_`).
- Static helpers, not in the table: `ground_wait_initial_`, `air_wait_initial_`, `ground_end_initial_`,
  `air_end_initial_`, `ground_transition_` (shared with `kirby_ground_transition_`), `ground_to_air_`, `air_to_ground_`,
  `air_control_`, the vanilla air physics copy, `ryo_stage_setting_`, `ryo_rotation_`, `ryo_main_`, `ryo_collision_`,
  `ryo_destruction_`, `ryo_reflect_`, `ryo_shield_bounce_`, `effect_projectile_main_`.
- Not ported (outside the worklist): `kirby_ground_collision_` and `kirby_air_collision_` (GoemonSpecial.asm 661-683)
  and `kirby_ground_to_air_` / `kirby_air_to_ground_` (only used by those two). Kirby.asm lines 427-436 reference the
  Kirby collisions, so Kirby's Goemon NSP falls back to the parent's collision until they are ported.
  `ryo_blast_zone_` is also not ported; see deviation 3.

## Registration (integration task, not done here)

- `decomp/src/ft/ftremix/ftremixported.c` must add `extern const FTRemixProc dFTRemixGoemonNSPProcs[];`,
  `extern const s32 dFTRemixGoemonNSPProcsNum;` and a `{ dFTRemixGoemonNSPProcs, &dFTRemixGoemonNSPProcsNum }` entry.
  Until then the GoemonNSP routines fall back to the parent (Mario). `ftremixported.c` was not edited.

## Vanilla patches

- None needed by this scope. The asm's `OS.patch_start(0xEB0D8, 0x80170698)` (GoemonSpecial.asm 2764) is the
  `item_hurtbox_patch_`, outside GoemonNSP.
- Vanilla calls used (names from `build/remixasm/refs/vanilla_funcs.txt`): `ftMainSetStatus` 0x800E6F24,
  `ftMainPlayAnimEventsAll` 0x800E0830, `ftAnimEndCheckSetStatus` 0x800D9480, `mpCommonProcFighterOnFloor` 0x800DDDDC,
  `mpCommonProcFighterLanding` 0x800DE6E4, `mpCommonSetFighterAir` 0x800DEEC8, `mpCommonSetFighterGround` 0x800DEE98,
  `mpCommonSetFighterWaitOrFall` 0x800DEE54, `ftPhysicsClampAirVelXMax` 0x800D8EB8,
  `ftPhysicsSetGroundVelAbsStickRange` 0x800D8A70, `ftPhysicsSetGroundVelTransferAir` 0x800D87D0,
  `ftPhysicsClampAirVelXStickRange` 0x800D8FC8, `ftPhysicsApplyAirVelXFriction` 0x800D9074,
  `gmCollisionGetFighterPartsWorldPosition` 0x800EDF24, `wpManagerMakeWeapon` 0x801655C8, `wpManagerGetGroupID` 0x801655A0,
  `wpMainDecLifeCheckExpire` 0x80167FE8, `wpMainApplyGravityClampTVel` 0x80168088, `wpMapTestAllCheckCollEnd` 0x80167C04,
  `efManagerDustExpandSmallMakeEffect` 0x800FF648, `efManagerSparkleWhiteMakeEffect` 0x80100480, FGM 0x800269C0
  (`func_800269C0_275C0`), `wpSamusBombProcReflector` 0x801692C4, `wpMarioFireballProcHop` 0x801686F8,
  cos 0x80035CD0 and sin 0x800303F0 (pikashared.asm 86/100 comments).
- Air physics copy (ROM 0x548F0, 0x40 bytes, N64 0x800D90E0): decoded with `tools/remix_disasm_rom.py`. The bytes
  are `if (fp->is_fastfall) ftPhysicsApplyFastFall else ftPhysicsApplyGravityDefault`, then
  `return ftPhysicsCheckClampAirVelXDecMax`. The port calls those functions. The `bnez v0` after the copy becomes
  the early return in `ftRemixGoemonNSPAirProcPhysics`.

## Shared Remix subsystems

- `REMIX-TODO(GFXRoutine)`: `ground_wait_main_` / `air_wait_main_` begin GFX routine `GOEMON_CHARGE` (id 0x73,
  bass.log, asm lines 281 and 325) through `jal 0x800E9814`. The port skips the GFX start; see `ftRemixGoemonNSPStartCharge`.
- Custom items: none in this scope.

## Weapons

- Ryo descriptor `WPDesc` is built on the stack per spawn (`ftRemixGoemonFillRyoDesc`), with
  `kind = 0x22` (`Projectile.id.GOEMON_RYO`, bass.log line 5675), `p_weapon = fp->data->p_file_special1`
  (file 6), `o_attributes` = the properties' hitbox offset (0x00 normal, 0x40 fire), and transform types
  `{nGCMatrixKindTra, 0x45, 0}` (the N64 render word `0x12450000`). Procs: update `ryo_main_`, map `ryo_collision_`,
  hit/shield/setoff/absorb `ryo_destruction_`, hop `ryo_shield_bounce_`, reflector `ryo_reflect_`.
- Effect descriptor (fire or sparkle, `ftRemixGoemonFillEffectDesc`): `o_attributes` 0x80 or 0xC0, transform
  `{nGCMatrixKindTra, 0x48, 0}`, update `effect_projectile_main_`, all other procs NULL.
- Flags: Ryo `WEAPON_FLAG_COLLPROJECT` (asm `lui a3, 0x8000`, which is bit 31); effect 0.

## Deviations from the N64 layout (read before reviewing)

1. Remix Ryo state. The asm keeps the charge flag at WPStruct 0x1B4 and the effect link at 0x1B8. The port keeps
   them in `FTRemixGoemonRyoVars`, overlaid on `wp->weapon_vars` (a union, so it is PC-safe). Raw N64 offsets
   would be wrong on LP64, because WPStruct and FTStruct contain pointers before those offsets. Consequence: the
   port no longer writes `attack_pos[1]` (N64 0x1B4..). Only the Ryo uses those words, so this is probably harmless.
2. Charge timer and flag. The asm keeps them at FTStruct 0x0B18 and 0x0B1C. The port uses
   `FTRemixGoemonChargeVars` overlaid on `fp->status_vars` (for the same reason as deviation 1).
3. Blast zone. `ryo_stage_setting_` stores `ryo_blast_zone_` at N64 WPStruct 0x298 (asm line 906), which is
   `weapon_vars`, not the blast-zone slot (`proc_dead`, 0x294 in the decomp). It never runs on N64, so it is not
   ported and `proc_dead` is left alone.
4. Duration. `ryo_stage_setting_` writes the properties duration to WPStruct 0x268 (asm line 908). That word is
   `is_camera_follow` / `is_static_damage`. `wpMainDecLifeCheckExpire` reads `lifetime` (0x264), which holds the
   instance id (asm line 883). So the duration is never read back. The port stores the instance id in `lifetime`
   as the asm does, and does not write the duration.
5. Explosion position. For a charged Ryo, `ryo_main_` (asm 1029-1032) and `ryo_destruction_` (1131-1134) pass the DObj
   address itself to `0x80100480` (no `+0x1C`), where the smoke path passes `&translate`. The port passes
   `&dobj->translate.vec.f` in both cases. This is probably a Remix bug, not behaviour anyone relies on.
6. `ground_transition_` target compare. Asm line 485 (`beql a2, at`) compares the caller's a2 with the wait action,
   but the comment says "branch if target action = NSP_Ground_Wait". a2 is undefined at that point, so the port
   follows the comment: a transition to the wait action starts at frame 0.
7. Effect main NULL guard. `effect_projectile_main_` (asm 1091-1094) dereferences the linked Ryo without a null
   check. The port returns TRUE (destroy) when the link or the Ryo's DObj is NULL.

## Uncertainties

- Kirby action ids 0x189-0x192 (Kirby NSP ground begin to air end). Derived like the Conker notes: 0xDC + 83 + the
  index of each `add_new_action(KIRBY, ...)` line in Kirby.asm (ground begin is index 90, air end is index 99).
  This matches the Conker and Mewtwo derivations but is not checked against `remix.z64`. A wrong id affects only
  Kirby and J Kirby.
- Remix Goemon ids 0xE5-0xEE are read from the `string_0x0E5`-`0x0EE` labels in Goemon.asm (lines 264-273). The
  table layout (ground_to_air_table, air_to_ground_table) agrees with those names.
- `REMIX_ID_GOEMON` (0x41) and `REMIX_ID_JKIRBY` (0x30) come from `remix/remix_roster_data.h`.
- `ground_collision_` uses `mpCommonProcFighterOnFloor` (0x800DDDDC, asm line 639). Conker's port uses
  `mpCommonProcFighterOnEdge` (0x800DDE84), so the two Remix routines differ here. The port follows the Goemon asm.
- `ground_interrupt_` for non-Goemon fighters (asm 410-411, the `beq t7, at` branch) takes the Kirby path for any
  fkind other than Goemon. The port does the same, so Ebi and other fighters that reuse `ground_walk_physics_`
  get the Kirby walk table, as in the asm.
