# F10 menu: Character Data screen (0x1A) and VS records (0x19)

Status: Character Data (0x1A) ported to the 30-fighter Remix order, not compiled or run. VS records (0x19) left vanilla, because the Remix asm does not change that screen. Both decisions follow the coordinator's option 1.

## Scope decided

The Remix asm (`.toolchain/pkgs/smashremix-src/src/CharacterDataScreen.asm`) registers 18 extra fighters on the data screen. Its `set_char_order` (`:938-980`) lists 30 fighters, and the scroll max is `new_characters + 11 = 29` (`extend_index_`, `:982-992`). The brief's "all 97" would need data the asm never defines for 67 fighters, so it is out of scope. No content was invented.

## Character Data screen (0x1A): what was ported

New files (owned by this feature):
- `decomp/src/mn/mndata/mnremixchardata.h`: page count 30, Remix file IDs `0x10F5` (bios) and `0x10F6` (images), file indexes 4 and 5 in `sMNCharactersFiles`, sprite-kind enum, and the lookup API.
- `decomp/src/mn/mndata/mnremixchardata.c`: the 30-entry order (Remix ids, `set_char_order` order), the 18 extended entries (raw asm bio, works and name values, name x/y from `add_char_to_data_screen`), and the decoder. The decoder adds the asm addends (bio `+0x30`, works `+0x10`, name `+0x10`), takes the top-byte flag as "use the extended file", and masks the offset to 24 bits.

Edited `decomp/src/mn/mndata/mncharacters.c` (all hooks `#ifdef REGION_REMIX`; the vanilla path is unchanged):
- Include and `MNCHARACTERS_FIRST_PAGE` / `MNCHARACTERS_LAST_PAGE` macros (`0..29` under REGION_REMIX; `nFTKindPlayableStart`/`End` otherwise).
- `dMNCharactersFileIDs`: appends `0x10F5` and `0x10F6`.
- `mnCharactersGetFighterKind` and `mnCharactersGetPage`: delegate to the Remix order.
- `mnCharactersMakeFighter`: `ftManagerSetupFilesAllKind(desc.fkind)` before `ftManagerMakeFighter`, the same pattern as `mnPlayersVSMakeFighter` (`mnplayersvs.c:2063-2068`).
- `mnCharactersMakeStory`, `mnCharactersMakeWorks`, `mnCharactersMakeName`: early return into static helpers that read the sprite from the Remix extended files. The name helper uses the asm name x/y. Story and name draw after the vanilla tag and wallpaper blocks, so the tag still shows.
- `mnCharactersMakeEmblem`: for extended fighters the gobj is created but nothing is attached, so `sMNCharactersEmblemGObj` stays non-null and `ChangeFighter`'s eject path stays valid (REMIX-TODO(chardata-emblem)).
- `mnCharactersGetMotion`: extended fighters use `dMNCharactersCommonMotionDescs` (vanilla special and attack1 tables are indexed by fkind and would overflow).
- `mnCharactersUpdateMotionName`: returns before the special-move name sprite for extended fighters (REMIX-TODO(chardata-motion-names)).

Syntax: `scripts/remix/syntax-check.sh` reports OK for `mnremixchardata.c` and `mncharacters.c`. The build flags include `-DREGION_REMIX -DPORT`, so the Remix branches were compiled.

## Left as REMIX-TODO (not in the asm or not located in the decomp)

- `chardata-emblem`: the asm adds no emblem models for the 18 fighters. Emblem stays blank for them.
- `chardata-motion-names`: no special-move name sprites for the 18 fighters.
- `chardata-special-motions`: the 18 fighters use common motion descriptors, so their special and attack1 buttons show the common (null) status instead of a per-fighter motion. Unverified visually.
- `chardata-stats`: the asm's `special_attack_offsets` (USP/NSP/DSP image offsets, patch at `0x15EE74`) and the extended action sequences (`special_action_pointers`, `jab_action_pointers`) have no decomp function to hook (the patch addresses are not mapped to `decomp/src`). Not ported.

## VS records (0x19): no change

The Remix asm does not change scene 0x19. `Global.asm:84` only defines `DATA_VS_RECORD(0x19)`, and `BGM.asm:644` only references it for music. `VsStats.asm` is the results-screen stats overlay, not this screen. So on 0x19 the Remix game shows the vanilla 12 playable fighters, and the paging over 97 rows in the brief has no Remix layout to copy. `decomp/src/mn/mndata/mnvsrecord.c` is untouched. If the lead wants 97 rows there, that is new layout work, not a port.

## For the lead to check on screen

- Scene 0x1A, `SSB64_START_SCENE=26`. Expected: 30 fighters in order MARIO, LUIGI, PEACH, BOWSER, DRM, DK, WARIO, LINK, YLINK, SHEIK, GND, SAMUS, YOSHI, KIRBY, DEDEDE, FOX, FALCO, WOLF, PIKACHU, JIGGLYPUFF, MTWO, CAPTAIN, NESS, MARTH, GOEMON, SONIC, CRASH, MARINA, BANJO, CONKER. Left and right wrap at both ends.
- For an extended fighter (e.g. FALCO, WARIO, PEACH): the model appears (its files load on change), the name and works sprites come from the Remix files, and the emblem and special-move names are blank (REMIX-TODO above).
- Check the Remix o2r actually provides file IDs `0x10F5` and `0x10F6`. If the loader cannot find them, the screen fails when an extended fighter is selected. This was not verified.
- Scene 0x19, `SSB64_START_SCENE=25`: unchanged; should match vanilla.

## Not done

- No build, no run, no git commit (per the brief).
- ai-memory: the project scope for BattleShip did not resolve (`workspace "JRickey"` not found, and the repo has no `.ai-memory.toml`). Only the three `_global` rules were read.
