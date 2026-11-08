# Switch: stage select start and 1P VS card stutter (2026-11-08)

## Symptoms

- On the Switch, the first entry to the stage select froze for about 1.3 s.
- The 1P VS intro card stuttered with the Fighting Polygon Team, multi-Yoshi and multi-Kirby opponents. Frames took about 54 ms, while 60 fps allows 16.7 ms.
- A docked Switch could not answer the update popup: only touch worked, and the B button started the download.

## How it was measured

Two opt-in env vars report timings on the console. On the Switch, set them in `env.txt` in the app folder, one `NAME=value` per line:

- `SSB64_SLOW_FRAME_MS=<ms>` logs every frame slower than the threshold, split into game logic, display list walk and end frame.
- `SSB64_FRAME_PROFILE=1` adds per-frame time and call counts for: display list address checks, vertex and texture fixups, and file loads (`port/port_frame_profile.*`).

`mnMapsFuncStart` also logs how long each of its steps takes (`[mnmaps] start: ...`).

## Root causes and fixes

1. **Stage select sizing.** `mnMapsAllocModelHeaps` sizes every Remix stage file with `lbRelocGetFileSize`. Each call loaded and decompressed the file and all its dependencies.
   - `lbRelocGetFileSize` and `lbRelocGetExternBytesNum` now read only the file header. They use `ArchiveManager::LoadFilePrefix`, a new partial `zip_fread` in libultraship.
   - The headers are cached until the next real load.
   - The final sizes are kept in `reloc_sizes.cache` in the app folder (`port/bridge/reloc_size_cache.*`). The file is keyed by the asset recipe hash plus each mounted archive's path and size.
   - Result on the Switch: 1283 ms → 422 ms → 0 ms from the second boot on.
2. **Card file reloads.** The card's animation code called `lbRelocGetForceExternHeapFile` every frame, which reloaded the same files. That cost 30 loads, or 7.6 ms per frame.
   - The bridge now remembers which file sits at each heap address (`sPortForceHeapLoads`). When the same file is asked for at the same address, it is reused.
   - The record is dropped when the range is overwritten, and on `lbRelocInitSetup`.
3. **Display list bounds checks.** The interpreter checked about 25,000 addresses per frame against the registered display list ranges (9.3 ms). Most addresses were in game-built lists outside any range, so every check fell through to the slow path.
   - `port_dl_range_lookup` now returns the span around the address that has the same answer, including the gaps between ranges.
   - libultraship keeps eight of those spans (`WalkBoundsCheck`, registered with `Fast::RegisterDLRangeLookup`).
   - Result: 2 checks per frame.
4. **Update popup.** The popup relied on ImGui gamepad navigation, which had no focused item.
   - It now reads the pad buttons directly.
   - The Switch SDL maps buttons by position, so Nintendo A arrives as `GamepadFaceRight`. Confirm and cancel are therefore swapped under `__SWITCH__`.

## Result on hardware

- Card frames: about 54 ms → 21–25 ms.
- Stage select start: 0 ms after the first boot.

## Still open

- Display list walk: 18–21 ms per frame on the card.
- Every 8th frame, game logic takes about 20 ms.
