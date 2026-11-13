# Smash Remix in the port: architecture

How the BattleShip-Remix build (`SSB64_VERSION=remix`) runs Smash Remix 2.0.1. Code: `port/remix/` (C++ side: registries, RAM image,
roster loader, overlays), `decomp/src/**/*remix*` and `decomp/src/{ft/ftremix,gr/grremix,it/itremix}/` (game side), `torch/src/factories/ssb64/Remix*`
(extraction), `tools/gen_remix_*.py` (generators), `tests/remix/` (unit tests). The Remix side of each topic is described in
[`fighters.md`](fighters.md), [`stages_and_moveset.md`](stages_and_moveset.md), [`menus.md`](menus.md), [`audio.md`](audio.md),
[`css_reference.md`](css_reference.md) and [`modes_base.md`](modes_base.md); how to port a routine is in [`porting_guide.md`](porting_guide.md).
Tests: `cmake --build build --target run_remix_mode_tests` (unit) and the scripts of `.claude/skills/remix-smoke-tests/` (in-game), see section 12.

## 1. Build, data flow and conventions

### 1.1 Variants and guards

`SSB64_VERSION` is `remix` (default), `us` or `jp`. The remix build defines `PORT`, `REGION_REMIX`, `REGION_US` and `VERSION_US`
(Remix is a US-region game: every `#if REGION_US` branch of the decomp stays active; `REGION_REMIX` selects the file table, the asset
pipeline and the Remix hooks). The `us` and `jp` builds do not compile the Remix-only sources (`CMakeLists.txt` filters
`ft/ftremix`, `gr/grremix`, `it/itremix`, `credits/remix`, every `decomp/src/<dir>/*remix*.c` and `port/remix/`, except `remix_debug.*`
whose env shortcuts also exist in the vanilla builds), so the vanilla builds stay equal to the original game.

| Guard | Where | Meaning |
|---|---|---|
| `#ifdef REGION_REMIX` ... `#else` ... `#endif` | vanilla decomp files | A Remix patch site. The `#else` branch is the original code, so a vanilla build is unchanged. |
| `#ifdef PORT` | files of `ft/ftremix/<fighter>/`, `gr/grremix`, `it/itremix` | The file only exists in the port (those directories are Remix-only, so the guard is a safety net). |
| `#if defined(PORT) && defined(REGION_REMIX)` | headers and logic files outside the Remix-only directories | Same, for files that live next to vanilla ones (`gm/`, `mn/`, `sc/`, `if/`, `lb/`). |
| `*rules.c` / `*rules.h` | pure rule files | Only `int` and `float`, no decomp types, so `tests/remix` compiles them standalone. |

Hooks into a vanilla function are small (`#ifdef REGION_REMIX` plus one call); the logic lives in a file the Remix owns and is declared in
`decomp/src/ft/ftremix/ftremixhooks.h` (fighters) or in the header of the module. Details of the convention are in the porting guide.

### 1.2 Data flow

```
smashremix-src (asm, pinned 5e04fe7)      baserom.remix.z64 (user's ROM)         baserom.us.z64
   |  scripts/remix/assemble-remix.sh          |  torch o2r (yamls/remix/*.yml)
   v                                           v
build/remixasm/remix.sym  ---> tools/gen_remix_actions.py        BattleShip-Remix.o2r:
   |                              |  port/remix/remix_symbols.h     reloc files (5455 ids), remix/fighters,
   |                              |  port/remix/remix_action_layout.h  remix/ram_image, audio/*, particles/*
   |                              |  ft/ftremix/ftremixvanillaprocs.c, ftremixdemoscripts.c
   |                              v  mn/mnmaps/mnremixstagerandomtable.inc
   +--> tools/remix_port_refs.py (build/remixasm/refs: struct_offsets.txt, vanilla_funcs.txt)
runtime: remix_ram_image_load -> remix_roster_load -> remix_toggles_init -> remix_panel_init -> remix_training_init   (port/port.cpp)
```

`remix.sym` is the label-to-RAM-address map of the Remix build; it is produced by assembling the Remix source with a patched bass
(`scripts/remix/assemble-remix.sh`, everything under `.toolchain/` and `build/remixasm/`). No ROM-derived byte is versioned: the
generators emit addresses, names and numbers; ROM data reaches the game only through the `.o2r`.

### 1.3 Generated files

Run the generators as `.toolchain/host/venv/bin/python -I tools/<script>.py`; nothing generated is edited by hand.

| Generator | Output |
|---|---|
| `gen_remix_actions.py` (needs `build/remixasm/remix.sym`, `refs/` and `baserom.remix.z64`) | `port/remix/remix_symbols.h` (`REMIX_SYM_<Scope>__<label>` for every routine the bind tables reference, every label of the CSS fighters' scopes and the `EXTRA_SYMBOLS` / `EXTRA_SYMBOL_PREFIXES` tables: `Stages.*`, `Character.*.table`, `Toggles.*`, `Hazards.*`); `port/remix/remix_action_layout.h` (addresses of the action-array table and the six special-entry tables, action count per id); `ft/ftremix/ftremixvanillaprocs.c` and `ftremixdemoscripts.c` (vanilla address to decomp function / overlay motion script); `mn/mnmaps/mnremixstagerandomtable.inc`; the per-fighter worklists of `build/remixasm/worklists/` |
| `gen_remix_ported.py`, `gen_remix_items.py` | `ft/ftremix/ftremixported.c` (`dFTRemixPortedProcTables`: every `dFTRemix<Name>Procs[]` under `ft/ftremix/*/`), `it/itremix/itremixitems.c` (`dITRemixItemDescs`) |
| `gen_remix_roster.py`, `gen_remix_css.py`, `gen_remix_stage_ids.py` | `port/remix/remix_roster_data.h` (`kRemixRoster[97]` and `REMIX_ID_*` from `Character.asm` / `costumes/*.asm`), `remix_css_data.h` (CSS slots, portrait/name/logo offsets, announcer FGMs), `gr/grremix/grremixids.h` (`nGRRemixStage_*`) |
| `gen_remix_credits.py`, `gen_remix_music_titles.py`, `gen_remix_training_characters.py` | `credits/remix/*`, `remix_music_titles_data.inc`, `remix_training_characters_data.inc` (names and ids only) |
| `gen_remix_parity.py` | `docs/remix/parity.md`, one row per Remix `.asm` with its port status |

## 2. Fighters

### 2.1 fkind equals the Remix id

Every Remix character has an id (`Character.id.X`, 0x00 to 0x60, `REMIX_ID_*` in `remix_roster_data.h`). The port uses it as the fighter kind: vanilla fighters 0x00 to 0x1A keep their `nFTKind*` value, the 68 new
fighters 0x1D to 0x60 are "synth fighters" with `fkind >= 27`, and 0x1B (the "?" model of the Random bookend of the VS character select, never fought) is registered too. 0x1C stays unregistered so that 28
(`nFTKindNull`) never names a fighter. Reason: every Remix table is indexed by the Remix id (`Character.*.table` in the RAM image: costumes, CSS, Kirby copy, AI), so no translation layer sits between the asm data and the C code.

Decomp tables sized for 27 fighters (`dFTManagerDataFiles[]`, special status lists, ...) cannot be indexed by a synth fkind. The single redirection point is `port/fighter_registry.{h,cpp}`: a `FighterDescriptor`
per fkind (`ft_data`, special descriptors and enter handlers, entry statuses, costume row, scale, CSS fields, per-fighter callbacks such as ECB override and shield-hitlag skip, results layout). Vanilla rows are
seeded at boot; Remix rows are registered by `remix_roster_load`. A per-fkind loop bounded by `nFTKindEnumCount` silently skips synths and the failure shows up later as memory corruption; such sites use
`port_fighter_kind_limit()` or the registry (`ftManagerPortSetupSynthFileSizes` in `ft/ftmanager.c` is the model), and tables that replace a 27-row vanilla one are read from the image by Remix id (Kirby copy
in `ftremixkirbycopy.c`, Falcon Dive offsets, costumes). `docs/remix/fighters.md` lists the sites.

### 2.2 Torch blobs

Torch builds the Remix data from the user's ROM into `BattleShip-Remix.o2r`; the port never parses the ROM. Factories (`torch/src/factories/ssb64/`, registered in `torch/src/Companion.cpp`):

| File | Responsibility |
|---|---|
| `RemixRom.{h,cpp}` | `RequireRemixRom` (country `E`, size above the vanilla size) and the RAM-to-ROM windows: vanilla data `RAM - 0x80084800`, expanded code `RAM - 0x7D800000` (code image RAM `0x80400000` = ROM `0x2C00000`), overlay window `0x80390000..` `RAM - 0x80288A20` |
| `RemixFighterTable`, `RemixFightersFactory` | `SSB64:REMIXFIGHTERS`: the FTData struct table (ROM `0x92610`, 97 pointers to 0x78-byte structs) and each fighter's `mainmotion` / `menu` motion arrays |
| `RemixAudioCtl`, `RemixAudioFactory` | `SSB64:REMIXAUDIO` with a `bank` parameter (section 9) |
| `RemixBlobFactory` | `SSB64:REMIXBLOB`: a raw ROM range (`offset`, `size`), never translated as a segmented address; used for `remix/ram_image` and the particle banks |

`yamls/remix/remix.yml` declares `fighters` and `ram_image` (ROM `0x2C00000`, `0x400000` bytes), `audio.yml` the eight audio banks, `particles.yml` the particle banks, and the `reloc_*.yml` files (written by
`tools/generate_yamls.py --version remix`) the relocatable files; `port/resource/RelocFileTable.remix.{h,cpp}` maps file id 0..5454 to the resource path (`tools/generate_reloc_table.py`). Remix `File.asm` ids (0x0854 to
0x154E) are ordinary reloc file ids. Blob integers are little-endian.

`remix/fighters`: a 16-byte header (`'RMXF'` 0x46584D52, version 1, count 97, reserved 0); 97 records of 0x80 bytes in Remix id order (`+0x00 remix_id`, `+0x04 file_ids[9]` in FTData order main, mainmotion, submotion, model,
shieldpose, special1..4, `+0x28 file_main_size`, `+0x2C o_attributes`, `+0x30 file_anim_size`, `+0x34 mainmotion_count`, `+0x38 mainmotion_off`, `+0x3C menu_count`, `+0x40 menu_off`, rest zero); then the `FTMotionDesc` arrays, 12
bytes each (`anim_file_id`, `command_offset`, `flags`). The two offsets are byte offsets from the start of the motion region (right after the records); `menu` is the `submotion` array. `tools/test_remix_blobs.py` compares every
blob with the Python references (`tools/remix_dump_fighters.py`, `tools/remix_dump_audio.py`):

```
cmake -S torch -B build-torch && cmake --build build-torch -j <n>
./build-torch/torch o2r baserom.remix.z64 -s $PWD -d build/remix-test/run1
.toolchain/host/venv/bin/python -I tools/test_remix_blobs.py --o2r build/remix-test/run1/BattleShip.o2r
```

The normal build runs Torch itself (`ExtractAssets` target) and re-extracts when the Torch binary, `config.yml`, a yaml or the ROM changes (`build_and_roms.md`).

### 2.3 Roster loader

`port/remix/remix_roster.{h,cpp}` (called from `port/port.cpp` after the vanilla registry is seeded) reads `remix/fighters` and registers the synths.

1. Validate the blob: size at least 16, magic `RMXF`, version 1, `count == std::size(kRemixRoster)` (97), records fit.
2. For each id 0x1B and 0x1D..0x60: read the record, check `remix_id` against the index and `kRemixRoster`, require a vanilla parent (`parent_remix_id < 27`; a vanilla parent's fkind is its id), read the two motion arrays with bounds checks.
3. `port_fighter_synth_ft_create` (`decomp/src/ft/ftport.c`) clones the parent's `FTData` and overwrites the Remix fields (file ids, sizes, attribute offset, motion arrays); the runtime slots (`p_file_*`, `p_particle`, motion count) are owned
   by the block and freed by `port_fighter_synth_ft_destroy`. The FTData is built on the decomp side because `fttypes.h` needs `ssb_types.h`, which the C++ port cannot include (`CMakeLists.txt` forbids `decomp/include` in `port/`);
   the C++ side passes plain values (`port/remix/remix_synth_ft.h`).
4. The descriptor is a copy of the parent's with `ft_data`, `results_name`, `scale` (`Character.menu_zoom.table`, in the roster data) and the results layout replaced; specials are rebound from the Remix action array (2.5), entry
   statuses from `Character.entry_action.table`, per-fighter callbacks by `ftRemixBindDescriptorHooks`.
5. All or nothing: every synth is built before the first `port_fighter_register`; a failure registers nothing and the vanilla registry stays intact.
6. Afterwards: `patchVanillaAttributeOffsets` (Remix rebuilt the main files of vanilla fkinds 13..25), `patchVanillaMenuZoom` (fkinds 12..26) and `bindKirbyCopyActions` (Kirby's motion array grows through
   `port_fighter_extend_mainmotion` and his action array gains the 238 copy-ability actions).

Fields the Remix data does not supply stay the parent's (particle scripts and textures, FGMs, hit-lag hooks, default costumes, ...); code that must differ per Remix fighter reads the Remix tables (2.7, section 8).

### 2.4 Runtime facts about synth fighters

- Figatree heap: `ftManagerSetupFileSize` / `ftManagerAllocFighter` sized only the 27 vanilla rows, so a synth got 0 bytes and the animation file overwrote the skeleton; `sFTManagerPortFileSizes` (by fkind) applies the same rules.
- Motion scripts: Remix rows for its new fighters store `command_offset` as a raw RAM address into the code image, and goto/subroutine operands inside those scripts are raw too. `ftPortMapMotionOffset` maps the row through
  `remix_ram_image_resolve` (or `ftRemixFindDemoScript` for vanilla overlay scripts); `portRelocSetFallbackResolver` lets `PORT_RESOLVE` map operands that do not decode as tokens.
- Hidden joints: Remix rows for moves it reimplements omit the XRotN/TransN/YRotN flags (0xE0000000) that the parent's status routines expect; `ftPortParentHiddenJointFlags` merges them from the parent's row, and `ftmain.c` clears the
  three joints in the entry statuses of the polygon fighters.

### 2.5 Action binding

Remix gives each new fighter an action array (statuses `0xDC + i`, 0x14 bytes per entry: a word with motion id, attack id and status flags, then the addresses of the update, interrupt, physics and map routines), six special
entry tables (ground and air NSP/USP/DSP) and the Kirby copy tables (`kirby_ground_nsp`, `kirby_air_nsp`). The first `count - add_actions` entries mirror the parent's.

`ft/ftremix/ftremixbind.c` (`ftRemixBindSpecials`, `ftRemixBindKirbyActions`) builds the synth's `FTStatusDesc` table and enter handlers. An address below `0x80400000` is a vanilla routine, looked up in `dFTRemixVanillaProcs`
(generated); anything else is looked up in `dFTRemixPortedProcTables`, the per-fighter `dFTRemix<Name>Procs[]` tables of `{ REMIX_SYM_..., fn }`. An address without a port falls back to the parent's routine for that slot (a generic
wait/drift routine for new actions) and is logged (`ftRemix: id 0x.. action 0x.. slot N: Remix routine 0x.. not ported`); `ftRemix: id 0x.. bound N actions: V vanilla, P ported, F fallback routines` summarises it. Ported
routines live in `decomp/src/ft/ftremix/<fighter>/` (one directory per fighter family, one `FTRemixProc` table per file; clone-family code in `ftremix*shared.c`). `ftRemixPowerData` gives the FTData whose special files a
move reads: Remix addresses a character's files through global pointers, so Kirby wearing a copied power reads the copied fighter's files.

### 2.6 Moveset commands

Remix extends the motion script opcodes with 0xD0..0xFF (`command.asm`) by hooking the three script runners. `ft/ftremix/ftremixcommand.c` implements 0xD0..0xDC with the two Remix dispatch tables (normal and fast-forward:
frame speed, armor, hitbox direction, translation multiplier, y velocity, fast fall, random sfx, kinetics, hitbox FGM, env color, direction switch, goto moveset file, taunt voice); `ftMainUpdateMotionEventsAll`, `...Forward`
and `...ForwardEffect` call `ftRemixCommandCheckRun` before the vanilla decode (without it the runner never advances on an unknown opcode and the watchdog kills the script after 256 iterations). Per-player command state
is reset in `ftRemixCommandResetAttack` and `ftMainSetStatus`; it reaches the engine through `port/remix/remix_command_hooks.cpp` (listeners for the hitbox-slot reset, damage direction and env-color events, registered in
`port/port.cpp`) and small hooks in `ftMainPlayHitSFX`, `ftPhysicsApplyGroundVelTransN`, `ftPhysicsGetAirVelTransN` and `ftParamSetTexturePartID` (Metal Mario texture form).

### 2.7 Patches in vanilla code and subsystems

- `ftremixhooks.h` collects the Remix routines vanilla code calls; each checks the fighter itself. Examples: `ftremixchartables.c` (`Character.grounded_script` and `on_hit` jump tables), `ftremixdeschooks.c` (ECB and
  shield-hitlag callbacks on the right synth), `ftremixleftovers.c`, `ftremix*shared.c`. `ftremixfields.{h,c}` map Remix scratch words by N64 offset onto the LP64 `passive_vars` / `status_vars` unions.
- Entry effects: `ftremixentry.c` + `ftremixentrydata.c` (`Character.entry_script`): the vanilla appear routines read their model from the vanilla fighter's file globals, so a clone's effect is pointed at the entering
  fighter's file slot; polygon variants skip it.
- Subsystems, one module each: Size (`ftremixsize*.c`), CharEnvColor (`ftremixenvcolor.c`, read in `ftdisplaymain.c`), Reflect (`ftremixreflect.c`: custom reflect hitboxes, kind `id << 16 | 3`), Kirby (`ftremixkirbycopy.c`,
  `ftremixkirbyhats.c`, `ftremixkirbypower.c`), damage types (`ftremixdamage.c`, `ftremixdamagestun.c`, `ftremixbury.c`, `ftremixpoison.c`), Surface (`grremixsurface.c`), extra costumes by part image (`ftremixpartimage*.c`,
  `ftremixyoshiegg.c`), Sonic classic (`ft/ftremix/sonic/`), D-pad control and map (`ftremixdpad*.c`), AI (section 8). Each ported gameplay toggle has its own file
  (`ftremix<name>.c`: hitstun, DI, air dodge, tripping, Z-cancel, parry, rage, speed, ...).
- GFX: `lb/lbremixgfx.c` appends the Remix particle scripts and texture blocks (33 new ids from 0x5C, 9 texture blocks) to the common effect bank, copied from the image with the byte-swap rules of the vanilla banks; a new id
  runs the handler of its base effect with its own script (`lbRemixGfxTakeScriptOverride`). `gm/gmremixcolscripts.c` copies the GFXRoutine colour animation streams (ids from 0x56) into `dGMColScriptsDescs`, turning
  goto/subroutine operands into registered pointers.

## 3. Remix RAM image and symbols

Remix boots by copying ROM `0x2C00000` to RAM `0x80400000` (4 MB). That image holds all its code and tables (character tables, costume tables, stage tables, toggles, AI tables, text).
`remix/ram_image` is that range; `port/remix/remix_ram_image.{h,cpp}` keeps it as native `u32` words (each big-endian word of the ROM converted once), the same layout reloc files have after their blanket swap.

| API | Use |
|---|---|
| `remix_ram_image_resolve(addr)` | Host pointer into the image, NULL outside it; also registered as the reloc-token fallback resolver |
| `remix_ram_image_read_word/half/byte(addr)` | Big-endian reads as the N64 would do them (halfword and byte by shifting the native word) |
| `remix_ram_image_make_file(start, size, dst)` | Copies a region into a scene buffer as a small reloc file (internal pointers become tokens) so display lists and textures in it get the runtime fixups |

Rules that follow from the layout:

- A table read as `u32` has its halfwords in logical order (`FTKirbyCopy` is `(copy_id << 16) | hat`); byte streams (particle scripts, texture pixels) must be copied back to big endian first
  (`lbRemixGfxCopyBytes`).
- Vanilla overlay RAM (below `0x80400000`) is not in the image: a vanilla-overlay float such as the one `robot_bee_laser_main_` reads cannot be read there and is a documented deviation.
- Game code names image addresses with `REMIX_SYM_*` from `remix_symbols.h` (5431 symbols); a table the code needs and the generator does not export is added to `EXTRA_SYMBOLS` in `tools/gen_remix_actions.py` and the file
  regenerated. Code that reads a Remix table at run time keeps the data out of the repository and out of the C source: stage tables (`Stages.*`), character tables (`Character.*.table`), costumes, AI behaviours,
  1P stage rows, Training music, item spawn rates, hazard parameters.

## 4. Settings, panel and save

### 4.1 Toggles registry

`port/remix/remix_toggles.{h,cpp}` builds the Remix settings from the image: the five `Menu.entry` lists (`Toggles.head_{remix,gameplay,music,stage,pokemon}_settings`, linked by the word at `+0x1C`) hold
531 toggles; the non-title nodes sorted by address are exactly the order of `Toggles.profile_defaults_{CE,TE,NE,JP}` (the four profiles: Community, Tournament, Netplay, Japanese). A node is
`type, value, min, max, a_function, string_table, copy_address, next, label, extra, title...` (`Menu.asm`). Values live in the CVar `gRemix.Toggle.<index>` (persistent). Overrides for a run:
`SSB64_REMIX_TOGGLES="Title=value;..."` (title of the entry in `Toggles.asm`, case-insensitive) and `SSB64_REMIX_PROFILE=0..3`.

Game code reads a setting with `gmRemixToggle(<label>)` or `gmRemixToggleVS(<label>)` (`gm/gmremixtoggle.h`): `label` is the name in `Toggles.asm` without `entry_`, resolved to
`REMIX_SYM_Toggles__entry_<label>`; the VS variant returns 0 in Bonus and 1P Game (`Toggles.single_player_guard`). `remix_display_settings.cpp` maps Disable Anti-Aliasing and FPS Display to the renderer's MSAA and
the stats window each frame.

Decision: the menu the Remix draws (`Menu.asm`, the settings screen) is not replicated; the port menu has the same entries and values (`PortMenu::AddMenuRemix` in `port/gui/PortMenu.cpp`: a **Remix** menu with one
sidebar entry per group, checkbox / combo box / slider per toggle, profile buttons under the first group). The registries are the single source of truth: native screens that exist in the Remix (the CSS
panel menu, the Training Z menu, the item switch) read and write the same values.

### 4.2 Per-port panel

`port/remix/remix_player_panel.{h,cpp}`: the 20 options of the Remix CSS panel (`css/*.asm`) plus StockCount, for four ports (`RemixPanelOption`), stored as `gRemix.Panel.<Key>.P<n>`, edited in the port menu
(Remix > Player Panel P1..P4) and in the CSS panel menu drawn by `mn/mnplayers/mnremixcsspanelmenu*.c` (golden button, three items per page, `CharacterSelectDebugMenu.asm`). Override for a run:
`SSB64_REMIX_PANEL="Size.2=1;Damage.1=50"`. Match code reads `gmRemixPanel(OPTION, fp->player)` (`gm/gmremixpanel.h`; value indices are the asm's; Bonus and 1P Game only honour the human port). Custom player
tag names are in `remix_panel_tag_name` (`SSB64_REMIX_TAG_NAMES`).

### 4.3 Training and mode options

`port/remix/remix_training.{h,cpp}`: the Training Z-menu entries (`Training.asm` 3899-3913: shield break, OOS, tech, DI, spam practice, percent, per-port type/character/costume/spawn, music, stage background, D-pad)
as `gRemix.Training.<Key>` (override `SSB64_REMIX_TRAINING="ShieldBreak=1;Oos=3"`). The in-match Z menu (`sc/sc1pmode/sc1ptrainingremixmenu*`, 23 entries on 2 pages with the layout of `Training.asm`) and the port menu
(Remix > Training) edit the same registry. `port/remix/remix_modes.{h,cpp}`: options the Remix draws inside its menus for VS modes (Smashketball type, Tag Team queue slots), CVars edited in Remix > Modes and Remix > Tag Team.

### 4.4 Save

`GMCOMMON_FIGHTERS_SAVE_NUM` (`gm/gmdef.h`) is 97 under `REGION_REMIX` (12 otherwise) and sizes `vs_records`, `spgame_records` and the KO/played-against matrices in `lb/lbtypes.h`; bounds checks use `ARRAY_COUNT`. The unlock masks
(`fighter_mask`, 16 bits) stay for the 12 vanilla fighters. `lbBackupInitRemixDefaults` copies default row 0 to rows 12..96. `LBBackup1PRecord` carries the Remix additions (Multi-Man and Cruel KOs, Home-Run Contest distance,
Bonus 3 time, Remix 1P and All-Star records). A save written by another build fails the checksum and restarts from the defaults.

## 5. Stages

Remix keeps every stage attribute in a table indexed by stage id (`Stages.asm`, ids 0x00..0xD7; 0x29 Deku Tree is the first custom one). The port's `gkind` is the Remix stage id (`grremixids.h`).

| Concern | Port |
|---|---|
| Tables | `gr/grremix/grremixstage.{h,c}` read them from the image: `Stages.stage_file_table` (map file id and header offset, used by `mpCollisionInitGroundData` for gkind above 0x28), `function_table`, `class_table`, `background_table`, `default_music_track_table`, `alternate_music_table`, zoom, position, icon, series logo and variant tables |
| Ground dispatch | `grMainSetupMakeGround` -> `grRemixMakeGround`: Break the Targets and Board the Platforms classes go to the bonus ground; otherwise it switches on `Stages.function_table`: a vanilla maker address (`grInishieMakeGround`, `grJungleMakeGround`, ...), `GRREMIX_FUNC_CLONE` (`jr ra`, no hazards, 75 stages) or a Remix hazard routine handed to `grRemixHazardsMakeGround` (`grremixhazards.c`), which asks one `grRemixHazard<Stage>` handler per file (`grremixhazards_<stage>.c`) until one recognises `REMIX_SYM_Hazards__<name>` |
| Hazards | One `grremix<stage>.c` per hazard stage (water, Mad Monster Mansion, Big Boo's Haunt, Corneria, GB Land, Pirate Land, Japes, Casino Night, Onett, Toad's Turnpike, Rainbow Road, RTTF, Metallic Madness, Spawned Fear, Bowser's Stadium bombs, Yoshi's Island clouds, Dream Land wind, Peach bumper); Time Twister and Scuttle Town use stage group 2 (`grremixstagebg.c`: background fade, camera routine) |
| Toggles | `grRemixHazardsAreOn()` / `grRemixMovementIsOn()` read Hazard Mode (VS and Training screens only); `grremixgating.c` and `#ifdef REGION_REMIX` blocks in the vanilla makers (`grjungle.c`, `grinishie.c`, `grzebes.c`, `grcastle.c`, ...) gate the hazards the Remix patches inside vanilla makers (`mk_clone_hazard_removal_`, `congo_jungle_barrel_`) |
| Hazard items | KlapTrap, Robot Bee, Car, Bullet Bill, Scuttle Minion, Cacodemon are ordinary Remix items (section 6); their animation tracks are relative to `grRemixHazardsItemFileTop` (stage header `+0xE0` minus `0x7E8`, the pointer the asm saves at `0x801313F0`) |
| Music and wallpaper | `grRemixGetStageBGM` (`default_music_track_table` override, then Occasional/Rare/Rare2 alternates by chance 65/16/10/9, then the Random Music toggle, `grremixrandommusic.c`); `Stages.bg_type` picks the common wallpaper (`grWallpaperMakeDecideKind`) |
| Stage select | `mn/mnmaps/mnremix*`: 3 x 6 icon grid in 5 pages (`stage_table_normal`, `stage_table_pages`), RANDOM as a table cell, Roulette, tournament layout (`tournament_hazard_mode_table`), series emblem (`mnremixemblem.c`), variants on C-down (`mnremixstagevariant.c`, `Stages.variant_table`), panel text (`mnremixstagepanel.c`); rules in `*rules.c` |
| Bonus stages | Break the Targets and Board the Platforms of Remix fighters: `sc/sc1pmode/sc1pbonusremix.c` (`Character.BTT_TABLE`, `REMIX_BTT_TABLE`, ...) |

Decisions: a stage that the Remix itself leaves inert stays inert. Peach's Castle II uses `function.CLONE` in Stages.asm and nothing references `peach_castle_II_setup_`, so the Banzai Bill factory
(`grremixbanzai.c`) is a faithful port of dead code that never runs; Scuttle Town's minions are switched off by `b _end` in the asm and `GRREMIX_SCUTTLE_MINIONS_ENABLED` mirrors it. The Remix writes a stage's hazards
into vanilla makers through hooks, so those makers are gated, not rewritten.

## 6. Items

Remix adds items from id 0x2D (`Item.asm` `add_item`, 31 items: 0x2D Cloaking Device to 0x4B Turnip; the standard items are 0x2D..0x41, the hazard items 0x3A..0x40, the fighter items 0x42..0x4B). Each fills a row of
extended tables: spawn routine, spawn-GFX flag, pre-pickup (hold), pickup (consume), drop, throw and player collision (touch). The vanilla code indexes its own tables by item kind and `Item.asm` patches those
lookups for ids >= 0x2D; the port does the same through `itRemixGetItemDesc` at the same vanilla sites.

| Piece | Port |
|---|---|
| Descriptor | `ITRemixItemDesc` (`it/itremix/itremix.h`): `kind`, `proc_make`, `is_show_spawn_gfx`, `proc_hold`, `proc_consume`, `proc_dropped`, `proc_thrown`, `proc_touch` |
| One item | `it/itremix/itremix<name>.{c,h}` exporting `dITRemix<Name>ItemDesc`; `tools/gen_remix_items.py` lists it in `itremixitems.c` |
| Dispatch sites | `itManagerMakeItemSetupCommon` / `itManagerMakeItemKind` (spawn: `itRemixMakeItem`), `itMainSetFighterHold`, `itMainSetFighterDrop`, `itMainSetFighterThrow`, `ftCommonLightGetProcDamage` (consume), `ftMainUpdateDamageStatItem` (touch), `ftcommonget.c` |
| Standard items | Not owned by a fighter (`parent_gobj` NULL): file = `gITManagerCommonData` (file 0xFB patched in place by the Remix), `o_attributes` = the item footer offset. Spawn weights and switches: `itremixspawn.c` (`Stages.custom_item_spawn_rate_table`, `itRemixStandardItemIsOn`), Item Containers and Pokemon picker (`itremixcontainers.c`, `itremixpokemonpicker.c`, `itremixpokemonvoices.c`), the VS item switch (`mn/mnvsmode/mnremixitemswitchlegend.c` for its legend) |
| Fighter items | `desc.p_file` comes from the owner's `fp->data->p_file_specialN` set on a copy of the static `ITDesc` before `itManagerMakeItem` |
| Debug | `SSB64_REMIX_FORCE_ITEM=<kind>`, `SSB64_REMIX_FORCE_ITEM_AT_P1=1`, `SSB64_REMIX_ITEM_LOG=1` (`[remix-item] spawn|hold|consume|touch|drop|throw|bullet kind=0x..`) |

Item state lives in `ip->item_vars` (a file-local struct cast with a `_Static_assert` that it fits); per-player effect timers (Super/Poison Mushroom, Lightning, Franklin Badge, Stopwatch) are file-local arrays
indexed by player, cleared on the scene-end path the asm uses.

## 7. Modes, menus and screens

Every Remix screen or mode is vanilla-menu or vanilla-scene code under `REGION_REMIX` plus Remix-only files named `*remix*`; the settings screen (`Menu.asm`) is the one thing not redrawn.

| Area | Modules |
|---|---|
| Mode base | `gm/gmremixmode.*` (`gmRemixModeGet`: `gMNVSModeRemixMode`, set by the VS Remix Modes page `mn/mnvsmode/mnremixvsmode.c`, or `SSB64_REMIX_MODE`), `gmremixmodecss.c` (CSS profile and cycle per mode: `BeforeCSS`, `LeaveCSS`, `StartMatch`), `gmremixtimedstock*.c` ([`modes_base.md`](modes_base.md)) |
| VS modes | Stamina (`gmremixstamina*`, `ft/ftremix/ftremixstamina*.c`, `ifremixstaminahud.c`), Tug of War, King of the Hill, Tag Team (`gmremixtagteam*`, `mnremixtagteamcss*`), Smashketball (ball: `itremixbasketball.c`), 12-Character Battle (`gmremixtwelvecb*`, `mnremixtwelvecb*`), teams incl. yellow (`mnremixteams*`), results rows (`gmremixgameend*`), Match Stats (`mnremixvsstats*`) |
| 1P and bonus | `sc/sc1pmode/sc1premix*` (mode ids, Remix 1P and All-Star data read from the image in `sc1premix1pdata`, Multi-Man and Cruel, Home-Run Contest, Bonus 3, title cards, endings, Rest Area, AFK enemy, high-score gating), `mn/mn1pmode/mnremix1pmode.c` |
| Training | `sc/sc1pmode/sc1ptrainingremix*` (D-pad controls, Z menu, action display, legend, spawn, exit), `sc1ptrainingrules.c`, `ft/ftremix/ftremixtraining.c` (shield break, OOS, spam), `mnremixcpulevel*` (CPU level to 10) |
| Character select | `mn/mnplayers/mnremixcss.*` (3 x 10 grid shared by the VS, 1P, Training and Bonus CSS; `port/remix/remix_css_data.h`), `mnremixcostumes.*` (C buttons: costume and shade), `mnremixvariants.*` (D-pad: special, polygon, J, E), `mnremixcssrandom.c`, `mnremixcssslide*`, `mnremixsonicclassic.c`, `mnremixcssbonus.c`, `mnremixcsspanelmenu*` |
| Data and extras | `mn/mndata/mnremixchardata.c`, `mnremixsoundtest.c`; `mn/mncommon/mnremixgallery.c`, `mnremixtitleversion.c`, `mnremixtimeouts.c`; `credits/remix/`; `mn/mnvsmode/mnremixitemswitchlegend.c`, `mnremixwinbgm.c` |
| Interface | `if/ifremix*` (announcer, combo meter, HUD, player tags, pause legend, sprites, rectangle, Gentlemen's Reset, battle place) |
| Camera and effects | `gm/gmremixcameramode.c`, `gmremixcamerapause.c`, `gmremixspecialzoom*.c`, `gmremixmagnify.c`, `gmremixgroupdisplay*.c` (`gGCCommonLinks` is `Render.GROUP_TABLE`), `ef/efremixblastzone.c`, `efremixscreenshake.c` |

Decisions: costumes beyond `num_costumes` are applied by swapping part images on the fighter's joints before it is drawn (`ftRemixPartImageApplyFighter`) instead of hooking the MObj creation; the Tag Team queue is drawn on the CSS and
the port menu edits the same data; 12CB keeps a game vector of 48 entries instead of the asm's 24, which overflows in a long battle.

## 8. AI

The CPU uses the Remix tables from the image: `ft/ftremix/ftremixai*.c` with pure rules in `ftremixai*rules.c` and small hooks in `ftcomputer.c`. Attack lists (`Character.ai_behaviour.table`, level-10 `lv10_ai_behaviour.table`) are built at
run time (`ftRemixAiGetAttackList`); custom input scripts of ids >= 0x31 come from `AI.command_table` (`ftremixaicommands.c`); then per-fighter prevent-attack vetoes, long range, aerial chase, recovery, size, DI, tech, level 10
post-processing, movement, prediction and Single Button Mode CPU hooks. A Remix level 10 exists beyond the vanilla 1..9 (`mnremixcpulevel*` in the CSS).

## 9. Audio

`yamls/remix/audio.yml` declares eight `SSB64:REMIXAUDIO` banks (the `bank` parameter picks the builder in `RemixAudioFactory.cpp`):

| Bank | Content |
|---|---|
| `B1_sounds1_ctl`, `B1_sounds2_ctl` | The runtime image of a ctl: the vanilla blob at the start, the code image (ROM `0x2C00000..0x3000000`) at `RAM - ctl base`, zeros between. Ctl base RAM `0x800472D0` (music) and `0x8004D9F0` (SFX); every offset inside the ctl is relative to that base, so the Remix extensions (bank map at RAM `0x80411170`, SFX extension at RAM `0x80413200`) land in the code image. |
| `B1_sounds1_tbl`, `B1_sounds2_tbl` | A contiguous ROM slice from the TBL base to the last reachable sample (`FindRemixSampleExtent` walks the ctl); new samples live at ROM `0x3000000+` |
| `S1_music_sbk` | The sequence file from ROM `0x2504750` to the end of the last entry (375 sequences: entry 0 and 47..374 point past `0x3000000`) |
| `fgm_unk`, `fgm_tbl`, `fgm_ucd` | The FGM package and the FGM sound/microcode tables; `fgm_tbl` and `fgm_ucd` are rebuilt as self-contained blobs (big-endian count, offsets relative to the blob, deduplicated records; 1288 and 1545 entries) |

`port/bridge/audio_bridge.cpp` parses the ctl blobs with `BankParser` (bounds checked against the blob size), points wavetables and sequences straight into the blob data (they outlive the audio heap, which is
N64-sized) and wires music CSP players to `B1_sounds1_ctl` and the SFX bank to `B1_sounds2_ctl`. Runtime pieces: stage BGM (section 5), per-fighter victory BGM (`Character.winner_bgm.table`, `mn/mnvsmode/mnremixwinbgm.c`), BGM
state, pause song change, Menu Music, Random Music and volume toggles (`gm/gmremixbgm*.c`, `gmremixmenumusic.c`), Sound Test with 375 songs and 1094 voices via `MIDI.extended_music_map_table` /
`FGM.extended_voice_map_table` (`mnremixsoundtest.c`), Flat Zone footsteps (`ft/ftremix/ftremixsfxreplace.c`), Japanese sounds, the Pokemon announcer (`if/ifremixannouncer.c`, `ifremixhitannouncer.c`; FGM ids from 0x45B) and the music
title banner (`port/remix/remix_music_title.*`, 308 titles generated from `BGM.asm`). FGM playback goes through `func_800269C0_275C0(id)`; `SSB64_AUDIO_FGM_LOG=1` logs every id and `SSB64_AUDIO_DUMP_PATH` writes a WAV (`audio.md`,
`audio_extraction.md`).

## 10. Text, fonts and sprites

Remix draws text with its Kabel font and positions it with `Render.draw_string_`. `port/remix/remix_font*.{c,cpp}` decodes the font from the user's `.o2r` (layout rules of `draw_string_` and `apply_max_width_` in pure C:
`remix_font_layout.c`, tested) and `remix_hud_text.{h,cpp}` draws labels as an overlay window: callers pass the asm arguments (X, Y of the glyph box top, scale, alignment) in N64 screen coordinates and call
`remix_hud_text_put(slot, ...)` every frame (a label lives for the frame it was put in and the next one); the slot map is documented in the header. `remix_input_display.cpp` draws the Input Display controllers.
Remix sprites that vanilla code lacks (button glyphs, extension sprites, stock icons with extra palettes) are real SObjs: `if/ifremixsprite.*` and `ifremixspriterules.c` (offset and colour table, tint = PRIM, palette = ENV).

## 11. Key decisions

| Decision | Reason |
|---|---|
| fkind = Remix id, one registry | Remix tables are indexed by id; the 27-row decomp arrays cannot be indexed by synths (2.1) |
| Remix data read from the RAM image at run time, never copied into the repository | No ROM-derived bytes in versioned files; a table is one `REMIX_SYM_*` away and stays exactly the Remix's |
| Torch builds blobs; the port parses plain values; FTData is built on the decomp side | The port stays independent of ROM layouts (checked byte for byte by `tools/test_remix_blobs.py`); the C++ port cannot include decomp headers |
| Behaviour equals the Remix, no hack that falls back to the parent | A fighter that plays the parent's move is wrong; an unported slot is logged and listed in the audit. A Remix that is inert (Banzai Bill, Scuttle minions) stays inert in the port. Only code that clearly uses the wrong register is ported as intended, with a record in `docs/bugs/` (`remix_tugofwar_asm_deviations_2026-11-05.md`) |
| A deviation is documented where it lives | `DEVIATION(<reason>)` comments citing the asm file and line; bugs with a cause in `docs/bugs/remix_*`; behaviour the Remix leaves undefined (a null pointer on N64, a joint the model lacks) gets a logged guard (`ftcommoncapturepulled.c`, `ftcommonthrown2.c`) |
| Remix's settings framework (`Menu.asm`) is not redrawn | The port menu has the same entries; the registries are shared with the native screens (CSS panel menu, Training Z menu) |
| Registry values are CVars with run-time overrides | Persistent from the port menu and scriptable for tests (`SSB64_REMIX_TOGGLES`, `_PANEL`, `_TRAINING`, `_PROFILE`) |
| Pure rules separated from game code | Rules compile standalone and are unit tested; the game code supplies the state |
| Vanilla builds exclude all Remix sources; hooks keep the original in `#else` | The original game stays equal to the decomp |
| No N64 byte offsets on LP64 structs | Struct layouts differ; scratch words go through `ftremixfields` |
| Kabel text as an overlay | The vanilla game has no such font; the overlay reproduces the asm positions |

## 12. Test harness

**Unit tests** (`tests/remix/`, CMake target `remix_mode_tests`, run with `run_remix_mode_tests`): pure C, no ROM, no window, no libultraship. Each `test_*_rules.c` defines a `remixTest<Name>Rules` suite with the `CHECK_EQ` /
`CHECK_TRUE` / `RUN_TEST` macros of `test_check.h`, registered in `test_main.c` and `tests/remix/CMakeLists.txt` together with the `*rules.c` sources it compiles (modes, 1P, Training, AI, CSS and stage select, font decode and
layout, sprites, D-pad, part images, ...). A rule's test is trusted once mutating the rule makes the suite fail.

```
PATH=$PWD/.toolchain/host/ninja:$PATH .toolchain/host/cmake/CMake.app/Contents/bin/cmake --build $PWD/build --target run_remix_mode_tests
```

**Python checks**: `tools/test_remix_blobs.py` (Torch blobs against the references), `tools/test_remix_css_offsets.py` (every CSS sprite offset points at a Sprite header in the `.o2r`), `tools/test_remix_menu_zoom.py` (menu zoom column
against the ROM), `tools/test_n64_addr_to_symbol.py`.

**In-game smoke tests**: `.claude/skills/remix-smoke-tests/` (`vs.sh`, `special_matrix.sh`, `stage_sweep.sh`, `hazard_sweep.sh`, `item_cpu_sweep.sh`, `stage_select_sweep.sh`, `css.sh`, `parallel.sh`, `sheet.py`, input scripts in
`inputs/`) run the real binary headless with environment overrides (`SSB64_START_SCENE`, `SSB64_VS_FKINDS`, `SSB64_VS_STAGE`, `SSB64_VS_ITEMS`, `SSB64_REMIX_MODE`, `SSB64_REMIX_1P_MODE`, `SSB64_REMIX_TOGGLES`, `SSB64_REMIX_PANEL`,
`SSB64_REMIX_TRAINING`, `SSB64_INPUT_SCRIPT`, `SSB64_MAX_FRAMES`, `SSB64_LOG_PATH`, `SSB64_SAVE_PATH`, ...; the skill lists them all). Every run writes its own log and save, so batches run in parallel. The shortcut helpers are
`remix_debug_env_int` / `remix_debug_env_text` in `port/remix/remix_debug.*`.

**Reference emulator**: `scripts/macos/setup-ref-emu.sh` builds mupen64plus 2.6.0 with an LLE RSP, the angrylion RDP and a scripted input plugin (`scripts/ref-emu/input-script.c`, same line format as `SSB64_INPUT_SCRIPT`) under
`.toolchain/ref-emu`; `scripts/ref-emu/capture.sh baserom.remix.z64 <frames> build/ref-shots/<name> [input.txt]` captures the real Remix for side-by-side comparison (`scripts/ref-emu/inputs/`).

**Patch audit**: `tools/audit_remix_patches.py` lists every `OS.patch_start` of the Remix with the decomp function it lands in and whether the port hooks it (`porting_guide.md`, section 11).

Audit ledger (every item closed): `docs/remix/patch_audit.md` and `docs/remix/ROADMAP.md`.
