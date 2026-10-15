# 0x48 CRASH: CrashDSP (Dig) port notes

Source: `.toolchain/pkgs/smashremix-src/src/Crash/CrashSpecial.asm` (scope CrashDSP, lines 1307-2200).
Port: `decomp/src/ft/ftremix/crash/ftremixcrashdsp.c`, table `dFTRemixCrashDSPProcs` / `dFTRemixCrashDSPProcsNum`.
Coverage: 11 of 11 worklist routines are in the table. CrashNSP and CrashUSP were not touched.

## 1. Vanilla patches

### dig_ecb_patch_ (RAM 0x800E6F4C, hook at 0x800E6F4C via OS.patch_start(0x6274C))

Not ported in the C file. The hook lives in `ftMainSetStatus` (`decomp/src/ft/ftmain.c`, the block that calls
`port_fighter_ecb_override`). Crash needs a registered `ecb_override` on its fighter descriptor. The callback
type is `PortFTECBOverrideFn` (`port/fighter_registry.h:46`):

    int (*)(struct FTStruct *fp, int next_status_id, float *out_upper, float *out_middle)

Profile to register for fkind REMIX_ID_CRASH (0x48), from `dig_ecb_patch_` lines 2152-2194:

- next status 0xE6 (DSPWait) or 0xE7 (DSPTurn): `*out_upper = 150.0F; *out_middle = 80.0F;` (0x4316 / 0x42A0), return 1.
- any other next status: `*out_upper = 320.0F; *out_middle = 190.0F;` (0x43A0 / 0x433E), return 1.
- The callback writes `attr->map_coll.top` and `.center`, which is what the asm writes at attr+0x9C and attr+0xA0.
- The patch runs only for Crash, so the callback must return 0 for other fkinds. The registry already does this per fkind.

Registration point is the Crash fighter descriptor (not in this file). Integrator action: set `ecb_override` there.

### Hooks into vanilla code (the DSP entries)

- The DSP action table entries in `Crash.asm` (lines 618-623) point the main/physics/collision slots at CrashDSP routines.
  Those are covered by the table in the port, not by a patch.
- `Character.table_patch_start(air_dsp, ...)` and `ground_dsp` (Crash.asm lines 651-658) point the special entries at
  `CrashDSP.initial_` and `CrashDSP.dive_air_initial_`. The port table exports both.

## 2. Shared Remix subsystems (REMIX-TODO markers)

| Subsystem | Asm location | Port behaviour |
|---|---|---|
| PokemonAnnouncer.burrow_announcement_ | line 1369 (begin_initial_) | skipped, REMIX-TODO(PokemonAnnouncer) |
| PokemonAnnouncer.dig_announcement_ | line 1497 (end_initial_) | skipped, REMIX-TODO(PokemonAnnouncer) |
| Vanilla joint track routine 0x8000BD1C ("apply animation track to joint") | lines 1454 (wait_initial_), 1483 (turn_intial_), 1530 (end_initial_) | not called. The routine has no name in `build/remixasm/refs/vanilla_funcs.txt` or the decomp. Its track data is the Remix file `track_dig_wait.bin`, `track_dig_turn.bin` and `track_dig_end.bin` (in `Crash/`). REMIX-TODO(animation) |
| Dig GFX GObj position copy (topjoint translate to GObj +0x4C/+0x50/+0x54) | end_initial_ | skipped, REMIX-TODO(dig gfx). These offsets fall inside GObj.gobjscripts, which the decomp does not name. |
| dig_gfx_main_ (RAM 0x805811C4) | the attached Dig GFX's per-frame routine | not ported. It is not in this worklist and no CrashDSP routine calls it, but the visible dig effect depends on it. It also calls PokemonAnnouncer-free helpers (0x800FF3F4, 0x800FD4F8, 0x80009A84, Size.multiplier_table). Needs its own pass, probably under the Crash GFX worklist. |

Rumble: the asm calls `Global.rumble_` (RAM 0x80115530, `Global.asm:227`). The port calls
`gmRumbleSetPlayerRumbleParams(fp->player, 0, 30)`. That mapping is an assumption, see section 4.

## 3. Interface notes for other workers

- Remix status ids in the port are file-local: DSPBegin 0xE5, DSPWait 0xE6, DSPTurn 0xE7, DSPEnd 0xE8, DSPDive 0xE9, DSPAirDive 0xEA.
- `ftRemixCrashDSPSlideOffEndInitial` and `ftRemixCrashDSPBeginInitialFromDive` are static. They are passed as function pointers to `mpCommonProcFighterOnFloor` and `mpCommonProcFighterLanding` and do not need exporting.
- Dig state is kept in the parent's status_vars union at 0x0B18/0x0B1C/0x0B20 (overlay `FTRemixCrashDSPStatusVars`).
  The Dig GFX pointer is 8 bytes on LP64, so the overlay assumes status_vars has room after 0x0B18.
  NSP and USP must not use status_vars slots 0x0B18-0x0B28 while Dig is active.
  Both are Crash-specific, so this only matters if they share the union.
- Table registration: the integrator must add this table to `dFTRemixPortedProcTables` (`ftremixported.c`) for Crash,
  combined with the NSP/USP tables from other workers, or the 11 routines are never bound.

## 4. Uncertainties (asm line numbers)

1. **DSP action ids 0xE5-0xEA.** The values are inferred from the Crash action string table (lines 56-61: `DigginItBegin`..`DigginItAirDive` at 0xE5..0xEA). The `constant DSPBegin(...)` declarations are not in the scope I read, so these are unconfirmed by a direct constant.
2. **Global.rumble_ vs gmRumbleSetPlayerRumbleParams** (line 2076). `Global.asm:227` documents `rumble_` as (port, rumble_id, duration), and rumble ids 0x2, 0x3 and 0x7 are disabled on action change. The decomp equivalent is assumed, and the action-change side effect is not replicated. `ftParamMakeRumble` was not used because it is gated on `pkind == Man`, which the asm does not check.
3. **dig_update_ zero test** (scope at line 2030). The asm tests the raw bits of `x velocity` (`beqz at`). The port uses `== 0.0F`, which differs only for -0.0.
4. **dive_physics_ kinetic state** (scope at line 1861). The asm branches when kinetic state (0x14C) is zero. The port compares `ga != nMPKineticsGround`, which assumes `nMPKineticsGround == 0`.
5. **EFDesc tail.** `dig_gfx_struct` (RAM 0x805812E8) is two words in the Remix ROM. The C `sFTRemixCrashDigGfxDesc` sets flags and dl_link, sets file_head at runtime, and zeros the rest. The engine must not read the fields past file_head for this effect.
6. **Dive air friction.** `air_dive_friction_` (line 1905) passes a stack attribute whose only meaningful slot is air_friction (0x54). The port copies `*fp->attr` and overrides `air_friction`. The copy is a full FTAttributes per call. This is equivalent because `ftPhysicsApplyAirVelXFriction` only reads air_friction.
7. **dive_air_physics_ fast-fall branch.** The copied 0x800D90E0 code (lines 1880-1902) has a fast fall branch in the seven skipped lines. The port applies dive gravity unconditionally, as the asm does, and does not replicate the skipped fast fall logic.
8. **Crash `floor_flags & 0x4000`** (initial_, lines 1323-1335). The asm comment says "clipping id". The port names it `coll_data.floor_flags`, which the offset table gives at 0xF4, with the drop-through bit as a literal. The decomp has no named constant for it.
9. **Unnamed topjoint copy at wait/end.** The rotate.vec.f.y copies from the topjoint to the Dig GFX root DObj are ported, and the offsets are named in the decomp.

## 5. Verification

- `scripts/remix/syntax-check.sh decomp/src/ft/ftremix/crash/ftremixcrashdsp.c` prints `syntax-check OK` with no warnings.
- Not run: cmake, make, ninja, the game, the full build, and no commits.
