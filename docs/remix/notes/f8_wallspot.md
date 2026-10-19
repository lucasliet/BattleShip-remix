# F8 Wall Teching and Spot Dodging (WallTeching.asm, SpotDodge.asm)

Settings: `Wall Teching` (`Toggles.asm` `entry_wall_teching`, line 2440) and `Spot Dodging` (`entry_spot_dodge`, line 2437).
Both are `entry_bool`, default `OS.FALSE` in CE, TE, NE and JP. Values: `0` Off, `1` On.
Run overrides: `SSB64_REMIX_TOGGLES="Wall Teching=1"` and `SSB64_REMIX_TOGGLES="Spot Dodging=1"`.
Both asm files read the entry with `Toggles.read` (entry + 4), so the port uses `gmRemixToggle` (no single player guard).

Files: `decomp/src/ft/ftremix/ftremixwalltech.c` and `decomp/src/ft/ftremix/ftremixspotdodge.c` (new),
hooks in `decomp/src/ft/ftcommon/ftcommonwalldamage.c` and `decomp/src/ft/ftcommon/ftcommonpass.c`,
declarations appended to `decomp/src/ft/ftremix/ftremixhooks.h`.

RAM addresses: the `OS.patch_start` pairs of both files give RAM = ROM + 0x80085FC0 in this area (checked against
the three pairs 0xBC644/0x80141C04, 0xBC9DC/0x80141F9C and 0xBF0B0/0x80144670). `tools/remix_disasm_rom.py` labels
differ by 0x20 unless `--vram` is passed explicitly.

## Patches

| Asm | RAM | Vanilla function (vanilla_funcs.txt) | Status | Notes |
|---|---|---|---|---|
| `WallTeching.patch` (line 7) | 0x80141C04 | `ftCommonWallDamageSetStatus` (ftcommonwalldamage.c) | applied | The `ftMainSetStatus(nFTCommonStatusWallDamage, ...)` call becomes `ftRemixWallTechTryStart`; on FALSE it runs the vanilla call. The success path keeps the vanilla tail (knockback store, rumble, intangible timer, `is_hitstun`), as the asm returns to 0x80141C28. |
| `WallTeching.track_successful_tech` (line 65) | 0x80144670 | `ftCommonPassiveSetStatus` (ftcommonpassive.c) | skipped, `REMIX-TODO(VsStats)` | Counter only (`tech_success_tracker`). No hook added. |
| `WallTeching.track_successful_roll` (line 87) | 0x80144714 | `ftCommonPassiveStandSetStatus` (ftcommonpassivestand.c) | skipped, `REMIX-TODO(VsStats)` | Counter only (`tech_success_tracker`). No hook added. |
| `SpotDodge.check_input` (line 8) | 0x80141F9C | `ftCommonGuardPassCheckInterruptGuard` (ftcommonpass.c) | applied | Before the pass check: `ftRemixSpotDodgeCheckInterrupt` (spot dodge, returns TRUE). The pass input is `ftRemixSpotDodgePassInput(fp, vanilla)`, which is the vanilla result when off and the platform drop when on. The Z check is the vanilla one. |

## Behaviour ported

Wall Teching (`ftRemixWallTechTryStart`):
- Off: returns FALSE, vanilla wall damage.
- On, `fp->tics_since_last_z >= 20` (`FTCOMMON_PASSIVE_BUFFER_TICS_MAX`): miss, returns FALSE, vanilla wall damage (asm `_fail`).
- On, within 20 tics: `vel_damage_air.x` and `.y` zeroed (asm 0x54/0x58), `ftMainSetStatus(nFTCommonStatusPassive, 0.0F, 2.0F, FTSTATUS_PRESERVE_NONE)`
  (Action.Tech = 0x51 = Passive in `ftcommonstatus.h`), then `proc_update = ftAnimEndSetFall`, `proc_physics = NULL`,
  `proc_map = mpCommonProcFighterCliffFloorCeil`, `proc_interrupt = NULL`.
- The `hitstun_tics` store of the asm (0xB18) is line 60 of the vanilla function, which already runs before the hook.

Spot Dodging (`ftRemixSpotDodgeCheckInterrupt`, `ftRemixSpotDodgePassInput`):
- Slam: `stick_range.y < -32` and `tap_stick_y < 4` (`FTREMIX_SPOTDODGE_SLAM_*`).
- Soft platform: `coll_data.floor_flags & 0x4000` (`MAP_VERTEX_COLL_PASS`).
- Spot dodge: slam, and (not on a soft platform, or `stick_range.y < -52`), and Z held (`button_hold & button_mask_z`).
  Result: `ftMainSetStatus(nFTCommonStatusDamageHi2, 0.0F, 1.0F, FTSTATUS_PRESERVE_NONE)` (Action.DamageHigh2 = 0x26),
  moveset on `motion_scripts[0][0]` and `[1][0]` (0x86C, 0x8AC), `proc_interrupt = NULL`, `proc_map = mpCommonSetFighterFallOnEdgeBreak`.
- Platform drop: slam, soft platform, `stick_range.y >= -52`. The pass input is TRUE and the vanilla Z check and guard pass follow.
- Anything else: pass input FALSE. When the setting is on, the vanilla `ftCommonPassCheckInputSuccess` is not used.

## Deviations and uncertainties

1. Spot dodge moveset encoding: `Moveset.WAIT(2)` is `0x04000002` (`dh 0x0400`, `dh 2`), as in the AirDodge port.
2. The asm's spot dodge `Action.change` passes `a0 = fp->fighter_gobj` (0x04). The port uses the `fighter_gobj` argument, which is the same GObj.
3. `0x800DDF44` is named `mpCommonSetFighterFallOnEdgeBreak` in `vanilla_funcs.txt`. The asm comment says "same collision routine as dash attack"; the port uses the decomp name at that address.
4. With the setting on, a slam on a soft platform between -32 and -52 is a platform drop, while vanilla requires `stick_range.y <= -53`. This follows the asm.
5. `VsStats` tech counters (the two `track_*` hooks, and the success and miss increments of the wall tech) are not ported. `REMIX-TODO(VsStats)` marks the two places in `ftremixwalltech.c`. These only affect the VS stats screen.

## Verification (lead)

- Build and run were not done here. `scripts/remix/syntax-check.sh` reports `syntax-check OK` for `ftremixwalltech.c`, `ftremixspotdodge.c`, `ftcommonpass.c` and `ftcommonwalldamage.c` (the last two include `ftremixhooks.h`).
- Wall Teching: `SSB64_REMIX_TOGGLES="Wall Teching=1"`. Hit a wall, and press Z within 20 frames before the wall contact: expect the Passive status (a tech) with no wall bounce status. Without Z: the normal wall damage bounce.
- Spot Dodging: `SSB64_REMIX_TOGGLES="Spot Dodging=1"`. Hold Z (shield), then slam the stick down quickly off a platform: expect DamageHigh2 with the intangible spot dodge moveset. Slamming on a soft platform above -52 should drop through the platform as before. With the setting off, behaviour is vanilla.
