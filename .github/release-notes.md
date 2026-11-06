## Highlights

**New: self-updating Android and Switch.** The game now downloads and installs new releases itself.

- Android: on launch the app checks for a new release, downloads the APK and asks Android to install it (one tap to
  confirm). The first install from this release must be done by hand; later releases update in place. If you installed
  a build signed with another key, uninstall it once first.
- Switch: About > Check for updates downloads the new `BattleShip-Remix.nro` and replaces it on the SD card, then
  Restart Game relaunches it. The release now also ships the bare `BattleShip-Remix.nro`.
- Remix menu: every setting has a short tooltip explaining what it does.

## Install

No game data is included: you need your own ROM.

- `baserom.remix.z64`: Smash Remix 2.0.1, made by applying the Remix 2.0.1 patch from the
  [Smash Remix releases](https://github.com/JSsixtyfour/smashremix/releases) to your Super Smash Bros. NTSC-U v1.0 ROM
  (SHA-1 `13716381ed5e29be2606e0f4724b18fd00789d04`; `.z64`, `.n64` or `.v64`).

**macOS, Linux, Windows:** run the app. On the first launch it asks for the ROM and extracts `BattleShip-Remix.o2r`
into its app data folder. The macOS app is not notarized: the first time, right-click it and choose Open (or run
`xattr -dr com.apple.quarantine /Applications/BattleShip-Remix.app`).

**Nintendo Switch:** the console does not read the ROM. Run a desktop build once to extract `BattleShip-Remix.o2r`
and `BattleShip-Remix.o2r.recipe`, unzip `BattleShip-Remix-switch.zip` to the root of the SD card, copy the two files
next to `BattleShip-Remix.nro` in `sdmc:/switch/BattleShip-Remix/`, and launch it in full-memory mode (hold R while
opening an installed game). When a release changes the asset pipeline, copy the newly extracted `.o2r` and `.recipe`
again.

**Android (arm64):** install `BattleShip-Remix-android.apk` (allow installs from unknown sources), open it and pick the
Smash Remix 2.0.1 ROM; it extracts `BattleShip-Remix.o2r` on the phone. It installs next to the vanilla BattleShip app.
32-bit Android (armeabi-v7a) is not shipped yet.

## Files

| File | Platform |
|---|---|
| `BattleShip-Remix.dmg` | macOS (Apple Silicon) |
| `BattleShip-Remix-x86_64.AppImage` | Linux x86_64 |
| `BattleShip-Remix-windows.zip` | Windows x64 |
| `BattleShip-Remix-switch.zip` | Nintendo Switch homebrew (`.nro`, `f3d.o2r`, `config.yml`, `assets/`) |
| `BattleShip-Remix-android.apk` | Android arm64-v8a, Android 7+ |
| `SHA256SUMS.txt` | Checksums |

Report bugs on [this repository's issue tracker](https://github.com/lucasliet/BattleShip-remix/issues), not on
BattleShip, Smash Remix or Harbour Masters.
