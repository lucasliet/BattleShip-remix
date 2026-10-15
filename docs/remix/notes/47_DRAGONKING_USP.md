# 47 DRAGONKING USP: notes for the up special port

Port: `decomp/src/ft/ftremix/dragonking/ftremixdragonkingusp.c`
Table: `dFTRemixDragonKingUSPProcs` / `dFTRemixDragonKingUSPProcsNum` (7 routines, all in scope).
Source: `.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm` lines 5-414 (scope DragonKingUSP) and `DragonKing/DragonKing.asm` lines 235-259 (action params and `Character.add_new_action`).
Action ids: USPGround 0xEF, USPGroundThrow 0xF0, USPAir 0xF1, USPAirPull 0xF2, USPLandingThrow 0xF3 (see uncertainties).

## Routines in the table

| Remix label | Port | Asm line |
|---|---|---|
| `air_initial_` | `ftRemixDragonKingUSPAirInitial` | 43 |
| `air_pull_collision_` | `ftRemixDragonKingUSPAirPullCollision` | 353 |
| `air_pull_main_` | `ftRemixDragonKingUSPAirPullMain` | 233 |
| `air_pull_physics_` | `ftRemixDragonKingUSPAirPullPhysics` | 274 |
| `ground_collision_` | `ftRemixDragonKingUSPGroundCollision` | 311 |
| `ground_initial_` | `ftRemixDragonKingUSPGroundInitial` | 14 |
| `throw_main_` | `ftRemixDragonKingUSPThrowMain` | 190 |

Static helpers, not in the table: `ground_throw_initial_` as `ftRemixDragonKingUSPGroundThrowInitial`, `air_pull_initial_` as `ftRemixDragonKingUSPAirPullInitial`, `landing_throw_initial_` as `ftRemixDragonKingUSPLandingThrowInitial`, `ground_to_air_` as `ftRemixDragonKingUSPGroundToAir`, `throw_setup_` as `ftRemixDragonKingUSPThrowSetup`, `air_pull_friction_` as `ftRemixDragonKingUSPAirPullFriction`, plus the drain `ftRemixDragonKingUSPDrain`.

Exported helper (not a table entry), for the vanilla hook in the next section:

```c
sb32 ftRemixDragonKingUSPIsCaptureFlashOverride(GObj *player_gobj);
```

## Vanilla calls resolved

| Address | Decomp function | Used by |
|---|---|---|
| `0x800E6F24` | `ftMainSetStatus` | all status changes |
| `0x800E0830` | `ftMainPlayAnimEventsAll` | initial routines |
| `0x8015E310` | `ftYoshiSpecialNSetCatchParams` | ground_initial_, air_initial_, ground_to_air_ |
| `0x800E8098` | `ftParamSetCaptureImmuneMask` | throw_setup_ (mask 0x3F) |
| `0x8014A0C0` | `ftCommonThrowProcUpdate` | throw_main_ |
| `0x800EA248` | `ftParamUpdateDamage` | drain |
| `0x800DDDDC` | `mpCommonProcFighterOnFloor` | ground_collision_ |
| `0x800DE6E4` | `mpCommonProcFighterLanding` | air_pull_collision_ |
| `0x800DEE98` | `mpCommonSetFighterGround` | landing_throw_initial_ |
| `0x800DEEC8` | `mpCommonSetFighterAir` | ground_to_air_ |
| `0x801605FC` | `ftCaptainSpecialHiProcStatus` | proc_status in air_initial_ and ground_to_air_ |
| `0x800D8D68` | `ftPhysicsApplyGravityClampTVel` | air_pull_physics_ |
| `0x800D8FA8` | `ftPhysicsCheckClampAirVelXDecMax` | air_pull_physics_ |
| `0x800D9044` | `ftPhysicsClampAirVelXStickDefault` | air_pull_physics_ |
| `0x800D9074` | `ftPhysicsApplyAirVelXFriction` | air_pull_friction_ (fake attr, air_friction 1.75F) |

The `OS.copy_segment(0x548E0/0x54924/0x54948)` copies of vanilla air drift `0x800D90E0` are ported as C in `ftRemixDragonKingUSPAirPullPhysics`. The fast fall branch (ROM 0x54900-0x5491C) is dropped, as the asm does, and the gravity call is replaced with the pull gravity.

## Vanilla patch that the port depends on

`drain_flash_fix_` (asm `OS.patch_start(0x8A1E4, 0x8010E9E4)`) replaces the `beqz t3` at RAM 0x8010E9E4 in `ifCommonPlayerDamageUpdateDigits` (`decomp/src/if/ifcommon.c:595`, from `build/remixasm/refs/vanilla_funcs.txt`). Its semantics, ported from the asm:

- Override when the player object (`a1 + 0x78`) is non-null, its player struct has a non-null `capture_gobj`, the capturer is `REMIX_ID_DRAGONKING`, the capturer status is USPAirPull or USPGroundThrow, and capturer `motion_vars.flags.flag0 != 0`. The C helper is `ftRemixDragonKingUSPIsCaptureFlashOverride`.
- When overriding, or when the vanilla temp `t3` is 0, the code goes to RAM 0x8010E9F4 (store `v1` to `0x54(sp)`).
- Otherwise it falls through to the original path (`s0 = sp + 0x3C`, store `4` to `0x54(sp)`).

Exact C to insert in the vanilla function, under `#ifdef REGION_REMIX`, at the place of the `beqz t3` branch. The vanilla owner must bind the local names (the `t3` variable, the `v1` store value and the `a1` gobj) because the decomp source does not name them:

```c
#ifdef REGION_REMIX
    if (ftRemixDragonKingUSPIsCaptureFlashOverride(<gobj read from a1 + 0x78>) || (<t3 value> == 0))
    {
        <path that stores v1 to 0x54(sp), i.e. RAM 0x8010E9F4>
    }
    else
    {
        <path that sets s0 = sp + 0x3C and stores 4 to 0x54(sp), i.e. RAM 0x8010E9B0>
    }
#else
    <original branch>
#endif
```

The prototype the vanilla file needs: `extern sb32 ftRemixDragonKingUSPIsCaptureFlashOverride(GObj *player_gobj);`. The function is defined non-static in `ftremixdragonkingusp.c` under `#ifdef PORT`. The vanilla file must include it under `#ifdef REGION_REMIX`.

Note: `a1 + 0x78` is read as the GObj pointer by the asm, but the decomp variable that holds `a1` is not identified here. The vanilla owner must confirm that `a1` is the GObj that the hook dereferences.

## Remix scratch words and state

- Temp variables 1/2/3 are `motion_vars.flags.flag0/1/2` (N64 0x17C/0x180/0x184), used in C directly.
- Turn frame timer `0x0B18` is written through `ftRemixFighterWord(fp, 0x0B18)` from `ftremixfields.h`.
- Attacker heal: `fp->damage_heal` (0x0818), captured player: `fp->catch_gobj` (0x840), search gobj: `fp->search_gobj` (0x830), capture gobj: `fp->capture_gobj` (0x844), star counter: `star_invincible_tics` (0x05B0).

## REMIX-TODO markers

- `REMIX-TODO(Item.CloakingDevice)` in `ftRemixDragonKingUSPDrain`. The asm reads `Item.CloakingDevice.cloaked_players[captured player port]` and skips the damage when it is set. That table is owned by an unported item, so the port treats every captured player as not cloaked. Source: `DragonKingSpecial.asm` `_drain` blocks in `throw_main_` (line 190) and `air_pull_main_` (line 233).

## Uncertainties and deviations

1. `air_initial_` (asm line 43 onwards), `and v1, v1, t6; sb v1, 0x018D(a0)`, masks the struct pointer in `v1` (not the byte), so on N64 the byte written is the low 3 bits of the pointer. The asm comment says "disable fast fall flag", and `Marina/MarinaSpecial.asm` has the same pattern. The port follows the intent and clears `is_absorb`, `absorb_lr`, `is_goto_attack100` and `is_fastfall` like the Marina port does. On N64, an 8-byte aligned struct would have written 0, which also clears `is_magnify_show`, `is_limit_map_bounds` and `is_invisible`. The deviation is not reproduced.
2. The `air_pull_initial_` comment says `y velocity * 0.5 + Y_SPEED`, but the code uses `lui at, 0x3E00` (0.125F). The port follows the code (0.125F).
3. Action ids 0xEF to 0xF3 come from the action name string table in `DragonKing.asm` (the entries that are still named `HealthAbsorb*`). The order of the `Character.add_new_action` calls (lines 255-259) matches that, but the numeric ids were not checked against the built ROM.
4. `throw_main_` and `air_pull_main_` read `catch_gobj` without a null check, which the asm also does. A capture with no captured player would crash on both.
5. `ftPhysicsApplyAirVelXFriction` gets a stack `FTAttributes` with only `air_friction` set, as the asm does with its fake attribute at `sp`. This assumes the decomp friction function reads only `air_friction`, which it does.
6. Helper `ftRemixDragonKingUSPAirPullPhysics` does not call `ftPhysicsApplyFastFall`, as the asm skips that branch.
