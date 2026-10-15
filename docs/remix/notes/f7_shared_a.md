# F7 shared_a: Dragon King HUD, Knockback, Dragon Ball, Sandbag

Brief: `docs/remix/f7_leftover_worker_brief.md`. Source asm: `.toolchain/pkgs/smashremix-src/src/`.
Not built, not run, no commits (per brief).

## 1. Dragon King HUD drain flash, RAM 0x8010E9E4: APPLIED

- Asm: `DragonKing/DragonKingSpecial.asm` lines 366-405, `scope drain_flash_fix_` (ROM 0x8A1E4).
  The source lives in `DragonKingSpecial.asm`, not `DragonKingHUD.asm` (that file has no patch).
- Vanilla function: `ifCommonPlayerDamageUpdateDigits` (`decomp/src/if/ifcommon.c`, 0x8010E8F4).
  Verified by disassembly: the prologue at 0x8010E8F4 stores `flash_reset_wait` into the branch at 0x8010E9E4.
- Vanilla semantics: `color_id = flash_reset_wait ? GMCOMMON_PLAYERS_MAX : player`. The override
  forces the normal colour when the displayed fighter's `capture_gobj` (0x844) is a Dragon King
  (fkind 0x47) with status 0xF0 (USPGroundThrow) or 0xF2 (USPAirPull) and the capturer's temp
  variable 1 (`motion_vars.flags.flag0`, 0x17C) is non-zero.
- Port:
  - New file `decomp/src/ft/ftremix/dragonking/ftremixdragonkingdrainflash.c`, guarded by
    `#if defined(PORT) && defined(REGION_REMIX)`. Routine `ftRemixDragonKingDrainFlashSuppressed(GObj *)`.
  - `decomp/src/ft/ftremix/ftremixhooks.h`: block appended before the final `#endif` with the prototype.
  - `decomp/src/if/ifcommon.c`: include of `ft/ftremix/ftremixhooks.h` under `#ifdef REGION_REMIX`, and the
    colour condition in `ifCommonPlayerDamageUpdateDigits` split with `#ifdef REGION_REMIX` / `#else`
    (vanilla path unchanged).
- Input: `gSCManagerBattleState->players[player].fighter_gobj` (SCPlayer field, sctypes.h). The asm reads
  it from the same player struct the function already indexes (0x78 offset not re-verified against
  struct_offsets.txt).
- Syntax check: the three files report `syntax-check OK`.
- Not verified in game.

## 2. Knockback patch at RAM 0x800E44EC: NOT APPLIED (address is not in Knockback.asm)

- 0x800E44EC is in `items/WaddleDoo.asm` (`scope beam_knockback_direction_fix`, ROM 0x5FCEC), not in
  `Knockback.asm`. It is a Waddle Doo beam item patch, so it is in the item scope, not this one.
- Vanilla enclosing entry: `ftMainProcessHitCollisionStatsMain` (`decomp/src/ft/ftmain.c`, 0x800E3EBC).
  The exact C statement is not mapped. The hook replaces `lui at,0x40A0; mtc1 at,f6` (5.0F threshold) and,
  for the Waddle Doo beam projectile (id `Projectile.id.DEDEDE_WADDLEDOO_BEAM`), sets f6 to 0.5F and
  stores `vel`/`0x7F4`, `0x7F8` fields, then jumps to 0x800E4508. Disassembled ROM 0x5FCB0-0x5FD20.
- Not applied: the register-level C site (f6 use after 0x800E4508) is not mapped, and a partial hook
  would change vanilla behaviour. Needs the item owner or a follow-up with the full disassembly.
- The Knockback.asm patches are separate: `randomize_angles_` (0x800D8008, ftManagerMakeFighter),
  `get_angle_hitbox_` (0x800DF424, ftMainParseMotionEvent), `get_angle_throw_` (0x8014B16C,
  ftCommonThrownReleaseThrownUpdateStats). They are the Remix knockback angle randomizer (training-mode
  random angles, screen tables, `Global.get_random_int_`). No port exists. NOT APPLIED; needs a scope decision.

## 3. Dragon Ball effect and DRAGONKING get_entry_file_ptr_: no change needed

- Dragon ball GFX: `decomp/src/ft/ftremix/dragonking/ftremixdragonkingnsp.c` no longer has a
  `REMIX-TODO(GFX)` marker. The only marker left in the NSP port is `REMIX-TODO(Size)` (lines 165, 174):
  the `Size.multiplier_table` factor is not applied, multiplier 1.0F. `47_DRAGONKING_NSP.md` is stale
  (it lists the GFX as skipped).
- The `REMIX-TODO(lag)` at line 270 concerns the Kirby copy routines, not this scope.
- `get_entry_file_ptr_` (`captainshared.asm` line 294, ROM 0x7EDBC, RAM 0x801035BC) only special-cases
  GND, JFALCON, BOWSER and CONKER. DRAGONKING is not in that list, and DragonKing has no entry script in
  `dFTRemixEntryScripts` (`ftremixentry.c`). No mismatch found. Nothing applied.

## 4. Sandbag file pointer: no change needed

- Sandbag is `REMIX_ID_SANDBAG` 0x3C (`port/remix/remix_roster_data.h`, `Character.asm` line 3739,
  parent CAPTAIN, entry script `nFTRemixEntryScriptCaptainCar` in `ftremixentry.c`).
- The car-entry file comes from `get_entry_file_ptr_`. Sandbag is not listed, so the asm falls back to
  Falcon's file 7 (0x8013103C). The port matches that: `dFTRemixEntryCarAnims` has no SANDBAG row, so
  `efManagerCaptainEntryCarMakeEffect` keeps `gFTDataCaptainSpecial2`.
- Sandbag's own fighter file is `File.SANDBAG_MAIN` (roster row, 0x0EB). No extra pointer is needed.
- Uncertainty: equality of `gFTDataCaptainSpecial2` with the Falcon file-7 pointer was not checked in code.

## Do not touch

- 0x800E33AC is in `Piano/PianoSpecial.asm` (line 1099). Owned by the piano worker. Not touched.

## REMIX-TODOs

- Existing, unchanged: `REMIX-TODO(Size)` in `ftremixdragonkingnsp.c` (lines 165, 174).
- New: none.

## Notes for the lead

- The scope brief names `docs/remix/notes/f7_shared_a.md`, which did not exist, so this file was created.
- Two addresses in the scope brief do not match their stated files: 0x800E44EC (WaddleDoo, not Knockback)
  and 0x8010E9E4 (DragonKingSpecial.asm, not DragonKingHUD.asm). The notes above reflect the actual sources.
- The new `ftremixdragonkingdrainflash.c` is picked up by the build only if the build globs the
  `ftremix` directories. Check that before the next build.
