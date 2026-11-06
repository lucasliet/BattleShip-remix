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
5. Workers (Agent) run in parallel, one package of the current lot per worker (the lot lists its packages and which files
   each owns), with a brief written for it (not kept in the repository). They write code or research; the lead builds,
   runs the game and signs off. A lot closes with its own commit.
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

**M11 Finalization closed (2026-11-06): "Remix playable" (T9.3) reported.** Only the user passes T9.1b and T10.2 are
open. The closed milestones are summarised in
**History and decisions**.

| Milestone | State |
|---|---|
| M0 to M10 | closed; open leftovers: T9.1b and T10.2 (user) |
| **M11 Finalization** | Phases R, 1 to 8 done (Phase 8 on 2026-11-06) |

- **Next step:** none for the lead; the user passes T9.1b and T10.2. Phase 5b closed on 2026-11-06: every ledger
  item is `done` or `n/a`, `parity.md` regenerated (no `partial` row).
- **Depends on the user:** T9.1b (listening and visual pass) and T10.2 (Switch hardware).
- **Last checkpoint:** CP12 (2026-11-05, Phases 4 and 5 closed; outer `e8faa57`, decomp `be259fe8b`).
- **Overall estimate:** ~99% by the table at the end of this file (2026-11-06; only the user passes are open).

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
- [ ] T9.1b Listening pass by the user (the lead cannot hear): stages, victory song, Sound Test, hit SFX, menu graphics; `t9_1b_checklist.md`. Open (M11 Phase 8).
- [x] T9.2 Vanilla `REGION_US` builds and plays: CMake of `us`/`jp` skips Remix-only sources (`ft/ftremix`, `gr/grremix`, `it/itremix`, `port/remix/`, keeping `remix_debug`); `REGION_REMIX` guards in `ftport.c`, `port.cpp`.
- [x] T9.3 Report to the user in Portuguese ("Remix playable"), 2026-11-06 after Phase 8; adaptations listed in
  `deviations.md` and the `adapted` rows of `parity.md`.

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

- [x] **Phase 1, rest of I7** (model part image or palette swap, `MODEL_PART_IMAGE_INIT_`; `notes/i7_part_image.md`): the
  Yoshi egg colour when the egg is thrown, laid and at the entry. Done in Phase 5b lot 3 (package F2, with CT-9; evidence in the ledger rows GP-11 `pc_gp11_*` and CT-9 `pc_ct9_*`). Costumes
  beyond `num_costumes` in the match and on the CSS, and the shield and escape egg, are done.
- [x] **Phase 5b** gaps found by the patch audit (2026-11-05, plan approved the same day). The ledger is
  [`patch_audit.md`](patch_audit.md): one ID per item (prefix per group), asm location, decomp function, what the player
  misses, hook site, effort; the per-scope map is [`patch_audit_scopes.md`](patch_audit_scopes.md). Every open ledger item
  (251 at the audit) is placed below; lot 8 and the Phase 6 list close the count. The CPU-only scopes got the rows SH-15 to
  SH-27 and ST-27 on 2026-11-05 (Phase 6 step 1; 263 open items since).
  - **Acceptance per item:** read the asm scope, port under `#if defined(PORT) && defined(REGION_REMIX)`
    (`porting_guide.md`), a unit test in `tests/remix/` when the item is a pure rule, the in-game test of its lot, then its
    ledger state `done (<run name, evidence>)` or `n/a (<reason checked in the asm>)`. A visual item is compared with a
    reference-emulator capture.
  - **Acceptance of the phase:** no open ledger item outside the AI ones (Phase 6), `tools/audit_remix_patches.py` re-run,
    `parity.md` consistent with the ledger.
  - **Placement by file:** an item sits in the lot and package that own the file it edits, so some IDs leave their prefix's
    lot (results screen MD-12..17 in lot 5, OP-37, IT-8 and IT-10 in lot 2, IT-7 and MS-14 in lot 3, IT-1 in lot 6, the MS
    screen items in lot 5). Lots run one after the other.
  - **One owner per file:** packages of a lot run in parallel (one worker each) only when their files are disjoint; a
    shared file is serialized as each lot states.
  - **Duplicate IDs close together:** GP-1 = CT-14 (with the rapid-jab part of FT-38 and FT-43), SH-7 = CT-2, SH-2 = CT-8,
    GP-7 = CT-15, GP-8 = CT-21, FT-1 = CT-18, FT-8 = CT-19, MS-14 = CT-3, MS-15 = ST-20, the double-jump part of FT-28 =
    SH-1.
  - **Lot close:** build, `run_remix_mode_tests`, the lot's smoke runs, a regression mini-set (special matrix of the
    touched fighters, three VS matches on Remix stages, the VS CSS flow, the vanilla build compiles), then the decomp
    commit and the outer commit (submodule bump, ledger states, this file).

  - [x] **Lot 0, groundwork** (2026-11-05): 46 symbols exported (`REMIX_SYM_ResultsScreen__*`, `CharacterDataScreen__*`, the
    `Character.*.table` list, `MIDI`, `BGM`, `Spawn`, `Slippy.hip_translation*`; the MN-6, MD-21, GP-6 and MD-23 labels are left to their lots, they name code scopes and
    the data those items need is found there); `scene.sh`; the logs `SSB64_REMIX_FIGHTER_LOG`
    (`ftremixtestlog.c`), `SSB64_BGM_LOG` (`gmremixbgmlog.c`), `SSB64_PORT_MENU_OPEN`, `SSB64_REMIX_AI_LOG` (`ftremixailog.c`), checked in
    `l0_logs` and `l0_ai`; the Character Data reference lap `build/ref-shots/chardata_lap` (input
    `scripts/ref-emu/inputs/chardata_lap.txt`); ledger corrections. The other input scripts and reference inputs below are
    written at the start of the lot that uses them (they need in-game timing).
    1. Symbols: add to `EXTRA_SYMBOLS` in `tools/gen_remix_actions.py` and regenerate `port/remix/remix_symbols.h` (every
       label is in `build/remixasm/remix.sym`): `ResultsScreen.series_logo_offset_table`, `.series_logo_zoom_table`,
       `.series_logo_color_table`; the `CharacterDataScreen.extend_*` tables; `Character.*.table` for `crowd_chant_fgm`,
       `yoshi_egg`, `winner_fgm`, `str_wins_lx`, `str_winner_ptr/lx/scale`, `label_height`, `sound_type`,
       `costume_shield_color`, `ness_jump`, `pipe_turn`, `fighter_reflect`, `jab_3*`, `rapid_jab*`, `ai_attack_prevent`,
       `cpu_attack_weight`; `TwelveCharBattle.defeated_array`; the `CharacterSelect` random and bookend labels (MN-6); the
       Training magnifier background (MD-21); `custom_display_lists_*` (GP-6); the `DKMode` portrait pattern (MD-23);
       `CharEnvColor.override_table` (MN-20); `BGM.random_table`, `BGM.random_count` (MS-3); `Toggles.itsatrap` (MS-4);
       `Spawn.neutral_table` (MS-17); `MIDI.priority_override_table`, `.bend_range_override_table`,
       `.master_volume_override_table` (MS-22, MS-23). Check: build passes, each define matches `remix.sym`.
    2. Test tooling (`.claude/skills/remix-smoke-tests/`): `scene.sh <scene> <input> <name> [frames] [timeout]` (scene ids
       of the `SCKind` enum, `decomp/src/sc/scdef.h`: VS mode menu 9, CSS 16 VS, 17 1P, 18 Training, 19 and 20 Bonus, VS
       results 24, Character Data 26, N64 logo 27, 1P game 52, bonus stage 53, Training 54, Congratulations 55, credits 56,
       Data menu 58, Sound Test 59, Auto-Demo 61); input scripts for the Character Data lap, jab 3 and rapid jab, fsmash,
       dair, grab and throw, shield hold, bury mash, pipe, held charge, Dedede inhale, cargo, results with C held, practice
       with L on GAME SET, 1P pause with L, D-pad on the CSS, All-Star Rest Area wait, 12CB entry, pause with C/A/B and the
       stick at its limit, hit on a surface, Casino bumper, Puff sing, Super Sonic up-special after a hit, Congo Falls
       barrel exit, hanging on a platform that disappears.
    3. Env-gated logs in the port (off by default): `SSB64_REMIX_FIGHTER_LOG` (status, percent, ammo halfword, knockback,
       projectile lifetime), `SSB64_BGM_LOG` (track id at every BGM change), `SSB64_PORT_MENU_OPEN` (port menu open at
       boot), `SSB64_REMIX_AI_LOG` (CPU recovery attempts and outcomes).
    4. Reference inputs for `scripts/ref-emu/capture.sh`: Character Data lap over the 18 extras, 1P CSS, results with a
       Remix winner, 1P title cards.
    5. Ledger corrections found while planning: ids are in `port/remix/remix_roster_data.h`; hook path of FT-7 and GP-11;
       the FT-29 hook (`ftCommonAttackAirCheckInterruptCommon` not found); the "hip" labels of FT-44; subfolder paths of
       ST-1, ST-16, ST-22, IT-2, MS-14; the MS-22 hook site.
  - [x] **Lot 1, Character Data screen** (2026-11-05: MN-16 to MN-19 done; `l1_lap` against `ref-shots/chardata_lap`,
    `l1_dedede_forced`, stress runs `l1_stress_all`/`_rand`/`_dedede` without a crash; the crash write-up is
    `docs/bugs/remix_chardata_dedede_crash_2026-11-05.md`; emblem deviation in `deviations.md` 5.6) (`mn/mndata/mncharacters.c`, one worker), in order: the crash of `b12_chardata3`
    (scene 58, A on Characters, R up to Dedede: SIGSEGV after about 17 s in `lbCommonAddFighterPartsFigatree` from
    `ftMainSetStatus` from `mnCharactersFighterProcUpdate`); MN-16 and MN-17; MN-18, the emblem of the 18 extra fighters
    (`extend_series_logo_offsets` 0x80131F2C: `series_logo_offset_table` and the zoom and colour tables); MN-19, special
    actions, jab combo (`jab_action_pointers` 0x801326E4) and special-move names (`extend_special_attack_offsets`
    0x80132E24, file 0x10F6; closes `REMIX-TODO(chardata-motion-names)`). Test: the lap through every fighter without a
    crash, compared with `build/ref-shots/chardata2` (Dedede with the Kirby emblem and "B Inhale"; Falco with the Star Fox
    emblem, "B+↑ Fire Bird" and "B+↓ Reflector") and the full-lap reference.
  - [ ] **Lot 2, match flow and stages.** ST-1 opens the lot (every match on a Remix stage ends with FAILURE instead of
    GAME SET).
    - Progress (2026-11-05): all packages coded, built and committed; done in the ledger with their runs: ST-1, ST-2, ST-3,
      ST-4, ST-5, ST-6, ST-7, ST-8, ST-9, ST-13, ST-14, ST-16, ST-17, ST-20, ST-21, ST-22, ST-25, ST-26, IT-8, OP-37, MS-13,
      MS-15, MS-17, MS-26 (new: `Spawn.original_table`). Fixed on the way: the VS results screen aborted after a
      four-Remix-fighter match (`docs/bugs/remix_vs_results_status_buffer_2026-11-05.md`). Found: since Phase 5 the BTT/BTP
      stages run the bonus ground (`bonus_fix_1_`), so the VS shortcut cannot load them; the stage sweep skips them and they
      are run through the 1P path (SKILL.md).
    - Also done after the commit: ST-12 and ST-15 (lldb counts in CPU matches).
    - **Left (coded, not yet seen in game):** ST-10 (Peach BTT pipe), ST-11 (platform that vanishes under a hanging fighter),
      ST-18 (hazard item draw order), ST-19 (Rainbow Chomp past the bounds: not reached in 6000 frames), ST-23 (Stage Select
      OFF random), ST-24 (Training background by parent stage), IT-10 (ball into the Smashketball barrels: not reached in
      8000 frames, nor with `SSB64_SMASH_BALL_X` over a barrel). Each needs a scripted scenario.
    - S1 match flow (owner of `ifcommon.c`, `ftcommondead.c`, `gmcamera.c`), serial: ST-1 with ST-26; ST-17 (countdown
      stoplight and FGM); ST-20 = MS-15 with ST-21 and IT-8 (pause camera, stick limits, close-up zoom); ST-4 with MS-13
      last (Dragon King HUD, input display under the timer; calls in `sc1pbonusstage.c` and `sc1ptrainingmode.c` after S3
      and S4).
    - S2 surfaces (owner of `ftmain.c`): ST-2 (ground knockback table: Z-cancel punishments, LAVA, surfaces), ST-3, the
      `ftmain.c` part of ST-6, ST-22.
    - S3 barrels and bonus (`grjungle.c`, `ftcommontarucann.c`, `sc1pbonusstage.c`, `grcastle.c`): ST-5 with ST-6, ST-7
      with ST-9, ST-12, ST-8, ST-10, IT-10.
    - S4 stage visuals and options: ST-11, ST-13, ST-14, ST-15, ST-16, ST-18, ST-19, ST-23, ST-24 (after ST-1: both edit
      `grremixstage.c`), ST-25 with OP-37, MS-17.
    - Tests: `vs.sh` with `SSB64_VS_STOCKS=1` on Remix stages; `SSB64_KOTH_GOAL` / `SSB64_SMASH_GOAL`; surfaces on Big
      Blue, Mute City (153) and Casino (159) against reference captures; barrels on Congo (2, 105), Smashketball (90),
      Falls (94); bonus with `scene.sh 53` and `SSB64_SPGAME_STAGE` (172, 140, 161); toggles with `SSB64_REMIX_TOGGLES`;
      `stage_sweep.sh` for ST-25; FGM log for ST-17.
  - [ ] **Lot 3, fighters** (GP, SH, FT, CT outside AI). Progress (2026-11-05): all eight packages coded and built; roster special matrix (94 fighters × 3)
    clean; unit tests 6062 checks. Done in the ledger with their runs: GP-1 = CT-14, FT-38, FT-43, OP-20, SH-10, FT-15, SH-1,
    FT-50, SH-11, SH-12, IT-7, SH-4, GP-7 = CT-15; FT-19 `n/a`. The rest is ported and marked "in-game check pending" in the
    ledger. Verification batch (2026-11-05, 184 runs `pc_*`, scenarios with `lldb_count.sh`/`lldb_first.sh`): 41 more items done
    across lots 2 to 4 (ledger evidence); FT-49 first failed because the input released L before the voice frame, then passed; 27 items still need a scenario
    that triggers them. Open: GP-6 (Visibility per-part render modes: needs a renderer state for the
    alpha render mode), GP-12 (GFX 0x64 user not found). Deviations in `deviations.md` 5.8.
  F5 and F6 are single owners of `efmanager.c` and `ftmain.c`; F5
    starts after F2 and F3; the `ftcommonattackair.c` and `ftremixkirbypower.c` parts of FT-29 wait for F8 and F4.
    - F1 jab and smash (`ftcommonattack1.c`, `ftcommonattack100.c`, `ftcommonattacks4.c`): GP-1 = CT-14, FT-38, FT-43,
      SH-3, the attack part of SH-6.
    - F2 jump and Yoshi (`ftcommonjumpaerial.c`, `ftcommoncaptureyoshi.c`, `ftcommonthrown1.c`, guard and catch files,
      `ftremixyoshishared.c`, `ftremixpartimage*`): SH-1, SH-2 = CT-8, FT-28, FT-14, CT-9, the rest of I7, GP-11.
    - F3 Ness, Lucas, Pikachu (`wpness*`, `itnesspkfire.c`, `ftness*`, `ftpikachu*`, `wppikachu*`,
      `ftremixpianoshared.c`): SH-4, SH-5, the thunder part of SH-6, FT-27, FT-30, FT-31, FT-33, FT-34, FT-35.
    - F4 Kirby, Bowser, Samus (`ftmanager.c`, `ftremixkirby*`, Samus charge files, `ftremixbind.c`, `ftremixbowser.c`,
      `ftcommondead.c`): SH-10, SH-11, SH-12 with IT-7, SH-13 (hats part), FT-18, FT-19, FT-39, FT-40, CT-6, CT-12, FT-51,
      OP-20.
    - F5 effects (`efmanager.c`, `ftremixdkcaptainshared.c`, `ifremixannouncer.c`, `ftcommoncapturekirby.c`): SH-8,
      FT-20, FT-21, FT-32, FT-50, the `efmanager.c` parts of SH-4, SH-6, CT-9.
    - F6 `ftmain.c` by function: `ftMainSetStatus` GP-10, FT-29, FT-37; `ftMainProcPhysicsMap` SH-9, FT-15, FT-42; hit
      FT-47, FT-48; then GP-2 (`ftremixcommand.c`, `ftphysics.c`).
    - F7 tables (`ftparam.c` owner, `ftremixchartables.c`, `mpcommon.c`, `ftremixshieldcostume.c`, `ftremixdeschooks.c`,
      `sys/controller.c`, `ftcommoncapture.c`): CT-3 = MS-14, CT-4, CT-5, CT-7, CT-10, CT-11, FT-24, FT-44, FT-49, the
      `ftparam.c` parts of SH-1 and SH-13.
    - F8 others: GP-3..6, GP-7 = CT-15, GP-9, GP-12, GP-13, SH-7 = CT-2, SH-14, FT-16, FT-17, FT-22, FT-23, FT-25, FT-26,
      FT-41, FT-45, FT-46.
    - Tests: `special_matrix.sh` for every touched fighter, `vs.sh` with the new inputs, the Kirby copy test for F4,
      `SSB64_REMIX_FIGHTER_LOG` for ammo, knockback, heal and lifetime, reference captures for egg colour, Visibility and
      fireball.
  - [ ] **Lot 4, items and Size.** Progress (2026-11-05): coded and built; IT-2 and IT-3 done with runs; IT-4, IT-5, IT-6, IT-9 and IT-11
    ported, in-game check pending (the shadows are too small in the headless frames; the sing toggle showed no difference in the
    frames taken). Note: `SSB64_REMIX_FORCE_ITEM` rewrites every item made, container drops included; use `SSB64_FORCE_ITEM_KIND`
    for container tests. I1 (`itbox.c`, `itmain.c`, `ftremixstartitem.c`): IT-2 first (Remix items never come
    out of containers), IT-3. I2 (`ftshadow.c`, `ftremixsize.c`): IT-5, IT-6. I3: IT-4, IT-9, IT-11. Tests:
    `SSB64_FORCE_CONTAINER_DROP` with `SSB64_REMIX_ITEM_LOG` and `item_cpu_sweep.sh`; `SSB64_REMIX_PANEL` (Start With,
    Size, Visibility) against the reference; two matches in a row for IT-4; `SSB64_REMIX_FORCE_ITEM` for IT-11.
  - [ ] **Lot 5, CSS, menus and screens.** Progress (2026-11-05): coded and built; 38 items done with runs (MN-1, MN-2, MN-3,
    MN-6 to MN-10, MN-13, MN-14, MN-15, MN-20, MD-3, MD-12 to MD-17, CT-1, CT-13, OP-29, OP-30, OP-31, OP-41, MS-1, MS-7, MS-9, MS-10, MS-11, MS-12,
    MS-18, MS-20, MS-21, MS-24 and the new CT-24), MS-2 n/a (the port never shows the N64 logo); MN-4, MN-5, MN-11, MN-12,
    MD-20, MD-22, OP-32, OP-33, OP-40 and MS-19 ported, in-game check pending (they need clicks on CSS buttons or the
    mode menus; the CSS press point is the hand + (20, 3), see `inputs/css_panelmenu_visibility.txt`). Found and fixed on the way: the Gallery
    label read the common menu file; the port menu built before the Remix toggles loaded, so the Remix section never showed
    (now added after `remix_toggles_init`, `SSB64_PORT_MENU_PAGE` opens a page for screenshots); `syAudioGetBGMPlaying` cleared
    when a song without a loop ended, unlike `BGM.safe_id`; Giant DK crashed the VS results and Metal Mario and the polygons had
    empty menu motions (`Character.asm` `copy_menu_actions`, CT-24,
    `docs/bugs/remix_giant_dk_results_menu_motions_2026-11-05.md`). The results sweep `l5_rs_*` (82 Remix winners) is clean.
    - C1 CSS common (owner of `mnplayersvs.c`, `mnplayers1pgame.c`, `mnplayers1pbonus.c`, `mnplayers1ptraining.c`): MN-1,
      MN-2, MN-3, MN-4, MN-5, MN-11, MD-3, MD-22.
    - C2 CSS per mode (after C1, same files, `mnremixvariants.c`, `mnremixvsstamina.c`): MN-6, MN-7, MN-9, MN-10, MN-12,
      OP-29, OP-30, OP-31, OP-32, OP-33, OP-40, OP-41, MD-20, the CSS part of MN-8.
    - C3 results (owner of `mnvsresults.c`, `mnremixemblem.c`, `remix_roster.cpp`, `ftport.c`): MN-13, the results part of
      MN-8, MD-12, MD-13, MD-14, MD-15, MD-16, MD-17, CT-13, CT-1.
    - C4 CSS options and menus (`mnvsmode.c`, `ftremixdpad.c`, `mnremixcsspanelmenu*`): MN-14, MN-15, MN-20.
    - C5 screens (`mnstartup.c`, `mncongra.c`, `mnremixgallery.c`, `mndata.c`, `port/gui/PortMenu.cpp`, `scautodemo.c`,
      `scremixvsdemo.c`, `mnsoundtest.c`, `scstaffroll.c`, `mnremix1pmode.c`): MS-1, MS-2 (Remix splash after the N64
      logo), MS-10, MS-11, MS-12 (Gallery), MS-18, MS-24 ("Other Screens" in the port menu), MS-20, MS-21 (VS demo name
      plates), MS-7, MS-9, MS-19.
    - Tests: `scene.sh` on scenes 16 to 20 and 9, 27, 55, 56, 58, 59, 61 against reference captures;
      `SSB64_PORT_MENU_OPEN`; unit tests `test_css_rules`, `test_tagteam_rules`, `test_teams_rules`, `test_stamina_rules`,
      `test_dpad_control_rules`; results with `SSB64_VS_STOCKS` and Remix winners, the C-held input, the FGM log and
      `wavstat.py`.
  - [ ] **Lot 6, modes** (MD and OP). Progress (2026-11-05): coded by five workers (M1 to M6, M2 and M3 in one) and built; 7087 unit checks;
    done with runs: OP-1, OP-3, OP-5, OP-6, OP-7, OP-10, OP-17, OP-28; OP-42 partial (room doll drop screen not placed);
    OP-8 waits for MS-3; the rest ported, in-game check pending. Test note: the 1P scene shortcut needs `SSB64_SPGAME_FKIND`
    (fkind 28 otherwise), and an All-Star run starts from the 1P CSS (scene 17) with `SSB64_REMIX_1P_MODE=allstar`.
    - M1 1P game (owner of `sc1pgame.c`), three serial batches: (a) OP-4, OP-5, OP-6, OP-7, OP-8, OP-15; (b) OP-11, OP-12,
      OP-13 (trait and level only), IT-1; (c) OP-18, OP-19, OP-24, OP-25, OP-26, OP-45 and the `sc1pgame.c` part of OP-10.
      The `sc1pgame.c` parts of OP-17, OP-34 and OP-23 come after (c).
    - M2 1P overlays (owner of `ifcommon.c`, `sc1premixbonusexit.c`, `sc1premix1p.c`, `sc1pmanager.c`): OP-1, OP-2, OP-3,
      OP-9, MD-18, OP-17, OP-34.
    - M3 bonus stage (`sc1pbonusstage.c`, after M2 for OP-34/OP-35): OP-10, OP-35, OP-38, OP-39.
    - M4 1P intro (`sc1pintro.c`): OP-21, OP-27, OP-42, OP-43, OP-44, and the 1P title cards against the reference (one
      Marina pose for the Marina team, Mad Piano size, Metal Mario Bros texture).
    - M5 All-Star, stock, 12CB: OP-22, OP-28, OP-23 with MD-1, MD-8, MD-4, MD-5, MD-6, MD-7, MD-23.
    - M6 others: OP-14, OP-16, OP-36, OP-46, OP-47, MD-2, MD-19, MD-9, MD-21.
    - Tests: `SSB64_REMIX_1P_MODE`, `SSB64_START_SCENE=52`, the practice, pause and 12CB inputs; unit tests
      `test_1p_mode_rules`, `test_1p_enemy_rules`, `test_1p_bonus_remix_rules`, `test_css_cpu_costume_rules`,
      `test_gameend_rows_rules`, `test_stockmode_rules`, `test_twelvecb_rules`, `test_twelvecb_custom`; `SSB64_BGM_LOG`;
      reference captures of the 1P cards, the Rest Area and the 12CB screens.
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
  1. [x] (2026-11-05) Baseline with `ai_recovery.sh 10 4 14000` (`ai_rec_1` to `ai_rec_10`, level 9, 3 stocks, four CPUs): 712
     recovery episodes, 631 ok (88.6%), 59 KO, 22 open at the frame limit. This is the reference for the Phase 6 acceptance
     (the CP7 count of 181 of 222 used another episode definition). Two of the ten runs aborted on the VS results screen
     (status buffer full after a four-Remix-fighter match), fixed in Phase 5b lot 2. Ledger IDs for the CPU-only scopes
     counted apart: dkshared `giant_punch_fix_1`, `cpu_fix_2`; jigglypuffkirbyshared
     `kirby_cpu_inhale` 1 to 3, `multi_jump_recovery_fix_`, `remix_skip_offensive_usp_air_`; nessshared
     `cpu_usp_usage_fix_`, `cpu_usp_control_fix_`, `cpu_pk_thunder_attack_`, `improved_pk_thunder_`,
     `save_cpu_recovery_location_`; yoshishared `yoshi_cpu_fix_1`; the Zebes acid AI line; linkshared
     `cpu_pull_out_bombs_` checked (marked ported). `SSB64_REMIX_AI_LOG` and a measurement script (10 four-fighter matches,
     level 9, 3 stocks); the baseline re-measured with it before any change (181 of 222 episodes at CP7, measured with a temporary
     log since removed).
  2. [x] coded (2026-11-05): FT-6, FT-8 = CT-19, FT-9, FT-11, FT-12, FT-13, CT-17 (the PREVENT routines now act per
     candidate in `ftComputerCheckDetectTarget`, as the asm does), CT-20, FT-52; unit tests. Rule files, in parallel: FT-6, FT-8 = CT-19, FT-12, FT-13 (decision in its row first), CT-17, CT-20, FT-9, FT-52,
     FT-11 (human handicap up to 40 and the icons above 9).
  3. Re-measure after the second batch (2026-11-05): 811 episodes, 720 ok (88.8%, not below the 88.6% baseline), 70 KO of
     which 60 self-destructs; the two vanilla-only runs (unchanged code) show 12 self-destructs in 15 KOs, so most level-9
     recovery KOs are self-destructs in vanilla too. Second batch coded (2026-11-05): FT-3 with FT-36, FT-4 with FT-7 (FT-10 was already ported), GP-8 = CT-21 (all seven
     Reflect.asm AI patches), MD-10 and MD-11 (ball and hill targets), the self-destruct count in the AI log (`selfko` in
     `ai_recovery.sh`); PUFF_DSP is commented out in the asm (nothing to port); the Default CPU LVL refresh and the `CpuLevel9`
     conflict with level 10 are left (a design decision). Deviations in `deviations.md` 5.9. First batch coded (2026-11-05): SH-16 to SH-27 (SH-20 and SH-21 narrower than the ledger, `ai.md`), ST-27, FT-1 = CT-18
     (all 37 `recovery_logic` entries, `ftremixairecoverylogic*.c`), FT-2 (`_improve_recovery_drift`), FT-5; level-10 matches
     `p6_lv10_a` to `_d` run the new routines without errors. Re-measure: 658 episodes, 568 ok (86.3%) against 631 of 712
     (88.6%): the runs are deterministic, but any AI change alters every fighter's match, and KOs while recovering are mostly
     hits taken off stage (vanilla fighters whose recovery code did not change show the same spread). Next: the AI log counts
     self-destructs (KO with no hit taken during the episode), and that count is the acceptance metric. `ftcomputer.c`,
     serial: FT-1 = CT-18 (`custom_recovery_logic`, whose registry slot nothing fills), FT-2, FT-5, FT-3
     with FT-36, FT-4 with FT-7, FT-10, GP-8 = CT-21 (reflect and absorb), MD-10 and MD-11 (Smashketball and KOTH
     targets), the CPU scopes of step 1, then the 2026-11-03 list: `cpu_post_process` per fighter, PUFF_DSP, the vanilla
     PREVENT rows, Banjo P06, Kirby P14, P11 of Sheik, Peppy, Bowser and Giga Bowser, the 30 `cpu_*` routines,
     `_improve_recovery_drift`, the Default CPU LVL refresh and the `CpuLevel9` conflict with level 10.
  4. [x] Acceptance (2026-11-06). `REMIX-TODO(AI)` at 0 (the last five became notes checked in the asm). Every AI ledger
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
  - [x] ~~Final packages without the `.o2r`~~ dropped (user decision 2026-11-06: no GitHub CI release; the `release.yml`
    matrix entries and the install doc stay as written, unused). Original text: (macOS `.app`/`.dmg`/`.zip`, AppImage x86_64, Switch `.nro`): first confirm that
    first-run extraction works for the Remix (the game asks for the user's Remix ROM and builds `BattleShip-Remix.o2r`), as
    for the US build; `remix` in the `release.yml` matrix (`scripts/package-macos.sh` and `scripts/package-linux.sh` accept
    `SSB64_VERSION=remix`); install doc (which ROM: vanilla plus the Remix 2.0.1 patch, and how extraction works).
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
  cheat with scene 0x38 gives no presented frame in 600 s, so there is no reference capture; the port credits
  (`p8r_credits`) roll without error.
  `parity.md`: 277 `ported`, 2 `adapted`, 10 `n/a`, 0 `partial`.

---

## Progress estimate (update at every checkpoint)

From the patch audit (2026-11-05): share of the Remix patches that are ported, adapted or `n/a` with a checked reason, per
area (patch counts of the ledger groups; an S-effort item weighs as much as an L one, so this undercounts the work done).

| Area | Weight | Done |
|---|---|---|
| Fighters (roster, specials, projectiles, entries, shared patches, per-fighter tables) | 35% | 100% (every GP, SH, FT, CT item `done` or `n/a`) |
| Items and Size | 10% | 100% (IT) |
| Stages (loader, select, hazards, camera, surfaces) | 20% | 100% (ST) |
| Audio | 10% | 100% (MS audio scopes; listening left to T9.1b) |
| Settings, menus, modes | 20% | 100% (MN, MD, OP, MS) |
| Final verification | 5% | ~80% (Phase 8 and T9.3 done; T9.1b and T10.2 by the user) |
| **Weighted total** | | **~99%** (2026-11-06: 95 + 5×0.80 = 99; the 2026-11-05 audit estimate was ~72%) |
