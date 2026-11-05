# Smash Remix 2.0.1 in the PC port: roadmap

This is the **only plan** of the Remix implementation. Whoever works on the Remix (agent or human) follows this file in
order, without opening work outside it. Widescreen / aspect ratio is **out of scope** (the game stays 4:3). The reference
documentation is indexed in [`README.md`](README.md).

## How to use (work protocol)

1. At the start of each session, read **Current position** and the open phases of M11.
2. Work **only on the first open item** of the current phase. A blocked item gets `BLOCKED: <reason>` and the next one is
   done. Never start the next phase with items open in the current one, unless a block is recorded.
3. An item is ticked `[x]` only after it passes its acceptance test (build plus the test it names). Note the short evidence
   next to it (commit, stage id, capture), and set its state in `patch_audit.md` when it is an audit item.
4. End of a phase = **checkpoint**: bug write-up in `docs/bugs/` when one applies (with its index row), commits in the
   decomp and in the main repository **without attribution trailers**, update of **Current position** below and of the
   user's checkpoint memory (`project_remix_port_checkpoint.md`, outside the repository). It is also a checkpoint when the
   user writes "checkpoint".
5. Workers (Agent) run in parallel, one small item per worker, with a brief written for the item (not kept in the
   repository). They write code or research; the lead builds, runs the game and signs off.
6. A significant bug fixed becomes `docs/bugs/<topic>_<YYYY-MM-DD>.md` with a row in `docs/bugs/README.md`.
7. Reply to the user in Portuguese; everything written in the repository is in English. Report "Remix playable" (T9.3)
   only after Phase 8.

## Fixed rules

- Toolchain and dependencies only in `.toolchain/` (no brew, system pip, sudo, `/opt`, `/usr/local`, `~/.cache`,
  `~/.cmake`). Python: `.toolchain/host/venv/bin/python -I`. `builtin cd`.
- Build: `PATH=$PWD/.toolchain/host/ninja:$PATH .toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target ssb64 -j 4`
  (absolute paths; a new file in `decomp/src` needs `cmake -S $PWD -B $PWD/build`).
- ROM-derived bytes never go into versioned files. Authorised source: `.toolchain/pkgs/smashremix-src`.
- No fallback to the parent fighter and no containment hacks: the behaviour is the Remix's. A necessary deviation is
  recorded in `deviations.md` (or a `docs/bugs/` write-up when it comes from a bug).
- Code in English, no `//` comments that repeat the code. `#ifdef REGION_REMIX` inside vanilla files.
- References in comments: `torch/` and `libultraship/` are independent repositories, so their comments do not cite docs
  of this repository. `decomp/` may cite `docs/remix/*.md` and `docs/bugs/`. Durable findings go into the topic doc
  (`docs/remix/<topic>.md`), never into per-task notes or `build/`.
- Tests: skill `remix-smoke-tests` (`.claude/skills/remix-smoke-tests/`: `vs.sh`, `special_matrix.sh`, `stage_sweep.sh`,
  `item_cpu_sweep.sh`, `css.sh`, `sheet.py`, inputs and how to read results), captures in `build/agent-shots/vs/`; reference
  emulator `scripts/ref-emu/capture.sh` (`notes/i8_reference_emulator.md`). A `CAMetalLayer nextDrawable` crash is a macOS
  flake: rerun before investigating.

## Current position

**In progress: M11 Finalization, Phase 5b (gaps found by the patch audit of 2026-11-05).** What is left for "Remix playable"
is Phase 1 (rest of I7) and Phases 5b to 8 of M11, at the end of this file. The closed milestones are summarised in
**History and decisions**.

| Milestone | State |
|---|---|
| M0 to M10 | closed; open leftovers: T9.1b and T10.2 (user), T9.3 (after Phase 8) |
| **M11 Finalization** | Phases R, 2, 3, 4 and 5 done; Phase 1 done except the rest of I7; Phases 5b, 6, 7 and 8 open |

- **Next step:** Phase 5b, lot by lot, then Phase 6 (AI) and Phases 7 and 8, with the rest of I7 in Phase 7.
- **Depends on the user:** T9.1b (listening and visual pass) and T10.2 (Switch hardware). "Remix playable" (T9.3) is only
  reported after Phase 8.
- **Last checkpoint:** CP12 (2026-11-05, Phases 4 and 5 closed; outer `e8faa57`, decomp `be259fe8b`).
- **Overall estimate:** ~72% by the table at the end of this file (recomputed from the patch audit, 2026-11-05).

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
- [ ] T9.1 Full sweep: the automatable part is done (sweeps, 4-player matches in six modes, Training through the CSS); the regression pass is M11 Phase 8. Defect fixed: Salty Runback/R+L combo needed the held state **equal** to the mask (`Joypad.check_buttons_all_`); `gmremixgameendinputrules.*`.
- [ ] T9.1b Listening pass by the user (the lead cannot hear): stages, victory song, Sound Test, hit SFX, menu graphics; `t9_1b_checklist.md`. Open (M11 Phase 8).
- [x] T9.2 Vanilla `REGION_US` builds and plays: CMake of `us`/`jp` skips Remix-only sources (`ft/ftremix`, `gr/grremix`, `it/itremix`, `port/remix/`, keeping `remix_debug`); `REGION_REMIX` guards in `ftport.c`, `port.cpp`.
- [ ] T9.3 Report to the user in Portuguese ("Remix playable"). Only after Phase 8.

### M10. Switch
- [x] T10.1 `scripts/switch/build.sh remix` (`BattleShip-Remix.nro`; fix: `<math.h>` before `<ft/fighter.h>` in `ftremixaipostsheik.c`) and `scripts/switch/pack.sh`.
- [ ] T10.2 Hardware validation by the user (`docs/switch_port_2026-10-12.md`); six rounds of findings, fixed except the two listed last:
  - R1 black screen/slow audio: `gfx_vtx_addr_is_unresolved` rejected pointers below 4 GB (`libultraship/src/fast/interpreter.cpp`). R2/R3 title offsets (`mntitle.c`); the odd 1P Game was Classic Co-op, now off by default in `REGION_REMIX`.
  - R4/R5 CSS `menu_zoom` (`tools/gen_remix_roster.py`), entry statuses from `Character.entry_action.table`, default save masks (`scmanager.c`), watchdog/slow-frame logs (`port/port.cpp`).
  - R6 Tag Team crash: `sSCVSBattleStatusBuffer[100]` (`scvsbattlefiles.c`) overflowed with 24-fighter queues; 1024 under `REGION_REMIX` plus a critical log in `lbreloc_bridge.cpp`.
  - Open (Phase 7): `gfx_step: walked past registered DL range` stutter, Metal Mario tiny in Training CSS (not reproduced).

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
from the user's ROM on first run; CI builds `remix` in the matrix of `.github/workflows/release.yml`.

Closed phases:

- [x] **Phase R** (recover interrupted work): Kabel HUD of the HRC, button sprites as native SObj (`menus.md`), versioned package scripts (`scripts/linux/`, `scripts/macos/package-remix-contained.sh`; `build_and_roms.md`), old subagent work re-checked, current `parity.md` (`adapted` counts as done).
- [x] **Phase 1** (infrastructure), all but part of I7: I1 display groups `gmremixgroupdisplay*`; I2 extension sprites per SObj `if/ifremixsprite*`; I3 Kabel text `port/remix/remix_font_layout.*`; I4 runtime BGM `gm/gmremixbgm*`; I5 `tools/n64_addr_to_symbol.py`; I6 stage group 2 and camera `gr/grremix/grremixstagebg.*` (closes T1.5b); I8 reference emulator `.toolchain/ref-emu/` (`scripts/ref-emu/capture.sh`, `notes/i8_reference_emulator.md`). I7 part image/palette swap (`ftremixpartimage*`, `ftremixyoshiegg.c`, `notes/i7_part_image.md`) delivers extra costumes in match and in the CSS cycle and the Yoshi shield/escape egg colour. **Open:** thrown, laid and entry egg colour. Classic Sonic is not I7 (main-file swap); done in Phase 5.
- [x] **Phase 2** (visual of the modes, T8.3): all `remix_hud_text_*` callers use the asm parameters; Match Stats (sprite buttons, Kabel page, stock icons); pause legend; 12CB CSS steps 1 to 19b (`css_reference.md`; Gentlemen's Reset `if/ifremixgentlemensreset.*`, Custom set and StockMode Manual in `mn/mnplayers/mnremixtwelvecb*`); Multi-Man HUD sprites; HRC counter; Training item-name image, D-pad legend and Z menu of 23 entries (`sc1ptrainingremix*`); results labels (`mnVSResultsMakeRemixRow`); KOTH meter; music-title banner (LUS toast and `remix_overlay.*` removed). Deviations: "Press Z" opens the port menu in the prompt (`remix_training_open_menu`); the Dragon King Multi-Man HUD, the Smashketball result row and the 12CB CPU-opponent reset branch were checked by pure rules only. Empty-panel Smash emblem: no Remix patch draws it (nothing to port).
- [x] **Phase 3** (stages and gameplay, T2.9b, T2.11): Size of Ness (`ftremixsizeness.*`) and entries, Banjo USP wave, Goemon cloud, Yoshi `stage_hazard_table`, Rainbow Chomp, Metallic laser, Pirate cannonball (its speed does not exist in the asm; `cannonball_properties_struct` is dead data), Whispy wind box (`grRemixDreamLandGetWindBox`), Z-cancel cruel (`docs/bugs/remix_zcancel_egg_lp64_2026-11-04.md`), D-pad Control `ftremixdpad.c`, Corneria laser, Marina throw_turn, throw announcer, Shield Costume `ftremixshieldcostume.c`, Hitbox display. Zebes/Japes fields proved not portable. Deviations: NKirby keeps the vanilla fthrow (apparent Remix bug, compare in the emulator in Phase 8); Big Boo background sprite (`bg_animation_`) not ported; Kirby Cube pair `n/a`.
- [x] **Phase 4** (CSS, stage select, panel, screens; T8.1/T8.2): title "Smash Remix v2.0.1" (`mnremixtitleversion.*`), credits glyphs and speed assist, Gallery with 40 (`mnremixgalleryrules.*`), item-switch legend, RANDOM ALL, transition logos; stage-select grid of 5 pages with roulette and tournament layout (`mnremixstage*`); panel features (D-pad Map, DpadControl, tag names, Start With, Practice 1P, Skip Start Cheer, Enemy Player/AFK, Link JP sounds, CPU costumes); VS CSS bookends, variant icons and J/E flag (`tools/test_remix_css_offsets.py`), slide-in `mnremixcssslide*`, high-score gating `sc1premixhighscores*`; per-port panel menu in the CSS (`mn/mnplayers/mnremixcsspanelmenu*`, `panel.md`); CSS crash fix `docs/bugs/remix_css_bookend_width_img_2026-11-04.md`. Deviations: Roulette ignores directions and draws among the unlocked stages of the page; tournament Random list not ported (reopened: ST-23); port RNG; no `dont_freeze_stage` or Settings shortcut; the CSS model does not reload on Model/Kirby Hat/Visibility/Size changes (reopened: MN-20). Handicap icons above 9 moved to Phase 6.
- [x] **Phase 5** (modes): CSS mode headers, announcers, GOAL/Time picker, Tag Team queue drawn in the CSS (`css_reference.md`, `mnremixcssmoderules`); Classic Sonic (`CSONIC_MAIN`, `deviations.md`); yellow team (`css_reference.md`); Tag Team/Last/Tug of War/Stamina (`modes/tagteam.md`, `modes/stamina.md`, `modes/tugofwar.md`; `docs/bugs/remix_tugofwar_asm_deviations_2026-11-05.md`); KOTH and Smashketball visuals (`modes/koth.md`, `modes/smashketball.md`); 1P leftovers (`modes/singleplayer.md`: records, Remix BTT/BTP, Bonus 3, Rest Area, Giga Bowser end); result extra row (`gmRemixGameEndRowsRowTics/RowY`); Training Show Action, Hold to Exit. Deviations: 12CB announces the vanilla voice instead of "Player N Wins" (reopened: MD-4); yellow "WINS!" x = 175.0 as the Remix reads it outside the array (the per-fighter x is MD-14); team card shows one Dedede (one-frame pose; checked against the emulator in Phase 5b lot 6); the Training Custom spawn on respawn does not exist in the asm.

- [ ] **Phase 1, rest of I7** (model part image or palette swap, `MODEL_PART_IMAGE_INIT_`; `notes/i7_part_image.md`): the
  Yoshi egg colour when the egg is thrown, laid and at the entry. Costumes beyond `num_costumes` in the match and on the
  CSS, and the shield and escape egg, are done.
- [ ] **Phase 5b** gaps found by the patch audit (2026-11-05, before Phase 6). The ledger is
  [`patch_audit.md`](patch_audit.md): one ID per item (prefix per group), asm location, decomp function, what the player
  misses, hook site, effort; the per-scope map is [`patch_audit_scopes.md`](patch_audit_scopes.md). **Acceptance per item:**
  ported and seen in game (a capture compared with the reference emulator when it is visual), or `n/a` with the reason
  checked in the asm; its state set in the ledger. **Acceptance of the phase:** no open ledger item outside the AI ones
  (Phase 6), `tools/audit_remix_patches.py` re-run, `parity.md` consistent with the ledger. Lots, most visible first:
  - [ ] Lot 1, Character Data screen: emblem of the 18 extra fighters (`extend_series_logo_offsets`, 0x80131F2C in
    `mnCharactersMakeEmblem`: `ResultsScreen.series_logo_offset_table` and the zoom and colour tables), special-move names
    (`extend_special_attack_offsets` 0x80132E24 and `extend_special_attacks_file` in `mnCharactersUpdateMotionName`; closes
    `REMIX-TODO(chardata-motion-names)`), special-move animations (`special_action_pointers`; the extras use
    `dMNCharactersCommonMotionDescs` today), jab combo (`jab_action_pointers`, 0x801326E4), MN-16 and MN-17, and the crash
    seen in `b12_chardata3` (scene 58, A on Characters, R up to Dedede: SIGSEGV after about 17 s in
    `lbCommonAddFighterPartsFigatree` from `ftMainSetStatus` from `mnCharactersFighterProcUpdate`). Reference capture:
    `build/ref-shots/chardata2` (Dedede with the Kirby emblem and "B Inhale"; Falco with the Star Fox emblem, "B+↑ Fire
    Bird" and "B+↓ Reflector").
  - [ ] Lot 2, match flow and stages: ST-1 first (every match on a Remix stage ends with FAILURE instead of GAME SET),
    then the other ST items (the Remix ground knockback table ST-2 drives the Z-cancel punishments, LAVA and the stage
    surfaces) and the MS pause camera, spawn and stage items.
  - [ ] Lot 3, fighters: GP-1 (jab 3 and rapid jab of the Remix fighters: the parent-kind and rapid-jab events have no
    listener), the SH shared patches (J Yoshi, the Ness and Pikachu families, pipe turn for 11 fighters, Falcon punch
    owners), the FT rows outside AI.asm (Lucas has 12 of 14 patches missing), the CT per-fighter tables and the other GP
    items.
  - [ ] Lot 4, items and Size: the IT items (Remix items never come out of containers, IT-2, first).
  - [ ] Lot 5, CSS, menus and screens: the MN items and the MS screen items (Remix splash after the N64 logo, Gallery
    entry and controls, "Other Screens" in the port menu, VS demo name plates).
  - [ ] Lot 6, modes: the MD and OP items, plus the 1P title cards checked against the reference emulator (one Marina pose
    for the Marina team, Mad Piano size, Metal Mario Bros texture: reported by earlier 1P work, no capture yet).
  - [ ] Lot 7, audio: the MS BGM, FD theme, MIDI and FGM items (per-song volume and priority overrides, alt and random
    music outside VS, the 30-second hurry swap by track).
  - [ ] Lot 8, `parity.md` generated from the ledger states (`tools/gen_remix_parity.py`), so a file with open items is
    never `ported`.
- [ ] **Phase 6** AI. **Acceptance:** `REMIX-TODO(AI)` at 0, every AI item of the ledger closed (the FT rows of AI.asm,
  including `custom_recovery_logic`, whose registry slot nothing fills; the SH CPU hooks; GP-8 reflect and absorb; the MD
  KOTH and Smashketball CPU targets; FT-11 human handicap up to 40 and the icons above 9), recovery at level 9 without
  regression (base 181/222). Also from the 2026-11-03 plan: `cpu_post_process` per fighter, `auto_jab_grab_`, AERIAL_CHASE,
  PUFF_DSP, the vanilla PREVENT rows, Banjo P06, Kirby P14, P11 of Sheik, Peppy, Bowser and Giga Bowser, the 30 `cpu_*`
  routines, `_improve_recovery_drift`, the Default CPU LVL refresh and the `CpuLevel9` conflict with level 10. Reference:
  [`ai.md`](ai.md).
- [ ] **Phase 7** open bugs, instrumentation and packages.
  - [ ] Bugs: Poly Young Link missing on the CSS until the first reset; Metal Mario partly tiny in Training; Switch stutter
    (`walked past registered DL range`, stale token 0x57EE); the rc 139 flake on Tal Tal.
  - [ ] Peach's Castle: blue quads with the roof-tile texture float in the sky when the camera zooms out (2026-11-05,
    `b11_pc_wide` in Remix and `b11_us_pc` in the vanilla build, both with P1 walking left; the reference emulator shows
    nothing like it). A port defect of the stage, older than the Remix work.
  - [x] Giga Bowser 1.70 zoom on the CSS (2026-11-05): not a bug. The reference capture (`build/ref-shots/giga_css2`, D-pad
    up over Bowser on the VS CSS of the real Remix) shows Giga Bowser overflowing the panel at the port's size
    (`b10_giga_css`).
  - [x] `PEACH_TURNIP_GFX` out of bounds (2026-11-05): not a bug. Dump of file 5144 (`SSB64_DUMP_FILE_ID`): slots 919 to 928
    are the turnip's 10 images; the two outside the file (927 and 928, targets 2482 and 2352 in a file of 1176 words) are
    images 8 and 9, and the turnip only uses images 3 to 6 (`stage_setting_` L181-217). Zeroing those slots, as the bridge
    does, is safe.
  - [ ] Instrumentation: `fflush` before `abort()`, hitch summary per scene, slow `lbReloc` report.
  - [ ] Test packages with the `.o2r` embedded (macOS script and AppImage), marked for personal use.
  - [ ] Final packages without the `.o2r` (macOS `.app`/`.dmg`/`.zip`, AppImage x86_64, Switch `.nro`): first confirm that
    first-run extraction works for the Remix (the game asks for the user's Remix ROM and builds `BattleShip-Remix.o2r`), as
    for the US build; `remix` in the `release.yml` matrix (`scripts/package-macos.sh` and `scripts/package-linux.sh` accept
    `SSB64_VERSION=remix`); install doc (which ROM: vanilla plus the Remix 2.0.1 patch, and how extraction works).
- [ ] **Phase 8** final verification. Regression: 175 stages, special matrix 95 × 3, item sweep, 4 players in every mode
  through the CSS and Training, the vanilla build, unit tests. User passes: T9.1b (listening and visual), T10.2 (Switch).
  Reference comparison: emulator captures (I8) against the port on every mode screen, CSS, stage select, pause and results;
  each difference is fixed or documented as an adaptation. Close: `parity.md` only `ported` / `adapted` / the approved
  `n/a`, T9.3 (report "Remix playable" with the list of adaptations) and the final checkpoint.

---

## Progress estimate (update at every checkpoint)

From the patch audit (2026-11-05): share of the Remix patches that are ported, adapted or `n/a` with a checked reason, per
area (patch counts of the ledger groups; an S-effort item weighs as much as an L one, so this undercounts the work done).

| Area | Weight | Done |
|---|---|---|
| Fighters (roster, specials, projectiles, entries, shared patches, per-fighter tables) | 35% | ~68% (GP, SH, FT: 400 of 590 patches) |
| Items and Size | 10% | ~93% (IT: 156 of 168) |
| Stages (loader, select, hazards, camera, surfaces) | 20% | ~64% (ST: 132 of 207) |
| Audio | 10% | ~78% (BGM, FD, FGM and MIDI scopes of MS: 25 of 32) |
| Settings, menus, modes | 20% | ~78% (MN, MD, OP, MS: 788 of 1004) |
| Final verification | 5% | ~55% (T9.2 done; T9.1 automatable part done; T9.1b and T9.3 after Phase 8) |
| **Weighted total** | | **~72%** (2026-11-05, after the patch audit: 35×0.68 + 10×0.93 + 20×0.64 + 10×0.78 + 20×0.78 + 5×0.55 = 72.1; before the audit the estimate was ~94% from the per-file parity table, which counted a file as done when it had a port module) |
