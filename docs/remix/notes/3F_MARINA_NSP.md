# 0x3F MARINA: NSP and Cargo ports (notes)

Port file: `decomp/src/ft/ftremix/marina/ftremixmarinansp.c`
Table: `dFTRemixMarinaNSPProcs` / `dFTRemixMarinaNSPProcsNum` (15 entries).
Source: `.toolchain/pkgs/smashremix-src/src/Marina/MarinaSpecial.asm`.

Scope: 15 of 15 worklist routines (2 MarinaCargo, 13 MarinaNSP) are in the table.
Helpers are `static` and not in the table: grab/transition procs, the three
`*_pull_initial_`/`*_throw_initial_` entries, `grab_pull_setup_`, the end-move
physics, and the Kirby id selection.

## 1. Vanilla patches the scope relies on

The C below goes into the vanilla decomp functions under `#ifdef REGION_REMIX`.
Function boundaries come from the nearest-start lookup in `vanilla_funcs.txt`,
so check each one against the asm before inserting.

- **`MarinaCargo.shake_check_`** (`OS.patch_start(0xC89F4, 0x8014DFB4)`,
  asm L167-201). Nearest vanilla function: `ftDonkeyThrowFFCheckInterruptThrowFCommon`
  (`decomp/src/ft/ftchar/ftdonkey/ftdonkeythrowff.c:114`). Behaviour: a grounded
  Marina with B pressed enters `CargoShake` and returns TRUE from the interrupt.
  Insert, as a call to a non-static `ftRemixMarinaCargoShakeCheckInterrupt`:
  ```c
  #ifdef REGION_REMIX
  if (ftRemixMarinaCargoShakeCheckInterrupt(fighter_gobj) != FALSE) return TRUE;
  #endif
  ```
  This routine is NOT ported here. It needs `shake_initial_` (see section 2).
- **`MarinaCargo.jump_patch_`** (`0xC8568` -> `0x8014DB28`, asm L205-228).
  Nearest function: `ftDonkeyThrowFJumpSetStatus` (`ftdonkeythrowffall.c:47`).
  For `REMIX_ID_MARINA` / `REMIX_ID_NMARINA`, uses action `Marina.Action.CargoJump`
  (0xFA) and frame speed 1.0F. Otherwise keeps the vanilla action and speed 0.
- **`MarinaCargo.air_patch_`** (`0xC850C` -> `0x8014DACC`, asm L232-252).
  Nearest function: `ftDonkeyThrowFFallSetStatus` (`ftdonkeythrowffall.c:34`).
  Frame speed is 1.0F for (N)Marina, otherwise 0.
- **`MarinaCargo.jumpsquat_patch_`** (`0xC83A4` -> `0x8014D96C`, asm L256-275).
  Nearest function: `ftDonkeyThrowFKneeBendSetStatus` (`ftdonkeythrowfkneebend.c:50`).
  Frame speed is 1.0F for (N)Marina, otherwise 0.
- **`MarinaCargo.throw_f_patch_`** (`0xC9000` -> `0x8014E5C0`, asm L279-302).
  Nearest function: `ftCommonCaptureShoulderedSetStatus` (`ftcommoncapture.c:146`),
  which may be a false match. Verify before inserting. Behaviour: ThrowF damage is
  5 for (N)Marina and 8 otherwise. For Marina it also sets `gem_spawned` (0xB28)
  to FALSE. Those two lines are the ones that matter for `shake_main_`.
  ```c
  #ifdef REGION_REMIX
  if ((fp->fkind == REMIX_ID_MARINA) || (fp->fkind == REMIX_ID_NMARINA)) { MARINA_GEM_SPAWNED(fp) = FALSE; }
  #endif
  ```
  `MARINA_GEM_SPAWNED` is `((fp)->status_vars.common.turn.lr_turn)` in the port.

## 2. Gaps that the lead must resolve

- **`MarinaCargo.shake_initial_`** (0x80568628, asm L34-64) is not in the 0x3F
  worklist and is not ported. It is the action entry for `CargoShake`, set by the
  `shake_check_` patch. Without it the Cargo shake cannot start. It needs a
  non-static port, and the patch must call it.
- **`MarinaNSP.ground_pull_initial_`, `air_pull_initial_`, `ground_throw_initial_`,
  `air_throw_initial_`** are not in the worklist. They are kept `static` in the file,
  and the port passes them as C function pointers where the asm passes raw
  addresses (`li a1, ...`, L339, L376, L956, L990). The yoshi catch and transition
  callbacks are the only consumers. Confirm that the binder does not also need
  their N64 addresses (for example for a table patch that jumps to them by address).
- **`ground_initial_` as a Kirby table patch**: `Kirby.asm` L562-563 patches Kirby's
  ground NSP entry to `MarinaNSP.ground_initial_`. That routine is in the table.

## 3. Derived constants (verify after rebuild)

- **Kirby action ids** 0x17F-0x188 (`nFTRemixKirbyStatusMarinaNSP*`). Derived by
  searching `build/remixasm/remix.z64` for the Kirby action struct whose physics and
  collision words are `MarinaNSP.ground_physics_`/`ground_collision_` (0x80568D00 /
  0x80568F64). That gives index 163 from `Character.KIRBY_action_array` (RAM
  0x804A1BA0), so id 0xDC + 163 = 0x17F. The air NSP (0x80568D7C / 0x80568F88) gives
  0x184, and the air throw (0x80568ECC / 0x805690B0) gives 0x186. The ids are
  consecutive in `Kirby.asm` (`add_new_action` order at L417-426).
- **Item.Gem.id = 0x42**. `Item.asm` L1515-1539 registers `add_item` in order from
  0x2D. `Gem` is the 22nd entry (index 21), so 0x2D + 21 = 0x42.
- **Grab joint** `fp->joints[30]` (0x960 = 0x8E8 + 30 * 4).

## 4. Uncertainties and approximations (asm line numbers)

1. **throw_turn_ rotation (L652-697)**: REMIX-TODO. The step loads
   `0x8018C230..0x8018C23C` (`lui 0x8019; lwc1 0xC230/0xC234/0xC238/0xC23C`). In
   `baserom.us.z64` at the linear map (ROM = RAM - 0x80084800), those words are
   instructions, not floats. The rotation write (`DObj rotate.vec.f.y`, offset 0x34)
   is skipped. The turn timer still counts down. Marina keeps her facing during
   the throw turn.
2. **air_initial_ fast-fall byte (L386-388)**: the asm masks `v1` with 0x07, but
   `v1` holds a stale value after the calls, so the stored byte is not
   reproducible. The port clears `is_absorb`, `absorb_lr`, `is_goto_attack100` and
   `is_fastfall`, which is the intent (same mask as Falco's ProcInterrupt).
3. **throw_air_collision_ (L1013-1029)**: the asm reads v0 from
   `mpCommonProcFighterWaitOrLanding` (0x800DE934). That is `mpCommonProcFighterLanding`
   with `mpCommonSetFighterWaitOrLanding`, and v0 is its return. The port calls
   `mpCommonProcFighterLanding` directly. Confirm the tail-call assumption.
4. **Shake timer and gem slots**: `0xB18` is `status_vars.common.dead.wait` and `0xB28`
   is `status_vars.common.turn.lr_turn` (`MARINA_TURN_TICS` and
   `MARINA_GEM_SPAWNED`). Both are union members the parent never reads during
   these actions, but that depends on the parent's flow.
5. **Ground and air end-move physics (L808-860)**: copied from vanilla
   `0x800D8BB4` (ground friction) and `0x800D8D68`, `0x800D9074` (air). The copy
   is traction 1.0F and air friction 3.0F, which the port reproduces. The copy
   segments (`0x543B4`, `0x543EC`) were checked against `remix_disasm_rom.py`.
   `air_throw_move_physics_` (L887-901) uses THROW_GRAVITY 2.0F; it was checked
   against `0x800D90E0` (`0x548E0`, `0x54924`).
6. **Grab pull setup (L476-488)**: `ftParamSetCaptureImmuneMask(fp, 0x3F)` is the
   asm literal 0x3F, not `FTCATCHKIND_MASK_ALL`, which is a different value.
7. **throw direction (L492-544, L548-603)**: the stick tests are ported in the
   asm's branch order: stick_y >= 40 gives U, stick_y < -39 gives D, then
   stick_x * DIRECTION < -39 turns Marina around with timer 8 (ground) or 6 (air).
   This matches `beql`/`bnel` with nullified delay slots.
8. **Cargo shake loop (L144-157)**: `sw r0, 0x180` sits in the delay slot of
   `bnez`, so it clears on every loop frame. The port does the same.
9. **Frame-zero test**: `c.le.s` against 0.0F (Cargo jump, pull mains), and the
   loop marker `0x80000000` (-0.0F) for `shake_main_`, are checked by bits.

## Hooks applied

Checked by disassembly: at each patch RAM address, the vanilla word equals the asm's
"original line 1". Correction to section 1: `jumpsquat_patch_` is at 0x8014D964 (asm L257),
not 0x8014D96C.

| Patch | RAM | Vanilla function (file) | Inserted C (under `#ifdef REGION_REMIX`) |
|---|---|---|---|
| `shake_check_` | 0x8014DFB4 | `ftDonkeyThrowFFCheckInterruptThrowFCommon` (`ftchar/ftdonkey/ftdonkeythrowff.c`), after `fp` is set | `if (ftRemixMarinaCargoShakeCheckInterrupt(fighter_gobj) != FALSE) return TRUE;` before the ThrowF button test |
| `jump_patch_` | 0x8014DB28 | `ftDonkeyThrowFJumpSetStatus` (`ftchar/ftdonkey/ftdonkeythrowffall.c`) | `ftMainSetStatus(..., ftRemixMarinaCargoJumpStatusId(fp), 0.0F, ftRemixMarinaCargoAnimSpeed(fp), ...)`, vanilla call in `#else` |
| `air_patch_` | 0x8014DACC | `ftDonkeyThrowFFallSetStatus` (`ftdonkeythrowffall.c`) | speed argument = `ftRemixMarinaCargoAnimSpeed(fp)`, vanilla in `#else` |
| `jumpsquat_patch_` | 0x8014D964 | `ftDonkeyThrowFKneeBendSetStatus` (`ftdonkeythrowfkneebend.c`) | speed argument = `ftRemixMarinaCargoAnimSpeed(fp)`, vanilla in `#else` |
| `throw_f_patch_` | 0x8014E5C0 | `ftCommonCaptureShoulderedSetStatus` (`ftcommon/ftcommoncapture.c`) | `ftRemixMarinaCargoThrowFInitGemSpawned(capture_fp);` and damage base `ftRemixMarinaCargoThrowFDamage(capture_fp)` (5 or 8), vanilla `8` in `#else` |

Non-static functions (prototypes in `ft/ftremix/ftremixhooks.h`, Marina block):
`ftRemixMarinaCargoShakeCheckInterrupt`, `ftRemixMarinaCargoJumpStatusId`,
`ftRemixMarinaCargoAnimSpeed`, `ftRemixMarinaCargoThrowFDamage`,
`ftRemixMarinaCargoThrowFInitGemSpawned`.

Ported with these hooks:

- `MarinaCargo.shake_initial_` (0x80568628) is `static ftRemixMarinaCargoShakeInitial`. It is not in
  the action array (Marina.asm L529 gives CargoShake an init of 0), so it has no table entry. It is
  reached only through `ftRemixMarinaCargoShakeCheckInterrupt`. It uses `ftRemixMarinaSpecialNClearTempVars`
  and the shared `ftRemixMarinaDSPPlayVoice` (made non-static in `ftremixmarinadsp.c`).
- Action ids: CargoJump 0xFA and CargoShake 0xFB (Marina.asm L164-165).

Uncertainties:

1. `shake_check_` checks only `REMIX_ID_MARINA`, as the asm does (`Character.id.MARINA`). The other
   four patches check (N)Marina.
2. `throw_f_patch_`: `capture_fp` is the thrower. The patched `lwc1 a2, 0x288(s1)` is
   `capture_fp->motion_attack_id`, which confirms it.
3. The Cargo jump, air and jumpsquat frame speed is 1.0F for (N)Marina and 0.0F otherwise. This matches
   the `mfc1 a3, f0` delay slots, which give 0.0F.
