# 4C_LANKY_NSP: Lanky Kong neutral special

Source: `.toolchain/pkgs/smashremix-src/src/Lanky/LankySpecial.asm`, scope `LankyNSP` (lines 6-398).
Port: `decomp/src/ft/ftremix/lanky/ftremixlankynsp.c`, table `dFTRemixLankyNSPProcs` / `dFTRemixLankyNSPProcsNum`.
Routines in the worklist: 3 (`ground_initial_`, `air_initial_`, `main_`). All 3 are ported.
Weapon: the grape (`grape_main_`, `grape_collision_`, `grape_hit_`, `grape_destruction_`, `grape_projectile_struct`, `grape_stage_setting_`) is ported as file-local statics, not table entries, because the worklist does not list it.

## 1. Vanilla patches

None. `LankySpecial.asm` has no `OS.patch_start` blocks and no `j`/`jal` hooks into vanilla code. The vanilla calls it makes are listed below, all resolved through `vanilla_funcs.txt`:

| RAM | Decomp function | Used by |
|---|---|---|
| 0x800E6F24 | `ftMainSetStatus` | initial, cancel |
| 0x800E0830 | `ftMainPlayAnimEventsAll` | initial, cancel |
| 0x800DEE54 | `mpCommonSetFighterWaitOrFall` | `main_` idle check |
| 0x800EDF24 | `gmCollisionGetFighterPartsWorldPosition` | `main_` spawn |
| 0x801655C8 | `wpManagerMakeWeapon` | grape spawn |
| 0x80035CD0 | `__cosf` (wolf port, ftremixwolf.c:211) | grape velocity |
| 0x800303F0 | `__sinf` (same source) | grape velocity |
| 0x80167FA0 | `wpMainVelSetModelPitch` | grape spawn |
| 0x80167FE8 | `wpMainDecLifeCheckExpire` | grape proc_update |
| 0x80168088 | `wpMainApplyGravityClampTVel` | grape proc_update |
| 0x80167C04 | `wpMapTestAllCheckCollEnd` | grape proc_map |
| 0x800FF648 | `efManagerDustExpandSmallMakeEffect` | grape expire, destruction |
| 0x800FDC04 | `efManagerDamageNormalLightMakeEffect` | grape proc_hit |
| 0x800269C0 | `func_800269C0_275C0` (FGM play) | grape destruction (FGM 1521) |
| 0x801686F8 | `wpMarioFireballProcHop` | grape proc_hop |
| 0x801692C4 | `wpSamusBombProcReflector` | grape proc_reflector |

## 2. Shared Remix subsystems

None. Everything the routines touch is vanilla or a small fighter-local helper that is ported in the file.

## 3. Constants and data taken from the asm

- Action ids: `Lanky.Action.NSPG` 0xDF and `NSPA` 0xE0 (`Lanky/Lanky.asm:38-39`). Kirby's ids are `Kirby.Action.LANKY_NSP_Ground` 0xE7 and `LANKY_NSP_Air` 0xE8 (`Kirby/Kirby.asm:487-488`).
- Projectile id: `Projectile.id.LANKY_NUT` = 0x25 (`Projectile.asm`, the 6th `add_projectile` after the vanilla last id 0x1F; the same counting gives `SHEIK_NEEDLE` 0x23 and `CONKER_NUT` 0x24, which match the existing ports).
- Hand joints: the asm reads `FTStruct` 0x092C (Kirby) and 0x0930 (others). `joints` is at 0x08E8 with 4-byte entries, so these are `joints[17]` and `joints[18]`.
- Ammo: `FTStruct` 0x0B20 is a status_vars scratch word (0x0B18..0x0B4B), accessed with `ftRemixFighterWord`.
- Temp variables 1/2/3 are `motion_vars.flags.flag0/1/2` (0x17C/0x180/0x184).
- Grape properties (`grape_properties_struct`): duration 70, max speed 50.0, gravity 0.0, rotation speed 0.4, angles 0.0 (ground and air), initial speed 40.0, palette index 0.0. The min speed, bounce and file-pointer fields are not read by the grape routines, so the port does not have constants for them.
- Frame 14.0 is 0x41600000 (verified with python).

## 4. Uncertainties

1. **Grape duration is not written** (asm `grape_stage_setting_`, line 218: `sw t3, 0x0268(v1)` with t3 = 70). N64 0x268 is `is_camera_follow` and `is_static_damage`, not `lifetime` (0x264, `wptypes.h:223`). `wpManagerMakeWeapon` already clears both flags (`wpmanager.c:270, 274`), so the port does not write the word. `wpMainDecLifeCheckExpire` (`wpmain.c:60`) expires only when `lifetime` reaches exactly 0. Because nothing sets `lifetime`, a grape may never expire by duration and would only be removed by a collision. This matches the Conker port's treatment of the same store, but it is an open behaviour question for review.
2. **Dead spawn offsets** (asm lines 103-114). The asm writes the offsets (Kirby x = 0x43960000 = 300.0, not the 200 in the comment at line 112; non-Kirby y = 0xC3480000 = -200.0) before calling `gmCollisionGetFighterPartsWorldPosition` (line 118). That function always overwrites the vector (`gmcollision.c:515`), so the offsets never reach the grape. The port omits them and keeps the call and the z = 0 store (asm line 120).
3. **Dead loads and stores.** Lines 93, 94 and 96 load `t6` and `at` values that are overwritten before they are read. The `lw a0` duplicates at lines 29-30 are the same. The port does not reproduce them.
4. **Kirby selection** (asm lines 16-21, 52-57, 97-101, 154-160). The asm adds `Kirby - Lanky` to the Lanky id when the fighter is Kirby or J Kirby, which gives the Kirby id directly. The port selects the Kirby id with `ftRemixLankyNSPStatusForFighter` and `ftRemixLankyNSPIsKirby`, which is equivalent.
5. **Cancel ammo handling** (asm lines 137-142). The asm stores `ammo - 1` and then, when the old ammo was 0, resets it to 0. The port returns early when ammo is 0 and otherwise decrements. Equivalent while ammo is never negative, which holds because the initial value is 3 and it only decrements from above 0.
6. **Grape kinetic check** (asm lines 143-149). The asm treats `ga == 0` as grounded. The port uses `ga == nMPKineticsAir` for aerial, as the Falco port does. Equivalent for the 0/1 values `ga` takes.
7. **Grape rotation** (asm lines 294-298). The asm adds the rotation speed to the DObj rotation word at 0x30 without any spin multiplier. The port writes `rotate.vec.f.x`, the same offset Conker uses.
8. **Grape damage argument** (asm line 340). The asm passes `WPStruct` 0x234 (`hit_normal_damage`) as the size argument of `efManagerDamageNormalLightMakeEffect`, with is_static = 0. The port does the same.
9. **Unused proc slot.** The desc word at +0x00 is "unknown" in the asm. The port sets `flags = 0`.
