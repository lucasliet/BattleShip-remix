# F13 notes: Remix AI hooks outside AI.asm (T7.1 inventory)

Read-only survey for ROADMAP T7.1 (Remix AI). Source of truth for the Remix side:
`.toolchain/pkgs/smashremix-src/src`. No source was edited and nothing was built.
Scope note: the ai-memory project scope could not be resolved (`workspace 'JRickey' not found`),
so no project rules were loaded. Global rules were read (no commit attribution, no live provider tests).

## 1. Improved AI toggle

Toggle definition: `Toggles.asm:2428`, `entry_improved_ai: entry_bool("Improved AI", ...)`, chained after
`entry_punish_on_failed_z_cancel` (`Toggles.asm:2427`). Default value is in the entry_bool arguments (not interpreted here).

Remix readers (all `Toggles.read` / `Toggles.guard` / `OS.read_word(entry + 0x4)`):

| file:line | enclosing scope | what changes when the toggle is ON |
|---|---|---|
| AI.asm:107 | evade_fix_ (AI.asm:94) | Remix 1P is always "advanced AI"; otherwise the toggle gates the invulnerable/evade check. |
| AI.asm:166 | recovery_fix_ (AI.asm:133) | Skips the up-B check (except Classic 1P/RTTF). |
| AI.asm:191 | fix_target_pos_x (AI.asm:182) | Clamps the CPU walk target inside the blast zone (padding 200.0, 0x4348). |
| AI.asm:323 | fix_remix_recovery (AI.asm:240) | Pikachu recovery uses normal behaviour when ON. |
| AI.asm:508, 519 | random_teching_ (AI.asm:443) | Random teching is gated by level 10 in vanilla; the toggle guards the advanced path. Training uses menu values. |
| AI.asm:710 | z_cancel_ (AI.asm:667) | CPU Z-cancel behaviour gated by the toggle. |
| AI.asm:3744 | _check_respawn (AI.asm:3743) | CPUs mash Start to respawn in team battles. |
| AI.asm:6720 | cpu_di_ (AI.asm:6533) | DI path reads the toggle. |
| nessshared.asm:1190 | improved_pk_thunder_ (nessshared.asm:1159) | Animation-frame fix for PK Thunder when ON. |

Notes:
- Remix 1P (`singleplayer_mode_flag == 4`) forces advanced AI regardless of the toggle (AI.asm:107, 3744 region).
- AI.asm:6896 (render_10_) is the handicap indicator, not a toggle reader.

Port side:
- `gmRemixToggle(improved_ai)` is not used anywhere in `decomp/src` or `port`. The only hit is the address define
  `REMIX_SYM_Toggles__entry_improved_ai 0x804625D4u` in `port/remix/remix_symbols.h:4068`.
- Registry: `port/remix/remix_toggles.cpp` walks the Toggles.asm node chain from the RAM image (heads at
  `REMIX_SYM_Toggles__head_*`, `kExpectedToggles = 531`) and reads titles at runtime. Because `entry_improved_ai`
  is a chain node, the title should appear in the registry. This is inferred from the chain, not verified at runtime.
- Header: `decomp/src/gm/gmremixtoggle.h` defines `gmRemixToggle(name)` = `remix_toggle_get(REMIX_SYM_Toggles__entry_##name)`.

## 2. Single Button Mode CPU hooks

Asm: `SingleButtonMode.asm`, `scope handle_cpu_inputs_` (line 118). Patch sites from f8 notes:
- A/B: 0xAC73C -> 0x80131CFC (`_a_b`)
- Z/L: 0xAC784 -> 0x80131D44 (`_z_l`)

Port already has:
- `decomp/src/ft/ftremix/ftremixsinglebutton.c:131` `ftRemixSingleButtonCpuAOrBPress(mode, cpu_button)`
- `decomp/src/ft/ftremix/ftremixsinglebutton.c:153` `ftRemixSingleButtonCpuZOrLPress(mode, cpu_button)`
- Declared in `decomp/src/ft/ftremix/ftremixhooks.h:396-397`.
- The human-side filter `ftRemixSingleButtonModeFilter` IS wired (`ftmain.c:1468`).

Missing: **no caller** of the two CPU helpers exists in `decomp/src` or `port` (grep confirms only definitions and the
header). They are unwired, marked `REMIX-TODO(AI)` in `docs/remix/notes/f8_speed_sbm.md`.

## 3. Fighter-specific CPU routines (outside the AI/Attacks.asm files)

Method: grep for `scope cpu_`, `scope *cpu*`, `cpu_post_process`, `cpu_attack_weight`, and the `OS.patch_start` lines
next to them. Routine pointers are filled by `Character.table_patch_start(...)` (indexed by fkind).

Port status column: "no" = no C with that behaviour. The `ai_*_routine` slots exist in
`port/fighter_registry.h:145-158` (accessors in `fighter_registry.cpp:263-277`), but **no code assigns them**
(grep for `ai_*_routine =` in port and decomp is empty).

| file:line | routine label | what it does | vanilla patch / table | port C? |
|---|---|---|---|---|
| linkshared.asm:740 | cpu_pull_out_bombs_ | Link clones, Peach, Marina pull bombs | patch 0xB3444 -> 0x80138A04 | yes: `ftcomputer.c:6911-6916` (ELINK, JLINK, PEACH); MARINA case at `ftcomputer.c:6938` (charge condition not verified) |
| linkshared.asm:822 | cpu_post_process (LINK, JLINK, ELINK, YLINK) | per-frame CPU post process | table entries at linkshared.asm:871-878 | no (YLINK at `ftcomputer.c:6928` is the input routine, not post process) |
| nessshared.asm:795 | cpu_usp_usage_fix_ | Ness/PK USP usage for CPU | patch 0xAF170 -> 0x80134730 | no |
| nessshared.asm:822 | cpu_usp_control_fix_ | USP control for CPU | patch 0xB1BA0 -> 0x80137160 | no |
| nessshared.asm:1133 | cpu_pk_thunder_attack_ | PK Thunder CPU attack | patch 0xB344C -> 0x80138A0C | no |
| nessshared.asm:1245 | save_cpu_recovery_location_ | saves CPU recovery point | patch 0xCE6E8 -> 0x80153CA8 | no |
| dkshared.asm:288 | cpu_fix_2 | DK CPU fix | patch 0xB39B8 -> 0x80138F78 | no |
| dkshared.asm:459 | cpu_post_process (DK, JDK) | per-frame CPU post process | table at dkshared.asm:587-590 | no |
| jigglypuffkirbyshared.asm:513 | kirby_cpu_inhale | Kirby inhale for CPU | patch 0xB165C -> 0x80136C1C | no |
| jigglypuffkirbyshared.asm:539 | kirby_cpu_inhale_2 | same family | patch 0xB3948 -> 0x80138F08 | no |
| jigglypuffkirbyshared.asm:561 | kirby_cpu_inhale_3 | same family | patch 0xB1B30 -> 0x801370F0 | no |
| jigglypuffkirbyshared.asm:617 | kirby_cpu_inhale_4 | same family | patch 0x5DBE8 -> 0x800E23E8 | no |
| jigglypuffkirbyshared.asm:1034 | cpu_post_process (JIGGLYPUFF, JPUFF) | per-frame CPU post process | table at 1096-1099 | no |
| jigglypuffkirbyshared.asm:1103 | cpu_post_process_kirby | Kirby post process | table (routine ptr) | no |
| jigglypuffkirbyshared.asm:1176 | cpu_attack_weight | puff attack weights | table | no (generic dispatch exists, see below) |
| jigglypuffkirbyshared.asm:1213 | cpu_attack_weight_kirby | Kirby attack weights | table | no |
| pikashared.asm:144 | cpu_post_process (PIKACHU, EPIKA, JPIKA) | per-frame CPU post process | table at 190-195 | no |
| yoshishared.asm:278 | yoshi_cpu_fix_1 | Yoshi CPU fix | patch 0xAFFCC -> 0x8013558C | no |
| yoshishared.asm:524 | cpu_post_process (YOSHI, JYOSHI) | per-frame CPU post process | table at 578-581 | no |
| yoshishared.asm:583 | cpu_attack_weight (YOSHI, JYOSHI) | Yoshi attack weights | table at 623-626 | no |
| foxshared.asm:134 | cpu_post_process (FOX, JFOX) | per-frame CPU post process | table at 180-183 | no |
| captainshared.asm:613 | cpu_attack_weight (FALCON, JFALCON) | Falcon attack weights | table at 684-687 | no |
| captainshared.asm:689 | cpu_post_process_falcon | Falcon post process | table at 757-760 | no |
| Costumes.asm:1096 | cpu_costumes_ | C-button CPU costume on the panel (not AI logic) | patch 0x136770 -> 0x801384F0 | no (CSS feature, not AI) |
| Character.asm:2935 | cpu_post_process (default table) | generic post process, all fighters | patch 0xB52C4 -> 0x8013A884 | no |
| Character.asm:2949 | cpu_attack_weight (default) | generic attack weights | table | partial: generic dispatch at `ftcomputer.c:4439-4450` calls `port_fighter_ai_attack_weight_routine(fkind)`, which is always NULL |
| AI.asm:6533 | cpu_di_ | adds DI to CPU behaviour | patch 0xB4E08 -> 0x8013A3C8 | no |
| AI.asm:3732 | cpu_post_process (core) | core post-process hook (AI.asm, not a fighter file) | not captured | no |

No `cpu_*` scope exists in the Banjo, Bowser, Conker, Dedede, DSamus, Ebi, Ganondorf, Goemon, Lanky, Marth,
Mewtwo, Peach or Wario directories. Falco/Falco.asm and Sonic/Sonic.asm only reference `AI.ROUTINE.*` input
routines (excluded as AI-input data).

Port marker check:
- `grep REMIX-TODO(AI)` in decomp/port: only `decomp/src/ft/ftremix/ftremixhandicap.c:14` (AI reads the handicap byte).
- No `REMIX-TODO(cpu`* marker exists for fighters.
- `REMIX_SYM_*__cpu_post_process` / `cpu_attack_weight` appear only as address defines in
  `port/remix/remix_symbols.h` (for example lines 277, 593, 855); they are not used as code.

## 4. How the CPU level reaches fighters

- Remix toggle: `Toggles.asm:2403` `entry_default_cpu_level` ("Default CPU LVL (V.S.)", range 0..10, default 0).
- Asm applies it once per boot in `AI.asm:21` (`force_default_cpu_lvl_setup`) and refreshes on A-press in
  `Toggles.asm:1740` (`set_cpu_level`, writes RAM 0x800A4D28 for four slots, sets `AI.is_default_cpu_lvl_set`).
- Port: `decomp/src/gm/gmremixspawnsetup.c:243` `gmRemixDefaultCPULevelSetup()`. It reads
  `gmRemixToggle(default_cpu_level)`, uses `GMREMIX_DEFAULT_CPU_LEVEL` (3) when the toggle is 0, and writes
  `gSCManagerTransferBattleState.players[].level` once per boot. Called from `decomp/src/mn/mnplayers/mnplayersvs.c:5721`.
- The A-press refresh (`set_cpu_level`) is **not ported** (comment at gmremixspawnsetup.c:240).
- `ftcomputer.c` uses `this_fp->level` / `fp->level` directly: input_wait (lines 3693-3718), damage size (3979),
  charge and SD decisions (3529, 4258, 4362, 5192, 5214, 5247, 5443, 5510). Acceptance-relevant (specials and recovery)
  paths are these plus the missing Improved AI recovery branches above.
- Separate port enhancement, not a Remix toggle: `port/enhancements/CpuLevel9.cpp` (CVar `gEnhancements.CpuLevel9`)
  forces level 9 via `port_enhancement_cpu_level_9()` in `decomp/src/sc/scmanager.c:1445` and `mnplayersvs.c:5365-5367`.

Gap for T7.1 acceptance (level 9 CPU uses specials and recovers, 10 Remix fighters): level is wired, but the
Remix CPU post-process and attack-weight routines are not, and Improved AI recovery branches are not read. ftcomputer.c
has only three `REGION_REMIX` blocks (lines 6, 6911, 6927).

## 5. Hooks outside AI.asm and ftcomputer.c (cheap grep only)

- Items: no `Computer`/`com->`/`ftComputer` reference in `decomp/src/it` (grep empty). `Item.asm` has no AI/cpu refs.
- Stages/hazards: `Hazards.asm:269` extends the AI stage check so CPUs walk away from tornados on Hyrule Remix;
  `Hazards.asm:3615-3631` sets the CPU STATE to RECOVER. Not checked against the port (not verified in decomp).
- Skipped: a full search of `decomp/src/gr` and `decomp/src/it` for hitbox/polygon CPU logic. Only the cheap grep above was done.

## Open items for T7.1

1. Wire `ftRemixSingleButtonCpuAOrBPress` / `ftRemixSingleButtonCpuZOrLPress` into the CPU input path (no caller today).
2. Port `gmRemixToggle(improved_ai)` consumers: recovery_fix_, fix_remix_recovery, evade_fix_, fix_target_pos_x, _check_respawn, z_cancel_, random_teching_, cpu_di_.
3. Assign the `ai_*_routine` registry slots, or call the Remix routines per fighter (cpu_post_process, cpu_attack_weight, cpu_pull_out_bombs_ already done).
4. Port `set_cpu_level` (A-press refresh) if the Remix menu refresh is required.
5. Verify the Marina charge condition in `ftcomputer.c:6938` and the Improved AI title in the runtime registry.
