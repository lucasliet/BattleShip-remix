# 4C LANKY USP: port notes

Source: `.toolchain/pkgs/smashremix-src/src/Lanky/LankySpecial.asm`, scope `LankyUSP` (lines 401-1009).
Port: `decomp/src/ft/ftremix/lanky/ftremixlankyusp.c`, table `dFTRemixLankyUSPProcs` (11 entries, all worklist routines).

## Action ids

- USPGBegin 0xE5, USPABegin 0xE6, USPMove 0xE7, USPTurn 0xE8, USPEnd 0xE9, USPDamage 0xEA.
- Derived from `string_0x0E5`..`string_0x0EA` in `Lanky/Lanky.asm` (lines 47-52) and the
  `add_new_action` order at lines 613-618. Lanky.asm has no explicit `constant` for these.

## Routines in the table

| Remix label | Port |
|---|---|
| air_initial_ | ftRemixLankyUSPAirSetStatus |
| begin_air_collision_ | ftRemixLankyUSPBeginAirProcMap |
| begin_ground_collision_ | ftRemixLankyUSPBeginGroundProcMap |
| begin_main_ | ftRemixLankyUSPBeginProcUpdate |
| collision_ | ftRemixLankyUSPProcMap |
| damage_physics_ | ftRemixLankyUSPDamageProcPhysics |
| end_main_ | ftRemixLankyUSPEndProcUpdate |
| ground_initial_ | ftRemixLankyUSPGroundSetStatus |
| main_ | ftRemixLankyUSPMoveProcUpdate |
| physics_ | ftRemixLankyUSPProcPhysics |
| turn_main_ | ftRemixLankyUSPTurnProcUpdate |

Static helpers (not in the table): move/turn/end setters, begin ground/air transitions,
air control, stick angle, knockback turn, and the shared timer/cancel check.

## Vanilla patches (not applied, need insertion by the vanilla owner)

### damage_patch_ (asm lines 953-1008)

- Hook: `OS.patch_start(0xBBDE4, 0x801413A4)`, which replaces `jal 0x800E6F24` (change action) at
  RAM 0x801413A4 with `j damage_patch_`. The `lui a3, 0x3F80` delay-slot instruction is kept.
- Effect: when a LANKY fighter is hit out of USPGBegin, USPABegin, USPTurn or USPMove, a
  DamageFly next action (`0x0E..` range, Action.DamageFlyHigh..DamageFlyRoll) becomes
  `Lanky.Action.USPDamage` (0xEA). For Electric (DamageElec2), the saved damage action is
  replaced by USPDamage when that is a DamageFly action.
- The function containing 0x801413A4 is NOT in `build/remixasm/refs/vanilla_funcs.txt`. The
  nearest listed entry is `ftCommonDamageInitDamageVars` at 0x80140EE4 (ftcommondamage.c). The
  owner must confirm which decomp function contains 0x801413A4 before inserting the C.
- Logic to insert (asm 960-1007), in order. `next` is the action passed in a1, `cur` is the
  player's current action (`fp->status_id`), `saved` is the damage action saved at stack 0x54 in
  the vanilla frame. Insert it before the original `jal 0x800E6F24` under `#ifdef REGION_REMIX`:
  1. If `fp->fkind != REMIX_ID_LANKY`, go to the original call.
  2. If `cur` is not USPGBegin (0xE5), USPABegin (0xE6), USPTurn (0xE8) or USPMove (0xE7), go to the original call.
  3. If `next == Action.DamageElec2`: if `saved` is in [DamageFlyHigh, DamageFlyRoll], set
     `saved = 0xEA` (USPDamage) and go to the original call. Otherwise go to the original call.
  4. Else, if `next` is not in [DamageFlyHigh, DamageFlyRoll], go to the original call.
  5. Otherwise set `next = 0xEA` (USPDamage) and go to the original call.
  The original call keeps its own `lui a3, 0x3F80` (1.0F) argument. The vanilla owner must map
  `next`, `saved` and the stack slot to the vanilla function's real variables.

## Shared Remix subsystems

- None. The USP routines create no weapon (no `jal 0x801655C8`), spawn no custom item and call no
  GFX or announcer helper. No REMIX-TODO markers are needed.

## Helpers reached from vanilla by function pointer

- `begin_ground_to_air_` and `begin_air_to_ground_` are passed to vanilla as transitions
  (`li a1, begin_ground_to_air_`, asm 882 and 892). The port passes the C function pointers
  straight to `mpCommonProcFighterOnFloor` and `mpCommonProcFighterLanding`, so they stay static.
  If the vanilla common collision reads them from an N64 address instead, they must be added to
  the table, which the worklist does not list.

## Copied vanilla fragments (decoded with tools/remix_disasm_rom.py)

- `end_main_`: `OS.copy_segment(0xD7190, 0x20)`, from vanilla 0x8015B990 (Fox up special ending).
  Decoded as: `lwc1 f6, 0x78(a0)` (anim_frame), `mtc1 zero, f4`, `lui a1, 0x3F80`,
  `move a2, zero`, `c.le.s f6, f4`, `addiu a3, zero, 1`. The Remix code then calls
  `ftCommonFallSpecialSetStatus(gobj, 1.0F, 0, TRUE, FALSE, 0.375F, FALSE)` when anim_frame <= 0.
- `collision_`: `OS.copy_segment(0xD0D98, 0x78)` and `OS.copy_segment(0xD0E14, 0x44)`, from
  vanilla 0x80155598 (Mario up special collision, `func_80155598`). The first copy ends before
  `lui a2, 0x3E8F` (asm 946 replaces it). The port keeps the copied control flow: `ga` check,
  `motion_vars.flags.flag1` check, `vel_air.y >= 0` check, pass-cliff with `ftMarioSpecialHiProcPass`,
  and the cliff mask 0x3000 check. The landing lag is 0x3EC05C29 (the replaced `lui` plus the kept
  `ori a2, a2, 0x5C29` at D0E34).

## Uncertainties

1. **air_initial_ fast-fall byte (asm 455-458).** The routine never loads `v1` from the player
   struct, yet it does `and v1, v1, 0x07; sb v1, 0x018D(a0)`. The stored byte depends on an
   unloaded register, so it cannot be ported exactly. The port clears the four named bits
   (is_absorb, absorb_lr, is_goto_attack100, is_fastfall) and keeps the others. The three low
   bits it keeps (is_magnify_show, is_limit_map_bounds, is_invisible) may differ from the N64.
2. **atan2.** The address 0x8001863C has no decomp name in the repo. The port uses libm `atan2f`
   with `<math.h>`. The banjo USP port has the same REMIX-TODO for this address.
3. **Sin and cos.** 0x80035CD0 (cos) and 0x800303F0 (sin) are ported as `__cosf` and `__sinf`
   from `decomp/include/common.h`. These are the decomp names used elsewhere for these calls.
4. **Joypad bit.** `Joypad.B` is 0x4000 in `Joypad.asm` line 12, equal to `B_BUTTON`. The asm comment
   at line 578 says 0x0020, which is stale. The port uses `B_BUTTON`.
5. **Hitstun word.** damage_physics_ reads `0x0B18` as the hitstun timer (asm 778, `lw v1, 0x0B18`),
   which is the same word as the USPMove timer. This is intended by Remix. The port uses
   `ftRemixFighterWord` for both.
6. **Stick threshold for turn.** main_ and turn_main_ use `|stick_x| < 11` for no turn (asm 593),
   and damage_physics_ uses `10 <= |stick|` on the magnitude (asm 799). Both are ported as written.
7. **Asm typo.** The Remix label is `turn_intial_` (asm 497), kept only in the comment of
   `ftRemixLankyUSPTurnSetStatus`.
8. **ai-memory.** The project scope for `BattleShip` returned "workspace 'BattleShip' not found", so no
   project `_rules/` were loaded. Global `_rules/` were read in full.
