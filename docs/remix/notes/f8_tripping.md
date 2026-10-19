# F8 Tripping: port notes (ROADMAP T4.2)

Source: `.toolchain/pkgs/smashremix-src/src/Tripping.asm` (goombapatrol).
Port file: `decomp/src/ft/ftremix/ftremixtripping.c` (new, owned by this scope).
Helpers declared: `decomp/src/ft/ftremix/ftremixhooks.h`, block "Tripping" appended at the end.

## Toggle

- Title: `Tripping` (`Toggles.asm` line 2429, `entry_tripping`, INT entry, string table `string_table_tripping`).
- Values: `0` OFF, `1` LOW, `2` HIGH, `3` BRAWL. Read with `gmRemixToggle(tripping)` (the asm reads
  `Toggles.entry_tripping` directly, not through `single_player_guard`, so the VS variant is not used).
- Also read: `gmRemixToggle(punish_on_failed_z_cancel)`, the Z-cancel punishment, TRIP = 10 and RANDOM = 11 (`ZCancel.asm`).
- Lead test: `SSB64_REMIX_TOGGLES="Tripping=3"` (BRAWL, rarest) or `Tripping=2` (HIGH, easiest to see in a short run).
- What to look for: a dash dance (dash, then turn back) or a turn run, with the fighter's own input held, can
  trip. The fighter enters the DamageLw3 status with the trip sound (FGM 0x0456) and 8 frames of hitstun. At the end
  of the animation it changes to DownBounceD on the ground or Tumble in the air. A Z-cancel failure with punishment TRIP
  or RANDOM trips even with Tripping OFF.
- Odds per roll (asm comment): LOW turn run 1.2%, dash 1.0%, dash dance 1.5%; HIGH divides the range by 4, BRAWL by 16.

## Vanilla patches

| Asm | RAM | Vanilla function (`vanilla_funcs.txt`) | Applied | Port |
|-----|-----|----------------------------------------|---------|------|
| `tripping:` turn (`OS.patch_start(0xB9358)`) | 0x8013E918 | `ftCommonTurnSetStatus` (`ftcommonturn.c`) | yes | `ftRemixTrippingEnterStatus(fp, nFTCommonStatusTurn)` in the `ftMainSetStatus` argument |
| `tripping:` dash (`0xB9758`) | 0x8013ED18 | `ftCommonDashSetStatus` (`ftcommondash.c`) | yes | same helper with `nFTCommonStatusDash` |
| `tripping:` turnrun (`0xB9C58`) | 0x8013F218 | `ftCommonTurnRunSetStatus` (`ftcommonturnrun.c`) | yes | same helper with `nFTCommonStatusTurnRun` |
| `trip_turn_hitstun_retention_1` (`0xB9330`) | 0x8013E8F0 | `ftCommonTurnProcInterrupt` (no `// 0x` comment, sits between ProcUpdate and SetStatus) | yes | `ftRemixTrippingAllowsTurnStateReset(fp)` guards the `is_allow_turn_direction = FALSE` store |
| `trip_turn_hitstun_retention_2` (`0xB9388`) | 0x8013E948 | `ftCommonTurnSetStatus` | yes | same guard, after `ftMainPlayAnimEventsAll` |
| `damaged_check_trip` (`0xBAF28`) | 0x801404E8 | `ftCommonDamageCommonProcUpdate` (`ftcommondamage.c:110`) | yes | `ftRemixTrippingFinishDamage(fighter_gobj)`, early `return` where the asm jumps to 0x8014052C |

No patch skipped. No `REMIX-TODO` added: every subsystem the asm calls is ported (`syAudioPlayFGM`,
`syUtilsRandIntRange`, `ftMainSetStatus`, `ftMainPlayAnimEventsAll`).

Each vanilla site is guarded with `#ifdef REGION_REMIX`. The non-REMIX branch is the original line. With the toggle
OFF, every helper returns the vanilla value (`status_id` unchanged, reset TRUE), so vanilla behaviour is kept.

## Port decisions

- Trip flag: `static u8 sFTRemixTrippingFlag[4]`, indexed by `fp->player` (asm `player_trip_flag`).
- Hitstun: the asm writes a halfword at `0x0B1A` (the low 16 bits of `damage.hitstun_tics`, big-endian). The port
  assigns the whole `status_vars.common.damage.hitstun_tics = 8`. Equal when the upper half is zero, which holds for the
  turn and dash states the trip runs from. See uncertainties.
- Turn retention: the asm store `sw r0, 0x0B18` is the `is_allow_turn_direction` field, and it is also the start of
  the damage union (`hitstun_tics`). The port keeps the same field write, so the aliasing is the same.
- Recovery status: `fp->ga == nMPKineticsGround` gives DownBounceD (asm `beqzl` on kinetic 0), otherwise Tumble.
- Tumble is `0x039` (`Action.asm`), not in the vanilla status enum; defined as `FTREMIX_TRIPPING_STATUS_TUMBLE`.

## Uncertainties (asm lines)

1. `Tripping.asm` L79-80 `sh a1, 0x0B1A(v0)`: halfword write. The port writes the full s32 (see Port decisions).
2. `Tripping.asm` L196 `lw v0, 0x001C(a2)`: asm comment says "current frame of current action". The struct table names
   0x001C `status_total_tics`; the port reads `fp->status_total_tics`. If the lead sees the recovery fire too early or
   too late, check this field first.
3. `Tripping.asm` L71 `Global.get_random_int_` (0x80018994): the port uses `syUtilsRandIntRange(range)`, which returns
   (0, range-1) by the same `RandUShort * range / 65536` form. Not confirmed by reading 0x80018994 directly
   (not in `vanilla_funcs.txt`).
4. `Tripping.asm` L227 `jal 0x800E6F24` (ftMainSetStatus) was called with a2 = 0, a3 = 1.0F, flags 0. The port uses
   `FTSTATUS_PRESERVE_NONE`, which is 0 (`ftdef.h`).
5. Toggle defaults: only the entry line (`Toggles.asm` 2429) was read. The per-profile defaults were not checked.

## Verification

- `scripts/remix/syntax-check.sh` on: `ftremixtripping.c`, `ftremixhooks.h`, `ftcommonturn.c`, `ftcommondash.c`,
  `ftcommonturnrun.c`, `ftcommondamage.c`. All print `syntax-check OK`. The one warning in `ftcommondamage.c`
  (line 675, `&&` inside `||`) is in existing code, not in this patch.
- Not built and not run, as the brief requires.
