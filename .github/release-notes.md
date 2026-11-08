## Highlights

**The in-game updater installs the whole release on every platform.** It now downloads the package made for your
platform instead of a single file.

- **macOS:** the update is installed in place. The app downloads `BattleShip-Remix-macos.zip`, unpacks the new app next
  to the installed one and swaps it in when you restart the game; there is no need to open the DMG again. The app must be
  an installed `.app` in a folder you can write to (for example Applications): a copy run from the mounted DMG reports an
  error, and then you install from the release page. An update applied this way does not need the right-click Open step.
- **Nintendo Switch:** the updater downloads `BattleShip-Remix-switch.zip`, replaces the `.nro` right away and stages
  the other files of the zip (`f3d.o2r`, `config.yml`, `gamecontrollerdb.txt`, `assets/`, `README.txt`); they are moved
  into place the next time the game boots. Your `BattleShip-Remix.o2r` and `.recipe` are never touched: when a release
  changes the asset pipeline you still copy a freshly extracted pair from a desktop build, and the error screen explains it.
  If applying the staged files fails, the error screen names the file and tells you to delete the `update-pending`
  folder or reinstall from the zip.
- **Windows:** fixed the updater. Its restart script looked for `BattleShip.exe` instead of `BattleShip-Remix.exe`, so it
  never waited for the game to close, could not unpack over the running executable and could not restart the game.

Switch builds up to `v2.0.1-port.9` still download the bare `BattleShip-Remix.nro`, so that file stays in this release.
Builds from this release on update from the Switch zip.

## Install

No game data is included: you need your own ROM.

- `baserom.remix.z64`: Smash Remix 2.0.1, made from your Super Smash Bros. NTSC-U v1.0 ROM with the
  [Smash Remix web patcher](https://smashremix.net/patcher/) (in the browser, nothing to install) or the Remix 2.0.1
  patch from the [Smash Remix releases](https://github.com/JSsixtyfour/smashremix/releases) (ROM SHA-1 `13716381ed5e29be2606e0f4724b18fd00789d04`; `.z64`, `.n64` or `.v64`).

**macOS, Linux, Windows:** run the app. On the first launch it asks for the ROM and extracts `BattleShip-Remix.o2r`
into its app data folder. The macOS app is not notarized: the first time, right-click it and choose Open (or run
`xattr -dr com.apple.quarantine /Applications/BattleShip-Remix.app`). Later updates install from inside the game.

**Nintendo Switch:** the console does not read the ROM. Run a desktop build once to extract `BattleShip-Remix.o2r`
and `BattleShip-Remix.o2r.recipe`, unzip `BattleShip-Remix-switch.zip` to the root of the SD card, copy the two files
next to `BattleShip-Remix.nro` in `sdmc:/switch/BattleShip-Remix/`, and launch it in full-memory mode (hold R while
opening an installed game). Both files must come from a desktop build of the same release as the `.nro`: the game
refuses to start if the `.recipe` is missing or does not match, and the error screen shows both recipes. When a release
changes the asset pipeline, copy the newly extracted `.o2r` and `.recipe` again.

**Android (arm64):** install `BattleShip-Remix-android.apk` (allow installs from unknown sources), open it and pick the
Smash Remix 2.0.1 ROM; it extracts `BattleShip-Remix.o2r` on the phone. It installs next to the vanilla BattleShip app.
32-bit Android (armeabi-v7a) is not shipped yet.

**PlayStation 5 (homebrew):** needs a jailbroken PS5 with ShadowMountPlus. Unzip
`BattleShip-Remix-ps5-PPSA99264.zip` to `/data/homebrew/` (or the same folder on an external drive), so the game sits
in `PPSA99264/`, copy your ROM into that folder as `baserom.remix.z64` (by FTP), and open BattleShip Remix from the
Games row. The first launch extracts `BattleShip-Remix.o2r` into the same folder (a system notification tells when it
starts and ends); saves, settings and logs stay there too. L3+R3 opens the port menu (the Create button belongs to the
system).

## Files

| File | Platform |
|---|---|
| `BattleShip-Remix.dmg` | macOS (Apple Silicon) |
| `BattleShip-Remix-macos.zip` | macOS (Apple Silicon), the `.app` zipped; the in-game updater downloads it |
| `BattleShip-Remix-x86_64.AppImage` | Linux x86_64 |
| `BattleShip-Remix-windows.zip` | Windows x64 |
| `BattleShip-Remix-switch.zip` | Nintendo Switch homebrew (`.nro`, `f3d.o2r`, `config.yml`, `assets/`) |
| `BattleShip-Remix.nro` | The Switch `.nro` alone, kept for builds up to `v2.0.1-port.9` (their updater downloads it); newer builds update from the Switch zip |
| `BattleShip-Remix-android.apk` | Android arm64-v8a, Android 7+ |
| `BattleShip-Remix-ps5-PPSA99264.zip` | PlayStation 5 homebrew title folder (`PPSA99264/`) |
| `SHA256SUMS.txt` | Checksums |

Report bugs on [this repository's issue tracker](https://github.com/lucasliet/BattleShip-remix/issues), not on
BattleShip, Smash Remix or Harbour Masters.
