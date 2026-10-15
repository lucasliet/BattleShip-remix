# Shared patches: DKShared and CaptainShared

Sources: `.toolchain/pkgs/smashremix-src/src/dkshared.asm` (591 lines) and
`.toolchain/pkgs/smashremix-src/src/captainshared.asm` (761 lines).
Helper: `decomp/src/ft/ftremix/ftremixdkcaptainshared.c`, declared in
`decomp/src/ft/ftremix/ftremixhooks.h` (block "DK and Captain clones").
Vanilla lookups from `build/remixasm/refs/vanilla_funcs.txt`. Syntax check: all touched files
report `syntax-check OK` with `REGION_REMIX=1` in the build flags, so the Remix branches compile.

## Helpers exported (ftremixdkcaptainshared.c)

- `sb32 ftRemixDKSharedIsCargoClone(s32 fkind)`: TRUE for JDK, MARINA, NMARINA.
- `sb32 ftRemixCaptainSharedUsesFalconHandBone(s32 fkind)`: TRUE for GND, JFALCON, DRAGONKING.
- `EFDesc* ftRemixCaptainSharedKickDesc(FTStruct *fp, EFDesc *vanilla_desc)`: copy of the kick desc
  with `file_head = fp->data->p_file_special2` (file 7) for GND, JFALCON, DRAGONKING; vanilla desc otherwise.
- `EFDesc* ftRemixCaptainSharedPunchDesc(FTStruct *fp, EFDesc *vanilla_desc)`: same for the punch desc,
  `file_head = fp->data->p_file_special3` (file 8).

## Patches (DKShared)

| asm line | RAM | vanilla function (file) | status | C applied |
|---|---|---|---|---|
| 8 cargo_hold_fix_1 | 0x8014AB24 | ftCommonThrownProcUpdate (ft/ftcommon/ftcommonthrown1.c) | applied | capture kind term `&& !ftRemixDKSharedIsCargoClone` in the release check |
| 33 cargo_hold_fix_2 | 0x8014A18C | ftCommonThrowProcUpdate (ft/ftcommon/ftcommonthrow.c) | applied | DK-family OR cargo clone in the ThrowF branch |
| 57 item_fix_1 | 0x80145E20 | ftCommonGetProcUpdate (ft/ftcommon/ftcommonget.c) | applied | DK-family OR cargo clone chooses ftDonkeyThrowFWaitSetStatus |
| 81 item_fix_2 | 0x80146420 | ftCommonItemThrowProcUpdate (ft/ftcommon/ftcommonitemthrow.c) | applied | DK-family OR cargo clone in the heavy-throw action offset |
| 105 item_fix_3 | 0x80146A48 | ftCommonHeavyThrowDecideSetStatus (ft/ftcommon/ftcommonitemthrow.c) | applied | DK-family OR cargo clone adds nFTCommonStatusHeavyThrow4Start |
| 129 item_fix_4 | 0x800EA344 | ftParamUpdateDamage (ft/ftparam.c) | applied | heavy item drop skipped for DK-family, cargo clones included |
| 153 item_fix_5 | 0x801419B4 | ftCommonDamageUpdateMain (ft/ftcommon/ftcommondamage.c) | applied | heavy item catch resist for DK-family, cargo clones included |
| 177 item_fix_6 | 0x80140DC4 | ftCommonDamageCheckCatchResist (ft/ftcommon/ftcommondamage.c) | applied | DK-family throw-F hitstun check, cargo clones included |
| 202 fully_charged_check_ | 0x800EAC10 | ftParamMakeEffect (ft/ftparam.c) | applied | `case REMIX_ID_JDK:` falls into the Donkey charge-sparkle branch |
| 224 kirby_power_check_flash_ | 0x800E9A00 | ftParamResetStatUpdateColAnim (ft/ftparam.c) | applied | copy_id == REMIX_ID_JDK keeps the Donkey charge flash |
| 245 kirby_power_change_ (DK) | 0x80161EBC | ftKirbySpecialNInitPassiveVars (ft/ftchar/ftkirby/ftkirbyspecialn.c) | applied | `case REMIX_ID_JDK:` joins the Donkey case |
| 267 giant_punch_fix_1 | 0x80136C30 | (AI overlay, see uncertainty 2) | skipped: REMIX-TODO(AI) | none |
| 288 cpu_fix_2 | 0x80138F78 | func_ovl3_80138EE4 (ft/ftcomputer.c) | skipped: REMIX-TODO(AI) | none |
| 334 barrel_alternate | 0x80103428 | efManagerDonkeyEntryTaruMakeEffect (ef/efmanager.c) | skipped: REMIX-TODO(needs fighter) | none, see TODO list |
| 459 cpu_post_process (DK, JDK) | n/a | table patch | skipped: REMIX-TODO(AI) | none |

## Patches (CaptainShared)

| asm line | RAM | vanilla function (file) | status | C applied |
|---|---|---|---|---|
| 8, 11, 17 data patches | 0xA9AF5, 0xA9B00, 0xA9AD4 | falcon punch/kick anim structs (ROM) | skipped: REMIX-TODO(Size) | none |
| 122 get_kick_anim_struct_ | 0x80101EE4 | efManagerCaptainFalconKickMakeEffect (ef/efmanager.c) | applied (own file) | `ftRemixCaptainSharedKickDesc` for GND, JFALCON, DRAGONKING; Size scale update and `_apply_scale` skipped (REMIX-TODO(Size)) |
| 172 get_punch_anim_struct_ | 0x80101F90 | efManagerCaptainFalconPunchMakeEffect (ef/efmanager.c) | partly applied | `ftRemixCaptainSharedPunchDesc` for GND, JFALCON, DRAGONKING (fkind only). Kirby copy routing, WOLF slash, MTWO usmash and PokemonAnnouncer skipped (see TODO list) |
| 234 get_entry_anim_struct_ | 0x801035A0 | efManagerCaptainEntryCarMakeEffect (ef/efmanager.c) | skipped: REMIX-TODO(needs fighter) | none |
| 295 get_entry_file_ptr_ | 0x801035BC | efManagerCaptainEntryCarMakeEffect (ef/efmanager.c) | skipped: REMIX-TODO(needs fighter) | none |
| 339 get_punch_bone_ | 0x80101FC8 | efManagerCaptainFalconPunchMakeEffect (ef/efmanager.c) | applied | `ftRemixCaptainSharedUsesFalconHandBone` adds GND, JFALCON, DRAGONKING to joints[16] (asm 0x928) |
| 383 clown_car_animation1 | 0x801035C8 | efManagerCaptainEntryCarMakeEffect (ef/efmanager.c) | skipped: REMIX-TODO(needs fighter) | none |
| 412 clown_car_animation2 | 0x801035F4 | efManagerCaptainEntryCarMakeEffect (ef/efmanager.c) | skipped: REMIX-TODO(needs fighter) | none |
| 444 kirby_power_change_ (Captain) | 0x80161ECC | ftKirbySpecialNInitPassiveVars (ft/ftchar/ftkirby/ftkirbyspecialn.c) | applied | `case REMIX_ID_JFALCON:` and `case REMIX_ID_GND:` join the Captain case |
| 465 recovery_logic, 613 cpu_attack_weight, 689 cpu_post_process_falcon | n/a | table patches | skipped: REMIX-TODO(AI) | none |

Entry structs in the asm (not patches, but used by the patches above): `entry_anim_struct*`,
`kick_anim_struct*`, `punch_anim_struct*`, `slash_anim_struct_WOLF`, `usmash_anim_struct_MTWO`
are the EFDesc layouts. The clone entries are described in the TODO list.

## REMIX-TODO

- `REMIX-TODO(AI)`: giant_punch_fix_1 (dkshared 267), cpu_fix_2 (288), cpu_post_process (DK, JDK),
  recovery_logic (captainshared 465), cpu_attack_weight (613), cpu_post_process_falcon (689).
- `REMIX-TODO(Size)`: captainshared 8, 11, 17 (struct data for scaled Falcon punch render and kick update
  routines) and the kick `Size.falcon.kick.update_routine_._apply_scale` call. The clone kick and punch
  keep the vanilla update and render routines, so their gfx do not scale.
- `REMIX-TODO(PokemonAnnouncer)`: `PokemonAnnouncer.firepunch_announcement_` in get_punch_anim_struct_
  (Kirby copy path).
- `REMIX-TODO(needs fighter)`: barrel_alternate (dkshared 334), get_entry_anim_struct_ (234),
  get_entry_file_ptr_ (295), clown_car_animation1 (383) and clown_car_animation2 (412). Each needs the
  player struct (its character id and facing). `efManagerDonkeyEntryTaruMakeEffect(Vec3f *pos)` and
  `efManagerCaptainEntryCarMakeEffect(Vec3f *pos, s32 lr)` take no fighter. Fix: add a fighter
  parameter and update the callers. The entry descs are EFDesc literals (flags 0x04, dl_link 10,
  transform 0x1C/0x00, proc update efManagerHaveStructProcUpdate 0x800FD568, display gcDrawDObjTreeForGObj);
  the clown car needs per-file reloc offsets (0x248C/0x24C0/0x2530, 0x188C/0x18C0/0x1930 for Bowser and
  Conker), which the decomp does not yet expose.
- `REMIX-TODO(Kirby copy file)`: get_punch_anim_struct_ and get_kick_anim_struct_ use the copied power
  (`passive_vars.kirby.copy_id`) for Kirby and JKirby. The file pointers
  (Character.<NAME>_file_N_ptr of the copied fighter) are not in Kirby's `fp->data`, so the
  copy path keeps the vanilla Falcon desc.
- `REMIX-TODO(EFDesc literal)`: slash_anim_struct_WOLF (file 9, special4) and usmash_anim_struct_MTWO
  (file 7, special2) are separate EFDesc layouts with their own offsets (WOLF 0x8F0/0xA90/0/0xABC,
  MTWO 0x40/0x1E8/0/0x21C). Their only caller is the Kirby copy path.
- `REMIX-TODO(punch bone WOLF/MTWO)`: get_punch_bone_ uses joints[16] for WOLF and joints[18] for MTWO. Only
  the Falcon-family clones are applied here.

## Uncertainties

1. cargo_hold_fix_1 (asm 8-31): the branch target 0x8014AB3C is the continue path of the DK check.
   The C term `&& !ftRemixDKSharedIsCargoClone` is equivalent by reading the vanilla condition, but I did
   not disassemble the target block to confirm.
2. giant_punch_fix_1 (asm 267-286): the address 0x80136C30 maps to mnVSResultsOrderResults in
   vanilla_funcs.txt, which is an overlay mismatch. The asm comment says "when Donkey Kong Clones CPUs are
   functioning", so I treated it as AI and did not port it.
3. item_fix_1 (asm 57-79): the compare reads `0x8(v1)` (fkind). I matched it to ftCommonGetProcUpdate by the
   `ftDonkeyThrowFWaitSetStatus` call and the `ftCommonLiftWaitSetStatus` else branch.
4. get_punch_anim_struct_ (asm 171-231): the asm calls `PokemonAnnouncer.firepunch_announcement_` before the
   JFALCON and DRAGONKING checks, so that call runs for those clones too. The port skips it (REMIX-TODO).
5. kirby_power_check_flash_ (asm 224-243): the asm adds only JDK, not Marina. I followed that.
6. The Samus worker also edits ftparam.c (charge sparkle, kirby flash), ftkirbyspecialn.c (the copy-power
   switch) and efmanager.c (grapple glow). My edits sit beside theirs; I did not change their helpers.
   Their `ftRemixSamusSharedChargeSparkleFkind` and `ftRemixSamusSharedKirbyCopyPowerFkind` return the
   raw id for JDK, JFALCON and GND, which my case labels depend on.

## Vanilla files touched (all under `#ifdef REGION_REMIX`)

- decomp/src/ft/ftcommon/ftcommonthrown1.c, ftcommonthrow.c, ftcommonget.c, ftcommonitemthrow.c, ftcommondamage.c
- decomp/src/ft/ftparam.c (roster include, kirby flash, heavy drop, charge sparkle JDK case)
- decomp/src/ft/ftchar/ftkirby/ftkirbyspecialn.c (roster include, JDK / JFALCON / GND case labels)
- decomp/src/ef/efmanager.c (hooks include, kick desc, punch desc, punch bone)
- decomp/src/ft/ftremix/ftremixhooks.h (appended block only)
- New: decomp/src/ft/ftremix/ftremixdkcaptainshared.c

Not built, no cmake, no commits.
