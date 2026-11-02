# F14: Training mode port setup (Smash Remix 2.0.1)

Sources: `.toolchain/pkgs/smashremix-src/src/Training.asm` (ASM line refs), `decomp/src/sc/sc1pmode/sc1ptrainingmode.c` (C refs). Facts only. UNKNOWN = not determined from the read ranges.

## 1. struct.port_N (Training.asm 56-137)

Four records, stride 0x24 (reset handler computes `port * 0x24`, lines 921-923). Defaults are identical for all four ports.

| Off | Field | Default | Meaning and setters |
|---|---|---|---|
| 0x00 | ID | 0 | Training entry index (lines 164-171). Written from Global.vs char id via `char_id_to_entry_id` at the first load, only if char != NONE (lines 288-296). Set by menu "Character:". |
| 0x04 | type | 2 (Disabled) | 0 Human, 1 CPU, 2 Disabled (lines 2856-2858). Set by menu "Type:" (line 3668, range 0-2). Initialised from Global.vs+2 (line 297). |
| 0x08 | costume | 0 | Costume index. Menu "Costume:" range 0-5 (line 3667). Classic Sonic is stored as costume+6 (lines 300-306). |
| 0x0C | percent | 0 | Set by menu "Percent:" (`set_percent_`, lines 3622-3650). Reset to 0 at init (line 310). Applied on respawn when "Reset Sets Percent" is on (lines 443-449). |
| 0x10 | spawn_id | 0 | 0-3 = Port 1-4, 4 = Custom (comment line 45). Menu "Spawn:" range 0-4 (line 3669). Set to port index at SSS load (lines 841-853). |
| 0x14 | spawn_pos.x | 0.0 | float32, written by `set_custom_spawn_` (line 1396). |
| 0x18 | spawn_pos.y | 0.0 | float32, written by `set_custom_spawn_` (line 1398). |
| 0x1C | spawn_dir | FACE_LEFT (0xFFFFFFFF) | Written from fighter +0x44 (line 1400). |
| 0x20 | clipping_id | -1 | Fighter +0x140 if fighter action == CliffWait, else -1 (lines 1402-1409). |

Table of pointers: `struct.table` (lines 132-136). Menu entries per port: `tail_px` (lines 3660-3671).

## 2. Type values and side effects

- Values: 0 Human, 1 CPU, 2 Disabled. Default is 2 in the data, so a port starts Disabled unless the SSS load path copies values in.
- `load_character_` (lines 141-395) writes to the Global.vs player info:
  - char id to +0x03 (line 172), type to +0x02 (line 175), costume to +0x06 (line 183).
  - Tag: `color_id` = port index (0-3) when Human, 4 when not Human. Stored to +0x08 and +0x0A (lines 176-180).
- Classic Sonic: when char == SONIC and costume >= 6, the costume is reduced by 6 and the `classic_table` flag is set (lines 185-194 and the same block per port).
- pkind: Training.asm never writes pkind (no match in grep). UNKNOWN which game code reads Global.vs+0x02 to set pkind.
- controller: Training.asm never writes controller. UNKNOWN.
- level: not written by Training.asm. UNKNOWN.
- Disabled human guard (lines 918-937): on reset, if the human port (byte at 0x800A4AE3) has type == 2, the code sets type = 0 (Human) and sets the menu entry value to 0.

## 3. Character and Costume mapping

- `entry_id_to_char_id` (line 3368): menu entry index to vs char id. Order starts MARIO, FOX, DK, SAMUS, LUIGI, LINK, ... and ends with the Remix entries NCRASH, then `PLACEHOLDER` ("Random") (line 3466-3470).
- `char_id_to_entry_id` (line 3472): vs char id to menu entry index. Used at SSS init (line 292-293).
- Character menu max = `char_id_to_entry_id - entry_id_to_char_id - 2` (line 3660). Whether this excludes the Random entry: UNKNOWN.
- Costume: direct byte, no table.
- A Character change writes only the struct (ID). The fighter's vs char id is produced in `load_character_` on the next training load (section 4).

## 4. When a change takes effect

- `load_character_` (hook at 0x00116AA0, lines 157-163): if `reset_counter == 0` (came from SSS), it initialises the struct from Global.vs. Otherwise it reads the struct and writes Global.vs (`_load_p1`).
- `load_from_reset_` (lines 865-945) increments `reset_counter` (lines 898-901). Exit and reset both use this path.
- So Character, Costume and Type take effect on the next training load (reset or exit-reset), not immediately.
- Immediate respawn on menu change: no code path found in the read ranges. UNKNOWN.
- Menu changes do call `struct_to_tail_` only to refresh menu values (line 378 and 3700-3732).

## 5. Spawn

- Options: 0-3 = "Port 1".."Port 4", 4 = "Custom" (lines 3576-3580).
- Ports 0-3: `init_character_` (lines 426-441) only acts when spawn_id >= 4. For ports 1-4 the game's default spawn applies. Decomp: fighter position = `mpCollisionGetPlayerMapObjPosition(player, &desc.pos)` (sc1ptrainingmode.c line 2096). Stage map table: UNKNOWN in asm.
- Custom: stored in struct +0x14..+0x20 (section 1).
- "Set Custom Spawn" (`entry_set_custom_spawn_pN`, line 3670) calls `spawn_func_N_` (lines 3617-3620), which calls `set_custom_spawn(N)` (macro, lines 3592-3615):
  - `Character.port_to_struct_(N-1)` returns the fighter in port N-1. If null, the save is skipped.
  - It needs a live fighter in that port at button press. It reads the fighter's position pointer (+0x78), facing (+0x44), action (+0x24) and ledge clipping (+0x140).
  - Sets spawn_id = 4 (lines 1410-1412) and refreshes the label.
  - Note: it saves the fighter of the port whose menu it is (N), not necessarily the human.
- Application: `init_character_` (lines 426-441), only when `ra == 0x800D86B4` (respawn call site, line 423) and spawn_id >= 4. It sets facing (+0x44 = spawn_dir) and, if clipping_id >= 0, the ledge clip (+0x140).
- X/Y spawn_pos: not written in `init_character_`. UNKNOWN where it is applied.

## 6. Fighter file reloads

- Decomp loads files per fkind at scene start: `ftManagerSetupFilesAllKind(...fkind)` per player (sc1ptrainingmode.c line 2092) and `ftManagerSetupFilesPlayablesAll()` (line 2117). A Character (fkind) change therefore needs a scene start.
- Training.asm contains no file load, no `ftManager*` call and no map-position call (grep verified).
- Remix's mechanism: changes are applied by the training reload path (reset or exit-reset, section 4), so the scene restarts. Whether the restart is a full scene reload or a partial one: UNKNOWN.
- Costume: passed in `desc.costume` at creation (line 2102). Whether a costume-only change reloads files: UNKNOWN.
- Type, Percent, Spawn: no file reload (they are struct and fighter-field values).

## 7. Decomp changes needed for 4 ports

Current code only has one human and one CPU:

- `sc1PTrainingModeInitVars` (line 740). Lines 762-779 set only `players[player == gSCManagerSceneData.player]` and `players[dummy]`, with one `dummy` index.
- Lines 779-780: `gSCManagerBattleState->pl_count = 1; gSCManagerBattleState->cp_count = 1;`. These drive `desc.detail` (line 2101).
- `gSCManagerSceneData` holds only `training_man_fkind` / `training_com_fkind` and costumes (sctypes.h lines 469-471), filled from two CSS slots (mnplayers1ptraining.c lines 3232-3235). Four ports need per-port fkind, costume, pkind, level and spawn data.
- `sc1PTrainingModeFuncStart` loop (lines 2084-2115) already loops over all `ARRAY_COUNT(players)`, skips `nFTPlayerKindNot` (2088-2091), and sets `desc.controller = &gSYControllerDevices[player]` (2109). Good as a base. It needs per-port pkind, fkind, costume, level and a spawn override (replace line 2096 when spawn is Custom or Port N).
- `sc1PTrainingModeUpdateDummyBehavior` (lines 2040-2054) uses one `sSC1PTrainingModeMenu.dummy`. It needs per-port CP behaviour.
- Add a C mirror of `struct.port_N` (0x24 stride, fields in section 1) or a `static` array in `sc1ptrainingremix.c`, and apply the Remix type/spawn rules (sections 2 and 5) there, not in the vanilla scene code.
- Remix fields the decomp does not model yet: classic Sonic flags, percent toggle, custom spawn with clipping id.
