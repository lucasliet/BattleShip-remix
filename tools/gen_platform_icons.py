#!/usr/bin/env python3
"""Generate the platform icons from the two master icons.

Inputs (artwork, checked in):
  assets/icon.png                  original BattleShip tile, 1024x1024 RGBA
  assets/icon-remix.png            Smash Remix "SR" tile, 1024x1024 RGBA (transparent corners)

Outputs:
  assets/switch/icon.jpg           256x256 JPEG for the original BattleShip .nro
  assets/switch/icon-remix.jpg     256x256 JPEG for the BattleShip-Remix .nro
  assets/ps5/icon0.png             512x512 RGB for the PS5 title (sce_sys/icon0.png)
  assets/icon-remix.ico            Windows icon of BattleShip-Remix.exe (16 to 256 px)
  android/app/src/remix/res/drawable-nodpi/ic_launcher_foreground.png
                                   432x432 RGBA adaptive-icon foreground, tile in the safe zone

The macOS .icns and the Linux AppImage icon are made from assets/icon-remix.png at packaging time.
Needs Pillow only (run it with the repo venv: .toolchain/host/venv/bin/python -I tools/gen_platform_icons.py).
"""

from pathlib import Path

from PIL import Image

REPO_ROOT = Path(__file__).resolve().parent.parent
ASSETS = REPO_ROOT / "assets"
ORIGINAL_PNG = ASSETS / "icon.png"
REMIX_PNG = ASSETS / "icon-remix.png"
ANDROID_FOREGROUND = REPO_ROOT / "android/app/src/remix/res/drawable-nodpi/ic_launcher_foreground.png"

SWITCH_SIZE = 256
PS5_SIZE = 512
ANDROID_CANVAS = 432
# Adaptive icons crop to a 264 px circle-ish mask out of 432; the tile fits inside it.
ANDROID_TILE = 288
WINDOWS_ICON_SIZES = [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]


def flatten_on_black(tile, size):
    """Scales an RGBA tile to size and fills its transparent corners with black."""
    flat = Image.new("RGB", tile.size, (0, 0, 0))
    flat.paste(tile, mask=tile.getchannel("A"))
    return flat.resize((size, size), Image.LANCZOS)


def write_android_foreground(tile, path):
    """Centres the tile on a transparent adaptive-icon canvas."""
    canvas = Image.new("RGBA", (ANDROID_CANVAS, ANDROID_CANVAS), (0, 0, 0, 0))
    scaled = tile.resize((ANDROID_TILE, ANDROID_TILE), Image.LANCZOS)
    margin = (ANDROID_CANVAS - ANDROID_TILE) // 2
    canvas.alpha_composite(scaled, (margin, margin))
    canvas.save(path)


def main():
    original = Image.open(ORIGINAL_PNG).convert("RGBA")
    remix = Image.open(REMIX_PNG).convert("RGBA")
    (ASSETS / "switch").mkdir(parents=True, exist_ok=True)
    flatten_on_black(original, SWITCH_SIZE).save(ASSETS / "switch/icon.jpg", "JPEG", quality=92, optimize=True)
    flatten_on_black(remix, SWITCH_SIZE).save(ASSETS / "switch/icon-remix.jpg", "JPEG", quality=92, optimize=True)
    flatten_on_black(remix, PS5_SIZE).save(ASSETS / "ps5/icon0.png")
    write_android_foreground(remix, ANDROID_FOREGROUND)
    remix.save(ASSETS / "icon-remix.ico", sizes=WINDOWS_ICON_SIZES)
    print("wrote the Switch, PS5, Android and Windows icons")


if __name__ == "__main__":
    main()
