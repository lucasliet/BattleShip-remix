# F8 Air Dodging (AirDodge.asm)

Setting: `Air Dodging`, `Toggles.asm` `entry_air_dodge` (line 2432). Values: `0` Off (default in CE, TE, NE, JP),
`1` Melee, `2` Ultimate, `3` Air Dash. Override for a run: `SSB64_REMIX_TOGGLES="Air Dodging=1"` (or `=2`, `=3`).
The asm reads the value directly (`entry_air_dodge + 4`), so the port uses `gmRemixToggle(air_dodge)`; the single
button mode check reads `entry_single_button_mode + 4` through `gmRemixToggle(single_button_mode)`.

Port: `decomp/src/ft/ftremix/ftremixairdodge.c` (new), hooks in `ftcommonjumpaerial.c`, `mpcommon.c`, `ftmanager.c`,
declarations appended to `ftremixhooks.h`. The new file is picked up by the `GLOB_RECURSE` in `CMakeLists.txt` at the
next configure.

## Patches

| Asm (AirDodge.asm) | RAM | Vanilla function (vanilla_funcs.txt) | Status | Notes |
|---|---|---|---|---|
| `airdodge_check` (line 109) | 0x801401AC | `ftCommonJumpAerialCheckInterruptCommon` (ftcommonjumpaerial.c) | applied | Hook before the hammer check, `#ifdef REGION_REMIX`. Returns TRUE when the dodge or air dash started or was blocked by the air dash flag. |
| `reset_air_dash_grounded_` (line 64) | 0x800DE420 | `mpCommonSetFighterLandingParams`+0xB8 (mpcommon.c) | applied | Reset after `ftRemixRunGroundedScript`, the same dispatch. |
| `reset_air_dash_initial_` (line 83) | 0x800D7DB4 | `ftManagerInitFighter`+0x3C4 (ftmanager.c) | applied | Reset before the `switch (fp->fkind)` that the asm `jr t4` dispatch (initial script table) corresponds to. |
| `initial_` (line 163) | none | ftremixairdodge.c `ftRemixAirDodgeStart` | applied | Air dash flag check, then type dispatch. |
| `air_dodge_initial_` (line 200) | none | ftremixairdodge.c `ftRemixAirDodgeStartAirDodge` | applied | Melee and Ultimate. |
| `air_dash_initial_` (line 287) | none | ftremixairdodge.c `ftRemixAirDodgeStartAirDash` | applied | Air Dash. |
| `apply_air_movement_` (line 344) | none | ftremixairdodge.c `ftRemixAirDodgeApplyAirMovement` | applied | Stick angle (`syUtilsArcTan2`, the 0x8001863C of the asm), speed capped at 80, 1.1875F scale. |
| `main_` (line 453) | none | ftremixairdodge.c `ftRemixAirDodgeMain` | applied | Special fall at `anim_frame <= 0`. |
| `air_dash_main_` (line 487) | none | ftremixairdodge.c `ftRemixAirDodgeAirDashMain` | applied | `mpCommonSetFighterWaitOrFall` (0x800DEE54). |
| `movement_` (line 520) | none | ftremixairdodge.c `ftRemixAirDodgeMovement` | applied | Drag 0.875F on x and y while temp variable 2 is set. |
| `collision_` (line 549) | none | ftremixairdodge.c `ftRemixAirDodgeCollision` | applied | `mpCommonProcFighterLanding`. |
| `begin_landing_` (line 562) | none | ftremixairdodge.c `ftRemixAirDodgeBeginLanding` | applied | `ftCommonLandingFallSpecialSetStatus`. |
| `moveset`, `neutral_moveset`, `moveset_air_dash` (lines 11-50) | none | data | applied | `static const u32` scripts in the new file, set on `motion_scripts[0][0]` and `[1][0]` (0x86C, 0x8AC). |

No `REMIX-TODO` markers: no AI, Size, GFX or announcer code in this setting.

## Deviations and uncertainties

1. Turnaround skipped (air_dodge_initial_ line 210, air_dash_initial_ line 297): the asm calls `0x800E8044`
   (`ftParamSetStickLR`, which takes the player struct) with the GObj in a0. On N64 it reads GObj+0x1C2 and writes
   GObj+0x44 (low word of `buffer_mask`) and never changes the player's `lr`. The port does not make the call, so the
   facing stays as it was. The GObj write is not reproduced.
2. Byte 0x18D mask (lines 207-209, 294-296): the asm mask is `0x07`, not `0x7F` as its comment says. It clears
   `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall`. The port clears those four fields.
3. The dodge return value (`airdodge_check` exit, line 151): on N64 `v0` is whatever the last call left in it. The port
   returns TRUE. All callers of `ftCommonJumpAerialCheckInterruptCommon` either ignore the value or treat TRUE as
   "interrupt handled".
4. `ftMainSetStatus` flags (Action.change, line 214): the macro does not set the fifth argument, so the N64 value is
   stack garbage. The port passes `FTSTATUS_PRESERVE_NONE`.
5. The `movement_` comment says 0.90625 but the value is 0x3F60 = 0.875F. The port uses 0.875F.
6. Initial script placement: the asm hook sits in the `jr t4` dispatch, which only runs when the fkind is below
   `NUM_CHARACTERS`. The port resets before the switch, so it always runs. All Remix ids are in that range.
7. Moveset pointers: `FTMotionScript` is 0x20 bytes on N64, so 0x86C is `motion_scripts[0][0].p_script` and 0x8AC is
   `motion_scripts[1][0].p_script`. The moveset dw/dh values are written as the words the parser reads (WAIT =
   `0x0400XXXX`, AFTER = `0x0800XXXX`, END = 0).
8. `air_dash_player_port_array` is also written by `Stamina.asm` line 1408 (`li t9, AirDodge.air_dash_player_port_array`,
   the reset on hit). That hook is not in this scope: it must call `ftRemixAirDodgeResetAirDash(fp)` in the Stamina
   worker's port. Without it the air dash is not refreshed by a hit.

## Verification (lead)

- Build and run are not done here. `scripts/remix/syntax-check.sh` reports `syntax-check OK` for all five touched files.
- Set `SSB64_REMIX_TOGGLES="Air Dodging=1"`, then in a match press shield or R in the air. Expect the DamageLow3 status
  with the directional dodge moveset, then a special fall on landing.
- `=2` (Ultimate): a neutral stick should show the neutral dodge that drifts with the fall. The jump counter is kept.
- `=3` (Air Dash): one dash per air time. A second press in the same air time does nothing until the fighter lands.
- `=0` (Off): the jump check must match vanilla.
- Expected to fail until the Stamina hook is in: the air dash does not refresh after a hit (deviation 8).
