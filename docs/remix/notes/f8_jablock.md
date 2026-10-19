# F8 Jab Locking (JabLock.asm)

Scope: `.toolchain/pkgs/smashremix-src/src/JabLock.asm` (91 lines, 3 patches). Toggle `Toggles.entry_jab_lock`,
read directly as `entry_jab_lock + 0x4`, so the port uses `gmRemixToggle(jab_lock)` (not `gmRemixToggleVS`).
Symbol `REMIX_SYM_Toggles__entry_jab_lock` exists in `port/remix/remix_symbols.h`.

## Toggle

- Title in `Toggles.asm`: `Jab Locking` (line 2433). Values: `0` off (default), `1` on.
- Lead test: `SSB64_REMIX_TOGGLES="Jab Locking=1"`.
- What to look for in a short match: a fighter knocked down bounces (DownBounceD/U). Hit it with a weak
  (pre-knockdown) hit during the bounce: with the toggle on it restarts the bounce, not a damage status. After the
  bounce, the next hit that lands while the lock is set ends the bounce in DownStandD/U.
  With the toggle off, behaviour must match vanilla exactly.

## Patches

| # | Asm RAM | Vanilla function (decomp file) | Applied | Port |
|---|---------|--------------------------------|---------|------|
| 1 | 0x80144504 (`down_initial_`) | `ftCommonDownBounceSetStatus` (`ft/ftcommon/ftcommondownwaitbounce.c`) | yes | `ftRemixJabLockClear(fp)` after `attack_buffer = 0` |
| 2 | 0x8014139C (`handle_lock_`) | `ftCommonDamageInitDamageVars` (`ft/ftcommon/ftcommondamage.c`) | yes | `ftRemixJabLockDamageStatus(this_fp, &status_id_set)` after the Lanky hook, before `ftMainSetStatus` |
| 3 | 0x8014494C (`handle_reset_`) | `ftCommonDownAttackCheckInterruptDownBounce` (`ft/ftcommon/ftcommondownattack.c`) | yes | `ftRemixJabLockInterruptDownStand(fighter_gobj)` before the attack-buffer check |

Helpers: `decomp/src/ft/ftremix/ftremixjablock.c`, prototypes appended to `decomp/src/ft/ftremix/ftremixhooks.h`.
Lock bool: N64 FTStruct offset 0x0B28, accessed with `ftRemixFighterWord`. It sits inside the `status_vars` union and
has no named field in `ftcommon.h`.

## Semantics

- Patch 1 (asm lines 13-23): when the toggle is on, the lock is cleared on every DownBounce start.
- Patch 2 (asm lines 35-56): only for fp->status_id DownBounceD (0x43) or DownBounceU (0x44). Only when the damage
  status is below `Action.DamageElec2` (0x32, = `nFTCommonStatusDamageE2`), i.e. pre-knockdown. The damage status
  becomes the current action and the lock is set.
- Patch 3 (asm lines 66-85): when the lock is set, call `ftCommonDownStandSetStatus` (asm `jal 0x80144580`) and return
  TRUE. The lock check comes before the attack-buffer check, as in the asm.
- The asm's `_return` jumps re-run the displaced original instruction in the delay slot, so the port has no
  equivalent of it.

## Uncertainties

- Patch 2 position: the hook at 0x8014139C is the argument setup for `ftMainSetStatus` (`addiu a2, r0, 0` is 0.0F).
  The port puts it after the Lanky hook, the closest source position. Asm ordering relative to Lanky was not checked.
- The lock bool is never cleared outside patch 1. This matches the asm, which has no other writer.

## Verification

- `scripts/remix/syntax-check.sh` OK on `ftremixjablock.c`, `ftcommondownwaitbounce.c`, `ftcommondownattack.c` and
  `ftcommondamage.c`. Warnings in the last two are on vanilla lines that were not touched.
- Not built and not run (per brief).
