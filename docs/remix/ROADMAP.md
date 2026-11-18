# Smash Remix 2.0.1 in the PC port: roadmap

This is the **only plan** of the Remix work. Whoever works on the Remix (agent or human) follows this file, without
opening work outside it: new work first gets a milestone here. The Remix screens stay 4:3: widescreen is **out of scope**
for the Remix work (the port's own optional Widescreen setting only widens battle scenes). The reference documentation
is indexed in [`README.md`](README.md).

## How to use (work protocol)

1. At the start of each session, read **Current position** and any open milestone.
2. Work only on the first open item of the current milestone. A blocked item gets `BLOCKED: <reason>` and the next one is
   done. New work (a bug batch, an audit) opens a new milestone after the last one, with its tasks, before any code
   changes.
3. An item is ticked `[x]` only after it passes its acceptance test: the build, the unit tests, and an in-game smoke test
   (skill `remix-smoke-tests`) that the lead runs on the build that will be committed. A fix with pure rules also gets a
   unit test in `tests/remix/`, on top of the smoke test, never instead of it. A fix that touches several screens, modes
   or call sites is smoke-tested on each of them. Note the short evidence next to the item (run name, counts, what the
   screenshot shows), and set its state in `patch_audit.md` when it is a ledger item.
4. End of a milestone or phase = **checkpoint**: bug write-up in `docs/bugs/` when one applies (with its index row),
   commits in the decomp and in the main repository **without attribution trailers**, update of **Current position**.
   It is also a checkpoint when the user writes "checkpoint".
5. Workers (Agent) run in parallel, one package or batch each (disjoint files), with a brief written for it (not kept in
   the repository). They write code, research and their own before/after runs; the lead reruns the smoke tests, signs
   off and commits. A batch closes with its own commit.
6. A significant bug fixed becomes `docs/bugs/<topic>_<YYYY-MM-DD>.md` with a row in `docs/bugs/README.md`.
7. Reply to the user in Portuguese; everything written in the repository is in English.

## Fixed rules

- Toolchain and dependencies only in `.toolchain/` (no brew, system pip, sudo, `/opt`, `/usr/local`, `~/.cache`,
  `~/.cmake`). Python: `.toolchain/host/venv/bin/python -I`. `builtin cd`.
- Build with the `.toolchain/` cmake and ninja and absolute paths: `cmake --build <build dir> --target ssb64 -j <n>`; the
  configure line of each platform and the build directories are in `build_and_roms.md`. A new file in `decomp/src` needs
  the configure step again.
- ROM-derived bytes never go into versioned files. Authorised source: `.toolchain/pkgs/smashremix-src`.
- The asm is the reference, except where it is itself wrong: an asm bug, a missing piece or something the port already
  does better is kept or fixed in the port, never copied, and recorded in `deviations.md` (owner policy, 2026-11-15).
  No fallback to the parent fighter and no containment hacks; a necessary deviation is recorded in `deviations.md` (or
  a `docs/bugs/` write-up when it comes from a bug).
- Code in English, no `//` comments that repeat the code. `#ifdef REGION_REMIX` inside vanilla files.
- References in comments: `torch/` and `libultraship/` are independent repositories, so their comments do not cite docs
  of this repository. `decomp/` may cite `docs/remix/*.md` and `docs/bugs/`. Durable findings go into the topic doc
  (`docs/remix/<topic>.md`), never into per-task notes or `build/`.
- Tests: skill `remix-smoke-tests` (`.claude/skills/remix-smoke-tests/`: `vs.sh`, `scene.sh`, `css.sh`,
  `parallel.sh`, `special_matrix.sh`, `stage_sweep.sh`, `item_cpu_sweep.sh`, `sheet.py`, `lldb_count.sh` on macOS and
  `gdb_count.sh` on Linux, the inputs and how to read results), runs in `build/agent-shots/vs/`; reference emulator
  `scripts/ref-emu/capture.sh` (`notes/i8_reference_emulator.md`). A `CAMetalLayer nextDrawable` crash is a macOS
  flake: rerun before investigating.

## Current position

**M16 is open (ledger re-audit follow-ups, 2026-11-18); M0 to M15 are closed.** The Remix is playable
on every platform the releases ship, and the owner validated the listening pass, the Switch hardware and the updaters
on 2026-11-15. Releases are not tracked here: they follow `AGENTS.md` ("Releases") and `build_and_roms.md`.

| Milestone | State |
|---|---|
| M0 to M10 | closed; their user passes T9.1b and T10.2 validated by the owner (2026-11-15) |
| M11 Finalization | closed 2026-11-06: "Remix playable" (T9.3) reported; ledger `patch_audit.md` 266 `done`, 4 `n/a`, 0 `open`; `parity.md` 277 `ported`, 2 `adapted`, 10 `n/a`, 0 `partial` |
| M12 Platforms and releases | releases from `release-remix.yml` (desktop, Switch, PS5, Android) |
| M13 Toggle parity audit | closed 2026-11-15: 35 findings in 10 of the 11 batches, each fixed with a lead smoke test or kept as a `deviations.md` row |
| M14 Equivalence audit of the non-toggle logic | done 2026-11-17 (T14.1 to T14.6 on 2026-11-16, T14.7 on 2026-11-17): 23 pilot findings, 116 from the 22 audit batches and 36 from the second pass on the parts first left uncovered, each fixed with a lead smoke test, kept as a `deviations.md` row or not reproduced; G04-4 (Random in the 12CB Custom set) implemented under T14.7 |
| M15 Remix text in the game's display layers | done 2026-11-17: every Remix text is drawn by the game in the asm's display layers; the port overlay is gone |
| M16 Ledger re-audit follow-ups | open: T16.1 done 2026-11-18 |

- **Next step:** owner passes on the migrated screens; before any tag, build Windows and macOS (not buildable on the Linux machine) and rewrite the release notes Highlights.
- **Last checkpoint:** T16.1 (2026-11-18): the `prevent_results_bgm_on_css_` guard ported (`docs/bugs/results_bgm_replay_on_exit_2026-11-18.md`).
- **Overall estimate:** 100% by the table at the end of this file, which counts presence (every asm patch hooked); the
  equivalence of the non-toggle logic is what M14 measures.

## Owner validations

The passes only the owner or a console can do; all closed on 2026-11-15.

- [x] T9.1b Listening and visual pass by the owner (the lead cannot hear): stages, victory songs, Sound Test, hit SFX,
  menu graphics; `t9_1b_checklist.md`. Done: validated by the owner (2026-11-15).
- [x] T10.2 Switch hardware validation (`docs/switch_port_2026-10-12.md`, rounds R1 to R6 under **M10**). Done: validated by the owner (2026-11-15),
  after the display-list walk fix of 2026-11-14 (`docs/bugs/dl_walk_past_window_false_positive_2026-11-14.md`), the fix
  for the `gfx_step: walked past registered DL range` stutter. Metal Mario tiny on the Training CSS was never
  reproduced.
- [x] M12 updaters on a real install: the Switch zip update and the macOS in-place update. Done: validated by the owner (2026-11-15).

---

## History and decisions (M0 to M10)

Standing decisions:
- No fallback to the parent fighter and no containment hacks: behaviour equals the Remix. A necessary deviation is documented in `docs/bugs/`.
- Widescreen is out of scope (4:3). ROM-derived bytes never enter versioned files; Remix data is read at runtime from the Remix RAM image (`remix_ram_image`, symbols from `tools/gen_remix_actions.py`).
- The Remix menu framework (`Menu.asm`) is not replicated; the port menu (Remix section) has the same entries and values.
- Fighter ids equal the Remix fkind; registries, tables and the save are sized for 97 fighters.

### M0. Governance
One roadmap and one parity table.
- [x] T0.1 Single roadmap, referenced from `AGENTS.md`; `ROADMAP.md`.
- [x] T0.2 Per-`.asm` parity table; `docs/remix/parity.md` from `tools/gen_remix_parity.py`.

### M1. Stabilise roster, items, stages
All Remix stages load; the 15 ported hazards and the standard items behave in a match.
- [x] T1.1 Mementos (59) loads: relocation-chain fixups stop at the first pending slot; `portRelocBeginChainWalk`; `docs/bugs/reloc_chain_fixup_overwrites_pending_slot_2026-10-14.md`.
- [x] T1.2 Dream Greens (149) crash fixed in `grpupupu.c` (vanilla Dream Land untouched).
- [x] T1.3 Standard items verified in match; `docs/bugs/remix_standard_items_2026-10-14.md`. Psyduck is disabled in Remix 2.0.1 (n/a).
- [x] T1.4 Fifteen hazards verified in match; `gr/grremix`, `it/itremix`.
- [x] T1.4b Subsystem-free hazard `REMIX-TODO`s (KlapTrap, Robot Bee, Cacodemon animations, Pirate cannon, Rainbow chomp, Casino bumper); `gr/grremix`.
- [x] T1.5 Remaining hazards: `grremixtimetwister.c`, `grremixscuttle.c`, ScuttleMinion/BulletBill (items 0x3E/0x3D, `itremixitems.c`), Peach Castle II Banzai factory, Bowser's Stadium bombs, Yoshi clouds, Dream Land wind. Decisions: the 2.0.1 asm switches the Scuttle minion off (`b _end`), mirrored by `GRREMIX_SCUTTLE_MINIONS_ENABLED`; the Banzai Bill is dead code in 2.0.1 (nothing references `peach_castle_II_setup_`), so the factory stays faithful (Robot Bee 0x3B, no laser) and unreachable (`hazards.md`).
- [x] T1.5b Time Twister (210) and Scuttle Town (191) background/warp-ball fades. The source lists it open in M1, but M11 Phase 1 I6 reports it complete and seen on screen; `gr/grremix/grremixstagebg.*`. (State: done in M11.)
- [x] T1.6 Stage select with 85 stages in 10 pages; Training never reads outside the vanilla tables (`mnmaps.c`).
- [x] T1.7 Sweeps (80 fighters x NSP/DSP/USP, stages 41 to 215, 17 items) without crashes beyond the macOS `CAMetalLayer nextDrawable` flake.

### M2. Gameplay subsystems
- [x] T2.1 GFX + GFXRoutine (particle scripts, 50 colanim routines in `dGMColScriptsDescs` with `port_override`); `docs/bugs/remix_gfx_and_gfxroutine_2026-10-16.md`.
- [x] T2.2 CharEnvColor: `ftremixenvcolor.c`, read in `ftdisplaymain.c`. Deviation: the asm swaps the material display list for alpha (`custom_display_lists_*`); the port only uses env-colour alpha, so parts with special render modes (Yoshi tongue, GND shield) may lack the adjustment (reopened by the patch audit as GP-6).
- [x] T2.3 Reflect: custom hitbox (`FTSpecialColl.kind` = id << 16 | 3) with per-id routines; `ftremixreflect.c`. CPU hooks are M7.
- [x] T2.4 Size phases b and c (weights, speeds, jumps, armour, hitboxes, entries); `size.md`.
- [x] T2.4b Match size state (`Size.match_state_table`): `ftRemixSizeGetMatchState/SetMatchState` in `ftremixsize.c` (GIANT 2.25, TINY 0.5); Lightning, Metallic laser and Mushrooms restore the mode size; per-port state from the panel.
- [x] T2.5 PokemonAnnouncer: `if/ifremixannouncer.c`, `ifremixhitannouncer.c`. Naturalness is judged by T9.1b.
- [x] T2.6 Kirby: `ftremixkirbycopy.c` (Remix inhale table, 97 fighters), `ftremixkirbyhats.c`, `ftremixkirbypower.c`; `docs/bugs/remix_kirby_copy_actions_2026-10-16.md`.
- [x] T2.7 Other hit systems (Knockback, Poison, Damage, Surface, Blastzone, Camera, Spawn, CustomGrab, `command.asm`); `vanilla_patches.md`, `hazards.md`.
- [x] T2.7b Neutral spawns, blastzone warp, pause camera, conveyors (`grRemixSurfaceConveyorPush`), stun/flashbang (`ftremixdamagestun.c`), bury/plunge (`ftremixbury.c`).
- [x] T2.8 Piano `extra_part_visibility_`, Marina `prevent_item_despawn_`; `clear_items_in_1p` already covered.
- [x] T2.9 Containments kept as documented substitutes for undefined Remix behaviour: capture-joint fallback (`ftcommoncapturepulled.c`; Bowser's `joint_itemheavy_id` is 31, inherited from Yoshi) and NULL `throw_desc` (`ftcommonthrown2.c`; also moves without `SetThrow`, e.g. Banjo USP). On N64 the null pointer reads RDRAM page zero.
- [x] T2.9b Polygons' TransN guard. Closed in M11 Phase 3: motion rows have flags 0 in the Remix ROM; 0x40000000 comes from `ftPortParentHiddenJointFlags` (`ftport.c`); the guard in `ftmain.c` clears XRotN/TransN/YRotN in entry statuses.
- [x] T2.10 fkind audit: Kirby inhale reads `copy[victim_fp->fkind]` (also `efmanager.c`), Falcon Dive via `Character.falcon_dive_id.table`; the thrown-damage script uses the parent row (an index above 27 is undefined in the Remix).
- [x] T2.11 `REMIX-TODO` outside AI/modes/menus closed (M11 Phase 3). Raw `ITStruct` offsets mapped to named fields; `itremixblueshellrules.*`, `if/ifremixrectangle.*`. Reverted on purpose: Metallic laser and Marina rotation read 0x8019CAA0/0x8018C230 from the RAM image (starts at 0x80400000, returns 0); they stay DEVIATION/TODO with the reason.

### M3. Audio
- [x] T3.1 BGM: 375 sequences, per-fighter victory BGM (`mnremixwinbgm.c`), results load Remix fighter files (`mnvsresults.c`).
- [x] T3.2 SFX/voices: blobs `audio/fgm_tbl`, `fgm_ucd`; Sector Z hit-SFX pan (`lbcommon.c`).
- [x] T3.3 Sound test: `mnsoundtest.c`, `mnremixsoundtest.c` (375 songs, 1094 voices).

### M4. Settings and toggles
- [x] T4.1 Registry of 531 toggles with profiles CE/TE/NE/JP (`remix_toggles.cpp`), CVar `gRemix.Toggle.<n>`, `gmRemixToggle(<name>)`/`gmRemixToggleVS(<name>)` (`gm/gmremixtoggle.h`).
- [x] T4.2 Gameplay toggles, one file per system: about 60 toggles from Hitstun to Neutral Spawns, results toggles `gmremixgameend.c`, Menu Music `gmremixmenumusic.c`, Disable HUD `ifremixhud.c`, Model Display `ftremixhitboxdisplay.c`; `toggles.md`.
- [x] T4.3 Stage/item toggles: `grRemixHazardsAreOn`/`grRemixMovementIsOn`, `itRemixStandardItemIsOn`.

### M5. CSS, menus, screens
- [x] T5.1 VS CSS (`mnplayersvs.c`): 3x10 grid, costumes/shades, D-pad variants, Random Select; zoom crash `docs/bugs/remix_css_camera_zoom_null_battle_state_2026-10-20.md`.
- [x] T5.1b 1P/Training/Bonus CSS sharing `mnplayers/mnremixcss.c`.
- [x] T5.1c Stage select emblem (`mnmaps/mnremixemblem.c`) and variants on C-down (`mnmaps/mnremixstagevariant.c`).
- [x] T5.2 Per-port panel: `port/remix/remix_player_panel.*` (21 options, `SSB64_REMIX_PANEL`), read through `gmRemixPanel(OPTION, port)` in `gm/gmremixpanel.*`; `panel.md`.
- [x] T5.2b/T5.3b Panel and screen leftovers moved to T8.2.
- [x] T5.3 Screens: results no longer white (Metal backend lacked `SetColorWriteMask`; `docs/bugs/metal_redirect_z_color_write_2026-10-20.md`), Remix Modes page (`mnvsmode/mnremixvsmode.c`), Music Title/Input Display overlays, credits (`credits/remix/`), Character Data (`mndata/mnremixchardata.c`), Gallery (`mncommon/mnremixgallery.c`), VS stats (`mnremixvsstats.c`). Settings button, L shortcut and toggle menu are replaced by the port menu.
- [x] T5.4 Containment hacks (`docs/bugs/remix_css_and_parent_fallbacks_2026-10-12.md`): capture-joint and `SetThrow` guards log once; kept because inherited routines read those globals: CSS drop on unloaded file, figatree heap, parent `SetupFilesAllKind`.
- [x] T5.5 Save for 97 fighters (`GMCOMMON_FIGHTERS_SAVE_NUM`, `lbtypes.h`; `lbBackupInitRemixDefaults`): 8 KB to 123 KB; an old save fails the checksum and restarts.

### M6. Modes
Pure-rule modules with unit tests in `tests/remix/` (target `run_remix_mode_tests`).
- [x] T6.0 Mode base: `gm/gmremixmode.c`, `SSB64_REMIX_MODE`, Timed Stock, `port/remix/remix_hud_text`; `modes_base.md`.
- [x] T6.1a Stamina: `gm/gmremixstamina.*`. T6.1b Tug of War: `gm/gmremixtugofwar.*` (probable asm bugs in shield break and team credit checked before copying).
- [x] T6.1c King of the Hill: `gm/gmremixkoth*.c`, `mnremixgoalpicker`. Fix: team score counts once per frame (the asm summed per player).
- [x] T6.1d Tag Team: `gm/gmremixtagteam.c`. Decision: queue not drawn in the CSS (slot 0 from the CSS, slots 1 to 5 from the port menu, Random by default); Phase 5 later drew it in the CSS. The Remix does not avoid repeats in the queue (`_recent_randoms_lookup`, window of 6), so no repeat rule is ported.
- [x] T6.1e Smashketball: `gm/gmremixsmashketball*.c`.
- [x] T6.1f 12-Character Battle rules: `gm/gmremixtwelvecb*.c` (game vector of 48 instead of the asm's 24, which overflows). Visual was a provisional overlay until Phase 2. Fix: the START block once "seen" was a false positive; `mnplayersvs.c` now closes slots 3 and 4 in 12CB.
- [x] T6.2 1P modes (`sc/sc1pmode/*`, `mn/mn1pmode/mnremix1pmode.*`, `SSB64_REMIX_1P_MODE`):
  - Bonus 2 exit `sc1premixbonusexit.*`; Multi-Man/Cruel `sc1premixmultiman.*` (KO record `multiman_kos/cruel_kos`; `_percent_fix`, `crash_prevent` n/a); HRC `sc1premixhrc*.*` (record `LBBackup1PRecord.hrc_distance`); `docs/bugs/remix_1p_hrc_and_path_cp6e_2026-10-27.md`.
  - R1 data from the RAM image `sc1premix1pdata.*`; R2 draw `sc1premix1prules.*` (deviation: the asm loop that never ends returns -1 and the draw picks another fighter); R3 per-stage match (`docs/bugs/remix_1p_size_override_cp6f_2026-10-27.md`; deviation: `_opponent_cpu` compares against an overwritten register, so CPU level is 10 on Very Hard everywhere); R4 title cards `sc1premixintro.*`; R5 Giga Bowser final (`sc1PRemix1PBossHitPoints`: 200/250/300).
  - A1 All-Star: `sc1premixallstar.*` (order, progress, Rest Area, hearts, 24-member final, end and credits).
- [x] T6.3 Training (the Remix has no save state): D-pad freeze/advance/model/quick reset and Reset Count `sc1ptrainingremix.*`, 13 extra items in the pause item row.
- [x] T6.D1, T6.D2, T6.D3, T6.D4: test shortcuts `port/remix/remix_debug.*`; `gmremixmode.c` split; mode-rule unit tests (`remix_mode_tests`); mode files above 300 lines split.
- [x] T6.D5 Review of worker code. Deviations: 127-stock cap in Tug of War (asm wraps at -128); `last_hit` indexed by victim port, not KOer team (`TugOfWar.asm:302`).
- [x] T6.D6 Tag Team icons and stock steal `gm/gmremixtagteamicons.*`; `docs/bugs/remix_tagteam_icons_and_link_boomerang_2026-10-29.md`.

### M7. AI
Finding: the 30 Remix fighters used the parent's attack list (`remix_roster.cpp` copies the descriptor); Remix tables are in the image (`Character.ai_behaviour.table`, `lv10_ai_behaviour.table`).
- [x] T7.1 AI.asm (70 hooks), Improved AI, Single Button Mode CPU hooks, `VanillaLv10Attacks.asm`; `ft/ftremix/ftremixai*.c` with small hooks in `ftcomputer.c`; `docs/bugs/remix_routine_gobjs_on_effect_link_2026-10-29.md`.
  - [x] T7.1a attack tables `ftremixaitables.*`. [x] T7.1b custom input scripts `ftremixaicommands.*` (deviation: TURNAROUND_WAIT waits for Turn or TurnRun; jump table became a `switch`). [x] T7.1c PREVENT/LONG_RANGE `ftremixaiprevent.*`, `ftremixailongrange.*`. [x] T7.1d combat/recovery (`ftremixaiteching.*`, `ftremixaispecials.*`). [x] T7.1e level 10 and DI (`ftremixailv10.*`, `ftremixailevel.*`). [x] T7.1f CPU level to 10 in the CSS (`mnremixcpulevel.*`).
  - AI leftovers (`REMIX-TODO(AI)`) are Phase 6.

### M8. Remaining parity
- [x] T8.1 (absorbed by M11 Phase 4) Ported: AerialFastFall, Skeleton, timeouts (`mnremixtimeouts.*`), tournament rules (`sc/scremixtournament.*`), SFXReplace, Practice overlays, DK Mode (`ftremixdkmode.*`; a port variable because the RAM image is read-only), Sword Trail, MagnifyingGlass (`gmremixmagnify.*`), VsDemo (`scremixvsdemo.*`), Accessibility, Hitbox (`ftremixhitbox.c`), Japan fixes (`ftremixjapanfix_reflect.*`). Cheats not needed (unlock-all and music are port toggles); Walljump is inactive in 2.0.1.
- [x] T8.2 Leftovers of M5/M6/M7, mostly absorbed by M11 Phases 4 and 5 (the source left it unticked). Done here: Training Z menu as port section (`remix_training.*`; deviation: CPU only), `cpu_di_` (`ftremixaidi.c`), result rows, Stock Mode Last (`gmremixstockmode.c`), pause legends (`ifremixpauselegend.*`), handicap rules (`mnRemixHandicap*`), Training Music, per-port Training chain (`sc1ptrainingportrules.*`; deviation: option **Auto** keeps the CSS value).
- [x] T8.3 Visual fidelity of mode texts/HUDs: executed as M11 Phase 2.

### M9. Final desktop verification
- [x] T9.1 Full sweep: the automatable part is done (sweeps, 4-player matches in six modes, Training through the CSS); the regression pass is M11 Phase 8. Defect fixed: Salty Runback/R+L combo needed the held state **equal** to the mask (`Joypad.check_buttons_all_`); `gmremixgameendinputrules.*`.
- [x] T9.1b Listening pass by the user: validated by the owner (2026-11-15) (see **Owner validations**).
- [x] T9.2 Vanilla `REGION_US` builds and plays: CMake of `us`/`jp` skips Remix-only sources (`ft/ftremix`, `gr/grremix`, `it/itremix`, `port/remix/`, keeping `remix_debug`); `REGION_REMIX` guards in `ftport.c`, `port.cpp`.
- [x] T9.3 Report to the user in Portuguese ("Remix playable"), 2026-11-06 after Phase 8; adaptations listed in
  `deviations.md` and the `adapted` rows of `parity.md`.

### M10. Switch
- [x] T10.1 `scripts/switch/build.sh remix` (`BattleShip-Remix.nro`; fix: `<math.h>` before `<ft/fighter.h>` in `ftremixaipostsheik.c`) and `scripts/switch/pack.sh`.
- [x] T10.2 Hardware validation by the user (`docs/switch_port_2026-10-12.md`), validated by the owner (2026-11-15); six rounds of findings:
  - R1 black screen/slow audio: `gfx_vtx_addr_is_unresolved` rejected pointers below 4 GB (`libultraship/src/fast/interpreter.cpp`). R2/R3 title offsets (`mntitle.c`); the odd 1P Game was Classic Co-op, now off by default in `REGION_REMIX`.
  - R4/R5 CSS `menu_zoom` (`tools/gen_remix_roster.py`), entry statuses from `Character.entry_action.table`, default save masks (`scmanager.c`), watchdog/slow-frame logs (`port/port.cpp`).
  - R6 Tag Team crash: `sSCVSBattleStatusBuffer[100]` (`scvsbattlefiles.c`) overflowed with 24-fighter queues; 1024 under `REGION_REMIX` plus a critical log in `lbreloc_bridge.cpp`.
  - Last two: the `gfx_step: walked past registered DL range` stutter (display-list walk fix of 2026-11-14) and Metal Mario tiny in the Training CSS (never reproduced).

---

## M11. Finalization (approved 2026-11-03)

End criterion: "Remix playable" with **all** Remix features present. An adaptation to the port is accepted when the feature
exists for the player and the deviation is documented in `deviations.md`. `n/a` only for the approved items: the Spawned
Fear Practice overlay (no observable effect) and Advanced Hurtbox with the `Hitbox.asm` debug patches (debug toggle only).
The credits speed assist, approved as `n/a` on 2026-11-03, turned out to be ported (`scstaffroll.c` calls
`sc1PRemixAfkIsSpeedAssistActive`); the Character Data emblem and move names left the `n/a` list on 2026-11-05 because the
asm has them. In `parity.md` every Remix file ends as `ported` or `adapted`, never `partial`.

User decisions (2026-11-03): the reference emulator lives in `.toolchain/`, outside git (I8); test packages embed the `.o2r`
(personal use, marked in the file name and script header), the final publishable packages carry no `.o2r` and extract it
from the user's ROM on first run; CI builds `remix` in the matrix of `.github/workflows/release.yml` (superseded: releases come from `release-remix.yml`, see **M12**).

Closed phases:

- [x] **Phase R** (recover interrupted work): Kabel HUD of the HRC, button sprites as native SObj (`menus.md`), versioned package scripts (`scripts/linux/`, `scripts/macos/package-remix-contained.sh`; `build_and_roms.md`), old subagent work re-checked, current `parity.md` (`adapted` counts as done).
- [x] **Phase 1** (infrastructure), all but part of I7: I1 display groups `gmremixgroupdisplay*`; I2 extension sprites per SObj `if/ifremixsprite*`; I3 Kabel text `port/remix/remix_font_layout.*`; I4 runtime BGM `gm/gmremixbgm*`; I5 `tools/n64_addr_to_symbol.py`; I6 stage group 2 and camera `gr/grremix/grremixstagebg.*` (closes T1.5b); I8 reference emulator `.toolchain/ref-emu/` (`scripts/ref-emu/capture.sh`, `notes/i8_reference_emulator.md`). I7 part image/palette swap (`ftremixpartimage*`, `ftremixyoshiegg.c`, `notes/i7_part_image.md`) delivers extra costumes in match and in the CSS cycle and the Yoshi shield/escape egg colour; the thrown, laid and entry egg colour followed in Phase 5b lot 3 (below). Classic Sonic is not I7 (main-file swap); done in Phase 5.
- [x] **Phase 2** (visual of the modes, T8.3): all `remix_hud_text_*` callers use the asm parameters; Match Stats (sprite buttons, Kabel page, stock icons); pause legend; 12CB CSS steps 1 to 19b (`css_reference.md`; Gentlemen's Reset `if/ifremixgentlemensreset.*`, Custom set and StockMode Manual in `mn/mnplayers/mnremixtwelvecb*`); Multi-Man HUD sprites; HRC counter; Training item-name image, D-pad legend and Z menu of 23 entries (`sc1ptrainingremix*`); results labels (`mnVSResultsMakeRemixRow`); KOTH meter; music-title banner (LUS toast and `remix_overlay.*` removed). Deviations: "Press Z" opens the port menu in the prompt (`remix_training_open_menu`); the Dragon King Multi-Man HUD, the Smashketball result row and the 12CB CPU-opponent reset branch were checked by pure rules only. Empty-panel Smash emblem: no Remix patch draws it (nothing to port).
- [x] **Phase 3** (stages and gameplay, T2.9b, T2.11): Size of Ness (`ftremixsizeness.*`) and entries, Banjo USP wave, Goemon cloud, Yoshi `stage_hazard_table`, Rainbow Chomp, Metallic laser, Pirate cannonball (its speed does not exist in the asm; `cannonball_properties_struct` is dead data), Whispy wind box (`grRemixDreamLandGetWindBox`), Z-cancel cruel (`docs/bugs/remix_zcancel_egg_lp64_2026-11-04.md`), D-pad Control `ftremixdpad.c`, Corneria laser, Marina throw_turn, throw announcer, Shield Costume `ftremixshieldcostume.c`, Hitbox display. Zebes/Japes fields proved not portable. Deviations: NKirby keeps the vanilla fthrow (apparent Remix bug, compare in the emulator in Phase 8); Big Boo background sprite (`bg_animation_`) not ported; Kirby Cube pair `n/a`.
- [x] **Phase 4** (CSS, stage select, panel, screens; T8.1/T8.2): title "Smash Remix v2.0.1" (`mnremixtitleversion.*`), credits glyphs and speed assist, Gallery with 40 (`mnremixgalleryrules.*`), item-switch legend, RANDOM ALL, transition logos; stage-select grid of 5 pages with roulette and tournament layout (`mnremixstage*`); panel features (D-pad Map, DpadControl, tag names, Start With, Practice 1P, Skip Start Cheer, Enemy Player/AFK, Link JP sounds, CPU costumes); VS CSS bookends, variant icons and J/E flag (`tools/test_remix_css_offsets.py`), slide-in `mnremixcssslide*`, high-score gating `sc1premixhighscores*`; per-port panel menu in the CSS (`mn/mnplayers/mnremixcsspanelmenu*`, `panel.md`); CSS crash fix `docs/bugs/remix_css_bookend_width_img_2026-11-04.md`. Deviations: Roulette ignores directions and draws among the unlocked stages of the page; tournament Random list not ported (reopened: ST-23); port RNG; no `dont_freeze_stage` or Settings shortcut; the CSS model does not reload on Model/Kirby Hat/Visibility/Size changes (reopened: MN-20). Handicap icons above 9 moved to Phase 6.
- [x] **Phase 5** (modes): CSS mode headers, announcers, GOAL/Time picker, Tag Team queue drawn in the CSS (`css_reference.md`, `mnremixcssmoderules`); Classic Sonic (`CSONIC_MAIN`, `deviations.md`); yellow team (`css_reference.md`); Tag Team/Last/Tug of War/Stamina (`modes/tagteam.md`, `modes/stamina.md`, `modes/tugofwar.md`; `docs/bugs/remix_tugofwar_asm_deviations_2026-11-05.md`); KOTH and Smashketball visuals (`modes/koth.md`, `modes/smashketball.md`); 1P leftovers (`modes/singleplayer.md`: records, Remix BTT/BTP, Bonus 3, Rest Area, Giga Bowser end); result extra row (`gmRemixGameEndRowsRowTics/RowY`); Training Show Action, Hold to Exit. Deviations: 12CB announces the vanilla voice instead of "Player N Wins" (reopened: MD-4); yellow "WINS!" x = 175.0 as the Remix reads it outside the array (the per-fighter x is MD-14); team card shows one Dedede (one-frame pose; checked against the emulator in Phase 5b lot 6); the Training Custom spawn on respawn does not exist in the asm.

- [x] **Phase 1, rest of I7** (model part image or palette swap, `MODEL_PART_IMAGE_INIT_`; `notes/i7_part_image.md`): the
  Yoshi egg colour when the egg is thrown, laid and at the entry. Done in Phase 5b lot 3 (package F2, with CT-9; evidence in the ledger rows GP-11 `pc_gp11_*` and CT-9 `pc_ct9_*`). Costumes
  beyond `num_costumes` in the match and on the CSS, and the shield and escape egg, are done.
- [x] **Phase 5b** gaps found by the patch audit (2026-11-05, plan approved the same day). The ledger is
  [`patch_audit.md`](patch_audit.md): one ID per item (prefix per group), asm location, decomp function, what the player
  misses, hook site, effort; the per-scope map is [`patch_audit_scopes.md`](patch_audit_scopes.md). Every open ledger item
  (251 at the audit) was placed in the lots below; lot 8 and Phase 6 closed the count. The CPU-only scopes got the rows SH-15 to
  SH-27 and ST-27 on 2026-11-05 (Phase 6 step 1; 263 open items since).
  - **Acceptance per item:** read the asm scope, port under `#if defined(PORT) && defined(REGION_REMIX)`
    (`porting_guide.md`), a unit test in `tests/remix/` when the item is a pure rule, the in-game test of its lot, then its
    ledger state `done (<run name, evidence>)` or `n/a (<reason checked in the asm>)`. A visual item is compared with a
    reference-emulator capture.
  - **Acceptance of the phase:** no open ledger item outside the AI ones (Phase 6), `tools/audit_remix_patches.py` re-run,
    `parity.md` consistent with the ledger.
  - **Duplicate IDs close together:** GP-1 = CT-14 (with the rapid-jab part of FT-38 and FT-43), SH-7 = CT-2, SH-2 = CT-8,
    GP-7 = CT-15, GP-8 = CT-21, FT-1 = CT-18, FT-8 = CT-19, MS-14 = CT-3, MS-15 = ST-20, the double-jump part of FT-28 =
    SH-1.

  - [x] **Lot 0, groundwork** (2026-11-05): 46 symbols exported to `port/remix/remix_symbols.h`
    (`tools/gen_remix_actions.py`), `scene.sh`, the env-gated logs `SSB64_REMIX_FIGHTER_LOG`, `SSB64_BGM_LOG`,
    `SSB64_PORT_MENU_OPEN` and `SSB64_REMIX_AI_LOG`, the Character Data reference lap, ledger corrections.
  - [x] **Lot 1, Character Data screen** (2026-11-05): MN-16 to MN-19 (the crash on the lap to Dedede, the emblems of
    the 18 extra fighters, special actions, jab combo and move names), checked against `build/ref-shots/chardata_lap`;
    `docs/bugs/remix_chardata_dedede_crash_2026-11-05.md`; emblem deviation in `deviations.md` 5.6.
  - [x] **Lot 2, match flow and stages** (ST, IT-8, IT-10, OP-37, MS-13, MS-15, MS-17, MS-26): every match on a Remix
    stage ended with FAILURE instead of GAME SET (ST-1); countdown, pause camera, surfaces, barrels and bonus, stage
    options. Fixed on the way: the VS results screen aborted after a four-Remix-fighter match
    (`docs/bugs/remix_vs_results_status_buffer_2026-11-05.md`). Since Phase 5 the BTT/BTP stages run the bonus ground,
    so they are tested through the 1P path (SKILL.md).
  - [x] **Lot 3, fighters** (GP, SH, FT, CT outside AI): eight packages; roster special matrix 94 x 3 clean; a
    verification batch of 184 runs (`pc_*`, `lldb_count.sh`/`lldb_first.sh`). Deviations in `deviations.md` 5.8.
  - [x] **Lot 4, items and Size** (IT-2 to IT-11). `SSB64_REMIX_FORCE_ITEM` rewrites every item made, container drops
    included; container tests use `SSB64_FORCE_ITEM_KIND`.
  - [x] **Lot 5, CSS, menus and screens** (MN, MD-3, MD-12 to MD-17, MD-20, MD-22, CT-1, CT-13, CT-24, OP-29 to OP-33,
    OP-40, OP-41, MS screens; MS-2 `n/a`, the port never shows the N64 logo). Fixed on the way: the Gallery label read
    the common menu file; the port menu was built before the Remix toggles loaded (now after `remix_toggles_init`;
    `SSB64_PORT_MENU_PAGE` opens a page for screenshots); `syAudioGetBGMPlaying` cleared when a song without a loop
    ended; Giant DK crashed the VS results and Metal Mario and the polygons had empty menu motions
    (`docs/bugs/remix_giant_dk_results_menu_motions_2026-11-05.md`). Results sweep `l5_rs_*` (82 Remix winners) clean.
  - [x] **Lot 6, modes** (MD, OP): 1P game, 1P overlays, bonus stage, 1P intro, All-Star, stock modes and 12CB. Test
    notes: the 1P scene shortcut needs `SSB64_SPGAME_FKIND` (fkind 28 otherwise), and an All-Star run starts from the 1P
    CSS (scene 17) with `SSB64_REMIX_1P_MODE=allstar`.
  - Lots 2 to 6 closed their remaining in-game checks in the 2026-11-05/06 verification batches; the ledger rows carry
    each run (a few scenarios that never trigger in game, such as the Peach BTT pipe ST-10, are closed on unit tests
    with the attempts written in the row).

  - [x] **Lot 7, audio.** Done (2026-11-06): coded by two workers and built (7228 unit checks); every item checked in game:
    MS-3, MS-6, MS-8, MS-22, MS-23, then MS-5 (`l7_hurry_mk`, Mushroom Kingdom stage 8), MS-4 (`l7_trap`), MS-16 and MS-25
    (`l7_crash_burrow`). `SSB64_MIDI_LOG=1` logs the MIDI overrides. A1 BGM: MS-3 with MS-6 (alt and random music by C button and scene, FD theme), MS-4, MS-5 (the
    30-second hurry swap by track). A2 FGM and MIDI: MS-8, MS-22 with MS-23 (per-song priority, bend range and volume
    overrides; hook site confirmed first), MS-16 with MS-25 (Crash DSP voices and the announcer lines). Tests:
    `SSB64_BGM_LOG` with `test_bgm_rules` extended, the FGM log and `wavstat.py`, the audio dump against the reference
    RMS per track; each item added to the T9.1b listening list.
  - [x] **Lot 8, `parity.md` from the ledger:** `tools/gen_remix_parity.py` reads the ledger states, so a file with an open
    item is `partial`; `tools/audit_remix_patches.py` re-run, no unhooked patch without a ledger ID. Done (2026-11-06):
    every ledger item is placed on its `.asm` row (by the asm or scope cell, the table rows that patch a CT table, the
    including file for `css/*.asm`, or the section file); the Port column lists the open IDs, and every `partial` row has
    one. Rows: 287 (fighters 98 ported / 54 partial, items 29 / 4, stages 6 / 1, audio 5 / 1, menus and modes 30 / 14,
    AI 8 / 24, infra 4 done and 9 n/a); the partial rows are the 124 items still open (in-game checks of lots 5 to 7,
    CT-7 on every fighter row, and the Phase 6 items). Audit re-run: 1969 patches (hooked 1171, ambiguous 460, unhooked
    323, symbolic 12, no function 3), the same total as the 2026-11-05 audit; each of the 323 unhooked patches has a
    scope-map line, and each one with a GAP or STUB verdict is covered by a ledger item (the 13 that a name match missed
    sit in the line ranges or grouped scopes of MN-4, SH-1, SH-4, SH-6 and MD-13).
- [x] **Phase 6** AI, run beside Phase 5b lots 1 to 7 (user decision, 2026-11-05). One worker at a time owns
  `ft/ftcomputer.c`; the others work in `ft/ftremix/ftremixai*.c`, `ftcommoncatch1.c`, `ftcommondash.c` and
  `mnremixcpulevel.c`, which no Phase 5b package edits; registry edits (`port/fighter_registry.h`) are merged by the lead.
  **Acceptance:** `REMIX-TODO(AI)` at 0, every AI ledger item closed, recovery at level 9 not below the re-measured
  baseline, KOTH and Smashketball CPUs reach the hill and the ball, the reflect test with CPU Falco and Ness. Reference:
  [`ai.md`](ai.md).
  Steps 1 to 3 (2026-11-05): baseline with `ai_recovery.sh 10 4 14000` (712 recovery episodes, 88.6% ok), two
  coded batches (SH-16 to SH-27, ST-27, FT-1 = CT-18, FT-2 to FT-13, FT-36, FT-52, GP-8 = CT-21, CT-17, CT-20, MD-10,
  MD-11) and a re-measure (88.8%); the self-destruct count (`selfko`) became the acceptance metric. Deviations in
  `deviations.md` 5.9.
  Step 4, acceptance (2026-11-06): `REMIX-TODO(AI)` at 0 (the last five became notes checked in the asm). Every AI ledger
     item is `done` with its run: lldb hit counts in CPU-only matches with no vanilla fighter that takes the same path
     (`p6p_*`, `p6q_*`, `p6r_*`, `p6s_*`; `lldb_count.sh` gained `ret:` probes on the pure rules functions, since line
     breakpoints on a `return` land on merged lines in the Release build), with controls at 0 (level 9 for the level 10
     items, Improved AI off). Recovery at level 9: 88.8% against the 88.6% baseline (step 3). Smashketball (`p6_ball_a`,
     `_b`): the CPUs score and the match ends; KOTH (`p6_koth_a`, `_b`): the crown bars fill and a CPU stands on the hill.
     Reflect: CPU Falco, Wolf, J Ness and Lucas press and hold reflect/absorb (`p6p_reflect`). FT-11 on the VS CSS
     (`p6_css_lv10`, `p6_css_hcap`). Found on the way: CPU Peppy crashed on the down-special detonate check (fixed,
     `docs/bugs/remix_peppy_flashbang_slot_lp64_2026-11-06.md`). Notes: FT-9 (jab auto grab) and SH-25 at level 9 cannot
     fire in Remix either (ledger rows); under lldb, stage 92 stops at launch in objc_msgSend (no lldb: runs).
- [x] **Phase 7** open bugs, instrumentation and packages.
  - [x] Bugs (2026-11-06): status buffers: `sc1pgame.c` and `sc1ptrainingmode.c` already use 256 entries under
    `REGION_REMIX`; the 1P intro (100 entries) shows Remix enemies on stages 1 to 10 of Remix 1P with no "Status Buffer is
    full" (`p7_intro_*`). Not reproduced: Poly Young Link on the VS CSS (D-pad down on Young Link shows the model on the
    first pick, `p7_r2_120_0`, `p7_r2_122_0`), Metal Mario size on the Training CSS (`p7_tr_136_DU`, same size as the CPU
    Mario), the rc 139 flake on Tal Tal (12 four-fighter matches on 0x2E and 0xA7, all rc 0, `p7_taltal_*`). The Switch
    stutter (`walked past registered DL range`, stale token 0x57EE) needs the hardware: moved to T10.2.
  - [x] Peach's Castle: blue quads with the roof-tile texture float in the sky when the camera pans left (2026-11-05,
    `b11_pc_wide`, `b11_us_pc`): not a bug. They are layer-0 objects 3 and 4 of the Castle ground data (roof wedges, DLs
    0x2808 and 0x28F8 of file 106, animated between x = +-3000 and +-3900 near the camera bounds). A reference capture with
    P1 walking left (`build/ref-shots/pc_pan_remix`, frame 008) shows the same wedge in the top-left corner; the earlier
    reference `vs1` never panned that far.
  - [x] Giga Bowser 1.70 zoom on the CSS (2026-11-05): not a bug. The reference capture (`build/ref-shots/giga_css2`, D-pad
    up over Bowser on the VS CSS of the real Remix) shows Giga Bowser overflowing the panel at the port's size
    (`b10_giga_css`).
  - [x] `PEACH_TURNIP_GFX` out of bounds (2026-11-05): not a bug. Dump of file 5144 (`SSB64_DUMP_FILE_ID`): slots 919 to 928
    are the turnip's 10 images; the two outside the file (927 and 928, targets 2482 and 2352 in a file of 1176 words) are
    images 8 and 9, and the turnip only uses images 3 to 6 (`stage_setting_` L181-217). Zeroing those slots, as the bridge
    does, is safe.
  - [x] Instrumentation (2026-11-06): spdlog flushed before the `RelocPointerTable` aborts (`port_log` already flushes
    every write), `scene <id> summary: <frames>, <slow>, max <ms>` at every scene change (`p7_instr`), and
    `lbReloc: slow load file_id <id>` for a load above 20 ms.
  - [x] Test packages with the `.o2r` embedded (macOS script and AppImage), marked for personal use (2026-11-06):
    `dist/desktop/BattleShip-Remix-2.0.1-port.07d89c8-macOS-arm64.dmg/.zip` (the bundled binary reaches a VS match, rc 0)
    and `build-linux-release/BattleShip-Remix-2.0.1-port.ab57e0f-linux-x86_64.AppImage` (podman amd64; the Dockerfile now
    zeroes the AppImage magic of linuxdeploy and appimagetool, which broke binfmt under emulation).
  - [x] Final packages without the `.o2r`: superseded by the releases of `release-remix.yml` (**M12**).
- [x] **Phase 8** final verification. Regression: 142 VS stages (`parallel.sh stages`) plus the 33 bonus-class stages through
  the 1P path, special matrix 95 × 3, item sweep, 4 players in every mode
  through the CSS and Training, the vanilla build, unit tests. User passes: T9.1b (listening and visual), T10.2 (Switch).
  Reference comparison: emulator captures (I8) against the port on every mode screen, CSS, stage select, pause and results;
  each difference is fixed or documented as an adaptation. Close: `parity.md` only `ported` / `adapted` / the approved
  `n/a`, T9.3 (report "Remix playable" with the list of adaptations) and the final checkpoint.
  Evidence (2026-11-06): stages `st_*` 142 rc 0, none blank; bonus path `p8b_<3|7>_<fkind>` 160 runs (fkinds 0..11,
  29..96 on BTT and BTP), 2 shots each, no log error; special matrix `m_*` 285 rc 0 (`m_35_usp` was a Metal flake, clean
  on rerun); item sweep `cpu_46..65` 13 Remix items rc 0, 22 to 42 spawns each, no log error; 4 players through the VS
  CSS (`p8f_default`, `p8f_koth`, `p8f_smashketball`, `p8f_tugofwar`), Tag Team, teams and stamina by the VS shortcut
  (`p8f_tagteam_vs` swaps on a KO, `p8f_teams_vs`, `p8f_stamina_vs` 150 HP; the Tag Team CSS refuses START until every
  queue is full, as in `TagTeam.asm`), 12CB full loop `p8f_12cb_flow` (match, results, back to the CSS with the stock
  taken), Training `p8f_training`; `run_remix_mode_tests` 7236 checks, 0 failed; `build-us` `ssb64` builds. Reference:
  `build/ref-shots/` `css_menu`, `koth`, `tagteam`, `smashketball`, `tugofwar`, `pause`, `stats`, `r12` against
  `p8f_*`, `p8r_vsmenu`, `p8r_pause`: same layout and texts (the reference pause shows the legend opened with R).
  Sound Test: `build/ref-shots/p8_data` against `p8r_sound`, same layout, legend and cursor. Stage select, title, 1P
  intro cards, Character Data and the Giga Bowser CSS were compared in lots 1, 5 and 6. Congratulations:
  `build/ref-shots/p8_scene_37` (reached with a local mupen64plus cheat on the title scene: `D00A4AD0 0001`,
  `810465D2 0001` (`screen_interrupt`), `800A4AD0 0037`) against `p8r_congra`, same picture and text. Credits: the same
  cheat with scene 0x38 presents no frame after the jump (tried 600 s and 1800 s; the only shot is the boot notice), so
  there is no reference capture; the port credits
  (`p8r_credits`) roll without error.
  `parity.md`: 277 `ported`, 2 `adapted`, 10 `n/a`, 0 `partial`.

### M12. Platforms and releases (after M11)

Not part of the Remix port itself; recorded here so the plan matches what ships. Reference: `build_and_roms.md`, `AGENTS.md` ("Releases", "Asset extraction fingerprint", "Logs").

- [x] Releases: a `v*` tag on `feature/remix-port` runs `.github/workflows/release-remix.yml` (macOS `.dmg`, Linux AppImage, Windows zip, Switch zip and bare `.nro`, macOS app zip, Android arm64 APK, PS5 title zip, `SHA256SUMS.txt`) and opens a draft whose body is `.github/release-notes.md`. The inherited upstream workflows are disabled.
- [x] PS5 native title `PPSA99264` (`scripts/ps5/`, `port/ps5/`), with its zip updater.
- [x] Android arm64 APK (`dev.lucasliet.battleship.remix`), with its own boot-time updater (`AppUpdater`, `UpdatePrompt`, a `PackageInstaller` session).
- [x] Updater: start-up popup on desktop and Switch with restart after install; Switch swaps the running `.nro`.
- [x] Switch updater installs the full switch zip: it unpacks into `update-pending/`, swaps the `.nro` and applies the other files at the next boot; `.o2r` and `.recipe` stay manual. The bare `.nro` asset stays for one transitional release.
- [x] macOS updater installs the zipped `.app` in place (new `BattleShip-Remix-macos.zip` release asset; the new app is swapped in on restart).
- [x] Windows updater script fixed for the Remix executable name (`update_game.bat` named `BattleShip.exe`).
- [x] Hardware validation of the Switch zip update and of the macOS in-place update on a real install: validated by the owner (2026-11-15).
- [x] Asset recipe: the Torch revision is pinned in `tools/torch_extract_rev.txt`, so unrelated Torch commits do not force a re-extraction; the Switch error screen tells a missing `.recipe` from a mismatch.
- [x] Logs hold only the last run (`ssb64.log`, `logs/BattleShip-Remix.log`, PS5 `boot.txt`).
- [x] Bug reports go through `.github/ISSUE_TEMPLATE/bug_report.yml`.

### M13. Toggle parity audit against the asm (opened 2026-11-14)

Issue #8 showed that a toggle can be `ported` in the ledger and still differ from the asm in ways no smoke test caught:
Charge Smashes ran its logic on the wrong frame and scaled damage on the wrong motion event, Special Zoom called the
joint animation instead of the material animation and wrote the magnifier byte next to the right one
(`docs/bugs/remix_charge_smash_never_starts_2026-11-14.md`, `docs/bugs/remix_special_zoom_wallpaper_matanim_2026-11-14.md`).
M13 repeats that comparison for every other toggle of `toggles.md`: every asm site that reads the toggle against every
port site, looking for a wrong hook site or call frequency, field, constant, condition, vanilla function or value mapping.
The asm is the reference, except where it is itself wrong: an asm bug, a missing piece or something the port already does better is kept or fixed in the port, never copied, and recorded in `deviations.md` (owner policy, 2026-11-15).

- [x] T13.1 Audit, read-only, one auditor per batch and one skeptic per finding (only findings confirmed on both sides
  count). Batches (registry keys):
  1. `skip_results_screen`, `hold_to_pause`, `css_panel_menu`, `practice_overlay`, `cinematic_entry`, `flash_on_z_cancel`
  2. `model_display`, `special_model`, `advanced_hurtbox`, `hold_to_exit_training`, `improved_combo_meter`,
     `tech_chase_combo_meter`, `combo_meter`, `1v1_combo_meter_swap`
  3. `neutral_spawns`, `salty_runback`, `preserve_salty_song`, `stereo_fix`, `variant_random`, `disable_hud`,
     `full_results`, `skip_start_cheer`
  4. `default_cpu_level`, `puff_sing_anim`, `l_random_char`, `dpad_css_control`, `pk_thunder_reflect_crash_fix`,
     `flash_guard`, `screenshake`, `blastzone_gfx`
  5. `hitstun`, `hitlag`, `di`, `japanese_sounds`, `j_stun_sleep`, `momentum_slide`
  6. `shieldstun`, `z_cancel_opts`, `punish_on_failed_z_cancel`, `tripping`, `rage`, `footstool`
  7. `air_dodge`, `jab_lock`, `ledge_jump`, `perfect_shield`, `parry`, `spot_dodge`
  8. `fast_fall_aerials`, `ledge_trump`, `wall_teching`, `item_containers`, `game_speed`, `blastzone_warp`
  9. `single_button_mode`, `item_dropping`, `move_staling`, `stopwatch_behaviour`, `improved_ai`
  10. `play_music`, `random_music`, `menu_music`, `show_music_title`, `bgm_volume`, `fgm_volume`, `pokemon_voices`,
      `announcer_mode`
  11. `sss_layout`, `hazard_mode`, `whispy_mode`, `saffron_poke_rate`, `dragon_king_hud`, `camera_mode`, `yi_clouds`
- [x] T13.2 Fix the confirmed findings, one commit per batch. Every fix needs an in-game smoke test that shows the asm
  behaviour (before and after); a fix with pure rules also gets a unit test in `tests/remix/`, in addition to the smoke
  test, never instead of it. A significant one gets a `docs/bugs/` write-up. Sign-off: the worker's report is not
  enough; the lead reruns the smoke test of every fix on the build that will be committed (skill `remix-smoke-tests`),
  looks at the screenshots and logs, records the evidence under the batch below, and only then commits.
  Evidence per batch (lead sign-off):
  - Batch 2 (`combo_meter`, `special_model_display`): four level-9 CPUs with Combo Meter on show COMBO+2 to COMBO+8
    above the damage HUD in the attacker's colour. Sudden death (VS shortcut, `SSB64_VS_TIME=1`, game rules set to TIME
    from gdb at the first setup): `ifRemixComboMeterSetup` runs again with `is_suddendeath=1` and the records go from
    4 hits to 0. Smashketball overtime (`SSB64_REMIX_MODE=smashketball`, four CPUs): the second setup has
    `gmRemixSmashketballKeepsMatchStats()=1` and the records stay 2,0,2,2. Training (scene 54, D-pad Down, exit to the
    Training CSS and back): hitbox models in the first Training, normal models in the second, and the cfg file only
    changed the window position.
  - Batch 3 (`variant_random`, `disable_hud`, `stereo_fix`): menu rolls draw like `Global.get_random_int_safe_` (game
    RNG advanced 1 to 64 times by the frame counter). 12CB Custom D-pad Down (Custom set forced from gdb): DEFAULT only
    grid fighters, INCLUDE BONUS adds bonus ids (66, 74, 75, 76...), VANILLA only ids up to Ness; the same presses 7
    frames later give other fighters. Tag Team Random queue slots (stocks set to 5 from gdb): VANILLA only ids up to
    Ness, INCLUDE BONUS bonus and variant ids, and two press timings give different queues. Positional sounds: Home-Run
    Contest 7 of 7 and Sector Z Remix 6 of 6 with pan 0x80 (the sound's own pan), Dream Land positional (0x31 to 0x45).
    Training pause shows "Press Z for Custom Menu" with Disable HUD PAUSE and ALL. All runs rc=0, screenshots read.
  - Batch 4 (`puff_sing_anim`, `l_random_char`, `dpad_css_control`): Jigglypuff's Sing (gdb on the effect update) has
    the matrices [Tra, 0x2E] on 176 frames with the ring centred on her; the scale takes 130 values with the toggle on
    and stays 1.0 with it off. VS CSS with L random: after picking Mario, three L presses give Ness, Kirby and Crash;
    a CPU panel keeps its fighter (fkind 73 on all three samples). D-pad cursor while P1 carries P2's CPU token: the
    cursor leaves x=78 only 30 frames after D-pad Right (asm delay), and the token moves from Ness to Kirby. 1P, Bonus
    and Training CSS (gdb on each cursor update, D-pad Right held from the same frame): with the own token held over a
    portrait the cursor first moves 29 updates later than with the token placed (228 against 199 on all three screens).
    All runs rc=0, screenshots read.
  - Batch 5 (`japanese_sounds`): gdb printed the id passed to `ftRemixCommandPlayHitFGM` for five jab and tilt hits.
    Mario with the toggle at 0 sends 0x28/0x20, at 1 sends 0x93/0x8F; J Mario at 0 and 1 sends 0x93/0x92/0x8F, at 2
    sends 0x28/0x26/0x20, as in `Character.asm` `apply_sound_type_`.
  - Batch 6 (`footstool`, `z_cancel`): the footstool with no jump left plays FGM 1200 twice and puts P2 in status 32
    twice (0 with the toggle off). A level-10 CPU with Z-Cancel DISABLED lands its aerials in LandingAirNull (219) four
    times with four laugh FGMs (1316); with Z-Cancel at DEFAULT no laugh and normal landings.
  - Batch 7 (`perfect_shield`, `parry`): the perfect shield check now runs only for hop-capable airborne projectiles
    and items, as the asm patch site. gdb counts with P1 cycling the shield: Mario's fireballs 5 checks and 4 reflects;
    Pikachu's Thunder Jolt (no hop) 0 checks and 0 reflects; forced Green Shells 7 checks and 0 reflects, Eggs 4
    checks, Bob-ombs 0 checks. The direct-hit parry (kept per hitbox, `deviations.md` 5.3) played its sound 12 times
    against jabs. All runs rc=0, screenshots read.
  - Batch 8 (`fast_fall_aerials`, `item_containers`): Mario's nair with stick down lands at battle tic 64 with the
    toggle on and 81 with it off (frame 460: on the ground against still in the air). Crates with ALWAYS EXPLODE play
    0x3B 21 times and 0x3C never (DEFAULT: 25 and 23).
  - Batch 9 (`improved_ai`): four level-9 CPUs, 3 stocks, 14000 frames, gdb hit counts on the committed build.
    `fix_target_pos_x` now runs before the ground/air split: with Improved AI on, Flat Zone (50) made 2850 grounded
    calls of `ftRemixAiRecoveryClampWalkTargetX`, 80 of them with the target past the bounds minus 200 (clamped);
    Onett (73) 4899 grounded calls, none out of bounds (before the fix the call was air-only, 0 grounded calls).
    J Pikachu and E Pikachu (with Mario and Fox, Dream Land) now reach the vanilla Pikachu gate: 3568 calls of
    `ftRemixAiRecoveryKeepsPikachuVanilla` with Improved AI off, all on the vanilla side; 3106 with it on, none on
    the vanilla side (before, only fkind 9 reached it). Results screens normal, no abort or assert in the logs. The
    PK Thunder ledge table indexing (`port * 4` in the asm) is an asm bug, kept per port (`deviations.md` 5.9).
  - Batch 10 (`play_music`, `menu_music`, `bgm_volume`, `music_title`): toggles set at run time from gdb on the VS
    mode menu. Play Music OFF then closing the settings stops the kept track (71 to -1); with music off then on and a
    close, or A on BGM Volume, the menu track restarts (-1 to 71); in a match neither starts a menu track. RANDOM and
    RANDOM ALL with the 64 menu track playing: the VS CSS plays its own track 10 and the VS menu 44 again; the 1P,
    Training and both Bonus CSS play 10. Back from the Data screens RANDOM, RANDOM CLASSICS and RANDOM ALL roll a new
    track (262, 283, 334). SM64 trap: 0 of 150 RANDOM presses after three direct SM64 presses land on the trap
    (8 of 150 before). Music title at the 1P countdown: Meta Crystal and Duel Zone now show their banner until GO.
    All runs rc=0, screenshots read.
  - Batch 11 (`camera_mode`, `hazard_mode`, `dragon_king_hud`): 1P with Camera Mode SCENE, switched to NORMAL from gdb
    at the second camera: stages 2 and 3 get the vanilla routine table back (Default, Inishie, Zebes) and the default
    routine, so the SCENE table no longer leaks into the next camera. VS on Planet Zebes (3) and Mushroom Kingdom (8):
    FIXED gives a still full-stage view and SCENE the wide anim view, against the tracking camera with NORMAL. Rainbow
    Road (163): the Chain Chomp exists with Hazard Mode 0 and not with 1 or 2 (Hazards.asm tests the whole value). A
    VS match left on Dragon King (scene gkind 0x35 set from gdb) played on Dream Land puts the combo meter at y 167,
    above the damage HUD, since the battle-state gkind decides. Hazard Mode outside VS and Training and the Zebes acid
    AI keep the port's screen gate (`deviations.md` 5.7 and 5.9). All runs rc=0, screenshots read.
  - Follow-up checks on the final build:
    - RANDOM token (0x1B set from gdb before START) on the VS, 1P and Training CSS: VANILLA gives ids up to Ness (10,
      4, 10, 7, 11, 10), INCLUDE BONUS gives bonus ids (65), and a 7-frame later START changes the fighter.
    - Combo meter in 1P: set up once, its update ran 2636 times.
    - BGM Volume through its menu widget (`SSB64_PORT_MENU_SET="BGM Volume=60@400"` on the VS mode menu, music
      off at start): with Play Music turned ON from gdb the widget callback restarted track 71; with it still OFF
      nothing played.
    - Settings close through `PortMenu::UpdateElement` (menu opened with `SSB64_PORT_MENU_OPEN`, hidden from gdb):
      the hook ran, Play Music OFF stopped track 71, and Play Music ON restarted it from silence.
    - Camera BONUS on Planet Zebes: the default and Zebes slots hold the Mushroom Kingdom routine, which the camera
      runs.
    - Controls: with Perfect Shielding off, the check returns at entry and fireballs are never reflected. A level-9
      CPU with Improved AI and Z-Cancel DISABLED lands in 219 twice and 215 once, with 3 laughs.
    - Regression: stage sweep (0 to 16 and 41 to 215, 159 stages, no crash, no blank screen), special matrix of 24
      fighters (72 runs), CPU item sweep (5 kinds); all rc=0 with no abort or assert. Ids 17 to 40 are the bonus
      stages, which the VS shortcut cannot load.
- [x] T13.3 Update `toggles.md`, `patch_audit_scopes.md` and `deviations.md` for what changed, and the release notes.
  Done with each batch commit; the release notes have the M13 highlights (Combo Meter, settings checked against Smash
  Remix).

### M14. Equivalence audit of the non-toggle logic (opened 2026-11-15)

M11 checked that every asm patch has a port hook (presence); M13 checked that the hook does what the asm does, but only
for the settings toggles. The rest of the hand-ported logic (fighter specials, stage hazards, modes, items, shared
gameplay patches) never had that equivalence check. A read-only pilot on 2026-11-15 (one auditor per area, one skeptic
per finding, the same method as T13.1) sampled six areas and confirmed 23 differences:

| Area sampled | Routines equal to the asm | Confirmed differences |
|---|---|---|
| Marth (`MarthSpecial.asm`, `Marth.asm` logic) | 27 | 0 |
| Sonic and Super Sonic (`SonicSpecial.asm`, `SSonic.asm`, `Sonic.asm` logic) | 56 | 2 |
| Hazards of Pirate Land, Rainbow Road, Metallic Madness, Spawned Fear (`Hazards.asm`) | 19 | 10 |
| King of the Hill and Smashketball (`KingOfTheHill.asm`, `Smashketball.asm`) | 24 | 8 (4 more already documented) |
| Items Blue Shell, Stopwatch, Waddle Dee, Boomerang | 38 | 2 |
| `Damage.asm` and the shared fighter files | 40 | 1 |

Fighters show few differences; hazards, modes and items about one per two or three routines, the M13 rate. Acceptance
of every fix as in M13 (T13.2): an in-game smoke test that shows the asm behaviour before and after, rerun by the lead
on the build that will be committed; a unit test as well when the fix is a pure rule; a `deviations.md` row when the
asm is the one that is wrong (owner policy, 2026-11-15). The lots ran one after another on the same
tree and share files, so M14 is one commit in `decomp` and one in the outer repository.

- [x] T14.1 Fix the pilot findings (asm paths under `.toolchain/pkgs/smashremix-src/src/`; line numbers of 2026-11-15).
  - Lot A, hazards:
    - [x] A1 Pirate Land cannon DObjs get no transform XObj (asm 0x1C TraRotRpyRSca): the barrel never moves to
      -128/0/+128, the 1.125 fire pulse and the cannon animation are missing, the parts sit at the parent origin.
      `Hazards.asm:3770-3780` / `grremixpiratecannon.c:81,299-300`.
    - [x] A2 Pirate Land: no muzzle effect (`0x80100480`, white sparkle) when a cannonball spawns.
      `Hazards.asm:4179-4183` / `grremixpiratecannonball.c:84-105`.
    - [x] A3 Pirate Land: the cannonball's second transform is 0x48 (billboard with scale) in the asm, 0x47 in the port.
      `Hazards.asm:4326` / `grremixpiratecannonball.c:48-51`.
    - [x] A4 Rainbow Road: the Chain Chomp has no per-frame `gcPlayAnimAll` process, so it stays on its first animation
      frame at the spawn point instead of charging along the track. `Hazards.asm:5737-5743` /
      `grremixrainbow.c:75-85,131-138`.
    - [x] A5 Rainbow Road: the Chomp draws on display link 6 with the z-buffer; the asm uses link 0x11 with
      `grDisplayLayer0PriProcDisplay` (no z-buffer, drawn over the track). `Hazards.asm:5683,5707-5715` /
      `grremixrainbow.c:148`.
    - [x] A6 Rainbow Road start sequence: the asm plays 0x403 five times and 0x404 at 0xAC8 (delay-slot decrement);
      the port plays four and 0x404 at 0xA8C, so one beep is missing and the cycle is 60 frames short (fix `hazards.md`
      too). `Hazards.asm:5953-5965` / `grremixrainbow.c:28-29,106-118`.
    - [x] A7 Rainbow Road: the Chomp growl window reads GObj+0x0E (`frame_draw_last`, a global 8-bit counter), so it
      reopens every 256 frames for the Chomp's whole life; the port opens it only in its first 15 frames.
      `Hazards.asm:6081-6095` / `grremixrainbowchomp.c:61-75`.
    - [x] A8 Metallic Madness: `shrink_timer_` clears the laser shrink when the player has an active Super or Poison
      Mushroom; the port lets the 900-tic timer run and then overwrites the mushroom size.
      `Hazards.asm:4746-4760` / `grremixmetallic.c:216-224`.
    - [x] A9 Spawned Fear: the Cacodemon factory timer goes back to 0 (delay slot) even when a Cacodemon is alive; the
      port keeps counting, so a new one can appear right after the last dies, and a forced frame 3000 with one alive is
      never retried. `Hazards.asm:8641-8646` / `grremixcacodemonfactory.c:63-68`.
    - [x] A10 Spawned Fear: the Thunderball lacks the 0x47 billboard transform and the `wpMainVelSetModelPitch` call at
      spawn. `Hazards.asm:9235,9032-9039` / `grremixthunderball.c:70-74,153-171`.
  - Lot B, modes:
    - [x] B1 KOTH: every frame with no king (empty or contested hill) resets the 3.5 s win countdown of every team; the
      port freezes it. `KingOfTheHill.asm:1292-1305,1375-1381` / `gmremixkothking.c:123-127`.
    - [x] B2 KOTH: the meter frame sprite uses the `Color.damage_icon` tints (FFA0A0, B4B4FF, E6E678, A0FFA0, DCDCDC),
      not the darker bar colours. `KingOfTheHill.asm:1555-1558,1596,1623-1632` / `gmremixkothhud.c:81-83`.
    - [x] B3 KOTH: the hill re-pick retries invalid lines forever and only counts repeats of the current hill (4
      draws); the port spends one of 3 draws on every invalid line and keeps the old hill.
      `KingOfTheHill.asm:974-990,1005-1037` / `gmremixkoth.c:134-141`, `gmremixkothhill.c:92-100`.
    - [x] B4 KOTH hidden hill: the asm moves the hill as soon as the hidden marker leaves the blast zone, else on the
      121st hidden frame, and resets `hidden_frames` only there; the port skips the blast-zone check, moves one frame
      early and resets on every change. `KingOfTheHill.asm:1127-1148` / `gmremixkoth.c:164-168`.
    - [x] B5 KOTH: each full bar has its own flash starting at alpha 0; the port runs one shared phase from match start.
      `KingOfTheHill.asm:1459-1473` / `gmremixkothhud.c:87-95,102,116`.
    - [x] B6 Smashketball streak chant: the asm passes `FTStruct.player_num`, the port the port index (wrong or no
      chant when the ports used are not contiguous). `Smashketball.asm:897-899` /
      `gmremixsmashketballscore.c:67,75`.
    - [x] B7 Smashketball forced chant keeps the same-player check: one chant per streak, not one per basket from the
      third on. `Smashketball.asm:1136-1142` / `ftpublic.c:100-107`.
    - [x] B8 Smashketball CPU ball seek reads the item DObj translate; the port uses the owner's hand position while
      the ball is held (check in game first; low confidence). `Smashketball.asm:1175-1179` /
      `ftremixaimodes.c:37,60`. Not reproduced: the basketball has no pickup, drop or throw routine, so it is never held and both sides read
      the same DObj translate; the dead path is a `deviations.md` 5.9 row.
  - Lot C, items, fighters and shared patches:
    - [x] C1 Blue Shell: the rolling shell refreshes the first-place fighter every 8 frames and runs the red shell
      follow routine (`0x8017A3A0`) toward it; the port keeps the throw velocity. `items/BlueShell.asm:873-876,
      1349-1429` / `itremixblueshell.c:437-453,342-352`.
    - [x] C2 Blue Shell: when the interact counter runs out while rolling, the asm keeps the shell (its destroy branch
      falls into `_end`, which clears v0); the port destroys it. Likely an asm bug: keep the port and add a
      `deviations.md` row, after confirming. `items/BlueShell.asm:1174-1182,1238-1239` /
      `itremixblueshell.c:480-484`. Confirmed asm bug: the port keeps the red shell rule, `deviations.md` row.
    - [x] C3 Sonic spring: the asm turns only Tumble into JumpF (`sltu` against FallSpecial); the port's inclusive range
      also frees a helpless (FallSpecial) fighter. `Sonic/SonicSpecial.asm:1747-1752` / `ftremixsonicusp.c:311-314,
      447-450`. Fixed to the asm code (Tumble only). The asm comment at `SonicSpecial.asm:1736` lists Tumble to FallSpecial,
      so the code and its comment disagree; the port follows the code, the conflict is stated in
      `ftremixsonicspringrules.c`.
    - [x] C4 Super Sonic `end_invisibility_` only toggles the invisibility bit; the port also writes the moveset env
      colour, which hides the player tag for a few frames at the end of the teleport (same pattern in Sheik and Mewtwo:
      check them too). `Sonic/SonicSpecial.asm:3456-3467` / `ftremixssonicusp.c:68-72`.
    - [x] C5 Bury timer: the asm rounds 0.7 × percent to nearest (`cvt.w.s`), the port truncates (one frame shorter
      about half the time). `Damage.asm:385-397,434-446` / `ftremixbury.c:52-57`.
  - Lead reruns on the committed build (2026-11-16, Linux; every run rc=0, no abort/assert line in any log):
    - Lot A: stage 158 `m14a_pirategdb_after` cannon xobjs 1/1/1, ball second transform 0x48, Sparkle 3 of 3 shots, barrel
      and muzzle flash in the `m14a_pirate_after` sheet; stage 163 `m14a_rainbowgdb_after` Chomp on link 17 with
      `grDisplayLayer0PriProcDisplay`, charges and KOs Mario in the sheet, 0x403 at 2461 to 2701 and 0x404 at 2761, growl
      again at 2311 (180 rolls); stage 162 `m14a_metallicmid_after` EndShrink 0 and shrink timer 0 from t=1043; stage 195
      `m14a_cacoheld_after` factory timer 297 (was 3298), `m14a_cacoshoot_after` 4 shots with 4 pitch calls; hazard sweep
      of 158, 162, 163, 195.
    - Lot B: `m14b_b1_after` win timers back to 0 the frame after the hill empties, FGM 470 twice (was 1067);
      `m14b_hud_after` each full bar starts its own flash at alpha 0; `m14b_b3_after_s*` 53 of 53 picks change the hill
      (Sector Z 50 of 53, two valid lines); `m14b_b4_after` hill placed at t=119, moved on the 121st hidden frame;
      `m14b_b6_after_*` every forced chant resolves to the scorer; `m14b_b7_after_*` no cheer at a forced tic;
      `m14b_b8_check` held=0.
    - Lot C: `m14c_c1_after` the rolling shell turns and follows the first-place fighter (106 follow calls);
      `m14c_c3_after` JumpF from Tumble only; `m14c_c4*` invisibility ends with env colour 0 for Super Sonic, Sheik and
      Mewtwo; `m14c_c5_after` bury waits 103/100/102 for 5/1/4 percent.
- [x] T14.2 Audit of the rest, read-only, same method as the pilot (2026-11-15): 22 batches, 517 routines equal to the
  asm, 116 confirmed differences, 3 already documented, 12 refuted by the skeptic.

  | Batch | Scope | Equal | Confirmed |
  |---|---|---|---|
  | h1 to h4 | hazards of the other stages, `Blastzone.asm`, `Surface.asm`, the stage loader | 79 | 13 |
  | m1 to m4 | VS modes, Training, 12CB, HRC, the 1P modes and menus | 103 | 26 |
  | i1 to i3 | the other items | 28 | 20 |
  | f1 to f6 | the other fighters and Kirby | 153 | 28 |
  | s1 to s4 | shared fighter systems, CSS, screens, audio | 118 | 25 |
  | a1 | AI outside the Improved AI toggle | 36 | 4 |

  Parts each auditor left out are listed under "Not covered" below.
- [x] T14.3 Fix the T14.2 findings, one lot per batch (2026-11-16). Each fix has the before/after in-game test of its
  `docs/bugs/` write-up; the lead reran every after-test on the committed build (run names below; all rc=0, no
  abort/assert line). "deviation" means the asm is the one that is wrong: the port keeps its behaviour and
  `deviations.md` has the row (owner policy, 2026-11-15).
  - h1: H1-1 Great Bay, Twilight City and Pirate Land water runs as the GObj run function, before the fighters'
    interrupt step (`h1_after`: W before I on every frame, jump leaves at vx=28).
  - h2: H2-1 the `bg_animation_` actor runs on the 27 `add_bg_animation` stages (`h2a_*`: actor made on 0, 76, 113,
    156, 180, 191, 192 and not on 8, 64); H2-2 MMM taunter scan, deviation.
  - h3 (Bowser's Stadium, Time Twister, Scuttle Town, Zebes): H3-1 bomb hitstun only for bomb hits
    (`h3_bomb_hitby_after`); H3-2 starting bombs fall with a hitbox; H3-3 fade rectangle in room 0; H3-4 the new song
    waits for Hammer or Star music (`h3_tt_hammer_after`); H3-5 Remix lava levels no longer leak into vanilla Zebes
    (`h3_zebes_after`); H3-6 transition cues on the asm frames (`h3_tt_cues_after`).
  - h4: H4-1 `Stages.bg_type` wallpaper kinds (`h4a_wp*`); H4-2 same as H2-1; H4-3 Crash BTT/BTP 1-up easter egg
    (`h4a_crash_bt*`); H4-4 announcer set up in Training and the bonus stages (`h4a_ann_*`).
  - m1: M1-1 Stamina off in Tag Team and Tug of War; M1-2 Tag Team slot 0 team costume (`m2_after`); M1-3 Stamina HUD
    pop/flash, deviation; M1-4 stock steal by the victim's queue slot (`m4_after`).
  - m2 (Training, 12CB; `m2a_*`): M2-1 KO respawn at 0%; M2-2 D-pad Controls Reset Only/Off clears the freeze; M2-3
    D-pad shortcuts while paused; M2-4 camera stops with the game frozen; M2-5 frame advance on lag tics and at 2x/3x;
    M2-6 any controller pauses; M2-7 12CB controllers 3 and 4 cannot pause; M2-8 Custom spawn cleared on exit; M2-9
    12CB winner stays on the portrait.
  - m3 (HRC; `m3_*`): M3-1 camera follows at 1/8 per frame; M3-2 not reproduced (the HRC pause never runs the
    wallpaper); M3-3 Lanky Orangstand, deviation (waits for the Sandbag in flight, ends in FAILURE where the asm
    hangs); M3-4 record saved only on COMPLETE; M3-5 the Sandbag is player kind 5.
  - m4 (1P menus, Bonus, retry; `m4_*`): M4-1 Bonus CSS STAGE line follows the header; M4-2 L retry only with L alone;
    M4-3 1P menu wrap wait; M4-4 HRC TOTAL decimal point; M4-5 title card VS and name frame; M4-6 STAGE arrow click
    padding; M4-7 L retry in the bonus stages of a Practice run; M4-8 Bonus CSS TOTAL, deviation.
  - i1 (`m14i1_*`): I1-1 Flashbang and Conker grenade spin on rotate.z; I1-2 Flashbang spawn bit is `is_damage_all`;
    I1-3 Cloaking Device opacity from `pos_diff`; I1-4 holds the charged sparkle; I1-5 Deku Nut spawn runs the explode
    subroutine; I1-6 Deku Nut knockback scale halfword; I1-7 and I1-8 Cloaking Device, deviations; I1-9 Basketball
    bounce hitlag rounds; I1-10 Basketball hit on fire re-applies the colanim.
  - i2 (`i2_*`): I2-1 Super/Poison Mushroom replaces the running effect timer; I2-2 Franklin Badge reflect hitbox
    (custom reflect, FGM 0x41B, 15 reflections); I2-3 badge process recreated on a rematch; I2-4 mushroom rumble
    survives status changes (12 motor starts); I2-5 Mr. Saturn clears owner stats at spawn; I2-6 Mr. Saturn fall
    keeps the "never despawn" bit (`i2_sat_after_*`); I2-7 Lightning badge lookup, deviation.
  - i3 (`i3_a_*`): I3-1 Turnip ground timer 48; I3-2 Waddle Doo beam stars chain from the previous star; I3-3 star
    start position in `attack_pos[0].pos_curr`.
  - f1 (`f1_after_*`): F1-1 Banjo Shock Spring without air friction or speed cap; F1-2 same as I1-1; F1-3 Kirby's Crash
    spin GFX; F1-4 Crash eat sound not on the Heart Container; F1-5 Kirby with Crash keeps no shield hitlag on a
    blocked spin; F1-6 Conker grenade fuse cut rounds.
  - f2 (`f2_a_*`): F2-1 Kirby with the Dedede hat gets the 27 inhale transitions; F2-2 Dedede absorb flags and slope;
    F2-3 Dragon King ball on joint 16; F2-4 Dedede and Dark Samus initial scripts at spawn and rebirth.
  - f3 (`f3a_*`): F3-1 Lanky grape spawn offset; F3-2 Kirby with Lanky uses statuses 0x1C5/0x1C6; F3-3 Marina air jump
    heights (70.5 down to 43.5); F3-4 Marina consecutive air jump lock; F3-5 Cargo shake gem; F3-6 Ryo destruction and
    F3-7 Marina DSP absorb, deviations.
  - f4 (`f4_*`): F4-1 Peach floats out of the double jump; F4-2 Mad Piano fires bonus ammo + 1 books; F4-3 Shadow Ball
    "charge" announcer call; F4-4 Mewtwo air shoot keeps its x speed.
  - f5 (`f5a_*`): F5-1 Wario Body Slam clang tests the right fighter; F5-2 Wolf (x2.0) and Slippy (x0.40625) reflect
    speed; F5-3 J Link up special landing lag 0.75; F5-4 Slippy forced stick_y in the Firefox angle.
  - f6 (`f6_a_*`): F6-1 same as F2-1; F6-2 Kirby aerial Shadow Ball recoil, deviation; F6-3 no magic power from a
    Polygon Kirby.
  - s1 (`s1_*`): S1-1 throw random knockback angle read from the thrower; S1-2 same as F4-2; S1-3 env colour override
    only replaces the stage light colour; S1-4 Stamina lethal hit also clears status armour; S1-5 Thunder trail
    absorb, deviation.
  - s2 (CSS; `m14s2_*`): S2-1 C left/right keeps the team costume; S2-2 J/E flag position on the 1P, Training and
    Bonus screens; S2-3 token hit test (also the 12CB branch); S2-4 VS CSS stock counter and Manual arrows.
  - s3 (screens; `s3a_*`): S3-1 extra pause controls on every pause kind; S3-2 changing the song in the pause stops the
    Time Twister song changes; S3-3 VS Stats underlines; S3-4 Player Tag name alpha; S3-5 Highest Taken; S3-6 stripe
    state kept between pages; S3-7 team costumes when a team mode starts from the VS menu; S3-8 Player Tag black copy.
  - s4 (audio; `s4_*`): S4-1 Long KO never falls to the regular KO; S4-2 Clang every time; S4-3 Close match; S4-4
    announcer `used` reset, deviation; S4-5 announcer stops at GAME SET; S4-6 SFXReplace on the Remix Modes screen;
    S4-7 general comment draws until a free clip; S4-8 credits music from the menu.
  - a1 (`a1_*` to `a4_*`): A1-1 Goemon/Ebisumaru cloud stick fix on MoveAuto frames; A1-2 Sheik needle and Peppy
    revolver charge states; A1-3 level 10 offstage detection for Metal Mario and Metal Luigi; A1-4 level 10
    Smashketball prediction aims at the ball.
  - Fixed along the way, same causes: the KOTH first hill waits for the 120-frame start timer; the panel Damage is not
    reapplied on a respawn; the Waddle Doo beam start-up timer no longer overlaps the star lifetime.
  - Not covered by the auditors (read only in part, data tables, or left to another batch): the Game Boy Land camera
    patches in `Camera.asm`; the Great Bay CPU check (`Hazards.asm:3591`); the Casino knockback path into the
    `Surface.asm` table; Peach's Castle II Banzai Bill and the Scuttle Minion (unreachable); the stage-select menu
    routines of `Stages.asm`; the Stamina and Tag Team VS-menu and CSS presentation; the 12CB CSS presentation and
    Custom-set editing; the Remix 1P and All-Star randomization, title cards and final stages in
    `SinglePlayerModes.asm`; `replace_victory_image_` file ids and `env_mapping_fix_`; the Item Switch interface;
    most of `Size.asm`, `GFX.asm` and `CharEnvColor.asm` display lists; the CSS debug menu rendering; the KOTH Times
    digit positions on the results screen; the body of `_improve_recovery_drift` and the recovery routines registered
    outside `*/AI/`; Kirby branches of Wario, Marina, Conker, Lanky, Peach, Wolf and Piano read by sampling. The same
    class as F2-4 (a Remix fighter that reaches a vanilla `ftManagerInitFighter` case through its parent's
    initial script) was not checked for the other fighters.
- [x] T14.4 `hazards.md`, the mode docs, `announcer.md`, `menus.md`, `panel.md`, `deviations.md`,
  `patch_audit_scopes.md` and the `docs/bugs/` write-ups updated with each lot.
- [x] T14.5 Second pass, read-only (2026-11-16): the parts listed as not covered above, 15 batches (g01 to g15), same
  method. 256 routines equal to the asm, 36 confirmed differences in 10 batches, 11 already documented, 13 refuted.
  No difference in g05 (Remix 1P randomization, duo and polygon selection), g09 (`Size.asm`), g10 (`GFX.asm`,
  `CharEnvColor.asm`, `moveset.asm`, `Spawn.asm`, sword trail), g14 (the Kirby branches of Wario, Marina, Conker,
  Lanky, Peach, Wolf, Piano and the Dedede hat hooks) and g15 (initial scripts of every Remix fighter, entry routines,
  the shared fighter files, the N* folders).
- [x] T14.6 Fix the T14.5 findings (2026-11-16), lead reruns on the committed build (all rc=0, no abort/assert line):
  - g01 (camera, CPU stage checks): G01-1 CPUs avoid the tornado on Hyrule Castle DL, Omega and Remix (`g01a_torn*`:
    the twister check runs on every call on stages 4, 103, 104, 151); G01-2 CPUs leave a dropping platform on Mushroom
    Kingdom Remix, Big Boo's Haunt and Yoshi's Island II, G01-3 and the Great Bay turtle below y -1000 (`g01a_*`:
    Recover begins on the forced frames, not at y -900); G01-4 the pause pan offsets are cleared when a battle camera
    is made (`g01a_pan`).
  - g02 (stage select): G02-1 the random stage with Stage Select off never repeats the previous one (3000 draws);
    G02-2 R/Z page switch sound; G02-3 C-Down variant sound, deviation; G02-4 the idle auto-select; G02-5 Time Twister
    preview file; G02-6 R/Z, L and C-Down are exclusive of a cursor move (`g02_snd`, `g02_sim`).
  - g03 (Stamina VS menu, Tag Team CSS): G03-1 H.P. stick step ends at 999; G03-2 Stamina label position; G03-3
    Stamina Team rule text; G03-4 the H.P. picker background (`g03_*_after`); G03-5 Tag Team CSS toggles rewrite the
    queue (`g03_tt_after`).
  - g04 (12CB CSS): G04-1 J/E flags on the panel; G04-2 a set change re-evaluates the panel fighter (`g04_a2`); G04-3
    a same-character portrait change refreshes the panel (`g04_a3`); G04-5 leaving with B keeps the battle
    (`g04_leave1`); G04-6 set-arrow hit zones (`g04_a6`). G04-4 (the Random entry of the Z/R cycle) was done as T14.7.
  - g06: G06-1 the Fox ally of the Mario Bros. row takes the alternate costume (`g06a_after1`); G06-2 Game Over keeps
    the human's saved Size (`g06b_after`). Same cause, fixed by the lead: the Remix 1P pause exit no longer writes
    Size 0 (`lead_pexit_after`: `remix_panel_set` 0 hits, 1 before).
  - g07: G07-1 the panel Initial Damage is not applied in All-Star (`g07a_after`); G07-2 the Continue screen has the
    env-mapping lights (`g07b_after`).
  - g08 (items): G08-1 the Cloaking Device spawn runs the Maxim Tomato tail (13 arrows for 13 spawns); G08-2 the Item
    Switch exit saves the extended mask with everything off; G08-3 L and R are read from tic 1; the Turnip keeps its
    "never despawn" bit like Mr. Saturn (`g08_*_after`).
  - g11 (CSS panel menu): G11-1 Reset clears the Manual and Last stock counts; G11-2 a Stock change refreshes the Tag
    Team queue; G11-3 arrows blink 10 on, 10 off (`g11*_rerun`); G11-4 CPU-port panel values after 1P, deviation.
  - g12: G12-1 the Special Zoom gate accepts the 1P and Remix mode scenes (`g12zoom_*_after`: the decisive-stock check
    runs; `lead_zoom1p`, a 1P match with the damage forced to 300 under gdb: one KO zoom starts); G12-2 throw announcer damage,
    deviation; G12-3 the menu music alternate roll on every entry (`g12bgm_after`).
  - g13: G13-1 `_find_closest_ledge` lookup by loop index, deviation.
  - Still not compared, by their nature: data tables read from the Remix image (stage, title-card, All-Star and
    announcer tables, GFX routine streams), routines the asm builds with `OS.copy_segment` from vanilla code (checked
    against the decomp C, not the ROM bytes), display-list and render-order details, and the unreachable Banzai Bill
    and Scuttle Minion.

- [x] T14.7 G04-4, the Random entry of the 12CB Custom set (`TwelveCharBattle.asm` `check_input_` 4226-4300 over
  `Training.entry_id_to_char_id`, whose last entry is the Random placeholder; `was_random` in `game_setup_` 5295-5322,
  `gentlemens_reset_` 3666-3690, `handle_reset_` 2122-2185). Plain Remix behaviour that was missing from the port, not
  an owner decision; it missed the release that carried the rest of M14. Lead runs (2026-11-17, 12CB CSS, P1 on the
  Custom set over the Mario slot), unit tests 8340 checks, 0 failed:
  - `lead_g44_cycle`: Z from Mario shows "?" (panel RANDOM), Z again Poly Crash, R back to "?", R to Mario.
  - `lead_g44_match`: the game starts with a rolled fighter (`game 0 p1 fkind 1 ... random 1`); back on the CSS the
    slot shows Fox, dimmed as defeated, Best Character FOX.
  - `lead_g44_winner`: same game won by P1; on the CSS his token sits on the slot, now Fox, panel FOX.
  - `lead_g44_undo`: game taken back from the pause (`game taken back, now game -1, pool 24 / 24`); the slot is "?"
    again with the token on it, panel RANDOM.
  - `lead_g44_reset`: the RESET button puts the "?" back and the pools at 24 / 24.
  - `lead_g44_reg` (`css12cb_match_ko_back.txt`, no Random): unchanged, `random 0`.
  - Deviation kept: a taken-back game also forgets `was_random` (the asm leaves it set), row in `deviations.md`.

### M15. Remix text in the game's display layers (opened 2026-11-17)

Smash Remix draws its strings as objects of the game (`Render.draw_string_`, `Render.draw_texture_`), each in a room
(a display link with its own camera), so every other layer of the screen sorts against them: in the 12CB character
select the text is in room 0x1E (camera priority 35) and the cursor in link 32 (priority 20, drawn later), so the hand
passes in front of "Character Set" and the set names. The port draws all of it through `remix_hud_text` (text, bars
and rectangles; about 100 call sites in 29 files) as an overlay over the finished game image, so the text covers the
cursor, the tokens and any panel that Remix draws in front of it. One consequence already has a workaround and a
`deviations.md` row: the Training action rows hide while the Z menu is open, because the overlay would cross the menu
that covers them in Remix (room 0x39).

- [x] T15.1 Inventory, read-only (2026-11-17): [`text_layers.md`](text_layers.md) lists the 88 overlay call sites by
  screen family with the asm call, room and group of each, the camera order of every screen and the 39 calls whose
  overlay order is visibly wrong: all 25 of the character selects (the cursor, held and dropped tokens pass over the
  text in Remix), 8 of the battle HUD, 3 of the results, 2 Training action rows and the title version. The stage
  select, the pause legend, Gentlemen's Reset and the rest of Training already match, because Remix draws those
  strings in the front-most link. Each batch was rechecked against the asm by a second reader (19 corrections, the ones that change a verdict
  applied). The audit also lists seven differences that are not layer order (lifetime of "Game NN", a missing hide of
  the 12CB Custom legend, the Training rows over the vanilla pause menu, two sprites on a different link, the Input
  Display path, the unclipped music banner), to fix with their screens in T15.3.
- [x] T15.2 Draw path (2026-11-17): a layer is a GObj in the asm's group and display link
  (`ifRemixTextLayerMake`, `decomp/src/if/ifremixtextlayer.c`) that draws the slots bound to it with the game's sprite
  functions, from the Sprites of the Remix font file; the `remix_hud_text` API, slots and put calls stay, and a slot
  with no layer stays on the overlay. Pure rules (route table, life of a put in game tics, per-glyph layout, rectangle
  pixels) in `port/remix/remix_hud_layer_rules.c` with unit tests (8465 checks, 0 failed). Mechanism and limits in
  `text_layers.md`, "Draw path". Pilot: the 12CB character select texts in room 0x1E, groups 8, 9 and 10. Lead runs,
  each against the same input with `SSB64_REMIX_TEXT_OVERLAY=1`:
  - `lead_t152_layer` / `lead_t152_overlay`: the P1 hand covers "Character Set" and the set name on the layer, the
    text covers the hand on the overlay; same text position.
  - `lead_t152_best` / `lead_t152_best_ov` (after a game): "Best Character", DONKEY / LINK and the pools 22 / 24 drawn
    by the layer under the hand.
  - `lead_t152_center_*` (2D centred 4:3) and `lead_t152_off_*` (widescreen off): same position as the overlay.
  - No `DLBuffer`, abort or assert line in any log. The layer text is softer and a little darker at the edges than the
    overlay's, as the game multiplies the colour by the intensity of the glyph texel.
  - Deviation kept: the drop of a short glyph in a squeezed string (row in `deviations.md`).
- [x] T15.3 Move the screens over, one lot per screen family, each with an in-game test that shows the layer order
  (cursor over the text, panel over the text) and a comparison of the text position with the current overlay:
  the 12CB character select and results; the CSS panel menu and the VS, 1P, Training and Bonus selects; the stage
  select; Training (Z menu, action rows, legends: the hide workaround and its `deviations.md` row go away); the pause
  legend and Gentlemen's Reset; the battle HUD texts (player tags, KOTH, Smashketball, Tag Team, HRC, Multi-Man); the
  results screen and Match Stats; the title version and the Item Switch legend.
  Every lot is also checked in the three picture frames: widescreen with 2D stretched, widescreen with 2D centred
  4:3, and widescreen off. The CSS panel menu (the button on each card and its Shield, Player Tag and Visibility
  rows) is the reference case: its text already had a widescreen fix on the overlay
  (`docs/bugs/remix_hud_text_widescreen_stretch_2026-11-13.md`), so on all four cards the text must sit inside its
  card in each frame, as it does today.
- [x] T15.4 Remove what the overlay no longer needs, update `menus.md`, `panel.md`, `deviations.md` and the mode docs,
  and build every platform locally (desktop, Switch, PS5, Android) before any tag.
  Done 2026-11-17. Four lots (CSS panel menu and the VS, 1P, Training and Bonus selects with the 12CB Custom legend; 12CB, VS
  results and Match Stats; battle HUD; Training, pause, stage select, title, Item Switch, banner and Input Display), each
  checked in game against the forced overlay and in the three picture frames, then reviewed twice. The six non-layer findings
  were fixed with their screens; the Training action rows and Match Stats tag name deviation rows are gone. The overlay draw
  path and its env var are removed; `text_layers.md` describes the built system. Lead runs: `lead_t153_pm` (four panel menus:
  the hands pass over Shield and the text sits inside each card) and `lead_t153_12cb` (12CB match). 8516 unit checks, 0
  failed. Built locally: desktop Linux, Switch, PS5 and Android; Windows and macOS were not built (no host).

### M16. Ledger re-audit follow-ups (opened 2026-11-18)

The 2026-11-18 independent re-audit (326 asm scopes of the 2.007 `OS.patch_start` patches that the hook detection
flags as unhooked, symbolic or no-function, triaged against the port code without reading the ledger) confirmed every
ledger verdict except one, `BGM.asm:1407 prevent_results_bgm_on_css_` (the ledger's `NA` named a mechanism that does
not hold). Follow-ups land here.

- [x] T16.1 Port the `prevent_results_bgm_on_css_` guard (2026-11-18): pure rule
  `gmRemixMenuMusicRulesResultsBgmShouldReplay` (`gm/gmremixmenumusicrules.c`) applied in
  `mn/mnvsmode/mnvsresults.c:3268`, the exit signal read from the results screen's own exit path (`scene_curr` off
  `nSCKindVSResults` before its `syAudioStopBGMAll`). The A/B smoke run shows the race does not reproduce on PC (async
  bridge stop), so the guard is preventive and asm-faithful rather than a visible fix; the ledger line and the bug
  write-up say so (`docs/bugs/results_bgm_replay_on_exit_2026-11-18.md`). Unit test in `tests/remix/test_menu_music_rules.c`
  (8520 checks, 0 failed); `ssb64` builds clean; lead runs `bgmgap_before` / `bgmgap_after` (1-stock match into the
  results screen, START exit, `SSB64_BGM_LOG=1`): identical correct BGM sequences, one results-track play, the CSS
  starting its own screen track.

---

## Progress estimate (update at every checkpoint)

From the patch audit (2026-11-05): share of the Remix patches that are ported, adapted or `n/a` with a checked reason, per
area (patch counts of the ledger groups; an S-effort item weighs as much as an L one, so this undercounts the work done).

| Area | Weight | Done |
|---|---|---|
| Fighters (roster, specials, projectiles, entries, shared patches, per-fighter tables) | 35% | 100% (every GP, SH, FT, CT item `done` or `n/a`) |
| Items and Size | 10% | 100% (IT) |
| Stages (loader, select, hazards, camera, surfaces) | 20% | 100% (ST) |
| Audio | 10% | 100% (MS audio scopes; listening pass T9.1b done) |
| Settings, menus, modes | 20% | 100% (MN, MD, OP, MS) |
| Final verification | 5% | 100% (Phase 8, T9.3, M13, T9.1b and T10.2 done) |
| **Weighted total** | | **100%** (2026-11-15: the owner's passes closed the last 1%; the 2026-11-05 audit estimate was ~72%) |
