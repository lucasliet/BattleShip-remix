# F14 items_raw: REMIX-TODO closure (ROADMAP T2.11)

Scope: `decomp/src/it/itremix/itremix{klaptrap,bulletbill,scuttleminion,robotbee,cacodemon,car,turnip,spawn,pitfall,waddledee}.c`.
Asm source: `.toolchain/pkgs/smashremix-src/src/Hazards.asm`, `src/Items/*.asm`, `src/Dedede/*.asm`, `src/Poison.asm`.

## Mapping rule used

The brief maps N64 ITStruct offsets to the decomp struct by adding 4 from 0x248 on (N64 = struct_offsets map + 4).
Checked against the port: N64 0x1D0/0x1D4 = `attack_coll.attack_pos[1].pos_prev.x/y` (0x164 + 0x60 + 0xC), N64 0x35C =
`item_vars + 0xC`, N64 0x2CE = ITStruct 0x2CA (`times_thrown` bits), N64 0x11C = `attack_coll.element` (asm writes 0 = punch).

## Decisions

| File:line | State | Reason |
|---|---|---|
| klaptrap `itRemixKlapTrapSetupHitbox` | mapped | Hazards.asm 1780-1781 clear 0x1D0/0x1D4, written to `attack_pos[1].pos_prev.x/y`. The brief's attack_count argument is not needed, because the write is done whatever attack_count is. 0x35C was already `unk_0x35C = 0` in `itRemixKlapTrapSetupVars`. |
| bulletbill `itRemixBulletBillSetupHitbox` | mapped | Hazards.asm 8281-8283: 0x1D0/0x1D4 to `pos_prev.x/y`, 0x35C to `item_vars+0xC`. The bullet has no named vars, so the write is a raw s32 slot write. |
| robotbee `itRemixRobotBeeSetupHitbox` | mapped | Hazards.asm 4873-4875, same three writes. |
| scuttleminion `itRemixScuttleMinionSetupHitbox` | mapped | Hazards.asm 7628-7630, same three writes. |
| cacodemon `itRemixCacodemonApplyHitbox` | mapped | Hazards.asm 8736 writes 0 to 0x11C (= `attack_coll.element`, punch) and 0x1D4 (`pos_prev.y`). 0x1D0 is not cleared by the Cacodemon asm, so only `pos_prev.y` is written. |
| cacodemon `itRemixCacodemonMakeItem` | mapped | 0x35C = `item_vars+0xC`. The clear runs before `target_gobj` because on LP64 `target_gobj` overlaps that slot. |
| car `itRemixCarApplyHitbox` | mapped | Hazards.asm 6854-6856: 0x1D0/0x1D4 and 0x35C, same writes. The existing car DEVIATION about item_vars+4 (0x354) is unchanged. |
| turnip `itRemixTurnipDroppedProc` | DEVIATION | Turnip.asm 284-286 clears mask 0x02 of N64 0x2CE (the "don't randomly despawn" flag). The port maps 0x2CE to ITStruct 0x2CA, where mask 0x02 is the low bit of `times_thrown` (0x02CA:4-6). No named flag exists, and clearing it would change the throw count, so it is not written. The earlier TODO text was correct on this point. |
| spawn `itRemixClearActiveItems` | DEVIATION | Item.asm:1577 calls `Poison.clear_poison_` (Poison.asm:269, clears `poisoned_players`). The Poison subsystem is not ported; ROADMAP lists it as T2.x/M3. |
| pitfall `itRemixPitfallBeginBuried` | SKIPPED | Items/Pitfall.asm:375-390 destroys the pitfall on a hazardous floor using `Surface.get_clipping_flag_` and `Toggles.entry_hazard_mode`. `Surface.get_clipping_flag_` has no decomp function, so the destroy is not ported. |
| pitfall `itRemixPitfallStickToSurface` | SKIPPED | Items/Pitfall.asm:469-478 calls `ftParamMakeRumble` (0x800E806C, decomp `ftparam.c:282`) when the match_info entry at 0x78 is set. The asm passes the item struct as the FTStruct argument and the match_info field is not named. A faithful call would read item memory as a fighter, so it is not ported. |
| waddledee file header | SKIPPED | Dedede minion pointers are FT 0x0ADC and 0x0AE0 (Dedede.asm:699-700 clear, 921-922 read for DSP). The port has no named accessor for them; `ftremixfields.c` reads them only through the generic `ftRemixFighterWord`. |

## Missing for the Waddle Dee owner slots (report for the lead)

- Named fighter fields for Dedede FT 0x0ADC (minion 1) and 0x0AE0 (minion 2), with the clear on spawn (Dedede.asm:699-700)
  and the double-minion check of the DSP (Dedede.asm:921-922).
- `itremixwaddledee.h` has the minion routines (`itRemixWaddleDeeMinionFree`, `itRemixWaddleDeeRemoveMinionCheck`), but the
  slot writes that link a spawned minion to the Dedede fighter are not in the port yet.
- Source: `DededeSpecial.asm:3216` sets 0x033E on a thrown minion; that is the item-side flag and is not checked here.

## Uncertainties

- The attack_count of each hazard item is copied from ITAttributes in `itmanager.c:378`. The Remix item file does not set it,
  so it was not confirmed per item. No decision above depends on it.
- The ITAttackPos offset arithmetic (`attack_pos[1]` at 0x1C4) was taken from the brief and the port's struct
  (`pos_curr` 12 + `pos_prev` 12 + 4 + Mtx44f 64 + 4 = 0x60). It was not re-verified against the ROM.
- The LP64 port makes `GObj *` 8 bytes, so `item_vars+0xC` is only the N64 slot in the cast-based writes. The
  cacodemon ordering comment covers the one case where that overlaps a named pointer.

## Test notes for the lead (game)

- Klaptrap, bullet bill, robot bee, scuttle minion, car, cacodemon: the hitbox clears have no visible effect expected. Check
  that the hazards still hit and still die at blast zones on a stage with hazards on.
- Pitfall: not changed for the hazard floor. Expect the buried behaviour on every floor until the Surface helper is ported.
