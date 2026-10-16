# F7 leftovers (T2.8): four vanilla patches from the Remix leftover list

Scope: the four REMIX-TODO leftovers of ROADMAP T2.8. Two applied, two skipped.

## Patch status

| # | Asm (file:line) | RAM | Vanilla function (vanilla_funcs.txt) | Status |
|---|---|---|---|---|
| 1 | `Piano/Piano.asm:498` `extra_part_visibility_` | 0x800E7220 | `ftMainSetStatus`, `decomp/src/ft/ftmain.c` (hook after `fp->is_item_show = TRUE;`) | Applied |
| 2 | `items/WaddleDoo.asm:410` `beam_knockback_direction_fix` | 0x800E44EC | `ftMainProcessHitCollisionStatsMain`, `decomp/src/ft/ftmain.c:2803` | Skipped |
| 3 | `Item.asm:2893` `prevent_item_despawn_` (Marina) | 0x80173C04 | `itMapCheckDestroyDropped`, `decomp/src/it/itmap.c:262` | Applied |
| 4 | `Item.asm:1587` `clear_items_in_1p_` | 0x8018FD40, 0x8018E8C0 | `sc1PTrainingModeUnderlineProcDisplay` (+0x60), `sc1PTrainingModeMakeCPDisplay` (+0x50) | Skipped |

Not in this scope: `HRC.asm:665` `prevent_item_despawn_` (bat, Smashketball basketball), RAM 0x80173C0C, same
function as patch 3. It is a separate patch for the item owners.

## 1. Piano extra_part_visibility_ (applied)

- The hook replaces `ori t6,t4,2; sb t6,0x190(s1)` (is_item_show = TRUE) at 0x800E7220. The vanilla C is
  `fp->is_item_show = TRUE;` followed by `fp->is_cliff_hold = FALSE;` (`ftmain.c`), which matches the asm's
  `andi t7,t6,0xFE`. The hook sits between the two writes.
- Piano only (`fp->fkind == REMIX_ID_PIANO`). Actions 0xA6 (Grab) and 0xA7 (GrabPull), and 0x64..0x97
  (LightItemPickup..HammerLanding), keep the parts shown. Any other action hides them.
- Hide: `is_item_show = FALSE`; for joints 0x0E..0x10, `modelpart_status[joint - nFTPartsJointCommonStart]
  .modelpart_id_curr = -1` (asm byte 0x0975 + 2*joint, which equals index joint - 4, verified against
  `modelpart_status` at 0x097C and `nFTPartsJointCommonStart == 4`), `gcRemoveMObjAll(joints[joint])`, then
  `joints[joint]->dl = NULL`; finally `is_modelpart_modify = TRUE` (asm 0x018C mask 0x10, the revert flag that
  vanilla consumes on the next action change).
- The asm comment "push model part" is stale. The called routine is `gcRemoveMObjAll(DObj *)` (0x800091F4,
  `decomp/src/sys/objman.c:1411`), which the notes in `f7_kirby_piano2.md` flagged as a naming conflict.
- Helper: `ftRemixPianoExtraPartVisibility` in `decomp/src/ft/ftremix/ftremixleftovers.c`.

## 2. Waddle Doo beam_knockback_direction_fix (skipped)

- The asm works on the projectile struct `s4` (id at 0x0C, velocity at 0x20), the attack collision `s1` (angle
  at 0x2C, element at 0x0C) and the FTStruct `s5` (damage_angle 0x07F4, damage_element 0x07F8). The weapon
  velocity compare (`|vel.x| < 5.0F`, asm 0x800E44F0-0x800E4524) has no matching statement in
  `ftMainProcessHitCollisionStatsMain`. The only `ABSF(... vel_air.x) < 5.0F` in the function is at
  `ftmain.c:2822`, and it reads the ITStruct item, not the weapon.
- A partial hook would change the knockback direction of every weapon hit, so nothing was applied.
- Still open: map the register-level site (asm 0x800E44EC-0x800E4508 jumps to 0x800E4508 after the f0 load).
  The beam id `Projectile.id.DEDEDE_WADDLEDOO_BEAM` is defined in `Projectile.asm:29`, and has no C constant yet.

## 3. Marina prevent_item_despawn_ (applied)

- The hook replaces `beq v1,4` (times_thrown == ITEM_THROW_NUM_MAX return) at 0x80173C04. The vanilla
  despawn block is the one `if (ip->times_landed == ITEM_LANDING_DESPAWN_CHECK)` in `itMapCheckDestroyDropped`.
  Its body, `times_thrown == MAX || syUtilsRandIntRange(4) == 0`, is skipped for the Marina items.
- Asm exempt jump target 0x80173C28 is after the whole block, so the exempt items still run the
  `times_landed == ITEM_LANDING_NUM_MAX` proc_status branch. The C guard has the same effect.
- Items (Item.asm 2902..2910): Turnip, Gem, Shuriken, Boomerang, ClanBomb. The C compares `ip->kind` against
  `dITRemix<Name>ItemDesc.kind`, because Turnip and Shuriken have no kind macro.
- Vanilla edits: `decomp/src/it/itmap.c` (include of `ft/ftremix/ftremixhooks.h` under REGION_REMIX, and the
  `if` guard at the despawn block).
- Helper: `ftRemixLeftoverItemSkipsDespawn` in `ftremixleftovers.c`.

## 4. clear_items_in_1p_ (skipped)

- Both hooks are epilogue hooks that call `Item.clear_active_custom_items_` (asm 1587-1600). The vanilla
  functions at 0x8018FD40 and 0x8018E8C0 are named `sc1PTrainingModeUnderlineProcDisplay` and
  `sc1PTrainingModeMakeCPDisplay`, which do not match the asm's RTTF/BTT/BTP stage init. The map entries
  in `vanilla_funcs.txt` are likely wrong for these overlay addresses, so the hook site is unconfirmed.
- The port already clears this state on every scene change: `itRemixClearActiveItems()` in
  `decomp/src/sc/scmanager.c` (around line 1134). A 1P match enters through that scene boundary.
- Open: confirm the stage init function and whether the RTTF/BTT/BTP entry is a scene change. Then either
  call `itRemixClearActiveItems()` there or confirm the scene-boundary clear is enough.

## Wiring and verification

- `scripts/remix/syntax-check.sh` reports `syntax-check OK` for `ftremixleftovers.c`, `ftmain.c` and `itmap.c`.
  The `ftmain.c` warnings at lines ~4087 are pre-existing and unrelated.
- Lead: confirm that the build picks up the new `decomp/src/ft/ftremix/ftremixleftovers.c` (the notes of
  `f7_shared_a.md` raise the same glob question for ftremix files).
- Lead: no build, game run or git was done here. The Piano visibility and the Marina despawn need a game check
  (Piano grab and item pickup; Marina throw landing on the floor).
- No `REMIX-TODO` markers were added or removed for these patches.
