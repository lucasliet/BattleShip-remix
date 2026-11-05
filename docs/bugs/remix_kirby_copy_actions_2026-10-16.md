# Remix Kirby copy abilities: action array, motions and a loader overreach

Code: `decomp/src/ft/ftremix/ftremixbind.c` (`ftRemixBindKirbyActions`), `ftremixkirbycopy.c` (copy table, `ftRemixPowerData`),
`ftremixkirbypower.c`, `ftremixkirbyhats.c`, `kirby/ftremixkirbyclone.c` (`ftRemixKirbyCloneActionSwap`),
`decomp/src/ft/ftport.c` (`port_fighter_extend_mainmotion`), `port/remix/remix_roster.cpp`,
`port/bridge/lbreloc_byteswap.cpp` (`chain_fixup_vertex`). Test: `SSB64_REMIX_KIRBY_COPY_TEST=<fkind>` makes Kirby (P1) copy that
power at frame ~405; put the copied fighter in the match (`vs.sh 8,<fkind> inputs/nsp_kirby.txt`), because the copy's weapon
and item files load only for fighters present (`.claude/skills/remix-smoke-tests/SKILL.md`). Copying in the appear status
(frame 1) crashes, which is why the harness waits.

## Symptom

Kirby copying a Remix fighter crashed in the status setup for DSamus (34), Roy (74) and Dedede (64), and the copied power ran
the copied fighter's own routines instead of Kirby's. For Dedede the hat (ids 32 and 33) drew no Kirby body at all.

## Root causes

1. **Kirby's Remix rows were never bound.** Remix grows Kirby's own action array (238 actions: 0xDC + i) and his motion array
   (the copy abilities are Kirby actions such as `DEDEDE_NSP_BEGIN_GROUND`, not the copied fighter's), and routes the copy
   through `Character.kirby_ground_nsp.table` / `kirby_air_nsp.table`. The port only bound the 0x1D..0x60 fighters, so Kirby
   ran the copied fighter's own special handler and looked its statuses up in that fighter's table.
2. **Remix clones of vanilla fighters copy through the vanilla Kirby routines.** The Kirby table entry of a clone is its
   parent's (for example Falco gets `ftKirbyCopyFoxSpecialNSetStatus`), and `clone_power_action_swap_`
   (`ftRemixKirbyCloneActionSwap`) swaps the status id for the clone's own Kirby action (Falco 0xEB to 0x13C). That id only
   resolves once Kirby's action array and motion array are the grown ones.
3. **Special files are read through global pointers.** Remix reads a fighter's weapon and item files from
   `Character.X_file_n_ptr`, so Kirby with a copied power reads the copied fighter's files. The port read
   `fp->data->p_file_special*`, which for Kirby is Kirby's.
4. **Loader overreach (`chain_fixup_vertex`).** A `G_VTX` that asks for more vertices than its array holds ran into the display
   list Remix packs right after the array. A list that starts with a sync command (E7 pipe sync) looks like the vertex
   (-6400, 0, 0), which passes the coordinate checks, so the list's words were rotated by 16 bits and the hat's display list
   became unreadable (Kirby body invisible). Found by dumping the hat file word at the display list offset after each loader
   step (pass 1, pass 2, each chain slot).

## Fix

- `ftRemixBindKirbyActions` builds Kirby's descriptor rows from Remix's Kirby action array, and `port_fighter_extend_mainmotion`
  grows Kirby's vanilla main motion array with the Remix rows past the vanilla count. `remix_roster.cpp` calls both after the
  synth fighters are registered.
- `FighterDescriptor.kirby_copy_handler[2]` (`port/fighter_registry.h`) holds the two `kirby_*_nsp` entries;
  `ftKirbySpecialNSetStatusSelect` and `ftKirbySpecialAirNSetStatusSelect` use them first. `tools/gen_remix_actions.py` emits
  the table addresses and Kirby's count.
- `ftRemixPowerData(fp)` returns the copied fighter's `FTData` for a Kirby with a copied power; the Remix weapon and item sites
  use it for their special files.
- `chain_fixup_vertex` stops at the first vertex that is a sync/end command with no argument bits.
- `ftremixkirbypower.c`: `magic_hat_on` and `taunt_loses_power` per port (`set_absorbed_`, `set_absorbed_hat_`,
  `hat_loss_prevent_`, `kirby_power_loss_prevent_`). With the menu setting off they behave like the game: no magic hat, power
  lost.
- Start-with-hat (`spawn_with_hat`): the hat chosen in the panel is applied through `ftRemixKirbyHatPanelModelPart` in
  `ftManagerInitFighter`.

## Findings worth keeping

- The Remix RAM image keeps words in host order: tables read as `u32` have their two halfwords in logical order
  (`FTKirbyCopy` is `(copy_id << 16) | hat`), and byte streams (particle scripts, texture pixels) must be copied back to big
  endian first (`lbRemixGfxCopyBytes`).
- Kirby's copy table is the Remix `Character.kirby_inhale_struct.table`, indexed by Remix character id (97 rows in
  `ftremixkirbycopy.c`); the vanilla table of the Kirby file only has 27 rows.

## Verification

All 16 copies of the `SSB64_REMIX_KIRBY_COPY_TEST` batch (fkinds 29, 30, 31, 33, 34, 35, 37, 38, 39, 40, 58, 59, 63, 64, 65,
74) ran with `rc=0` and 7 screenshots each. DSamus shows the dark charge shot, Goemon the Goemon hat, Dedede the Dedede hat
with the body. Run the batch with `parallel.sh 6 custom <file>`.
