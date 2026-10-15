# 0x3B SONIC: SonicUSP (Spring Jump) port notes

Port file: `decomp/src/ft/ftremix/sonic/ftremixsonicusp.c`
Table: `dFTRemixSonicUSPProcs` / `dFTRemixSonicUSPProcsNum` (5 entries).
Source: `.toolchain/pkgs/smashremix-src/src/Sonic/SonicSpecial.asm` (scope SonicUSP, lines 1198-1428).

Ported (worklist, 5 of 5): `ground_initial_` (1203), `air_initial_` (1237), `main_air_` (1270),
`interrupt_` (1353), `air_physics_` (1376). Helper `_create_spring` / `_begin_movement` are inlined in
`main_air_` as static helpers.

## Required follow-ups outside this file (not edited by this worker)

1. Register the table. `ftremixsonic.c` is not edited here. Either add the table to
   `ftremixported.c` (`extern` + `{ dFTRemixSonicUSPProcs, &dFTRemixSonicUSPProcsNum }`) or
   merge it into `dFTRemixSonicProcs` in `ftremixsonic.c`. Both owners must agree.
2. Vanilla patch, `SonicUSP.action_check_patch_` (asm 1397-1428). It hooks
   `ftCommonSpecialAirCheckInterruptCommon` (0x80150F08) at 0x80150F10
   (`decomp/src/ft/ftcommon/ftcommonspecialair.c:176`). The patch returns FALSE early (asm `j 0x8015104C`) when the
   fighter is Sonic and the up special bool is set. Insert at the top of the function body under `#ifdef REGION_REMIX`:

   ```c
   #ifdef REGION_REMIX
       if ((fp->fkind == REMIX_ID_SONIC) && ftRemixSonicUSPIsUpSpecialUsed(fp))
       {
           return FALSE;
       }
   #endif
   ```

   Needs `#include "ft/ftremix/ftremix.h"`-level visibility of the prototype:
   `sb32 ftRemixSonicUSPIsUpSpecialUsed(FTStruct *fp);` (non-static, defined in this file).
   Verify that `Character.id.SONIC` equals `REMIX_ID_SONIC` (0x3B) in the port. The asm uses the same name.
3. `SonicUSPRefresh` (asm 1184-1186, ROM/RAM 0x80560200, `REMIX_SYM_SonicUSPRefresh`) is the on_hit callback
   (`Character.table_patch_start(on_hit, Character.id.SONIC, 0x4)`, asm 1188-1196). It is not in the worklist and is not
   ported. Its body is `sw r0, 0x0ADC(a0)`, so it clears the up special bool. A port would be
   `*ftRemixSonicUSPUpSpecialBool(fp) = FALSE;` and it needs the on_hit calling convention. Whoever owns the on_hit tables
   should decide.
4. Spring projectile (see below). The projectile id `Projectile.id.SONIC_SPRING` is 0x20
   (`.toolchain/pkgs/smashremix-src/src/Projectile.asm`, first `add_projectile`). The vanilla `nWPKind` enum ends at
   0x1F (`decomp/src/wp/wpdef.h:77`). A WPKind for 0x20 must be added before the spring can be spawned.

## Shared subsystems: not ported (REMIX-TODO(weapon))

The spring projectile is a weapon. Its routines are not in the worklist, and they are called from `main_air_`
through `jal`, so they are out of scope here. The surrounding logic is ported, and the spawn is replaced by a marker.

- `SonicUSP.spring_stage_setting_` (asm 1456-1549): calls `wpManagerMakeWeapon` (0x801655C8) with
  `spring_projectile_struct` and the player GObj. It sets the spring's image to coiled, sets the spring kinetic state
  (`0x00FC`) from the ground/air flag, stores the object in temp variable 3, and plays FGM 0x3D7 (classic 0x3DF) via
  `FGM.play_`. `Sonic.classic_table[port]` picks the classic variant.
- `SonicUSP.spring_main_` (asm 1552-1659): duration countdown (`wpMainDecLifeCheckExpire` equivalent 0x80167FE8),
  gravity and rotation (0x80167FA0 and 0x80168088), smoke on expiry (0x800FF648), and the enable/uncoil logic.
- `SonicUSP.check_spring_bounce_` (asm 1665-1857): players bounce off the spring. It has per-character action
  checks for Fox, Falco, Peppy, Kirby, JKirby, Yoshi, Bowser, GBowser, Ness, Lucas, Conker, Marth. The Ness and Lucas
  case clears the psi magnet overlay (0x0A20-0x0A88). It also does the jump smoke (0x800FF3F4), the spring rumble
  (`Global.rumble_`) and restores Sonic's specials (up special bool cleared).
- `spring_projectile_struct` (asm 1999-2012, RAM 0x80560AB0) and `spring_properties_struct` (asm 2016-2029, RAM 0x80560AF0):
  the Remix weapon descriptor and its properties. Proc map from the descriptor: main = `spring_main_`,
  clip and hurtbox = 0x80169108 (`wpSamusBombProcMap`, `decomp/src/wp/wpsamus/wpsamusbomb.c:131`), clang =
  0x80168A14 (`wpFoxBlasterProcReflector`, `decomp/src/wp/wpfox/wpfoxblaster.c:96`), and the rest are 0. These mappings
  are read from the asm, not verified in the port.
- `change_spring_room_` (asm 1431-1449): vanilla patch at 0x80165C2C, which is not in `vanilla_funcs.txt`. It
  moves the spring into room 0x0B. It belongs to the spring projectile port.

Consequence: Sonic's Spring Jump moves Sonic up and starts the aerial movement, but the spring object is not visible
and does not bounce other fighters.

## Remix state modelled in this file

- Temp variable 3 (`motion_vars.flags.flag2`, asm 0x0184). In Remix this holds the spring GObj pointer.
  `main_air_` tests its high halfword (asm `lhu 0x0184`). While the spring is not created, the port sets bit
  `SONIC_USP_SPRING_OBJECT_SLOT` (0x00010000) as a stand-in so the movement starts. **Remove this stand-in when the
  spring object is ported.** Low 16 bits (asm `sb 0x186`, `sb 0x187`) hold the floor line ID byte and the
  spring-in-air flag, written by `ground_initial_`. The spring creation reads them.
- Movement flag (0x0B18) and grounded flag (0x0B1C): `sFTRemixSonicUSPMovement[]` and
  `sFTRemixSonicUSPGrounded[]`, indexed by `fp->player`.
- Up special bool (0x0ADC, `passive_vars`): accessed through `(s32 *)&fp->passive_vars`. Other Sonic routines
  use the same word (SSonic counter, asm 2943-2948), so it stays in fighter memory and is not a static.

## Uncertainties (asm line numbers in SonicSpecial.asm)

1. 0x0B18 and 0x0B1C overlap other Sonic routines. Target object (NSP, asm 176/262/294/350) and charge level (DSP,
   asm 2106-2243) use them too. The port makes them per-player statics, which is equivalent only because
   `air_initial_` resets both at the start of every USP. The other workers must agree.
2. Stand-in slot (see above, asm 1312-1341 and 1283-1295). Movement with the spring absent is an assumption. Check
   this against the real Remix game.
3. `ground_initial_` (asm 1203-1230): the spring Y lift (+144.0F, `0x43100000`) is written before `air_initial_`
   (the asm store is in the delay slot of `jal air_initial_`). `mpCommonSetFighterAir` runs first. Ported in that order.
4. `ground_initial_` floor check (asm 1222-1227): `bltzl` sets the in-air flag when `floor_line_id < 0`. Ported as
   `floor_line_id < 0`.
5. `main_air_` (asm 1270-1351): the create branch uses the halfword test on bits 31-16 (`lhu 0x0184`). Ported as
   `& 0xFFFF0000`. The movement path is skipped when temp variable 1 (0x017C) is zero, and the animation-end check
   runs on both paths (`_check_end`).
6. `air_physics_` (asm 1376-1392): player air control is applied only when the movement flag is set. Otherwise no
   physics call is made (not the default air physics). Ported as-is.
7. The status id is `0xE4`, named `nFTRemixSonicStatusSpring`. `air_initial_` has the comment `NSP_Air_B`, but
   `Sonic.asm` names 0xE4 `Action.Spring`. Confirm the action array entry for 0xE4 in the port.
8. `SonicUSPRefresh` and `action_check_patch_` are not in the worklist. Both are described above. The up special bool
   clears on hit (`SonicUSPRefresh`), and the vanilla patch blocks special moves while it is set.

## Spring (projectile port)

Ported in `decomp/src/ft/ftremix/sonic/ftremixsonicusp.c`, asm `SonicSpecial.asm` scope SonicUSP:
`spring_stage_setting_` (1456-1549), `spring_main_` (1552-1659), `check_spring_bounce_` (1665-1857),
the descriptor `spring_projectile_struct` (1999-2012) and `spring_properties_struct` (2016-2029).
The Remix action table (`_check_action`, `_change_action`, `_draw_smoke_gfx`, `_set_y_velocity`,
`_end_rumble`, `_next`) is ported as `ftRemixSonicUSPSpringBounceKind` and its helpers.

This supersedes the stand-in section above. `SONIC_USP_SPRING_OBJECT_SLOT` is removed, and flag2's
high halfword is no longer written by the port.

### Port decisions

- The spring GObj lives in `sFTRemixSonicUSPSprings[player].gobj`. It is NULL after `air_initial_`, which
  matches Remix's `sw` of 1 to temp variable 3 there. `main_air_` tests the pointer where Remix tests
  `lhu 0x0184`, so the created state is the same.
- Remix keeps the spring GObj's enable flag (0x40) and FGM id (0x44) on the GObj. They are in the same
  per-player entry (`is_bounce_enabled`, `fgm_id`).
- Remix uses a pointer that may be stale after the spring is destroyed. The port checks liveness by walking
  `gGCCommonLinks[nGCCommonLinkIDWeapon]` before any dereference (`ftRemixSonicUSPSpringIsAlive`). This avoids a
  use-after-free when a hit or wall destroys the spring.
- Flag2 low halfword is unchanged: byte 0x186 is the saved floor line ID and byte 0x187 the in-air flag, written by
  `ground_initial_` and read by spring creation.
- Descriptor: `kind = 0x20` (`Projectile.id.SONIC_SPRING`). `wp->kind` is only stored (`wpmanager.c:127`).
  No table in `decomp/src/wp` is indexed by it. `p_weapon = fp->data->p_file_special1` (file 6).
  `o_attributes = 0`, `flags = 0`. Procs: update `ftRemixSonicUSPSpringMain`, map and hit `wpSamusBombProcMap`
  (0x80169108), setoff `wpFoxBlasterProcReflector` (0x80168A14), the rest NULL. The asm labels the second
  proc as "clang"; the decomp name is kept.
- Spawn flags: `WEAPON_FLAG_COLLPROJECT` (0x80000000 in asm), parent fighter.
- Spawn position: `gmCollisionGetFighterPartsWorldPosition(fp->joints[4], pos)`, then y - 228.0F, z = 0.
  N64 `0x08F8` is `joints[4]`.
- Spring WPStruct: `lifetime` = 240 (asm 0x0268), `attack_coll.attack_count` = hitbox (asm 0x0150),
  `coll_data.floor_line_id` (asm 0x00A0), `ga` (asm 0x00FC).
- The DObj rotation is `rotate.vec.f.x` (asm 0x0030). Image index is `mobj->texture_id_curr` and palette is
  `mobj->palette_id` (asm 0x0080 / 0x0088 of the DObj's `mobj`).
- Bounce velocity is `physics.vel_air.y` = 130.0F (asm 0x004C). Knockback Y is `physics.vel_damage_air.y`
  (asm 0x0058). Fast-fall byte 0x018D clears absorb, absorb_lr, goto_attack100 and fastfall through
  `ftRemixSonicUSPClearAirFlags`, which `air_initial_` now also uses.
- Rumble: `gmRumbleSetPlayerRumbleParams(port, 0, 5)` for `pkind == nFTPlayerKindMan` (asm `lbu 0x0023`,
  which is the low byte of the `pkind` s32 at 0x0020).

### REMIX-TODO markers in the file

- `REMIX-TODO(classic)`: `Sonic.classic_table` selects the classic spring image index (3), the palette and FGM
  0x3DF. The table is `sFTRemixSonicDSPClassicTable` in `ftremixsonicdsp.c`, which is static. The port uses the
  normal variant only.
- `REMIX-TODO(psi magnet)`: the Ness, JNess and Lucas magnet actions clear the Remix psi magnet overlay (asm
  `_psi_magnet`: FTStruct 0x0A20, 0x0A24, 0x0A28, 0x0A30, 0x018C and 0x0A88). Those offsets are vanilla fields in
  the port (`p_loop_sfx`, `loop_sfx_id`, `colanim`, ...), so the clear is skipped.

### Vanilla patch: `change_spring_room_` (asm 1431-1449), not applied

- RAM 0x80165C2C, inside `wpManagerMakeWeapon` (`decomp/src/wp/wpmanager.c:94`). That function is not listed in
  `vanilla_funcs.txt`, so the address was matched by hand. The patch replaces the room constant in the
  `gcAddGObjDisplay` call. Vanilla passes `14` (0x0E). The Remix code passes 0x0B for the spring. The
  `lw a0, 0x007C(sp)` original line is the weapon GObj.
- Exact C to insert (in `wpmanager.c`, `#ifdef REGION_REMIX`, replacing the literal `14`):

  ```c
  #ifdef REGION_REMIX
  s32 ftRemixSonicUSPSpringDisplayRoom(GObj *weapon_gobj);
  #endif
  ...
  #ifdef REGION_REMIX
      gcAddGObjDisplay(weapon_gobj, proc_display, ftRemixSonicUSPSpringDisplayRoom(weapon_gobj), GOBJ_PRIORITY_DEFAULT, ~0);
  #else
      gcAddGObjDisplay(weapon_gobj, proc_display, 14, GOBJ_PRIORITY_DEFAULT, ~0);
  #endif
  ```

- Helper, to be added non-static to `ftremixsonicusp.c` when the patch is applied:

  ```c
  s32 ftRemixSonicUSPSpringDisplayRoom(GObj *weapon_gobj)
  {
      WPStruct *wp = wpGetStruct(weapon_gobj);

      if (wp->kind == SONIC_USP_SPRING_WPKIND)
      {
          return 0x0B;
      }
      return 0x0E;
  }
  ```

  Asm: `beql a2, a3` with a3 = 0x20 selects room 0x0B; otherwise 0x0E.

### Uncertainties (asm line numbers in SonicSpecial.asm)

1. Remix's check reads 0x0090 as Y velocity. The port's FTStruct field at that offset is `coll_data.pos_diff.y`
   (struct_offsets). Confirm that this is the same value that Remix's Y velocity holds.
2. Two overlapping springs for the same player. Remix keeps each spring's flags on its own GObj. The port keeps
   one entry per player, so a new USP (which resets the pointer in `air_initial_`) disables bounce on the old spring.
   Remix does not do that. This follows the per-player design the brief asked for.
3. `_check_action` clears the spring's hit record flag byte (0xE0). The port sets the named hurt, shield and reflect
   bits and clears absorb and group_id. Remix also clears the top bit of timer_rehit in that byte, which the port
   does not do.
4. The Remix player-Z check in `check_spring_bounce_` tests the raw bits for zero. The port uses `!= 0.0F`, which
   differs only for -0.0.
5. `spring_stage_setting_` computes a float product that is never stored (the store is commented
   out in the asm). The port omits it.
6. `spring_properties` fields `bounce multiplier` (0x0010), `initial angle` and `min speed` are never read by the
   SonicUSP asm. They are not ported.
7. The file-9 graphic (`SONIC_SPRING_GRAPHIC`) is not referenced by the SonicUSP asm. It is not used here.
8. `change_spring_room_` is not applied (see above). Until it is, the spring is drawn in room 0x0E.

## Step 2 wave 2026-10-14 (partial — kimi quota window)

- LANDED (W3, partial): classic-mode table (ftRemixsonic.c sFTRemixSonicIsClassic + accessors), PSI magnet overlay clear, Spring bounce for Ness/Lucas. REMIX-TODO(Size) where applicable.
- Roy counter and Flashbang: NOT started (worker died on quota).
