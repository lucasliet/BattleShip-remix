# Tug of War: deliberate deviations from TugOfWar.asm (2026-11-05)

Two spots of `TugOfWar.asm` index or compare with the wrong register. The port does what the code clearly meant. Everything else
in the credit rules follows the asm.

## last_hit_by_port read by team, written by port

`credit_stock_to_koer_` (`TugOfWar.asm:302`): when the KOer is on the victim's team the stock goes to the last opponent that hit
the victim. The asm reads `last_hit_by_port` with `t2`, the victim's team (`lbu t2, 0x000C(s0)`), but every write
(`update_last_hit_by_._damage`, `._shield`) indexes it by the victim's port. In a free-for-all team == port, so nothing changes.
In a team battle the asm reads the slot of another port. The port reads the victim's own port (`gmRemixTugOfWarGetLastHit(victim->player)`).

## shield break by a teammate

`update_last_hit_by_._shield` (`TugOfWar.asm:~410`): the "same team" test compares the attacker's team (`t0`) with `t1`, which at
that point holds the ADDRESS of `last_hit_by_port` (`li t1, last_hit_by_port`), so it never matches and a teammate that breaks
the shield is recorded as the last hit. The damage path compares the teams correctly. The port skips a shield break by a teammate
(`gmRemixTugOfWarRecordShieldBreak`), as the comment of the hook says ("unless they're on the same team").

## Stock cap

The asm guards the stock increment with `slti 0xFF` on a signed byte, which never limits, so 127 wraps to -128. The port stops at 127
(`GMREMIX_TUGOFWAR_STOCK_MAX`).
