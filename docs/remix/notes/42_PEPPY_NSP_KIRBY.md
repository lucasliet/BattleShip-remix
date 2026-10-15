# 42 PEPPY NSP Kirby variants notes

Port: `decomp/src/ft/ftremix/peppy/ftremixpeppynsp.c`, added to the existing table `dFTRemixPeppyNSPProcs` (now 18 entries, `dFTRemixPeppyNSPProcsNum` unchanged in form). Source: `.toolchain/pkgs/smashremix-src/src/Peppy/PeppySpecial.asm`, scope `PeppyNSP`.

## Routines ported (3)

| Remix label | RAM | Common collision | Transition |
|---|---|---|---|
| `kirby_ground_begin_collision_` | 0x80573ED8 | `0x800DDDDC` = `mpCommonProcFighterOnFloor` | `ftRemixPeppyNSPAirBeginTransition` |
| `kirby_ground_charge_collision_` | 0x805742D4 | `0x800DDDDC` | `ftRemixPeppyNSPAirChargeTransition` |
| `kirby_ground_shoot_collision_` | 0x805747B4 | `0x800DDDDC` | `ftRemixPeppyNSPAirShootTransition` |

Asm lines (PeppySpecial.asm): 284, 650, 1080. Each is a stack-frame wrapper that sets `a1` to the transition and jumps to the common ground collision.

The Peppy grounded collisions call `0x800DDE84` (`mpCommonProcFighterOnEdge`, stops on the edge). The Kirby versions call `0x800DDDDC` (`mpCommonProcFighterOnFloor`, slides off the ledge). The C ports use `mpCommonProcFighterOnFloor`, matching the Marth and Mewtwo Kirby ports.

The Kirby transitions are the existing `ftRemixPeppyNSP*Transition` helpers. Their Kirby action id comes from `ftRemixPeppyNSPSelectStatus`, which the asm also does with `Character.id.KIRBY` / `JKIRBY` checks in `air_begin_transition_`, `air_charge_transition_` and `air_shoot_transition_`.

## Interrupt routines (not ported)

`ground_shoot_interrupt_` and `air_shoot_interrupt_` are not referenced by any action array row. A grep of `Peppy/Peppy.asm`, `Kirby/Kirby.asm` and `JKirby/JKirby.asm` for these labels returned no matches. They are not ported.

## Uncertainties

1. The asm `kirby_ground_*_collision_` wrappers never call the ground-edge transition for Kirby. The Kirby slide-off is inferred from the common routine address (`0x800DDDDC`), as the Marth and Mewtwo ports also did.
2. The Kirby action ids and the Peppy action ids are unchanged from the earlier port (see `42_PEPPY_NSP.md` section 4). No new id was needed.
