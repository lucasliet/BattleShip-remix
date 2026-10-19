# F8 worker notes: Z-Cancel, Punish Failed Z-Cancel, Flash On Z-Cancel

Scope: `.toolchain/pkgs/smashremix-src/src/ZCancel.asm` (plus the Practice.asm flash hook, which lives in the same vanilla
function). Port file: `decomp/src/ft/ftremix/ftremixzcancel.c`, hook in `decomp/src/ft/ftcommon/ftcommonattackair.c`,
helper declared by appending a block to `decomp/src/ft/ftremix/ftremixhooks.h`.

## Toggles

| Title (SSB64_REMIX_TOGGLES key) | Entry | Values | Read with |
|---|---|---|---|
| `Z-Cancel` | `entry_z_cancel_opts` | 0 DEFAULT, 1 DISABLED, 2 MELEE (7 frames), 3 AUTO, 4 GLIDE MODE | `gmRemixToggle` (asm reads entry+4, no guard) |
| `Punish Failed Z-Cancel` | `entry_punish_on_failed_z_cancel` | 0 off, 1 7% Damage, 2 Lava Floor, 3 Shield-Break, 4 Instant K.O., 5 Force Taunt, 6 Bury, 7 Laugh Track, 8 Egg, 9 Sleep, 10 Trip, 11 Random | `gmRemixToggle` |
| `Flash On Z-Cancel` | `entry_flash_on_z_cancel` | 0 off, 1 on | `gmRemixToggle` (Practice.asm uses `Toggles.guard`) |

Test example: `SSB64_REMIX_TOGGLES="Z-Cancel=4"`, `"Punish Failed Z-Cancel=3"`, `"Flash On Z-Cancel=1"`.

## Patches (vanilla function `ftCommonAttackAirProcMap`, 0x80150A08, ftcommonattackair.c)

| Asm address | Remix source | Status | What the port does |
|---|---|---|---|
| 0x80150A40 (`_z_cancel_opts`, `j` + nop) | ZCancel.asm 234-325 | applied | `ftRemixZCancelResult` + `ftRemixZCancelAttackAirLanding`: DEFAULT/MELEE window `tics_since_last_z < 11 / 7`; DISABLED always fails; AUTO always succeeds; GLIDE miss returns without landing. Replaces the whole landing branch under `#ifdef REGION_REMIX` (vanilla kept in `#else`). |
| 0x80150A48 (`_cruel_z_cancel`, `j` in the fail path) | ZCancel.asm 18-230 | applied | `ftRemixZCancelPunish` + `ftRemixZCancelPunishMode`: all 11 modes, RANDOM rolls 1..10 (`syUtilsRandIntRange(10) + 1`). Skip-landing modes return before the landing set. |
| 0x80150AE8 (Practice.asm `flash_on_z_cancel_`, `jal` to ftCommonLandingSetStatus) | Practice.asm 197-240 | applied (Practice scope overlap) | `ftRemixZCancelFlashOnLanding`: after the landing status is set, flashes (`ftParamCheckSetFighterColAnimID(gobj, 8, 0)`) when `tics_since_last_z < 11`. |
| `ZCancel.setup_` (called from Render.asm 2254/2315/2422/2444, SinglePlayerModes.asm 2389) | ZCancel.asm 329-349 | skipped | Loads the Yoshi egg file 0x153 for EGG / RANDOM. |
| `_cpu_z_cancel` (`at` set by `AI.z_cancel_`) | ZCancel.asm 244-273 | skipped, REMIX-TODO(AI) | Player branch only. |

Port enhancements kept: `port_enhancement_IsAutoZCancelEnabled()` turns every miss into a normal landing (as before);
`port_enhancement_IsFailedZCancelFlashEnabled()` still flashes red on a failed landing. Both files
(`port/enhancements/AutoZCancel.cpp`, `FailedZCancelFlash.cpp`) are unchanged.

## REMIX-TODO markers in the port

- `REMIX-TODO(VsStats)`: success and miss counters (`VsStats.z_cancel_success_tracker` / `miss_tracker`), no port.
- `REMIX-TODO(Tripping)`: the trip punishment sets the hitstun and the DamageLow3 action, and plays 0x456, but does not set
  `sFTRemixTrippingFlag[player]` (static in `ftremixtripping.c`). Without it the trip does not recover; a setter is needed.
- `REMIX-TODO(Damage)`: BURY (`Damage.bury_initial_`) is skipped, the landing is skipped too.
- `REMIX-TODO(ZCancel.setup_)`: Yoshi egg file not loaded, so EGG may show no egg model.
- `REMIX-TODO(HowToPlay)`: no how-to-play screen check, so punishments also apply on that screen.
- `REMIX-TODO(1P final boss)`: INSTANT_KO always kills; Remix skips it in the 1P final stage while the match is off.
- `REMIX-TODO(AI)`: CPU Z-cancel branch.

## Approximations

- INSTANT_KO clears the overlay with `p_loop_sfx = NULL`, `loop_sfx_id = 0`, `ftParamResetColAnim(&colanim)`. Remix clears
  0x0A20, 0x0A24, 0x0A28, 0x0A30, 0x0A88 by address; `ftParamResetColAnim` may clear a little more.
- TRIP writes `status_vars.common.damage.hitstun_tics = 8` (full s32 at 0x0B18); Remix writes only the halfword at 0x0B1A.
- EGG zeroes `status_vars.common.dead.wait` (word 0x0B18) and `unk_ft_0x7A0` (0x07A0) to match the asm's two stores.
  The 0x0B18 name is a stand-in; the field is not the egg's capturer.
- Surface flags (ON 0x12, LAVA 0x1E) write the low halfword of `coll_data.floor_flags` (0x0F6).
- Hidden egg fighter sets `is_invisible` (0x018D bit 7), the byte Remix ORs with 0x01 in the 0x018C halfword.
- RANDOM uses the port RNG (`syUtilsRandIntRange`), not Remix's `Global.get_random_int_`. The rolls are the same range
  (1..10) but the sequence differs.
- Flash: `FTStruct.tics_since_last_z` is documented as resetting to 65536 on action state change, and the flash reads it
  after `ftCommonLandingSetStatus`, as the asm does. If that reset happens on the landing status, the flash never fires.
  Check in a game run (`Flash On Z-Cancel=1`, Z-cancel a landing inside the window).

## Lead: what to check

- `scripts/remix/syntax-check.sh` passes for `ftremixzcancel.c`, `ftcommonattackair.c`, `ftremixhooks.h`. Not built or run.
- The new `.c` file needs a CMake reconfigure to be globbed (`decomp/src/ft/*.c`).
- Run: Z-Cancel=3 (auto success), Z-Cancel=4 (glide: no landing on a miss), Punish=3 (shield-break on a late landing),
  Punish=9 (sleep), Punish=4 (KO outside the 1P final stage), Punish=8 (egg, see the setup_ TODO).
- With all three toggles at 0 the vanilla path is unchanged except for the `ftRemixZCancelAttackAirLanding` call, which
  returns the same landing for flag1 == 0, success and failed-without-punishment (`Punish=0`).
