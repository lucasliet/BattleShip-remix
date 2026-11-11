# PS5 never loaded a hi-res texture pack (`mods/` listing refused)

Issue: lucasliet/BattleShip-remix#5.

## Symptom

A hi-res pack zip in `/data/homebrew/PPSA99264/mods/` (the title's `mods/` folder) never showed up in game. The pack is
on by default on the PS5, and the same zip works on desktop and Switch. The port log said:

```
HiResPack: cannot scan /app0/mods (filesystem error: in recursive_directory_iterator: Operation not permitted ["/app0/mods"]); hi-res pack disabled this run
```

## Root cause

The PS5 title is refused `opendir` under `/app0` (EPERM), as already listed in `docs/remix/build_and_roms.md` for the
ROM lookup and Torch. `HiResPack::Init` lists `mods/` with `Directory::ListFiles`, which uses
`std::filesystem::recursive_directory_iterator`, which is built on `opendir`. The iterator threw, so the scan was
dropped and no texture was indexed.

## Fix

`port/ps5/ps5_dir_list.cpp` lists a folder with `open(O_DIRECTORY)` and `getdents`, without `opendir`, and walks
subfolders (`stat` decides when `getdents` gives no type). On the PS5, `HiResPack::Init` uses it instead of
`Directory::ListFiles`; when a call fails, the log names the call, the path and the errno text. Other platforms are
unchanged.

## Verification

PS5 hardware, SSB Reloaded `ssb-reloaded-v2.0.1-battleship.zip` (220 MB) in `mods/`:

- before: the error above, no pack textures;
- after: `scanned /app0/mods — 1 files, 4512 indexed, 432 unparsed, 175 hash collisions, 1 zip(s) in 23 ms`, preload
  warmed 2682 textures (LRU 511/512 MB), hit rate rising to 59% after 448 lookups, 0 decode failures, pack textures
  visible in game;
- loose PNGs (910 from `UI/Main Menu`, `UI/HUD`, `Stages/Dream Land`, `Characters/Mario`, zip renamed away):
  `scanned /app0/mods — 911 files, 854 indexed, 0 unparsed, 56 hash collisions, 0 zip(s) in 98 ms`, 854 warmed
  (235 MB), hits from the menus on, 0 decode failures, textures visible in game.
