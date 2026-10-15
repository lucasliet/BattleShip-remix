# Leftover Kirby copy routines (B): DSamusNSP, EbiNSP, DragonKingNSP

Scope: the Kirby and J Kirby copy variants of three neutral specials, added to the existing fighter files.
Each one is in its `dFTRemix<Table>Procs[]` table.

## Ported

| File | Routines added | Table |
|------|----------------|-------|
| `decomp/src/ft/ftremix/dsamus/ftremixdsamusnsp.c` | `aerial_b_button_buffer_kirby`, `air_charge_kirby_main`, `kirby_ground_charge_collision_`, `air_charge_collision_kirby_` | `dFTRemixDSamusNSPProcs` (8 entries) |
| `decomp/src/ft/ftremix/ebi/ftremixebinsp.c` | `kirby_ground_collision_`, `kirby_air_collision_`, helpers `kirby_ground_to_air_` and `kirby_air_to_ground_` (with the Kirby tables) | `dFTRemixEbiNSPProcs` (12 entries) |
| `decomp/src/ft/ftremix/dragonking/ftremixdragonkingnsp.c` | `kirby_ground_collision_`, `kirby_air_collision_`, helpers `kirby_ground_to_air_` and `kirby_air_to_ground_` | `dFTRemixDragonKingNSPProcs` (8 entries) |

Source locations (`.toolchain/pkgs/smashremix-src/src/`):
- DSamus: `DSamus/DsamusSpecial.asm` 7-22 (aerial_b_button_buffer_kirby), 268-334 (air_charge_kirby_main), 387-400 (kirby_ground_charge_collision_), 474-486 (air_charge_collision_kirby_).
- Ebi: `Ebi/EbiSpecial.asm` 513-522, 526-535 (collisions), 596-621 (kirby_ground_to_air_), 625-648 (kirby_air_to_ground_), 673-693 (tables).
- DragonKing: `DragonKing/DragonKingSpecial.asm` 807-831, 833-847 (collisions), 863-917 (kirby_ground_to_air_), 919-end (kirby_air_to_ground_).

## Kirby action ids

The ids come from the order of the `Character.add_new_action(KIRBY, ...)` lines in `Kirby/Kirby.asm`. The index is 0-based, and the id is `0xDC + 83 + index` (`0x12F + index`). The `KIRBY` array size is `0x67C`, so `83 = 0x67C / 0x14`.

- DSamus ground charge `DSCharging` = index 4 = `0x133` (already in the file)
- DSamus air charge `DSamusNSPChargeAir` = index 126 = `0x1AD` (already in the file)
- Ebi `EBI_NSP_Ground_Begin` to `EBI_NSP_Air_End` = index 114 to 123 = `0x1A1` to `0x1AA`
- DragonKing `DRAGONKING_NSP_Ground` = index 124 = `0x1AB`, `DRAGONKING_NSP_Air` = index 125 = `0x1AC`

The Goemon notes (index 90 = `0x189`) use the same rule, so they agree. The ids are not read from the ROM. Derived from the asm only, unverified against the built ROM.

## Vanilla patches and hooks

None in this scope. The Kirby hooks that set up DSamus copies are outside the four DSamus routines (`initial_variable_override_kirby`, `remove_shoot_flag_aerial_transition_kirby`, `check_aerial_charge_kirby`, `set_aerial_charge_kirby`, `air_charge_transition_` with its on-hit switch) and are not added here. The shared `air_charge_transition_` and `ground_charge_transition_` ports already branch on Kirby and J Kirby.

## Shared subsystems

- DragonKing `kirby_ground_to_air_` and `kirby_air_to_ground_` store `0x800F9C8C` and `0x800F9CC4` in `proc_lagstart` and `proc_lagend` (FTStruct 0x0A04 and 0x0A08). This is a REMIX-TODO(lag) in `ftremixdragonkingnsp.c`, and the lag procs are not set. The `0x800E9C78` and `0x800E9CC4` vanilla pair is `ftParamProcPauseEffect` and `ftParamProcResumeEffect`. The address `0x800F9C8C` does not name any function in the decomp (it follows `mpCollisionCheckProjectRWall` at `0x800F9C30`), so the asm may be a typo for `0x800E9C8C` or may point at Remix code. The `0x800E9CC4` value matches `ftParamProcResumeEffect`, which suggests the `lui` high half is off. Verify in the built ROM and in game before setting these pointers.

## Uncertainties

1. DragonKing lag procs (above). Behaviour when Kirby copies Dragon King and hitlag starts is the open point.
2. `kirby_ground_to_air_` in Ebi and DragonKing calls `0x800D8EB8`, which the existing port maps to `ftPhysicsClampAirVelXMax`. The new Kirby code uses the same mapping, as the existing Ebi ground-to-air does.
3. The DSamus Kirby main reads and writes ammo at `0x0AE0`, which is `passive_vars.kirby.copysamus_charge_level`. The shared main uses `0x0ADC`. The rest of the logic matches the shared version. The max-ammo branch calls `ftKirbyCopySamusSpecialNDestroyChargeShot` (`0x80156E60`) instead of `ftSamusSpecialNDestroyChargeShot`.
4. `aerial_b_button_buffer_kirby` reads the hat id at N64 `0x0980` (`ftRemixFighterKirbyHat`) and compares it to `0x13` (Dark Samus hat). It then tail-calls `ftSamusSpecialNStartProcInterrupt` (`0x8015D464`).
5. Flags: the Ebi Kirby transitions use `0x0802` (`WALK_TRANSITION_FLAGS`), where the Ebi transitions use `0x0803`. DragonKing Kirby uses `0x4006` (RUMBLE, EFFECT, COLANIM).

## Verification

- `scripts/remix/syntax-check.sh` prints `syntax-check OK` for all three files.
- No build, no game run, no commits, as the brief requires.
