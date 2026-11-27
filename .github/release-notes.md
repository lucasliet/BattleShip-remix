## Highlights

- **Remix 1P: the Kirby Team stage no longer crashes.** Two faults on that stage are fixed: a Kirby could appear as a
  hat with no body and crash the game shortly after, and a Kirby whose copied power fires a projectile (Banjo's egg,
  for example) crashed the game when it used its neutral special. Reported in issue #10.
- **Remix text is drawn by the game, in front of or behind the same things as in Smash Remix.** The texts that Smash
  Remix adds (stock counts and the Manual stock marks, the panel menus, the 12-Character Battle texts, player tags,
  the match HUD of the Remix modes, Training rows, the pause legend, the stage select, Match Stats and more) were drawn
  by the port over the whole picture, so the cursor hand, held tokens and menu panels passed behind them. They are now
  drawn in the same layers as in Smash Remix: the hand covers "Shield" and the stock numbers, a menu panel covers the
  text under it, and the Training rows go under the Z menu and the pause menu. The text keeps its place in widescreen
  with 2D stretched, with 2D centred in 4:3 and with widescreen off, and inside each card of the panel menus. The edges
  of the letters are a little softer than before, as the game draws them.
- **12-Character Battle: the Random entry works in the Custom set.** The "?" slot picks a fighter when the match
  starts, the loser slot shows the fighter that was rolled, and undo and reset bring the "?" back. The Custom legend
  hides while the panel menu of the token holder is open, and "Game NN" on the results appears with the results label.
- **Input Display** is drawn as game sprites, in the layer Smash Remix uses, instead of a separate overlay.
- **Every Smash Remix patch was re-checked against the port** (all 2,007 of the ROM hack's patches, read one by one
  from its assembly source). One missing piece was found and fixed: leaving the VS results screen could restart its
  music as the screen tore down, and with Menu Music on a Random setting the character select then kept the results
  track instead of playing menu music. The exit guard Smash Remix uses is now in place; on desktop the race did not
  actually trigger, so this matches Smash Remix behavior on every audio backend rather than changing anything audible
  there.

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
| `BattleShip-Remix-switch.zip` | Nintendo Switch homebrew (`.nro`, `f3d.o2r`, `config.yml`, the fonts in `assets/custom/fonts/`) |
| `BattleShip-Remix.nro` | The Switch `.nro` alone, kept for builds up to `v2.0.1-port.9` (their updater downloads it); newer builds update from the Switch zip |
| `BattleShip-Remix-android.apk` | Android arm64-v8a, Android 7+ |
| `BattleShip-Remix-ps5-PPSA99264.zip` | PlayStation 5 homebrew title folder (`PPSA99264/`) |
| `SHA256SUMS.txt` | Checksums |

Report bugs on [this repository's issue tracker](https://github.com/lucasliet/BattleShip-remix/issues), not on
BattleShip, Smash Remix or Harbour Masters.
