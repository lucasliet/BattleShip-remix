# Smart App Control blocks new unsigned Windows executables

## Symptom

From `v2.0.1-port.10` on, Windows 11 with Smart App Control on refused to open `BattleShip-Remix.exe` ("Smart App
Control blocked an app that may be unsafe", "we couldn't verify its publisher"). Players reported it as an antivirus
alert during the in-game update, because the update script restarted the game into the new exe. `v2.0.1-port.9` opened
on the same machine.

## Cause

None of our executables is signed. Smart App Control asks Microsoft's cloud service about each executable; when the
service cannot vouch for it, it allows the file only if it carries a valid signature. A new build is a new file the
service has not seen, and whether it passes varies from build to build. On the reporting machine (Smart App Control on,
`OneDrive\Área de Trabalho`):

| Executable | UTF-8 manifest | Result |
|---|---|---|
| `v2.0.1-port.9` game and `torch.exe` (published, run before) | no | run |
| `v2.0.1-port.10` and the first `v2.0.1-port.11` build | yes | blocked |
| Test builds B, C, D of the port.11 code | no | opened |
| `torch.exe` of build D | no | blocked (`CreateProcess` error 4551, no notice shown) |
| Build E: the game with Torch linked in | no | blocked |

The first guess was the `activeCodePage` UTF-8 element that `9f3d3de` added to the manifest, since B, C and D opened
without it; build E, also without it, was blocked, so the manifest is not shown to be the cause. Only a signature makes
the result predictable, and a test build that opens does not prove the release build will: the release CI compiles
new files.

## Changes

- **No manifest; UTF-8 paths another way.** `main` calls `setlocale(LC_CTYPE, ".UTF-8")` on Windows before any path is
  built (`port_use_utf8_narrow_strings` in `port/port.cpp`). The MSVC STL picks the code page of `std::filesystem`
  narrow strings from the C runtime locale (`__std_fs_code_page` returns `CP_UTF8` when `___lc_codepage_func()` does),
  and the UCRT file functions follow it too, so paths reach libzip, spdlog and SDL as UTF-8. A failed `setlocale` is
  logged. The locale does not reach the ANSI (`A`) Win32 calls: build C opened from an accented folder but failed the
  first-run extraction with "Could not open extraction log for writing" (`CreateFileA` on a UTF-8 path). The calls that
  take a path the port built use the wide (`W`) versions through `port/win_utf8.h` (`Utf8ToWide`, `WideToUtf8`):
  `CreateFileW` and `CreateProcessW` in `first_run.cpp`. The `A` calls left in the crash handler only print module
  names or open a relative ASCII file.
- **Torch inside the game.** Torch is linked into the game on every desktop (`SSB64_TORCH_IN_PROCESS`,
  `port/torch_in_process.cpp`), as on Android and the PS5, and the Windows, macOS and Linux packages ship no `torch`
  executable: one unsigned executable less for Windows to judge. `RunTorchInProcess` runs the same O2R export as
  `torch o2r <rom> -s <dir> -d <dir>` and copies Torch's spdlog output into `logs/asset-extract.log` while it runs, then
  puts the game's logger and level back (Torch's `config.yml` sets a global level). On MSVC the game also links
  `yaml-cpp` for its `YAML_CPP_STATIC_DEFINE` (LNK2019 on `YAML::Node` without it).
- **Updater.** The Windows updater stays notify-only (`b95f1dd`) and the asset stays `BattleShip-Remix-windows-x64.zip`,
  so the port.10 script never runs again. The in-game install (`update_game.bat`) stays off: its ANSI calls read UTF-8
  paths only under the manifest, and an update from a folder with an accented name cannot be tested until a newer
  release is published.
- **`v2.0.1-port.11` on Windows** ships build D with the `torch.exe` of `v2.0.1-port.9`, uploaded by hand: that pair
  opened and extracted on the reporting machine. Code signing (SignPath Foundation application sent) is the fix that
  lasts.

## Verification

- Build D with the `torch.exe` of `v2.0.1-port.9` next to it extracted the ROM from `OneDrive\Área de Trabalho` with
  Smart App Control on and reached the game.
- In-process Torch on macOS, run with `SHIP_HOME` set to an empty folder named `Área de Trabalho`: the game extracted
  `BattleShip-Remix.o2r` in about 7 s and booted. Its 5,484 entries have the same CRCs as the archive the `torch`
  sidecar extracted, and the recipe is `a1b04b12…`, the Switch's. The CI-built macOS app did the same; the CI Linux
  AppImage and Windows zip built with Torch linked in.
