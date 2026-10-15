# 0x39 MTWO, MewtwoDSP notes

Port: `decomp/src/ft/ftremix/mewtwo/ftremixmewtwodsp.c`, table `dFTRemixMewtwoDSPProcs` (4 entries, `dFTRemixMewtwoDSPProcsNum`).
Source: `.toolchain/pkgs/smashremix-src/src/Mewtwo/MewtwoSpecial.asm` lines 2021-2137.

## 1. Vanilla patches

None. No `OS.patch_start` or `j`/`jal` hook in MewtwoDSP points into vanilla code that the port must replace. The action bindings (`Character.edit_action(MTWO, 0xE5/0xE6, ...)` in `Mewtwo.asm` lines 299-302) and the special entry table patches (`Mewtwo.asm` lines 347, 350) are handled by the generated Remix binding, not by a vanilla edit.

## 2. Shared Remix subsystems

- `PokemonAnnouncer.disable_announcement_` (`PokemonAnnouncer.asm:799`), called first by `ground_initial_` (`MewtwoSpecial.asm:2027`) and `air_initial_` (`MewtwoSpecial.asm:2047`). Not ported. Marked with `REMIX-TODO(PokemonAnnouncer)` in both routines. Skipped behaviour: it calls `toggle_announcer_` (`PokemonAnnouncer.asm:2003`), which reads `Toggles.entry_announcer_mode`, `Global.match_info` and the stage id. When the announcer is on, it calls `announcer_main_` with `disable_call` and, if that does not handle the call, plays FGM `0x800269C0` (`func_800269C0_275C0`) with the value left in `v1`. The `v1` input is not set in the disable routine itself, so the FGM number is not known from this scope. The port skips the whole routine, so the disable callout and its FGM never play for Mewtwo DSP.

## 3. Uncertainties

- `MewtwoSpecial.asm:2130`, `air_transition_`: the Remix comment says "momentum capture?". Vanilla `0x800D8EB8` maps to `ftPhysicsClampAirVelXMax` (`decomp/src/ft/ftphysics.c:283`, which calls `ftPhysicsClampAirVelX(fp, attr->air_speed_max_x)`). The port calls `ftPhysicsClampAirVelXMax(fp)` as the vanilla mapping says. The Remix comment and the vanilla name disagree, so check this in game.
- `MewtwoSpecial.asm:2107` and `2128`: argument 4 (`flags`) is `1` for the transitions and `0` for the initial specials. The port uses `FTSTATUS_PRESERVE_HIT` (bit 0, `decomp/src/ft/ftdef.h:40`, `nFTStatusPreserveHit` is the first enum entry) and `FTSTATUS_PRESERVE_NONE`. This matches the asm bit value.
- `MewtwoSpecial.asm:2058`: `sw r0, 0x004C(a0)` is `physics.vel_air.y = 0`, from `struct_offsets.txt` (FTStruct 0x004C). Set after `ftMainPlayAnimEventsAll`, as in the asm.
- `MewtwoSpecial.asm:2103`, `2124`: `lw a2, 0x0078(a0)` reads GObj `anim_frame` (GObj 0x78) because a0 is the GObj at that point (the `lw a0, 0x0084` in the delay slot only feeds the call). The port uses `fighter_gobj->anim_frame`.
- `MewtwoSpecial.asm:2032`, `2052`, `2106`, `2127`: the asm comments call `0x800E0830` "unknown common subroutine" in the initial specials. `build/remixasm/refs/vanilla_funcs.txt` maps it to `ftMainPlayAnimEventsAll`, so the port uses that. The transitions call `0x800E6F24`, which maps to `ftMainSetStatus`.

## 4. Integration note (not changed here)

- `decomp/src/ft/ftremix/ftremixported.c` line 50 lists `dFTRemixMewtwoProcs` (defined in `decomp/src/ft/ftremix/mewtwo/ftremixmewtwo.c`) as the only Mewtwo table. The DSP table `dFTRemixMewtwoDSPProcs` is not registered there. The integrator must add it to `dFTRemixPortedProcTables` or merge it into the Mewtwo table, because the file contract says one table per file.
- Helpers `ftRemixMewtwoDSPGroundTransition` and `ftRemixMewtwoDSPAirTransition` are `static` and are not in the table, because the worklist does not list them. They are reached only from the collision routines through function pointers.
