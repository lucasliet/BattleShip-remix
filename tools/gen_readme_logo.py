#!/usr/bin/env python3
"""Generate the README wordmark assets/SmashRemix.png.

Same treatment as the SR icon (tools/gen_switch_icon.py) and the original BattleShip wordmark: a distressed
yellow-to-orange fill, a red outline and a black offset shadow, over a black starburst, transparent background.

Needs Pillow only: .toolchain/host/venv/bin/python -I tools/gen_readme_logo.py
"""

import math
import random
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont, ImageOps

REPO_ROOT = Path(__file__).resolve().parent.parent
OUTPUT_PNG = REPO_ROOT / "assets" / "SmashRemix.png"
FONT_PATH = "/System/Library/Fonts/Supplemental/Arial Black.ttf"

WIDTH, HEIGHT = 2300, 640
WORD = "SMASH REMIX"
LETTER_SIZE = 220
LETTER_STRETCH_Y = 1.3
LETTER_TRACKING = 22
OUTLINE_GROW = 15
SHADOW_OFFSET = (18, 22)
FILL_TOP = (255, 238, 0)
FILL_MID = (255, 176, 0)
FILL_BOTTOM = (255, 118, 0)
OUTLINE_COLOR = (255, 0, 0)
STAR_SPIKES = 26
STAR_INNER = (0.80, 0.70)
STAR_OUTER = (1.0, 1.0)
STAR_JITTER = 0.10
RNG_SEED = 64


def glyph(character, font):
    canvas = Image.new("L", (LETTER_SIZE * 2, LETTER_SIZE * 2), 0)
    ImageDraw.Draw(canvas).text((LETTER_SIZE // 4, 0), character, font=font, fill=255)
    box = canvas.getbbox()
    if box is None:
        return None
    cropped = canvas.crop(box)
    return cropped.resize((cropped.width, round(cropped.height * LETTER_STRETCH_Y)), Image.LANCZOS)


def lettering_shape():
    """Filled shape of the word, centred on the canvas."""
    font = ImageFont.truetype(FONT_PATH, LETTER_SIZE)
    space = round(LETTER_SIZE * 0.35)
    glyphs = [glyph(character, font) for character in WORD]
    total = sum(space if g is None else g.width + LETTER_TRACKING for g in glyphs) - LETTER_TRACKING
    height = max(g.height for g in glyphs if g is not None)
    shape = Image.new("L", (WIDTH, HEIGHT), 0)
    x = (WIDTH - total) // 2
    top = (HEIGHT - height) // 2 - SHADOW_OFFSET[1] // 2
    for g in glyphs:
        if g is None:
            x += space
            continue
        shape = ImageChops.lighter(shape, paste_at(g, x, top + height - g.height))
        x += g.width + LETTER_TRACKING
    return shape.filter(ImageFilter.GaussianBlur(3)).point(lambda value: 255 if value > 128 else 0)


def paste_at(image, x, y):
    layer = Image.new("L", (WIDTH, HEIGHT), 0)
    layer.paste(image, (x, y))
    return layer


def grow(shape, radius):
    return shape.filter(ImageFilter.MaxFilter(radius * 2 + 1)).filter(ImageFilter.GaussianBlur(2)).point(
        lambda value: 255 if value > 110 else 0
    )


def starburst(box):
    """Black spiky ellipse around the lettering box, deterministic."""
    rng = random.Random(RNG_SEED)
    centre_x, centre_y = (box[0] + box[2]) / 2, (box[1] + box[3]) / 2
    radius_x = (box[2] - box[0]) / 2 + 90
    radius_y = (box[3] - box[1]) / 2 + 110
    points = []
    for index in range(STAR_SPIKES * 2):
        angle = math.pi * index / STAR_SPIKES
        scale = STAR_OUTER if index % 2 == 0 else STAR_INNER
        jitter = 1 + rng.uniform(-STAR_JITTER, STAR_JITTER)
        points.append((centre_x + math.cos(angle) * radius_x * scale[0] * jitter,
                       centre_y + math.sin(angle) * radius_y * scale[1] * jitter))
    mask = Image.new("L", (WIDTH, HEIGHT), 0)
    ImageDraw.Draw(mask).polygon(points, fill=255)
    return mask


def noise_field(cells, seed):
    random.seed(seed)
    field = Image.effect_noise((cells * WIDTH // HEIGHT, cells), 90).resize((WIDTH, HEIGHT), Image.BICUBIC)
    return ImageOps.autocontrast(field, cutoff=1)


def vertical_ramp(box, top_value, bottom_value):
    ramp = Image.new("L", (WIDTH, HEIGHT), top_value)
    draw = ImageDraw.Draw(ramp)
    for y in range(box[1], box[3]):
        ratio = (y - box[1]) / (box[3] - box[1])
        draw.line([(0, y), (WIDTH, y)], fill=round(top_value + (bottom_value - top_value) * ratio))
    return ramp


def gradient_fill(shape):
    box = shape.getbbox()
    fill = Image.new("RGB", (WIDTH, HEIGHT), FILL_TOP)
    draw = ImageDraw.Draw(fill)
    for y in range(box[1], box[3]):
        ratio = min(1.0, (y - box[1]) / (box[3] - box[1]) * 1.1)
        colour = tuple(round(FILL_MID[c] + (FILL_BOTTOM[c] - FILL_MID[c]) * ratio) for c in range(3))
        draw.line([(0, y), (WIDTH, y)], fill=colour)
    field = Image.blend(noise_field(14, RNG_SEED), noise_field(48, RNG_SEED + 1), 0.5)
    patches = ImageChops.add(field, vertical_ramp(box, 225, 40), 1, -128).point(lambda v: 255 if v > 128 else 0)
    fill.paste(Image.new("RGB", (WIDTH, HEIGHT), FILL_TOP), mask=patches)
    return fill


def compose():
    shape = lettering_shape()
    outline = grow(shape, OUTLINE_GROW)
    shadow = ImageChops.offset(outline, *SHADOW_OFFSET)
    image = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    image.paste((0, 0, 0, 255), mask=starburst(outline.getbbox()).filter(ImageFilter.GaussianBlur(1)))
    image.paste((0, 0, 0, 255), mask=shadow.filter(ImageFilter.GaussianBlur(1)))
    image.paste(OUTLINE_COLOR + (255,), mask=outline.filter(ImageFilter.GaussianBlur(1)))
    image.paste(gradient_fill(shape), mask=shape.filter(ImageFilter.GaussianBlur(1)))
    return image.crop(image.getbbox())


def main():
    compose().save(OUTPUT_PNG, optimize=True)
    print(f"wrote {OUTPUT_PNG}")


if __name__ == "__main__":
    main()
