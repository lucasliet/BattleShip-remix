# F8 Charge Smashes (ROADMAP T4.2): ChargeSmashAttacks.asm

Toggle: title `Charge Smashes` (Toggles.asm `entry_charged_smashes`, type INT, default 0 in every profile, range 0..2).
Read as `gmRemixToggle(charged_smashes)` (the asm reads `Toggles.entry_charged_smashes + 4` directly, no VS guard).
Values: 0 off; 1 charge, released automatically at 60 frames (1 second); 2 unlimited, no automatic release.
Test: `SSB64_REMIX_TOGGLES="Charge Smashes=1"` (or `=2`).

What to see with the toggle on: hold A on the charge frame of a forward, up or down smash (or hold A during a Beam Sword,
Fan or Star Rod swing while holding that item). The animation freezes, the fighter shakes, a dust puff appears every 16
charge frames, sound 0x528 plays, and the charge GFX (routine 0x78) is set. Releasing A resumes the animation. With value 1
the charge releases by itself at 60 frames, with value 2 it does not.

## Patches

| Asm | RAM | Vanilla function (decomp file) | Status |
|-----|-----|--------------------------------|--------|
| `smash_attack_main` (line 47) | 0x800D94D0 | `ftAnimEndSetWait` (`ft/ftanimend.c`) | applied: `ftRemixChargeSmashHoldsAnimEnd` in front of the wait transition |
| `smash_attack_start` (line 26), called by `Character.asm` on_action_changed_hook (line 3003) | end of `ftMainSetStatus` (0x800E7AA4, the hook sits at the epilogue) | `ftMainSetStatus` (`ft/ftmain.c`) | applied: `ftRemixChargeSmashLogic(fighter_gobj)` after the last statement |
| `smash_attack_logic` (line 73) | n/a (called) | `ftremix/ftremixchargesmash.c` | ported, `ftRemixChargeSmashLogic` |
| `custom_shuffle` (line 298) | 0x800F2E74 | `ftDisplayMainProcDisplay` (`ft/ftdisplaymain.c`, the shuffle `syMatrixTra`) | applied: `ftRemixChargeSmashShufflePosition(fp)` under `REGION_REMIX` |
| `custom_shuffle_item` (line 338) | 0x800C9FC0 | `func_ovl0_800C9F70` (`lb/lbcommon.c`) | applied: same helper under `REGION_REMIX` |
| `frame_table` / `set_charged_smash_attacks` (line 588+) | data | port table in `ftremixchargesmash.c` | ported, all Remix ids |

Vanilla lines are kept under `#else` in every vanilla file. The helper header block is appended to `ftremixhooks.h`.

### Deviations

- `ftRemixChargeSmashHoldsAnimEnd` returns FALSE while the toggle is off. The Remix asm takes the charging branch whenever the
  DObj anim speed is 0 and does not transition, even with the toggle off. The brief requires vanilla behaviour when off, so the
  hold is gated on the toggle.
- The charge logic for an action change runs with the toggle off as a no-op (the first check), as in the asm.
- Fighters with no charge table (`REMIX_ID_RANDOM`, `REMIX_ID_NONE`, ids the asm never registers) get no charge. The asm would
  read a NULL table pointer there.

## Data

- Charge frames per Remix id: `sFTRemixChargeFramesByKind`, indexed by `REMIX_ID_*` (equal to the port fkind). Values copied
  from the asm `entry_*` labels. Crash, Peach and Roy values come from `Crash.asm`, `Peach.asm` and `Roy.asm`
  (`charge_smash_frames`), and NCrash and NPeach point at the same values, because those files register their tables through
  `set_charged_smash_attacks`. DrLuigi uses the Mario entry. Lanky uses its own entry.
- Item smash starts: `{ 14, 22, 7, 15 }` for Beam Sword (0x80), Bat (0x84), Fan (0x88), Star Rod (0x8C). The asm indexes by
  the byte offset `action - 0x80`, which is `(action - 0x80) / 4` here.
- GFX routine `CHARGE_SMASH` = 0x78 (`build/remixasm/bass.log`: "CHARGE_SMASH - ID is 0x78").

## Uncertainties (asm line numbers)

- 0x8000BB04 is the animation-speed setter. It is not in `vanilla_funcs.txt`. Inferred as `gcSetAnimSpeed` (as in
  `docs/remix/notes/4C_LANKY_DSP.md`). The port calls `gcSetAnimSpeed(fighter_gobj, speed)`, which sets every DObj of the fighter.
- The asm writes the GFX routine to 0x0A28, which the port reads as `fp->colanim.colanim_id` (as Poison does).
- The asm's `_spawn_smoke` passes a2 = 1.0F to `efManagerDustHeavyMakeEffect`. The decomp signature has no third argument, so it
  is not passed.
- `custom_shuffle` (asm lines 298-335) indexes the custom table by `is_shuffle_electric - 2`. Only charge sets the value 2, so the
  port uses one row for every value >= 2 instead of reading past the table.
- `smash_attack_start` runs on every action change in the asm and checks the action inside. The port does the same at the end
  of `ftMainSetStatus`. The `Character.asm` on_action_changed_hook must not add a second call.

## REMIX-TODO

- None in this scope. The Size.asm block of charged smashes (`fix_damage_`, asm lines 477-527) is outside this scope. It is
  already marked in `decomp/src/ft/ftmain.c` (line 330, "REMIX-TODO(ChargeSmash)"). Its state, the `charged_smash_fighter_array`
  of the asm, lives in `sFTRemixChargeStates` of `ftremixchargesmash.c`. The Size port needs a getter for the charge time of a
  player (not added here).
