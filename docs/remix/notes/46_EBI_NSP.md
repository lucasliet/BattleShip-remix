# 46_EBI NSP port notes

Port: `decomp/src/ft/ftremix/ebi/ftremixebinsp.c`, table `dFTRemixEbiNSPProcs` / `dFTRemixEbiNSPProcsNum`.
Source: `.toolchain/pkgs/smashremix-src/src/Ebi/EbiSpecial.asm`, scope `EbiNSP`, lines 1-696.
Routines in the table: 10 of 10 worklist entries (`air_begin_initial_`, `air_begin_main_`, `air_collision_`,
`air_wait_main_`, `end_main_`, `ground_begin_initial_`, `ground_begin_main_`, `ground_collision_`,
`ground_interrupt_`, `ground_wait_main_`).

## Action ids

- Ebi (`Character.add_new_action`, Ebi.asm 387-396): `Action.X = (action_array_size.MARIO / 0x14) + n + 0xDC`.
  NSP_Ground_Begin is n = 0, so 0xE5 through NSP_Air_End = 0xEE. This matches `string_0x0E5` in Ebi.asm.
- The `Ebi.Action` scope constants at Ebi.asm 206-215 (`NSPBEGIN(0xE6)`, `NSPWAIT(0xE6)`, ...) are copied from Goemon
  and are off by one. The port uses 0xE5-0xEE. The asm itself uses the macro-generated `Ebi.Action.NSP_*` names.
- Kirby (`Kirby.asm` 451-460): `0xDC + 83 + index`, the same rule the Goemon port uses. index is 114 for
  EBI_NSP_Ground_Begin, so the ids are 0x1A1-0x1AA. Derived from the verified Goemon Kirby ids (0x189 at index 90), not
  read from a built ROM.

## Vanilla patches

None. EbiSpecial.asm has no `OS.patch_start` or `j`/`jal` hooks into vanilla code for these routines. The Ebi
add_new_action lines (Ebi.asm 387-396) reference vanilla physics at 0x800D8BB4 and 0x800DDEE8, which are outside this
file.

## Shared subsystems

- No weapon is created. `end_main_` (asm 308-329) only checks the anim frame and calls mpCommonSetFighterWaitOrFall.
  Its header comment mentions a projectile for temp variable 1, but no such code is present. No `jal 0x801655C8`.
- No custom items, GFX or sounds in this scope.

## Shared routines outside the worklist (static, in the Ebi file)

These are called from the 10 routines and are not in the table:
`ground_wait_initial_`, `air_wait_initial_`, `ground_end_initial_`, `air_end_initial_`, `ground_to_air_`,
`air_to_ground_`.

## Routines owned by other worklists

`kirby_ground_collision_` (asm 513-522), `kirby_air_collision_` (526-535), `kirby_ground_to_air_` (596-621) and
`kirby_air_to_ground_` (625-648) are used only by Kirby's action array. They appear in `30_JKIRBY.md`. This port does
not define them. Kirby's ground collision reaches `kirby_ground_to_air_`, which uses flags 0x0802 and Kirby ids.

GoemonNSP physics (`ground_walk_physics_`, `ground_back_walk_physics_`, `air_physics_`) are referenced by Ebi's action
array, and they are ported in `decomp/src/ft/ftremix/goemon/`.

## Registration needed (not done here, per the brief)

`dFTRemixEbiNSPProcs` must be added to `dFTRemixPortedProcTables` in `ftremixported.c` by the lead. That file was not
edited.

## Behaviour choices and uncertainties

1. `ground_transition_` asm 415-417: the second `beql a2, at` compares a2, which holds the start-frame register and not
   the target. The comment says "branch if target action = NSP_Ground_Wait", so the port tests the target, following
   the comment. Same choice as the Goemon port. Source lines 416-417.
2. `ground_begin_main_` and `air_begin_main_` call the wait routine directly when B is held (asm 211-249). The
   `air_begin_main_` comment says "transition on animation end", but there is no animation-end check in the code. The
   port has no check, which follows the code. The Goemon port uses `ftAnimEndCheckSetStatus` here, so the two differ.
   Source lines 247-248.
3. Transition flags: `ground_transition_` uses 0x0802 (asm 435). `ground_to_air_` and `air_to_ground_` use 0x0803
   (asm 556, 585), which adds FTSTATUS_PRESERVE_HIT. The port keeps these values as they are.
4. `ground_interrupt_` Kirby branch: any fkind other than Ebi goes to the Kirby path (asm 342-362). It includes Kirby
   and J Kirby, as the port handles them. Only Ebi reaches the Goemon branch.
5. Charge words (N64 0x0B18, 0x0B1C) are written in the begin initials and read by no routine in this scope. They are
   kept because other routines may read them. Accessed via `ftRemixFighterWord` in `ftremixfields.h`.
6. The Ebi `*_initial_` routines clear temp variables after `ftMainPlayAnimEventsAll` (asm 36-40), which matches the
   order in the asm.
