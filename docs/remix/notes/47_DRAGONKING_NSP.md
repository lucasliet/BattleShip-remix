# 47 DRAGONKING NSP: notes for the neutral special port

Port: `decomp/src/ft/ftremix/dragonking/ftremixdragonkingnsp.c`
Table: `dFTRemixDragonKingNSPProcs` / `dFTRemixDragonKingNSPProcsNum` (6 routines, all in scope).
Source: `.toolchain/pkgs/smashremix-src/src/DragonKing/DragonKingSpecial.asm` lines 415-957 (scope DragonKingNSP).
Action ids: `DragonKing/DragonKing.asm` lines 79-88 (DragonBall 0xE4, DragonBallAir 0xE5).

## Routines in the table

| Remix label | Port |
|---|---|
| `ground_initial_` (asm 424) | `ftRemixDragonKingNSPGroundInitial` |
| `air_initial_` (447) | `ftRemixDragonKingNSPAirInitial` |
| `main_` (516) | `ftRemixDragonKingNSPProcUpdate` |
| `physics_aerial_` (758) | `ftRemixDragonKingNSPAerialProcPhysics` |
| `ground_collision_` (794) | `ftRemixDragonKingNSPGroundProcMap` |
| `air_collision_` (820) | `ftRemixDragonKingNSPAirProcMap` |

Static helpers (not in the table): `dragon_ball_` (542) as `ftRemixDragonKingNSPDragonBall`, `destroy_ball_on_hit_` (739) as `ftRemixDragonKingNSPDestroyBallOnHit`, `ground_to_air_` (846) and `air_to_ground_` (902) as `ftRemixDragonKingNSPGroundToAir` / `ftRemixDragonKingNSPAirToGround`.

## Vanilla calls resolved

- `0x800E6F24` ftMainSetStatus; `0x800E0830` ftMainPlayAnimEventsAll; `0x8015FB40` ftCaptainSpecialNInitStatusVars (ground_initial_, air_initial_).
- `0x800D9480` ftAnimEndCheckSetStatus. Its proc is chosen by `ga` (0x14C): ground (0) gives `0x8013E1C8` = ftCommonWaitSetStatus, air gives `0x8013F9E0` = ftCommonFallSetStatus. The asm `lui 0x8014` + `addiu 0xE1C8` / `0xF9E0` sign-extends to these addresses.
- `0x800DDE84` mpCommonProcFighterOnEdge, `0x800DE6E4` mpCommonProcFighterLanding.
- `0x8015FA8C` ftCaptainSpecialNSwitchStatusAir, `0x8015FA2C` ftCaptainSpecialAirNSwitchStatusGround.
- `0x800D91EC` ftPhysicsApplyAirVelFriction, `0x800D90E0` ftPhysicsApplyAirVelDrift.

## Remix scratch words

- Word `0x0B20` (status_vars union, the dragon ball object pointer) is accessed through `ftRemixFighterWord`. The initial routines reset it to 0, as the asm does.
- `0x017C`, `0x0180` are `motion_vars.flags.flag0` and `flag1`. `0x09EC` is `proc_damage`. `0x014C` is `ga`.

## REMIX-TODO markers

1. **GFX (dragon ball effect).** `dragon_ball_` consumes the spawn request (flag0 == 1, cleared as the asm does) but does not call `efManagerMakeEffectNoForce` on the Remix GFX struct at `0x8013E1D4`. That struct is Remix data, not ported. Skipped: the ball object, its scale (`Size.multiplier_table` for the port slot times 0.75, or 0.5 for Kirby and J Kirby), its hand attachment (`gmCollisionGetFighterPartsWorldPosition` on joint part 0xC), the gfx position updates and the on-hit hook install inside `dragon_ball_`. The ball is visual and does not affect hitboxes or damage in the asm, so this is a visual gap, not a damage gap. The `augment_ball_update_routine_` patch at `0x801017F0`, which moves the ball with the hand each frame, is also not applied (see vanilla patches below).
2. **GFX (destroy).** `destroy_ball_on_hit_` returns early because the ball word is always 0. `destroy_ball_object_` (`efManagerSetPrevStructAlloc` / `0x80009A84` free) is not ported.
3. **Size.** Covered by item 1; the `Size.multiplier_table` read is not ported.

## Vanilla patches (for the vanilla owner to apply, not applied here)

These are `OS.patch_start` blocks in the Remix scope that the port skips because the ball is not spawned:

- `augment_ball_update_routine_`: `OS.patch_start(0x7CFF0, 0x801017F0)`, replaces the gfx update routine call. Rebinding it is only needed once the ball exists. Not applied.
- `destroy_ball_object_`: `OS.patch_start(0x7D010, 0x80101810)`. Not applied.

## Kirby routines (not in this worklist)

`kirby_ground_begin_initial_`, `kirby_air_begin_initial_`, `kirby_ground_collision_`, `kirby_air_collision_`, `kirby_ground_to_air_`, `kirby_air_to_ground_` are in the same scope but do not appear in `47_DRAGONKING.md`. They are not in this table. They are referenced by `Action.KIRBY.DragonKingNSP*` (Kirby copy path), so they belong to the Kirby copy worker.

## Uncertainties

- Asm 528-529: the `ga` branch relies on `beqzl` delay-slot semantics. The chosen proc matches the sign-extended addresses, but this was verified by arithmetic, not by a disassembly run. `tools/remix_disasm_rom.py` on `scope main_` would confirm it.
- Asm 771 and 781: `physics_aerial_` calls the drift and friction routines with `a0` unchanged, as assumed.
- The ball's effect-based visuals are unverified because the game was not run (brief rule).

## Step 2 wave 2026-10-14 (partial — kimi quota window)

- LANDED (W2, partial — quota cut the worker after this file): electric ball complete (Vars via 0x0B20, SetBallPosition/DestroyBall/BallProcUpdate, gfx EFDesc, DragonBall spawn, Kirby-copy check incl. JKIRBY). REMIX-TODO(Size) markers left where the Size multiplier applies.
- Piano and JKirby: NOT started (worker died on quota).
