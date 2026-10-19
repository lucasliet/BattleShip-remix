# F8 Rage: port notes (ROADMAP T4.2)

Source: `.toolchain/pkgs/smashremix-src/src/Rage.asm` (scope Rage).
Port file: `decomp/src/ft/ftremix/ftremixrage.c` (new, owned by this scope).
Helpers declared: `decomp/src/ft/ftremix/ftremixhooks.h`, block "Rage setting" appended before the final `#endif`.

## Toggle

- Title: `Rage` (`Toggles.asm` line 2430, `entry_rage`, INT entry, string table `string_table_rage`).
- Values: `0` OFF, `1` ULTIMATE, `2` SMASH 4, `3` BERSERK, `4` FATIGUE. The asm applies the modes by value:
  1 = Ultimate (0.1 scale), 2 = Smash 4 (0.15), 3 = Manic (+0.5), 4 = Depression (-0.5). The menu titles BERSERK and
  FATIGUE are the names of values 3 and 4 in the asm, and the multipliers follow the asm, not the titles.
- Read with `gmRemixToggle(rage)` (the asm reads `Toggles.entry_rage` directly through `Toggles.read`, not through
  `single_player_guard`, so the VS variant is not used).
- Lead test: `SSB64_REMIX_TOGGLES="Rage=1"` (ULTIMATE). A short match with a fighter at 100% or more is enough.
- What to look for:
  - Smoke puffs from a fighter at 100% or more. The rate rises at 125% and again at 150%.
  - Knockback of a hit taken from an attacker with high damage grows with the attacker's percent. A hit from a fighter
    at 0% should match vanilla. The Ultimate check (below) means a move with knockback weight set gets no bonus under
    Ultimate.

## Vanilla patches

| Asm | RAM (asm) | Vanilla function (`vanilla_funcs.txt`) | Applied | Port |
|-----|-----------|----------------------------------------|---------|------|
| `direct_attack` (`OS.patch_start(0x5F770)`) | 0x800E3F70 | `ftMainProcessHitCollisionStatsMain` (`ftmain.c`, fighter hitlog) | yes | `ftRemixRageSaveAttackerPercent(attacker_fp)` before the knockback call |
| `projectile_attack` (`0x5F8FC`) | 0x800E40FC | `ftMainProcessHitCollisionStatsMain` (weapon hitlog) | yes | `ftRemixRageSaveProjectileOwnerPercent(wp)` after `wpMainGetStaledDamage` |
| `throw` (`0xC5AA0`) | 0x8014B060 | `ftCommonThrownReleaseThrownUpdateStats` (`ftcommonthrown2.c`) | yes | `ftRemixRageSaveAttackerPercent(capture_fp)` before the knockback calc (the thrower is the attacker) |
| `knockback_multiply` (`0x65760`) | 0x800E9F60 | `ftParamGetCommonKnockback` (`ftparam.c`) | yes | `ftRemixRageKnockbackScale(knockback, knockback_weight)` before the 2500 clamp |
| `gen_gfx` (`0x5DBD0`) | 0x800E23D0 | `ftMainProcPhysicsMap` (`ftmain.c`) | yes | `ftRemixRageSmokeEffect(fp)` at the top of the `proc_map` block, before the mask stores |

No patch skipped. No `REMIX-TODO`: every subsystem the asm calls is ported (`ftParamMakeEffect` 0x800EABDC,
`Global.frame_counter`, the knockback formula itself).

Each vanilla site is guarded with `#ifdef REGION_REMIX`. With the toggle OFF, every helper returns before touching
state, so vanilla behaviour is kept.

## Port decisions

- `attacker_damage_percent` (asm `db` + word stores) is one `static s32 sFTRemixRageAttackerPercent`. Single global, as
  in the asm, not per player.
- Damage clamp `[35, 150]`, scale `(damage - 35) / 115.0F`, multiplier added to 1.0F, the knockback multiplied by it.
  The asm float constants are exact in float (0.10009765625, 0.150390625, 0.5, -0.5), so they are written in decimal.
- The reset of `sFTRemixRageAttackerPercent` to 0 happens only on the path that applies the multiplier, as in the asm
  (the Ultimate skip and the OFF path jump past the store).
- Smoke: effect id 7 (`or a2, r0, r0` for joint 0, NULL position and scatter, `lr` from `fp->lr`, FALSE FALSE).
- Frame counter: `Global.frame_counter` at 0x8003CB30 is the same address as `dSYAudioCurrentTic` (`sys/audio.c`),
  declared `extern` in the port file.
- Player owner test for a projectile: `owner_gobj->id == 0x3E8`, the same check as `itremixbasketball.c`.

## Uncertainties (asm lines)

1. `Rage.asm` 126-134 (fixed-knockback check): the code branches to the end when `a3 != 0` under Ultimate. So Ultimate
   applies only when `knockback_weight == 0`, which reads opposite to the comment "Skip if Ultimate". The port keeps the
   asm behaviour. Check against a real Ultimate hit before changing it.
2. `Rage.asm` 58-63 (projectile_attack): the comment says "s2 = item object", but the hook sits in the weapon hitlog
   branch (`wpMainGetStaledDamage`, `s4` is the WPStruct). The port reads `WPStruct.owner_gobj`.
3. `Rage.asm` 271 (gen_gfx): the asm comment gives `Ef_Kind_DustExpandLarge, 4`, but the code loads `a1 = 7` and
   `a2 = 0`. The port follows the code (effect 7, joint 0).
4. `Rage.asm` gen_gfx (lines 227-260): the asm stores `mask_unk` before `mask_prev`. The port calls the smoke before both stores. The
   effect does not read the mask, so the spawn is the same.
5. Toggle value names: asm value 3 (BERSERK title) applies the Manic +0.5 and value 4 (FATIGUE title) applies -0.5.
   Kept as the asm applies them.
6. The throw hook RAM 0x8014B060 is in the asm `OS.patch_start`; the port puts the call where the asm's first
   overwritten pair (`lw a0, 0x2C(s1)`, `lw t1, 0xC(v1)`) sits, before the throw knockback calc.

## Verification

- `scripts/remix/syntax-check.sh` on: `ftremixrage.c`, `ftremixhooks.h`, `ftmain.c`, `ftparam.c`,
  `ftcommon/ftcommonthrown2.c`. All print `syntax-check OK`. Warnings in `ftmain.c` and `ftparam.c` are in existing
  code (`&&` inside `||`), not in these patches.
- Not built and not run, as the brief requires.
- The new `ftremixrage.c` must be picked up by the build (the lead wires the file list if the ftremix glob does not).
