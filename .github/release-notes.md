## Highlights

**New: PlayStation 5.** BattleShip-Remix now runs as a native homebrew title (`PPSA99264`, "BattleShip Remix") on
jailbroken PS5 consoles with ShadowMountPlus. Unzip the folder to `/data/homebrew/` (or an external drive), drop your
ROM in it, and the first launch converts it on the console, with system notifications while it works. Saves,
settings and logs stay in the title folder. The touchpad, or L3+R3, opens the port menu, which works with the pad;
its About page checks GitHub for new releases and installs them in place, starting with the next release.

**New artwork.** A new Smash Remix logo and "SR" icon on every platform: the macOS app and DMG background, the
Windows executable, the Linux AppImage and window icon, the Android launcher, the Switch Home menu and the PS5 home
screen tile and background.

The self-updaters (Android, Switch, desktop) offer this version automatically.

## Install

No game data is included: you need your own ROM.

- `baserom.remix.z64`: Smash Remix 2.0.1, made from your Super Smash Bros. NTSC-U v1.0 ROM with the
  [Smash Remix web patcher](https://smashremix.net/patcher/) (in the browser, nothing to install) or the Remix 2.0.1
  patch from the [Smash Remix releases](https://github.com/JSsixtyfour/smashremix/releases) (ROM SHA-1 `13716381ed5e29be2606e0f4724b18fd00789d04`; `.z64`, `.n64` or `.v64`).

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
| `BattleShip-Remix-x86_64.AppImage` | Linux x86_64 |
| `BattleShip-Remix-windows.zip` | Windows x64 |
| `BattleShip-Remix-switch.zip` | Nintendo Switch homebrew (`.nro`, `f3d.o2r`, `config.yml`, `assets/`) |
| `BattleShip-Remix-android.apk` | Android arm64-v8a, Android 7+ |
| `BattleShip-Remix-ps5-PPSA99264.zip` | PlayStation 5 homebrew title folder (`PPSA99264/`) |
| `SHA256SUMS.txt` | Checksums |

Report bugs on [this repository's issue tracker](https://github.com/lucasliet/BattleShip-remix/issues), not on
BattleShip, Smash Remix or Harbour Masters.
