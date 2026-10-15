# 0x44 BANJO NSP: notes for the port in `decomp/src/ft/ftremix/banjo/ftremixbanjonsp.c`

Source: `.toolchain/pkgs/smashremix-src/src/Banjo/BanjoSpecial.asm`, `scope BanjoNSP` (lines 1 to about 1040).
Table: `dFTRemixBanjoNSPProcs` (12 entries, all worklist routines) and `dFTRemixBanjoNSPProcsNum`.

## 1. Vanilla patches and hooks

No `OS.patch_start` blocks and no `j`/`jal` hooks into vanilla code are needed by this scope. Every call from the
NSP routines to vanilla code is a plain `jal`, which the C calls already express:

- `0x800E6F24` ftMainSetStatus, `0x800E0830` ftMainPlayAnimEventsAll, `0x800DEE54` mpCommonSetFighterWaitOrFall
- `0x800DEE98` mpCommonSetFighterGround, `0x800DEEC8` mpCommonSetFighterAir, `0x800D8EB8` ftPhysicsClampAirVelXMax
- `0x800DDE84` mpCommonProcFighterOnEdge, `0x800DE6E4` mpCommonProcFighterLanding
- `0x800DE80C` mpCommonProcFighterCliff (see section 3, item 6)
- `0x800D90E0` ftPhysicsApplyAirVelDrift, `0x800D91EC` ftPhysicsApplyAirVelFriction
- `0x800EDF24` gmCollisionGetFighterPartsWorldPosition, `0x801655C8` wpManagerMakeWeapon, `0x80167FA0` wpMainVelSetModelPitch
- `0x80167FE8` wpMainDecLifeCheckExpire, `0x80168088` wpMainApplyGravityClampTVel, `0x800FF648` efManagerDustExpandSmallMakeEffect
- `0x80035CD0` / `0x800303F0` cos / sin (`__cosf` / `__sinf` in the port, as in the Conker nut)

The Remix action table entries in `Banjo.asm` (lines 498 to 503) and the Kirby entries in `Kirby.asm` (lines 445 to
450) pass these routines as action parameters. Those tables belong to the table-binding side, not to this file.

## 2. Shared Remix subsystems

- Custom items (`Item.*.SPAWN_ITEM`): not used by the NSP routines.
- Custom GFX, `PokemonAnnouncer`, `Size` and similar: not used by the NSP routines.
- `REMIX-TODO` markers: none in this file.
- Helpers ported as `static`: the status setters, the transitions, the copied-segment initials, the egg spawn,
  update and destruction routines, and the stick-direction helper.

## 3. Uncertainties (asm line numbers refer to BanjoSpecial.asm)

1. **Egg lifetime offset (asm lines 522 and 609; expiry calls at 870 and 917).** The asm stores the duration (30
   forward, 120 backward) at WPStruct 0x268. The decomp names 0x264 as `lifetime`, the field that
   `wpMainDecLifeCheckExpire` (the asm's `jal 0x80167FE8`) decrements, and it names 0x268 as the `is_camera_follow` and `is_static_damage` bits. The
   port writes `wp->lifetime`, which follows the expiry check. If the decomp offsets are right, the asm's own write is
   to the bits, and the expiry then depends on whatever `wpManagerMakeWeapon` leaves in 0x264. This needs an
   on-hardware or GBI check. The same question applies to the Conker nut, whose port skips the write.
2. **Clear of N64 0x29C (asm lines 524 and 603).** The asm zeroes the weapon_vars word the Mario fireball uses. The port
   writes it through `weapon_vars.charge_shot.is_full_charge`, because the union has no generic name for that word.
3. **Egg angles (asm lines 1013 and 1014, backward egg props).** The backward egg's angle fields are the raw float32
   128, which the asm passes to cos and sin unchanged. The port keeps 128.0F as radians, which is probably unintended
   in Remix but is faithful to the data.
4. **Unused egg fields.** The min speed and bounce multiplier fields in both egg property structs are not read by the
   egg routines in this file. The hop proc (`wpBossBulletProcHop`) may read them. It is not checked here.
5. **Temp variable 1 reset in `shoot_*_main_` (asm lines 435 and 486, animation-end delay slot).** The asm
   clears flag0 through `v0`. `v0` may have been clobbered by the egg spawn call
   before it, so the asm behaviour there is not reliable. The port clears `fp->motion_vars.flags.flag0` directly.
6. **`0x800DE80C` name (asm lines 745 and 758).** The decomp has no address comment for it. `mpCommonProcFighterCliff` (mpcommon.c line 640)
   sits between `mpCommonCheckFighterCliff` (0x800DE7D8) and `mpCommonCheckFighterCeilHeavyCliff` (0x800DE87C), and
   its comment says "allow CliffCatch interrupt", which matches the asm's "transition on landing, allow ledge grab".
   The port uses it on that basis.
7. **Physics selection in `air_shoot_physics_` (asm line 686 onward).** `lui t8, 0x800E` plus `addiu 0x90E0` sign-extends
   to 0x800D90E0 (drift) and `addiu 0x91EC` to 0x800D91EC (friction). The port selects drift when temp variable 2 is
   non-zero, as the `bnezl` does.
8. **Kirby ids.** Kirby's action ids are `0x12F + index`, where index is the position of the call among Kirby's
   `add_new_action` calls (Kirby.asm lines 337 to 450). The BANJO_NSP* entries are indices 108 to 113, giving
   0x19B to 0x1A0. The same formula reproduces Conker (0x144 for index 21) and Marth (0x150 for index 33) in the
   existing ports.
9. **Egg hop and reflector procs.** Both eggs use `0x8016DD2C` (`wpBossBulletProcHop`) as proc_hop and
   `0x801692C4` (`wpSamusBombProcReflector`) as proc_reflector. The forward egg's proc_map is `0x80175914`
   (`itLGunWeaponAmmoProcMap`), and the backward egg's is `0x801685F0` (`wpMarioFireballProcMap`).
10. **Copied vanilla segments.** `ground_shoot_*_initial_` (`OS.copy_segment(0xD84A0, 0x14)`) sets argument 4 to 2
    and stores the player struct. The air shoot initials (`0xD84E8`, 0x3C) also convert a grounded fighter to
    aerial, which the port does with `ga == nMPKineticsGround`. The transitions (`0xD83F0` and `0xD8444`, 0x20 each)
    call `mpCommonSetFighterGround` and `mpCommonSetFighterAir`. The port replaces each copy with the equivalent C,
    and the disassembly above is the basis for those statements.
11. **Flag value 2.** The asm passes 2 as the fifth argument to `ftMainSetStatus` in every transition and in the
    copied shoot initials. The port uses `FTSTATUS_PRESERVE_COLANIM`, which is `1 << 1`.
12. **Egg spawn offsets.** The forward egg starts at offset (-40, 0 for Banjo or -160 otherwise, 0) and the backward
    egg at (0, 0, 0), both passed to `gmCollisionGetFighterPartsWorldPosition` on joint 21 (N64 0x093C). The routine
    treats its vector as an offset in the part's space, as the asm implies, but this is not verified.
13. **Begin proc at animation end.** `begin_main_` and the shoot end checks test `anim_frame <= 0` (the asm's
    `c.le.s`). The port follows this, and the sign convention of `anim_frame` is not otherwise verified.
