# f7 shared_c: leftover Remix patches (fireball bounce, bob-omb walk FGM, Link boomerang destroy)

Scope from `docs/remix/f7_leftover_worker_brief.md`. Syntax check: all touched files report `syntax-check OK`.
No cmake, build, game run or git. Cloud `enable_flash` skipped (owned by another worker).

## Files touched

| File | Change |
|------|--------|
| `decomp/src/it/itcommon/itbombhei.c` | `itBombHeiWalkInitVars`: REGION_REMIX guard, bob-omb fuse FGM only for `nITKindBombHei` |

No other file was edited in this scope. The fireball and Link boomerang patches were already applied by the earlier
waves (see below), and this pass only verified them.

## Patch status

| # | asm (RAM) | source | vanilla function | status |
|---|-----------|--------|------------------|--------|
| 1 | `bounce_effect_` (0x801686A0) and `bounce_destroy_check` (0x80168664) | `fireball.asm` | `wpMarioFireballProcMap` (`decomp/src/wp/wpmario/wpmariofireball.c`) | ALREADY APPLIED, verified (below) |
| 2 | `remove_bob_bomb_walk_fgm_` (0x8017798C, ROM 0xF23CC) | `items/WaddleDee.asm` (line 130), `items/ScuttleMinion.asm` (same hook) | `itBombHeiWalkInitVars` (0x80177848, `decomp/src/it/itcommon/itbombhei.c`) | APPLIED in this pass |
| 3 | `boomerang_crash_fix_` (0x8016D364, 0x8016D3F4, 0x8016D58C) | `linkshared.asm` lines 788-820 | `wpLinkBoomerangCheckOwnerCatch` / `wpLinkBoomerangProcUpdate` (`decomp/src/wp/wplink/wplinkboomerang.c`) | ALREADY APPLIED, verified (below) |

## 1. Fireball bounce (fireball.asm)

- The Dr. Mario, Dr. Luigi, J Mario, J Luigi and Piano fireballs all use `fireball.asm`. `DrMario`, `DrLuigi`, `JMario`,
  `MLuigi` and `JLuigi` directories contain no bounce or fireball patch of their own (grep checked).
- `bounce_effect_`: capsule and book play small smoke (`efManagerDustLightMakeEffect`), egg plays FGM 0x506, fireball keeps
  the fire grind (`0x80102DEC` = `efManagerFireGrindMakeEffect`). Port: `wpRemixMarioFireballBounceEffect`.
- `bounce_destroy_check`: egg min velocity 20.0 when `mask_curr & 0x21 == 0`, else -1280.0 (the asm `lui` delay-slot order
  was checked). Port: `wpRemixMarioFireballMinVel`.
- Still REMIX-TODO(GFX): the normal-hit GFX id swap (0x81 Dr. Mario, 0x97 Dr. Luigi) in `wpRemixMarioFireballNormalHitGfx`.
- Uncertainty (already noted in `shared_fireball.md`): `0x800FF048` arguments `wp->lr` and `1.0F` are assumed.

## 2. Bob-omb walk FGM (WaddleDee.asm hook)

- The asm hook at 0x8017798C plays FGM `nSYAudioFGMBombHeiFuse` (0x2D) only when the item id is `Hazards.standard.BOBOMB`
  (`nITKindBombHei`). Minions (Waddle Dee, Waddle Doo, Scuttle) reuse this walk init and must stay silent.
- Port: `itBombHeiWalkInitVars` under `#ifdef REGION_REMIX`, checking `ip->kind == nITKindBombHei`. The `#else` keeps the
  original call.
- The `REMIX-TODO(vanilla patch)` in `item_WaddleDee.md` is now applied. Its owning scope should update that note.

## 3. Link boomerang destroy path (linkshared.asm boomerang_crash_fix_)

- The asm stops `wpLinkBoomerangCheckOwnerCatch` from calling `wpMainDestroyWeapon` when the boomerang is caught, and returns
  TRUE instead. The port does the same under `REGION_REMIX`.
- Verified: `wpProcessProcWeaponMain` (`decomp/src/wp/wpprocess.c` line 133) calls `wpMainDestroyWeapon` when `proc_update`
  returns TRUE, so the destroy happens once. The RETURN branch's tail (asm 0xE7FCC, `or v0,r0,r0` replaced by `nop`) matches
  the port's `return wpLinkBoomerangCheckOwnerCatch(...)`.
- The Item Boomerang (`items/Boomerang.asm`, `item_Boomerang.md`) has no destroy patch of its own. Its only destroy call
  is `itMainDestroyItem` in the tomato tail, already ported. `Item.asm` `prevent_item_despawn_` skips Boomerang despawn on
  floors; it is a Marina/Item vanilla patch outside this scope and is not checked here.

## REMIX-TODO

- `REMIX-TODO(GFX)`: `wpRemixMarioFireballNormalHitGfx` (`decomp/src/wp/wpmario/wpmarioremixfireball.c`), hit-GFX id swap.

## Uncertainties

1. The `itBombHeiWalkInitVars` hook assumes the Remix item kind for Bob-omb is `nITKindBombHei` (0x0F). Verify in game
   that a Waddle Dee or Scuttle minion walks silently and a Bob-omb still plays the fuse.
2. The asm patch replaces two instructions (jal + delay slot) at 0x8017798C; the port replaces the whole `func_800269C0_275C0`
   call, so the same instruction order holds.
