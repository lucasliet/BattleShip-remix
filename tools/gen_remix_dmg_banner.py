#!/usr/bin/env python3
"""Generate the BattleShip-Remix DMG background (assets/macos_dmg_banner-remix.png).

The blue wave texture of the original banner (assets/macos_dmg_banner.png, below its BATTLESHIP logo)
is scaled to the banner size and the Smash Remix title (assets/SmashRemix.png) goes where the original
logo sits, leaving the lower part free for the app and Applications icons.

Needs Pillow only (run it with the repo venv: .toolchain/host/venv/bin/python -I tools/gen_remix_dmg_banner.py).
"""

from pathlib import Path

from PIL import Image

REPO_ROOT = Path(__file__).resolve().parent.parent
ASSETS = REPO_ROOT / "assets"
ORIGINAL_BANNER = ASSETS / "macos_dmg_banner.png"
TITLE = ASSETS / "SmashRemix.png"
OUTPUT = ASSETS / "macos_dmg_banner-remix.png"

TEXTURE_TOP = 460
TITLE_WIDTH = 1300
TITLE_TOP = 60


def main():
    banner = Image.open(ORIGINAL_BANNER).convert("RGB")
    texture = banner.crop((0, TEXTURE_TOP, banner.width, banner.height))
    scale = max(banner.width / texture.width, banner.height / texture.height)
    texture = texture.resize((round(texture.width * scale), round(texture.height * scale)), Image.LANCZOS)
    left = (texture.width - banner.width) // 2
    top = (texture.height - banner.height) // 2
    canvas = texture.crop((left, top, left + banner.width, top + banner.height)).convert("RGBA")

    title = Image.open(TITLE).convert("RGBA")
    title = title.resize((TITLE_WIDTH, round(title.height * TITLE_WIDTH / title.width)), Image.LANCZOS)
    canvas.alpha_composite(title, ((banner.width - TITLE_WIDTH) // 2, TITLE_TOP))
    canvas.convert("RGB").save(OUTPUT)
    print(f"wrote {OUTPUT}")


if __name__ == "__main__":
    main()
