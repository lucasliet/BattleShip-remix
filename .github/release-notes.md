## Highlights

- **All platforms: settings changed in the game are kept after a restart**
  ([#6](https://github.com/lucasliet/BattleShip-remix/issues/6)). The game rejected its own save at every launch and
  wrote the defaults back, so sound, screen adjust, VS and 1P records, unlocks and Remix bonus records were lost each
  time. A save written by an older build is reset to the defaults once more at the first launch of this release;
  from then on it is kept. Item Switch and the VS rules (handicap and the like) are not part of the save in the original
  game or in Smash Remix, so they still start from their defaults. Port Menu settings were never affected.
- **All platforms: Remix settings changed inside the game are saved right away.** The stage select hazard mode, the
  Training menu, the CSS panel, the Remix modes and the profile loaders only reached `BattleShip.cfg.json` when
  something else saved it, so closing the game from the system (PS5 button, swiping the app away on Android) lost them.
- **All platforms: the Remix text drawn over the screen fits the widescreen picture.** With widescreen on, the CSS
  panel menu text (Shield, Player Tag, Visibility...) of the 1P and 4P cards spilled into the next card; it now sits
  inside its card, and the other Remix labels drawn the same way follow.
- **Conker's up special: the tail spins and stays on the body**
  ([#7](https://github.com/lucasliet/BattleShip-remix/issues/7)). The same cause broke other Remix specials, now
  fixed too: Marina's neutral special drew her head apart from the body, Dragon King sank into the floor during his
  down special, and Banjo's down special and Dedede's and Peach's neutral specials showed a wrong pose.
- **macOS, Linux and Windows: the ROM extractor is a separate program again.** The app, the AppImage and the Windows
  zip carry the `torch` program next to the game, as up to `v2.0.1-port.10`. The extracted files are the same, and an
  `.o2r` you already have is kept.

Nothing changed in the asset pipeline: on the Switch, keep the `BattleShip-Remix.o2r` and `.recipe` you already have.
Switch builds up to `v2.0.1-port.9` still download the bare `BattleShip-Remix.nro`, so that file stays in this release.

## Install

No game data is included: you need your own ROM.

- `baserom.remix.z64`: Smash Remix 2.0.1, made from your Super Smash Bros. NTSC-U v1.0 ROM with the
  [Smash Remix web patcher](https://smashremix.net/patcher/) (in the browser, nothing to install) or the Remix 2.0.1
  patch from the [Smash Remix releases](https://github.com/JSsixtyfour/smashremix/releases) (ROM SHA-1 `13716381ed5e29be2606e0f4724b18fd00789d04`; `.z64`, `.n64` or `.v64`).

**macOS, Linux, Windows:** run the app. On the first launch it asks for the ROM and extracts `BattleShip-Remix.o2r`
into its app data folder. The macOS app is not notarized: the first time, right-click it and choose Open (or run
`xattr -dr com.apple.quarantine /Applications/BattleShip-Remix.app`). On macOS and Linux later updates install from
inside the game; on Windows the game tells you about them and you extract the new zip into the game folder.

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
| `BattleShip-Remix-windows-x64.zip` | Windows x64 |
| `BattleShip-Remix-switch.zip` | Nintendo Switch homebrew (`.nro`, `f3d.o2r`, `config.yml`, `assets/`) |
| `BattleShip-Remix.nro` | The Switch `.nro` alone, kept for builds up to `v2.0.1-port.9` (their updater downloads it); newer builds update from the Switch zip |
| `BattleShip-Remix-android.apk` | Android arm64-v8a, Android 7+ |
| `BattleShip-Remix-ps5-PPSA99264.zip` | PlayStation 5 homebrew title folder (`PPSA99264/`) |
| `SHA256SUMS.txt` | Checksums |

Report bugs on [this repository's issue tracker](https://github.com/lucasliet/BattleShip-remix/issues), not on
BattleShip, Smash Remix or Harbour Masters.
