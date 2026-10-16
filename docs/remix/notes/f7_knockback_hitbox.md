# F7 knockback and hitbox: Knockback.asm and Hitbox.asm

Brief: `docs/remix/f7_leftover_worker_brief.md`. Source: `.toolchain/pkgs/smashremix-src/src/Knockback.asm`, `Hitbox.asm`.
Not built, not run, no commits. Every touched file reports `syntax-check OK`.

## Knockback.asm

| Asm | RAM | Vanilla function (vanilla_funcs.txt) | Status |
|---|---|---|---|
| `randomize_angles_` (line 15) | 0x800D8008 | `ftManagerMakeFighter` (ft/ftmanager.c), after `fp->player = desc->player` | APPLIED |
| `get_angle_hitbox_` (line 142) | 0x800DF424 | `ftMainParseMotionEvent` (ft/ftmain.c), `attack_coll->angle` | APPLIED |
| `get_angle_throw_` (line 183) | 0x8014B16C | `ftCommonThrownReleaseThrownUpdateStats` (ft/ftcommon/ftcommonthrown2.c), `ft_throw->angle` argument | APPLIED |
| `randomize_angle_` (line 219) | 0xBB3FC | `ftCommonDamageGetKnockbackAngle` | SKIPPED: the whole block is commented out in the asm ("Scrapped") |

- Helper: `decomp/src/ft/ftremix/ftremixknockback.c`. Declared in `ftremixhooks.h`
  (`ftRemixKnockbackRandomizeAngles`, `ftRemixKnockbackAngle`).
- State: `sFTRemixKnockbackScreen[4]` (asm `screen_table`), `sFTRemixKnockbackAngles[4][0x200]` (asm
  `table_p1..p4`), `sFTRemixKnockbackState[4]` (asm `state_table`, written only by the CSS menu
  `css/Knockback.asm`, not ported).
- The CSS menu default is `DEFAULT_VALUE(0)` = "Default", so the state is 0 and the angle hooks return the vanilla angle.
  `randomize_angles_` still runs on every scene change and consumes RNG draws, as the asm does.
- RNG: `Global.get_random_int_` (0x80018994) is mapped to `syUtilsRandIntRange(N)`. This mapping is inferred from
  the call contract (returns 0..N-1), not checked against the `// 0x` comment of the decomp function.
- Scene: `Global.current_screen` (0x800A4AD0) is `gSCManagerSceneData.scene_curr` (scmanager.c:49).

## Hitbox.asm

| Asm patch | RAM | Vanilla function | Status |
|---|---|---|---|
| `force_grab_immunity_` (line 194) | 0x800E6064 | `ftMainSearchFighterCatch` (ft/ftmain.c), `is_grabbable` test | PARTIAL: Crash DSPWait/DSPTurn applied; Giga Bowser in Remix 1P is REMIX-TODO(singleplayer) |
| `hitbox_mode_` (line 24) | 0x800F2BFC | `ftDisplayMainProcDisplay` | SKIPPED, REMIX-TODO(hitbox display) |
| `projectile_` (line 116) | 0x80167538 | `wpDisplayMain` | SKIPPED, REMIX-TODO(hitbox display) |
| `grab_immunity_and_armor_frames_` (line 155) | 0x800F279C | `ftDisplayMainDrawParts` | SKIPPED (Advanced Hurtbox display, default off) |
| `make_hurtboxes_transparent_` (line 248) | 0x800F2830, 0x8017193C | `ftDisplayMainDrawParts`, `itDisplayHitCollisions` | SKIPPED |
| `make_hitboxes_transparent_` (line 325) | 0x800F33B0 | `ftDisplayMainProcDisplay` | SKIPPED |
| `add_reflect_absorb_hitboxes` (line 341) | 0x800F3358 | `ftDisplayMainProcDisplay` | SKIPPED (display list builder not ported) |
| `add_reflect_absorb_hitboxes` dw buffer sizes (lines 491-506) | 0x8013B9C8, 0x80138CC8, 0x80135B70, 0x80135B80 | mn VS/1P/results buffers | SKIPPED: data words, not ported |
| `render_hitbox_plus_model_` (line 510) | 0x800F2FD0 | `ftDisplayMainProcDisplay` | SKIPPED |
| `render_hitbox_plus_projectile_` (line 546) | 0x80167578, 0x801675AC | `wpDisplayMain` | SKIPPED |
| `render_hitbox_plus_item_` (line 589, 9 hooks) | 0x80171D20 .. 0x8018346C | `itDisplay*`, `itPippi*`, `itKabigon*` | SKIPPED |
| `fix_hitbox_position_` (line 713) | 0x800F3310 | `ftDisplayMainProcDisplay` | SKIPPED |
| star KO display fix (line 775) | 0x800F29A0 | `ftDisplayMainProcDisplay` | SKIPPED |
| `skip_rendering_hitboxes_` (line 781) | 0x800F336C | `ftDisplayMainProcDisplay` | SKIPPED |
| `render_ecb_over_hurtboxes_` (line 808) | 0x800F3648 | `ftDisplayMainProcDisplay` | SKIPPED |

- Helper: `decomp/src/ft/ftremix/ftremixhitbox.c`, `ftRemixForceGrabImmunity(FTStruct *victim)`, declared in `ftremixhooks.h`.
  The C site uses `other_fp`, the victim (asm `s0`, `lw t6, 0x0008(s0)`).
- Crash action ids: `DSPWait` 0xE6 and `DSPTurn` 0xE7 (Crash.asm `Character.add_new_action`, same as
  `ftremixcrashdsp.c`).
- Giga Bowser: the asm checks `SinglePlayerModes.singleplayer_mode_flag == REMIX_1P_ID (4)`. The flag
  (SinglePlayerMenus.asm line 420 writes it) has no decomp counterpart yet, so the branch stays FALSE.
- Toggles: `Toggles.entry_special_model` (Remix default OS.FALSE, Toggles.asm 2387) and
  `Toggles.entry_advanced_hurtbox` (default OS.FALSE, Toggles.asm 2388). Their settings menu is a later milestone.
  The hitbox display patches are skipped, so no toggle constant is wired yet.
- Uncertainty: the hitbox display patches with default-off toggles still change vanilla display state
  (`hitbox_mode_` writes 0 to 0x0B4C). Applying them as a set needs the display port first.

## Vanilla edits (all under `#ifdef REGION_REMIX`, vanilla branch unchanged)

- `decomp/src/ft/ftmanager.c`: one hook call after `fp->player = desc->player;`.
- `decomp/src/ft/ftmain.c`: `attack_coll->angle` in `ftMainParseMotionEvent`; the `is_grabbable` test in `ftMainSearchFighterCatch`.
- `decomp/src/ft/ftcommon/ftcommonthrown2.c`: the `ftCommonDamageInitDamageVars` call in `ftCommonThrownReleaseThrownUpdateStats`.
- `decomp/src/ft/ftremix/ftremixhooks.h`: appended block (append only).

## REMIX-TODOs

- `REMIX-TODO(toggles)`: CSS Knockback state (`sFTRemixKnockbackState`) in `ftremixknockback.c`. Remix default 0 ("Default").
- `REMIX-TODO(singleplayer)`: Giga Bowser force grab in Remix 1P, `ftremixhitbox.c`.
- `REMIX-TODO(hitbox display)`: every hitbox display patch in the table above, listed in `ftremixhitbox.c`.
