# Remix Kirby copy abilities: action array, motions and a loader overreach (2026-10-16)

## Symptom

Kirby copying a Remix fighter crashed in the status setup for DSamus (34), Roy (74) and Dedede (64), and the copied power
ran the copied fighter's own routines instead of Kirby's. For Dedede the hat (id 32 and 33) drew no Kirby body at all.

## Root causes

1. **Kirby's Remix rows were never bound.** Remix grows Kirby's own action array (238 actions: 0xDC + i) and his motion array
   (the copy abilities are Kirby actions such as `DEDEDE_NSP_BEGIN_GROUND`, not the copied fighter's), and routes the copy
   through `Character.kirby_ground_nsp.table` / `kirby_air_nsp.table`. The port only bound the 0x1D..0x60 fighters, so Kirby
   ran the copied fighter's own special handler and looked its statuses up in that fighter's table.
2. **Remix clones of vanilla fighters copy through the vanilla Kirby routines.** The Kirby table entry of a clone is its
   parent's (e.g. Falco gets `ftKirbyCopyFoxSpecialNSetStatus`), and `clone_power_action_swap_`
   (`ftRemixKirbyCloneActionSwap`) swaps the status id for the clone's own Kirby action (Falco 0xEB to 0x13C). That id only
   resolves once Kirby's action array and motion array are the grown ones.
3. **Special files are read through global pointers.** Remix reads a fighter's weapon and item files from
   `Character.X_file_n_ptr`, so Kirby with a copied power reads the copied fighter's files. The port read
   `fp->data->p_file_special*`, which for Kirby is Kirby's.
4. **Loader overreach (`chain_fixup_vertex`, `port/bridge/lbreloc_byteswap.cpp`).** A `G_VTX` that asks for more vertices than
   its array holds ran into the display list Remix packs right after the array. A list that starts with a sync command (E7
   pipe sync) looks like the vertex (-6400, 0, 0), which passes the coordinate checks, so the list's words were rotated by 16
   bits and the hat's display list became unreadable (Kirby body invisible). Found by dumping the hat file word at the display
   list offset after each loader step (pass 1, pass 2, each chain slot).

## Fix

- `ftRemixBindKirbyActions` (`ftremixbind.c`) builds Kirby's descriptor rows from Remix's Kirby action array, and
  `port_fighter_extend_mainmotion` (`ftport.c`) grows Kirby's vanilla main motion array with the Remix rows past the vanilla
  count. `remix_roster.cpp` calls both after the synth fighters are registered.
- `FighterDescriptor.kirby_copy_handler[2]` holds the two `kirby_*_nsp` entries; `ftKirbySpecialNSetStatusSelect` and
  `ftKirbySpecialAirNSetStatusSelect` use them first. `gen_remix_actions.py` emits the table addresses and Kirby's count.
- `ftRemixPowerData(fp)` returns the copied fighter's FTData for a Kirby with a copied power; the Remix weapon and item
  sites use it for their special files.
- `chain_fixup_vertex` stops at the first vertex that is a sync/end command with no argument bits.
- `ftremixkirbypower.c`: `magic_hat_on` and `taunt_loses_power` per port (`set_absorbed_`, `set_absorbed_hat_`,
  `hat_loss_prevent_`, `kirby_power_loss_prevent_`). With the menu off they behave like the game: no magic hat, power lost.

## Evidence

All 16 copies of the `SSB64_REMIX_KIRBY_COPY_TEST` batch run with the copied fighter in the match (copy and inhale need the
fighter's files, so the harness uses `vs.sh 8,<copy>`): 29, 30, 31, 33, 34, 35, 37, 38, 39, 40, 58, 59, 63, 64, 65, 74 all
`rc=0` with 7 screenshots. DSamus shows the dark charge shot, Goemon the Goemon hat, Dedede the Dedede hat with the body.

## Open

`spawn_with_hat` (start with a hat chosen in the menu) needs the CSS and settings, in M4/M5. The harness copies at frame 330;
copying in the appear status still crashes.
