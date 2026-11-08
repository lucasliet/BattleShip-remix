# Builds and ROMs: BattleShip original (US) and BattleShip-Remix

Covers the ROMs each build needs, which Remix version they are, and the desktop, packaging, Switch, Android and PS5 build commands. Prerequisites and the
generic steps for Linux, macOS and Windows are in `BUILDING.md`; this page adds what is specific to the Remix. Code and scripts: `CMakeLists.txt` (`SSB64_VERSION`,
`SSB64_APP_NAME`) and the `scripts/` folder of each platform.

No ROM and no `.o2r` enters git (`*.z64`, `build/`, `build-*/` and `dist/` are in `.gitignore`). Whoever clones the repository supplies the ROMs below and
generates the `.o2r` on the desktop.

## Required ROMs

They sit in the **repository root**. CMake accepts `.z64`, `.n64` and `.v64` (Torch normalizes the byte order).

| File | Used for | Size | SHA-1 | MD5 |
|---|---|---|---|---|
| `baserom.remix.z64` | Remix build: source of `BattleShip-Remix.o2r` (`torch o2r`) and of the fighter, audio and file tables | 65 536 000 | `13716381ed5e29be2606e0f4724b18fd00789d04` | `2b2d6b295106c54216b7fc7a2f14346e` |
| `baserom.us.z64` | Vanilla build. Optional for the Remix: dev builds derive three CSS stage PNGs from it (`derive_stage_assets.py`), and `scripts/remix/assemble-remix.sh` assembles the Remix source over it | 16 777 216 | `e2929e10fccc0aa84e5776227e798abc07cedabf` | `f7c52568a31aadf26e14dc2b6416b2ed` |

SHA-256 of the reference copies:

- `baserom.remix.z64`: `7efec9e0983656bb0219a23c511cd1505a5f84d524e50ad4284dc1c7eb4d1403`
- `baserom.us.z64`: `15592e79d3c5295cef4371d4992f0bd25bec2102fc29644c93e682f7ea99ef3d`

N64 header CRCs: Remix `34EE4749 1833C64C`; US `916B8B5B 780B85A4`. Both have the game code `NALE`; the internal name of the Remix ROM is `SMASH REMIX`.

The JP ROM (`baserom.jp.z64`) is not used by the Remix.

Without `baserom.us.z64` the Remix build compiles (the `DeriveStageAssets` target becomes a stub) and the stage select still shows every thumbnail:
`port/css_icons/port_css_stage_assets.cpp` derives the background and icon from the wallpaper sprite in the extracted `.o2r`, as in every release build.

## Which Remix version

**Smash Remix 2.0.1**, the release ROM (the one the project distributes on the Releases tab of the `JSsixtyfour/smashremix` repository, applied over a US ROM).

Evidence that `baserom.remix.z64` is 2.0.1:

- The string `Smash Remix v2.0.1` is inside the ROM.
- The Remix source used (`.toolchain/pkgs/smashremix-src`) is pinned at commit `5e04fe7fcd023cd43c71f25f89bb6e810d254d55` (tag `2.0.1`, 2026-03-08).
  Assembled by `scripts/remix/assemble-remix.sh` over `baserom.us.z64`, the code image (ROM `0x2C00000`, 4 MB) matches the one in `baserom.remix.z64`
  except in 1 038 of 1 048 576 words, which are data read from the injected files.

The hashes above are those of the reference copy. If another 2.0.1 release copy differs, compare the game code, the version string and the code image before
concluding it is wrong.

The Remix source ships `original.xdelta` (SHA-256 `e892db9f57135ae2ba380e6db98516dd849dcabb4be5c553ea48158ec7dece1d`), which produces the base ROM for the asm
assembly from a US ROM. The vanilla ROM cannot stand in for `baserom.remix.z64`, because many Remix edits are inside compressed ROM files. The project does not run xdelta.

## Two products, two build directories

| Product | `SSB64_VERSION` | ROM | Binary | Assets archive | Suggested build directory |
|---|---|---|---|---|---|
| BattleShip (original SSB64, US) | `us` | `baserom.us.z64` | `BattleShip` | `BattleShip.o2r` | `build-us` |
| BattleShip-Remix | `remix` (default) | `baserom.remix.z64` (+ `baserom.us.z64`) | `BattleShip-Remix` | `BattleShip-Remix.o2r` | `build` |

The game code is compiled per region (`REGION_US` or `REGION_REMIX`), so each version is its own build with its own directory. The binary and `.o2r` names come from
`SSB64_APP_NAME` (`CMakeLists.txt`). Both can exist side by side.

## Desktop build

Choose the product with `SSB64_VERSION` (`us` or `remix`, default `remix`) and use one build directory per product (table above):

```bash
cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release -DSSB64_VERSION=remix
cmake --build build --target ssb64 -j 4
```

For the original BattleShip use `-B build-us -DSSB64_VERSION=us`; it only needs `baserom.us.z64`, and the `BattleShip` binary loads `BattleShip.o2r` and `f3d.o2r` from the directory
where it runs (`ExtractAssets` generates them from the ROM). The Remix result in `build/` is `BattleShip-Remix` (the target is `ssb64`), `BattleShip-Remix.o2r`,
`BattleShip-Remix.o2r.recipe` and `f3d.o2r`. A new file under `decomp/src` needs the `cmake -S ... -B ...` step again before the build.

### Optional: desktop dependencies without a system install (macOS)

Instead of installing the libraries on the system, two scripts put everything under a `.toolchain/` folder inside the repository (it is not in git). Run
`scripts/switch/setup-toolchain.sh` first (it downloads cmake, ninja and the Switch toolchain, and writes `.toolchain/env.sh`, which the other scripts need), then
`scripts/macos/setup-host-deps.sh` (libzip, nlohmann_json, tinyxml2, spdlog and SDL2 under `.toolchain/host/deps`). Neither uses a package manager or `sudo`; they expect `python3`,
`curl`, `unzip`, `uv` and the Xcode command line tools. The desktop script does not build GLEW: `libGLEW.a` 2.1.0 has to be in `.toolchain/host/deps/lib` already (a known gap).
Then configure with that cmake and ninja and point CMake at those libraries:

```bash
PATH=$PWD/.toolchain/host/ninja:$PATH
.toolchain/host/cmake/CMake.app/Contents/bin/cmake -S $PWD -B $PWD/build -GNinja -DCMAKE_BUILD_TYPE=Release -DSSB64_VERSION=remix \
  -DCMAKE_IGNORE_PREFIX_PATH="/opt/homebrew;/usr/local" -DCMAKE_PREFIX_PATH=$PWD/.toolchain/host/deps
```

### Assets

`ssb64` depends on `ExtractAssets`: with `baserom.remix.z64` present, the build runs `torch o2r baserom.remix.z64 -s <repo> -d build/extracted` and copies the result.
It then redoes the `.o2r` only when Torch, `config.yml`, `yamls/remix/*.yml` or the ROM change. The `.recipe` (41 bytes) is the SHA-1 of those inputs
(`<torch sha>:<config.yml sha>:<sha of each yaml>`); if it changes, the port re-extracts the `.o2r` by itself.

To redo only the assets: `cmake --build build --target ExtractAssets`.

## Test packages of BattleShip-Remix (macOS and Linux)

These packages **embed `BattleShip-Remix.o2r`**, which is derived from the ROM. They are for personal use only and are never redistributed. A distributable package does
not carry the `.o2r`: it is extracted from the user's ROM on first run, as `scripts/package-macos.sh`, `scripts/package-linux.sh` and
`scripts/package-windows.ps1` do for every version. The fork's releases come from `.github/workflows/release-remix.yml`: a `v*` tag builds the
macOS `.dmg` (ad-hoc signed, not notarized), the Linux AppImage, the Windows zip and the Switch SD-card zip (without `.o2r`), and opens a draft release whose
body is `.github/release-notes.md`. The Remix build's updater reads `lucasliet/BattleShip-remix` releases
(`port/enhancements/Updater.cpp`). The inherited upstream workflows are disabled in the fork.

- **macOS:** first make a Release build with `-DNON_PORTABLE=ON` in `build-macos-release/`, then run `scripts/macos/package-remix-contained.sh`. It reads the `.o2r`,
  `f3d.o2r` and the icons from `build/` and writes `dist/desktop/BattleShip-Remix.app`, `.dmg` and `.zip`. It does not use dylibbundler: it expects the static libraries of the optional macOS
  setup above in `.toolchain/host/deps`. Scratch files go to `$TMPDIR` (default `build-macos-release/`) and are removed on exit.
- **Linux x86_64 (AppImage):** run `scripts/linux/package-remix-appimage.sh [build|package|all]`. It needs podman, builds inside a container described by
  `scripts/linux/Dockerfile.jammy` and writes to `build-linux-release/`.
- The version comes from `DIST_VERSION`, default `2.0.1-port.<short HEAD>`. Check that the `.o2r.recipe` in `build/` matches the current Torch before packaging.

## Installing a release package of BattleShip-Remix

1. Download the package of your platform (`BattleShip-Remix.dmg`, `BattleShip-Remix-x86_64.AppImage`,
   `BattleShip-Remix-windows.zip`). It has no game data.
2. Make the Smash Remix 2.0.1 ROM from your own Super Smash Bros. NTSC-U v1.0 ROM, with the web patcher at
   https://smashremix.net/patcher/ or the Remix 2.0.1 patch from the Releases tab of `JSsixtyfour/smashremix` (`.z64`, `.n64` or `.v64`; check the SHA-1 in **Required ROMs**).
3. Name it `baserom.remix.z64` (or `.n64` / `.v64`) and put it in the app data folder that the error dialog of the first
   launch prints, or next to the executable; or drag it onto the window of the first-run wizard.
4. On the first launch the game runs Torch and writes `BattleShip-Remix.o2r` and its `.recipe` to the app data folder; a later
   update of the asset pipeline re-extracts by itself.

Checked on 2026-11-06 with `SHIP_HOME` pointing at an empty folder that held only `baserom.remix.z64` and `f3d.o2r`: the
game extracted a 59 MB `BattleShip-Remix.o2r` and booted to the Remix splash (`first_run: extracted BattleShip-Remix.o2r`).

## Switch build (BattleShip and BattleShip-Remix)

The console never reads a ROM: the `.nro` is compiled with devkitA64 and the `.o2r` and its `.recipe` come from the desktop build of the same release (the game refuses to start when the `.recipe` is missing or its hash differs from the one baked into the `.nro`).

```bash
scripts/switch/setup-toolchain.sh        # once; downloads cmake, ninja and devkitA64, so it needs the network (a download blocked by the host makes it exit with code 2)

# Remix (default)
scripts/switch/build.sh remix            # build-switch/BattleShip-Remix.nro
VERSION=remix scripts/switch/pack.sh     # dist/switch/SD/switch/BattleShip-Remix/

# Original BattleShip (US)
scripts/switch/build.sh us               # build-switch/BattleShip.nro
VERSION=us scripts/switch/pack.sh        # dist/switch/SD/switch/BattleShip/
```

`pack.sh` looks for `<app>.o2r`, `<app>.o2r.recipe` and `f3d.o2r` in `build/` and then in the repository root, never in `build-us/`. For the original, copy those three files
from `build-us/` to the root (or to `build/`) before packing.

`pack.sh` picks the version only from the `VERSION` variable (default `remix`), not from an argument; without `VERSION=us` it packs the Remix. `build-switch/` is shared:
running `build.sh us` and then `build.sh remix` reconfigures the same directory, so pack each version right after its build.

`pack.sh` only copies: the `.nro` from `build-switch/`, and `BattleShip-Remix.o2r`, `BattleShip-Remix.o2r.recipe` and `f3d.o2r` from `build/`. If one is missing it only prints
`AVISO` and goes on, and the package comes out incomplete. So:

1. Run the desktop build first (it produces the `.o2r` files from `baserom.remix.z64`).
2. Run the Switch build and `pack.sh`.
3. Copy the whole folder `dist/switch/SD/switch/BattleShip-Remix/` to the SD card, under `/switch/`.
4. Open it from the Homebrew Menu in full-memory mode (hold R while opening an installed game).

The Home menu icon is a 256x256 JPEG embedded in the `.nro` by CMake (`assets/switch/icon.jpg` for the original, `assets/switch/icon-remix.jpg` for the Remix, with "SR"). Both
are made from `assets/icon.png` and `assets/icon-remix.png` (the "SR" artwork) by `.toolchain/host/venv/bin/python -I tools/gen_platform_icons.py` (Pillow only), which also writes the PS5 `icon0.png` and the Android remix launcher foreground.

### In-game updater (Switch)

The About menu (Updates section) checks `releases/latest` of `lucasliet/BattleShip-remix` and, on "Download Update", downloads the bare release asset `BattleShip-Remix.nro`
(the release workflow publishes it next to the zip) to `<running .nro>.part`, checks the size and the `NRO0` header, then swaps it over the running file (the old one
stays as `.old` until the swap succeeds). "Restart Game" relaunches it through `envSetNextLoad`. Only the `.nro` is replaced: if a release also changes `f3d.o2r`,
`config.yml`, `assets/` or needs a new `.o2r`/`.recipe`, those still have to be copied by hand. Code: `port/enhancements/UpdaterSwitch.cpp` (libcurl through the
`switch-curl` portlib, pinned in `scripts/switch/packages.lock`; TLS is the console's own ssl service), shared logic and tests in `UpdateRelease.cpp`/`UpdateInstall.cpp`
(`cmake --build build --target run_updater_tests`).

If the `.o2r` or Torch change (a new `.recipe`), rebuild the package: an old `.o2r` on the SD card with a new `.nro` causes a storm of invalid reloc tokens. The game writes
`ssb64.log` and `logs/BattleShip-Remix.log` next to the `.nro`, on the SD card; both are overwritten on every launch.

## Checklist for a fresh clone

1. `git submodule update --init --recursive`.
2. Put `baserom.remix.z64` and `baserom.us.z64` in the root and check the SHA-1 against the table above (`shasum -a 1 baserom.*.z64`).
3. The dependencies listed in `BUILDING.md` (on macOS, optionally the scripts under "Optional: desktop dependencies without a system install").
4. Desktop build (above), then the Switch build and pack.
5. For the Remix symbol map and the action tables: `scripts/remix/assemble-remix.sh` (writes `build/remixasm/remix.sym`), then
   `.toolchain/host/venv/bin/python -I tools/gen_remix_actions.py`.

## Android build (BattleShip and BattleShip-Remix)

Built on a Linux host with the Android SDK, NDK `29.0.14206865` and JDK 17+ (Gradle `externalNativeBuild` runs the same `CMakeLists.txt`).
The version is a Gradle property: `-Pssb64.version=remix` (default) or `us`.

```
cd android
./gradlew assembleDebug -Pssb64.version=remix -Pssb64.abis=arm64-v8a
```

| Version | Package id | Label | Archive |
|---|---|---|---|
| `remix` | `dev.lucasliet.battleship.remix` | BattleShip Remix | `BattleShip-Remix.o2r` |
| `us` | `com.jrickey.battleship` | BattleShip | `BattleShip.o2r` |

Debug builds add `.debug` to the package id. The app asks for the ROM on the first launch and runs Torch on the phone
(`port/android_torch_bridge.cpp`); the bridge renames the output to the version's archive and writes its `.recipe`, and
`AssetExtractor.haveExtractedRom` asks for the ROM again when the recipe changes. Per-version resources (launcher icon)
live in `android/app/src/<version>/res`.

ABIs: arm64-v8a is tested on a Samsung S25. armeabi-v7a compiles but is not tested (no 32-bit device; the Remix C was
written for LP64), so releases ship arm64 only. There is no x86/x86_64 build (the coroutine backend has no x86 swap).
Release signing reads `SSB64_RELEASE_KEYSTORE*` from the environment (repository secrets in CI); without them the APK is
debug-signed.

## PS5 build (BattleShip-Remix)

A native PS5 title (`PPSA99264`, "BattleShip Remix") for jailbroken consoles with ShadowMountPlus. Built on a Linux
x86_64 host (or the `build-ps5` job of `release-remix.yml`) with a toolchain kept under `.toolchain/ps5`; `scripts/ps5/setup-toolchain.sh` needs `git`, `curl`, `unzip`, `tar` with xz, `uv` and a host C compiler:

```
scripts/ps5/setup-toolchain.sh      # pinned SDK, LLVM 18, ps5-opengl, boilerplate, SDL (scripts/ps5/packages.lock)
scripts/ps5/build-gl-sdk.sh         # ps5-opengl's GL SDK and its static SDL2
scripts/ps5/build.sh remix          # build-ps5/ps5-app/dist/PPSA99264/ and PPSA99264.zip
scripts/ps5/deploy.sh <ps5-ip> build-ps5/ps5-app/dist/PPSA99264   # FTP to /data/homebrew/PPSA99264, then a rescan
```

The title folder holds `eboot.bin`, `sce_sys/` (param.json, icon, `pic0.dds` from `scripts/ps5/make_title_art.py`),
`sce_module/libc.prx`, `f3d.o2r`, `gamecontrollerdb.txt` and `assets/` (Torch's `config.yml` and `yamls/`, menu fonts).
The user adds `baserom.remix.z64`; on the first launch `port/ps5/ps5_assets.cpp` runs Torch in the game and writes
`BattleShip-Remix.o2r` and its `.recipe` next to it, with system notifications at start and end. The game's data
directory is the title folder (`/app0`, writable for a folder install), so saves, settings and `logs/` stay there.

What the title cannot do, and the workaround:

| Restriction | Workaround |
|---|---|
| `opendir` on `/app0` fails | ROM looked up by name; Torch reads a `.torch-filelist` per yaml folder (`cmake/ps5_torch_filelists.cmake`) |
| `lstat`, `getcwd`, `realpath` refused (EPERM) | `port/ps5/ps5_path_shims.c`, wrapped at link (as griches/PS5_Ghostship does) |
| a named `std::locale` crashes (SIGBUS) | yaml-cpp uses `std::locale::classic()` (patched in torch's CMakeLists for PS5) |
| every write to storage costs tens of ms | logs are buffered; Torch logs at `warn` |
| the Create button opens the system menu | L3+R3 (or the touchpad) toggles the port menu (`port/ps5/ps5_menu_button.cpp`) |

Logs over FTP: `/data/homebrew/PPSA99264/boot.txt` (boot steps, crash backtrace), `logs/BattleShip-Remix.log`.
