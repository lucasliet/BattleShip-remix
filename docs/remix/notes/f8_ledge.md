# F8 Edge C-Jumping and Ledge Trumping (LedgeJump.asm, LedgeTrump.asm)

Scope: `.toolchain/pkgs/smashremix-src/src/LedgeJump.asm` (3 patches, 1 helper routine) and
`LedgeTrump.asm` (2 patches). Toggles read as `entry_ledge_jump + 0x4` and `Toggles.read(entry_ledge_trump)`, so the
port uses `gmRemixToggle(...)` (not `gmRemixToggleVS`). Both symbols exist in `port/remix/remix_symbols.h`.

## Toggles

- `Edge C-Jumping`: `Toggles.asm` line 2434, values `0` off (default), `1` on.
  Lead test: `SSB64_REMIX_TOGGLES="Edge C-Jumping=1"`.
- `Ledge Trumping`: `Toggles.asm` line 2439, values `0` off (default), `1` on.
  Lead test: `SSB64_REMIX_TOGGLES="Ledge Trumping=1"`.

What to look for in a short match:
- Edge C-Jumping: a fighter hanging from a ledge (CliffWait) taps a C button. It climbs (quick or slow by damage)
  and the climb turns into a stick jump at the end of the climb animation. With the toggle off, a C tap does nothing
  and the fighter falls after the wait.
- Ledge Trumping: two fighters, one hanging from a ledge (CliffCatch or CliffWait) and one jumping to the same ledge.
  With the toggle on, the holder is knocked into DamageAir3 (pushed away from its facing direction, 50.0 velocity,
  30 tics of hitstun) and the new fighter grabs the ledge. With the toggle off, the new fighter cannot grab it.

With each toggle off, behaviour must match vanilla exactly. The cliff proc updates now have a `default:` case under
`#ifdef REGION_REMIX` only, so the non-Remix build keeps the original switch.

## Patches

| # | Asm RAM | Vanilla function (decomp file) | Applied | Port |
|---|---------|--------------------------------|---------|------|
| LJ1 | 0x80144DDC (`extend_ledge_input_check`) | `ftCommonCliffWaitProcInterrupt` (`ft/ftcommon/ftcommoncliffcatchwait.c`) | yes | the fall check is skipped when `ftRemixLedgeJumpCheckCButtons` returns TRUE |
| LJ2 | 0x80144F24 (`extend_under_100_ledge_options`) | `ftCommonCliffQuickProcUpdate` (`ft/ftcommon/ftcommoncliffclimb.c`) | yes | `default:` case calls `ftRemixLedgeJumpCliffMotionDefault` |
| LJ3 | 0x80144FA8 (`extend_over_100_ledge_options`) | `ftCommonCliffSlowProcUpdate` (`ft/ftcommon/ftcommoncliffclimb.c`) | yes | `default:` case calls `ftRemixLedgeJumpCliffMotionDefault` |
| LT1 | 0x800DE66C (`ledge_check`) | `mpCommonRunFighterSpecialCollisions` (`mp/mpcommon.c`) | yes | inside the holder loop, `ftRemixLedgeTrumpSkipsHolder` routes to `goto next_gobj` |
| LT2 | 0x800DE688 (`track_ledge_grabs`) | `mpCommonRunFighterSpecialCollisions` (`mp/mpcommon.c`) | skipped | `REMIX-TODO(VsStats)`, VS ledge grab counter, no port (same as ZCancel) |

Helpers: `decomp/src/ft/ftremix/ftremixledgejump.c` (LJ1, LJ2/LJ3), `decomp/src/ft/ftremix/ftremixledgetrump.c` (LT1).
Prototypes appended to `decomp/src/ft/ftremix/ftremixhooks.h`.

Asm semantics that the port follows:
- LJ1: `check_input` reads `button_tap` (`lh 0x01BE`) masked with CL/CR/CU/CD (the C buttons). On a hit it calls
  `ftCommonCliffQuickOrSlowSetStatus(fighter, 3)`, the "made up value" that queues a jump through the cliff proc
  updates. The asm returns 1 and skips `ftCommonCliffWaitCheckFall`.
- LJ2/LJ3: the status 3 (Quick) and 3+3 = 6 (Slow) match no vanilla case (0..2, 3..5), so they reach the `b` at the end
  of the switch. The hook there is `jump_initial`: jump force 80.0F (`0x42A00000`) in `kneebend.jump_force`,
  `kneebend.is_shorthop = FALSE`, `kneebend.input_source = 1` (stick), then `ftCommonJumpSetStatus`.
- LT1: the asm pushes only when the holder's status is `CliffCatch` (0x54) or `CliffWait` (0x55). The holder's
  `physics.vel_air.y = 50.0F` (`0x42480000`), `vel_air.x = 50.0F * lr * -1` (asm: `cvt.s.w` of `lr`, then `mul.s` by
  -1.0F), `damage.hitstun_tics = 30`. The asm then falls into the loop continuation (`j 0x800DE678`), so both paths
  keep searching: with the toggle on, the holder never blocks the grab. The toggle-off path returns `is_ceilstop`.

## Uncertainties

- LT2 (not ported): the asm reads `v0` for the player index (`lbu v0, 0x0D(v0)`), but `v0` is the return of the
  preceding `mpCommonSetFighterLandingParams` call, not a struct pointer. The tracker is skipped, so this does not
  affect the port. When `VsStats` is ported, use `this_fp->player` and check the value of `v0` in the asm first.
- LT1: `ftRemixLedgeTrumpSkipsHolder` runs in the holder loop before the `return is_ceilstop`. With the toggle on the
  search goes on after a trump, as the asm does (the `_ledge_trump_end` path). The asm's `addiu s0, a0, 0` is
  overwritten by the save/restore, so it does not matter here.
- LJ1: the asm's `button_tap` check uses the newly tapped buttons only. A C button held from a previous climb does not
  trigger the jump, as in the asm.
- LJ2/LJ3: the port gates the jump with the toggle. The asm does not gate it, but status 3/6 is only queued by LJ1,
  which is already gated, so the behaviour is the same.

## Verification

- `scripts/remix/syntax-check.sh` OK on `ftremixledgejump.c`, `ftremixledgetrump.c`, `ftcommoncliffcatchwait.c`,
  `ftcommoncliffclimb.c`, `mpcommon.c`. The two `-Wempty-body` warnings in `mpcommon.c` are on vanilla lines 124 and
  141, not touched.
- Not built and not run (per brief).
