# F9 worker brief: Damage.asm stun, flashbang, bury and plunge (ROADMAP T2.7b)

Repository root: `/Users/lucas/Projetos/Pessoal/BattleShip`. Absolute paths and `builtin cd`. Remix source (read-only):
`.toolchain/pkgs/smashremix-src/src/Damage.asm` (973 lines). Read `docs/remix/f7_leftover_worker_brief.md` first (rules,
translation rules, final report), then `docs/remix/notes/f7_damage_poison.md` (what is already ported and what was skipped) and
`docs/remix/notes/f8_airdodge.md` plus `decomp/src/ft/ftremix/ftremixairdodge.c` as the model of a Remix status ported to C
(new action ids, move scripts as `static const u32`, `ftMainSetStatus`, main/interrupt/collision procs).
You only write code and notes. The lead builds and runs. Do not commit. No attribution trailers anywhere.

## Context

Damage types 7..0xC already get their GFX and colour animation (`decomp/src/ft/ftremix/ftremixdamage.c`). The patches that
change the ACTION a hit starts are skipped: `stun_damage_action_` (RAM 0x8014158C, vanilla function
`ftCommonDamageGotoDamageStatus`+0x2C in `decomp/src/ft/ftcommon/ftcommondamage.c`, the branch that decides sleep) and
`stun_electric_action_` (0x801412E8). The flashbang item, the Deku Nut and the bury hazard need them.

## Worker S (stun side), files `ftremixdamagestun.c` and its block in `ftremixhooks.h`

Port: `stun_damage_action_` dispatch (198 to 300) for STUN, DEKU_STUN, FLASHBANG and BURY (the BURY branch calls
`ftRemixDamageBuryOrPlungeInitial(GObj *fighter_gobj, sb32 is_start)`, which worker B defines; include `ftremixbury.h`),
`stun_initial_modified_`, `deku_stun_initial_modified_`, `flashbang_initial_modified_` (302 to 538), `stun_main_aerial_`
(539 to 576) and `stun_electric_action_` (911 to 973). Hook both vanilla sites under `#ifdef REGION_REMIX` in
`ftcommondamage.c` (read the file; find the damage-type switch/branch that matches the asm comment). Damage type ids:
`Damage.id.*` at the top of `Damage.asm`; the port constants are in `ftremixdamage.c`.

## Worker B (bury side), files `ftremixbury.c` and `ftremixbury.h`

Port `bury_or_plunge_initial_`, `bury_initial_` (376 to 471), `bury_player_`, `unbury_player_`, `plunge_player_`,
`plunge_ground_transition`, `plunge_main_`, `plunge_interrupt_`, `plunge_collision_` (577 to 910, including the patch at
RAM 0x800E86B4, `prevent_wind_push_while_buried`: find the vanilla function by address in
`build/remixasm/refs/vanilla_funcs.txt` and hook it under `#ifdef REGION_REMIX`). Public function:
`void ftRemixDamageBuryOrPlungeInitial(GObj *fighter_gobj, sb32 is_start);` declared in `ftremixbury.h` (worker S includes it).
The asm `buried_main_` routine pointer comparison (line 211) becomes a status-id or proc comparison in C: document the choice.

## Both

- Every behaviour of the vanilla game stays identical when no damage type 7..0xC is used.
- Notes in `docs/remix/notes/f9_damage_stun.md` (S) or `f9_damage_bury.md` (B): table of ported asm routines (line, RAM,
  vanilla function, applied or skipped and why), deviations, and what the lead should see (`SSB64_REMIX_FORCE_ITEM=<kind>`
  with the flashbang item: 0x?? is item `Flashbang` in `Item.asm` `add_item` order, base 0x2D).
- Final report under 30 lines: files, hooks, what is not ported.
