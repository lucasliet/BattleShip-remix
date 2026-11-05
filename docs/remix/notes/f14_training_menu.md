# F14 notes: Training custom menu (ROADMAP T8.2, steps 10 to 13 of T6.3) and CPU DI

Source: `.toolchain/pkgs/smashremix-src/src/Training.asm` (entries 3899-3913, `shield_break_mode_` 2308, `di_practice_mode_` 2457,
`spam_practice_` 2496, `spam_practice_timer_` 2600, OOS tables 3014-3042) and `AI.asm` (`cpu_di_` 6533, `di_input` 2039, tech table 443-660,
Spam scripts 1917-2010).

## Update 2026-11-04: the Z menu is ported

The pause Z key now opens the Remix Z menu itself (`sc1ptrainingremixmenu*.c`, `sc1ptrainingmenurules.*`,
`sc1ptrainingmenuentries.*`, `sc1ptrainingremixspawn.*`): 23 entries on 2 pages with the `Training.asm` layout and the
`Menu.asm` navigation. It reads and writes the same registry below, so the port menu (**Remix > Training**) and
`SSB64_REMIX_TRAINING` still work. Seen in run `v3_trzmenu` (ROADMAP Fase 2). The decision below is kept as history.

## Decision (superseded): the menu is the port menu

The Remix Z menu is drawn by the Remix menu framework (`Menu.asm`), which the port does not redraw (same decision as the Tag Team queue and the
per-port CSS panel). The entries are a registry stored as CVars `gRemix.Training.<Key>` and edited in the port menu, **Remix > Training**
(`port/remix/remix_training.*`, `PortMenu::AddMenuRemixTraining`). `SSB64_REMIX_TRAINING="ShieldBreak=2;SpamPractice=11;..."` overrides them in a
run. The Z key does not open anything in the match. Effects are real: each entry changes the fighters as in the asm.

| Entry | Key | Port state |
|---|---|---|
| Shield Break Mode (Off, On, Infinite) | `ShieldBreak` | done, `ftremixtraining.c` + `sc1ptrainingrules.c` (state machine), seen: Infinite holds the CPU in shield |
| OOS Action (12) | `Oos` | done (tables of both frames, wait for the last jumpsquat frame for DSP and NAir), unit tests; not seen in game (needs a hit on a shielding CPU) |
| Music | not in the registry | **open**: needs the 0x8019xxxx `bgm_table` and the music toggle list (`play_bgm_`) |
| CPU Teching (7) | `Tech` | done, `ftRemixAiTechByMenu` in `ftremixaiteching.c` (backward, forward, in place, left and right relative to the facing, none); the failed tech now counts `VsStats` TechMiss |
| CPU DI Type, Strength, Direction, Start on Hit | `DiType`, `DiStrength`, `DiDirection`, `DiFirstHit` | done with `cpu_di_`, see below; not seen in game |
| D-pad Controls (On, Reset Only, Off) | `Dpad` | done, `sc1PTrainingRemixPollDpad` |
| DI Practice Mode | `DiPractice` | done, hitlag 2 and `ftCommonDamageCommonProcLagUpdate` on every fighter that is past the KO statuses, each frame the objects run |
| Spam Practice (13 moves), Interval, Interval Random | `SpamPractice`, `SpamInterval`, `SpamRandom` | done, seen: Jab every 40 frames (18 spam log lines in 700 frames); the timer is one byte like the asm (999 wraps), counts only CPU outside hitlag, and Spam is ignored while Shield Break is on |
| Percent and Reset Sets Percent (per port, P1 to P4) | `Percent1..4`, `ResetSetsPercent1..4` | done: a fighter created for the match takes the percent when Reset Sets Percent is on (`ftRemixPanelGetInitialDamage`, the asm `init_character_`), and changing Percent in the menu resets the fighter to 0 then adds it (`set_percent_`); **seen**: `Percent1=88;Percent2=123` starts the match at 88% and 123% |
| Stage Background | `StageBg` | done, `grWallpaperMakeDecideKind`; **seen** (Dream Land background instead of the training one); the magnifying-glass background of the asm is not done |
| Preserve Menu Selections | none | **deviation**: CVars always persist across resets, so the entry has no work to do |

Not done (per-port chain of the Z menu): Character, Costume, Type (Human, CPU, Disabled), Spawn, Set Custom Spawn.
They change the roster of the match mid-run (`load_character_`, `init_character_`, `set_custom_spawn_`), not just numbers. Open in T8.2.

## Deviation: Shield Break only for CPUs

`shield_break_mode_` runs after the player input of every fighter and has no controller-kind check, but its comment says "Forces CPUs to
shield". On a human it would lock the player into the shield, so the port applies it to CPU fighters (`ftRemixTrainingShieldBreakInput`).

## CPU DI (`cpu_di_`, closes the AI leftover "cpu_di_")

`ftremixaidirules.*` (pure, 16 tests with mutation check) decides; `ftremixaidi.c` reads the fighter. Hook: `ftComputerProcessObjective`, before the
objective switch, then the Spam hook (the asm hooks 0x8013A3C8 and 0x8013A3D0). Facts read in the asm:

- The hit count is `SCPlayerData.combo_count_foe` (match_info + 0x74 + 0x74 * port is player + 0x54).
- "hitstun" (`lw 0x40(a1)`) is `hitlag_tics`; the DI is applied during hitlag, which is why DI Practice forces hitlag 2.
- Parameters per port (type, strength, direction, advanced index) persist through a combo (`hit_count != 0`). The asm **reads** the stored values
  but **writes** a local copy that starts at -1 outside a combo; the port keeps that quirk.
- Training uses the menu. Outside Training: level 10 rolls 0 to 8 (0 none, 1 to 4 smash high, 5 to 8 slide high); other levels need the advanced AI
  (Remix 1P, or Improved AI outside classic 1P), level 1 to 3 never, 4 to 6 low strength, 7 to 9 medium.
- Scripts are built from the `di_input` macro (smash: stick tap and release; slide: the same tap with the other axis swept), wait nibble 1, 3 or 5.

## Also closed here (AI.asm random_teching_)

`ftRemixAiIsRemix1P` = `sc1PRemix1PIsActive`, Single Button Mode = `gmRemixToggle(single_button_mode)`, off-stage = FTStruct 0x018D mask 0x04 =
`is_magnify_show` (the asm tests the halfword at 0x018C with mask 4).

## Harness

`SSB64_TRAIN_FKINDS=<human>,<cpu>` boots a Training match without the CSS (`remix_debug_train_fkind`, `scmanager.c`), stage from `SSB64_VS_STAGE`.
Spam log line: `training spam: port N option N timer N`.

## Test in game

1. `SSB64_TRAIN_FKINDS=0,8 SSB64_REMIX_TRAINING="ShieldBreak=2"`: Kirby shields (seen).
2. `ShieldBreak=1;Oos=3` and hit the shield: after the stun Kirby grabs.
3. `DiType=3;DiStrength=1;DiDirection=1;DiFirstHit=1` and hit the CPU: stick wiggle in hitlag.
4. `Tech=4` and spike the CPU on the floor: it rolls left.
