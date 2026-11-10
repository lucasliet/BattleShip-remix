# Smart App Control blocked the Windows exe that carried the UTF-8 manifest

## Symptom

From `v2.0.1-port.10` on, Windows 11 with Smart App Control on refused to open `BattleShip-Remix.exe` ("Smart App
Control blocked an app that may be unsafe"). Players reported it as an antivirus alert during the in-game update,
because the update script restarted the game into the new exe. `v2.0.1-port.9` opened on the same machine.

## Investigation

- port.9 and port.10/port.11 ship the same files, all unsigned. The only difference in the exe besides the code is the
  `activeCodePage` UTF-8 element that `9f3d3de` added to the manifest (`port/ssb64.manifest`).
- Smart App Control first asks Microsoft's cloud service about an exe; when the service cannot decide, it allows signed
  apps and blocks unsigned ones. The leading guess was that new unsigned exes had no reputation yet.
- Test on the blocked machine: the port.11 code built without the manifest (a new exe the cloud had never seen) opened.
  So reputation alone does not explain the block; the manifest element does.

## Fix

The manifest is gone. `main` calls `setlocale(LC_CTYPE, ".UTF-8")` on Windows before any path is built
(`port_use_utf8_narrow_strings` in `port/port.cpp`). The MSVC STL picks the code page of `std::filesystem` narrow
strings from the C runtime locale (`__std_fs_code_page` returns `CP_UTF8` when `___lc_codepage_func()` does), and the
UCRT file functions follow it too, so paths reach libzip, spdlog and SDL as UTF-8 as before. A failed `setlocale` is
logged.

The locale does not reach the ANSI (`A`) Win32 calls, which the manifest had also switched to UTF-8: the first `setlocale`
build opened from an accented folder but failed the first-run extraction with "Could not open extraction log for
writing" (`CreateFileA` on a UTF-8 path). The calls that take a path the port built now use the wide (`W`) versions
through `port/win_utf8.h` (`Utf8ToWide`, `WideToUtf8`): `CreateFileW` and `CreateProcessW` for torch in
`first_run.cpp`. The `A` calls left in the crash handler only print module names or open a relative ASCII file.

The Windows updater stays notify-only (`b95f1dd`) and the asset stays `BattleShip-Remix-windows-x64.zip`, so the
port.10 script never runs again. The in-game install (`update_game.bat`) is not turned back on: its ANSI calls also
relied on the manifest, and an update from a folder with an accented name cannot be tested until a newer release is
published.

## Verification

- The port.11 build without the manifest opened under Smart App Control on the reporting machine.
- The `setlocale` build opened from `OneDrive\Área de Trabalho` on the same machine (Smart App Control on, f3d.o2r
  loaded, first-run screen drawn) and failed only at the extraction log, which the `W` calls fix.
