#!/usr/bin/env python3
"""Generate the README wordmark assets/BattleShip-Remix.svg from assets/BattleShip.svg.

The original BATTLESHIP wordmark is kept as is. A "REMIX" row is added under it in the same treatment: a black
starburst, an offset black shadow, a red outline, the yellow-to-orange letter gradient and yellow distressed
patches near the top of the letters. E and I reuse the original glyph outlines; R, M and X are drawn in the same
blocky style. Stdlib only: .toolchain/host/venv/bin/python -I tools/gen_readme_logo.py
"""

import math
import random
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SOURCE_SVG = REPO_ROOT / "assets" / "BattleShip.svg"
OUTPUT_SVG = REPO_ROOT / "assets" / "BattleShip-Remix.svg"

SOURCE_VIEWBOX = "0 0 1992 602"
OUTPUT_HEIGHT = 900
CAP_HEIGHT = 240
ROW_SCALE = 0.9
ROW_TOP = 548
ROW_CENTRE_X = 996
LETTER_GAP = 2
OUTLINE_WIDTH = 46
SHADOW_OFFSET = (16, 20)
STAR_SPIKES = 15
RNG_SEED = 64

GLYPHS = {
    "R": ("M0 0H82C142 0 178 28 178 76C178 110 160 134 128 145L190 240H118L68 154H62V240H0Z"
          "M62 50V106H80C104 106 116 96 116 78C116 60 104 50 80 50Z", 190),
    "E": ("m1212.5549 345.0209-15.6599 51.81592h-143.7028V161.47737h140.709l11.9753 51.81593h-91.887v37.53776"
          "h66.0941v51.81592h-66.5546v42.37392z", None),
    "M": ("M0 240V0H64L108 104L152 0H216V240H154V110L122 184H94L62 110V240Z", 216),
    "I": ("m1650.6068 496.60861-59.6459 4.37557-.2302-239.73502 59.8761-6.4482z", None),
    "X": ("M0 0H70L102 66L134 0H204L138 118L208 240H138L102 172L66 240H-4L66 118Z", 208),
}
ORIGINAL_BOXES = {"E": (1053.152, 161.477, 1224.530, 396.837), "I": (1590.731, 254.800, 1650.607, 500.984)}
ROTATIONS = {"R": -2.5, "E": 3.0, "M": -1.5, "I": 4.0, "X": -3.0}


def glyph_transform(letter, left):
    """Transform that puts a glyph with its top-left at (left, ROW_TOP), cap height CAP_HEIGHT * ROW_SCALE."""
    path, width = GLYPHS[letter]
    if width is None:
        x0, y0, x1, y1 = ORIGINAL_BOXES[letter]
        scale = CAP_HEIGHT * ROW_SCALE / (y1 - y0)
        width = (x1 - x0) * scale
        base = f"translate({left:.2f} {ROW_TOP}) scale({scale:.5f}) translate({-x0:.3f} {-y0:.3f})"
    else:
        width *= ROW_SCALE
        base = f"translate({left:.2f} {ROW_TOP}) scale({ROW_SCALE})"
    centre = (left + width / 2, ROW_TOP + CAP_HEIGHT * ROW_SCALE / 2)
    rotate = f"rotate({ROTATIONS[letter]} {centre[0]:.2f} {centre[1]:.2f})"
    return path, f"{rotate} {base}", width


def row_layout(word):
    widths = [glyph_transform(letter, 0)[2] for letter in word]
    left = ROW_CENTRE_X - (sum(widths) + LETTER_GAP * (len(word) - 1)) / 2
    placed = []
    for letter, width in zip(word, widths):
        path, transform, _ = glyph_transform(letter, left)
        placed.append((path, transform))
        left += width + LETTER_GAP
    return placed


def starburst_points():
    rng = random.Random(RNG_SEED)
    centre_x, centre_y = ROW_CENTRE_X, ROW_TOP + CAP_HEIGHT * ROW_SCALE / 2
    radius_x, radius_y = 720, 290
    points = []
    for index in range(STAR_SPIKES * 2):
        angle = math.pi * index / STAR_SPIKES
        reach = 1.0 if index % 2 == 0 else 0.52
        jitter = 1 + rng.uniform(-0.12, 0.12)
        points.append(f"{centre_x + math.cos(angle) * radius_x * reach * jitter:.1f},"
                      f"{centre_y + math.sin(angle) * radius_y * reach * jitter:.1f}")
    return " ".join(points)


def letters(placed, style, extra=""):
    return "\n".join(f'    <path d="{path}" transform="{extra}{transform}" style="{style}"/>' for path, transform in placed)


def remix_layer():
    placed = row_layout("REMIX")
    top, bottom = ROW_TOP, ROW_TOP + CAP_HEIGHT * ROW_SCALE
    shadow = f"translate({SHADOW_OFFSET[0]} {SHADOW_OFFSET[1]}) "
    outline = f"fill:#000;stroke:#000;stroke-width:{OUTLINE_WIDTH};stroke-linejoin:round;fill-rule:evenodd"
    red = f"fill:red;stroke:red;stroke-width:{OUTLINE_WIDTH};stroke-linejoin:round;fill-rule:evenodd"
    return f"""
  <defs>
    <linearGradient id="rx-fill" x1="0" x2="0" y1="0" y2="1">
      <stop offset=".2" style="stop-color:#ff0"/>
      <stop offset="1" style="stop-color:#ff6400"/>
    </linearGradient>
    <linearGradient id="rx-fade" x1="0" x2="0" y1="{top}" y2="{bottom}" gradientUnits="userSpaceOnUse">
      <stop offset="0" style="stop-color:#fff"/>
      <stop offset=".55" style="stop-color:#000"/>
    </linearGradient>
    <mask id="rx-top" maskUnits="userSpaceOnUse" x="0" y="0" width="1992" height="{OUTPUT_HEIGHT}">
      <rect x="0" y="{top - 40}" width="1992" height="{bottom - top + 80}" fill="url(#rx-fade)"/>
    </mask>
    <filter id="rx-grunge" x="0" y="0" width="1992" height="{OUTPUT_HEIGHT}" filterUnits="userSpaceOnUse">
      <feTurbulence type="fractalNoise" baseFrequency="0.035 0.09" numOctaves="3" seed="7" result="noise"/>
      <feColorMatrix in="noise" type="matrix" values="0 0 0 0 1  0 0 0 0 1  0 0 0 0 0.05  0 0 0 14 -8.4" result="spots"/>
      <feComposite in="spots" in2="SourceAlpha" operator="in"/>
    </filter>
  </defs>
  <g id="remix">
    <polygon points="{starburst_points()}" style="fill:#000"/>
{letters(placed, outline, shadow)}
{letters(placed, red)}
{letters(placed, "fill:url(#rx-fill);fill-rule:evenodd")}
    <g mask="url(#rx-top)" filter="url(#rx-grunge)">
{letters(placed, "fill:#fff;fill-rule:evenodd")}
    </g>
  </g>
"""


def main():
    source = SOURCE_SVG.read_text()
    if f'viewBox="{SOURCE_VIEWBOX}"' not in source:
        raise SystemExit(f"{SOURCE_SVG}: expected viewBox {SOURCE_VIEWBOX!r}; update the layout constants")
    remix = source.replace(f'viewBox="{SOURCE_VIEWBOX}"', f'viewBox="0 0 1992 {OUTPUT_HEIGHT}"', 1)
    body_start = remix.index("</defs>") + len("</defs>")
    remix = remix[:body_start] + remix_layer() + remix[body_start:]
    OUTPUT_SVG.write_text(remix)
    print(f"wrote {OUTPUT_SVG}")


if __name__ == "__main__":
    main()
