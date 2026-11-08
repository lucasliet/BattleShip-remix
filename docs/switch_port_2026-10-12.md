# BattleShip — Nintendo Switch (homebrew) port notes

**Status:** shipped in the releases (`BattleShip-Remix.nro` and `BattleShip-Remix-switch.zip`); first written 2026-10-12.
**Branches:** `ssb64-nx` in the `libultraship/` submodule, `feature/remix-port` here.
Build and packaging commands: `docs/remix/build_and_roms.md` (Switch build, in-game updater).

## Architecture

```
host (macOS / Linux / Windows desktop)         Nintendo Switch (homebrew NRO)
──────────────────────────────────────         ──────────────────────────────
user's ROM (baserom.remix.z64)                 BattleShip-Remix.nro  (ssb64 + LUS ssb64-nx)
  └─ first run / ExtractAssets (Torch)         f3d.o2r               (shaders)
       └─ BattleShip-Remix.o2r + .recipe       BattleShip-Remix.o2r  (assets from the user's ROM)
            └─ copied to the SD card:          BattleShip-Remix.o2r.recipe
               sdmc:/switch/BattleShip-Remix/
```

- The game logic is decompiled C compiled natively for aarch64 (the same path as Android; `coroutine_aarch64.S` is reused).
- Render: Fast3D → desktop OpenGL 2.1 core through `switch-mesa` (Nouveau/GM20B) and `switch-glad`; window, audio and input through `switch-sdl2`.
- No JIT: `DISABLE_SCRIPTING=ON` is forced (as on Android and UWP), so the TinyCC/funchook mod system does not exist on the Switch.
- The toolchain is contained in `.toolchain/` (devkitPro `.pkg.tar.*` pinned by SHA256 in `scripts/switch/packages.lock`); nothing is installed on the system.

## Port layers

| Layer | What was done |
|---|---|
| `libultraship` branch `ssb64-nx` | Switch support ported from Net64DD/main-nx (same structural generation; the nx delta is 27 files): SwitchImpl/SwitchController, `switch.cmake` (deps through FetchContent), the imgui-switch patch in the chain, `gfx_sdl2` (GL 2.1 core + glad + GetDisplaySize), Context/Gui/os.cpp/ResourceManager wiring |
| `port/` (app) | `__SWITCH__` coroutine (aligned_alloc, no guard page), first run replaced by the asset check in `port/switch_assets.cpp`, crash-handler signals left at the libnx default, CMake exclusions, NRO packaging (`nx_generate_nacp`/`nx_create_nro`), the libcurl updater (`UpdaterSwitch.cpp`) |
| Toolchain | `scripts/switch/{setup-toolchain,build,pack,relocate-toolchain}.sh`: idempotent bootstrap, sha256 check against the lock, prefix relocation, offline bundle |

CI: `.github/workflows/build-switch.yml` is disabled in the fork; the NRO is built by the `build-switch` job of `release-remix.yml` (devkitpro/devkita64 container with a dated tag; an ephemeral runner, so the local containment rule does not apply).

## Asset check at start-up

The console never reads a ROM and never extracts anything (libnx has no `std::system()` and no dialogs). Before the game starts, `SwitchAssetsAreCurrent` compares the `.o2r.recipe` next to the archive with the recipe hash baked into the `.nro` and shows the system error screen when:

- `BattleShip-Remix.o2r` is missing;
- `BattleShip-Remix.o2r.recipe` is missing or empty;
- the recipes differ (the screen shows the first 8 characters of both). The `.o2r` and the `.nro` must come from the same release, because some releases change the extraction.

The Details text (with the absolute destination folder) is also written to `ssb64.log`.

## Decisions and reasons

- **LUS: port of the nx delta, not a rebase.** Net64DD/main-nx and the ssb64 fork share the upstream merge-base `f5c3843f`; the isolated nx delta is small (+805/−19). Rebasing ssb64 (197 commits) onto main-nx would rewrite 197 patches for nothing.
- **GL stack = desktop GL 2.1 through Mesa Nouveau, not GLES.** The Fast3D shaders on the GL core path emit `#version 110/130`, the profile validated on SoH Switch. `USE_OPENGLES` stays Android-only.
- **No crash handler of our own:** newlib has no execinfo; the HOS crash reporter does the job.
- **Widescreen stays off on the Switch** (`kWidescreenDefault = 0` in `port/widescreen/widescreen.cpp`): Center4_3 left side bars on battle backdrops and offset the CSS fighters from the selector on hardware.
- **Full-memory mode is required:** hold R while opening an installed game. Applet mode does not have enough memory.

## Hardware validation checklist

The Remix hardware rounds are recorded in `docs/remix/ROADMAP.md` (T10.2). Items still worth checking on a console:

- [ ] Boot in full-memory mode; applet mode must fail with a clear message.
- [ ] Title → CSS → a 4-player VS match at 60 fps docked and handheld.
- [ ] Joy-Con / Pro Controller: input, gyro, rumble.
- [ ] Dock ↔ handheld mid-session without a crash (the LUS viewport resize has historically been sensitive).
- [ ] Audio comes back after sleep (applet hook workaround).

## Open questions

1. Default clock profile: should `SwitchProfiles::MAXIMUM` be applied at boot (`ApplyOverclock` in the Gui init)? Check power draw and temperature.
2. Whether any other file besides the `.nro` should be replaced by the updater (today only the `.nro` is; `f3d.o2r`, `config.yml`, `assets/` and a new `.o2r`/`.recipe` are copied by hand).
