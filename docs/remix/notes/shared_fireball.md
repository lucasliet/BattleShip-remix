# shared_fireball: fireball.asm patches ported to C

Source: `.toolchain/pkgs/smashremix-src/src/fireball.asm` (551 lines). Scope: cloned Mario fireball
(Dr. Mario capsule, J Mario, J Luigi, Dr. Luigi, Piano book). Syntax check: all touched files
report `syntax-check OK`. No build, no commit.

## Files touched

| File | Change |
|------|--------|
| `decomp/src/wp/wpvars.h` | REGION_REMIX fields on `wpMarioFireballAttributes`: `file_fkind`, `palette_base`, `palette_range` |
| `decomp/src/wp/wpmario/wpmariofireball.h` | `wpMarioFireballMakeWeaponAtAngle` prototype; `WP_MARIO_FIREBALL_INDEX_*` (REGION_REMIX) |
| `decomp/src/wp/wpmario/wpmariofireball.c` | Remix table entries (ids 2..6), spawn split (angle param), file/palette hooks, bounce/hit/egg hooks |
| `decomp/src/wp/wpmario/wpmarioremixfireball.c` (new) | Helpers, guarded `#if defined(PORT) && defined(REGION_REMIX)` |
| `decomp/src/ft/ftremix/ftremixhooks.h` | Appended declaration block for the new helpers |
| `decomp/src/ft/ftchar/ftmario/ftmariospecialn.c` | `ftMarioSpecialNProcAccessory` uses `wpRemixMarioFireballIndexForFkind` |
| `decomp/src/ft/ftchar/ftkirby/ftkirbycopymariospecialn.c` | `ftKirbyCopyMarioSpecialNProcAccessory`: index helper, Piano book branch, Piano spawn joint |

Not touched: `decomp/src/ft/ftremix/banjo/`, `decomp/src/ft/ftremix/piano/`.

## Exported helper for Piano's DSP port

`GObj* wpRemixPianoBookMake(GObj *fighter_gobj, Vec3f *pos)` (declared in `ftremixhooks.h`).
It implements `Book.create_`: one book per count (count = bonus_ammo truncated, or 1), resets bonus_ammo
at N64 0x0ADC, each book at its own spread angle. Returns the first book or NULL. Piano DSP should call it
in place of its single `wpMarioFireballMakeWeapon(…, WP_MARIO_FIREBALL_INDEX_BOOK)`.
Kirby copying Piano already calls it (see `ftkirbycopymariospecialn.c`).

## Patch-by-patch

| asm (RAM) | vanilla function | status | notes |
|-----------|------------------|--------|-------|
| `hit_effect_` (0x801686CC) | `wpMarioFireballProcHit` | applied | `wpRemixMarioFireballHitEffect`: capsule FGM 0x2C7 + normal hit gfx, book FGM 0x1F + dust + normal hit gfx; both skip the sparkle. Fireball and egg keep the vanilla explosion + sparkle. GFX id swap is REMIX-TODO(GFX). |
| `bounce_effect_` (0x801686A0) | `wpMarioFireballProcMap` (fire grind call) | applied | `wpRemixMarioFireballBounceEffect`: capsule and book small smoke, egg FGM 0x506, fireball keeps fire grind. |
| `bounce_destroy_check` (0x80168664) | `wpMarioFireballProcMap` (`vel_min` compare) | applied | `wpRemixMarioFireballMinVel`: egg threshold 20.0 if `coll_data.mask_curr & 0x21 == 0`, else -1280.0. The egg (`ftremix/banjo`, `BANJO_EGG_PROJECTILE_ID` 0x21) reuses `wpMarioFireballProcMap`, so the check applies to it; banjo files were not edited. |
| `Book.create_` (called from 0x801687A0 path) | none (Piano-only) | applied as export | `wpRemixPianoBookMake`, see above. |
| `Book.kirby_fix_` (0x80156A08) | `ftKirbyCopyMarioSpecialNProcAccessory` (joint) | applied | Piano copy spawns from `joints[7]` (N64 0x0904); other copies keep `joints[17]` (0x092C). |
| `capsule_subroutine_` (via `load_capsule_subroutine_` 0x80156000 / 0x80156BBC) | `ftMarioSpecialNProcAccessory` / Kirby copy | applied in spawn path | Random palette chosen at spawn in `wpMarioFireballMakeWeaponAtAngle` (`palette_range`). The asm only calls the original accessory after setting the palette; the load routine itself (setting the accessory pointer) is handled by the Remix routing. |
| `capsule_flag` (0x8016880C) | `wpMarioFireballProcHit` input | skipped, replaced | The asm writes 1 at N64 0x2A0 to mark Dr. Luigi. On WPStruct that slot is `weapon_vars.fireball.index`, so the flag would clobber the index. The port identifies Dr. Luigi by `index == WP_MARIO_FIREBALL_INDEX_DRL`. |
| `get_fireball_struct_` (0x80155EB4, 0x80155EC0) | `ftMarioSpecialNProcAccessory` | applied | Remix fkind switch via `wpRemixMarioFireballIndexForFkind`; clones are no longer rejected by the AVOID_UB return. |
| `get_kirby_fireball_struct_` (0x80156A24, 0x80156A30) | `ftKirbyCopyMarioSpecialNProcAccessory` | applied | Same helper, keyed by `passive_vars.kirby.copy_id`. |
| `add_to_character` x4 (DRM, JMARIO, JLUIGI, DRL) | fireball jump tables | applied as table | Index map: DRM 2, JMARIO 3, JLUIGI 4, DRL 6 (Book 5 for Piano). |
| structs `capsule`, `jmario`, `jluigi`, `book`, `drl` | `dWPMarioFireballWeaponAttributes[]` | applied | Entries 2..6, Remix only. |

## Table values (struct, order, file, palette)

| id | name | lifetime | vel max | vel min | gravity | bounce | rotate | angle g/a | init | file fkind | palette |
|----|------|----------|---------|---------|---------|--------|--------|-----------|------|------------|---------|
| 2 | capsule | 140 | 60 | 25 | 1.5 | 0.95 | 0.3 | -0.4/-0.4 | 40 | DRM | random 0-2 |
| 3 | jmario | 140 | 55 | 30 | 1.2 | 0.85 | 0.3490659 | -0.08726647/-0.08726647 | 50 | JMARIO | 0 |
| 4 | jluigi | 90 | 55 | 30 | 0 | 0.85 | 0.4363323 | 0/0 | 36 | JLUIGI | 1 |
| 5 | book | 120 | 60 | 25 | 1.4 | 0.55 | 0.139626 | 1.0472/1.0472 | 50 | PIANO | 0 (spread angle) |
| 6 | drl | 100 | 55 | 30 | 0 | 0.85 | 0.4363323 | 0/0 | 36 | DRL | random 3-5 |

Projectile file: `wpRemixMarioFireballFile(file_fkind)` = `port_fighter_data(file_fkind)->p_file_special1`.
Offset for every Remix entry is 0, as the asm's `dw 0` field (fireball.asm line 39).

## Uncertainties (asm line numbers)

1. Hit damage: asm reads WP 0x234 (lines 131, 155). The port uses `wp->hit_normal_damage`, the field the struct table names at 0x234.
2. Bounce smoke args: `0x800FF048` (lines 203, 209) is called with a1/a2 left by the caller. The port passes `wp->lr` and `1.0F` (`efManagerDustLightMakeEffect`). Verify.
3. Book dust: `0x801003D0` takes `wp->lr` (line 149), as ported.
4. Capsule palette: the asm writes the random palette to the struct chosen by hat (Kirby, lines 404-411) or DRL fkind (lines 395-400). The spawned struct comes from the copy id (lines 432-437), so for a mismatched hat the asm spawns the table struct with a stale palette. The port applies the hat-based palette range to the spawned weapon (Kirby DRM hat 0x10 = 0-2, else 3-5). This is a deliberate deviation.
5. Capsule palette is rolled on every accessory tick with temp var 1 set (line 386), not only on spawn. The port rolls it once per spawn.
6. Piano bonus ammo: count = bonus when bonus > 0, else 1 (lines 280-285, 331-334). The asm loop with a negative bonus is not ported (it cannot be reached by normal play).
7. Book angle: the asm writes the same angle to ground and air (lines 318-320). Ported as a single angle.
8. Piano check uses `REMIX_ID_PIANO` (asm `Character.id.PIANO`, line 272). The Remix clone id is assumed to be the character id.
9. Egg hit (line 112, `// todo`) plays the vanilla fireball explosion and sparkle, as the asm does.
10. Remix fireball index 5 (book) is not in the accessory switch: Piano spawns only through `wpRemixPianoBookMake`.

## REMIX-TODO

- `REMIX-TODO(GFX)` in `wpRemixMarioFireballNormalHitGfx` (`wpmarioremixfireball.c`): the normal-hit GFX instruction id is not swapped to 0x81 (Dr. Mario) or 0x97 (Dr. Luigi); the vanilla 0x49 is used (asm lines 10-13, 121-136). Needs a GFX hook around `efManagerDamageNormalLightMakeEffect`.

No other `REMIX-TODO` markers. Dr. Luigi's hit GFX uses the same TODO.
