# F8 Parrying (BETA) (Parry.asm)

Setting: `Parrying (BETA)`, `Toggles.asm` `entry_parry` (line 2436). Values: `0` Off (default in CE, TE, NE, JP), `1` On.
Override for a run: `SSB64_REMIX_TOGGLES="Parrying (BETA)=1"`. The asm reads the value directly (`entry_parry + 4`), so the
port uses `gmRemixToggle(parry)` in every entry point. No Bonus or 1P Game guard.

Port: `decomp/src/ft/ftremix/ftremixparry.c` (new), hooks in `decomp/src/ft/ftmain.c` (four sites under `#ifdef REGION_REMIX`),
declarations appended to `ftremixhooks.h`. The new file is picked up by the `GLOB_RECURSE` in `CMakeLists.txt` at the next
configure.

## Patches

| Asm (Parry.asm) | RAM | Vanilla function | Status | Notes |
|---|---|---|---|---|
| `direct_hit_parry_bubble_check` (line 16, patch 0x60538) | 0x800E4D38 | `ftMainSearchHitFighter` (ftmain.c) | applied | Hook before the `is_shield` check inside `if (k != 0)`. A parry skips only the shield block: the asm jumps to 0x800E4DB4, the start of the damage block, which the intangible hurtbox then skips. |
| `projectile_hit_parry_bubble_check` (line 229, patch 0x60AC0) | 0x800E52C0 | `ftMainSearchHitWeapon` (ftmain.c) | applied | Hook before the `is_reflect` check inside `if (l != 0)`. A parry does `goto next_gobj` (asm 0x800E55A0). |
| `item_hit_parry_bubble_check` (line 429, patch 0x611A0) | 0x800E59A0 | `ftMainSearchHitItem` (ftmain.c) | applied | Hook before the `is_reflect` check inside `if (l != 0)`. A parry does `goto next_gobj` (asm 0x800E5BF4). |
| `apply_attacker_extra_hitlag` (line 624, patch 0x61E90) | 0x800E6690 | `ftMainProcParams` (ftmain.c) | applied | Hook after `fp->hitlag_tics = ftParamGetHitLag(...)`: +14 once when the attacker's hit was parried. |
| `activate_parry` (line 662) | none | `ftremixparry.c` `ftRemixParryActivate` | applied | ClangRecoil (0x53), intangible hurtbox, GFX routine PARRY, cleared hit values, sound 0x5BB, spark, ground bump (ftParamMakeEffect 0x16). |
| `check_parry_collision` (line 779) | none | `ftremixparry.c` `ftRemixParryCollides` | applied | Shield sphere with size 1.5F times `shield_size`, offset (0, map_coll.center, 0), gmCollisionTestSphere. |
| `is_hit_parry` (line 9) | none | `ftremixparry.c` `sFTRemixParryIsHitParry` | applied | Per port `u8` counter. |

No `REMIX-TODO` markers: no AI, Size, GFX or announcer code in this setting. The PARRY GFX routine is requested through
`ftParamCheckSetFighterColAnimID`, which already loads the Remix routine table for ids >= 0x56.

## Deviations and uncertainties

1. Direct hit, shield block skip (the patch returns to 0x800E4DB4 after a parry, which is the `bgez` target of the
   `is_shield` test in the vanilla code): the port evaluates the parry first and then `is_shield`, so a parry skips only the
   shield block and falls into the damage block. That block is skipped by the intangible hurtbox set in `ftRemixParryStart`.
2. Previous hitbox position (Parry.asm lines 99-102 direct hit, 319-323 projectile): the asm `beqzl at` tests the constant 2
   instead of the interpolate flag, so the delay slot is never run and the previous position is always used. The port keeps
   this for the direct and projectile hits. The item hit (lines 516-521) is correct and uses the previous position only when
   the state is interpolate (3), as the port does in `ftRemixParryItemPrevPos`. Only interpolated hitboxes can differ.
3. Projectile and item loops, skipped hitbox (Parry.asm line 275-281 and 475-481): when `gFTMainIsDamageDetect[i]` is FALSE,
   the asm does `lw v0, 0x50(t7)` (the attack count, nonzero) and jumps to `_loop_check_end`, which treats it as a collision.
   The port keeps that (`ftRemixParryWeaponHasHit` and `ftRemixParryItemHasHit` return TRUE). Effect: a multi-hitbox
   projectile or item in fighter range can be parried by a hitbox that is outside the range, when the parry window is open.
   Single-hitbox weapons and items are not affected (the skipped case needs a second hitbox). This looks like a Remix bug.
   Lead decision needed: keep it (this port) or fix it to `v0 = 0`.
4. Direct hit, attacker and defender hitlag (Parry.asm line 133-199): attacker `hitlag_tics` = `gmCommon_DamageCalcHitLag(
   hitbox damage + shield damage, defender status, 1.0F)` (no minus one), defender `hitlag_tics` = attacker hitlag + 11. Weapon
   and item: the parrier gets the same calc minus one tic when not zero (Parry.asm line 403-404, 598-599).
5. `activate_parry` frame argument: the asm comment says starting frame 0.0, but the encoded word is `lui a2, 0x3F80`, which
   is 1.0F. The port passes 1.0F.
6. The hurtbox write `sb t1, 0x05BB(a1)` (Parry.asm line 685) is the low byte of the `s32 hitstatus` at 0x5B8. The port writes
   `fp->hitstatus = nGMHitStatusIntangible` (value 3), which is the same value.
7. GFX routine id: `FTREMIX_PARRY_GFX_ROUTINE` is 0x7D. It is computed from `GFXRoutine.asm` (`id.PARRY = 0x55 + 40`,
   PARRY is the 40th `add_gfx_routine` line). Not checked against `build/remixasm/remix.sym`. If the parry glow shows the wrong
   colour animation, check this constant first.
8. Projectile stale damage: `wpMainGetStaledDamage` and `itMainGetDamageOutput` are used as the asm does (the damage written to
   `hit_normal_damage`, 0x234 and 0x264). The asm comment in the projectile hook says the game does not add shield damage;
   the port does not add it either.
9. Parry damage accounting: a parry does not run `ftMainUpdateShieldStatFighter`, so `shield_damage` and `shield_health` are not
   changed by a parry.

## Verification (lead)

- Build and run are not done here. `scripts/remix/syntax-check.sh` reports `syntax-check OK` for the three touched files.
- Set `SSB64_REMIX_TOGGLES="Parrying (BETA)=1"`. In a short match, hold shield on the ground and release it within 5 frames of
  the shield action, while an enemy hitbox reaches the shield. Expect: ClangRecoil animation, parry sound, white spark, the
  attacker freezes longer (hitlag + 14), no shield damage.
- Same test with a projectile (Fox blaster, Mario fireball): the projectile is consumed, the defender gets the stale-damage hitlag.
- `=0` (Off): vanilla behaviour, including the shield break and the shield damage.
- Item 3 above: a multi-hitbox projectile at the edge of the fighter range may parry without touching the shield sphere.
