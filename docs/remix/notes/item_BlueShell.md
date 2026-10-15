# Item 0x30 BlueShell: port notes

Source: `.toolchain/pkgs/smashremix-src/src/items/BlueShell.asm` (1570 lines).
Output: `decomp/src/it/itremix/itremixblueshell.c`, `itremixblueshell.h`.
Descriptor: `const ITRemixItemDesc dITRemixBlueShellItemDesc`, id 0x30, `proc_make` = `itRemixBlueShellMakeItem`, `SHOW_GFX_WHEN_SPAWNED` TRUE.
Verification: `scripts/remix/syntax-check.sh` prints `syntax-check OK`, no warnings.

## Item file and offset

- `ITDesc.p_file = &gITManagerCommonData` (file 0xFB, asm `dw 0x8018D040`).
- `o_attributes = 0xE30` (asm `FILE_OFFSET`).
- Spawn uses the red shell spawn tail (`OS.copy_segment 0xF5C4C 0xD8`, decoded with `tools/remix_disasm_rom.py`), mapped to `itRemixBlueShellSetupDObj` and `itRemixBlueShellSetupState`.

## Spawn signature

`GObj* itRemixBlueShellMakeItem(GObj *parent_gobj, Vec3f *pos, Vec3f *vel, u32 flags)`

## Routines: ported versus asm (27 scopes)

Ported (20):
- spawn_custom_item_based_on_red_shell_ -> itRemixBlueShellMakeItem (+ SetupDObj, SetupAttack, SetupState)
- prepickup_main_ -> itRemixBlueShellHoldProc
- resting_main_ -> itRemixBlueShellRestingProcMap
- ground_to_air_ -> itRemixBlueShellGroundToAir
- grounded_check_collision_ -> itRemixBlueShellGroundedCheckProcMap
- idle_ground_collision_ -> itRemixBlueShellIdleGroundProcMap
- idle_grounded_initial_ + idle_grounded_initial_2_ -> itRemixBlueShellIdleGroundedInitial (wrapper merged)
- throw_initial_ -> itRemixBlueShellThrowSetStatus
- air_to_ground_check_ -> itRemixBlueShellAirToGroundProcMap
- aerial_main_ -> itRemixBlueShellAerialProcUpdate
- collide_with_player_ -> itRemixBlueShellCollideWithPlayer (destroy path only, see below)
- collide_with_shield_ -> itRemixBlueShellCollideWithShield
- set_grounded / set_aerial -> itRemixBlueShellSetGrounded / SetAerial
- grounded_active_ -> itRemixBlueShellGroundedProcUpdate
- grounded_gfx_ -> itRemixBlueShellGroundedGfx
- grounded_collide_with_hurtbox_ -> itRemixBlueShellGroundedHurtboxCollide
- idle_hurtbox_collision_ -> itRemixBlueShellIdleHurtboxCollide
- hit_gfx_ -> itRemixBlueShellHitGfx

Not ported (7):
- ground_to_air_2: unused in the asm.
- aerial_stage_clipping_check_: unused in the asm.
- set_idle_aerial_ and set_idle_aerial_2: only reachable from the bounce path of collide_with_player_, which is dead (the HP check is commented out, so the destroy block always runs).
- custom_smoke_: partly replaced (see REMIX-TODO(gfx)).
- target_player_ and get_player_in_first_: see REMIX-TODO(player-in-first).

## Vanilla patches

None. BlueShell.asm has no `OS.patch_start`.
Vanilla references used as-is: `DROP_ITEM 0x801745FC` = `itTomatoDroppedSetStatus` (ittomato.c:147), the red shell functions `itRShellCommonProcShield` (0x8017B2F8), `itRShellCommonProcReflector` (0x8017B31C), `itRShellSpinInitVars`, `itRShellSpinAirInitVars`, `itRShellSpinCheckCollisionEdge`, `itRShellCommonClearAnim`, and `itMainCommonProcHop` (0x801733E4).

## REMIX-TODO

- REMIX-TODO(player-in-first): target_player_ (homing toward the player in first place, run every frame in grounded_active_) and get_player_in_first_ are not ported. The asm stores the player pointer at ITStruct 0x180, which has no named field. The Blue Shell therefore does not home and keeps its current velocity. The player list (asm global 0x800466FC) and Global.match_info (0x800A50E8) have no decomp name yet.
- REMIX-TODO(gfx): grounded_gfx_ calls the Remix custom_smoke_ (pink smoke, particle 0x8A). The port calls `efManagerDustLightMakeEffect(pos, lr, 1.0F)` instead. The timer cadence (8 frames) is kept.

## Uncertainties (UNCERTAIN in code)

- ITStruct offsets: the asm N64 offsets were mapped by meaning, not by the table. The table is offset by +4 from the asm for fields after `attack_coll` and `damage_coll`. Mapping used: item_vars base 0x350 (asm 0x350 = `damage_all_delay`, the throw timer; 0x351 = `dust_effect_int`; 0x352 = `health`; 0x353 = `is_damage`; 0x354 = `is_setup_vars`; 0x355 = `interact`; 0x358 = `shell.vel_x`); damage_queue 0x298; damage_lr 0x2A4; damage_gobj 0x2A8; `lifetime` 0x2C0 (death timer); `is_unused_item_bool` 0x2D3 bit 0x04; `times_thrown` 0x2CE bits 0x0E; `is_allow_pickup` 0x2CE bit 0x80.
- The asm spawn stores the float at 0x8019CDAC into DObj+0x34. The port takes this as `rotate.vec.f.y` = 90 degrees (red shell convention). The hold proc writes 0.0 to the same field.
- Constants at 0x8019CDA4 and 0x8019CDA8 are not read from the ROM. The port uses 0.7F (ITRSHELL_RECOIL_MUL_X), taken from the red shell.
- `collide_with_shield_` stores `v0` into health (0x352), and v0 is not set on that path. The port stores 0.
- `hit_gfx_` stores the leftover `t1` into the gfx timer (0x351). The port stores 8.
- `set_idle_aerial_` multiplies by `f0`, which is never set. Not ported (dead code).
- `grounded_gfx_` and `hit_gfx_` pass a2 = 2^32 to custom_smoke_. Not applicable with the port's dust call.

## Sweep 2026-10-14 (SSB64_REMIX_FORCE_ITEM, fast spawn 60t, 1500 frames, none.txt)

PASS — rc=0, 5 screenshots, no crash marks in ssb64.log. Spawn verified visually for the aggregate run; pickup/effect still untested without input script.

## Final re-sweep 2026-10-14 (post hook wave)

PASS — rc=0, 5 screenshots, 0 crash marks, WITH all vanilla hooks active (stopwatch speed hook in gcRunGObjProcess, basketball bounce/hitlag hooks in itmap/itprocess/ftmain, franklin immunity hooks, unified mushrooms, goldengun shoot path, cloaking immunity+scene-end clear).
