# F10 panel: KIRBY_HAT (Kirby Hat, css/KirbyHat.asm)

Panel value 0..42 per port (`REMIX_PANEL_KIRBY_HAT`). Asm: `src/KirbyHats.asm` kirby_hat_select_ (patch 0x800D7E08,
line 582), spawn_with_table_ (line 464), spawn_with_hat (line 510); file loads kirby_hat_files_training_ (658),
kirby_hat_files_1p_ (765), kirby_hat_files_vs_ (696); menu `css/KirbyHat.asm` onchange_handler.

## Behaviour ported

- The spawn hat of Kirby and J Kirby (`REMIX_ID_JKIRBY`) comes from the panel value of its port, when the fighter spawns
  with no copy power (`copy_id == nFTKindKirby`).
- Value 0 (None): vanilla spawn, nothing changes.
- Values 1..40: the fighter of spawn_with_table_[value] (CSS order); its hat is the copy row `copy_modelpart_id` of that
  fighter (the asm onchange_handler reads the same halfword, `lh a2, 0x0002(t3)`).
- Value 41 (???): the None row, hat 0x25 (table patch `Character.table_patch_start(kirby_inhale_struct, 0x2, NONE, 0xC)`).
- Value 42 (Random): one of the 40 fighters (1..40), rolled with `syUtilsRandIntRange(40)`. The asm rolls with
  `Global.get_random_int_`, so the sequence is not the same, only the distribution.
- The hat file is loaded lazily by `ftRemixKirbyHatsGetFile` (`ftremixkirbyhats.c`) the first time a Remix hat modelpart
  (>= 0x0F) is resolved, the same path the copy hats use. The fighter's own file is not needed for a hat: copy hats live in
  Kirby's file or in KIRBY_CUSTOM_HATS / _2.

## Edited

- `decomp/src/ft/ftremix/ftremixkirbyhats.c`: includes `gm/gmremixpanel.h`, `sys/utils.h`; new table
  `sFTRemixKirbyHatsPanelFighter`, `ftRemixKirbyHatsPanelFighterID` (static), `ftRemixKirbyHatPanelModelPart` (public).
- `decomp/src/ft/ftremix/ftremixhooks.h`: declaration appended after the InputDelay block.
- `decomp/src/ft/ftmanager.c`, `ftManagerInitFighter`, Kirby / J Kirby case, REGION_REMIX branch: when the panel hat
  returns a modelpart, set `FTKIRBY_COPY_MODELPARTS_JOINT` to it and leave the switch. Otherwise the existing Remix copy
  row path runs unchanged (value 0 keeps the vanilla behaviour, and J Kirby still gets no default when the panel is None).

Syntax check OK for `ftremixkirbyhats.c` and `ftmanager.c` (`ftremixhooks.h` is included by the first one).

## Not done / to verify in game

- Magic hat (41): only the hat is set. The asm magic branch of kirby_hat_select_ calls `Kirby.clear_magic_hat_charge`
  and does not set `magic_hat_on`; the port's `sFTRemixKirbyMagicHatOn` (ftremixkirbypower.c) stays FALSE at spawn. The
  magic flag read in `src/Size.asm:364` (initialize_base_multiplier_table_) belongs to the Size scope and is not touched.
- Hat loss with taunt / hit (hat_loss_prevent_, kirby_power_loss_prevent_) is not part of this spawn hook.
- J Kirby uses the same `FTKIRBY_COPY_MODELPARTS_JOINT` as Kirby; the joint is assumed, not checked in game.
- Panel hat is applied on every fighter init (respawn too), as the asm spawn hook does.
- No `REMIX-TODO` for KirbyHat existed in `decomp/src`, so nothing was deleted.
- Not built or run (lead builds and tests).
