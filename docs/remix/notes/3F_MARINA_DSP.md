# 3F MARINA DSP: notes for the port in decomp/src/ft/ftremix/marina/ftremixmarinadsp.c

Source: .toolchain/pkgs/smashremix-src/src/Marina/MarinaSpecial.asm, scope MarinaDSP (lines 1350-2623).
Table: dFTRemixMarinaDSPProcs (14 of 14 worklist routines), dFTRemixMarinaDSPProcsNum.

## 1. Vanilla patches (not editable from this file)

### 1a. Item absorb charge (RAM 0x800E5A10, ROM 0x61210)

- Decomp function: `ftMainSearchHitItem`, decomp/src/ft/ftmain.c:3600.
- Patched instruction: the `jal 0x800E3860` (`ftMainUpdateReflectorStatItem`) at ftmain.c:3721.
- What the patch does: when the fighter's reflect hitbox (`fp->special_coll`) is the
  custom type (reflect type 3) with a non-zero custom id, it sets `add_charge` to the
  absorbed item's charge level, then runs the original call.
- Insert before ftmain.c:3721 (`ftMainUpdateReflectorStatItem(...)`), inside the existing branch:

  ```c
  #ifdef REGION_REMIX
                                      if (((fp->special_coll->kind & 0xFFFF) == 0x0003) && ((fp->special_coll->kind >> 16) != 0))
                                      {
                                          ftRemixMarinaDSPSetAddCharge(fp, ftRemixMarinaDSPGetItemChargeLevel(ip));
                                      }
  #endif
  ```

- Required prototypes (both defined non-static in ftremixmarinadsp.c):

  ```c
  s32 ftRemixMarinaDSPGetItemChargeLevel(ITStruct *ip);
  void ftRemixMarinaDSPSetAddCharge(FTStruct *fp, s32 add_charge);
  ```

- The `kind` packing: Remix's absorb_struct has index in the high halfword and reflect type in the
  low halfword. The port stores it as `(index << 16) | type` in `special_coll->kind`.

### 1b. Projectile absorb charge (RAM 0x800E5330, ROM 0x60B30): NOT PORTED

- Decomp function: `ftMainSearchHitWeapon`, decomp/src/ft/ftmain.c:3403, patched call at ftmain.c:3526
  (`ftMainUpdateReflectorStatWeapon`, 0x800E31B4).
- Calls `get_projectile_level_` (MarinaSpecial.asm:2415), which is outside the DSP worklist and was
  not ported here. Whoever owns the Marina projectile patch must port it, or this absorb will not
  add charge for projectiles.

## 2. Shared subsystems (REMIX-TODO markers in the port)

1. **Item spawn (`create_and_assign_clanpot_item_`, MarinaSpecial.asm:2264-2340).**
   `pull_main_` (ftremixmarinadsp.c, `ftRemixMarinaDSPPullProcUpdate`) clears temp variable 1 and the
   charge level, but does not spawn the Gem, Shuriken, Boomerang or ClanBomb (`Item.*.SPAWN_ITEM`), does
   not set `Item.skip_item_spawn_gfx_`, and does not pick the item up with `itMainSetFighterHold`
   (0x80172CA4). The pull therefore creates no item. This is the largest gameplay gap in this file.
2. **Custom reflect routine (reflect type 3).** The absorb hitbox is attached (`absorb_setup_`,
   MarinaSpecial.asm:2246-2258) as `sFTRemixMarinaDSPAbsorbColl`, which is laid out like Remix's
   `absorb_struct` (MarinaSpecial.asm:2613-2623). The Remix reflect routine itself is not ported.
   Vanilla `gmCollisionCheckItemAttackSpecialCollide` already consumes `special_coll`.

## 3. Uncertainties (asm line numbers)

1. **absorb_initial_ kinetic test (MarinaSpecial.asm:1525-1532).** The asm reads
   `lw t6, 0x018C(v0)` (the flag word) and labels it "kinetic state", branching to DSPA_Absorb when
   nonzero. The decomp puts kinetic state in `ga` (0x14C), and Remix reads 0x14C for kinetic
   state elsewhere (MarinaSpecial.asm:181, 1229). The port uses `fp->ga == nMPKineticsAir`. This
   changes behaviour when the flag word is nonzero but the fighter is grounded (and vice versa).
   Confirm against the built Remix ROM before relying on it.
2. **Action ids 0xFE to 0x10B.** Marina.asm has no named DSP constants. The ids come from the
   `add_new_action` order (Marina.asm:532-574: CargoShake = 0xFB, so DSPG_Begin = 0xFE). The ROM agrees:
   in remix.z64, the `ground_begin_main_` entry is 3 action entries (0x14 bytes each) after
   `shake_main_`, and `ground_end_main_` is 3 entries after `ground_begin_main_`. The commented
   Clanpot labels at Marina.asm:168-181 use the same ids.
3. **Item charge table (MarinaSpecial.asm:2497-2577).** 76 entries, index = item kind. The Turnip
   id 0x4B is derived from the add_item order in Item.asm:1515-1548 (custom items start at 0x2D, one
   per call). Not checked in game.
4. **Turnip charge (MarinaSpecial.asm:2469-2483).** Peach Turnip damage 30, 12 and 6 map to 4, 3 and
   2. Other damage values fall through to the table, as in the asm (no branch to the end).
5. **ground_wait_main_ / air_wait_main_ (MarinaSpecial.asm:1785-1823).** These do not call
   update_buffer_, so b_press_buffer is read as the previous frame's value. `-1` (first frame) counts
   as a pull request because the asm tests `beqz` (any nonzero value).
6. **Release lag reuse.** `ftNessSpecialLw_UpdateReleaseLag` and `ftNessSpecialLw_InitStatusVars` are
   the Ness routines the asm calls at 0x80155518 and 0x80155454. They write release_lag (0x0B18),
   is_release (0x0B1C) and gravity_delay (0x0B20), which the port reads as min_frame_timer,
   b_not_held and b_press_buffer (the latter reset to -1 after the Ness init, as in the asm).
7. **ground_end_main_ / air_end_main_ / pull / stow idle check (MarinaSpecial.asm:1977-1984).** The
   asm transitions to idle when the animation frame is `<= 0.0`, taken literally.
8. **Taunt voice (MarinaSpecial.asm:1617-1628).** Used in pull_initial_ only: alternate FGM when
   `button_hold & button_mask_l` is nonzero, else the default. FGM ids 0x420 and 0x43C
   (MarinaSpecial.asm:28-29). The FGM play function is `func_800269C0_275C0`.
9. **Status flags.** Table arguments 0x0092 and 0x0097, and the wait and absorb flags 0x0804 and 0x0006,
   use the decomp FTSTATUS_PRESERVE_* bits as listed in decomp/src/ft/ftdef.h. The port keeps them as
   written in the asm. This assumes the same bit meaning as ftMainSetStatus's flags.
10. **Charge cap on stow and absorb.** The charge is capped at MARINA_DSP_MAX_CHARGE (4), as in the
    asm's `sltiu` checks (MarinaSpecial.asm:1549-1553 and 2085-2089).

## 4. Vanilla call mapping used by the port

| Asm call | Port call |
|---|---|
| 0x800E6F24 | ftMainSetStatus |
| 0x800E0830 | ftMainPlayAnimEventsAll |
| 0x80155454 | ftNessSpecialLw_InitStatusVars (extern) |
| 0x80155518 | ftNessSpecialLw_UpdateReleaseLag (extern) |
| 0x800D9480 | ftAnimEndCheckSetStatus |
| 0x800DDE84 | mpCommonProcFighterOnEdge |
| 0x800DE6E4 | mpCommonProcFighterLanding |
| 0x800DEE54 | mpCommonSetFighterWaitOrFall |
| 0x800DEE98 | mpCommonSetFighterGround |
| 0x800DEEC8 | mpCommonSetFighterAir |
| 0x800D8EB8 | ftPhysicsClampAirVelXMax |
| 0x800269C0 | func_800269C0_275C0 (extern) |
| 0x801728D4 | itMainDestroyItem |
| 0x80172CA4 | itMainSetFighterHold (NOT called: REMIX-TODO(Item)) |

## Hooks applied (absorb charge)

Supersedes section 1a (item absorb is applied) and section 1b (projectile absorb is ported).
Both patches check the reflect hitbox the same way the asm does.

| Patch | RAM | Vanilla function (file:line) | Inserted C (under `#ifdef REGION_REMIX`) |
|---|---|---|---|
| `item_absorb_charge_` | 0x800E5A10 | `ftMainSearchHitItem` (`decomp/src/ft/ftmain.c`), before `ftMainUpdateReflectorStatItem` | `ftRemixMarinaDSPAbsorbItemCharge(fp, ip);` |
| `projectile_absorb_charge_` | 0x800E5330 | `ftMainSearchHitWeapon` (`decomp/src/ft/ftmain.c`), before `ftMainUpdateReflectorStatWeapon` | `ftRemixMarinaDSPAbsorbProjectileCharge(fp, wp);` |

- The reflect-hitbox test is `static ftRemixMarinaDSPIsCustomReflect`. It is the asm check: type 3 in
  the low halfword and a non-zero custom id (the franklin badge is 0). It also returns FALSE when
  `special_coll` is NULL, which the asm does not check.
- `ftRemixMarinaDSPAbsorbItemCharge` and `ftRemixMarinaDSPAbsorbProjectileCharge` are non-static. Their
  prototypes are in `ftremixhooks.h`. Both call `ftRemixMarinaDSPSetAddCharge`.
- `get_projectile_level_` (MarinaSpecial.asm L2415-2455) is `static ftRemixMarinaDSPGetProjectileChargeLevel`,
  with the charge-shot branch in `ftRemixMarinaDSPGetChargeShotLevel`. Charge size is read from
  `wp->weapon_vars.charge_shot.charge_size`. That is N64 0x2A4, which the port layout puts at 0x2A0
  (`build/remixasm/refs/struct_offsets.txt`).
- Projectile ids (`Projectile.asm`: VANILLA_LAST_ID 0x1F, then `add_projectile` order): GOLDENGUN_BULLET
  0x26, PIRATELAND_CANNONBALL 0x28. The other ids are the literal values in the asm. Not checked in game.

Uncertainties: the item condition now lives in one helper rather than inline in ftmain.c, which
keeps the two hooks identical. The item pull still spawns no item (section 2.1 above is unchanged).
