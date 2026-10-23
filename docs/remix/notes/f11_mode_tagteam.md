# F11 mode research: Tag Team (TagTeam.asm, VsRemixMenu.mode.TAG_TEAM = 2)

Status: research only. No code, no build, no run. Source read in full: `.toolchain/pkgs/smashremix-src/src/TagTeam.asm` (2851 lines).
Line numbers below refer to that file unless another file is named. "Hook" means an `OS.patch_start(ROM, RAM)` pair. Patched RAM
addresses are given first, then the ROM offset.

Decomp name matching: the asm comments name the vanilla function for most hooks. Each name below is checked against `decomp/src/`.
`no match found` means the name is not defined in the decomp. The decomp has no address map, so no hook address is verified.

## 1. What the mode is

Tag Team is a VS match where each port (player slot) plays a queue of characters, one after another, instead of one fighter. The
queue is built on the VS CSS: each port has up to 6 slots, and the number of slots in use equals the stock count. When a fighter
loses a stock, the next queued character of that port spawns (`load_next_char_`, line 987). Teams (`Global.vs.teams`) add team
costumes, shade rules and stock stealing (`handle_stock_stealing_`, line 1746).

Win condition: `before_css_setup_` (line 377) writes `Global.vs.game_mode` = `0x0003` and the CSS stocks and teams values (set
on the CSS). Two Remix sources disagree on what value 3 means, so the lead must confirm it before porting:
- `Global.asm:136` says `1 = time, 2 = stock, 3 = both`.
- `TimedStock.asm:65` says `3 = stock`, but its code (`score_fix_`, line 64 onward) takes the time branch only for value 1, so 3
  takes the stock score path.
- The Tag Team asm comment at line 382 and 571 says "TIMED STOCK". The decomp enum for `game_mode` was not found by grep.
Stock score path in `score_fix_` (TimedStock.asm): deaths counted against `StockMode.initial_stock_count_table`. Tug of War is special
cased there; Tag Team is not. Timer and stock count come from the CSS values, not from the mode.

Selecting flags:
- `VsRemixMenu.vs_mode_flag` = 2 (`VsRemixMenu.mode.TAG_TEAM`, `VsRemixMenu.asm:15`). Set by the page-2 row
  `VsRemixMenu.asm:94`: `db 0x10, mode.TAG_TEAM, 0x1, 0x0` (screen 0x10 = VS_CSS, vs_mode_flag 2, page_flag 1, 12CB flag 0).
- `Global.vs.game_mode` = 3 (written in `before_css_setup_` and `start_match_setup_`).
- `Global.vs.stocks`, `Global.vs.teams` are CSS values, saved and restored by this file.
- `Global.match_info` is not a flag here. The per-player match block is written directly (`s2` in `before_css_setup_`, stride 0x74).
- Single player flag: not used. Tag Team is VS only.
- Port side today: `decomp/src/mn/mnvsmode/mnremixvsmode.h` keeps `gMNVSModeRemixMode`, with TAG_TEAM = 2 (see
  `docs/remix/notes/f10_menu_vsmodes.md`). Nothing in the port reads it yet.

Title: the title image is offset `0x2738` (written to `0x8013B7C8`, lines 399-402, both FFA and teams). The menu text is at
`VsRemixMenu.asm:94` (`dh 0x4305, 0x4292; dw 0x00007D38`). The internal name in code is "squad strike" (`VsRemixMenu.asm:697-708`,
`TwelveCharBattle.asm:1976`). The displayed name is not confirmed by the asm.

## 2. Flow

Menu entry to results. The mode dispatch uses three tables in `VsRemixMenu.asm`:
- `mode_setup_table` (before CSS): entry 2 is `TagTeam.before_css_setup_` (`VsRemixMenu.asm:537`).
- `mode_setup_leave` table (leave CSS back to VS menu): entry 2 is `TagTeam.leave_css_setup_` (`VsRemixMenu.asm:619`).
- `mode_setup_start_table` (start match): entry 2 is `TagTeam.start_match_setup_` (`VsRemixMenu.asm:676`).
- Start gate: `VsRemixMenu.asm:697-709` calls `TagTeam.css.prevent_start_with_empty_slots_` when vs_mode_flag is 2.

| Step | Vanilla screen | Hook or call | What it does |
|---|---|---|---|
| Menu entry | VS Game Mode menu, screen 0x09, page 2 | `VsRemixMenu.asm:94` row, port `mnremixvsmode.c` | Sets vs_mode_flag = 2 and goes to VS CSS. |
| Enter CSS | VS CSS, screen 0x10 | `VsRemixMenu.asm:537` -> `before_css_setup_` (`TagTeam.asm:377`) | Sets game mode 3, stocks, teams, title offset, stock tables. Copies character queues into the match block. |
| CSS setup | VS CSS | `Render.asm:2343` calls `TagTeam.css.setup_` after `CharacterSelect.setup_` (`Render.asm:2340-2343`) | Creates 6 stock icon objects per port above the panels (file 0x0019). `CharacterSelect.setup_` name: `no match found` in decomp. |
| CSS token place / pick / costume / toggle | VS CSS | `css.update_queue_` hooks (lines 1817-2029, list in section 3) | Writes the queue slot for the port (`update_slot_`), refreshes stock icon cache, advances current slot. |
| CSS B and recall | VS CSS | `css.clear_slot_and_go_to_previous_slot_` (`TagTeam.asm:2399`), `check_recall_with_a_` (`:2480`) | Clears or recalls a slot, moves the current slot. |
| CSS stock count change | VS CSS | `adjust_current_slot_on_stock_count_change_` hook (`:2511`) | Clamps current slots and clears slots above the new count. |
| CSS START | VS CSS | `VsRemixMenu.asm:697-709` -> `prevent_start_with_empty_slots_` (`:2583`) | Blocks start if a non-NA port's queue is not filled (see risk 8.2). |
| Start | VS CSS -> start | `VsRemixMenu.asm:676` -> `start_match_setup_` (`:556`) | Resolves Random slots, team costumes and shades, copies stocks and tables. |
| Stage select | screen 0x15 | none in `TagTeam.asm` | Vanilla. The mode has no stage-select hook. |
| Match load | match start (`scBattle_StartStockBattle`) | hooks `0x8018D40C`, `0x8018D424`, `0x8018D4F4` (ROM `0x10A2FC`, `0x10A314`, `0x10A3E4`) -> `use_character_queues_on_load_` (`:693`) | Copies slot 0 of each port into the match block, resets stolen state, sets up 4 custom heaps, preloads files, loads first fighter per port. |
| Heap size | match load | hook `0x8018D4C4` (ROM `0x10A3B4`) -> `use_largest_anim_heap_` (`:1588`) | Anim heap sized from the largest animation, not the character's. |
| Particle banks | match load | hooks `0x80115A6C`, `0x80115A98` (ROM `0x9126C`, `0x91298`) -> `load_particle_bank_stuff_in_normal_heap` (`:1547`) | Temporarily clears the alt heap pointer so particle banks go to the normal heap. |
| Stock lost | match | hook `0x8013CF60` (ROM `0xB79A0`) -> `load_next_char_` (`:987`) | Picks the next queue slot and spawns the next fighter. |
| Stock icons on HUD | match | hooks `0x8010F8B0`, `0x8010F990` (ROM `0x8B0B0`, `0x8B190`) -> `show_all_icons_` (`:1617`) | Sets HUD stock icon texture and palette per queue slot. |
| Stock stolen (teams) | match | hook `0x801103C0` (ROM `0x8BBC0`) -> `handle_stock_stealing_` (`:1746`) | Thief port gets the victim's icon. |
| Token pickup | CSS | hooks `0x8013763C` (ROM `0x1358BC`, `move_to_next_slot_on_pickup`, `:2364`), `0x80137208` (ROM `0x135488`, `speed_up_token_pickup_`, `:2712`) | Advances the slot on pickup (not when recalling). Pickup wait goes from 0x1E frames to 2. |
| Results | results | none in `TagTeam.asm` | Vanilla. No hook in this file. |
| Leave CSS (B) | VS CSS -> VS menu | `VsRemixMenu.asm:619` -> `leave_css_setup_` (`:492`) | Saves the CSS state into this file, restores VS menu globals. |

Hook name matches (vanilla decomp):
- `ftCommonRebirthDownSetStatus`: name match, `decomp/src/ft/ftcommon/ftcommonrebirth.c` (declared in `ftcommonfunctions.h`).
- `ftParamGetCostumeTeamID`: name match, `decomp/src/mn/mnplayers/mnplayersvs.c` and `mnvsmode.c` (used by `start_match_setup_`, line 633).
- `ftManagerSetupFilesAllKind`, `ftManagerSetupFilesPlayablesAll`, `ftManagerMakeFighter`, `ftManagerDestroyFighter`,
  `ftParamUnlockPlayerControl`: name matches, `decomp/src/sc/` (sc1pmode files).
- `mnPlayersVSRecallPuck` (`mnplayersvs.c:3794`) is a candidate for the recall hook at `0x80137FF8`, but the asm name is
  `mnRecallToken`, which has no match. Not verified.
- `no match found`: `mnRecallToken`, `mnBattleSyncFighterDisplay`, `mnIsHumanWithCharacterSelected`, `ftParamInitModelTexturePartsAll`,
  `ftManagerAllocAnimHeapKind`, `scBattle_StartStockBattle`, `mnBattleSaveMatchInfo`, `CharacterSelect.setup_`.
- The hook `0x8013CF60` is a name match (ftCommonRebirthDownSetStatus). Other hook addresses have no verified decomp name.

## 3. State and data

Globals (all in `TagTeam` scope unless noted). "Writer" means the routines that write the value.

| Name | Size, line | Meaning | Writers |
|---|---|---|---|
| `stocks` | dw, `:13` | CSS stock count, saved | `before_css` (`:388-391`, writes the CSS and Global copies), `leave_css` (`:499`), `start_match` (`:563`) |
| `teams` | dw, `:18` | CSS teams flag, saved | `before_css` (`:394-397`), `leave_css` (`:504`), `start_match` (`:568`) |
| `stockmode_table` | dw x4, `:23` | `StockMode.stockmode_table` copy (per port) | `before_css` (`:404-413`), `leave_css` (`:509`), `start_match` (`:577`) |
| `stock_count_table` | db x4 (read as word), `:28` | `StockMode.stock_count_table` copy | `leave_css`, `start_match` |
| `previous_stock_count_table` | db x4, `:33` | `StockMode.previous_stock_count_table` copy | `before_css` only if coming from VS menu (`:419-427`) |
| `character_queues` | 4 ports x 6 slots x 4 bytes, `:39-68` (96 bytes) | per slot: char_id, costume_id, shade, char_id-if-random or is_classic | `update_slot_` (`:2036`), `start_match` (random resolution) |
| `current_slot` | db x4, `:72` | per port: next slot to fill, 0..stocks | `move_to_next_slot_`, `clear_slot_*`, stock change hook |
| `icon_cache` | 4 ports x 6 slots x `ICON_CACHE_SLOT_SIZE`, `:91-93` | texture, palette, image chunk, 8 palettes per slot | `update_slot_` (`:2073`), `update_previous_icons_` (`:2175`) |
| `stolen_count` | db x4, `:97` | count of stolen stocks per port | `handle_stock_stealing_` (`:1765`), reset at match load (`:756`) |
| `stolen_stocks` | dw x4, `:102` | pointer to victim queue slot | `handle_stock_stealing_` (`:1797`), reset at match load |
| `stolen_icons` | dw x2 x4, `:107-112` | image chunk and palette of stolen icon | `handle_stock_stealing_` (`:1805-1807`), reset at match load |
| `preload_map` | 4 bytes x `Character.NUM_CHARACTERS` pointers, `:118-160` | per char, 0-terminated list of file ids | built by macros from 187 `add_preload` calls (`:163-371`) |
| `file_0019_pointer` | dw, `:2846` | pointer to stock icon file 0x0019 (1P CSS) | `setup_` (`:2741`) |
| `load_all_files_.always_load` | dw, `:1541` | flag, TRUE while loading a heap slot | `load_next_char_`, `load_all_files_` hook |

Sizes from the asm constants (`:82-89`):
- `ICON_TEXTURE_SIZE` 0x80, `ICON_PALETTE_SIZE` 0x28, `ICON_IMAGE_CHUNK_SIZE` 0x10, 8 default palettes (0x140).
- `ICON_CACHE_SLOT_SIZE` = 0x1F8 by the constant expression. The comment at `:81` says 0x1C8 and "5 palettes". The comment is stale.
- `ICON_CACHE_PORT_SIZE` = 6 slots = 0xBD0. Total icon cache = 0x2F40.
- Queue per port = 6 x 4 = 0x18 bytes. The queue address for port `p` is `character_queues + p*0x18`.

Hard-coded RAM addresses (no decomp name verified for any of them):
- `0x8013BDAC` game_mode, `0x8013BD80` CSS stocks, `0x8013BDA8` CSS teams, `0x8013B7C8` title offsets, `0x8013BDCC` frames elapsed.
- `0x800A4D0B` (Global.vs.game_mode, `Global.asm:137`), `0x800A4D0A` teams, `0x800A4D0F` stocks.
- `0x800465E8` main heap struct, `0x800D62E0` file manager, `0x80046700` item list head, `0x80046838` projectile list head.
- `0x801317CC` icons-per-port redraw flag, `0x80131598` damage interface array, `0x80116DD0` default fighter desc, `0x80130D9C` largest anim size.
- `0x80137F9C`, `0x80137FF8`, `0x80136128` (`mnIsHumanWithCharacterSelected`, recall, sync; the calls, not hooks).

Heaps and files:
- Custom heaps: `CharacterSelect.dynamic_css.heap_struct_0..3`, each reset to size 0x4C000 starting at `custom_heap_address` (`:819`).
- Heap slots: `CharacterSelect.dynamic_css.heap_slot_0..3`, one per port (`:838-855`). Each slot: char_id (4 bytes), 4 additional_char_ids, total files count.
- `CharacterSelect.dynamic_css.alt_heap_pointer` selects the heap used for the next file load.
- `CharacterSelect.dynamic_css.slot_used_by_port` (2 words), cleared at load.
- File table: `file_table` (8 bytes per loaded file), `0x800D62E0 + 0x18` = total files loaded.
- `Character.variants_with_same_model.table` (`:1444`): up to 4 variants per char loaded into the same slot.
- Custom Sonic: `Sonic.classic_table` (per port is_classic byte).

Files. The mode loads no strings. Named file ids used directly:
- `0x0019`: stock icon image file for the 1P CSS (`setup_`, `:2741`). Vanilla-used file.
- `0x0E4` etc.: the preload table holds 187 entries. They are plain constants in `add_preload`. Remix-only ids were not checked one by one.
- Names that look Remix-only (verify in the Remix file list): `File.KIRBY_CUSTOM_HATS`, `File.KIRBY_CUSTOM_HATS_2`, `File.REMIX_1P_KIRBY`.
- Sound: `FGM.announcer.css.TAG_TEAM` (783, `FGM.asm:2016`), `sounds/Tag_Team.aifc` (`FGM.asm:1077`). The 12CB code also plays id 783 for Tag Team (`TwelveCharBattle.asm:1765-1767`).
- Title image offset: `0x2738`.
- Menu text sprite offset `0x7D38` (`VsRemixMenu.asm:94`).

## 4. In-match logic (order of events)

A. CSS enter (`before_css_setup_`, `:377`). Sets game mode 3, stocks, teams from the saved TagTeam values, title offset, and copies
the stock tables. Clears `CharacterSelect.random_char_flag_vs`. For each port: clamps `current_slot` to the stock count
(`get_stock_count_`); if the player type is NA, sets char_id NONE; if the slot char_id is NONE and the player is not HMN, sets NA;
if RANDOM, costume and shade are default; otherwise copies costume and shade from the slot.

B. CSS placement and costume changes (`css.update_queue_`, `:1817`). Runs on token select (`_select_with_token`, `:1880`), costume
change (`:1898`), fighter sync (`:1912`), and the four toggle paths (FFA<->teams, team change, `:1927-2026`). Each calls
`update_slot_` (`:2036`), which:
- writes char_id, costume_id, shade into `character_queues[port][current_slot]`;
- sets is_classic for Sonic;
- copies the fighter's stock icon texture, its palette, and the default palette for each costume (6 entries) plus the yellow
  team costume into `icon_cache[port][current_slot]`.
`update_previous_icons_` (`:2175`) then updates earlier slots: in FFA, costume ids shift away from duplicate chars of earlier
ports; in teams, shades increment for teammates with the same char.

C. Token pickup (`move_to_next_slot_on_pickup`, `:2364`). If not recalling, `move_to_next_slot_` (`:2336`) runs:
- `get_stock_count_` (`:2302`) returns the stock count for the port. It is the CSS stocks unless StockMode is DEFAULT (0) or LAST
  (1), in which case the manual count is capped by `stock_count_table`.
- If `current_slot` is below the count, it increments. If above, it clamps to the count.
- Pickup wait is 2 frames (`speed_up_token_pickup_`, `:2712`).

D. B while holding a token (`clear_slot_and_go_to_previous_slot_`, `:2399`). If the held panel is HMN or CPU and the current slot
is not 0: sets current_slot to `current - 1`, clears the queue slot at the old current index, calls `update_slot_`, and plays
`FGM.menu.CONFIRM`. The exact meaning of "old current" versus "the slot just placed" was not traced to runtime (see risk 8.4).

E. Recall with A over a portrait (`check_recall_with_a_`, `:2480`). Only if: the player is HMN with a character, the cursor state is
"not holding" (2), and the token was not just placed (`min_frames_elapsed_until_recall`, frames from `0x8013BDCC`). Then
`move_to_next_slot_`, then the vanilla recall call.

F. Stock count change (`adjust_current_slot_on_stock_count_change_`, `:2511`). For each port, if `current_slot >= new count`: sets
current slot to the count and clears queue slots above it (`0x1C000000` = empty). If the slot went down, calls the sync routine
(`0x80136128`, no name match).

G. START gate (`prevent_start_with_empty_slots_`, `:2583`). For each port that is not NA: reads the stock count, then checks the
queue slot at index = count. If that slot's char_id is NONE, start is refused. The comment says "make sure none are NONE, up to
stock count", but the code checks only one slot (risk 8.2).

H. Start (`start_match_setup_`, `:556`). Copies stocks, teams, game mode 3 and the tables. Loops over all 24 queue slots:
- If char_id is RANDOM (`Character.id.PLACEHOLDER`): picks a random char (`CharacterSelect.get_random_char_id_`) and a random legal
  costume (`Costumes.get_random_legal_costume_`). In teams, costume becomes the team costume (`ftParamGetCostumeTeamID`, team index
  from the CSS struct at +0x40), and shade is raised for each earlier port of the same team with the same char (`:648-667`).

I. Match load (`use_character_queues_on_load_`, `:693`). Runs per port (loop over `s0`, port id):
- Queue index for slot 0: `StockMode.previous_stock_count_table` and LAST mode (`:728-741`). If LAST, the queue start skips the
  stocks already used before the mode started.
- Copies char_id, costume_id, shade into the match block (`v1 + 0x23`, `+0x26`, `+0x27`).
- Clears `stolen_count`, `stolen_stocks`, `stolen_icons` (`:754-774`).
- If Sonic: `Sonic.classic_table[port]` set from the slot (`:776-786`).
- For port 0 only: sets up 4 custom heap structs of 0x4C000 each, clears 4 heap slots (`:803-855`).
- Preloads files from `preload_map` for each queue char (`:859-892`).
- If the port is not NA: loads the first char into its heap slot with `alt_heap_pointer` set, plus its variants
  (`load_variants_in_same_slot_`, `:1437`), and records the total files loaded (`:894-921`).
- After all files load, resets the alt heap (`_reset_alt_heap`, `:937`; safeguard `:974`).

J. Stock lost (`load_next_char_`, `:987`). Triggered by `ftCommonRebirthDownSetStatus`. Only when vs_mode_flag is 2.
- Queue index = stocks setting (`Global.vs.stocks`) minus stocks remaining (player struct `+0x14`), minus `stolen_count[port]`, plus the
  LAST skip (`global stocks - initial_stock_count`). If `stolen_stocks[port]` is set, that pointer is used instead, and the index
  arithmetic is skipped (`:1025`).
- If char_id is RANDOM, uses the queue's randomized char (`+0x03`). For Sonic, updates is_classic.
- If the char or Sonic's classic flag changed: copy `dFTManagerDefaultFighterDesc` (`0x80116DD0`) into a spawn struct, fill team,
  port, detail, costume, shade, handicap, cp_level, stock count, skip-shadow and skip-entry flags (0xC0), damage 0, pl_kind,
  controller, anim heap. Destroy the old fighter (`0x800D78E8`).
- Heap: `get_heap_slot_for_char_id_`. If not loaded, find a free heap (not used by any port's char or additional char ids),
  `reset_heap_slot_`, fix the file table (move entries), then load files (`0x800D786C`), variants, and playables (`0x800D782C`).
- Spawn: `ftManagerMakeFighter` (`0x800D7F3C`), `ftParamUnlockPlayerControl` (`0x800E7F68`). Sets damage series icon from the new
  char's stock icon footer, and repositions it.
- Match block updated (char, costume, shade). Items and projectiles owned by the old fighter are re-pointed to the new one
  (item list `0x80046700`: owner `+0x08`, Blue Shell `+0x180`, Flashbang/Waddle/Gordo `+0x1C4`; projectile list `0x80046838`).
- If the char is the same: only costume and shade are updated (`ftParamInitModelTexturePartsAll` call, `0x800E9248`).

K. HUD stock icons (`show_all_icons_`, `:1617`). Index = `remaining - slot + falls` (`:1629-1662`), with the LAST skip added. The
icon whose index equals `falls` uses the loaded fighter's icon if the queue slot matches the fighter (char, costume, Sonic classic),
or if a stolen icon exists (`:1664-1716`). Other icons use the port's icon cache.

L. Stock stolen (teams, `handle_stock_stealing_`, `:1746`). The victim port gets `stolen_count + 1`. The thief port's icon object
points at the victim's icon cache slot (`current_slot - stolen_count`), and `stolen_stocks[thief]` points at the victim's queue slot.
`stolen_icons[thief]` stores the copied image chunk and palette. The trigger is in the vanilla routine at ROM `0x8BBC0`, whose
name is not in this file (risk 8.3).

M. CSS slot icons (`setup_`, `:2732`; `slot_icon_update_routine_`, `:2639`). Creates 6 icon objects per port (one draw call plus
5 `TEXTURE_INIT_` calls) and links them. The update routine hides icons at or above the stock count, hides all icons for N/A ports,
uses the cache palette and texture for filled slots, and the default texture otherwise. Master Hand (`Character.id.BOSS`) gets a
smaller scale (`:2678-2680`).

N. Other per-frame effects: none. This file has no per-frame update of its own, only the event hooks above.

## 5. HUD and text

- No string literals in `TagTeam.asm`. The only label is the title image at offset `0x2738`, drawn by the vanilla sprite path.
- CSS: 6 stock icons per port above each panel. Image footer from file 0x0019 (`setup_`, `:2741-2800`). Visibility from the stock
  count and NA status (`slot_icon_update_routine_`). These are sprites; the port needs no text drawing for them.
- Match HUD: stock icons per port (`show_all_icons_`), and the damage series icon swap (`load_next_char_`, `:1298-1343`). Sprites.
- Menu: "Remix Modes" page 2 button text is a sprite (`VsRemixMenu.asm:94`). The f10 port already uses the vanilla sprite path for
  this. Nothing in Tag Team needs `remix_overlay.h` (ImGui text).
- Team colours: palettes from `Teams.new_costume_table` (yellow) and `ftParamGetCostumeTeamID` (red, blue, green, yellow).

## 6. Dependencies

- StockMode (`StockMode.asm`): `stockmode_table`, `stock_count_table`, `previous_stock_count_table`, `initial_stock_count_table`,
  `mode.LAST` (1), `mode.DEFAULT` (0). Tag Team reads and writes all of them, and `get_stock_count_` depends on them.
  `StockMode.asm:93` also reads vs_mode_flag.
- TimedStock (`TimedStock.asm` `score_fix_`): stock score path. Tag Team is not special cased there.
- Stamina: not referenced by `TagTeam.asm`. The `game_mode` comment lists 5 = stamina, but Tag Team does not write 5.
- Neutral Spawns: not referenced by `TagTeam.asm`. The decomp's `gmremixspawnsetup.c:201` says Smashketball follows the Neutral
  Spawns toggle. Tag Team has no such rule in the asm.
- Results screen: `gmremixgameend.h` is the decomp for the Remix VS end. No Tag Team hook there.
- Shared with 12CB (`TwelveCharBattle.asm`): 12CB has its own `TAG_TEAM` branches at lines 1764-1767 (announcer, plays id 783),
  1810-1813, 1874-1913 (CSS), 1974-1975, 2029-2030 (`_squad_strike` wrap logic at 1976 and 2031). Any change to shared code
  must be checked against those branches.
- Shared with Render: `Render.asm:2285` (Smashketball check), `Render.asm:2343` (calls `TagTeam.css.setup_`), `Render.asm:2455`
  (clears vs_mode_flag on mode select).
- CharacterSelect: `get_random_char_id_`, `get_heap_slot_for_char_id_`, `reset_heap_slot_`, `dynamic_css` heap structs, and CSS
  player structs (`CSS_PLAYER_STRUCT`, 0xBC bytes each). `CharacterSelect.asm:6229-6235` and `6614` call into this file.
- Teams and costumes: `Teams.new_costume_table`, `Character.default_costume.table`, `Costumes.get_random_legal_costume_`,
  `ftParamGetCostumeTeamID`.
- Items and projectiles: lists at `0x80046700` and `0x80046838` (re-owning on fighter swap).
- Sonic: `Sonic.classic_table`.
- VsRemixMenu: mode dispatch tables and the saved globals (`global_stocks`, `global_teams`, `global_stockmode_table`,
  `global_stock_count_table`, `global_previous_stock_count_table`, `global_stage_select`, `global_stage`).
- Other modes using vs_mode_flag but not this file: KingOfTheHill.asm, ComboMeter.asm, HRC.asm, TwelveCharBattle.asm, Render.asm.

## 7. Port plan

Each step is for one Haiku worker in one session, unless marked as lead work. The port has no Tag Team logic yet, so the only
existing piece is `gMNVSModeRemixMode = 2` in `decomp/src/mn/mnvsmode/mnremixvsmode.c`. Steps 1 and 2 are pure logic and need no
game. Steps 3 onward need the lead to run the game and check the screen named in each step.

Step 0 (lead, before any worker): confirm the `game_mode` value 3 in the decomp (see section 1). Confirm the stock-count
off-by-one in `prevent_start` (risk 8.2). Decide whether the N64 custom heaps (step 14) are needed in the PC port.

1. Queue index math (pure C, no game). Create `decomp/src/gm/gmremixtagteamqueue.c` and `.h`. Functions: queue index for a fighter
   loss (stocks setting, remaining, stolen count, LAST skip), and queue index for the first slot. Test: a host-side unit test if the
   decomp has a test harness; otherwise a small check that compiles on the host. Lead sees nothing on screen; accept on test output.
2. Queue state (pure C, no game). Same module or `gmremixtagteamslots.c`. Functions: update slot, move to next slot (clamped to the
   stock count), clamp on stock change, start check. Depends on 1 only for the stock count helper. Lead sees nothing on screen;
   accept on test output and the off-by-one decision (step 0).
3. CSS state save and restore. Create `decomp/src/mn/mnvsmode/mnremixtagteamcss.c` and `.h`: `before_css` (game mode 3, stocks,
   teams, title, stock tables), `leave_css`, `start_match` table copy. Hook into `mnvsmode.c` where the port already calls the mode
   helpers. Depends on step 0 (game mode). Shared dispatch with 12CB: if the 12CB step lands first, reuse its table entry shape.
   Lead sees: entering VS CSS in Tag Team shows the chosen stocks and teams; backing out to the VS menu restores the menu values.
4. CSS queue writes. In `decomp/src/mn/mnplayers/mnplayersvs.c`, on token place, token pick, B clear, recall and costume change, call
   the step 2 helpers. Depends on 2 and 3. Lead sees: after placing N characters, the Nth slot fills, and B clears the last slot
   (exact behaviour is unverified, risk 8.4).
5. CSS stock icons. Copy stock icon texture, palette and default palettes into the per-port icon cache, and create 6 icon objects
   per panel above the portrait. Depends on 4. Lead sees: 6 icons per panel, count equals the stock count, team palettes in Teams.
6. CSS START gate. Refuse START while a non-NA port has an empty required slot. Depends on 2 and the step 0 decision. Lead sees:
   START refused with an empty slot, allowed when every slot is filled.
7. Start resolution. Resolve Random slots to a real char and legal costume at START, and team costumes and shades. Depends on 4.
   Lead sees: a Random slot becomes a real character at match start, Teams show team colours.
8. Match load. Copy each port's slot 0 into the match player block and reset stolen state. Depends on 2 and 7. Lead sees: the match
   starts with the queued first characters (identify the first fighter per port on the HUD).
9. Next queued character (pure). Choose the next slot after a stock loss, including the stolen pointer and the LAST skip. Depends
   on 1 and 2. Lead sees nothing on screen; accept on test output.
10. Fighter swap. On stock loss, destroy the old fighter, spawn the next queued char with the spawn struct, re-point items and
    projectiles, update the damage series icon. Depends on 9. Needs the port's fighter spawn API, which the lead must check first.
    Lead sees: after losing a stock, the next queued character spawns in that port, and items owned by the old fighter stay usable.
11. Stock stealing in teams. Depends on 9 and 10 and on the vanilla stock-tally routine the port uses (risk 8.3). Lead sees: the
    thief port's HUD icon changes to the victim's icon.
12. HUD stock icons. Depends on 2 and 9. Lead sees: HUD icon count per port equals remaining stocks, icons in queue order.
13. Token pickup timing. Set the pickup wait to 2 frames in Tag Team only. No dependency. Lead sees: picking up a token takes 2
    frames instead of 30 in Tag Team, and no change in other modes.
14. Heaps and file preload. Lead decides first (step 0). If the PC port keeps a per-port loader, map `custom_heap`, `preload_map`
    and the variants table onto it. Depends on 8 and on the 12CB heap code (shared `dynamic_css`). Not a Haiku step.
15. Shared code with 12CB. Any change to `TAG_TEAM` branches shared with 12CB (announcer id 783, `_squad_strike` wrap) must land
    with the 12CB step, not before it. Depends on the 12CB step. Lead sees: 12CB still works after the Tag Team steps.

## 8. Risks and unknowns

8.1 `game_mode` 3 is ambiguous. `Global.asm:136` says "both", `TimedStock.asm:65` says "stock", and the asm comments say "TIMED
    STOCK". The decomp enum was not found. Until the lead confirms the value, the win condition (time plus stock or stock only) is
    not settled.

8.2 Start gate off by one (reading of the code, not run). `current_slot` ends at the stock count after the last placement
    (`move_to_next_slot_`, `:2336-2360`, clamps at the count). `prevent_start_with_empty_slots_` checks queue index equal to the
    stock count (`:2614-2619`), which is the slot after the last one used. If that slot is always empty, START would always be
    refused. If the stock count is 6, the index reads the next port's first slot (port 0 to 2) or past the array (port 3). Smoke test:
    3 stocks, place 3 characters, press START. The comment at `:2588` says "up to stock count", which suggests the intent was
    to check slots 0 to N-1.

8.3 Stock stealing trigger. `handle_stock_stealing_` is a hook into a vanilla routine at ROM `0x8BBC0` (`0x801103C0`). The caller
    and the condition that decides a steal are not in this file. The steal is only inferred from the code.

8.4 B clears a slot. `clear_slot_and_go_to_previous_slot_` clears the queue slot at the old current index and moves current back by
    one. Whether that is the slot the player just placed depends on when `move_to_next_slot_on_pickup` runs. Not traced to runtime.

8.5 Stale comments. `:81` says the slot is 0x1C8 with 5 palettes; the constant expression gives 0x1F8 with 8 palettes. Trust the
    constants, not the comment. `:2588` says "up to stock count" but the code checks one slot (see 8.2).

8.6 Address to name matching. Most hook comments name functions that are not in the decomp (`mnRecallToken`,
    `mnBattleSyncFighterDisplay`, `mnIsHumanWithCharacterSelected`, `ftManagerAllocAnimHeapKind`, `ftParamInitModelTexturePartsAll`,
    `scBattle_StartStockBattle`, `mnBattleSaveMatchInfo`). These are marked `no match found`. The name matches listed in section 2 are
    real functions, but no ROM address was checked against them.

8.7 Heap model. `custom_heap`, `file_table`, `0x800465E8` and `0x800D62E0` belong to the N64 memory model. There is no port
    equivalent identified. The lead must decide whether the port keeps the heap logic (step 14).

8.8 Remix-only files. The preload list has 187 entries of plain numbers. Which are Remix-only, and which the port already loads,
    was not checked one by one. Only names with `REMIX` or `CUSTOM` in the asm were flagged.

8.9 Name shown to players. The code uses "squad strike" internally. The displayed name (title image `0x2738`, menu text `0x7D38`)
    was not read as an image. Confirm on screen before naming anything in the port.

8.10 Stock count upper bound. The asm uses 6 slots per port and does not state a maximum stock setting. A setting above 6 would
    index past the queue. Check the CSS stock range before porting step 2.

8.11 No build or run. This is read-only research. No compile, no game run, no cmake. The asm was not assembled.

8.12 ai-memory. The project scope for BattleShip did not resolve (`project 'BattleShip' not found in workspace 'default'`, and
    `JRickey/BattleShip` is not a valid project name). Only the three `_global` rules pages were read.
