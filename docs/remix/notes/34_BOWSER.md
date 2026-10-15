# 0x34 BOWSER notes (worker F5)

Source: `.toolchain/pkgs/smashremix-src/src/Bowser/BowserSpecial.asm`, plus
`bowser.asm` for the action ids and special tables, and
`Kirby/Kirby.asm` lines 195-196 for the Kirby ids.
C file: `decomp/src/ft/ftremix/bowser/ftremixbowser.c`
Table: `dFTRemixBowserProcs` / `dFTRemixBowserProcsNum` (6 entries).

## Ported (6 of 10 worklist routines)

| Remix label | N64 addr | C routine |
|---|---|---|
| BowserNSP.ground_initial_ | 0x8054D970 | ftRemixBowserNSPGroundSetStatus |
| BowserNSP.air_initial_ | 0x8054DA04 | ftRemixBowserNSPAirSetStatus |
| BowserUSP.air_initial_ | 0x8054E560 | ftRemixBowserUSPAirSetStatus |
| BowserUSP.ground_physics_ | 0x8054E5FC | ftRemixBowserUSPGroundProcPhysics |
| BowserUSP.air_physics_ | 0x8054E644 | ftRemixBowserUSPAirProcPhysics |
| BowserDSP.air_physics_ | 0x8054E73C | ftRemixBowserDSPAirProcPhysics |

## Not ported (4 of 10 worklist routines)

These are left out of `dFTRemixBowserProcs`, so they fall back to the parent
(Yoshi) routines. The brief asks for every worklist routine in the table, so
this is a deviation that needs a follow-up pass.

1. **BowserNSP.main_** (0x8054DAA0, asm 125-754). Its flame and projectile
   logic is built from `OS.copy_segment` fragments: raw bytes from
   `baserom.us.z64` at 0xC1D34, 0xC1D88, 0xC1DC4, 0xC1E08, 0xC1E24, 0xC1E40,
   0xC1E9C, 0xC1EB0, 0xC1ED8, 0xC1F80, 0xC1FDC, 0xC2004, 0xC203C, 0xC2050,
   0xC20E0, and in projectile spawn 0xF0988, 0xF098C, 0xF09C4, 0xF0A8C,
   0xF0AC4, 0xF0AF8. Many of them are delay-slot fragments of Fireflower code,
   so they only make sense in the context of the branch they came from. The
   routine also calls the shared subsystem `PokemonAnnouncer.flamethrower_announcement_`
   (asm 588), which is not ported. The idle transition (asm 720-733) passes
   the vanilla proc 0x800EEE54 to 0x800D9480 (`ftAnimEndCheckSetStatus`).
   Still to do: decode the copied blocks and reconstruct the branch structure.
2. **BowserNSP.air_collision_** (0x8054E390, asm 757-792). It passes
   `air_to_ground_` (Remix, not in this worklist) as the proc to
   `mpCommonProcFighterLanding` (0x800DE6E4). The normal landing proc
   0x800DE8E4 is used only when frame >= 20 and B is not held. Needs the
   `air_to_ground_` port first.
3. **BowserFThrow.main_** (0x8054E7F0, asm 1201-1213). It passes
   `transition_1_` (Remix helper, asm 1245-1253) to 0x800D9480. That helper is
   `OS.copy_segment(0xC4EFC, 0x3C)` with no explicit end, so it falls into
   following code. Not reconstructed.
4. **BowserFThrow.collision_** (0x8054E818, asm 1218-1239). It copies
   0xC4E94 (0x18 bytes, decoded as `addiu sp,-32; sw ra; lw t6,132(a0);
   sw a0,32(sp); jal 0x000DE6B0; sw t6,28(sp)`) and calls `transition_2_`
   (copy 0xC4FEC, 0x20). Not ported.

## Vanilla patches and hooks the scope relies on (not in this file)

- `BowserUSP.ledge_grab_patch_` (asm 1060-1082): `OS.patch_start(0xD96A4,
  0x8015EC64)`. It sets temp variable 2 (0x180) to 1 for Bowser and
  G-Bowser, then calls `jal 0x800DEEC8` (`mpCommonSetFighterAir`). Needs a patch in the vanilla
  file under `#ifdef REGION_REMIX`. Not in worklist.
- `BowserDSP.skip_projectile_` (asm 1157-1191): `OS.patch_start(0xD9864,
  0x8015EE24)`. It skips the vanilla call `0x8016C954` (`wpYoshiStarMakeStars`,
  from vanilla_funcs) for Bowser and G-Bowser. Not in worklist.
- `BowserUSP.air_direction_` (asm 1032-1056) is called from moveset command
  `58000002` and calls `0x80160370` (`ftCaptainSpecialHiProcInterrupt`). Not
  in worklist, not ported.

## Uncertainties (asm line numbers)

- Lines 969-971 (USP ground physics): `lui a3, 0x4230` sits in the `beqz`
  delay slot, so it runs in both cases. The fallthrough `lui a3, 0x4120`
  then overrides it when temp variable 2 is non-zero. The port maps
  flag1 == 0 to 44.0F and flag1 != 0 to 10.0F, which matches that behaviour.
- Lines 945-948 (USP air initial): the `beql` delay slot `lui v1,
  G_INITIAL_SPEED` executes only when the branch is taken, so the G-Bowser
  speed is 80.0F and the Bowser speed is 60.0F.
- Lines 1008-1014 (USP air physics): the `beql` delay slot `lui a3,
  G_MAX_X_SPEED` executes only for G-Bowser. Non-G-Bowser takes the
  `X_ACCELERATION` and `MAX_X_SPEED` lines. The port uses a branch on fkind.
- Lines 1102-1103 (DSP air physics): `mtc1 at, f4` sets f4 to 1.0 but nothing
  reads it, because the store to 0x48 is commented out (line 1106). Dropped
  as dead code.
- Lines 1104-1112 (DSP air physics): `joints[0]` is taken from 0x8E8, which
  `struct_offsets.txt` lists as `joints` (DObj *[37]). The asm writes 0x44 and
  0x40 from 0x48, so the port copies `scale.vec.f.z` into `.y` and `.x`.
- `ftMainPlayAnimEventsAll` is called with an extra `a1` in asm (lines 128-130
  for ground, 111-116 for air). The decomp function takes only the gobj, so
  the extra argument is ignored.
- `ftRemixBowserWordAt` reads and writes integers at the N64 FTStruct offsets
  0x0B18..0x0B30 through an s32 pointer. Those offsets are untyped scratch
  storage in the decomp (`status_vars` union). Strict-aliasing is a
  theoretical risk; the Remix code relies on the same storage.
- Kirby action ids 0x129 and 0x12C are the `Kirby.Action.BOWSER_NSP_Ground` and
  `_Air` values from `Kirby/Kirby.asm` lines 195-196.
- Bowser action ids come from `bowser.asm`: 0xDF = WhirlingFortressAir,
  0xE4 = FireBreath, 0xE7 = FireBreathAir.

## Second pass

C file: `decomp/src/ft/ftremix/bowser/ftremixbowser.c`. Table now has 9 entries.
Syntax check passes (`scripts/remix/syntax-check.sh`).

### Ported in this pass (3 of the 4 routines assigned)

| Remix label | N64 addr | C routine |
|---|---|---|
| BowserNSP.air_collision_ | 0x8054E390 | ftRemixBowserNSPAirProcMap |
| BowserFThrow.main_ | 0x8054E7F0 | ftRemixBowserFThrowProcUpdate |
| BowserFThrow.collision_ | 0x8054E818 | ftRemixBowserFThrowProcMap |

Static helpers (not in the table, only referenced as C pointers):

- `air_to_ground_` 0x8054E4F4 -> `ftRemixBowserNSPAirToGround`
- `BowserFThrow.transition_1_` 0x8054E86C -> `ftRemixBowserFThrowTransition1`
- `BowserFThrow.transition_2_` 0x8054E8BC -> `ftRemixBowserFThrowTransition2`

Vanilla calls used: `mpCommonCheckFighterLanding` (0x800DE6B0),
`mpCommonProcFighterLanding` (0x800DE6E4), `mpCommonSetFighterWaitOrLanding`
(0x800DE8E4), `mpCommonSetFighterGround` (0x800DEE98), `ftAnimEndCheckSetStatus`
(0x800D9480), `ftMainSetStatus` (0x800E6F24).

### Not ported

- **BowserNSP.main_** (0x8054DAA0) is still missing from the table. The assembled
  routine runs from 0x8054DAA0 to about 0x8054E390 (about 0x8F0 bytes). It also
  covers `BowserNSP_part2_` (0x8054DE18), `idletransition_` (0x8054E314),
  projectile spawn (0x8054E000) and the effect parameter blocks in Remix RAM
  (0x80198xxx). Those were not reconstructed in this pass. Its Bowser and Kirby
  status ids and the parent fallback stay as before.

### Corrections to the first pass

- Kirby fire breath action ids in the ROM are **0x13E** (grounded) and **0x13F**
  (aerial). The first pass used 0x129 and 0x12C from the `Kirby.asm` source
  lines 195-196, which the built ROM does not use. The enum now has 0x13E and
  0x13F. Found in `air_to_ground_` (0x8054E518-0x8054E52C) and in
  `ground_initial_` / `air_initial_` (`ori a1, zero, 0x13E` and `0x13F`).
- `transition_2_` passes flags 0x80 (`FTSTATUS_PRESERVE_TEXTUREPART`) in the ROM.
  The source has no flags for it.
- `air_to_ground_` passes `DObj(gobj->obj)->anim_speed` as the speed argument.
  The ROM loads `gobj+0x74` and then `+0x78`.

### Uncertainties

- `air_to_ground_` (source line 877-878) reads fkind through `s1`
  (`lw a2, 0x0008(s1)`), and nothing in the routine sets `s1`. The port reads
  `fp->fkind` from `ftGetStruct(gobj)`, which assumes `s1` holds the fighter
  struct at that point. This matches how the Kirby code uses `s1`.
- `transition_1_` clears bit 0x40 of the grabbed fighter's byte at 0x192 (`andi
  0xFFBF`, `sb`). That is `is_ignore_dead` (bit 1, 0x40) in `struct_offsets.txt`,
  so the port assigns `catch_fp->is_ignore_dead = FALSE`.
- `BowserFThrow.collision_` tests `fp->physics.vel_air.y < 0.0F` (ROM `lwc1 f4,
  0x4C`, `c.lt.s`, `bc1fl`).
- `BowserNSP.air_collision_` tests `anim_frame >= 20.0F` and `!(button_hold &
  CONT_B)` (ROM `andi 0x4000`). With frame < 20, or with B held, it lands into
  `air_to_ground_`. Otherwise it uses the normal landing.

## Third pass (BowserNSP.main_ attempt, not ported)

C file unchanged in this pass: `dFTRemixBowserProcs` still has 9 entries.
`BowserNSP.main_` (0x8054DAA0, 0x8F0 bytes) is still missing. Decoded with
`tools/remix_disasm_rom.py 0x2D4DAA0 0x8F0 --remix`. The routine reaches:

- `BowserNSP_part2_` (0x8054DE18, asm 390-511), the projectile spawn chain
  (`projectile_spawn` 0x8054E230 path and `gbowser_projectile_spawn`), and
  `hitbox_generation` (0x8054E0E0, asm 582-618).
- `idletransition_` (0x8054E314): `jal 0x800D9480` with `a1 = 0x800EEE54`
  (`ftAnimEndCheckSetStatus`). Small enough to port once the main routine lands.
- `func_8054E000` (asm 530-575, the ammo-subtract tail of projectile spawn),
  which calls `0x800EDF24` (`gmCollisionGetFighterPartsWorldPosition`).

### Blockers (why no code was written)

1. **Flame weapon descriptor at RAM 0x80199D34.** `hitbox_generation` builds it
   with `lui a1,0x8019; addiu a1,a1,-0x62CC` and passes it to
   `wpManagerMakeWeapon` (0x801655C8) with `a2 = spawn pos`, `a3 = 0x80000000`
   (WEAPON_FLAG_COLLPROJECT). Its contents (kind, `p_weapon` file, proc
   pointers) are not in `decomp/src`. Reading `baserom.us.z64` at
   `0x80199D34 - 0x80084800` gives code, so the fighter-segment mapping does
   not apply to it. The descriptor must be located in the Remix image before
   the WPDesc can be written as static const data.
2. **Remix RAM data at 0x8019xxxx.** The routine reads fixed RAM tables that
   are not in any source checked: 0x80198678 (flame origin, via `0x8019 + -0x7988`),
   0x80198684 and 0x80198690 (smoke effect params, the ammo-check branches),
   0x8019869C, 0x801986A8, 0x801986B4 (effect parameter blocks, copied with
   `lw/sw` into the stack), and the pointer at `-0x639C` in 0x80198019 used by
   `projectile_spawn` (`lw t0, -0x639C($t0)` then `0x360` offset for the
   direction float table at `t0 + 0x360 + 4*a2`).
3. **Shared subsystem.** `PokemonAnnouncer.flamethrower_announcement_` (asm 588,
   reached through `hitbox_generation` at RAM 0x80510890) is not ported. It
   needs a `/* REMIX-TODO(PokemonAnnouncer): ... */` marker when the routine is written.
4. **Flame weapon procs.** The WPDesc proc pointers (`proc_update`, `proc_map`,
   `proc_hit`, and the rest) cannot be named until blocker 1 is resolved.
5. **Copy-segment fragments.** About 25 fragments (ROM 0xC1D34..0xC2050,
   0xC1E08..0xC1FDC, 0xF0988..0xF0AF8) are real vanilla instructions. The
   decoded forms are in the disassembly, so they can be ported as C once the
   data tables above are known. The rest of the routine (ammo logic, button
   checks, animation hooks) is decodable from the disassembly.

### Suggested next step

Find the Remix RAM initialisation for 0x80198xxx and 0x80199D34 (likely a Remix
copy from the Remix ROM at boot). Once the bytes are located, the main routine
can be ported in one pass, with the fragments written as explicit C.

## Third pass, follow-up: BowserNSP.main_ ported

Supersedes the blocker list above. C file: `decomp/src/ft/ftremix/bowser/ftremixbowser.c`.
Table `dFTRemixBowserProcs` now has 10 entries, with `BowserNSP.main_` (0x8054DAA0)
mapped to `ftRemixBowserNSPProcUpdate`. `syntax-check.sh` prints `syntax-check OK`.

### Sources used (decomp, not rebuilt)

- Flame weapon: `itFFlowerWeaponFlameMakeWeapon` (`it/itcommon/itfflower.c`). Its
  descriptor is `dITFFlowerWeaponFlameWeaponDesc`, at RAM 0x80199D34 in the ROM.
  The Remix hitbox calls `wpManagerMakeWeapon` with flags 0x80000000, the same as
  the decomp call, because `WEAPON_FLAG_PARENT_FIGHTER` is 0.
- Angle table: the same expression as `itFFlowerShootFlame`
  (`*dITFFlowerItemDesc.p_file + llITCommonDataFFlowerFlameAngles`). The ROM reads
  `*(0x80199C64)` and then `+0x360`, which is the same chain.
- Smoke and origin offsets: the vanilla Fire Flower values in
  `ft/ftcommon/ftcommonitemshoot.c` (make_flame_offset {60,100,0}, haveammo {0,0,-180},
  dust {0,0,-180}). The Remix ROM replaces the origin z with 120 (Bowser) or -20 (Kirby).
- Effect kinds: 0xB = `nEFKindDustLight`, 0x13 = `nEFKindDustDashSmall` (checked
  against `ef/efdef.h`).
- Motion id 0x36 = `nFTMotionAttackIDFireFlowerShoot` (54).
- Callees: `ftParamMakeEffect` (0x800EABDC), `ftParamMakeRumble` (0x800E806C),
  `gcSetAnimSpeed` (0x8000BB04), `gmCollisionGetFighterPartsWorldPosition` (0x800EDF24),
  `ftAnimEndCheckSetStatus` (0x800D9480), `__cosf` (0x80035CD0), `__sinf` (0x800303F0).

### Correction to the first pass

The idle proc passed to `ftAnimEndCheckSetStatus` is **0x800DEE54**
(`mpCommonSetFighterWaitOrFall`), not 0x800EEE54. The asm uses `lui 0x800E` with
`addiu -0x11AC`, which is 0x800DEE54.

### Approximations and uncertainties

1. `BowserNSP.main_` reads the frame count for the first-frame flag from
   `lw t7, 0x0B24(sp)`, which is a stack slot far outside the frame (probably a typo
   for `0x0B24(s0)`). The port uses the FTStruct word 0x0B24 instead, as the source
   comment states.
2. The first-frame sparkle (`0x1F`, SparkleWhiteScale) is set and then overwritten
   by `0x13` (DustDashSmall) before the only `jal`, so the ROM never plays the sparkle.
   The port plays only the dust.
3. FGM ids 0x30 and 0x1A are Remix literals. They do not match the vanilla
   `nSYAudioFGM*` enum (index 26 is `ItemMapCollide`, index 48 `SwordSwing3`), so the
   Remix sound table is used as written.
4. Ammo halfwords: Bowser's ammo is stored in the word at 0x0ADC (passive_vars) and
   Kirby's in the word at 0x0AE0 (passive_vars.kirby.copysamus_charge_level at the same
   place). Those are only used by Kirby's own copy of this move, so no conflict is expected.
5. The flame joints are numeric indices: `fp->joints[7]` for Bowser and
   `fp->joints[13]` for Kirby, as the ROM uses them.
6. `PokemonAnnouncer.flamethrower_announcement_` (RAM 0x80510890) is skipped and marked
   `REMIX-TODO(PokemonAnnouncer)` in `ftRemixBowserNSPMakeFlame`.
7. The `ftRemixBowserNSPSpawnFlame` index mirrors past 5 (`8 - index`), as the
   ROM does. The G-Bowser sine argument is -0.5 (0xBF000000).

### Vanilla patches the routine relies on (not in this file)

- `BowserUSP.ledge_grab_patch_` and `BowserDSP.skip_projectile_` from the first pass
  are still outstanding. Nothing new is needed for `main_`.
