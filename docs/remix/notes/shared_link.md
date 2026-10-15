# LinkShared (linkshared.asm) patches

Source: `.toolchain/pkgs/smashremix-src/src/linkshared.asm` (976 lines).
Helpers: `decomp/src/ft/ftremix/ftremixlinkshared.c`, declared in `ftremixhooks.h` (block "Link clones").
Existing YoungLink hooks reused: `ftRemixYoungLinkUpSpecialAirVelY`, `ftRemixYoungLinkUpSpecialLandingLag`,
`ftRemixYoungLinkBoomerangTiltVel`, `ftRemixYoungLinkBoomerangSmashVel`, `ftRemixYoungLinkBombSetOwner`.

## Patch status

| asm line | RAM address | vanilla function (vanilla_funcs.txt) | status |
|---|---|---|---|
| 172-227 `get_entry_anim_struct_1` | 0x80102AE8 | efManagerLinkEntryWaveMakeEffect (ef/efmanager.c:5239) | NOT APPLIED, see E1 |
| 229-280 `get_entry_anim_struct_2` | 0x80102B44 | efManagerLinkEntryBeamMakeEffect (ef/efmanager.c:5258) | NOT APPLIED, see E1 |
| 287-375 `create_bomb_` (4 hooks) | 0x801865A0, 0x801865D4, 0x801865DC, 0x80186658 | itLinkBombMakeItem (it/itfighter/itlinkbomb.c:613) | PARTIAL, see B1 |
| 381-407 `bomb_flash_` | 0x801859DC | itLinkBombExplodeWaitUpdateScale (itlinkbomb.c:149) | NOT APPLIED, see B2 |
| 413-438 `bomb_explosion_` | 0x801863B0 | itLinkBombExplodeUpdateAttackEvent (itlinkbomb.c:528) | NOT APPLIED, see B2 |
| 443-477 `item_shield_fix_` (2 hooks) | 0x800E870C, 0x800E8760 | ftParamLinkResetShieldModelParts / ftParamSetHammerParams (ft/ftparam.c) | APPLIED, see S1 |
| 482-511 `grab_shield_fix_` | 0xC4A98 / 0x8014A058 | ftCommonCatchWaitSetStatus (ft/ftcommon/ftcommoncatch2.c:69) | APPLIED, see S1 |
| 516-550 `dair_fix_` (2 hooks) | 0x801508F4, 0x80150994 | ftCommonAttackAirLwProcHit / ProcUpdate (ft/ftcommon/ftcommonattackair.c) | APPLIED, see S1 |
| 555-583 `sword_trail_fix_` | 0x800E6768 | ftMainProcParams afterimage block (ft/ftmain.c ~4256) | APPLIED, see S2 |
| 589-622 `up_special_fix_` | 0x8016CB54 | wpLinkSpinAttackMakeWeapon (wp/wplink/wplinkspinattack.c:115) | APPLIED, see U1 |
| 628-670 `boomerang_fix_` | 0x8016DAC0 | wpLinkBoomerangMakeWeapon (wp/wplink/wplinkboomerang.c:566) | APPLIED, see U2 |
| 675-708 `up_special_velocity_` | 0x80164388 | ftLinkSpecialAirHiSetStatus (ft/ftchar/ftlink/ftlinkspecialhi.c:354) | APPLIED, see U3 |
| 713-736 `up_special_aerial_drift_` | 0x80163FF8 | ftLinkSpecialAirHiProcUpdate (ftlinkspecialhi.c:223) | APPLIED, see U4 |
| 788-820 `boomerang_crash_fix_` (3 hooks) | 0x8016D364, 0x8016D3F4, 0x8016D58C | wpLinkBoomerangCheckOwnerCatch / wpLinkBoomerangProcUpdate (wplinkboomerang.c) | APPLIED, see C1 |
| 738-780 `cpu_pull_out_bombs_` | 0x80138A04 | AI | REMIX-TODO(AI), skipped |
| 822-878 `cpu_post_process` (4 table patches) | table | AI | REMIX-TODO(AI), skipped |
| 882-976 `recovery_logic` (3 table patches) | table | AI (ledge_check included) | REMIX-TODO(AI), skipped |

## Applied C changes

Every vanilla change is under `#ifdef REGION_REMIX`; the `#else` keeps the original line.

- S1 Link family (`ftRemixLinkSharedIsLinkFamily`: LINK, NLINK, YLINK, NYLINK, ELINK, JLINK):
  - `decomp/src/ft/ftparam.c` ftParamLinkResetShieldModelParts, ftParamSetHammerParams (include added at top).
  - `decomp/src/ft/ftcommon/ftcommonattackair.c` ftCommonAttackAirLwProcHit, ftCommonAttackAirLwProcUpdate.
  - `decomp/src/ft/ftcommon/ftcommoncatch2.c` ftCommonCatchWaitSetStatus (include added at top).
- S2 sword trail (`ftRemixLinkSharedHasSwordTrail`: LINK, YLINK, ELINK, JLINK; NLINK excluded as in asm):
  - `decomp/src/ft/ftmain.c`, the `afterimage.is_itemswing == FALSE` case, condition `fkind == nFTKindLink`.
- U1 Up Special weapon: `decomp/src/wp/wplink/wplinkspinattack.c` wpLinkSpinAttackMakeWeapon spawns from a copy of
  `dWPLinkSpinAttackWeaponDesc`; `ftRemixLinkSharedUpSpecialWeapon` sets `p_weapon` to the clone's `p_file_main`
  (YLINK, ELINK, JLINK). Include added.
- U2 Boomerang weapon: `decomp/src/wp/wplink/wplinkboomerang.c` wpLinkBoomerangMakeWeapon spawns from a copy of
  `dWPLinkBoomerangWeaponDesc`; `ftRemixLinkSharedBoomerangWeapon` sets `p_weapon` to the owner clone's
  `p_file_special1`. Owner is `passive_vars.kirby.copy_id` for Kirby and J Kirby.
- U3 Up Special air velocity: `ftLinkSpecialAirHiSetStatus` chains
  `ftRemixLinkSharedUpSpecialAirVelY(fp, ftRemixYoungLinkUpSpecialAirVelY(fp, ...))`. Values: JLINK 71.0F, ELINK 72.0F,
  YLINK 52.0F (existing), vanilla 69.0F.
- U4 J Link Up Special drift: `ftLinkSpecialAirHiProcUpdate` passes
  `ftRemixLinkSharedUpSpecialFallDrift(fp, FTLINK_SPINATTACK_FALLSPECIAL_DRIFT)` as the drift argument (JLINK 0.75F).
- C1 Boomerang crash: `wpLinkBoomerangCheckOwnerCatch` now returns `sb32` (prototype in `wplinkboomerang.h` updated).
  Remix returns TRUE on catch without calling `wpMainDestroyWeapon`; `wpLinkBoomerangProcUpdate` RETURN branch returns
  the catch result. Vanilla path still destroys inside the function.

## Skipped, with reasons

- E1 Entry animation structs (asm 172-280, RAM 0x80102AE8 / 0x80102B44). The patch swaps the EFDesc passed to
  `efManagerMakeEffectNoForce` with a copy whose `file_head` is the owner's `p_file_special2` (file 7). The asm reads the
  player struct from `s0`, but `efManagerLinkEntryWaveMakeEffect(Vec3f *pos)` and
  `efManagerLinkEntryBeamMakeEffect(Vec3f *pos)` have no fighter parameter. Needed: add a fighter gobj parameter, or
  do the swap in the caller (entry code), plus EFDesc copies from the vanilla `dEFManagerLinkEntry{Wave,Beam}EffectDesc`
  (efmanager.c:1174) with `file_head` swapped. The same patch also covers Marth, Roy, Marina, Banjo, Goemon and Ebisumaru
  entries; those are other scopes, not Link clones.
- B1 Bomb creation (`create_bomb_`): not done. Missing: (a) per-owner ITDesc copy (`bomb_struct_elink`/`_jlink` are
  vanilla 0xF8-byte copies with `file_1` = owner); (b) the Young Link bombchu item and render routine 0x48 (existing
  YoungLink hooks cover the bombchu state table only); (c) saving the owner id on the item for later reads. The port has
  no ITStruct field for the owner. `coll_data.cliff_id` is only written (H2 in ftremixyounglink.c) and not read back, so
  it could carry the owner for ELINK/JLINK too, but that is an unverified design choice. REMIX-TODO(ITStruct).
- B2 Bomb flash and explosion (asm 381-438): they choose the file-1 pointer from the owner's struct
  (`*dItLinkBombItemDesc.p_file` is the vanilla one). Needs the owner from B1 (REMIX-TODO(ITStruct)).

## Uncertainties (asm line numbers)

- S1 item_shield_fix_ (443-477) and dair_fix_ (516-550) and grab_shield_fix_ (482-511): as written, each routine's
  branches all end at `jr ra` with no state change, so the asm alone does not show the effect. The port follows the
  stated intent (the Link checks in the callers also cover the clones). Verify in game: Young, E and J Link shield swap,
  grab shield and dair hitbox.
- S2 sword_trail_fix_ (555-583): for Link variants it returns to the next instruction (normal trail path), for others it
  jumps to 0x800E69B4. The port reads the same condition.
- U4 up_special_aerial_drift_ (713-736): the patch sets `a1` (the drift float) before the call, and the port assumes
  that `a1` is the second argument (drift) of `ftCommonFallSpecialSetStatus`. Verify J Link drift in game.
- C1 boomerang_crash_fix_ (788-820): relies on `wpProcessProcWeaponMain` destroying the weapon when its proc_update
  returns TRUE. Not verified in the port; check `wp/wpmanager.c` before relying on it.
- U2: `wpManagerMakeWeapon` reads `wp_desc->p_weapon` at spawn only (wpmanager.c:121) and does not keep the desc, so a
  local copy is safe.

## Build

`decomp/src/ft/ftremix/ftremixlinkshared.c` is new. CMake uses `file(GLOB_RECURSE ...)` on `decomp/src/ft`
(CMakeLists.txt:521), so re-run cmake configure to pick it up. Not built here, per brief.

## Syntax checks (scripts/remix/syntax-check.sh)

OK for: ftremixlinkshared.c, ftparam.c, ftcommon/ftcommonattackair.c, ftcommon/ftcommoncatch2.c, ftmain.c,
ftchar/ftlink/ftlinkspecialhi.c, wp/wplink/wplinkboomerang.c, wp/wplink/wplinkspinattack.c.
Warnings reported in those files are pre-existing, not on the changed lines.

## REMIX-TODO markers

- `REMIX-TODO(AI)`: cpu_pull_out_bombs_, cpu_post_process, recovery_logic (with ledge_check).
- `REMIX-TODO(ITStruct)`: create_bomb_ owner and per-owner bomb file; bomb_flash_; bomb_explosion_.
- Entry structs (E1) need a fighter parameter, not a REMIX-TODO comment in code.
