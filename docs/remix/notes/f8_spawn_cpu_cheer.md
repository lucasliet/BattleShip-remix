# F8 notes: Neutral Spawns, Default CPU LVL (V.S.), Skip Start Cheer

Scope: `.toolchain/pkgs/smashremix-src/src` (Spawn.asm, AI.asm, Toggles.asm, Training.asm). New code in
`decomp/src/gm/gmremixspawnsetup.c` and `.h`. Syntax-check OK for the three touched C files. Not built, not run.

## Patches

| # | Toggle | Asm (ROM / RAM) | Vanilla function (decomp) | Status |
|---|--------|-----------------|---------------------------|--------|
| 1 | Neutral Spawns | Spawn.asm:185 `load_spawn_`, `OS.patch_start(0x00076764, 0x800FAF64)` | `mpCollisionGetPlayerMapObjPosition` (mp/mpcollision.c:3516) is the asm hook site, but the VS spawn selection is `port_comp_ruleset_get_spawn` (sc/sccommon/scvsbattle.c:126), which calls it | Applied |
| 2 | Default CPU LVL (V.S.) | AI.asm:38 `force_default_cpu_lvl_setup`, `OS.patch_start(0x1391AC, 0x8013AF2C)` | `mnPlayersVSInitVars` (mn/mnplayers/mnplayersvs.c:5539, 0x8013AEC8) | Applied (once per boot). A-press refresh skipped |
| 3 | Skip Start Cheer | Training.asm:2740-2770 `skip_crowd_cheer_`, hooks 0x116D68 (0x80190548), 0x112FA8 (0x8018E868), 0x10E550 (0x8018FCF0) | see below | Skipped |

### 1. Neutral Spawns (applied)

- Remix flow (Spawn.asm 185-300): toggle off gives the vanilla spawn. Toggle on: Smashketball always neutral (skipped,
  see TODO). Otherwise teams (`valid_teams`, when `Global.vs.teams`) or singles (`valid_singles`) pick a neutral slot per
  player, and `neutral_table[stage][slot]` gives x and y (z is not written, as in the asm). No valid setup keeps vanilla.
- Port: `port_comp_ruleset_get_spawn` calls the vanilla lookup first, then `gmRemixNeutralSpawnApply(player, pos)` and
  returns under `#ifdef REGION_REMIX`. The vanilla lookup stays the fallback. `port_comp_ruleset_get_spawn` is the only
  VS caller, so no screen check is needed (the asm checks `Global.current_screen == 0x16`).
- Stage id: `gSCManagerSceneData.maps_vsmode_gkind`. The decomp enum (grdef.h) gives `nGRKindCastle = 0x00` ...
  `nGRKindLast = 0x10`, the same numbers as the Remix VS stage ids. Ids above 0x10 keep vanilla.
- Tables: `neutral_table` rows 0x00-0x10 (17 stages x 4 slots), `valid_teams` (36 rows), `valid_singles` (6 rows),
  generated from Spawn.asm (dw hex entries converted to exact f32 hex literals).

### 2. Default CPU LVL (V.S.) (applied, once per boot)

- AI.asm 38-60: on the first VS setup (`AI.is_default_cpu_lvl_set == 0`), the level of the four ports is set from
  `Toggles.entry_default_cpu_level`. Value 0 gives level 3.
- Port: `gmRemixDefaultCPULevelSetup()` (static flag, same as the asm flag) writes
  `gSCManagerTransferBattleState.players[i].level`, called from `mnPlayersVSInitVars` before `mnPlayersVSInitSlotAll`
  (verified order: 5682 vs 5707). `SCPlayerData.level` is at offset 0, matching the asm writes at +0.
- Skipped: the A-press handler (Toggles.asm 1740-1765, `set_cpu_level`, "refresh already-set CPU levels without reboot").
  The decomp has no callback for the port menu toggle. Effect: a level changed in the menu applies at the next app
  start, not immediately. REMIX-TODO(port menu callback).

### 3. Skip Start Cheer (skipped)

- Asm: `skip_crowd_cheer_` (Training.asm 2740-2770) patches three `jal` sites that play the crowd cheer FGM 0x0272
  (`jal 0x800269C0`, `func_800269C0_275C0`) for Training reset (0x116D68), Bonus 1/2 (0x112FA8) and 1P / Bonus 3 / HRC
  (0x10E550).
- Why skipped: the decomp C has no call to FGM 0x0272 in any sc/ file (grep for `0x800269C0`, `0x272`,
  `func_800269C0_275C0` in `decomp/src/sc`), so there is no vanilla call to guard. `vanilla_funcs.txt` maps 0x80190548
  to `sc1PTrainingModeFuncStart` and 0x8018E868 / 0x8018FCF0 to training functions, which does not match the Bonus
  and 1P hooks. The lead should check the mapping before a hook is written.
- Other VS start flow uses: none found (the hook is only in Training, Bonus and 1P in the asm).

## Toggles

| Title (Toggles.asm) | Symbol | Type / range | Defaults CE / TE / NE / JP | Read as |
|---------------------|--------|--------------|----------------------------|---------|
| `Neutral Spawns` | `REMIX_SYM_Toggles__entry_neutral_spawns` | bool | 1 / 1 / 1 / 1 | `gmRemixToggle` (Toggles.guard, not single_player_guard) |
| `Default CPU LVL (V.S.)` | `REMIX_SYM_Toggles__entry_default_cpu_level` | int 0-10 | 0 / 10 / 0 / 0 | `gmRemixToggle` (AI.asm reads the entry directly) |
| `Skip Start Cheer` | `REMIX_SYM_Toggles__entry_skip_start_cheer` | int 0-2 | 0 / 0 / 0 / 0 | not applied |

Override: `SSB64_REMIX_TOGGLES="Neutral Spawns=0"`, `"Default CPU LVL (V.S.)=5"`.

## What to see in a short match

- Neutral Spawns on (default): VS 1v1 on Peach's Castle (0x00). Players start at the neutral slots of Spawn.asm row 0x00
  (x -1613 / 1613, y 1554 / 1412 for the two high slots, y 662 for the low ones), not at the vanilla top spawns (x about -210).
  Turn it off with `Neutral Spawns=0`: spawns return to vanilla. 3-player and FFA have no valid setup, so vanilla spawns.
- Default CPU LVL (V.S.) `=5`: on the first VS CSS of the boot, every CPU slot starts at level 5. Level 3 for `0`.
  Change the value and restart the app to see the new default (the menu refresh is not ported).
- Skip Start Cheer: no effect yet.

## Changed files

- `decomp/src/gm/gmremixspawnsetup.c` (new), `decomp/src/gm/gmremixspawnsetup.h` (new)
- `decomp/src/sc/sccommon/scvsbattle.c`: include block, `port_comp_ruleset_get_spawn` hook (`#ifdef REGION_REMIX`)
- `decomp/src/mn/mnplayers/mnplayersvs.c`: include, `mnPlayersVSInitVars` hook (`#ifdef REGION_REMIX`)

## Uncertainties and TODOs

- REMIX-TODO(Smashketball): `_smashketball` (Spawn.asm ~195) needs `VsRemixMenu.vs_mode_flag` and
  `Smashketball.player_spawns`; no port. Smashketball follows the Neutral Spawns toggle.
- Port enhancement `port_enhancement_neutral_spawns` (CVar `gEnhancements.NeutralSpawns`) is skipped under
  `REGION_REMIX`, because the Remix hook returns before it. The Remix toggle decides. Decision for the lead to confirm.
- Stage row 0x0A: Remix calls it "Dream Land Beta 2"; the decomp enum calls 0x0A `nGRKindPupupuNew` ("Test Stage").
  The id matches, but the name differs, so check that VS on 0x0A shows the stage Remix expects.
- `SCPlayerData` stride versus the asm `0x74` steps is assumed equal, not checked in this pass.
- REMIX-TODO(port menu callback): Default CPU LVL A-press refresh (Toggles.asm `set_cpu_level`).
- Skip Start Cheer: see the section above.
- Lead fix (2026-10-19): the neutral table row came from `gSCManagerSceneData.maps_vsmode_gkind`, which is the stage select
  cursor memory and stays 0 in the VS shortcut, so every stage used the Peach's Castle row and the entry pipe floated above
  the Dream Land platforms. The row now comes from `gSCManagerBattleState->gkind`. Checked on Dream Land, Peach's Castle and
  Kongo Jungle: the pipe sits on the platform or the bridge end.
