# F14 notes: Training custom menu (ROADMAP T8.2, steps 10 to 13 of T6.3) and CPU DI

Source: `.toolchain/pkgs/smashremix-src/src/Training.asm` (entries 3899-3913, `shield_break_mode_` 2308, `di_practice_mode_` 2457,
`spam_practice_` 2496, `spam_practice_timer_` 2600, OOS tables 3014-3042) and `AI.asm` (`cpu_di_` 6533, `di_input` 2039, tech table 443-660,
Spam scripts 1917-2010).

## Update 2026-11-04: the Z menu is ported

The pause Z key now opens the Remix Z menu itself (`sc1ptrainingremixmenu*.c`, `sc1ptrainingmenurules.*`,
`sc1ptrainingmenuentries.*`, `sc1ptrainingremixspawn.*`): 23 entries on 2 pages with the `Training.asm` layout and the
`Menu.asm` navigation. It reads and writes the same registry below, so the port menu (**Remix > Training**) and
`SSB64_REMIX_TRAINING` still work. Seen in run `v3_trzmenu` (ROADMAP Fase 2). The decision below is kept as history.

## Update 2026-11-05: Fase 5 leftovers (Training.asm)

1. **Custom spawn on the respawn after a KO: the asm does not do it, the port matches.** `init_character_` (Training.asm 399-463) is the hook
   at 0x800D7A1C inside `ftManagerInitFighter` and tests `ra` (stack slot 0x34, `lw t0, 0x34(sp)`) against 0x800D86B4. That address is the
   return of `jal ftManagerInitFighter` at 0x800D86AC in `ftManagerMakeFighter` (checked with `tools/remix_disasm_rom.py`), so the facing,
   the ledge clipping id and "Reset Sets Percent" apply only when a fighter is created (match start, quick reset, Reset). The halo
   respawn (`ftCommonRebirthDownSetStatus`) calls `ftManagerInitFighter` from its own address, takes `_end`, and uses the stage halo
   position and 0% (`Spawn.asm load_respawn_point_` 18-60 only fixes the beta stages). The comment "skip if respawning" and the older
   notes ("respawn call site") name the creation path. Nothing to port: the KO respawn keeps the halo, as in Remix.
2. **Per-port dummy behaviour.** Shield Break, OOS, Spam, DI Practice, Percent, CPU DI and CPU Teching were already per fighter (state
   arrays indexed by `fp->player`, menu values global, applied to every CPU like `AI.asm` 525-545 and 6540-6650 read them). What followed a
   single CPU was the CP option of the vanilla pause menu (Stand, Walk, Evade, Jump, Default): `obey_cp_state_` (506-544) loops the four
   ports and calls the vanilla routine for every port whose type is CPU, so `sc1PTrainingModeUpdateDummyBehavior` now sets the behaviour
   of every CPU fighter. `disable_during_pause_` and `enable_during_unpause_` (548-622) lock and unlock every port that is not N/A, now
   `sc1PTrainingModeSetPortsLocked` (the owner keeps the held-B rule of `sc1PTrainingModeCheckLeaveMenu`). The dummy port still drives the
   COMBO and damage displays like the vanilla HUD.
3. **Live music change.** Already working: the Music entry of the Z menu is a COMMIT entry; A runs `sc1PTrainingRemixPlayMusic`, the
   `play_bgm_` port (3823-3857): it stores the track as the stage track (`gMPCollisionBGMDefault`), plays it unless the current track is
   the hammer or the star, and keeps the pause volume (`syAudioPlayBGM` does not touch the BGM volume, the resume sets 0x7800). The
   `gmremixbgm*` helpers (`gmRemixBgmChange`) are not used on purpose: they also update `BGM.current_track`, which `play_bgm_` does not
   write. Still open: the 17 entries of the port are the fixed part of the table; the Remix list appends the toggleable MIDI tracks
   (assembler loop at 3774-3785 and 3809-3817, no symbol), so the track count of a full Remix build is longer than 17.
4. **L legend "Show Action" and "(Hold --)".** New `sc1ptrainingremixaction.*` and `sc1ptrainingremixexit.*`, pure rules in
   `sc1ptrainingleftoverrules.*` (`tests/remix/test_training_leftover_rules.c`).
   - Show Action (`run_` 1969-1978, `create_action_strings_` 1475-1565, `update_frame_counts_` 2097-2152, `update_actions_` 2156-2277): L
     in the pause flips the flag (it persists across resets like the asm word). Per fighter, the action name at (20, 55 + 26 * port) and
     the frame count at (20, 66 + 26 * port), scale 0.75; the count is `frames outside hitlag + the first hitlag frame`, restarts at 1
     on an action change, and runs right after `gcRunAll`. Names come from the Remix RAM image: ids below 0xDC use
     `Action.shared_action_string_table`, the rest `Character.action_string.table[fkind][id - 0xDC]` (null pointer = no text). Both
     labels are **missing from `port/remix/remix_symbols.h`**: the file uses guarded fallback macros with the addresses of
     `build/remixasm/remix.sym` (0x80402908 and 0x8049C700); add `"Action.shared_action_string_table"` and
     `"Character.action_string.table"` to `EXTRA_SYMBOLS` in `tools/gen_remix_actions.py` and the guards become dead code.
   - DEVIATION(overlay text): the asm draws the rows in room 0x39 under the translucent panel of the Z menu; the port overlay is always
     on top, so the rows are hidden while the Z menu is open.
   - L glyph at (20, 16) scale 0.875 with "Show" (20, 27.25) and "Action" (20, 38.5), group 0x15 like the Reset Count, shown with the pause.
   - Hold to Exit (`hold_to_exit_` 751-809, `run_` 2002-2038, `setup_` 1590-1611): with `Toggles.entry_hold_to_exit_training` on, the
     Exit option needs A held 33 frames on the pause owner's controller; the counter resets when the cursor leaves Exit
     (`_reset`). Text "(Hold --)" at (169, 166) scale 0.875 with the A glyph at (202, 165), and the blue bar at (73, 179), 2 high, +2
     wide per held frame up to 64, only with the cursor on Exit and the Z menu closed. The glyph group is 0x1C (the asm 0xF is the
     battle pause legend group in the port); the overlay text leaves out the dashes so the glyph is not crossed out.
   - Text slots 105..116 and rect slot 30 are in the map of `remix_hud_text.h`.
5. **Held item name.** The path is the one of the asm and was already complete: `sc1PTrainingModeItemDisplayProcDisplay` maps a held kind in
   0x2D..0x39 to entry `kind - 0x2D` (`Item.asm` `_fix_item_id_check` 330-339 and `_get_custom_image` 347-365, table 0x330 of file
   0xFE), now through `sc1PTrainingLeftoverRulesGetHeldItemEntry` with tests. The held name uses the HUD table, not the menu table
   that was seen. A new log line `training item hud: held entry N sprite WxH` is written when the sprite is set; a zero size or a
   missing line points to the table or to the held kind.

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
